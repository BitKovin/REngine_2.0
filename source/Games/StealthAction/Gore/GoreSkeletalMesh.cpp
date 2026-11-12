#include "GoreSkeletalMesh.h"
#include "GoreProfile.h"
#include "GoreSettings.h"

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
    }

    void GoreSkeletalMesh::SetGoreState(const RenderState& state)
    {
        if (state.version == goreState.version && state.profile == goreState.profile)
            return;

        goreState = state;
    }

    void GoreSkeletalMesh::ClearGoreState()
    {
        goreState = RenderState();
    }

    void GoreSkeletalMesh::OnAssetsReloaded()
    {
        SkeletalMesh::OnAssetsReloaded();

        // One interior command per submesh. finalInteriorCommands is not touched: the renderer may still hold the old ones.
        interiorCommands.clear();
        for (size_t i = 0; i < GetSubMeshCount(); ++i)
        {
            auto ic = std::make_shared<GoreDrawCommand>();
            ic->pass = GoreDrawCommand::Pass::Interior;
            interiorCommands.push_back(ic);
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
    }

    // Index ranges per submesh: one range per run of bones with the same primitive mask.
    void GoreSkeletalMesh::BuildRanges()
    {
        const Profile& profile = *goreState.profile;

        pendingRanges.resize(model->meshes.size());

        for (size_t m = 0; m < model->meshes.size(); ++m)
        {
            SubmeshRanges& ranges = pendingRanges[m];
            ranges.front.clear();
            ranges.interior.clear();
            ranges.shadowCap.clear();
            ranges.shadowEdge.clear();

            const roj::SkinnedMesh& mesh = model->meshes[m];
            if (mesh.boneIndexFirst.empty() || mesh.boneOwnBounds.size() + 1 != mesh.boneIndexFirst.size())
                continue;

            const SubmeshRole role = m < profile.submeshRoles.size() ? profile.submeshRoles[m] : SubmeshRole::Volume;
            if (role == SubmeshRole::Attachment)
                continue;

            const size_t n = mesh.boneIndexFirst.size() - 1;

            // Bind-space AABBs of the wounds, grown by edge noise and rim.
            std::array<glm::vec3, kMaxWounds> woundMin, woundMax;
            for (int i = 0; i < goreState.woundCount; ++i)
            {
                const Wound& w = goreState.wounds[i];
                const glm::vec3 e = EllipsoidHalfExtent(w.rotation, w.radii) * (1.0f + w.roughness + w.rimWidth);
                woundMin[i] = w.center - e;
                woundMax[i] = w.center + e;
            }

            // Bones inside a severed subtree are not drawn (bodies only; gib handling is phase 3).
            std::vector<uint8_t> removed;
            if (!goreState.isGib && goreState.cutCount > 0)
            {
                removed.assign(n, 0);
                for (int i = 0; i < goreState.cutCount; ++i)
                {
                    const Cut& c = goreState.cuts[i];
                    if (c.boneSkel >= profile.subtreeEnd.size()) continue;
                    for (size_t b = c.boneSkel; b < profile.subtreeEnd[c.boneSkel] && b < n; ++b)
                        removed[b] = 1;
                }
            }

            for (size_t b = 0; b < n; ++b)
            {
                const uint32_t first = mesh.boneIndexFirst[b];
                const uint32_t count = mesh.boneIndexFirst[b + 1] - first;
                if (count == 0) continue;
                if (!removed.empty() && removed[b]) continue;

                const roj::BoneBound& own = mesh.boneOwnBounds[b];

                float woundBits = 0.0f, cutBits = 0.0f, splatBits = 0.0f;

                if (own.hasData)
                {
                    for (int i = 0; i < goreState.woundCount; ++i)
                        if (AabbOverlap(woundMin[i], woundMax[i], own.bbMin, own.bbMax))
                            woundBits += Bit(i);

                    for (int i = 0; i < goreState.splatCount; ++i)
                    {
                        const Splat& s = goreState.splats[i];
                        if (SphereOverlapsAabb(s.center, s.radius, own.bbMin, own.bbMax))
                            splatBits += Bit(i);
                    }

                    for (int i = 0; i < goreState.cutCount; ++i)
                    {
                        const Cut& c = goreState.cuts[i];
                        if (!SphereOverlapsAabb(c.center, c.influenceRadius, own.bbMin, own.bbMax))
                            continue;

                        float maxDist = -1.0e9f;
                        for (int corner = 0; corner < 8; ++corner)
                        {
                            const glm::vec3 p((corner & 1) ? own.bbMax.x : own.bbMin.x,
                                              (corner & 2) ? own.bbMax.y : own.bbMin.y,
                                              (corner & 4) ? own.bbMax.z : own.bbMin.z);
                            maxDist = std::max(maxDist, glm::dot(glm::vec3(c.plane), p) - c.plane.w);
                        }
                        // A gib keeps the removed side, so it needs the opposite test.
                        if (goreState.isGib) maxDist = 1.0f;
                        if (maxDist > -0.01f)
                            cutBits += Bit(i);
                    }
                }

                const glm::vec4 mask(woundBits, cutBits, splatBits, 0.0f);

                auto append = [&](std::vector<DrawIndexRange>& list)
                {
                    if (!list.empty() && list.back().first + list.back().count == first && list.back().param == mask)
                    {
                        list.back().count += count;
                        return;
                    }
                    DrawIndexRange r;
                    r.first = first;
                    r.count = count;
                    r.param = mask;
                    list.push_back(r);
                };

                append(ranges.front);

                if (woundBits > 0.0f || cutBits > 0.0f)
                    append(ranges.interior);
            }
        }
    }

    void GoreSkeletalMesh::FillGpuData()
    {
        const Profile& profile = *goreState.profile;
        const std::vector<mat4>& palette = GetSkinPalette();
        const mat4 World = finalizedWorld;
        const double now = Time::GameTime;
        const double clock = RegenClock(goreState.frozenClock, goreState.pausedUntil, now);

        auto bindToWorld = [&](int skin) -> mat4
        {
            if (skin < 0 || skin >= static_cast<int>(palette.size())) return World;
            return World * palette[skin];
        };

        GpuData& g = pendingGpu;

        g.counts = glm::vec4(goreState.woundCount, goreState.cutCount, goreState.splatCount, 0.0f);
        g.params = glm::vec4(bgfx::getCaps()->homogeneousDepth ? 1.0f : 0.0f,
                             goreState.isGib ? 1.0f : 0.0f,
                             profile.fleshScale,
                             static_cast<float>(GoreSettings::Get().level));
        g.bloodColor = glm::vec4(profile.bloodColor, 1.0f);
        g.flesh = profile.flesh;
        g.fleshNormal = profile.fleshNormal;
        g.edgeNoise = profile.edgeNoise;
        g.blood = profile.blood;

        for (int i = 0; i < goreState.woundCount; ++i)
        {
            const Wound& w = goreState.wounds[i];
            const mat4 M = bindToWorld(w.anchorSkin);
            const float s = glm::length(vec3(M[0]));

            const mat4 Mb = glm::translate(mat4(1.0f), w.center) * glm::mat4_cast(w.rotation) * glm::scale(mat4(1.0f), w.radii);

            StoreRows(g.woundBind, i, glm::inverse(Mb));
            StoreRows(g.woundWorld, i, glm::inverse(M * Mb));

            float healScale = 1.0f;
            if (w.healDuration > 0.0f)
            {
                const float t = glm::clamp(static_cast<float>((clock - w.healStart) / w.healDuration), 0.0f, 1.0f);
                healScale = 1.0f - t * t * (3.0f - 2.0f * t);
            }

            g.woundParams[i] = glm::vec4(w.roughness, w.rimWidth, healScale, w.seed);
            g.woundExtra[i] = glm::vec4(w.reach * s, MeanRadius(w.radii) * s, static_cast<float>(w.kind), 0.0f);
        }

        for (int i = 0; i < goreState.cutCount; ++i)
        {
            const Cut& c = goreState.cuts[i];
            const mat4 M = bindToWorld(c.boneSkin);
            const float s = glm::length(vec3(M[0]));

            const glm::vec3 nBind = glm::vec3(c.plane);
            const glm::vec3 nWorld = glm::normalize(glm::transpose(glm::inverse(glm::mat3(M))) * nBind);
            const glm::vec3 pointWorld = glm::vec3(M * glm::vec4(nBind * c.plane.w, 1.0f));
            const glm::vec3 centerWorld = glm::vec3(M * glm::vec4(c.center, 1.0f));

            g.cutBind[i] = c.plane;
            g.cutBindSphere[i] = glm::vec4(c.center, c.influenceRadius);
            g.cutWorld[i] = glm::vec4(nWorld, glm::dot(nWorld, pointWorld));
            g.cutWorldSphere[i] = glm::vec4(centerWorld, c.influenceRadius * s);
            g.cutCenter[i] = glm::vec4(centerWorld, 0.0f);

            float regrowRadius = -1.0f;
            if (c.phase == CutPhase::Regrowing && c.regrowDuration > 0.0f)
            {
                const float t = glm::clamp(static_cast<float>((clock - c.regrowStart) / c.regrowDuration), 0.0f, 1.0f);
                regrowRadius = std::max(0.0001f, c.regrowMaxRadius * t);
            }
            g.cutRegrowBind[i] = glm::vec4(c.center, regrowRadius);
            g.cutRegrowWorld[i] = glm::vec4(centerWorld, regrowRadius > 0.0f ? regrowRadius * s : -1.0f);

            const float flags = (c.charred ? 1.0f : 0.0f) + (c.phase == CutPhase::Regrowing ? 2.0f : 0.0f);
            g.cutParams[i] = glm::vec4(c.roughness, c.seed, c.boneRadius, flags);

            const glm::vec3 up = std::abs(nWorld.y) < 0.99f ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);
            const glm::vec3 T = glm::normalize(glm::cross(nWorld, up));
            const glm::vec3 B = glm::cross(nWorld, T);
            const float reach = 2.0f * MaxExtent(profile, c.boneSkel);
            g.cutFrame[i * 2 + 0] = glm::vec4(T, c.boneRadius * s);
            g.cutFrame[i * 2 + 1] = glm::vec4(B, reach * s);
        }

        for (int i = 0; i < goreState.splatCount; ++i)
        {
            const Splat& sp = goreState.splats[i];
            g.splats[i] = glm::vec4(sp.center, sp.radius * sp.strength);
        }

        pendingUsePalette = false;   // phase 3 builds a modified palette here
    }

    void GoreSkeletalMesh::FinalizeFrameData()
    {
        SkeletalMesh::FinalizeFrameData();

        if (pendingActive)
            finalGpu = pendingGpu;

        if (pendingUsePalette)
        {
            renderPalette = pendingPalette;
        }
        usingRenderPalette = pendingUsePalette;

        const Profile* profile = goreState.profile;
        const size_t count = std::min(GetSubMeshCount(), interiorCommands.size());

        for (size_t i = 0; i < count; ++i)
        {
            GoreDrawCommand& cmd = static_cast<GoreDrawCommand&>(GetDrawCommandAt(i));
            const SubmeshRole role = (profile && i < profile->submeshRoles.size()) ? profile->submeshRoles[i] : SubmeshRole::Volume;
            const bool hasRanges = pendingActive && i < pendingRanges.size();

            cmd.BoneMatrices = GetRenderPalette();
            cmd.gore = &finalGpu;
            cmd.goreActive = pendingActive && role != SubmeshRole::Attachment && hasRanges;

            if (cmd.goreActive)
            {
                cmd.IndexRanges.assign(pendingRanges[i].front.begin(), pendingRanges[i].front.end());
                cmd.ShadowCapRanges.assign(pendingRanges[i].shadowCap.begin(), pendingRanges[i].shadowCap.end());
                cmd.ShadowEdgeRanges.assign(pendingRanges[i].shadowEdge.begin(), pendingRanges[i].shadowEdge.end());
                cmd.RangeParamUniform = "u_goreRangeMask";
            }
            else
            {
                cmd.IndexRanges.clear();
                cmd.ShadowCapRanges.clear();
                cmd.ShadowEdgeRanges.clear();
                cmd.RangeParamUniform = nullptr;
            }

            GoreDrawCommand& ic = *interiorCommands[i];
            ic.CopyCommonFrom(cmd);
            ic.goreActive = cmd.goreActive && !pendingRanges[i].interior.empty();
            ic.shell = role == SubmeshRole::Shell;

            if (ic.goreActive)
                ic.IndexRanges.assign(pendingRanges[i].interior.begin(), pendingRanges[i].interior.end());
            else
                ic.IndexRanges.clear();

            ic.Cull = StaticMeshDrawCommand::CullFace::Front;
            ic.RangeParamUniform = "u_goreRangeMask";
            ic.Material.CustomId = 0;
            ic.OnlyShadows = false;
            ic.ReceiveDetailShadows = false;
            ic.ShadowCapVbh = BGFX_INVALID_HANDLE;
            ic.ShadowCapIbh = BGFX_INVALID_HANDLE;
            ic.ShadowCapIndexCount = 0;
            ic.ShadowEdgeVbh = BGFX_INVALID_HANDLE;
            ic.ShadowEdgeIbh = BGFX_INVALID_HANDLE;
            ic.ShadowEdgeIndexCount = 0;
        }

        finalInteriorCommands = interiorCommands;
    }

    void GoreSkeletalMesh::CollectDrawCommands(std::vector<IDrawCommand*>& out)
    {
        SkeletalMesh::CollectDrawCommands(out);

        for (auto& ic : finalInteriorCommands)
            if (ic->goreActive && !ic->Hidden)
                out.push_back(ic.get());
    }
}
