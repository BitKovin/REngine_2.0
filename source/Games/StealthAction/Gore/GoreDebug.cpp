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
#include "../Entities/Enemy/Enemies/NpcHumanBase.h"
#include <Screenshot.h>
#include <Texture.hpp>
#include <EngineMain.h>
#include <Level.hpp>
#include <LevelObjectFactory.h>
#include <Time.h>
#include "../Entities/Player/Player.hpp"
#include <Ecs/Ecs.h>

#include <imgui/imgui.h>

#include <SDL2/SDL.h>
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
            Physics::HitResult hit = Physics::LineTrace(start, end, BodyType::GroupHitTest, {}, { Player::Instance });
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


    static std::string V3(const vec3& v) { return "(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ", " + std::to_string(v.z) + ")"; }
    static void ProbeLog(const std::string& m) { Logger::Log(m); }

    // Launch test: -gore_look <npcClass> [-gore_src <sourceClass>] [-gore_shots <n>]
    // Spawns the NPC in front of the player, then shoots it with trace hits (like real fire) so a -screenshot can show the result.
    void UpdateLookTest()
    {
        static const void* lastLevel = nullptr;
        static int ticks = 0;
        static Entity* npc = nullptr;
        static bool fired = false;

        auto& args = EngineMain::MainInstance->Arguments;
        auto look = args.find("gore_look");
        if (look == args.end() || look->second.empty()) return;
        if (Player::Instance == nullptr || Level::Current == nullptr) return;

        // Counts frames since the level changed (a freshly loaded level starts again at 0).
        if (lastLevel != Level::Current)
        {
            lastLevel = Level::Current;
            ticks = 0;
            npc = nullptr;
            fired = false;
        }
        ++ticks;

        Player* player = Player::Instance;

        if (npc == nullptr && ticks == 10)
        {
            const float yaw = Camera::finalizedRotation.y;
            const vec3 forward = MathHelper::GetForwardVector(vec3(0, yaw, 0));
            const vec3 eye = Camera::finalizedPosition;
            float dist = 1.8f;
            auto distArg = args.find("gore_dist");
            if (distArg != args.end() && !distArg->second.empty()) dist = static_cast<float>(std::atof(distArg->second[0].c_str()));
            vec3 pos = eye + forward * dist;
            auto ground = Physics::LineTrace(pos + vec3(0, 1, 0), pos - vec3(0, 4, 0), BodyType::World);
            pos = ground.hasHit ? ground.position + vec3(0, 1, 0) : pos;

            Entity* n = LevelObjectFactory::instance().create(look->second[0]);
            if (n == nullptr) { ProbeLog("gore_look: unknown class " + look->second[0]); npc = nullptr; ticks = -1000000; return; }
            n->Name = "gore_look_npc";
            n->Position = pos;   // before Start: the character controller is created at the entity position
            float yawOffset = 0.0f;
            auto yawArg = args.find("gore_yaw");
            if (yawArg != args.end() && !yawArg->second.empty()) yawOffset = static_cast<float>(std::atof(yawArg->second[0].c_str()));
            n->Rotation = vec3(0, yaw + 180.0f + yawOffset, 0);
            Level::Current->AddEntity(n);
            n->LoadAssetsIfNeeded();
            n->Start();
            n->Teleport(pos);
            n->Rotation = vec3(0, yaw + 180.0f + yawOffset, 0);
            if (auto* human = dynamic_cast<NpcHumanBase*>(n))
                human->aiDisabled = true;   // stands still: the picture is about the wounds
            n->Health = 100000;
            n->MaxHealth = 100000;
            npc = n;
            ProbeLog("gore_look: spawned " + look->second[0] + " at " + V3(n->Position) + " player " + V3(player->Position)
                + " camera " + V3(Camera::finalizedPosition) + " camRot " + V3(Camera::finalizedRotation) + " wanted " + V3(pos));
        }

        int shotTick = 130;
        auto waitArg = args.find("gore_wait");
        if (waitArg != args.end() && !waitArg->second.empty()) shotTick = std::atoi(waitArg->second[0].c_str());

        if (fired && ticks == shotTick)
        {
            if (npc->HasComponent<GoreComponent>())
            {
                auto g = npc->GetComponent<GoreComponent>();
                ProbeLog("gore_look: wounds " + std::to_string(g->woundCount) + " splats " + std::to_string(g->splatCount) + " pending " + std::to_string(g->pendingCount)
                    + " version " + std::to_string(g->version) + " cuts " + std::to_string(g->cutCount));
            }
            else ProbeLog("gore_look: npc has no GoreComponent");
            if (npc->HasComponent<GoreComponent>())
            {
                auto g = npc->GetComponent<GoreComponent>();
                GoreSkeletalMesh* gm = FindGoreMesh(npc);
                for (int ci = 0; ci < g->cutCount && gm; ++ci)
                {
                    const Cut& c = g->cuts[ci];
                    const mat4 M = gm->GetWorldMatrix() * gm->GetSkinPalette()[c.boneSkin];
                    const std::string boneName = gm->GetSkinnedModel()->skeleton.bones[c.boneSkel].name.str();
                    ProbeLog("  cut " + std::to_string(ci) + " bone " + boneName + " centerWorld " + V3(vec3(M * vec4(c.center, 1))) + " boneWorld "
                        + V3(MathHelper::DecomposeMatrix(gm->GetBoneMatrixWorld(boneName)).Position) + " normalBind " + V3(vec3(c.plane)) + " outline " + std::to_string(c.outlineRadius)
                        + " influence " + std::to_string(c.influenceRadius) + " npcPos " + V3(npc->Position));
                }
            }
            if (GoreSkeletalMesh* gm = FindGoreMesh(npc))
                ProbeLog("gore_look: mesh gore version " + std::to_string(gm->GetGoreVersion()) + " hasGore " + std::to_string(gm->HasGore()));
            else ProbeLog("gore_look: npc has no GoreSkeletalMesh");
            if (GoreSkeletalMesh* gm = FindGoreMesh(npc))
            {
                const Profile& pr = GetProfile(gm->filePath, *gm->GetSkinnedModel());
                auto tex = [](Texture* t) { return t == nullptr ? std::string("null") : (std::string(t->valid ? "valid" : "INVALID") + (bgfx::isValid(t->getHandle()) ? "/handle" : "/NOHANDLE")); };
                ProbeLog("gore_look: profile valid " + std::to_string(pr.valid) + " flesh " + tex(pr.flesh) + " normal " + tex(pr.fleshNormal)
                    + " edge " + tex(pr.edgeNoise) + " blood " + tex(pr.blood));
            }
            auto file = args.find("gore_file");
            Screenshot::Request(file != args.end() && !file->second.empty() ? file->second[0] : std::string("gore_look"));
        }

        if (fired && ticks == shotTick + 30)
        {
            SDL_Event quit;   // the test run ends by itself (stdout is flushed on a normal exit)
            SDL_zero(quit);
            quit.type = SDL_QUIT;
            SDL_PushEvent(&quit);
        }

        if (npc != nullptr && !fired && ticks > 70)
        {
            fired = true;
            ticks = 71;
            std::string srcClass = "weapon_pistol";
            auto src = args.find("gore_src");
            if (src != args.end() && !src->second.empty()) srcClass = src->second[0];
            int shots = 7;
            auto sh = args.find("gore_shots");
            if (sh != args.end() && !sh->second.empty()) shots = std::atoi(sh->second[0].c_str());

            const vec3 eye = Camera::finalizedPosition;
            const vec3 center = npc->Position + vec3(0, 0.3f, 0);
            const vec3 right = MathHelper::GetRightVector(vec3(0, Camera::finalizedRotation.y, 0));
            int landed = 0;
            for (int i = 0; i < shots; ++i)
            {
                const float u = shots > 1 ? (static_cast<float>(i) / (shots - 1) - 0.5f) : 0.0f;
                vec3 target = center + right * (u * 0.5f) + vec3(0, 0.45f * std::sin(i * 2.1f), 0);
                auto aim = args.find("gore_aim");
                if (aim != args.end() && !aim->second.empty())
                {
                    if (GoreSkeletalMesh* gm = FindGoreMesh(npc))
                        target = MathHelper::DecomposeMatrix(gm->GetBoneMatrixWorld(aim->second[0])).Position + right * (u * 0.02f);
                }
                const vec3 dir = glm::normalize(target - eye);
                Physics::HitResult hit = Physics::LineTrace(eye, eye + dir * 20.0f, BodyType::GroupHitTest, {}, { Player::Instance });
                ProbeLog("  trace " + std::to_string(i) + " target " + V3(target) + " hit " + std::to_string(hit.hasHit) + " entity " + (hit.entity ? hit.entity->ClassName : std::string("-")) + " box " + hit.hitboxName);
                if (!hit.hasHit || hit.entity != npc) continue;
                ++landed;

                GoreHitEvent event;
                event.victim = EntityHandle::FromEntity(npc);
                Hit& h = event.hit;
                h.point = hit.position;
                h.direction = args.find("gore_flip") != args.end() ? -dir : dir;   // test: blow towards the camera
                h.sweepNormal = MathHelper::GetUpVector(Camera::finalizedRotation);
                h.damage = 25.0f;
                h.sourceIndex = SourceIndexForClass(srcClass);
                Gore::SetHitBone(h, hit.hitboxName);
                h.counter = static_cast<uint32_t>(i + 1);
                h.seed = HashSeed(npc->Id, h.counter);
                EcsScheduler::Emit(event);
            }
            ProbeLog("gore_look: npc now at " + V3(npc->Position) + " health " + std::to_string(npc->Health));
            ProbeLog("gore_look: " + std::to_string(landed) + "/" + std::to_string(shots) + " hits landed with " + srcClass);
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
                Gore::SetHitBone(h, hit.hitboxName);
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
    void UpdateLookTest() {}
}

#endif
