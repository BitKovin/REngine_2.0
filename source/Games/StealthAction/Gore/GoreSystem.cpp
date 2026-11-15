#include "GoreSystem.h"
#include "GoreCap.h"
#include "GoreChunks.h"
#include "GoreCutSolver.h"
#include "GoreEvents.h"
#include "GoreGib.h"
#include "GoreProfile.h"
#include "GoreSettings.h"
#include "GoreSkeletalMesh.h"
#include "GoreSource.h"

#include <Ecs/Ecs.h>
#include <Ecs/ISystem.h>
#include <Entity.h>
#include <Physics.h>
#include <PhysicsConverter.h>
#include <Logger.hpp>
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

        // How deep a new wound of each source type goes (see GoreTypes.h kShellLayers): 0.5 = skin and a little meat,
        // 0.2 = down to the bone, 0.1 = through everything. A source may override it ("taper" in sources.json).
        float TaperOf(const Source& source, WoundKind kind)
        {
            if (source.taper >= 0.0f) return glm::clamp(source.taper, kMinTaper, 0.6f);
            switch (source.type)
            {
            case SourceType::Bullet:      return kind == WoundKind::Exit ? 0.34f : 0.45f;
            case SourceType::HeavyBullet: return kind == WoundKind::Exit ? 0.20f : 0.30f;
            case SourceType::Pellet:      return 0.22f;
            case SourceType::Slash:       return 0.34f;
            case SourceType::Blast:       return 0.14f;
            default:                      return 0.50f;
            }
        }

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

        // direction: bind space, the way the blow travelled (see Splat); none = a plain soft soak. bulletHole: draw a bullet hole, not a splash.
        void AddSplat(GoreComponent& g, const glm::vec3& center, float radius, float strength,
                      const glm::vec3& direction = glm::vec3(0), bool bulletHole = false)
        {
            Splat s;
            s.center = center;
            s.radius = radius;
            s.strength = strength;
            s.order = g.orderCounter++;
            s.direction = direction;
            s.bulletHole = bulletHole;

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
            if (a.limitSkel >= 0 || b.limitSkel >= 0) return false;   // a sever wound stays as it is
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
            w.taper = std::max(w.taper - 0.07f * weight, kMinTaper);   // stacking digs deeper
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
                    w.taper = std::min(w.taper, t.taper);
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
                merged.taper = std::min(merged.taper, other.taper);
                merged.order = w.order;
                GrowWound(merged, std::min(rt, rn), 1.0f);
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
                        const glm::vec3& radii, float roughness, float rim, float taper, Rng& rng)
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
            w.taper = taper;
            w.seed = rng.Next01();
            w.reach = reach;
            return w;
        }

        // A shot wound is long along the shot so it always reaches the real mesh surface (the hit point is on the hitbox
        // box, which is not exactly the skin). The centre sits slightly inside.
        Wound MakeShotWound(const Anchor& a, float reach, WoundKind kind, const glm::vec3& surfacePoint, const glm::vec3& dir,
                            float r, float zRadius, float roughness, float rim, float taper, Rng& rng)
        {
            const glm::vec3 center = surfacePoint + dir * (zRadius * 0.35f);
            return MakeWound(a, reach, kind, center, RotateZTo(dir), glm::vec3(r, r, zRadius), roughness, rim, taper, rng);
        }


        constexpr double kAuthoredDecaySeconds = 20.0;   // damage collected towards a part fades slowly
        constexpr float kKillingBlowWeight = 2.5f;       // the hit that kills counts extra: overkill tears things off
        constexpr float kBulletHoleDecal = 3.0f;         // extent of a bullet hole decal, in wound radii

        float EffectiveDamage(const Hit& hit) { return hit.lethal ? hit.damage * kKillingBlowWeight : hit.damage; }

        bool IsProtectedWhileAlive(const Profile& profile, uint16_t skel)
        {
            return skel < profile.protectedWhileAlive.size() && profile.protectedWhileAlive[skel] != 0;
        }

        bool InRemovedPart(const GoreComponent& gore, const Profile& profile, uint16_t skel)
        {
            for (int i = 0; i < gore.cutCount; ++i)
            {
                const uint16_t c = gore.cuts[i].boneSkel;
                if (c < profile.subtreeEnd.size() && skel >= c && skel < profile.subtreeEnd[c]) return true;
            }
            return false;
        }

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

        // The wound that tears a ragged band out of every layer on both sides of a torn cut's plane: skin ends first, meat next,
        // the bone sticks out furthest. Limited to the cut bone's subtree; the gib copies it (it is anchored on the cut bone).
        void AddSeverWound(GoreComponent& gore, const Profile& profile, const Cut& cut, Rng& rng)
        {
            Wound w;
            const glm::vec3 n = glm::normalize(glm::vec3(cut.plane));
            const float across = std::max(1.7f * cut.outlineRadius, 0.06f);
            w.center = cut.center;
            w.rotation = RotateZTo(n);
            w.radii = glm::vec3(across, across, 0.055f);
            w.anchorSkin = cut.boneSkin;
            w.anchorSkel = cut.boneSkel;
            w.limitSkel = static_cast<int16_t>(cut.boneSkel);
            w.kind = WoundKind::Crater;
            w.taper = kMinTaper;
            w.roughness = 0.45f;
            w.rimWidth = 0.6f;
            w.seed = rng.Next01();
            w.reach = 2.0f * MaxExtent(profile, cut.boneSkel);
            w.order = gore.orderCounter++;

            if (gore.woundCount < kMaxWounds)
            {
                gore.wounds[gore.woundCount++] = w;
            }
            else
            {
                int oldest = -1;   // replace the oldest wound that is not a sever wound
                for (int i = 0; i < gore.woundCount; ++i)
                    if (gore.wounds[i].limitSkel < 0 && (oldest < 0 || gore.wounds[i].order < gore.wounds[oldest].order)) oldest = i;
                if (oldest >= 0) gore.wounds[oldest] = w;
            }
        }

        // Cuts a severable bone at the plane nearest to `point` (leaning towards `normal`), hides its hitboxes, spawns the gib.
        // Meat chunks are a "full gore" effect: only a body whose gore level is Full throws them.
        void ThrowChunks(const GoreSkeletalMesh* mesh, const glm::vec3& point, const glm::vec3& direction, int count, float speed, float size)
        {
            if (mesh->goreSettings.level == GoreSettings::Level::Full) SpawnChunks(point, direction, count, speed, size);
        }

        bool MakeCut(Entity* self, GoreSkeletalMesh* mesh, GoreComponent& gore, const Profile& profile, const Source& source,
                     const Hit& hit, int candidate, const glm::vec3& point, const glm::vec3& normal, bool atJoint, bool strictTilt,
                     const glm::mat4& world, const std::vector<glm::mat4>& palette, Rng& rng)
        {
            CutRequest request;
            request.severableIndex = static_cast<uint16_t>(candidate);
            request.worldPoint = point;
            request.worldNormal = normal;
            request.atJoint = atJoint;
            request.strictTilt = strictTilt;

            const CutResult result = SolveCut(profile, *mesh->GetSkinnedModel(), world, palette, request, rng);
            if (!result.ok) return false;

            Cut cut = result.cut;
            cut.severedAt = Time::GameTime;
            cut.phase = CutPhase::Severed;

            // Every cut ends on its plane, and the face of the cut is generated from the meshes: a solid cross-section (the cap,
            // skin, flesh, bone, organs) for the body and for the gib. A blade leaves it clean; a bullet or a blast tears it open
            // with blood and chunks (`clean` only decides that). The face is built on the worker thread: the cut stays out of the
            // render state until it is ready (PushRenderStateIfChanged), and becomes a torn cut with a sever wound there if the
            // plane found no skin to slice.
            cut.clean = strictTilt;
            cut.roughness = 0.0f;                        // skin and flesh end exactly on the plane
            cut.cap = RequestCap(*mesh->GetSkinnedModel(), profile, cut);

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
            SpawnBloodBurst(point, hit.direction + glm::vec3(0, 0.5f, 0), 6, 1.2f);
            if (!cut.clean) ThrowChunks(mesh, point, hit.direction, 5, 3.5f, 0.035f);   // a blade does not throw meat

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
            if (candidate < 0)
            {
                // A hit on a bone that is not severable (the pelvis) lands just outside the bounds of the severable part above it
                // (the waist): the nearest severable part of its subtree counts, when the point is within 12 cm of it.
                float nearest = 0.12f;
                for (uint16_t b = first; b < last && b < profile.severableBySkel.size(); ++b)
                {
                    const int idx = profile.severableBySkel[b];
                    if (idx < 0 || SubtreeHoldsCut(gore, profile, b) || !profile.anyBounds[b].hasData) continue;
                    const float d = glm::distance(pBind, glm::clamp(pBind, profile.anyBounds[b].bbMin, profile.anyBounds[b].bbMax));
                    if (d < nearest) { nearest = d; candidate = idx; }
                }
            }
            if (candidate < 0)
            {
                // A hit on a hand counts for the forearm: walk up to the nearest severable ancestor.
                uint16_t b = anchor.skel;
                while (b < profile.parent.size())
                {
                    if (b < profile.severableBySkel.size() && profile.severableBySkel[b] >= 0)
                    {
                        if (!SubtreeHoldsCut(gore, profile, b)) candidate = profile.severableBySkel[b];
                        break;
                    }
                    const uint16_t parent = profile.parent[b];
                    if (parent >= b) break;   // root (INVALID_BONE_IDX is larger than any index)
                    b = parent;
                }
            }
            if (candidate < 0) return false;

            const SeverableBone& sev = profile.severable[candidate];

            const double now = Time::GameTime;
            float& damage = gore.severDamage[candidate];
            double& at = gore.severDamageTime[candidate];
            damage = damage * static_cast<float>(std::exp(-(now - at) / kAuthoredDecaySeconds)) + EffectiveDamage(hit) * source.severPower;
            at = now;

            // A living enemy keeps its limbs and head (it still collects the damage, so the killing blow tears them off).
            if (hit.alive && !hit.lethal && !mesh->goreSettings.severAlive) return false;
            // Whatever the setting, a living enemy never loses a part that holds a protected bone (a waist cut would take the head).
            if (hit.alive && !hit.lethal && sev.holdsProtected) return false;

            const bool instant = hit.lethal && sev.lethal && source.severPower > 0.5f && IsSeverType(source.type);
            if (damage < sev.detachDamage && !instant) return false;

            glm::vec3 normal = hit.direction;
            bool strict = false;
            if (source.type == SourceType::Slash)
            {
                strict = true;   // a blade only cuts when it goes across the limb
                if (glm::length(hit.sweepNormal) > 1.0e-4f)
                    normal = hit.sweepNormal;
                else
                    normal = glm::cross(hit.direction, std::abs(hit.direction.y) < 0.99f ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0));
            }
            else
            {
                normal = glm::vec3(0.0f);   // not a blade: straight across the bone (SolveCut falls back to the bone axis)
            }

            return MakeCut(self, mesh, gore, profile, source, hit, candidate, hit.point, normal, false, strict, world, palette, rng);
        }

        // An explosion (or a cannon ball) tears off the parts of the body that are close to its centre: the nearest severable
        // bones first, at most kBlastMaxCuts per blast, nothing below the sever threshold.
        constexpr int kBlastMaxCuts = 3;

        void BlastSever(Entity* self, GoreSkeletalMesh* mesh, GoreComponent& gore, const Profile& profile, const Source& source,
                        const Hit& hit, const glm::mat4& world, const std::vector<glm::mat4>& palette, Rng& rng)
        {
            if (gore.isGib || profile.severable.empty()) return;
            if (EffectiveDamage(hit) * source.severPower < 60.0f) return;
            if (hit.alive && !hit.lethal && !mesh->goreSettings.severAlive) return;

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
                if (hit.alive && !hit.lethal && sev.holdsProtected) continue;   // a living enemy keeps its head
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
                if (MakeCut(self, mesh, gore, profile, source, hit, c.index, centre, c.away, false, false, world, palette, rng)) ++cuts;
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
            bool lethal = false;
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
        gore.pending[gore.pendingCount - 1].alive |= hit.alive;
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
            if (mesh->GetSkinnedModel()->meshes.empty() || profile.loading) return;
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

            if (!gore.isGib && InRemovedPart(gore, profile, anchor.skel)) continue;   // that part is gone

            // A living enemy keeps its protected bones whole (the head): no hole, no cut, no meat, unless this hit kills it.
            // The blood still shows that the hit landed.
            if (hit.alive && !hit.lethal && IsProtectedWhileAlive(profile, anchor.skel))
            {
                // The sever damage is still collected (TrySever refuses the cut), so that the killing blow can take the part off.
                if (source.type != SourceType::Blast)
                    TrySever(self, mesh, gore, profile, source, hit, anchor, pBind, World, palette, rng);
                AddSplat(gore, pBind, glm::clamp(0.05f * hit.damage / std::max(source.refDamage, 1.0f), 0.03f, 0.1f), 0.8f, dBind);
                SpawnBloodBurst(hit.point, -hit.direction + glm::vec3(0, 0.4f, 0), 1, 0.7f);
                continue;
            }
            if (source.type != SourceType::Blast && TrySever(self, mesh, gore, profile, source, hit, anchor, pBind, World, palette, rng))
                continue;

            // A weapon that makes no holes (sources.json "holes": false; a pistol, a rifle): the hit is a decal of a bullet hole with
            // blood around it. Nothing is dug, thrown out or sprayed.
            if (!source.holes && source.type != SourceType::Blunt)
            {
                const float r = source.woundRadius * glm::clamp(std::sqrt(damage / source.refDamage), source.minScale, source.maxScale)
                              * rng.Range(0.8f, 1.2f) * (hit.lethal ? 1.5f : 1.0f);
                AddSplat(gore, pBind, kBulletHoleDecal * r, 1.0f, dBind, true);
                continue;
            }

            switch (source.type)
            {
            case SourceType::Bullet:
            case SourceType::HeavyBullet:
            {
                const bool heavy = source.type == SourceType::HeavyBullet;
                float r = source.woundRadius * glm::clamp(std::sqrt(damage / source.refDamage), source.minScale, source.maxScale) * rng.Range(0.8f, 1.2f);
                if (heavy) r *= 1.3f;
                r *= hit.lethal ? 1.5f : 1.0f;
                const float rz = std::max(r * 1.6f, 0.05f);

                AddWound(gore, MakeShotWound(anchor, reach, WoundKind::Entry, pBind, dBind, r, rz, 0.3f, 0.5f, TaperOf(source, WoundKind::Entry), rng));
                AddSplat(gore, pBind, 2.5f * r, 1.0f, dBind, true);   // the blood around the hole (the hole itself is cut out)
                SpawnBloodBurst(hit.point, -hit.direction + glm::vec3(0, 0.4f, 0), heavy ? 2 : 1, 0.7f);
                if (hit.lethal) ThrowChunks(mesh, hit.point, -hit.direction, 2, 2.0f, 0.022f);

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
                                                 glm::vec3(er, er, erz), 0.45f, 0.5f, TaperOf(source, WoundKind::Exit), rng));
                        AddSplat(gore, exitBind, 2.5f * er, 1.0f, dBind);
                        SpawnBloodBurst(exitWorld, hit.direction + glm::vec3(0, 0.25f, 0), heavy ? 5 : 3, 1.1f);
                        if (heavy) ThrowChunks(mesh, exitWorld, hit.direction, 3, 4.0f, 0.028f);
                        else ThrowChunks(mesh, exitWorld, hit.direction, 1, 3.0f, 0.022f);
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
                target->lethal |= hit.lethal;
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

                AddWound(gore, MakeWound(anchor, reach, WoundKind::Gash, center, rot, radii, 0.25f, 0.4f, TaperOf(source, WoundKind::Gash), rng));
                AddSplat(gore, pBind, 2.5f * source.woundRadius + 0.03f, 1.0f, dBind);
                SpawnBloodBurst(hit.point, -hit.direction + glm::vec3(0, 0.6f, 0), 3, 0.9f);
                ThrowChunks(mesh, hit.point, -hit.direction, 1, 2.0f, 0.025f);
                break;
            }

            case SourceType::Blunt:
            {
                AddSplat(gore, pBind, std::max(0.08f * hit.damage / source.refDamage, 0.03f), 0.8f, dBind);
                break;
            }

            case SourceType::Blast:
            {
                const float r = source.woundRadius * glm::clamp(std::sqrt(damage / source.refDamage), source.minScale, source.maxScale)
                              * (hit.lethal ? 1.5f : 1.0f);
                const float rz = std::max(r * 0.9f, 0.04f);
                AddWound(gore, MakeShotWound(anchor, reach, WoundKind::Crater, pBind, dBind, r, rz, 0.5f, 0.6f, TaperOf(source, WoundKind::Crater), rng));
                AddSplat(gore, pBind, 2.5f * r, 1.0f, dBind);
                SpawnBloodBurst(hit.point, glm::vec3(0, 1, 0), 6, 1.2f);
                ThrowChunks(mesh, hit.point, glm::vec3(0, 1, 0), 8, 5.0f, 0.04f);
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
            const float r = std::max(source->woundRadius * (1.5f + 0.6f * std::sqrt(static_cast<float>(c.count))), spread)
                          * (c.lethal ? 1.4f : 1.0f);
            const float rz = std::max(r * 0.9f, 0.04f);
            const float reach = 2.0f * MaxExtent(profile, anchor.skel);

            AddWound(gore, MakeShotWound(anchor, reach, WoundKind::Crater, c.center, dir, r, rz, 0.4f, 0.5f, TaperOf(*source, WoundKind::Crater), rng));
            AddSplat(gore, c.center, 2.5f * r, 1.0f, dir);

            if (c.skin >= 0 && c.skin < static_cast<int>(palette.size()))
            {
                const glm::mat4 toWorld = World * palette[c.skin];
                const glm::vec3 p = glm::vec3(toWorld * glm::vec4(c.center, 1.0f));
                const glm::vec3 d = glm::normalize(glm::vec3(toWorld * glm::vec4(dir, 0.0f)));
                SpawnBloodBurst(p, -d + glm::vec3(0, 0.4f, 0), 2, 0.8f);
                SpawnBloodBurst(p + d * 0.2f, d + glm::vec3(0, 0.25f, 0), 4, 1.2f);
                ThrowChunks(mesh, p + d * 0.15f, d, 2 + std::min(c.count, 6) / 2, 4.5f, 0.03f);
                ThrowChunks(mesh, p, -d, 2, 2.0f, 0.025f);
            }
        }

        gore.pendingCount = 0;
        gore.version++;
    }

    void PushRenderStateIfChanged(Entity* self, GoreComponent& gore)
    {
        GoreSkeletalMesh* mesh = FindGoreMesh(self);
        if (mesh == nullptr || mesh->GetSkinnedModel() == nullptr) return;

        // Cut caps are built on the worker: a cut is shown (and its gib) only once its cap is ready, a few frames at most. The
        // builds finish in the order they were queued, so the cuts still waiting are the last ones.
        int shownCuts = gore.cutCount;
        for (int i = 0; i < gore.cutCount; ++i)
        {
            Cut& c = gore.cuts[i];
            if (!c.cap) continue;
            const CapGeometry::State st = c.cap->GetState();
            if (st == CapGeometry::State::Building)
            {
                shownCuts = std::min(shownCuts, i);
            }
            else if (st == CapGeometry::State::Failed)
            {
                // The plane found no skin to slice: make it a torn cut after all.
                const Profile& failedProfile = GetProfile(mesh->filePath, *mesh->GetSkinnedModel());
                c.clean = false;
                c.cap.reset();
                c.roughness = 0.012f;
                Rng rng(static_cast<uint32_t>(c.seed * 16777216.0f) + 1u);
                AddSeverWound(gore, failedProfile, c, rng);
                ++gore.version;
            }
        }
        if (shownCuts != gore.pushedCuts)
            ++gore.version;
        if (mesh->GetGoreVersion() == gore.version) return;

        const Profile& profile = GetProfile(mesh->filePath, *mesh->GetSkinnedModel());
        if (!profile.valid && (mesh->GetSkinnedModel()->meshes.empty() || profile.loading)) return;   // geometry not loaded yet: retry

        RenderState state;
        state.version = gore.version;
        state.wounds = gore.wounds;
        state.woundCount = gore.woundCount;
        state.cuts = gore.cuts;
        state.cutCount = static_cast<uint8_t>(shownCuts);
        gore.pushedCuts = static_cast<uint8_t>(shownCuts);
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

    Gore::PushRenderStateIfChanged(self, gore);
}
REGISTER_TICK(GoreProcessHitsSystem)
