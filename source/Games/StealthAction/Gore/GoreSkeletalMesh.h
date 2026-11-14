#pragma once

#include "GoreCavity.h"
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
            std::vector<DrawIndexRange> front, shell, shadowCap, shadowEdge;
            bool hasHole = false;   // a wound or a cut opens this submesh somewhere: its skin is drawn two-sided
        };
        std::vector<SubmeshRanges> pendingRanges;

        std::vector<DrawIndexRange> pendingAnatomyRanges;

        // The closing surface of the wounds (GoreCavity.h). It is rebuilt on the worker thread when the wounds change: the old one
        // stays on screen until the new one is ready. cavityWounds is what the newest request was made for.
        std::shared_ptr<const CavityGeometry> cavityShown, cavityBuilding, pendingCavity, cavityHeld;
        std::array<Wound, kMaxWounds> cavityWounds;
        int cavityWoundCount = -1;
        GpuData pendingCavityGpu, cavityGpu;   // the same as pendingGpu / finalGpu, with the wounds the cavity was built from
        std::shared_ptr<GoreDrawCommand> cavityCommand, finalCavityCommand;
        void RefreshCavity();

        // kShellLayers commands per submesh: index = submesh * kShellLayers + layer. The final* copies are what the
        // renderer may still hold.
        std::vector<std::shared_ptr<GoreDrawCommand>> shellCommands, finalShellCommands;
        std::shared_ptr<GoreDrawCommand> anatomyCommand, finalAnatomyCommand;
        // One command per cut for the flat face of a blade cut. capHeld keeps the geometry (and its GPU buffers) alive
        // for as long as a command may still point at it.
        std::array<std::shared_ptr<GoreDrawCommand>, kMaxCuts> capCommands, finalCapCommands;
        std::array<std::shared_ptr<const CapGeometry>, kMaxCuts> capHeld;
        bool pendingActive = false;

        void BuildRanges();
        void FillGpuData();
        void BuildRenderPalette(const std::vector<mat4>& palette);
    };
}
