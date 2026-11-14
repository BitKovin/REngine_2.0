#pragma once

#include "GoreTypes.h"

#include <VertexData.h>
#include <skinned_model.hpp>
#include <bgfx/bgfx.h>

#include <atomic>
#include <memory>
#include <vector>

namespace Gore
{
    struct Profile;

    // The flat face of a blade cut: filled discs (skin, three flesh layers, bone, marrow, organs) on the cut plane, skinned like
    // the stump. Built once per cut on the worker thread (BuildPool); immutable once `state` is Ready; shared by the body and
    // the gib.
    struct CapGeometry
    {
        enum class State : uint8_t { Building, Ready, Failed };   // Failed: the plane found no skin near the cut centre

        State GetState() const { return state.load(std::memory_order_acquire); }
        std::atomic<State> state{ State::Building };   // written last by the worker, read by the game thread

        std::vector<VertexData> vertices;
        std::vector<uint32_t> indices;
        uint32_t bodyFirst = 0, bodyCount = 0;   // index range the body draws (faces the removed side)
        uint32_t gibFirst = 0, gibCount = 0;     // index range the gib draws (faces the other way)

        // GPU side. Created on the main thread (EnsureGpu, from FinalizeFrameData), freed with the object.
        mutable bgfx::VertexBufferHandle vbh = BGFX_INVALID_HANDLE;
        mutable bgfx::IndexBufferHandle  ibh = BGFX_INVALID_HANDLE;
        mutable bool gpuReady = false;

        void EnsureGpu() const;
        ~CapGeometry();
    };

    // Starts building the cap on the worker thread and returns it at once, in state Building. The model and the profile must
    // outlive the job (ClearProfiles waits for it). A Failed cap tells the caller to make a torn cut instead.
    std::shared_ptr<const CapGeometry> RequestCap(const roj::SkinnedModel& model, const Profile& profile, const Cut& cut);
}
