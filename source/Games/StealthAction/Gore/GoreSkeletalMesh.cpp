#include "GoreSkeletalMesh.h"
#include <EngineMain.h>
#include "GoreProfile.h"
#include "GoreSettings.h"
#include "GoreCap.h"
#include "GoreCutSolver.h"
#include "GoreFlesh.h"

#include <DrawCommands/SurfaceType.h>

#include <Time.hpp>

#include <bgfx/bgfx.h>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <algorithm>
#include <cmath>

namespace Gore
{
    namespace
    {
        // Bit i of a mask is stored as a float sum: bit i adds 2^i (exact for up to 24 bits).
        inline float Bit(int i) { return static_cast<float>(1u << i); }

        // 3 rows of the affine part of M, for "3 rows" upload (GLSL: dot(row, vec4(p, 1))).
        template <size_t N>
        void StoreRows(std::array<glm::vec4, N>& out, int slot, const glm::mat4& M)
        {
            out[slot * 3 + 0] = glm::row(M, 0);
            out[slot * 3 + 1] = glm::row(M, 1);
            out[slot * 3 + 2] = glm::row(M, 2);
        }

        bool AabbOverlap(const glm::vec3& aMin, const glm::vec3& aMax, const glm::vec3& bMin, const glm::vec3& bMax)
        {
            return aMin.x <= bMax.x && aMax.x >= bMin.x
                && aMin.y <= bMax.y && aMax.y >= bMin.y
                && aMin.z <= bMax.z && aMax.z >= bMin.z;
        }

        // True when bone b is an ancestor of the cut bone and the cut asked for its ancestors to be clipped as well.
        bool CutClipsAncestor(const Profile& profile, const Cut& c, size_t b)
        {
            if (c.boneSkel >= profile.severableBySkel.size() || b >= c.boneSkel || b >= profile.subtreeEnd.size() || profile.subtreeEnd[b] <= c.boneSkel)
                return false;
            const int idx = profile.severableBySkel[c.boneSkel];
            return idx >= 0 && profile.severable[idx].clipAncestors;
        }

        bool SphereOverlapsAabb(const glm::vec3& c, float r, const glm::vec3& bMin, const glm::vec3& bMax)
        {
            const glm::vec3 closest = glm::clamp(c, bMin, bMax);
            return glm::dot(closest - c, closest - c) <= r * r;
        }

        // Half extent of the bind-space AABB of an ellipsoid with the given rotation and radii.
        glm::vec3 EllipsoidHalfExtent(const glm::quat& rotation, const glm::vec3& radii)
        {
            const glm::mat3 R = glm::mat3_cast(rotation);
            glm::vec3 e(0.0f);
            for (int k = 0; k < 3; ++k)
            {
                float sum = 0.0f;
                for (int j = 0; j < 3; ++j)
                {
                    const float v = R[j][k] * radii[j];
                    sum += v * v;
                }
                e[k] = std::sqrt(sum);
            }
            return e;
        }

        float MeanRadius(const glm::vec3& radii) { return (radii.x + radii.y + radii.z) / 3.0f; }

        // The shader side of a list of wounds: the inverse ellipsoid matrices and their parameters.
        // cavities: the model can build the closing surface of a wound (GoreCavity.h). Such a wound removes the skin and every shell
        // inside the same hole and lets the cavity be its wall; without one, the shells' holes shrink with depth so the hole does
        // not show through.
        void FillWoundUniforms(GpuData& g, const Wound* wounds, int count, double clock, bool cavities)
        {
            for (int i = 0; i < count; ++i)
            {
                const Wound& w = wounds[i];
                const bool closed = cavities && HasCavity(w);
                const mat4 Mb = glm::translate(mat4(1.0f), w.center) * glm::mat4_cast(w.rotation) * glm::scale(mat4(1.0f), w.radii);

                StoreRows(g.woundBind, i, glm::inverse(Mb));

                float healScale = 1.0f;
                if (w.healDuration > 0.0f)
                {
                    const float t = glm::clamp(static_cast<float>((clock - w.healStart) / w.healDuration), 0.0f, 1.0f);
                    healScale = 1.0f - t * t * (3.0f - 2.0f * t);
                }

                g.woundParams[i] = glm::vec4(w.roughness, w.rimWidth, healScale, w.seed);
                // y: 1 = bones and organs stay behind this wound (a shot or a gash opens skin and flesh, not the insides).
                // A sever wound still eats the insides: the bone sticks out of a torn stump.
                // x: taper (0 when a cavity closes the hole). z: 1 = every layer has the same hole edge.
                g.woundExtra[i] = glm::vec4(closed ? 0.0f : w.taper, w.limitSkel >= 0 ? 0.0f : 1.0f, closed ? 1.0f : 0.0f, 0.0f);
            }
        }
    }

