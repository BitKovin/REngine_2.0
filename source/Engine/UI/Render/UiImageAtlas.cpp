#include "UiImageAtlas.h"

#include <AssetRegistry.h>
#include <Logger.hpp>
#include <Texture.hpp>

#include <algorithm>
#include <cstring>
#include <unordered_map>
#include <vector>

namespace
{
    constexpr int kPageSize = 2048;
    constexpr int kMaxPages = 4;
    constexpr int kMaxImage = 512;
    constexpr int kPadding = 4;
    constexpr int kLevels = 12;   // 2048 .. 1

    struct Segment { int X, Y, W; };
    struct Rect { int X, Y, W, H; };

    struct Page
    {
        std::vector<std::vector<uint8_t>> Levels;   // CPU mip chain, RGBA8
        std::vector<Segment> Skyline;
        std::vector<Rect> Dirty;                    // level-0 rectangles not uploaded yet
        bgfx::TextureHandle Texture = BGFX_INVALID_HANDLE;
    };

    struct Entry
    {
        UiAtlasImage Image;
        bool Standalone = false;
        bool Failed = false;
        std::string Path;
    };

    std::vector<Page> s_pages;
    std::unordered_map<std::string, Entry> s_entries;

    int LevelSize(int level) { return std::max(1, kPageSize >> level); }

    void CreatePage()
    {
        Page page;
        page.Levels.resize(kLevels);
        for (int l = 0; l < kLevels; ++l)
            page.Levels[l].assign(static_cast<size_t>(LevelSize(l)) * LevelSize(l) * 4, 0);
        page.Skyline.push_back({ 0, 0, kPageSize });
        page.Texture = bgfx::createTexture2D(kPageSize, kPageSize, true, 1, bgfx::TextureFormat::RGBA8, BGFX_SAMPLER_NONE);
        s_pages.push_back(std::move(page));
    }

    // Bottom-left skyline: the lowest position where a w x h block fits.
    bool Pack(Page& page, int w, int h, int& outX, int& outY)
    {
        int bestY = kPageSize + 1, bestX = 0, bestIndex = -1;
        for (size_t i = 0; i < page.Skyline.size(); ++i)
        {
            const int x = page.Skyline[i].X;
            if (x + w > kPageSize)
                continue;
            int y = 0;
            int remaining = w;
            for (size_t j = i; j < page.Skyline.size() && remaining > 0; ++j)
            {
                y = std::max(y, page.Skyline[j].Y);
                remaining -= page.Skyline[j].W;
            }
            if (y + h > kPageSize)
                continue;
            if (y < bestY || (y == bestY && x < bestX))
            {
                bestY = y;
                bestX = x;
                bestIndex = static_cast<int>(i);
            }
        }
        if (bestIndex < 0)
            return false;
        outX = bestX;
        outY = bestY;

        // raise the skyline over [bestX, bestX + w)
        std::vector<Segment> next;
        const int end = bestX + w;
        for (const Segment& s : page.Skyline)
        {
            if (s.X + s.W <= bestX || s.X >= end)
            {
                next.push_back(s);
                continue;
            }
            if (s.X < bestX)
                next.push_back({ s.X, s.Y, bestX - s.X });
            if (s.X + s.W > end)
                next.push_back({ end, s.Y, s.X + s.W - end });
        }
        next.push_back({ bestX, bestY + h, w });
        std::sort(next.begin(), next.end(), [](const Segment& a, const Segment& b) { return a.X < b.X; });
        page.Skyline = std::move(next);
        return true;
    }

    void Blit(Page& page, const Texture::Decoded& image, int x, int y)
    {
        std::vector<uint8_t>& level0 = page.Levels[0];
        const int w = image.width, h = image.height;
        // the image plus kPadding texels around it, with the edge texels repeated into the padding
        for (int py = -kPadding; py < h + kPadding; ++py)
        {
            const int sy = std::clamp(py, 0, h - 1);
            for (int px = -kPadding; px < w + kPadding; ++px)
            {
                const int sx = std::clamp(px, 0, w - 1);
                std::memcpy(&level0[(static_cast<size_t>(y + py) * kPageSize + (x + px)) * 4],
                            &image.pixels[(static_cast<size_t>(sy) * w + sx) * 4], 4);
            }
        }
    }

    // Box filter of the 2 x 2 texels above, for the rectangle [x0, x1) x [y0, y1) of `level`.
    void BuildLevel(Page& page, int level, int x0, int y0, int x1, int y1)
    {
        const int srcSize = LevelSize(level - 1);
        const std::vector<uint8_t>& src = page.Levels[level - 1];
        std::vector<uint8_t>& dst = page.Levels[level];
        const int dstSize = LevelSize(level);
        for (int y = y0; y < y1; ++y)
            for (int x = x0; x < x1; ++x)
            {
                const int sx0 = std::min(x * 2, srcSize - 1), sx1 = std::min(x * 2 + 1, srcSize - 1);
                const int sy0 = std::min(y * 2, srcSize - 1), sy1 = std::min(y * 2 + 1, srcSize - 1);
                const uint8_t* p[4] = { &src[(static_cast<size_t>(sy0) * srcSize + sx0) * 4], &src[(static_cast<size_t>(sy0) * srcSize + sx1) * 4],
                                        &src[(static_cast<size_t>(sy1) * srcSize + sx0) * 4], &src[(static_cast<size_t>(sy1) * srcSize + sx1) * 4] };
                uint8_t* out = &dst[(static_cast<size_t>(y) * dstSize + x) * 4];
                for (int c = 0; c < 4; ++c)
                    out[c] = static_cast<uint8_t>((p[0][c] + p[1][c] + p[2][c] + p[3][c] + 2) / 4);
            }
    }

