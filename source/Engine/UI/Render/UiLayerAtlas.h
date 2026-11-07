#pragma once
#include <UI/Render/UiRectAllocator.h>
#include <bgfx/bgfx.h>
#include <glm.h>
#include <cstdint>

// Offscreen colour-only pages for UI layers (README constraint 5: 2D colour only; phase 8 scene views have a pool of their own).
// Page 0 keeps cached layers; pages 1..3 are scratch and are emptied every frame. A pass never reads and writes the same
// page (D3D11 unbinds a texture that is still a render target), so callers name the pages they read when they allocate.
namespace UiLayerAtlas
{
    constexpr int kPageSize = 4096;
    constexpr int kMaxPages = 4;
    constexpr int kCachePage = 0;

    struct Region
    {
        int Page = -1;
        UiAtlasRect Rect;
        bool Valid() const { return Page >= 0 && Rect.Valid(); }
    };

    struct CacheEntry
    {
        Region R;
        uint64_t Hash = 0;
        double LastUsed = 0.0;
        int Frame = -1;          // frame in which the entry was last asked for
    };

    void Shutdown();
    // Empties the scratch pages and frees cached regions that went unused for three seconds.
    void BeginFrame();

    bool AllocScratch(int width, int height, uint32_t excludedPages, Region& out);
    void FreeScratch(const Region& region);

    // Cached regions are looked up by the layer's id. Allocating again for the same id frees the old region.
    CacheEntry* FindCached(uint64_t id);
    bool AllocCached(uint64_t id, int width, int height, Region& out);
    int CachedCount();

    bgfx::TextureHandle Texture(int page);
    bgfx::FrameBufferHandle FrameBuffer(int page);
    // u0, v0, u1, v1 of the region as sampled: v0 is the region's top row (OpenGL stores render targets bottom-up, so v is mirrored there).
    glm::vec4 UvRect(const Region& region);
    // The same rectangle as (min u, min v, max u, max v), pulled in by half a texel: the clamp range of the effect shaders.
    glm::vec4 ClampRect(const Region& region);
    uint32_t PageBit(int page);
}
