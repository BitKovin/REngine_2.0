#include "GoreSystem.h"
#include "GoreCutSolver.h"
#include "GoreEvents.h"
#include "GoreGib.h"
#include "GoreProfile.h"
#include "GoreSkeletalMesh.h"
#include "GoreSource.h"
#include "GoreWoundMesh.h"

#include <Ecs/Ecs.h>
#include <Ecs/ISystem.h>
#include <Entity.h>
#include <Physics.h>
#include <PhysicsConverter.h>
#include <Time.hpp>

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

        // Half extents of a wound ellipsoid along the axes of another frame (frameInv = inverse rotation of that frame).
        glm::vec3 HalfExtentsIn(const glm::mat3& frameInv, const Wound& w)
        {
            const glm::mat3 m = frameInv * glm::mat3_cast(w.rotation);   // column j = wound axis j in the frame
            glm::vec3 e(0.0f);
            for (int i = 0; i < 3; ++i)
            {
                float sum = 0.0f;
                for (int j = 0; j < 3; ++j) sum += m[j][i] * m[j][i] * w.radii[j] * w.radii[j];
                e[i] = std::sqrt(sum);
            }
            return e;
        }

        bool CanMerge(const Wound& a, const Wound& b)
        {
            if (a.anchorSkin != b.anchorSkin) return false;
            if (a.kind == WoundKind::Gash || b.kind == WoundKind::Gash) return false;   // a gash is long and thin: it stays its own wound
            return (a.kind == WoundKind::Exit) == (b.kind == WoundKind::Exit);          // exit wounds dig the other way
        }

        // Damage stacks: every hit that lands in or on an existing hole eats more of the body, the hole grows by the area of
        // the new one (capped, so a body is not erased by a machine gun).
        void GrowWound(Wound& w, float addedMeanRadius, float weight)
        {
            const float mean = std::max(MeanRadius(w.radii), 1.0e-4f);
            const float k = std::sqrt(1.0f + 1.6f * weight * (addedMeanRadius / mean) * (addedMeanRadius / mean));
            w.radii.x = std::min(w.radii.x * k, 0.4f);
            w.radii.y = std::min(w.radii.y * k, 0.4f);
            w.radii.z = std::min(w.radii.z * (1.0f + (k - 1.0f) * 0.7f), 0.25f);
            w.cavity.reset();
            w.cavityTicket.reset();
            w.cavityFailed = false;
        }

        // Overlapping holes become one: a bigger hole swallows the smaller ones (a new bigger hole replaces the older one,
        // a bullet into an existing hole changes nothing), otherwise the two grow into one hole that covers both.
        void AddWound(GoreComponent& g, Wound w)
        {
            w.order = g.orderCounter++;

            // Holes close to each other join up (well beyond touching: the flesh between them is gone too). When the table is
            // full, the nearest compatible hole takes the new one whatever the distance, so the budget makes holes bigger
            // instead of dropping old ones.
            const bool full = g.woundCount >= kMaxWounds;
            int target = -1;
            float best = FLT_MAX;
            for (int i = 0; i < g.woundCount; ++i)
            {
                if (!CanMerge(g.wounds[i], w)) continue;
                const float d = glm::distance(g.wounds[i].center, w.center);
                if ((full || d < 1.8f * (MeanRadius(g.wounds[i].radii) + MeanRadius(w.radii))) && d < best) { best = d; target = i; }
            }

            if (target >= 0)
            {
                Wound& t = g.wounds[target];
                const float rt = MeanRadius(t.radii), rn = MeanRadius(w.radii);

                if (best + rt <= rn)   // the new hole swallows the old one
                {
                    GrowWound(w, rt, 0.6f);
                    g.wounds[target] = w;
                    return;
                }
                if (best + rn <= rt)   // the new hit lands inside the old hole
                {
                    t.order = w.order;
                    GrowWound(t, rn, 1.0f);
                    return;
                }

                // Union in the frame of the bigger wound; depth stays the larger of the two.
                const bool oldIsBig = t.radii.x * t.radii.y * t.radii.z >= w.radii.x * w.radii.y * w.radii.z;
                Wound merged = oldIsBig ? t : w;
                const Wound& other = oldIsBig ? w : t;
                const glm::mat3 R = glm::mat3_cast(merged.rotation);
                const glm::mat3 Rinv = glm::transpose(R);
                const glm::vec3 ea = HalfExtentsIn(Rinv, merged), eb = HalfExtentsIn(Rinv, other);
                const glm::vec3 cb = Rinv * (other.center - merged.center);
                const glm::vec3 lo = glm::min(-ea, cb - eb), hi = glm::max(ea, cb + eb);
                glm::vec3 radii = (hi - lo) * 0.5f;
                radii = glm::min(radii, glm::max(merged.radii, other.radii) * 4.0f);
                radii.z = std::max(merged.radii.z, other.radii.z);
                merged.center += R * ((lo + hi) * 0.5f);
                merged.radii = radii;
                merged.roughness = std::max(merged.roughness, other.roughness);
                merged.rimWidth = std::max(merged.rimWidth, other.rimWidth);
                merged.reach = std::max(merged.reach, other.reach);
                merged.order = w.order;
                GrowWound(merged, std::min(rt, rn), 1.0f);
                merged.cavity.reset();   // the shape changed: rebuilt after the hits are processed
                merged.cavityTicket.reset();
                merged.cavityFailed = false;
                t = merged;
                return;
            }

            if (g.woundCount < kMaxWounds)
            {
                g.wounds[g.woundCount++] = w;
                return;
            }

            int oldest = 0;
            for (int i = 1; i < g.woundCount; ++i)
                if (g.wounds[i].order < g.wounds[oldest].order) oldest = i;
            g.wounds[oldest] = w;
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


        // Sever threshold (accumulated damage * severPower within the decay window). Body rules scale it in phase 4.
        constexpr float kSeverThreshold = 60.0f;
        constexpr double kSeverDecaySeconds = 3.0;

        bool IsSeverType(SourceType t)
        {
            return t == SourceType::Slash || t == SourceType::HeavyBullet || t == SourceType::Pellet || t == SourceType::Blast;
        }

        bool PointInBound(const roj::BoneBound& b, const glm::vec3& p, float margin)
        {
            return b.hasData && p.x >= b.bbMin.x - margin && p.x <= b.bbMax.x + margin
                && p.y >= b.bbMin.y - margin && p.y <= b.bbMax.y + margin
                && p.z >= b.bbMin.z - margin && p.z <= b.bbMax.z + margin;
        }

        bool SubtreeHoldsCut(const GoreComponent& gore, const Profile& profile, uint16_t bone)
        {
            for (int i = 0; i < gore.cutCount; ++i)
                if (gore.cuts[i].boneSkel >= bone && gore.cuts[i].boneSkel < profile.subtreeEnd[bone])
                    return true;
            return false;
        }

        // Cuts a severable bone at the plane nearest to `point` (leaning towards `normal`), hides its hitboxes, spawns the gib.
        bool MakeCut(Entity* self, GoreSkeletalMesh* mesh, GoreComponent& gore, const Profile& profile, const Source& source,
                     const Hit& hit, int candidate, const glm::vec3& point, const glm::vec3& normal, const glm::mat4& world,
                     const std::vector<glm::mat4>& palette, Rng& rng)
        {
            CutRequest request;
            request.severableIndex = static_cast<uint16_t>(candidate);
            request.worldPoint = point;
            request.worldNormal = normal;

            const CutResult result = SolveCut(profile, *mesh->GetSkinnedModel(), world, palette, request, rng);
            if (!result.ok) return false;

            Cut cut = result.cut;
            cut.severedAt = Time::GameTime;
            cut.phase = CutPhase::Severed;

            const uint8_t cutIndex = gore.cutCount;
            gore.cuts[gore.cutCount++] = cut;
            gore.severDamage[candidate] = 0.0f;

            // The part's hitboxes leave the physics world (the gib gets its own).
            std::vector<hashed_string> names;
            for (const HitboxData& h : mesh->metaData.hitboxes)
            {
                const uint16_t skel = mesh->GetSkeletonIndex(hashed_string{ h.boneName });
                if (skel != INVALID_BONE_IDX && skel >= cut.boneSkel && skel < profile.subtreeEnd[cut.boneSkel])
                    names.push_back(hashed_string{ h.boneName });
            }
            if (!names.empty())
                mesh->SetHitboxesActive(names, false);

            // Blood soaks the skin around the stump.
            AddSplat(gore, cut.center, 0.14f + cut.boneRadius * 3.0f, 1.0f);

            const glm::vec3 up(0, 1, 0);
            const float strength = glm::clamp(hit.damage / std::max(source.refDamage, 1.0f), 0.5f, 2.5f);
            GibRequest gib;
            gib.body = EntityHandle::FromEntity(self);
            gib.cutIndex = cutIndex;
            gib.impulse = hit.direction * (2.0f + 1.5f * strength) + up * 1.2f;
            GibPool::Request(gib);

            GorePartEvent part;
            part.victim = EntityHandle::FromEntity(self);
            part.boneSkel = cut.boneSkel;
            part.lost = true;
            EcsScheduler::Emit(part);
            return true;
        }

        // Severs a part when the blow is strong enough. Returns true when a cut was made (no wound is added for the hit).
        bool TrySever(Entity* self, GoreSkeletalMesh* mesh, GoreComponent& gore, const Profile& profile, const Source& source,
                      const Hit& hit, const Anchor& anchor, const glm::vec3& pBind, const glm::mat4& world,
                      const std::vector<glm::mat4>& palette, Rng& rng)
        {
            if (gore.isGib || profile.severable.empty() || gore.cutCount >= kMaxCuts || source.severPower <= 0.0f)
                return false;

            // Inside an already severed subtree: that is a stump hit, not a new sever.
            for (int i = 0; i < gore.cutCount; ++i)
            {
                const Cut& c = gore.cuts[i];
                if (c.boneSkel < profile.subtreeEnd.size() && anchor.skel >= c.boneSkel && anchor.skel < profile.subtreeEnd[c.boneSkel])
                    return false;
            }

            // Candidate: a severable bone that is the hit bone or below it and whose bounds contain the point (smallest wins).
            // A bone whose subtree holds an earlier cut is skipped (it would drag the old stump along).
            int candidate = -1;
            float candidateVolume = 1.0e9f;
            const uint16_t first = anchor.skel;
            const uint16_t last = first < profile.subtreeEnd.size() ? profile.subtreeEnd[first] : static_cast<uint16_t>(first + 1);
            for (uint16_t b = first; b < last && b < profile.severableBySkel.size(); ++b)
            {
                const int idx = profile.severableBySkel[b];
                if (idx < 0 || SubtreeHoldsCut(gore, profile, b)) continue;
                if (!PointInBound(profile.anyBounds[b], pBind, 0.02f)) continue;
                const glm::vec3 e = profile.anyBounds[b].bbMax - profile.anyBounds[b].bbMin;
                const float v = e.x * e.y * e.z;
                if (v < candidateVolume) { candidateVolume = v; candidate = idx; }
            }
            if (candidate < 0 && first < profile.severableBySkel.size() && profile.severableBySkel[first] >= 0
                && !SubtreeHoldsCut(gore, profile, first))
                candidate = profile.severableBySkel[first];
            if (candidate < 0) return false;

            const SeverableBone& sev = profile.severable[candidate];

            const double now = Time::GameTime;
            float& damage = gore.severDamage[candidate];
            double& at = gore.severDamageTime[candidate];
            damage = damage * static_cast<float>(std::exp(-(now - at) / kSeverDecaySeconds)) + hit.damage * source.severPower;
            at = now;

            const bool instant = hit.lethal && sev.lethal && source.severPower > 0.5f && IsSeverType(source.type);
            if (damage < kSeverThreshold && !instant) return false;

            glm::vec3 normal = hit.direction;
            if (source.type == SourceType::Slash)
            {
                if (glm::length(hit.sweepNormal) > 1.0e-4f)
                    normal = hit.sweepNormal;
                else
                    normal = glm::cross(hit.direction, std::abs(hit.direction.y) < 0.99f ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0));
            }

            return MakeCut(self, mesh, gore, profile, source, hit, candidate, hit.point, normal, world, palette, rng);
        }

        // An explosion (or a cannon ball) tears off the parts of the body that are close to its centre: the nearest severable
        // bones first, at most kBlastMaxCuts per blast, nothing below the sever threshold.
        constexpr int kBlastMaxCuts = 3;

        void BlastSever(Entity* self, GoreSkeletalMesh* mesh, GoreComponent& gore, const Profile& profile, const Source& source,
                        const Hit& hit, const glm::mat4& world, const std::vector<glm::mat4>& palette, Rng& rng)
        {
            if (gore.isGib || profile.severable.empty()) return;
            if (hit.damage * source.severPower < kSeverThreshold) return;

            // An explosion entity brings its own centre and reach; a cannon ball explodes where it hit, slightly inside.
            const bool entityBlast = hit.blastRadius > 0.0f;
            const float radius = entityBlast ? hit.blastRadius * 0.5f : source.blastRadius;
            if (radius <= 0.0f) return;
            const glm::vec3 centre = entityBlast ? hit.blastCenter : hit.point + hit.direction * 0.1f;

            struct Candidate { int index; float distance; glm::vec3 away; };
            std::vector<Candidate> candidates;
            for (size_t i = 0; i < profile.severable.size(); ++i)
            {
                const SeverableBone& sev = profile.severable[i];
                if (sev.skel >= profile.anyBounds.size() || !profile.anyBounds[sev.skel].hasData) continue;
                if (SubtreeHoldsCut(gore, profile, sev.skel)) continue;

                bool inCutPart = false;   // already gone with an earlier cut
                for (int c = 0; c < gore.cutCount; ++c)
                    if (sev.skel >= gore.cuts[c].boneSkel && sev.skel < profile.subtreeEnd[gore.cuts[c].boneSkel]) { inCutPart = true; break; }
                if (inCutPart) continue;

                glm::vec3 corners[8];
                BoundCorners(profile.anyBounds[sev.skel], corners);
                const glm::mat4 toWorld = BoneBindToWorld(world, palette, sev.skin);
                glm::vec3 lo(FLT_MAX), hi(-FLT_MAX);
                for (const glm::vec3& c : corners)
                {
                    const glm::vec3 p = glm::vec3(toWorld * glm::vec4(c, 1.0f));
                    lo = glm::min(lo, p);
                    hi = glm::max(hi, p);
                }
                const glm::vec3 nearest = glm::clamp(centre, lo, hi);
                const float d = glm::distance(nearest, centre);
                if (d > radius) continue;

                glm::vec3 away = (lo + hi) * 0.5f - centre;
                away = glm::length(away) > 1.0e-4f ? glm::normalize(away) : hit.direction;
                candidates.push_back({ static_cast<int>(i), d, away });
            }

            std::sort(candidates.begin(), candidates.end(), [](const Candidate& a, const Candidate& b) { return a.distance < b.distance; });

            int cuts = 0;
            for (const Candidate& c : candidates)
            {
                if (cuts >= kBlastMaxCuts || gore.cutCount >= kMaxCuts) break;
                if (SubtreeHoldsCut(gore, profile, profile.severable[c.index].skel)) continue;   // an earlier blast cut took it
                if (MakeCut(self, mesh, gore, profile, source, hit, c.index, centre, c.away, world, palette, rng)) ++cuts;
            }
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

            if (source.type != SourceType::Blast && TrySever(self, mesh, gore, profile, source, hit, anchor, pBind, World, palette, rng))
                continue;

            switch (source.type)
            {
            case SourceType::Bullet:
            case SourceType::HeavyBullet:
            {
                const bool heavy = source.type == SourceType::HeavyBullet;
                float r = source.woundRadius * glm::clamp(std::sqrt(damage / source.refDamage), source.minScale, source.maxScale) * rng.Range(0.8f, 1.2f);
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
                const float r = source.woundRadius * glm::clamp(std::sqrt(damage / source.refDamage), source.minScale, source.maxScale);
                const float rz = std::max(r * 0.9f, 0.04f);
                AddWound(gore, MakeShotWound(anchor, reach, WoundKind::Crater, pBind, dBind, r, rz, 0.5f, 0.6f, rng));
                AddSplat(gore, pBind, 2.5f * r, 1.0f);
                BlastSever(self, mesh, gore, profile, source, hit, World, palette, rng);
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
            // A shotgun blast at close range tears a hole as wide as its spread; every pellet adds to it.
            const float spread = source->clusterRadius * 1.0f * std::sqrt(std::min(static_cast<float>(c.count), 9.0f) / 5.0f);
            const float r = std::max(source->woundRadius * (1.5f + 0.6f * std::sqrt(static_cast<float>(c.count))), spread);
            const float rz = std::max(r * 0.9f, 0.04f);
            const float reach = 2.0f * MaxExtent(profile, anchor.skel);

            AddWound(gore, MakeShotWound(anchor, reach, WoundKind::Crater, c.center, dir, r, rz, 0.4f, 0.5f, rng));
            AddSplat(gore, c.center, 2.5f * r, 1.0f);
        }

        // Cavity geometry for every wound that has none yet (new or merged): built on a worker thread, drawn when it arrives.
        for (int i = 0; i < gore.woundCount; ++i)
        {
            Wound& w = gore.wounds[i];
            if (!w.cavity && !w.cavityTicket && !w.cavityFailed)
                RequestCavity(w, *mesh->GetSkinnedModel(), profile);
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
        // Only wounds whose cavity exists are drawn (the hole alone would show through the body).
        for (int i = 0; i < gore.woundCount; ++i)
            if (gore.wounds[i].cavity)
                state.wounds[state.woundCount++] = gore.wounds[i];
        state.cuts = gore.cuts;
        state.cutCount = gore.cutCount;
        state.splats = gore.splats;
        state.splatCount = gore.splatCount;
        state.frozenClock = gore.frozenClock;
        state.pausedUntil = gore.pausedUntil;
        state.isGib = gore.isGib;
        if (gore.isGib) state.gibSeverPalette = gore.gibSeverPalette;
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

    bool cavityArrived = false;
    for (int i = 0; i < gore.woundCount; ++i)
        cavityArrived |= Gore::PollCavity(gore.wounds[i]);
    if (cavityArrived) gore.version++;

    Gore::PushRenderStateIfChanged(self, gore);
}
REGISTER_TICK(GoreProcessHitsSystem)
