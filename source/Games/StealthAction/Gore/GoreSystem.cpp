#include "GoreSystem.h"
#include "GoreEvents.h"
#include "GoreProfile.h"
#include "GoreSkeletalMesh.h"
#include "GoreSource.h"

#include <Ecs/Ecs.h>
#include <Ecs/ISystem.h>
#include <Entity.h>
#include <Physics.h>
#include <PhysicsConverter.h>

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <vector>

namespace Gore
{
    namespace
    {
        // Rotation taking +z to the (normalised) direction d.
        glm::quat RotateZTo(const glm::vec3& d)
        {
            const float c = d.z;
            if (c > 0.99999f) return glm::quat(1, 0, 0, 0);
            if (c < -0.99999f) return glm::quat(0, 1, 0, 0);   // 180 degrees about x
            const glm::vec3 axis = glm::cross(glm::vec3(0, 0, 1), d);
            return glm::normalize(glm::quat(1.0f + c, axis.x, axis.y, axis.z));
        }

        float MeanRadius(const glm::vec3& r) { return (r.x + r.y + r.z) / 3.0f; }

        struct Anchor
        {
            int skin = -1;
            uint16_t skel = 0;
            std::string bone;
        };

        // Resolves the bone a hit belongs to: the named hitbox bone, or the hitbox closest to the point.
        bool ResolveAnchor(GoreSkeletalMesh* mesh, const Hit& hit, Anchor& out)
        {
            if (hit.hitBone[0] != 0)
            {
                const hashed_string name{ std::string(hit.hitBone) };
                const int skin = mesh->GetSkinIndex(name);
                if (skin >= 0)
                {
                    out.skin = skin;
                    out.skel = mesh->GetSkeletonIndex(name);
                    out.bone = hit.hitBone;
                    return out.skel != INVALID_BONE_IDX;
                }
            }

            Body* best = nullptr;
            float bestDist = FLT_MAX;
            for (Body* body : mesh->hitboxBodies)
            {
                const float d = glm::distance(FromPhysics(body->GetPosition()), hit.point);
                if (d < bestDist)
                {
                    bestDist = d;
                    best = body;
                }
            }
            if (best == nullptr) return false;

            const std::string& bone = Physics::GetBodyData(best)->hitboxName;
            const hashed_string name{ bone };
            out.skin = mesh->GetSkinIndex(name);
            out.skel = mesh->GetSkeletonIndex(name);
            out.bone = bone;
            return out.skin >= 0 && out.skel != INVALID_BONE_IDX;
        }

        // Far intersection of the shot ray with the OBB of a hitbox (world point). False if there is none.
        bool HitboxExitPoint(GoreSkeletalMesh* mesh, const std::string& bone, const Hit& hit, glm::vec3& exitPoint)
        {
            Body* body = mesh->FindHitboxByName(bone);
            if (body == nullptr) return false;

            const HitboxData* data = nullptr;
            for (const HitboxData& h : mesh->metaData.hitboxes)
                if (h.boneName == bone) { data = &h; break; }
            if (data == nullptr) return false;

            // Shape: translate(position) * rotate(rotation) * box(size / 2), in the body frame (Physics::CreateHitBoxBody).
            const glm::quat bodyRot = FromPhysics(body->GetRotation());
            const glm::vec3 bodyPos = FromPhysics(body->GetPosition());
            const glm::quat shapeRot = bodyRot * MathHelper::GetRotationQuaternion(data->rotation);
            const glm::vec3 center = bodyPos + bodyRot * data->position;
            const glm::vec3 half = data->size * 0.5f;

            const glm::quat inv = glm::inverse(shapeRot);
            const glm::vec3 o = inv * (hit.point - center);
            const glm::vec3 d = inv * hit.direction;

            float tMin = -FLT_MAX, tMax = FLT_MAX;
            for (int k = 0; k < 3; ++k)
            {
                if (std::abs(d[k]) < 1.0e-6f)
                {
                    if (std::abs(o[k]) > half[k] + 0.01f) return false;
                    continue;
                }
                float t0 = (-half[k] - o[k]) / d[k];
                float t1 = (half[k] - o[k]) / d[k];
                if (t0 > t1) std::swap(t0, t1);
                tMin = std::max(tMin, t0);
                tMax = std::min(tMax, t1);
            }
            if (tMax < tMin || tMax < 0.01f) return false;

            exitPoint = hit.point + hit.direction * tMax;
            return true;
        }