    void GoreSkeletalMesh::SetGoreState(const RenderState& state)
    {
        if (state.version == goreState.version && state.profile == goreState.profile)
            return;

        goreState = state;
        RefreshCavity();
    }

    void GoreSkeletalMesh::ClearGoreState()
    {
        goreState = RenderState();
        RefreshCavity();
    }

    // Game thread. Asks the worker for the closing surface of the wounds when they are not the ones the last request was for.
    void GoreSkeletalMesh::RefreshCavity()
    {
        const Profile* profile = goreState.profile;
        const roj::SkinnedModel* skinned = GetSkinnedModel();
        if (profile == nullptr || !profile->valid || skinned == nullptr || goreState.woundCount == 0)
        {
            cavityBuilding.reset();
            cavityShown.reset();
            cavityWoundCount = -1;
            return;
        }

        if (cavityWoundCount >= 0 && SameWounds(cavityWounds, cavityWoundCount, goreState.wounds, goreState.woundCount))
            return;

        cavityWounds = goreState.wounds;
        cavityWoundCount = goreState.woundCount;
        cavityBuilding = RequestCavity(*profile, goreState.wounds, goreState.woundCount);
        if (!cavityBuilding) cavityShown.reset();   // no wound has an inside (sever wounds only)
    }

    void GoreSkeletalMesh::OnAssetsReloaded()
    {
        SkeletalMesh::OnAssetsReloaded();

        // New command objects; the final* lists are not touched here: the renderer may still hold the old ones.
        shellCommands.clear();
        for (size_t i = 0; i < GetSubMeshCount() * kShellLayers; ++i)
        {
            auto sc = std::make_shared<GoreDrawCommand>();
            sc->pass = GoreDrawCommand::Pass::Shell;
            shellCommands.push_back(sc);
        }
        anatomyCommand = std::make_shared<GoreDrawCommand>();
        anatomyCommand->pass = GoreDrawCommand::Pass::Anatomy;
        cavityCommand = std::make_shared<GoreDrawCommand>();
        cavityCommand->pass = GoreDrawCommand::Pass::Anatomy;
        cavityCommand->cavity = true;
        cavityShown.reset();                // made for the old model
        cavityBuilding.reset();
        cavityWoundCount = -1;
        for (auto& cc : capCommands)
        {
            cc = std::make_shared<GoreDrawCommand>();
            cc->pass = GoreDrawCommand::Pass::Anatomy;
            cc->cap = true;
        }

        builtVersion = 0;
    }

    const std::vector<mat4>* GoreSkeletalMesh::GetRenderPalette() const
    {
        return usingRenderPalette ? &renderPalette : SkeletalMesh::GetRenderPalette();
    }

    void GoreSkeletalMesh::PreFinalize()
    {
        SkeletalMesh::PreFinalize();   // finalizedWorld, rewiring after a model reload

        pendingActive = false;
        pendingUsePalette = false;
        pendingCavity.reset();

        // Stencil shadows: a body that still has all its limbs casts the usual shadow of the model. A body that lost a limb
        // casts none (the volume of the whole model would still include the lost part, and cutting a volume per bone is not
        // reliable): its detail shadows are forced off until the limb is back.
        const bool lostLimb = goreState.version != 0 && !goreState.isGib && goreState.cutCount > 0;
        if (lostLimb && !detailShadowsForcedOff)
        {
            detailShadowsBeforeLoss = CastDetailShadows;
            CastDetailShadows = false;
            detailShadowsForcedOff = true;
        }
        else if (!lostLimb && detailShadowsForcedOff)
        {
            CastDetailShadows = detailShadowsBeforeLoss;
            detailShadowsForcedOff = false;
        }

        if (cavityBuilding && cavityBuilding->GetState() == CavityGeometry::State::Ready)
        {
            cavityShown = cavityBuilding;
            cavityBuilding.reset();
        }

        if (!HasGore() || model == nullptr || model->meshes.empty())
            return;

        // The mesh never loads the profile itself (game thread only): it uses the one the gore system set.
        const Profile* profile = goreState.profile;
        if (profile == nullptr || !profile->valid)
            return;

        if (GetSkinPalette().empty())
            return;

        pendingActive = true;

        if (builtVersion != goreState.version || builtProfile != profile || pendingRanges.size() != model->meshes.size())
        {
            BuildRanges();
            builtVersion = goreState.version;
            builtProfile = profile;
        }

        FillGpuData();

        // The cavity is subtracted by the wounds it was built from, which may be fewer than the current ones for a frame or two.
        if (cavityShown && !cavityShown->indices.empty())
        {
            pendingCavity = cavityShown;
            pendingCavityGpu = pendingGpu;
            pendingCavityGpu.counts.x = static_cast<float>(cavityShown->woundCount);
            FillWoundUniforms(pendingCavityGpu, cavityShown->wounds.data(), cavityShown->woundCount,
                              RegenClock(goreState.frozenClock, goreState.pausedUntil, Time::GameTime), true);
        }
    }

