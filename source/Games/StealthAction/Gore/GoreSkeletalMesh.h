#pragma once

#include "GoreDrawCommands.h"
#include "GoreRenderState.h"

#include <SkeletalMesh.hpp>

#include <bgfx/bgfx.h>
#include <memory>
#include <vector>

namespace Gore
{
    // A SkeletalMesh that can draw wounds (and, in phase 3, cuts). Without SetGoreState it is a plain SkeletalMesh.
    class GoreSkeletalMesh : public SkeletalMesh
    {
    public:
        explicit GoreSkeletalMesh(Entity* owner) : SkeletalMesh(owner) {}

        // Game thread only (gore tick). Copies when state.version differs from the current one.
        void SetGoreState(const RenderState& state);
        void ClearGoreState();   // back to a plain SkeletalMesh
        uint32_t GetGoreVersion() const { return goreState.version; }

        bool HasGore() const
        {
            return goreState.version != 0 && (goreState.woundCount || goreState.cutCount || goreState.splatCount);
        }

        void PreFinalize() override;
        void FinalizeFrameData() override;
        void CollectDrawCommands(std::vector<IDrawCommand*>& out) override;

    ~GoreSkeletalMesh() override;

    protected:
        std::shared_ptr<StaticMeshDrawCommand> CreateDrawCommand() override { return std::make_shared<GoreDrawCommand>(); }
        void OnAssetsReloaded() override;
        const std::vector<mat4>* GetRenderPalette() const override;

    private:
        RenderState goreState;          // written by SetGoreState (game thread), read in PreFinalize
        uint32_t builtVersion = 0;      // goreState version the ranges were built for
        const Profile* builtProfile = nullptr;

        GpuData pendingGpu, finalGpu;   // PreFinalize writes pending; FinalizeFrameData copies to final
        std::vector<mat4> pendingPalette, renderPalette;
        bool usingRenderPalette = false, pendingUsePalette = false;

        // Per submesh, rebuilt when builtVersion changes:
        struct SubmeshRanges
        {
            std::vector<DrawIndexRange> front, interior, shadowCap, shadowEdge;
        };
        std::vector<SubmeshRanges> pendingRanges;

        std::vector<std::shared_ptr<GoreDrawCommand>> interiorCommands, finalInteriorCommands;
        bool pendingActive = false;

        // Wound cavities: one skinned bowl per wound (see CavityGeometry). The list is filled in PreFinalize (game side)
        // and turned into GPU buffers and draw commands in FinalizeFrameData.
        std::vector<std::shared_ptr<const CavityGeometry>> pendingCavities, finalCavities;
        struct CavityGpu
        {
            std::shared_ptr<const CavityGeometry> geometry;
            bgfx::VertexBufferHandle vbh = BGFX_INVALID_HANDLE;
            bgfx::IndexBufferHandle ibh = BGFX_INVALID_HANDLE;
            std::shared_ptr<GoreDrawCommand> command;
            int idleFrames = 0;
        };
        std::vector<CavityGpu> cavityGpu;
        std::vector<std::shared_ptr<GoreDrawCommand>> visibleCavityCommands;
        void UpdateCavities();

        void BuildRanges();
        void FillGpuData();
        void BuildRenderPalette(const std::vector<mat4>& palette);
    };
}
