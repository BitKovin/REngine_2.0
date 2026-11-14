#pragma once

#include "GoreTypes.h"

#include <VertexData.h>
#include <skinned_model.hpp>
#include <bgfx/bgfx.h>

#include <array>
#include <atomic>
#include <memory>
#include <vector>

namespace Gore
{
    struct Profile;

    // The closing surface of the holes in a body. A wound is an ellipsoid that is taken out of the body (a boolean subtraction):
    // the skin and the flesh layers inside it are discarded by the shaders, and this is what is left of the ellipsoid, its inner
    // surface where it lies inside the body. Each vertex follows the bones of the body triangle nearest to it, so the surface
    // moves with the animation and the ragdoll. Where two wounds overlap, the shader removes the part of each surface that lies
    // inside the other one (it uses the `wounds` snapshot below), so only the outline of their union is left.
    //
    // Built on the worker thread (BuildPool); immutable once `state` is Ready.
    struct CavityGeometry
    {
        enum class State : uint8_t { Building, Ready };

        State GetState() const { return state.load(std::memory_order_acquire); }
        std::atomic<State> state{ State::Building };

        std::vector<VertexData> vertices;   // bind space. Color.r = index into `wounds`, Color.g = depth under the skin (0..1)
        std::vector<uint32_t> indices;

        // The wounds this geometry was made from: the surface of one is cut away by all the others.
        std::array<Wound, kMaxWounds> wounds;
        uint8_t woundCount = 0;

        // GPU side. Created on the main thread (EnsureGpu, from FinalizeFrameData), freed with the object.
        mutable bgfx::VertexBufferHandle vbh = BGFX_INVALID_HANDLE;
        mutable bgfx::IndexBufferHandle  ibh = BGFX_INVALID_HANDLE;
        mutable bool gpuReady = false;

        void EnsureGpu() const;
        ~CavityGeometry();
    };

    // Does this wound leave a hole with an inside (a shot, a blast, a gash)? A sever wound is closed by the torn stump instead,
    // a wound that heals shrinks and would leave its surface behind.
    bool HasCavity(const Wound& wound);

    // True when the two lists describe the same holes (the cavity geometry stays valid).
    bool SameWounds(const std::array<Wound, kMaxWounds>& a, int countA, const std::array<Wound, kMaxWounds>& b, int countB);

    // Starts building the cavities of `wounds` on the worker thread and returns at once, in state Building. The job holds only the
    // body grid of the profile (shared, immutable, with its own bone weights) and a copy of the wounds, so the model may be
    // released or replaced while it runs. Null when the model has no body to test against.
    std::shared_ptr<const CavityGeometry> RequestCavity(const Profile& profile, const std::array<Wound, kMaxWounds>& wounds, int count);
}