    // Index ranges per submesh: one range per run of bones with the same primitive mask.
    void GoreSkeletalMesh::BuildRanges()
    {
        const Profile& profile = *goreState.profile;

        pendingRanges.resize(model->meshes.size());

        // Bind-space AABBs of the wounds, grown by edge noise and rim.
        std::array<glm::vec3, kMaxWounds> woundMin, woundMax;
        for (int i = 0; i < goreState.woundCount; ++i)
        {
            const Wound& w = goreState.wounds[i];
            const glm::vec3 e = EllipsoidHalfExtent(w.rotation, w.radii) * (1.0f + w.roughness + w.rimWidth);
            woundMin[i] = w.center - e;
            woundMax[i] = w.center + e;
        }

        // Mask of everything that touches a box that follows skel bone `b`. Used for bone ranges and anatomy pieces.
        auto maskFor = [&](size_t b, const glm::vec3& bbMin, const glm::vec3& bbMax) -> glm::vec4
        {
            float woundBits = 0.0f, cutBits = 0.0f, splatBits = 0.0f;

            for (int i = 0; i < goreState.woundCount; ++i)
            {
                const Wound& w = goreState.wounds[i];
                if (w.limitSkel >= 0)   // a sever wound only touches the cut bone and what hangs on it
                {
                    const size_t first = static_cast<size_t>(w.limitSkel);
                    if (first >= profile.subtreeEnd.size() || b < first || b >= profile.subtreeEnd[first]) continue;
                }
                if (AabbOverlap(woundMin[i], woundMax[i], bbMin, bbMax))
                    woundBits += Bit(i);
            }

            for (int i = 0; i < goreState.splatCount; ++i)
            {
                const Splat& sp = goreState.splats[i];
                if (SphereOverlapsAabb(sp.center, sp.radius, bbMin, bbMax))
                    splatBits += Bit(i);
            }

            for (int i = 0; i < goreState.cutCount; ++i)
            {
                const Cut& c = goreState.cuts[i];
                // Only the cut bone itself (body) / its subtree (gib) is clipped by the plane - never neighbours that
                // merely lie near it (a diagonal limb's bounding box reaches far into the torso). A trunk cut also clips
                // its ancestors' ranges (see CutClipsAncestor).
                if (c.boneSkel >= profile.subtreeEnd.size())
                    continue;
                const bool inPart = b >= c.boneSkel && b < profile.subtreeEnd[c.boneSkel];
                if (!inPart && !CutClipsAncestor(profile, c, b))
                    continue;
                if (!goreState.isGib && inPart && b != c.boneSkel && !IsStraggler(c, b))
                    continue;
                if (!SphereOverlapsAabb(c.center, c.influenceRadius, bbMin, bbMax))
                    continue;

                float maxDist = -1.0e9f;
                for (int corner = 0; corner < 8; ++corner)
                {
                    const glm::vec3 pt((corner & 1) ? bbMax.x : bbMin.x, (corner & 2) ? bbMax.y : bbMin.y, (corner & 4) ? bbMax.z : bbMin.z);
                    maxDist = std::max(maxDist, glm::dot(glm::vec3(c.plane), pt) - c.plane.w);
                }
                // A gib keeps the removed side, so it needs the opposite test.
                if (goreState.isGib) maxDist = 1.0f;
                if (maxDist > -0.01f)
                    cutBits += Bit(i);
            }
            return glm::vec4(woundBits, cutBits, splatBits, 0.0f);
        };

        for (size_t m = 0; m < model->meshes.size(); ++m)
        {
            SubmeshRanges& ranges = pendingRanges[m];
            ranges.front.clear();
            ranges.shell.clear();
            ranges.hasHole = false;

            const roj::SkinnedMesh& mesh = model->meshes[m];
            if (mesh.boneIndexFirst.empty() || mesh.boneOwnBounds.size() + 1 != mesh.boneIndexFirst.size())
                continue;

            const SubmeshRole role = m < profile.submeshRoles.size() ? profile.submeshRoles[m] : SubmeshRole::Volume;
            if (role == SubmeshRole::Attachment)
                continue;

            const size_t n = mesh.boneIndexFirst.size() - 1;

            // Bones inside a severed subtree are not drawn (bodies only). The cut bone itself stays and is clipped by the plane.
            std::vector<uint8_t> removed;
            if (!goreState.isGib && goreState.cutCount > 0)
            {
                removed.assign(n, 0);
                for (int i = 0; i < goreState.cutCount; ++i)
                {
                    const Cut& c = goreState.cuts[i];
                    if (c.boneSkel >= profile.subtreeEnd.size()) continue;
                    for (size_t b = static_cast<size_t>(c.boneSkel) + 1; b < profile.subtreeEnd[c.boneSkel] && b < n; ++b)
                        removed[b] = 1;
                }
            }

            // A gib draws only its own subtree, plus the ancestors that stick out past the cut plane.
            const bool gibWithCut = goreState.isGib && goreState.cutCount > 0 && goreState.cuts[0].boneSkel < profile.subtreeEnd.size();
            const uint16_t gibFirst = gibWithCut ? goreState.cuts[0].boneSkel : 0;
            const uint16_t gibLast = gibWithCut ? profile.subtreeEnd[gibFirst] : 0;

            for (size_t b = 0; b < n; ++b)
            {
                const uint32_t first = mesh.boneIndexFirst[b];
                const uint32_t count = mesh.boneIndexFirst[b + 1] - first;
                if (count == 0) continue;
                if (!removed.empty() && removed[b])
                {
                    // Cloth that hangs far below a trunk cut (a robe's hem) stays on the body, clipped by the plane.
                    bool stays = true;
                    for (int i = 0; i < goreState.cutCount && stays; ++i)
                    {
                        const Cut& c = goreState.cuts[i];
                        if (c.boneSkel < profile.subtreeEnd.size() && b > c.boneSkel && b < profile.subtreeEnd[c.boneSkel] && !IsStraggler(c, b))
                            stays = false;
                    }
                    if (!stays) continue;
                }

                const roj::BoneBound& own = mesh.boneOwnBounds[b];

                if (goreState.isGib)
                {
                    if (!gibWithCut) continue;
                    const bool inPart = (b >= gibFirst && b < gibLast) || CutClipsAncestor(profile, goreState.cuts[0], b);
                    if (!inPart) continue;   // an ancestor of a trunk cut keeps the pixels past the plane (the shader decides which)
                }

                // A prop is never wounded or cut (no mask): it only follows its bone, so the ranges above hide it with the part it hangs on.
                const glm::vec4 mask = (own.hasData && role != SubmeshRole::Prop) ? maskFor(b, own.bbMin, own.bbMax) : glm::vec4(0.0f);

                auto append = [&](std::vector<DrawIndexRange>& list, const glm::vec4& mk)
                {
                    if (!list.empty() && list.back().first + list.back().count == first && list.back().param == mk)
                    {
                        list.back().count += count;
                        return;
                    }
                    DrawIndexRange r;
                    r.first = first;
                    r.count = count;
                    r.param = mk;
                    list.push_back(r);
                };

                append(ranges.front, mask);
                if (mask.x > 0.0f || mask.y > 0.0f) ranges.hasHole = true;

                // Flesh shells: only where a wound or a cut can show them, only on Volume submeshes.
                if (role == SubmeshRole::Volume && (mask.x > 0.0f || mask.y > 0.0f))
                    append(ranges.shell, mask);
            }
        }

        pendingAnatomyRanges.clear();
        if (profile.flesh)
        {
            const bool gibWithCut = goreState.isGib && goreState.cutCount > 0 && goreState.cuts[0].boneSkel < profile.subtreeEnd.size();
            const uint16_t gibFirst = gibWithCut ? goreState.cuts[0].boneSkel : 0;
            const uint16_t gibLast = gibWithCut ? profile.subtreeEnd[gibFirst] : 0;

            for (const FleshPiece& piece : profile.flesh->pieces)
            {
                const size_t b = piece.skel;

                if (goreState.isGib)
                {
                    if (!gibWithCut || ((b < gibFirst || b >= gibLast) && !CutClipsAncestor(profile, goreState.cuts[0], b))) continue;   // not part of this gib
                }
                else
                {
                    bool gone = false;   // strictly below a cut bone: left with the gib
                    for (int i = 0; i < goreState.cutCount && !gone; ++i)
                    {
                        const uint16_t cb = goreState.cuts[i].boneSkel;
                        if (cb < profile.subtreeEnd.size() && b > cb && b < profile.subtreeEnd[cb]) gone = true;
                    }
                    if (gone) continue;
                }

                const glm::vec4 mask = maskFor(b, piece.bbMin, piece.bbMax);
                if (mask.x <= 0.0f && mask.y <= 0.0f) continue;   // nothing opens the body near it: it is hidden anyway

                DrawIndexRange r;
                r.first = piece.first;
                r.count = piece.count;
                r.param = mask;
                pendingAnatomyRanges.push_back(r);
            }
        }
    }