        void AddSplat(GoreComponent& g, const glm::vec3& center, float radius, float strength)
        {
            Splat s;
            s.center = center;
            s.radius = radius;
            s.strength = strength;
            s.order = g.orderCounter++;

            if (g.splatCount < kMaxSplats)
            {
                g.splats[g.splatCount++] = s;
                return;
            }

            // Full: merge into a close splat, else replace the oldest.
            int nearest = 0;
            float nearestDist = FLT_MAX;
            int oldest = 0;
            for (int i = 0; i < g.splatCount; ++i)
            {
                const float d = glm::distance(g.splats[i].center, center);
                if (d < nearestDist) { nearestDist = d; nearest = i; }
                if (g.splats[i].order < g.splats[oldest].order) oldest = i;
            }

            if (nearestDist < g.splats[nearest].radius + radius)
            {
                Splat& t = g.splats[nearest];
                const float a = t.radius * t.radius, b = radius * radius;
                t.center = (t.center * a + center * b) / (a + b);
                t.radius = std::max(t.radius, radius);
                t.strength = std::max(t.strength, strength);
            }
            else
            {
                g.splats[oldest] = s;
            }
        }

        void AddWound(GoreComponent& g, Wound w)
        {
            w.order = g.orderCounter++;

            if (g.woundCount < kMaxWounds)
            {
                g.wounds[g.woundCount++] = w;
                return;
            }

            int nearest = 0;
            float nearestDist = FLT_MAX;
            int oldest = 0;
            for (int i = 0; i < g.woundCount; ++i)
            {
                const float d = glm::distance(g.wounds[i].center, w.center);
                if (g.wounds[i].anchorSkin == w.anchorSkin && d < nearestDist) { nearestDist = d; nearest = i; }
                if (g.wounds[i].order < g.wounds[oldest].order) oldest = i;
            }

            if (nearestDist < FLT_MAX && nearestDist < MeanRadius(g.wounds[nearest].radii) + MeanRadius(w.radii))
            {
                // Merge: volume-weighted centre, max radii, keep the older rotation.
                Wound& t = g.wounds[nearest];
                const float v1 = t.radii.x * t.radii.y * t.radii.z;
                const float v2 = w.radii.x * w.radii.y * w.radii.z;
                t.center = (t.center * v1 + w.center * v2) / (v1 + v2);
                t.radii = glm::max(t.radii, w.radii);
                t.reach = std::max(t.reach, w.reach);
            }
            else
            {
                g.wounds[oldest] = w;
            }
        }

        struct BindFrame
        {
            glm::mat4 toBind;
        };

        Wound MakeWound(const Anchor& a, float reach, WoundKind kind, const glm::vec3& center, const glm::quat& rot,
                        const glm::vec3& radii, float roughness, float rim, Rng& rng)
        {
            Wound w;
            w.center = center;
            w.rotation = rot;
            w.radii = radii;
            w.anchorSkin = static_cast<int16_t>(a.skin);
            w.anchorSkel = a.skel;
            w.kind = kind;
            w.roughness = roughness;
            w.rimWidth = rim;
            w.seed = rng.Next01();
            w.reach = reach;
            return w;
        }

        // A shot wound is long along the shot so it always reaches the real mesh surface (the hit point is on the hitbox
        // box, which is not exactly the skin). The centre sits slightly inside.
        Wound MakeShotWound(const Anchor& a, float reach, WoundKind kind, const glm::vec3& surfacePoint, const glm::vec3& dir,
                            float r, float zRadius, float roughness, float rim, Rng& rng)
        {
            const glm::vec3 center = surfacePoint + dir * (zRadius * 0.35f);
            return MakeWound(a, reach, kind, center, RotateZTo(dir), glm::vec3(r, r, zRadius), roughness, rim, rng);
        }

        struct Cluster
        {
            int skin = -1;
            uint16_t skel = 0;
            std::string bone;
            glm::vec3 center = glm::vec3(0);
            glm::vec3 dirSum = glm::vec3(0);
            int count = 0;
            float damage = 0.0f;
            uint32_t seed = 0;
        };
    }

    GoreSkeletalMesh* FindGoreMesh(Entity* entity)
    {
        if (entity == nullptr) return nullptr;
        for (IDrawMesh* drawable : entity->Drawables)
            if (auto* gm = dynamic_cast<GoreSkeletalMesh*>(drawable))
                return gm;
        return nullptr;
    }

    void MergeIntoLastPending(GoreComponent& gore, const Hit& hit)
    {
        if (gore.pendingCount == 0) return;
        gore.pending[gore.pendingCount - 1].damage += hit.damage;
        gore.pending[gore.pendingCount - 1].lethal |= hit.lethal;
    }

