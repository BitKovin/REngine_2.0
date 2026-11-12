#include "GoreDebug.h"

#ifndef DISTRIBUTION

#include "GoreComponent.h"
#include "GoreEvents.h"
#include "GoreProfile.h"
#include "GoreSkeletalMesh.h"
#include "GoreSource.h"
#include "GoreSystem.h"

#include <Camera.h>
#include <DebugDraw.hpp>
#include <Entity.h>
#include <MathHelper.hpp>
#include <Physics.h>
#include <Ecs/Ecs.h>

#include <imgui/imgui.h>

#include <cmath>
#include <cstring>
#include <map>
#include <string>
#include <tuple>

namespace Gore
{
    namespace
    {
        char g_className[64] = "weapon_pistol";
        bool g_drawVolumes = true;
        std::string g_closedMeshReport;
        uint32_t g_debugCounter = 0;

        // The NPC (with a gore mesh) under the crosshair, plus the trace data.
        Physics::HitResult TraceCrosshair(GoreSkeletalMesh*& meshOut, Entity*& entityOut)
        {
            meshOut = nullptr;
            entityOut = nullptr;
            const vec3 start = Camera::finalizedPosition;
            const vec3 end = start + MathHelper::GetForwardVector(Camera::finalizedRotation) * 60.0f;
            Physics::HitResult hit = Physics::LineTrace(start, end, BodyType::GroupHitTest);
            if (hit.hasHit && hit.entity != nullptr)
            {
                entityOut = hit.entity;
                meshOut = FindGoreMesh(hit.entity);
            }
            return hit;
        }

        void DrawWoundVolumes(GoreSkeletalMesh* mesh, const GoreComponent& gore)
        {
            const std::vector<mat4>& palette = mesh->GetSkinPalette();
            const mat4 world = mesh->GetWorldMatrix();

            for (int i = 0; i < gore.woundCount; ++i)
            {
                const Wound& w = gore.wounds[i];
                if (w.anchorSkin < 0 || w.anchorSkin >= static_cast<int>(palette.size())) continue;

                const mat4 toWorld = world * palette[w.anchorSkin];
                const mat3 rot = glm::mat3_cast(w.rotation);
                const vec3 axes[3] = { rot[0] * w.radii.x, rot[1] * w.radii.y, rot[2] * w.radii.z };
                const vec3 c = vec3(toWorld * vec4(w.center, 1.0f));

                for (int a = 0; a < 3; ++a)
                {
                    const vec3 d = vec3(toWorld * vec4(axes[a], 0.0f));
                    DebugDraw::Line(c - d, c + d, 0.05f, 0.004f, a == 2 ? DebugColor::Blue : DebugColor::Green);
                }
            }

            for (int i = 0; i < gore.cutCount; ++i)
            {
                const Cut& cut = gore.cuts[i];
                if (cut.boneSkin < 0 || cut.boneSkin >= static_cast<int>(palette.size())) continue;

                const mat4 toWorld = world * palette[cut.boneSkin];
                const vec3 n = glm::normalize(vec3(cut.plane));
                const vec3 c = vec3(toWorld * vec4(cut.center, 1.0f));
                const vec3 nw = glm::normalize(vec3(toWorld * vec4(n, 0.0f)));
                DebugDraw::Line(c, c + nw * 0.15f, 0.05f, 0.006f, DebugColor::Red);
            }
        }

        // Counts edges used by exactly one triangle after welding positions (1e-5). Must be 0 for a closed body.
        void CheckClosedMeshes(GoreSkeletalMesh* mesh)
        {
            g_closedMeshReport.clear();
            const roj::SkinnedModel* model = mesh->GetSkinnedModel();
            if (model == nullptr) return;

            const Profile& profile = GetProfile(mesh->filePath, *model);
            for (size_t m = 0; m < model->meshes.size(); ++m)
            {
                const roj::SkinnedMesh& sm = model->meshes[m];
                const bool volume = m < profile.submeshRoles.size() && profile.submeshRoles[m] == SubmeshRole::Volume;
                if (!volume) continue;

                std::map<std::tuple<long, long, long>, uint32_t> weld;
                std::vector<uint32_t> remap(sm.vertices.size());
                for (size_t v = 0; v < sm.vertices.size(); ++v)
                {
                    const vec3& p = sm.vertices[v].Position;
                    const auto key = std::make_tuple(std::lround(p.x * 1.0e5f), std::lround(p.y * 1.0e5f), std::lround(p.z * 1.0e5f));
                    remap[v] = weld.emplace(key, static_cast<uint32_t>(weld.size())).first->second;
                }

                std::map<std::pair<uint32_t, uint32_t>, int> edges;
                for (size_t i = 0; i + 2 < sm.indices.size(); i += 3)
                {
                    for (int k = 0; k < 3; ++k)
                    {
                        uint32_t a = remap[sm.indices[i + k]];
                        uint32_t b = remap[sm.indices[i + (k + 1) % 3]];
                        if (a == b) continue;
                        if (a > b) std::swap(a, b);
                        edges[{ a, b }]++;
                    }
                }

                int open = 0;
                for (const auto& e : edges)
                    if (e.second == 1) ++open;

                g_closedMeshReport += sm.name + ": " + std::to_string(open) + " open edges\n";
            }
        }
    }

    void DrawDebugWindow()
    {
        ImGui::Begin("Gore");

        GoreSkeletalMesh* mesh = nullptr;
        Entity* entity = nullptr;
        const Physics::HitResult hit = TraceCrosshair(mesh, entity);

        ImGui::InputText("source class", g_className, sizeof(g_className));
        ImGui::Checkbox("draw volumes", &g_drawVolumes);

        if (mesh == nullptr)
        {
            ImGui::TextUnformatted("No gore mesh under the crosshair.");
            ImGui::End();
            return;
        }

        ImGui::Text("Target: %s (%s)", entity->Id.c_str(), hit.hitboxName.c_str());

        if (ImGui::Button("Wound under crosshair"))
        {
            GoreHitEvent event;
            event.victim = EntityHandle::FromEntity(entity);

            Hit& h = event.hit;
            h.point = hit.position;
            h.direction = MathHelper::GetForwardVector(Camera::finalizedRotation);
            h.sweepNormal = MathHelper::GetUpVector(Camera::finalizedRotation);
            h.damage = 30.0f;
            h.sourceIndex = SourceIndexForClass(g_className);
            std::strncpy(h.hitBone, hit.hitboxName.c_str(), sizeof(h.hitBone) - 1);
            h.counter = ++g_debugCounter;
            h.seed = HashSeed(entity->Id, 0x80000000u + h.counter);

            EcsScheduler::Emit(event);
        }

        if (entity->HasComponent<GoreComponent>())
        {
            auto gore = entity->GetComponent<GoreComponent>();
            ImGui::Text("wounds %d  cuts %d  splats %d  pending %d  version %u",
                        gore->woundCount, gore->cutCount, gore->splatCount, gore->pendingCount, gore->version);
            if (g_drawVolumes) DrawWoundVolumes(mesh, *gore);
        }
        else
        {
            ImGui::TextUnformatted("No gore yet.");
        }

        ImGui::Text("render version %u", mesh->GetGoreVersion());

        if (ImGui::Button("Check closed mesh"))
            CheckClosedMeshes(mesh);
        if (!g_closedMeshReport.empty())
            ImGui::TextUnformatted(g_closedMeshReport.c_str());

        ImGui::End();
    }
}

#else

namespace Gore
{
    void DrawDebugWindow() {}
}

#endif