    void GoreSkeletalMesh::FillGpuData()
    {
        const Profile& profile = *goreState.profile;
        const std::vector<mat4>& palette = GetSkinPalette();
        const double now = Time::GameTime;
        const double clock = RegenClock(goreState.frozenClock, goreState.pausedUntil, now);

        GpuData& g = pendingGpu;

        g.counts = glm::vec4(goreState.woundCount, goreState.cutCount, goreState.splatCount, 0.0f);
        g.params = glm::vec4(0.0f, goreState.isGib ? 1.0f : 0.0f, profile.fleshScale, static_cast<float>(goreSettings.level));
        g.bloodColor = glm::vec4(profile.bloodColor, 1.0f);
        g.flesh = profile.fleshTexture;
        g.edgeNoise = profile.edgeNoise;
        g.blood = profile.blood;
        g.decal = profile.decal;
        g.bone = profile.boneT;
        g.guts = profile.guts;

        FillWoundUniforms(g, goreState.wounds.data(), goreState.woundCount, clock, profile.flesh && profile.flesh->body);

        for (int i = 0; i < goreState.cutCount; ++i)
        {
            const Cut& c = goreState.cuts[i];

            g.cutBind[i] = c.plane;
            g.cutBindSphere[i] = glm::vec4(c.center, c.influenceRadius);

            float regrowRadius = -1.0f;
            if (c.phase == CutPhase::Regrowing && c.regrowDuration > 0.0f)
            {
                const float t = glm::clamp(static_cast<float>((clock - c.regrowStart) / c.regrowDuration), 0.0f, 1.0f);
                regrowRadius = std::max(0.0001f, c.regrowMaxRadius * t);
            }
            g.cutRegrowBind[i] = glm::vec4(c.center, regrowRadius);

            const float flags = (c.charred ? 1.0f : 0.0f) + (c.phase == CutPhase::Regrowing ? 2.0f : 0.0f);
            g.cutParams[i] = glm::vec4(c.roughness, c.seed, c.boneRadius, flags);
        }

        for (int i = 0; i < goreState.splatCount; ++i)
        {
            const Splat& sp = goreState.splats[i];
            g.splats[i] = glm::vec4(sp.center, sp.radius * sp.strength);
            g.splatLook[i] = glm::vec4(sp.direction, static_cast<float>(sp.kind));
        }

        BuildRenderPalette(palette);
    }

