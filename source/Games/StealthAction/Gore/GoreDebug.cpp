#include "GoreDebug.h"

#ifndef DISTRIBUTION

#include "GoreComponent.h"
#include "GoreEvents.h"
#include "GoreFlesh.h"
#include "GoreGib.h"
#include "GoreProfile.h"
#include "GoreSkeletalMesh.h"
#include "GoreSource.h"
#include "GoreSystem.h"
#include "StealthGoreAdapter.h"

#include <AssetRegistry.h>
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
#include <chrono>
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
        static int lastLevel = -1;   // Level::CurrentLevelChangeId: a new level can be allocated where the old one was, so its address says nothing
        static int ticks = 0;
        static Entity* npc = nullptr;
        static bool fired = false;
        static vec3 startEye = vec3(0);   // where the camera was when the npc was placed (shots start there, whatever -gore_cam_up does)
        static bool reloaded = false;     // -gore_reload: another level was opened once, this is the second pass
        static int waited = 0;            // frames spent waiting for the model's gore profile (per level)

        auto& args = EngineMain::MainInstance->Arguments;
        auto look = args.find("gore_look");
        if (look == args.end() || look->second.empty()) return;
        if (Player::Instance == nullptr || Level::Current == nullptr) return;
        const auto reloadArg = args.find("gore_reload");

        // Counts frames since the level changed (a freshly loaded level starts again at 0).
        if (lastLevel != static_cast<int>(Level::CurrentLevelChangeId))
        {
            lastLevel = static_cast<int>(Level::CurrentLevelChangeId);
            ticks = 0;
            npc = nullptr;
            fired = false;
            waited = 0;
        }
        ++ticks;

        Player* player = Player::Instance;

        // Frame times: before the shot (ticks 30-70) and after it (71-130). Logged once at the end.
        {
            static std::chrono::steady_clock::time_point last;
            static double idleSum = 0, idleMax = 0, shotSum = 0, shotMax = 0;
            static int idleCount = 0, shotCount = 0;
            const auto now = std::chrono::steady_clock::now();
            // The frames around the picture stall on the read-back of the image: the window ends when the picture is requested.
            int windowEnd = 130;
            auto waitArg = args.find("gore_wait");
            if (waitArg != args.end() && !waitArg->second.empty()) windowEnd = std::min(windowEnd, std::atoi(waitArg->second[0].c_str()));
            if (ticks > 1)
            {
                const double ms = std::chrono::duration<double, std::milli>(now - last).count();
                if (ticks >= 30 && ticks <= 70) { idleSum += ms; idleMax = std::max(idleMax, ms); ++idleCount; }
                else if (ticks > 71 && ticks <= windowEnd) { shotSum += ms; shotMax = std::max(shotMax, ms); ++shotCount; }
                if (ticks == windowEnd && idleCount > 0 && shotCount > 0)
                    ProbeLog("gore_look: frame ms before shot avg " + std::to_string(idleSum / idleCount) + " max " + std::to_string(idleMax)
                        + ", after shot avg " + std::to_string(shotSum / shotCount) + " max " + std::to_string(shotMax));
            }
            last = now;
        }

        if (ticks == 1) GibPool::SetSuppressed(args.find("gore_nogib") != args.end());   // test: no gib, look at the stump

        // Test (-gore_cam_up H): hold the player H metres above the floor spot he started on, to look down at a cut face.
        {
            static float baseY = 0.0f;
            auto camUp = args.find("gore_cam_up");
            if (camUp != args.end() && !camUp->second.empty() && ticks >= 10)
            {
                if (ticks == 10) baseY = player->Position.y;
                else player->Teleport(vec3(player->Position.x, baseY + static_cast<float>(std::atof(camUp->second[0].c_str())), player->Position.z));
            }

            // -gore_cam_pitch P: feeds P mouse units of vertical movement spread over ticks 11..20.
            auto camPitch = args.find("gore_cam_pitch");
            if (camPitch != args.end() && !camPitch->second.empty() && ticks > 10 && ticks <= 20)
                Input::MouseDelta.y = static_cast<float>(std::atof(camPitch->second[0].c_str())) * 0.1f;
        }

        if (npc == nullptr && ticks == 10)
        {
            const float yaw = Camera::finalizedRotation.y;
            const vec3 forward = MathHelper::GetForwardVector(vec3(0, yaw, 0));
            const vec3 eye = Camera::finalizedPosition;
            startEye = eye;
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
                for (int i = 0; i < g->woundCount; ++i)
                {
                    const Wound& w = g->wounds[i];
                    ProbeLog("  wound " + std::to_string(i) + " center " + V3(w.center) + " radii " + V3(w.radii) + " taper " + std::to_string(w.taper));
                }
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
                ProbeLog("gore_look: profile valid " + std::to_string(pr.valid) + " flesh " + tex(pr.fleshTexture)
                    + " edge " + tex(pr.edgeNoise) + " blood " + tex(pr.blood));
                // The registry frees textures that were not requested during a level load; the profile must hold the registry's current ones.
                auto current = [](const std::string& path, Texture* t) { return path.empty() || AssetRegistry::GetTextureFromFile(path) == t; };
                ProbeLog("gore_look: profile textures are the registry's current ones: flesh " + std::to_string(current(pr.fleshPath, pr.fleshTexture))
                    + " edge " + std::to_string(current(pr.edgeNoisePath, pr.edgeNoise)) + " blood " + std::to_string(current(pr.bloodPath, pr.blood))
                    + " (level change " + std::to_string(static_cast<int>(Level::CurrentLevelChangeId)) + ")");
                // A pointer says nothing about the object (a new texture can be allocated where the freed one was). So the first pass marks the
                // object (Texture::transparent is read by nothing of the gore) and the second pass looks for the mark.
                if (Texture* flesh = pr.fleshTexture; flesh != nullptr && reloadArg != args.end())
                {
                    if (!reloaded) flesh->transparent = true;
                    else ProbeLog(std::string("gore_look: the flesh texture object from before the level change ")
                        + (AssetRegistry::GetTextureFromFile(pr.fleshPath)->transparent ? "survived it" : "was freed by it and requested again"));
                }
                if (pr.flesh) ProbeLog("gore_look: flesh pieces " + std::to_string(pr.flesh->pieces.size()));
            }
            if (args.find("gore_dump_bounds") != args.end())   // authoring aid: bind-space bounds of every bone that carries vertices
            {
                if (GoreSkeletalMesh* gm = FindGoreMesh(npc))
                {
                    const Profile& pr = GetProfile(gm->filePath, *gm->GetSkinnedModel());
                    for (size_t b = 0; b < pr.anyBounds.size(); ++b)
                        if (pr.anyBounds[b].hasData)
                            ProbeLog("  bounds " + gm->GetSkinnedModel()->skeleton.bones[b].name.str() + " min " + V3(pr.anyBounds[b].bbMin) + " max " + V3(pr.anyBounds[b].bbMax));

                    // Torso surface in bind space: along z (front = highest z, back = lowest) on an x/y grid, along x (sides) on a y grid.
                    auto rayAxis = [&](const vec3& origin, int axis, float& lo, float& hi)
                    {
                        lo = 1.0e9f; hi = -1.0e9f;
                        const int a1 = (axis + 1) % 3, a2 = (axis + 2) % 3;
                        for (const roj::SkinnedMesh& m : gm->GetSkinnedModel()->meshes)
                        {
                            for (size_t i = 0; i + 2 < m.indices.size(); i += 3)
                            {
                                const vec3& p0 = m.vertices[m.indices[i]].Position;
                                const vec3& p1 = m.vertices[m.indices[i + 1]].Position;
                                const vec3& p2 = m.vertices[m.indices[i + 2]].Position;
                                // Barycentric test in the plane orthogonal to `axis`.
                                const float d = (p1[a1] - p0[a1]) * (p2[a2] - p0[a2]) - (p2[a1] - p0[a1]) * (p1[a2] - p0[a2]);
                                if (std::abs(d) < 1.0e-9f) continue;
                                const float u = ((origin[a1] - p0[a1]) * (p2[a2] - p0[a2]) - (p2[a1] - p0[a1]) * (origin[a2] - p0[a2])) / d;
                                const float v = ((p1[a1] - p0[a1]) * (origin[a2] - p0[a2]) - (origin[a1] - p0[a1]) * (p1[a2] - p0[a2])) / d;
                                if (u < 0.0f || v < 0.0f || u + v > 1.0f) continue;
                                const float t = p0[axis] + u * (p1[axis] - p0[axis]) + v * (p2[axis] - p0[axis]);
                                if (axis == 0 && std::abs(t) > 0.3f) continue;   // sides: ignore arms
                                lo = std::min(lo, t);
                                hi = std::max(hi, t);
                            }
                        }
                    };
                    for (float y = 0.9f; y <= 1.7f; y += 0.1f)
                    {
                        std::string row = "  surface y " + std::to_string(y) + " | x:";
                        float lo, hi;
                        rayAxis(vec3(0, y, 0), 0, lo, hi);
                        row += " " + std::to_string(lo) + ".." + std::to_string(hi) + " | z(back..front) at x";
                        for (float x = -0.2f; x <= 0.201f; x += 0.1f)
                        {
                            rayAxis(vec3(x, y, 0), 2, lo, hi);
                            row += " " + std::to_string(x).substr(0, 5) + ":" + std::to_string(lo).substr(0, 6) + ".." + std::to_string(hi).substr(0, 6);
                        }
                        ProbeLog(row);
                    }
                }
            }
            auto file = args.find("gore_file");
            Screenshot::Request((file != args.end() && !file->second.empty() ? file->second[0] : std::string("gore_look")) + (reloaded ? "_L2" : ""));
        }

        // -gore_anim <name>: the shots are over, the body now plays that animation (looped); the picture comes at -gore_wait, so the
        // wounds can be looked at on a body that is not in its bind pose.
        {
            auto anim = args.find("gore_anim");
            if (fired && npc != nullptr && anim != args.end() && !anim->second.empty() && ticks == 75)
                if (GoreSkeletalMesh* gm = FindGoreMesh(npc)) gm->PlayAnimation(anim->second[0], true, 0.0f);
        }

        // -gore_reload <map>: after the picture another map is opened (a level change: at the end of its load the engine frees every asset
        // that was not requested while it loaded, and the gore textures are first requested at the first hit) and the whole test runs a
        // second time there. The second picture is <gore_file>_L2. Reopening the same map would not do: that is no level change.
        const bool reloadPending = reloadArg != args.end() && !reloadArg->second.empty() && !reloaded;
        if (fired && reloadPending && ticks == shotTick + 10)
        {
            reloaded = true;
            Level::LoadLevelFromFile("GameData/maps/" + reloadArg->second[0]);
            ProbeLog("gore_look: changing level to " + reloadArg->second[0]);
        }

        if (fired && !reloadPending && ticks == shotTick + 30)
        {
            SDL_Event quit;   // the test run ends by itself (stdout is flushed on a normal exit)
            SDL_zero(quit);
            quit.type = SDL_QUIT;
            SDL_PushEvent(&quit);
        }

        // Fire once the model has its gore profile (a model streams in tiers; the profile exists when the geometry is partitioned).
        bool profileReady = true;
        if (npc != nullptr && !fired && ticks > 70)
        {
            GoreSkeletalMesh* gm = FindGoreMesh(npc);
            profileReady = gm != nullptr && gm->GetSkinnedModel() != nullptr && GetProfile(gm->filePath, *gm->GetSkinnedModel()).valid;
            if (!profileReady && waited < 3000) { ++waited; ticks = 71; }   // keep waiting (bounded): the tick counter holds at 71
            else profileReady = true;
        }

        if (npc != nullptr && !fired && ticks > 70 && profileReady)
        {
            fired = true;
            ticks = 71;
            std::string srcClass = "weapon_pistol";
            auto src = args.find("gore_src");
            if (src != args.end() && !src->second.empty()) srcClass = src->second[0];
            int shots = 7;
            auto sh = args.find("gore_shots");
            if (sh != args.end() && !sh->second.empty()) shots = std::atoi(sh->second[0].c_str());

            const vec3 eye = glm::length(startEye) > 0.0f ? startEye : Camera::finalizedPosition;
            const vec3 center = npc->Position + vec3(0, 0.3f, 0);
            const vec3 right = MathHelper::GetRightVector(vec3(0, Camera::finalizedRotation.y, 0));
            int landed = 0;
            Entity* weaponEntity = nullptr;   // -gore_kill: stands in for the weapon (its class picks the gore source)
            for (int i = 0; i < shots; ++i)
            {
                const float u = shots > 1 ? (static_cast<float>(i) / (shots - 1) - 0.5f) : 0.0f;
                vec3 target = center + right * (u * 0.5f) + vec3(0, 0.45f * std::sin(i * 2.1f), 0);
                auto aim = args.find("gore_aim");
                if (aim != args.end() && !aim->second.empty())
                {
                    if (GoreSkeletalMesh* gm = FindGoreMesh(npc))
                        target = MathHelper::DecomposeMatrix(gm->GetBoneMatrixWorld(aim->second[0])).Position + right * (u * 0.02f);
                    auto dy = args.find("gore_aim_dy");   // shift of the aim point in world metres (the bone origin is rarely the spot to hit)
                    if (dy != args.end() && !dy->second.empty()) target.y += static_cast<float>(std::atof(dy->second[0].c_str()));
                    auto dx = args.find("gore_aim_dx");
                    if (dx != args.end() && !dx->second.empty()) target += right * static_cast<float>(std::atof(dx->second[0].c_str()));
                }
                vec3 shotEye = eye;
                const vec3 dir = glm::normalize(target - shotEye);
                Physics::HitResult hit = Physics::LineTrace(shotEye, shotEye + dir * 20.0f, BodyType::GroupHitTest, {}, { Player::Instance });
                ProbeLog("  trace " + std::to_string(i) + " target " + V3(target) + " hit " + std::to_string(hit.hasHit) + " entity " + (hit.entity ? hit.entity->ClassName : std::string("-")) + " box " + hit.hitboxName);
                if (!hit.hasHit || hit.entity != npc) continue;
                ++landed;

                GoreHitEvent event;
                event.victim = EntityHandle::FromEntity(npc);
                Hit& h = event.hit;
                h.point = hit.position;
                h.direction = args.find("gore_flip") != args.end() ? -dir : dir;   // test: blow towards the camera
                h.sweepNormal = MathHelper::GetUpVector(vec3(0, Camera::finalizedRotation.y, 0));
                // Test: tilt the slash plane. roll = about the blow direction (a diagonal cut across the body), pitch = about the
                // camera's right axis (a rising or falling cut).
                auto roll = args.find("gore_slash_roll");
                if (roll != args.end() && !roll->second.empty())
                    h.sweepNormal = glm::vec3(glm::rotate(glm::radians(static_cast<float>(std::atof(roll->second[0].c_str()))), glm::normalize(h.direction)) * glm::vec4(h.sweepNormal, 0.0f));
                auto pitch = args.find("gore_slash_pitch");
                if (pitch != args.end() && !pitch->second.empty())
                    h.sweepNormal = glm::vec3(glm::rotate(glm::radians(static_cast<float>(std::atof(pitch->second[0].c_str()))), right) * glm::vec4(h.sweepNormal, 0.0f));
                h.damage = 25.0f;
                auto dmg = args.find("gore_damage");
                if (dmg != args.end() && !dmg->second.empty()) h.damage = static_cast<float>(std::atof(dmg->second[0].c_str()));
                h.lethal = args.find("gore_lethal") != args.end();   // test: this hit is the killing blow
                h.alive = args.find("gore_alive") != args.end();   // test: the victim counts as alive (parts stay on)
                h.sourceIndex = SourceIndexForClass(srcClass);
                Gore::SetHitBone(h, hit.hitboxName);
                h.counter = static_cast<uint32_t>(i + 1);
                h.seed = HashSeed(npc->Id, h.counter);

                // -gore_kill <health>: the real game path. The enemy gets that much health and takes the shots as damage, so it
                // dies and ragdolls like in a game; the hit flags (alive, lethal) come from the damage code, not from the switches.
                auto kill = args.find("gore_kill");
                if (kill != args.end() && !kill->second.empty())
                {
                    if (i == 0)
                    {
                        npc->MaxHealth = static_cast<float>(std::atof(kill->second[0].c_str()));
                        npc->Health = npc->MaxHealth;
                    }
                    if (weaponEntity == nullptr)
                    {
                        weaponEntity = new Entity();
                        weaponEntity->ClassName = srcClass;
                        Level::Current->AddEntity(weaponEntity);
                    }
                    StealthGore::SetNextMeleeSweep(npc, h.sweepNormal);
                    npc->OnPointDamage(h.damage, h.point, h.direction, hit.hitboxName, nullptr, weaponEntity);
                    continue;
                }
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
