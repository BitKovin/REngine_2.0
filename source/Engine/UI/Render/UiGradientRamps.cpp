#include "UiGradientRamps.h"

#include <Logger.hpp>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <map>

namespace
{
    constexpr int kRows = 256;
    constexpr int kWidth = 256;
    constexpr size_t kMaxEntries = 65536;

    struct Entry
    {
        std::vector<UiRampStop> Stops;
        int Row = -1;
    };

    std::vector<Entry> s_entries;
    std::map<std::vector<float>, int> s_ids;
    int s_rowOwner[kRows];
    uint32_t s_rowFrame[kRows];
    bool s_rowDirty[kRows];
    std::vector<uint8_t> s_pixels;   // kRows * kWidth * 4
    bgfx::TextureHandle s_texture = BGFX_INVALID_HANDLE;
    uint32_t s_frame = 1;
    bool s_loggedFull = false;
    bool s_loggedIds = false;

    // Interpolates in premultiplied space, stores straight (8 bit).
    void FillRow(const Entry& entry, int row)
    {
        uint8_t* out = &s_pixels[static_cast<size_t>(row) * kWidth * 4];
        const std::vector<UiRampStop>& s = entry.Stops;
        for (int j = 0; j < kWidth; ++j)
        {
            const float t = static_cast<float>(j) / static_cast<float>(kWidth - 1);
            glm::vec4 premul;
            if (t <= s.front().Pos)
                premul = glm::vec4(glm::vec3(s.front().Color) * s.front().Color.a, s.front().Color.a);
            else if (t >= s.back().Pos)
                premul = glm::vec4(glm::vec3(s.back().Color) * s.back().Color.a, s.back().Color.a);
            else
            {
                size_t i = 1;
                while (i + 1 < s.size() && s[i].Pos < t)
                    ++i;
                const UiRampStop& a = s[i - 1];
                const UiRampStop& b = s[i];
                const float span = b.Pos - a.Pos;
                const float f = span > 0.00001f ? (t - a.Pos) / span : 1.f;
                const glm::vec4 pa(glm::vec3(a.Color) * a.Color.a, a.Color.a);
                const glm::vec4 pb(glm::vec3(b.Color) * b.Color.a, b.Color.a);
                premul = pa + (pb - pa) * f;
            }
            glm::vec4 straight(0.f);
            if (premul.a > 0.0001f)
                straight = glm::vec4(glm::vec3(premul) / premul.a, premul.a);
            straight = glm::clamp(straight, glm::vec4(0.f), glm::vec4(1.f));
            for (int c = 0; c < 4; ++c)
                out[j * 4 + c] = static_cast<uint8_t>(std::lround(straight[c] * 255.f));
        }
    }
}

namespace UiGradientRamps
{
    void Init()
    {
        if (bgfx::isValid(s_texture))
            return;
        s_pixels.assign(static_cast<size_t>(kRows) * kWidth * 4, 0);
        for (int i = 0; i < kRows; ++i)
        {
            s_rowOwner[i] = -1;
            s_rowFrame[i] = 0;
            s_rowDirty[i] = false;
        }
        s_entries.clear();
        s_ids.clear();
        s_frame = 1;
        // no initial memory: a texture created with data is immutable on some backends and rows are uploaded later
        s_texture = bgfx::createTexture2D(kWidth, kRows, false, 1, bgfx::TextureFormat::RGBA8,
                                          BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP);
    }

    void Shutdown()
    {
        if (bgfx::isValid(s_texture))
        {
            bgfx::destroy(s_texture);
            s_texture = BGFX_INVALID_HANDLE;
        }
        s_entries.clear();
        s_ids.clear();
        s_pixels.clear();
    }

    int Id(const std::vector<UiRampStop>& stops)
    {
        if (stops.empty())
            return -1;
        std::vector<float> key;
        key.reserve(stops.size() * 5);
        for (const UiRampStop& s : stops)
        {
            key.push_back(s.Pos);
            key.push_back(s.Color.r);
            key.push_back(s.Color.g);
            key.push_back(s.Color.b);
            key.push_back(s.Color.a);
        }
        const auto found = s_ids.find(key);
        if (found != s_ids.end())
            return found->second;
        if (s_entries.size() >= kMaxEntries)
        {
            if (!s_loggedIds)
            {
                Logger::Error("[UiRenderer] more than %d distinct gradients; the rest is not drawn", static_cast<int>(kMaxEntries));
                s_loggedIds = true;
            }
            return -1;
        }
        Entry entry;
        entry.Stops = stops;
        // positions must not decrease
        for (size_t i = 1; i < entry.Stops.size(); ++i)
            entry.Stops[i].Pos = std::max(entry.Stops[i].Pos, entry.Stops[i - 1].Pos);
        s_entries.push_back(std::move(entry));
        const int id = static_cast<int>(s_entries.size()) - 1;
        s_ids[key] = id;
        return id;
    }

    int Resolve(int id)
    {
        if (id < 0 || id >= static_cast<int>(s_entries.size()) || !bgfx::isValid(s_texture))
            return -1;
        Entry& entry = s_entries[id];
        if (entry.Row >= 0)
        {
            s_rowFrame[entry.Row] = s_frame;
            return entry.Row;
        }
        int row = -1;
        for (int i = 0; i < kRows && row < 0; ++i)
            if (s_rowOwner[i] < 0)
                row = i;
        if (row < 0)
        {
            uint32_t oldest = s_frame;
            for (int i = 0; i < kRows; ++i)
                if (s_rowFrame[i] < oldest)
                {
                    oldest = s_rowFrame[i];
                    row = i;
                }
        }
        if (row < 0)
        {
            if (!s_loggedFull)
            {
                Logger::Error("[UiRenderer] all 256 gradient ramp rows are in use this frame; some gradients are not drawn");
                s_loggedFull = true;
            }
            return -1;
        }
        if (s_rowOwner[row] >= 0)
            s_entries[s_rowOwner[row]].Row = -1;
        s_rowOwner[row] = id;
        s_rowFrame[row] = s_frame;
        entry.Row = row;
        FillRow(entry, row);
        s_rowDirty[row] = true;
        return row;
    }

    void UploadPending()
    {
        if (!bgfx::isValid(s_texture))
            return;
        for (int i = 0; i < kRows; ++i)
        {
            if (!s_rowDirty[i])
                continue;
            int last = i;
            while (last + 1 < kRows && s_rowDirty[last + 1])
                ++last;
            const int rows = last - i + 1;
            bgfx::updateTexture2D(s_texture, 0, 0, 0, static_cast<uint16_t>(i), kWidth, static_cast<uint16_t>(rows),
                                  bgfx::copy(&s_pixels[static_cast<size_t>(i) * kWidth * 4], static_cast<uint32_t>(rows) * kWidth * 4));
            for (int k = i; k <= last; ++k)
                s_rowDirty[k] = false;
            i = last;
        }
    }

    void OnBeforeBgfxFrame()
    {
        ++s_frame;
        s_loggedFull = false;
    }

    bgfx::TextureHandle Texture() { return s_texture; }

    int ResidentRows()
    {
        int n = 0;
        for (int i = 0; i < kRows; ++i)
            n += s_rowOwner[i] >= 0 ? 1 : 0;
        return n;
    }
}
