#include "UiBilboard.h"

#include <BgfxStateManager.h>
#include <bgfx/bgfx.h>

// ---------------------------------------------------------------------------
// Atlas implementation
// ---------------------------------------------------------------------------
UiBilboardAtlas::UiBilboardAtlas(ivec2 size)
{
    texture = new RenderTexture(
        size.x,
        size.y,
        TextureFormat::RGBA8,
        TextureType::Texture2D,
        false,
        BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP |
        BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT
    );

    texture->SetName("UiBillboardAtlas " + to_string(size.x) + "x" + to_string(size.y));

    allocator.Init(size.x, size.y);
}

UiBilboardAtlas::~UiBilboardAtlas()
{
    delete texture;
}

bool UiBilboardAtlas::TryAllocate(ivec2 reqSize, BillboardAllocation& outAlloc)
{
    UiAtlasRect rect;
    if (!allocator.Allocate(reqSize.x, reqSize.y, rect))
        return false;

    const int x = rect.X, y = rect.Y, w = rect.W, h = rect.H;
    outAlloc.atlas = this;
    outAlloc.rect = ivec4(x, y, w, h);

    float tw = (float)texture->width();
    float th = (float)texture->height();

    float minU = (float)x / tw;
    float maxU = (float)(x + w) / tw;
    float minV, maxV;

    const bgfx::RendererType::Enum renderer = bgfx::getRendererType();
    if (renderer == bgfx::RendererType::OpenGL ||
        renderer == bgfx::RendererType::OpenGLES)
    {
        // bgfx translates setViewRect into glViewport(x, atlasH-y-h, w, h),
        // so rendered pixels land at the complementary Y in the texture.
        minV = 1.0f - (float)(y + h) / th;
        maxV = 1.0f - (float)y / th;
    }
    else
    {
        // Non-GL render-to-texture writes content Y-flipped relative to
        // what the UI renderer emits; read the slot upside-down to cancel it.
        minV = (float)(y + h) / th;
        maxV = (float)y / th;
    }

    outAlloc.uvRect = vec4(minU, minV, maxU, maxV);
    return true;
}

void UiBilboardAtlas::Free(const BillboardAllocation& alloc)
{
    if (alloc.atlas != this || !alloc.IsValid()) return;
    allocator.Free({ alloc.rect.x, alloc.rect.y, alloc.rect.z, alloc.rect.w });
}

// ---------------------------------------------------------------------------
UiBillboardAtlasManager& UiBillboardAtlasManager::Get()
{
    static UiBillboardAtlasManager instance;
    return instance;
}

BillboardAllocation UiBillboardAtlasManager::Allocate(ivec2 size)
{
    // try existing atlases
    for (auto& atlas : atlases)
    {
        BillboardAllocation alloc;
        if (atlas->TryAllocate(size, alloc))
            return alloc;
    }

    // no room → create a new atlas
    auto newAtlas = std::make_unique<UiBilboardAtlas>(kAtlasSize);
    atlases.push_back(std::move(newAtlas));

    BillboardAllocation alloc;
    if (atlases.back()->TryAllocate(size, alloc))
        return alloc;

    // should never happen (billboard size is guaranteed ≤ atlas size)
    return {};
}

void UiBillboardAtlasManager::Free(BillboardAllocation& alloc)
{
    if (!alloc.IsValid()) return;
    alloc.atlas->Free(alloc);
    alloc.atlas = nullptr; // invalidate
}
