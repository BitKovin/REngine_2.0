#pragma once

#include <Renderer/Abstractions/FrameBuffer.h>
#include <Renderer/Abstractions/RenderTexture.h>

#include <glm/glm.hpp>

// Render targets of scene views (phase 8): colour RGBA8 + depth D24S8. Separate from the phase 7 layer atlas.
// Keyed by size: a view keeps its target while its size stays and it is used; a target nobody asked for in 3 seconds is freed.
// MSAA is off (the target is sampled by the UI as it is).
struct UiSceneTarget
{
    RenderTexture* Color = nullptr;
    RenderTexture* Depth = nullptr;
    Framebuffer* Buffer = nullptr;
    glm::ivec2 Size = glm::ivec2(0);
};

namespace UiSceneTargets
{
    // The target of `owner` with this size. Creates one (or takes a pooled one of that size) when the owner has none or the size changed.
    UiSceneTarget* Request(const void* owner, glm::ivec2 size);
    void Release(const void* owner);   // the owner is gone: its target goes back to the pool
    void EndFrame();                   // frees what was not used for 3 s
    int Count();                       // live targets (owned + pooled), for ui_stats and tests
}
