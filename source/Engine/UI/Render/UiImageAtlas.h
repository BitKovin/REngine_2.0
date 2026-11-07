#pragma once
#include <glm.h>
#include <bgfx/bgfx.h>
#include <string>

struct UiAtlasImage
{
    bgfx::TextureHandle Texture = BGFX_INVALID_HANDLE;
    glm::vec4 Uv = glm::vec4(0.f, 0.f, 1.f, 1.f);   // u0, v0, u1, v1 of the image inside Texture
    int Width = 0, Height = 0;                      // image size in texels
    int TextureWidth = 0, TextureHeight = 0;        // size of Texture (for pixelated sampling)
    bool Valid = false;                             // Texture and Uv can be used
    bool Pending = false;                           // not loaded yet; ask again next frame (the caller marks itself paint-dirty)
};

// Runtime atlas for images up to 512 px: skyline packing into 2048 x 2048 RGBA8 pages (at most 4), 4 px of edge-replicated padding.
// Larger images, and images that no longer fit, are standalone textures from the AssetRegistry. Main thread only (R1).
namespace UiImageAtlas
{
    // Decodes and packs the image on first use (cached afterwards, also when it failed).
    // An .svg is rasterised at the smallest power-of-two size (16 .. 512 texels, longer side) that covers `displayLongSidePx`, the size it
    // is drawn at in screen pixels; each size is its own cached entry. Width/Height stay the drawing's own size, so layout does not change
    // with the raster. Pass 0 for the drawing's own size.
    UiAtlasImage Get(const std::string& path, float displayLongSidePx = 0.f);
    // Uploads the new image regions and rebuilds the affected mip levels. Called at the end of UiSystem::Update.
    void Update();
    void Shutdown();
    int PageCount();
}
