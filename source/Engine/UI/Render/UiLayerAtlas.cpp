#include "UiLayerAtlas.h"

#include <Logger.hpp>

#include <algorithm>
#include <chrono>
#include <unordered_map>

namespace
{
    struct Page
    {
        bgfx::TextureHandle Texture = BGFX_INVALID_HANDLE;
        bgfx::FrameBufferHandle FrameBuffer = BGFX_INVALID_HANDLE;
        UiRectAllocator Allocator;
        bool Created = false;
    };

    Page s_pages[UiLayerAtlas::kMaxPages];
    std::unordered_map<uint64_t, UiLayerAtlas::CacheEntry> s_cache;
    int s_frame = 0;
    bool s_loggedCreateFailure = false;

    double Now()
    {
        return std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count();
    }

    bool EnsurePage(int index)
    {
        Page& page = s_pages[index];
        if (page.Created)
            return true;
        const uint64_t flags = BGFX_TEXTURE_RT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;
        page.Texture = bgfx::createTexture2D(UiLayerAtlas::kPageSize, UiLayerAtlas::kPageSize, false, 1, bgfx::TextureFormat::RGBA8, flags);
        if (bgfx::isValid(page.Texture))
        {
            bgfx::setName(page.Texture, "UiLayerAtlas");
            page.FrameBuffer = bgfx::createFrameBuffer(1, &page.Texture, false);
        }
        if (!bgfx::isValid(page.Texture) || !bgfx::isValid(page.FrameBuffer))
        {
            if (!s_loggedCreateFailure)
                Logger::Error("[UiRenderer] the layer atlas page could not be created; layers draw without effects");
            s_loggedCreateFailure = true;
            if (bgfx::isValid(page.Texture))
                bgfx::destroy(page.Texture);
            page.Texture = BGFX_INVALID_HANDLE;
            return false;
        }
        page.Allocator.Init(UiLayerAtlas::kPageSize, UiLayerAtlas::kPageSize);
        page.Created = true;
        return true;
    }
}

namespace UiLayerAtlas
{
    void Shutdown()
    {
        for (Page& page : s_pages)
        {
            if (page.Created)
            {
                bgfx::destroy(page.FrameBuffer);
                bgfx::destroy(page.Texture);
            }
            page = Page();
        }
        s_cache.clear();
    }

    uint32_t PageBit(int page) { return page >= 0 ? (1u << page) : 0u; }

    void BeginFrame()
    {
        ++s_frame;
        for (int i = 1; i < kMaxPages; ++i)
            if (s_pages[i].Created)
                s_pages[i].Allocator.Reset();
        const double now = Now();
        for (auto it = s_cache.begin(); it != s_cache.end();)
        {
            if (now - it->second.LastUsed > 3.0)
            {
                s_pages[kCachePage].Allocator.Free(it->second.R.Rect);
                it = s_cache.erase(it);
            }
            else
                ++it;
        }
    }

    bool AllocScratch(int width, int height, uint32_t excludedPages, Region& out)
    {
        for (int i = 1; i < kMaxPages; ++i)
        {
            if ((excludedPages & PageBit(i)) != 0 || !EnsurePage(i))
                continue;
            UiAtlasRect rect;
            if (s_pages[i].Allocator.Allocate(width, height, rect))
            {
                out.Page = i;
                out.Rect = rect;
                return true;
            }
        }
        return false;
    }

    void FreeScratch(const Region& region)
    {
        if (region.Valid() && region.Page != kCachePage)
            s_pages[region.Page].Allocator.Free(region.Rect);
    }

    CacheEntry* FindCached(uint64_t id)
    {
        auto it = s_cache.find(id);
        if (it == s_cache.end())
            return nullptr;
        it->second.LastUsed = Now();
        it->second.Frame = s_frame;
        return &it->second;
    }

    bool AllocCached(uint64_t id, int width, int height, Region& out)
    {
        if (!EnsurePage(kCachePage))
            return false;
        auto it = s_cache.find(id);
        if (it != s_cache.end())
        {
            s_pages[kCachePage].Allocator.Free(it->second.R.Rect);
            s_cache.erase(it);
        }
        UiAtlasRect rect;
        if (!s_pages[kCachePage].Allocator.Allocate(width, height, rect))
            return false;
        CacheEntry entry;
        entry.R.Page = kCachePage;
        entry.R.Rect = rect;
        entry.LastUsed = Now();
        entry.Frame = s_frame;
        s_cache[id] = entry;
        out = entry.R;
        return true;
    }

    int CachedCount() { return static_cast<int>(s_cache.size()); }

    bgfx::TextureHandle Texture(int page)
    {
        bgfx::TextureHandle none = BGFX_INVALID_HANDLE;
        return page >= 0 && page < kMaxPages ? s_pages[page].Texture : none;
    }

    bgfx::FrameBufferHandle FrameBuffer(int page)
    {
        bgfx::FrameBufferHandle none = BGFX_INVALID_HANDLE;
        return page >= 0 && page < kMaxPages ? s_pages[page].FrameBuffer : none;
    }

    glm::vec4 UvRect(const Region& region)
    {
        const float size = static_cast<float>(kPageSize);
        const float u0 = region.Rect.X / size, u1 = (region.Rect.X + region.Rect.W) / size;
        float v0 = region.Rect.Y / size, v1 = (region.Rect.Y + region.Rect.H) / size;
        const bgfx::Caps* caps = bgfx::getCaps();
        if (caps != nullptr && caps->originBottomLeft)
        {
            // bgfx maps setViewRect to glViewport(x, pageH - y - h, w, h): the rows land mirrored, and the region's top row is the high v.
            v0 = 1.f - v0;
            v1 = 1.f - v1;
        }
        return glm::vec4(u0, v0, u1, v1);
    }

    glm::vec4 ClampRect(const Region& region)
    {
        const glm::vec4 uv = UvRect(region);
        const float half = 0.5f / static_cast<float>(kPageSize);
        return glm::vec4(std::min(uv.x, uv.z) + half, std::min(uv.y, uv.w) + half, std::max(uv.x, uv.z) - half, std::max(uv.y, uv.w) - half);
    }
}
