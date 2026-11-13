#include "GoreGib.h"

#include "GoreComponent.h"
#include "GoreCutSolver.h"
#include "GoreEvents.h"
#include "GoreSettings.h"
#include "GoreSkeletalMesh.h"
#include "GoreSystem.h"

#include <Ecs/Ecs.h>
#include <Ecs/ISystem.h>
#include <Level.hpp>
#include <Physics.h>
#include <PhysicsConverter.h>
#include <Time.hpp>

#include <algorithm>
#include <deque>
#include <vector>

namespace Gore
{
    namespace
    {
        std::deque<GibRequest> g_queue;
        std::vector<EntityHandle> g_live;   // oldest first
        std::vector<uint32_t> g_queuedFrames;   // frame number each queued request was made (parallel to g_queue)
        uint32_t g_frame = 0;

        // A hitbox for the severed bone built from the merged bounds of its subtree (when the model has none there).
        HitboxData MakeFallbackHitbox(const Profile& profile, const roj::SkinnedModel& model, uint16_t S, const std::string& boneName)
        {
            glm::vec3 mn(1.0e9f), mx(-1.0e9f);
            const glm::mat4 toLocal = model.skeleton.bones[S].invBind;
            for (uint16_t b = S; b < profile.subtreeEnd[S]; ++b)
            {
                const roj::BoneBound& bb = profile.anyBounds[b];
                if (!bb.hasData) continue;
                glm::vec3 c[8];
                BoundCorners(bb, c);
                for (int i = 0; i < 8; ++i)
                {
                    const glm::vec3 l = glm::vec3(toLocal * glm::vec4(c[i], 1.0f));
                    mn = glm::min(mn, l);
                    mx = glm::max(mx, l);
                }
            }

            HitboxData h;
            h.boneName = boneName;
            h.position = (mn + mx) * 0.5f;
            h.size = glm::max(mx - mn, glm::vec3(0.05f));
            h.mass = 4.0f;
            return h;
        }
    }

    GoreGib::GoreGib()
    {
        ClassName = "gore_gib";
        SaveGame = false;
        mesh = new GoreSkeletalMesh(this);
        mesh->CastDetailShadows = false;
        Drawables.push_back(mesh);
    }

    GoreGib::~GoreGib() {}

    void GoreGib::Destroy()
    {
        Entity::Destroy();
        if (mesh) mesh->ClearHitboxes();
    }

    bool GoreGib::Init(Entity* body, GoreSkeletalMesh* bodyMesh, const Profile& profile, uint8_t cutIndex, const glm::vec3& impulse)
    {
        if (body == nullptr || bodyMesh == nullptr || bodyMesh->GetSkinnedModel() == nullptr) return false;
        if (!body->HasComponent<GoreComponent>()) return false;

        auto bodyGore = body->GetComponent<GoreComponent>();
        if (cutIndex >= bodyGore->cutCount) return false;

        const Cut cut = bodyGore->cuts[cutIndex];
        const uint16_t S = cut.boneSkel;
        if (S >= profile.subtreeEnd.size()) return false;
        const uint16_t end = profile.subtreeEnd[S];
        const roj::SkinnedModel& model = *bodyMesh->GetSkinnedModel();

        const SeverableBone* sev = nullptr;
        if (S < profile.severableBySkel.size() && profile.severableBySkel[S] >= 0)
            sev = &profile.severable[profile.severableBySkel[S]];
        const bool rigid = sev == nullptr || sev->gib == SeverableBone::GibKind::Rigid;

        Position = bodyMesh->Position;
        Rotation = bodyMesh->Rotation;
        spawnTime = Time::GameTime;

        mesh->Position = bodyMesh->Position;
        mesh->Rotation = bodyMesh->Rotation;
        mesh->Scale = bodyMesh->Scale;
        mesh->GravityAlignedRotation = bodyMesh->GravityAlignedRotation;
        mesh->Masked = bodyMesh->Masked;
        mesh->DepthPrePath = bodyMesh->DepthPrePath;
        mesh->TexturesLocation = bodyMesh->TexturesLocation;
        mesh->LoadFromFile(bodyMesh->filePath);
        mesh->PreloadAssets();

        // The part's hitboxes only. The root one has no parent; hitboxes whose parent is not in the part are dropped.
        std::vector<HitboxData> kept;
        {
            std::vector<std::pair<uint16_t, HitboxData>> candidates;
            for (const HitboxData& h : mesh->metaData.hitboxes)
            {
                const uint16_t skel = mesh->GetSkeletonIndex(hashed_string{ h.boneName });
                if (skel == INVALID_BONE_IDX || skel < S || skel >= end) continue;
                candidates.emplace_back(skel, h);
            }
            std::sort(candidates.begin(), candidates.end(), [](const auto& a, const auto& b) { return a.first < b.first; });

            if (candidates.empty())
            {
                candidates.emplace_back(S, MakeFallbackHitbox(profile, model, S, model.skeleton.bones[S].name.str()));
            }
            else if (rigid)
            {
                candidates.resize(1);   // one box for the whole part
            }

            for (auto& c : candidates)
            {
                HitboxData h = c.second;
                if (kept.empty())
                {
                    h.parentBone = "";
                }
                else
                {
                    bool parentKept = false;
                    for (const HitboxData& k : kept) parentKept |= k.boneName == h.parentBone;
                    if (!parentKept) continue;
                }
                kept.push_back(h);
            }
        }
        mesh->metaData.hitboxes = kept;

        mesh->CreateHitboxes(this);                       // built in the rest pose
        mesh->PasteAnimationPose(bodyMesh->GetAnimationPose());
        mesh->SetAnimationPaused(true);
        mesh->UpdatePose = false;
        mesh->RagdollPoseFollowStrength = 0.0f;
        mesh->UpdateHitboxes();                           // hitboxes jump to the body's pose
        mesh->StartRagdoll();

        // Velocities: what the body's hitbox moved with, plus the blow. (StartRagdoll's own come from the jump above.)
        for (Body* gibBody : mesh->hitboxBodies)
        {
            const std::string& name = Physics::GetBodyData(gibBody)->hitboxName;
            glm::vec3 v(0.0f);
            if (Body* src = bodyMesh->FindHitboxByName(name))
                v = FromPhysics(src->GetLinearVelocity());
            if (!std::isfinite(v.x + v.y + v.z) || glm::length(v) > 15.0f) v = glm::vec3(0.0f);
            Physics::SetLinearVelocity(gibBody, v + impulse);
            Physics::SetAngularVelocity(gibBody, glm::vec3(0.0f));
        }

        // Gore: the cut (removed side), the wounds that are on the part, and the pose at the moment of the cut.
        if (!HasComponent<GoreComponent>()) AddComponent<GoreComponent>();
        auto gore = GetComponent<GoreComponent>();
        gore->isGib = true;
        gore->cuts[0] = cut;
        gore->cutCount = 1;
        for (int i = 0; i < bodyGore->woundCount; ++i)
        {
            const Wound& w = bodyGore->wounds[i];
            if (w.anchorSkel >= S && w.anchorSkel < end && !w.cavityFailed)
                gore->wounds[gore->woundCount++] = w;
        }
        for (int i = 0; i < bodyGore->splatCount && gore->splatCount < kMaxSplats; ++i)
            gore->splats[gore->splatCount++] = bodyGore->splats[i];
        for (int i = 0; i < bodyGore->decalCount && gore->decalCount < kMaxDecals; ++i)
            gore->decals[gore->decalCount++] = bodyGore->decals[i];
        gore->orderCounter = bodyGore->orderCounter;
        gore->frozenClock = bodyGore->frozenClock;
        gore->pausedUntil = bodyGore->pausedUntil;
        gore->gibSeverPalette = bodyMesh->GetSkinPalette();
        gore->version = 2;

        return true;
    }