    void ProcessPendingHits(Entity* self, GoreComponent& gore)
    {
        GoreSkeletalMesh* mesh = FindGoreMesh(self);
        if (mesh == nullptr || mesh->GetSkinnedModel() == nullptr)
        {
            gore.pendingCount = 0;
            return;
        }

        const Profile& profile = GetProfile(mesh->filePath, *mesh->GetSkinnedModel());
        if (!profile.valid)
        {
            // Still loading: keep the hits; no profile for this model: drop them.
            if (mesh->GetSkinnedModel()->meshes.empty()) return;
            gore.pendingCount = 0;
            return;
        }

        const std::vector<mat4>& palette = mesh->GetSkinPalette();
        if (palette.empty()) return;   // model still loading, keep the hits for the next frame

        const mat4 World = mesh->GetWorldMatrix();

        std::stable_sort(gore.pending.begin(), gore.pending.begin() + gore.pendingCount,
                         [](const Hit& a, const Hit& b) { return a.counter < b.counter; });

        std::vector<Cluster> clusters;   // pellet clusters of this batch (a handful at most)

        for (int hi = 0; hi < gore.pendingCount; ++hi)
        {
            const Hit& hit = gore.pending[hi];
            const Source& source = GetSource(hit.sourceIndex);

            Anchor anchor;
            if (!ResolveAnchor(mesh, hit, anchor)) continue;
            if (anchor.skin >= static_cast<int>(palette.size())) continue;

            const glm::mat4 toBind = glm::inverse(World * palette[anchor.skin]);
            const glm::vec3 pBind = glm::vec3(toBind * glm::vec4(hit.point, 1.0f));
            glm::vec3 dBind = glm::vec3(toBind * glm::vec4(hit.direction, 0.0f));
            dBind = glm::length(dBind) > 1.0e-5f ? glm::normalize(dBind) : glm::vec3(0, 0, 1);

            Rng rng(hit.seed);
            const float reach = 2.0f * MaxExtent(profile, anchor.skel);
            const float damage = std::max(hit.damage, 1.0f);

            switch (source.type)
            {
            case SourceType::Bullet:
            case SourceType::HeavyBullet:
            {
                const bool heavy = source.type == SourceType::HeavyBullet;
                float r = source.woundRadius * std::sqrt(damage / source.refDamage) * rng.Range(0.8f, 1.2f);
                if (heavy) r *= 1.3f;
                const float rz = std::max(r * 1.6f, 0.05f);

                AddWound(gore, MakeShotWound(anchor, reach, WoundKind::Entry, pBind, dBind, r, rz, 0.3f, 0.5f, rng));
                AddSplat(gore, pBind, 2.5f * r, 1.0f);

                if (source.exit || heavy)
                {
                    glm::vec3 exitWorld;
                    if (HitboxExitPoint(mesh, anchor.bone, hit, exitWorld))
                    {
                        const float er = r * (heavy ? 2.2f : 1.6f);
                        const float erz = std::max(er * 1.6f, 0.05f);
                        const glm::vec3 exitBind = glm::vec3(toBind * glm::vec4(exitWorld, 1.0f));
                        // Mirror of the entry: the body is behind the exit point (towards -dir).
                        const glm::vec3 center = exitBind - dBind * (erz * 0.35f);
                        AddWound(gore, MakeWound(anchor, reach, WoundKind::Exit, center, RotateZTo(dBind),
                                                 glm::vec3(er, er, erz), 0.45f, 0.5f, rng));
                        AddSplat(gore, exitBind, 2.5f * er, 1.0f);
                    }
                }
                break;
            }

            case SourceType::Pellet:
            {
                // Clustered below, once all pellets of the batch are known.
                Cluster* target = nullptr;
                for (Cluster& c : clusters)
                {
                    if (c.skin == anchor.skin && glm::distance(c.center, pBind) < source.clusterRadius)
                    {
                        target = &c;
                        break;
                    }
                }
                if (target == nullptr)
                {
                    clusters.emplace_back();
                    target = &clusters.back();
                    target->skin = anchor.skin;
                    target->skel = anchor.skel;
                    target->bone = anchor.bone;
                    target->seed = hit.seed;
                }
                target->center = (target->center * static_cast<float>(target->count) + pBind) / static_cast<float>(target->count + 1);
                target->dirSum += dBind;
                target->count++;
                target->damage += damage;
                break;
            }

            case SourceType::Slash:
            {
                glm::vec3 nB = glm::vec3(toBind * glm::vec4(hit.sweepNormal, 0.0f));
                if (glm::length(nB) < 1.0e-5f)
                    nB = glm::cross(dBind, std::abs(dBind.y) < 0.99f ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0));
                nB = glm::normalize(nB);

                const glm::vec3 z = dBind;
                glm::vec3 x = glm::cross(nB, z);
                if (glm::length(x) < 1.0e-4f)
                    x = glm::cross(z, std::abs(z.y) < 0.99f ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0));
                x = glm::normalize(x);
                const glm::vec3 y = glm::cross(z, x);

                const float depth = std::max(source.woundRadius, 0.03f);
                const glm::vec3 radii(source.gashLength * 0.5f, source.woundRadius * 0.35f, depth);
                const glm::quat rot = glm::normalize(glm::quat_cast(glm::mat3(x, y, z)));
                const glm::vec3 center = pBind + dBind * (depth * 0.35f);

                AddWound(gore, MakeWound(anchor, reach, WoundKind::Gash, center, rot, radii, 0.25f, 0.4f, rng));
                AddSplat(gore, pBind, 2.5f * source.woundRadius + 0.03f, 1.0f);
                break;
            }

            case SourceType::Blunt:
            {
                AddSplat(gore, pBind, std::max(0.08f * hit.damage / source.refDamage, 0.03f), 0.8f);
                break;
            }

            case SourceType::Blast:
            {
                const float r = source.woundRadius * glm::clamp(hit.damage / source.refDamage, 0.5f, 2.0f);
                const float rz = std::max(r * 0.7f, 0.04f);
                AddWound(gore, MakeShotWound(anchor, reach, WoundKind::Crater, pBind, dBind, r, rz, 0.5f, 0.6f, rng));
                AddSplat(gore, pBind, 2.5f * r, 1.0f);
                break;
            }
            }
        }

        // One crater per pellet cluster.
        for (const Cluster& c : clusters)
        {
            // The pellet source is the one of the first hit of the batch with this skin (all pellets share a weapon).
            const Source* source = nullptr;
            for (int hi = 0; hi < gore.pendingCount; ++hi)
                if (GetSource(gore.pending[hi].sourceIndex).type == SourceType::Pellet) { source = &GetSource(gore.pending[hi].sourceIndex); break; }
            if (source == nullptr) continue;

            Anchor anchor;
            anchor.skin = c.skin;
            anchor.skel = c.skel;
            anchor.bone = c.bone;

            Rng rng(c.seed);
            const glm::vec3 dir = glm::length(c.dirSum) > 1.0e-5f ? glm::normalize(c.dirSum) : glm::vec3(0, 0, 1);
            const float r = source->woundRadius * (1.5f + 0.6f * std::sqrt(static_cast<float>(c.count)));
            const float rz = std::max(r * 0.8f, 0.035f);
            const float reach = 2.0f * MaxExtent(profile, anchor.skel);

            AddWound(gore, MakeShotWound(anchor, reach, WoundKind::Crater, c.center, dir, r, rz, 0.4f, 0.5f, rng));
            AddSplat(gore, c.center, 2.5f * r, 1.0f);
        }

        gore.pendingCount = 0;
        gore.version++;
    }

    void PushRenderStateIfChanged(Entity* self, GoreComponent& gore)
    {
        GoreSkeletalMesh* mesh = FindGoreMesh(self);
        if (mesh == nullptr || mesh->GetSkinnedModel() == nullptr) return;
        if (mesh->GetGoreVersion() == gore.version) return;

        const Profile& profile = GetProfile(mesh->filePath, *mesh->GetSkinnedModel());
        if (!profile.valid && mesh->GetSkinnedModel()->meshes.empty()) return;   // geometry not loaded yet: retry

        RenderState state;
        state.version = gore.version;
        state.wounds = gore.wounds;
        state.woundCount = gore.woundCount;
        state.cuts = gore.cuts;
        state.cutCount = gore.cutCount;
        state.splats = gore.splats;
        state.splatCount = gore.splatCount;
        state.frozenClock = gore.frozenClock;
        state.pausedUntil = gore.pausedUntil;
        state.profile = &profile;

        mesh->SetGoreState(state);
    }
}

EVENT_HANDLER(GoreHitHandler, GoreHitEvent)
{
    Entity* victim = e.victim.Resolve();
    if (!victim) return;

    Gore::GoreSkeletalMesh* mesh = Gore::FindGoreMesh(victim);
    if (!mesh || !mesh->GetSkinnedModel()) return;

    if (!victim->HasComponent<Gore::GoreComponent>())
        victim->AddComponent<Gore::GoreComponent>();

    auto gore = victim->GetComponent<Gore::GoreComponent>();
    if (gore->pendingCount < Gore::kMaxPendingHits)
        gore->pending[gore->pendingCount++] = e.hit;
    else
        Gore::MergeIntoLastPending(*gore, e.hit);
}

TICK_ENTITY(GoreProcessHitsSystem, PostLateUpdate, Gore::GoreComponent, self, gore)
{
    if (gore.pendingCount > 0)
        Gore::ProcessPendingHits(self, gore);

    Gore::PushRenderStateIfChanged(self, gore);
}
REGISTER_TICK(GoreProcessHitsSystem)