    void UploadRect(Page& page, int level, int x0, int y0, int x1, int y1)
    {
        const int size = LevelSize(level);
        const int w = x1 - x0, h = y1 - y0;
        if (w <= 0 || h <= 0)
            return;
        const bgfx::Memory* mem = bgfx::alloc(static_cast<uint32_t>(w) * h * 4);
        for (int row = 0; row < h; ++row)
            std::memcpy(mem->data + static_cast<size_t>(row) * w * 4, &page.Levels[level][(static_cast<size_t>(y0 + row) * size + x0) * 4],
                        static_cast<size_t>(w) * 4);
        bgfx::updateTexture2D(page.Texture, 0, static_cast<uint8_t>(level), static_cast<uint16_t>(x0), static_cast<uint16_t>(y0),
                              static_cast<uint16_t>(w), static_cast<uint16_t>(h), mem);
    }
}

namespace UiImageAtlas
{
    UiAtlasImage Get(const std::string& path)
    {
        auto found = s_entries.find(path);
        if (found != s_entries.end())
        {
            Entry& entry = found->second;
            if (entry.Standalone && !entry.Image.Valid && !entry.Failed)
            {
                Texture* texture = AssetRegistry::GetTextureFromFile(path);
                if (texture != nullptr && texture->valid)
                {
                    entry.Image.Texture = texture->getHandle();
                    entry.Image.Valid = bgfx::isValid(entry.Image.Texture);
                    entry.Image.Pending = false;
                    entry.Image.Width = entry.Image.TextureWidth = texture->width;
                    entry.Image.Height = entry.Image.TextureHeight = texture->height;
                }
            }
            return entry.Image;
        }

        Entry entry;
        entry.Path = path;
        Texture::Decoded decoded = Texture::DecodeFromFile(path, false);
        if (!decoded.valid)
        {
            entry.Failed = true;
            return s_entries.emplace(path, std::move(entry)).first->second.Image;
        }
        entry.Image.Width = decoded.width;
        entry.Image.Height = decoded.height;

        bool packed = false;
        if (decoded.width <= kMaxImage && decoded.height <= kMaxImage)
        {
            const int w = decoded.width + 2 * kPadding, h = decoded.height + 2 * kPadding;
            int x = 0, y = 0;
            for (size_t i = 0; i <= s_pages.size() && !packed; ++i)
            {
                if (i == s_pages.size())
                {
                    if (static_cast<int>(s_pages.size()) >= kMaxPages)
                        break;
                    CreatePage();
                }
                if (Pack(s_pages[i], w, h, x, y))
                {
                    Blit(s_pages[i], decoded, x + kPadding, y + kPadding);
                    s_pages[i].Dirty.push_back({ x, y, w, h });
                    entry.Image.Texture = s_pages[i].Texture;
                    entry.Image.TextureWidth = entry.Image.TextureHeight = kPageSize;
                    entry.Image.Uv = glm::vec4(static_cast<float>(x + kPadding) / kPageSize, static_cast<float>(y + kPadding) / kPageSize,
                                               static_cast<float>(x + kPadding + decoded.width) / kPageSize,
                                               static_cast<float>(y + kPadding + decoded.height) / kPageSize);
                    entry.Image.Valid = true;
                    packed = true;
                }
            }
        }
        if (!packed)
        {
            // too large, or every page is full: a standalone texture
            entry.Standalone = true;
            Texture* texture = AssetRegistry::GetTextureFromFile(path);
            if (texture != nullptr && texture->valid)
            {
                entry.Image.Texture = texture->getHandle();
                entry.Image.TextureWidth = texture->width;
                entry.Image.TextureHeight = texture->height;
                entry.Image.Valid = bgfx::isValid(entry.Image.Texture);
            }
            else
                entry.Image.Pending = true;
        }
        return s_entries.emplace(path, std::move(entry)).first->second.Image;
    }

    void Update()
    {
        for (Page& page : s_pages)
        {
            if (page.Dirty.empty())
                continue;
            for (const Rect& r : page.Dirty)
            {
                UploadRect(page, 0, r.X, r.Y, r.X + r.W, r.Y + r.H);
                int x0 = r.X, y0 = r.Y, x1 = r.X + r.W, y1 = r.Y + r.H;
                for (int level = 1; level < kLevels; ++level)
                {
                    x0 /= 2;
                    y0 /= 2;
                    x1 = (x1 + 1) / 2;
                    y1 = (y1 + 1) / 2;
                    BuildLevel(page, level, x0, y0, x1, y1);
                    UploadRect(page, level, x0, y0, x1, y1);
                }
            }
            page.Dirty.clear();
        }
    }

    void Shutdown()
    {
        for (Page& page : s_pages)
            if (bgfx::isValid(page.Texture))
                bgfx::destroy(page.Texture);
        s_pages.clear();
        s_entries.clear();
    }

    int PageCount() { return static_cast<int>(s_pages.size()); }
}