    void GoreGib::Update()
    {
        if (mesh == nullptr) return;
        mesh->UpdateHitboxes();
        wasRendered = mesh->WasRended;
    }

    void GoreGib::OnPointDamage(float Damage, vec3 Point, vec3 Direction, string bone, Entity* DamageCauser, Entity* Weapon)
    {
        (void)DamageCauser;
        (void)Weapon;

        GoreHitEvent event;
        event.victim = EntityHandle::FromEntity(this);
        event.hit.point = Point;
        event.hit.direction = glm::length(Direction) > 1.0e-5f ? glm::normalize(Direction) : glm::vec3(0, 0, 1);
        event.hit.damage = Damage;
        SetHitBone(event.hit, bone);
        event.hit.counter = ++hitCounter;
        event.hit.seed = HashSeed(Id, event.hit.counter);
        EcsScheduler::Emit(event);
    }

    namespace GibPool
    {
        void Request(const GibRequest& request)
        {
            g_queue.push_back(request);
            g_queuedFrames.push_back(g_frame);
        }

        void Tick()
        {
            ++g_frame;
            FxNewFrame();

            const GoreSettings& settings = GoreSettings::Get();

            int spawned = 0;
            while (!g_queue.empty() && spawned < settings.maxGibSpawnsPerFrame)
            {
                GibRequest r = g_queue.front();
                const uint32_t madeAt = g_queuedFrames.front();
                g_queue.pop_front();
                g_queuedFrames.erase(g_queuedFrames.begin());

                if (g_frame - madeAt > 3) continue;   // too late: the stump effects alone have to do

                Entity* body = r.body.Resolve();
                if (body == nullptr || Level::Current == nullptr) continue;

                GoreSkeletalMesh* bodyMesh = FindGoreMesh(body);
                if (bodyMesh == nullptr || bodyMesh->GetSkinnedModel() == nullptr) continue;

                const Profile& profile = GetProfile(bodyMesh->filePath, *bodyMesh->GetSkinnedModel());
                if (!profile.valid) continue;

                GoreGib* gib = new GoreGib();
                if (!gib->Init(body, bodyMesh, profile, r.cutIndex, r.impulse))
                {
                    delete gib;
                    continue;
                }

                Level::Current->AddEntity(gib);
                g_live.push_back(EntityHandle::FromEntity(gib));
                ++spawned;
            }

            // Drop the dead handles, then retire the oldest gibs over the budget (off-screen ones first, never a visible
            // one until the budget is exceeded by a margin).
            g_live.erase(std::remove_if(g_live.begin(), g_live.end(), [](EntityHandle& h) { return h.Resolve() == nullptr; }), g_live.end());

            const int over = static_cast<int>(g_live.size()) - settings.maxGibs;
            for (int i = 0; i < over; ++i)
            {
                Entity* e = g_live[i].Resolve();
                auto* gib = dynamic_cast<GoreGib*>(e);
                if (gib != nullptr && (!gib->wasRendered || over > 8))
                    gib->Destroy();
            }
        }
    }
}