    // Body: bones below a severed cut take the cut bone's matrix (vertices blended between the bone and its subtree stay
    // together and are cut by the plane). Gib: the bones outside the part keep the pose they had at the moment of the cut,
    // rigidly attached to the part.
    void GoreSkeletalMesh::BuildRenderPalette(const std::vector<mat4>& palette)
    {
        pendingUsePalette = false;

        const Profile& profile = *goreState.profile;
        if (goreState.cutCount == 0 || model == nullptr) return;

        const roj::FlatSkeleton& skeleton = model->skeleton;

        if (goreState.isGib)
        {
            const Cut& c = goreState.cuts[0];
            const std::vector<mat4>& pal0 = goreState.gibSeverPalette;
            if (c.boneSkel >= profile.subtreeEnd.size() || c.boneSkin < 0 || pal0.empty()
                || c.boneSkin >= static_cast<int>(palette.size()) || c.boneSkin >= static_cast<int>(pal0.size()))
                return;

            const uint16_t end = profile.subtreeEnd[c.boneSkel];
            const mat4 rel = palette[c.boneSkin] * glm::inverse(pal0[c.boneSkin]);

            pendingPalette.assign(palette.begin(), palette.end());
            for (uint16_t b = 0; b < skeleton.boneCount; ++b)
            {
                if (b >= c.boneSkel && b < end) continue;
                const int skin = skeleton.bones[b].skinIdx;
                if (skin < 0 || skin >= static_cast<int>(pendingPalette.size()) || skin >= static_cast<int>(pal0.size())) continue;
                pendingPalette[skin] = rel * pal0[skin];
            }
            pendingUsePalette = true;
            return;
        }

        bool copied = false;
        for (int i = 0; i < goreState.cutCount; ++i)
        {
            const Cut& c = goreState.cuts[i];
            if (c.phase != CutPhase::Severed || c.boneSkel >= profile.subtreeEnd.size()) continue;
            if (c.boneSkin < 0 || c.boneSkin >= static_cast<int>(palette.size())) continue;

            if (!copied)
            {
                pendingPalette.assign(palette.begin(), palette.end());
                copied = true;
            }

            const uint16_t end = profile.subtreeEnd[c.boneSkel];
            for (uint16_t b = c.boneSkel + 1; b < end; ++b)
            {
                const int skin = skeleton.bones[b].skinIdx;
                if (skin >= 0 && skin < static_cast<int>(pendingPalette.size()))
                    pendingPalette[skin] = pendingPalette[c.boneSkin];
            }
        }
        pendingUsePalette = copied;
    }

    void GoreSkeletalMesh::FinalizeFrameData()
    {
        SkeletalMesh::FinalizeFrameData();

        if (pendingActive)
        {
            finalGpu = pendingGpu;
            if (pendingCavity) cavityGpu = pendingCavityGpu;
        }

        if (pendingUsePalette)
        {
            renderPalette = pendingPalette;
        }
        usingRenderPalette = pendingUsePalette;

        const Profile* profile = goreState.profile;
        const size_t count = GetSubMeshCount();

        ModelFlesh* flesh = (profile && profile->flesh) ? profile->flesh.get() : nullptr;
        if (flesh && pendingActive) flesh->EnsureGpu();   // main thread: creates the shared buffers on first use

        for (size_t i = 0; i < count; ++i)
        {
            GoreDrawCommand& cmd = static_cast<GoreDrawCommand&>(GetDrawCommandAt(i));
            const SubmeshRole role = (profile && i < profile->submeshRoles.size()) ? profile->submeshRoles[i] : SubmeshRole::Volume;
            const bool hasRanges = pendingActive && i < pendingRanges.size();

            cmd.BoneMatrices = GetRenderPalette();
            cmd.gore = &finalGpu;
            cmd.goreActive = pendingActive && role != SubmeshRole::Attachment && hasRanges;
            cmd.pass = GoreDrawCommand::Pass::Front;
            cmd.layer = 0.0f;
            cmd.insetFraction = 0.0f;
            // A mesh with a hole in it shows its back faces through the hole (the far wall seen from inside, the inside of a sleeve).
            cmd.TwoSided = TwoSided || (cmd.goreActive && pendingRanges[i].hasHole);

            if (cmd.goreActive)
            {
                cmd.IndexRanges.assign(pendingRanges[i].front.begin(), pendingRanges[i].front.end());
                if (cmd.IndexRanges.empty() && goreState.cutCount > 0)
                    cmd.IndexRanges.push_back(DrawIndexRange());   // nothing of this submesh is left: an empty list would draw all of it
                cmd.RangeParamUniform = "u_goreRangeMask";
            }
            else
            {
                cmd.IndexRanges.clear();
                cmd.RangeParamUniform = nullptr;
            }

            for (int k = 0; k < kShellLayers; ++k)
            {
                const size_t slot = i * kShellLayers + k;
                if (slot >= shellCommands.size()) break;
                GoreDrawCommand& sc = *shellCommands[slot];
                sc.CopyCommonFrom(cmd);
                sc.Material.SurfaceTypeOverride = SurfaceType::Opaque;   // flesh is always opaque, whatever the skin texture is
                sc.Material.Masked = false;

                const bool hasBuffer = flesh && i < flesh->shellVbh.size() && bgfx::isValid(flesh->shellVbh[i]);
                sc.goreActive = cmd.goreActive && hasBuffer && i < pendingRanges.size() && !pendingRanges[i].shell.empty();
                sc.pass = GoreDrawCommand::Pass::Shell;
                sc.layer = static_cast<float>(k + 1);
                sc.insetFraction = kShellFraction[k];

                if (sc.goreActive)
                {
                    sc.VertexBuffer = flesh->shellVbh[i];             // the model's index buffer stays (CopyCommonFrom)
                    sc.IndexRanges.assign(pendingRanges[i].shell.begin(), pendingRanges[i].shell.end());
                }
                else
                {
                    sc.IndexRanges.clear();
                }

                sc.RangeParamUniform = "u_goreRangeMask";
                sc.Cull = StaticMeshDrawCommand::CullFace::None;      // the inside of a wall is visible through a hole
                sc.TwoSided = true;
                sc.DepthPrePass = true;
                sc.Material.CustomId = 0;
                sc.OnlyShadows = false;
                sc.ReceiveDetailShadows = false;
                sc.ShadowCapVbh = BGFX_INVALID_HANDLE;
                sc.ShadowCapIbh = BGFX_INVALID_HANDLE;
                sc.ShadowCapIndexCount = 0;
                sc.ShadowEdgeVbh = BGFX_INVALID_HANDLE;
                sc.ShadowEdgeIbh = BGFX_INVALID_HANDLE;
                sc.ShadowEdgeIndexCount = 0;
            }
        }

        if (anatomyCommand)
        {
            GoreDrawCommand& ac = *anatomyCommand;
            const bool hasFront = count > 0;
            if (hasFront) ac.CopyCommonFrom(static_cast<GoreDrawCommand&>(GetDrawCommandAt(0)));
            ac.Material.SurfaceTypeOverride = SurfaceType::Opaque;
            ac.Material.Masked = false;

            ac.goreActive = hasFront && pendingActive && flesh && bgfx::isValid(flesh->anatomyVbh) && bgfx::isValid(flesh->anatomyIbh)
                         && !pendingAnatomyRanges.empty();
            ac.pass = GoreDrawCommand::Pass::Anatomy;
            ac.layer = static_cast<float>(kAnatomyLayer);
            ac.insetFraction = 0.0f;

            if (ac.goreActive)
            {
                ac.VertexBuffer = flesh->anatomyVbh;
                ac.IndexBuffer = flesh->anatomyIbh;
                ac.IndexRanges.assign(pendingAnatomyRanges.begin(), pendingAnatomyRanges.end());
            }
            else
            {
                ac.IndexRanges.clear();
            }

            ac.RangeParamUniform = "u_goreRangeMask";
            ac.Cull = StaticMeshDrawCommand::CullFace::None;
            ac.TwoSided = true;
            ac.DepthPrePass = true;
            ac.Hidden = false;                 // submesh 0 may be on the hide list; the anatomy is not a submesh
            ac.Material.CustomId = 0;
            ac.OnlyShadows = false;
            ac.ReceiveDetailShadows = false;
            ac.ShadowCapVbh = BGFX_INVALID_HANDLE;
            ac.ShadowCapIbh = BGFX_INVALID_HANDLE;
            ac.ShadowCapIndexCount = 0;
            ac.ShadowEdgeVbh = BGFX_INVALID_HANDLE;
            ac.ShadowEdgeIbh = BGFX_INVALID_HANDLE;
            ac.ShadowEdgeIndexCount = 0;
        }

        if (cavityCommand)
        {
            GoreDrawCommand& vc = *cavityCommand;
            if (pendingCavity && pendingActive && count > 0) pendingCavity->EnsureGpu();   // main thread

            vc.goreActive = pendingCavity && pendingActive && count > 0 && bgfx::isValid(pendingCavity->vbh) && bgfx::isValid(pendingCavity->ibh);
            if (vc.goreActive)
            {
                vc.CopyCommonFrom(static_cast<GoreDrawCommand&>(GetDrawCommandAt(0)));   // transform, light, palette
                vc.VertexBuffer = pendingCavity->vbh;
                vc.IndexBuffer = pendingCavity->ibh;
                DrawIndexRange range;
                range.first = 0;
                range.count = static_cast<uint32_t>(pendingCavity->indices.size());
                // Every wound of the state the cavity was built for and every cut may take a piece of its surface away.
                range.param = glm::vec4(static_cast<float>((1u << pendingCavity->woundCount) - 1u),
                                        static_cast<float>((1u << goreState.cutCount) - 1u), 0.0f, 0.0f);
                vc.IndexRanges.assign(1, range);
                vc.gore = &cavityGpu;
            }
            else
            {
                vc.IndexRanges.clear();
            }

            vc.pass = GoreDrawCommand::Pass::Anatomy;
            vc.cavity = true;
            vc.layer = static_cast<float>(kAnatomyLayer);
            vc.insetFraction = 0.0f;
            vc.RangeParamUniform = "u_goreRangeMask";
            vc.Cull = StaticMeshDrawCommand::CullFace::None;
            vc.TwoSided = true;
            vc.DepthPrePass = true;
            vc.Hidden = false;
            vc.Material.SurfaceTypeOverride = SurfaceType::Opaque;
            vc.Material.Masked = false;
            vc.Material.CustomId = 0;
            vc.OnlyShadows = false;
            vc.ReceiveDetailShadows = false;
            vc.ShadowCapVbh = BGFX_INVALID_HANDLE;
            vc.ShadowCapIbh = BGFX_INVALID_HANDLE;
            vc.ShadowCapIndexCount = 0;
            vc.ShadowEdgeVbh = BGFX_INVALID_HANDLE;
            vc.ShadowEdgeIbh = BGFX_INVALID_HANDLE;
            vc.ShadowEdgeIndexCount = 0;

            cavityHeld = pendingCavity;   // the previous one (if any) is released here, after this frame's command was rewired
        }
        finalCavityCommand = cavityCommand;

        for (int i = 0; i < kMaxCuts; ++i)
        {
            if (!capCommands[i]) continue;
            GoreDrawCommand& cc = *capCommands[i];

            std::shared_ptr<const CapGeometry> geometry;
            if (pendingActive && count > 0 && i < goreState.cutCount && goreState.cuts[i].cap
                && goreState.cuts[i].cap->GetState() == CapGeometry::State::Ready)
                geometry = goreState.cuts[i].cap;
            if (geometry) geometry->EnsureGpu();   // main thread

            const uint32_t first = geometry ? (goreState.isGib ? geometry->gibFirst : geometry->bodyFirst) : 0;
            const uint32_t indexCount = geometry ? (goreState.isGib ? geometry->gibCount : geometry->bodyCount) : 0;

            cc.goreActive = geometry && bgfx::isValid(geometry->vbh) && bgfx::isValid(geometry->ibh) && indexCount > 0;
            if (cc.goreActive)
            {
                cc.CopyCommonFrom(static_cast<GoreDrawCommand&>(GetDrawCommandAt(0)));   // transform, light, palette
                cc.VertexBuffer = geometry->vbh;
                cc.IndexBuffer = geometry->ibh;
                DrawIndexRange range;
                range.first = first;
                range.count = indexCount;
                range.param = glm::vec4(0.0f);      // mask 0: a cap is never clipped
                cc.IndexRanges.assign(1, range);
            }
            else
            {
                cc.IndexRanges.clear();
            }

            cc.pass = GoreDrawCommand::Pass::Anatomy;
            cc.cap = true;
            cc.gore = &finalGpu;
            cc.layer = static_cast<float>(kAnatomyLayer);
            cc.insetFraction = 0.0f;
            cc.RangeParamUniform = "u_goreRangeMask";
            cc.Cull = StaticMeshDrawCommand::CullFace::None;
            cc.TwoSided = true;
            cc.DepthPrePass = true;
            cc.Hidden = false;
            cc.Material.SurfaceTypeOverride = SurfaceType::Opaque;
            cc.Material.Masked = false;
            cc.Material.CustomId = 0;
            cc.OnlyShadows = false;
            cc.ReceiveDetailShadows = false;
            cc.ShadowCapVbh = BGFX_INVALID_HANDLE;
            cc.ShadowCapIbh = BGFX_INVALID_HANDLE;
            cc.ShadowCapIndexCount = 0;
            cc.ShadowEdgeVbh = BGFX_INVALID_HANDLE;
            cc.ShadowEdgeIbh = BGFX_INVALID_HANDLE;
            cc.ShadowEdgeIndexCount = 0;

            capHeld[i] = geometry;   // the previous one (if any) is released here, after this frame's command was rewired
        }
        finalCapCommands = capCommands;
        finalShellCommands = shellCommands;
        finalAnatomyCommand = anatomyCommand;
    }

    void GoreSkeletalMesh::CollectDrawCommands(std::vector<IDrawCommand*>& out)
    {
        SkeletalMesh::CollectDrawCommands(out);

        for (auto& sc : finalShellCommands)
            if (sc->goreActive && !sc->Hidden)
                out.push_back(sc.get());

        if (finalAnatomyCommand && finalAnatomyCommand->goreActive)
            out.push_back(finalAnatomyCommand.get());

        for (auto& cc : finalCapCommands)
            if (cc && cc->goreActive)
                out.push_back(cc.get());

        if (finalCavityCommand && finalCavityCommand->goreActive)
            out.push_back(finalCavityCommand.get());
    }
}
