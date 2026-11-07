#include "UiSvg.h"

#include <FileSystem/FileSystem.h>
#include <Logger.hpp>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <unordered_map>

#define NANOSVG_IMPLEMENTATION
#define NANOSVGRAST_IMPLEMENTATION
#include <includedLibraries/nanosvg/nanosvg.h>
#include <includedLibraries/nanosvg/nanosvgrast.h>

namespace
{
    struct Doc
    {
        NSVGimage* Image = nullptr;
    };
    std::unordered_map<std::string, Doc> s_docs;

    NSVGimage* Parse(const std::string& path)
    {
        auto found = s_docs.find(path);
        if (found != s_docs.end())
            return found->second.Image;
        Doc doc;
        std::string text = FileSystemEngine::ReadFile(path);
        if (text.empty())
            Logger::Error("[UiSvg] cannot read %s", path.c_str());
        else
        {
            doc.Image = nsvgParse(text.data(), "px", 96.f);   // nsvgParse edits the buffer in place
            if (doc.Image == nullptr || doc.Image->width <= 0.f || doc.Image->height <= 0.f)
            {
                Logger::Error("[UiSvg] cannot parse %s", path.c_str());
                if (doc.Image != nullptr)
                    nsvgDelete(doc.Image);
                doc.Image = nullptr;
            }
        }
        return s_docs.emplace(path, doc).first->second.Image;
    }
}

namespace UiSvg
{
    bool IsSvgPath(const std::string& path)
    {
        const size_t n = path.size();
        return n > 4 && path[n - 4] == '.' && (path[n - 3] | 32) == 's' && (path[n - 2] | 32) == 'v' && (path[n - 1] | 32) == 'g';
    }

    bool Load(const std::string& path, float& intrinsicW, float& intrinsicH)
    {
        NSVGimage* image = Parse(path);
        if (image == nullptr)
            return false;
        intrinsicW = image->width;
        intrinsicH = image->height;
        return true;
    }

    bool Rasterize(const std::string& path, int longSide, Texture::Decoded& out)
    {
        NSVGimage* image = Parse(path);
        if (image == nullptr)
            return false;
        const float scale = static_cast<float>(longSide) / std::max(image->width, image->height);
        const int w = std::max(1, static_cast<int>(std::lround(image->width * scale)));
        const int h = std::max(1, static_cast<int>(std::lround(image->height * scale)));
        NSVGrasterizer* rast = nsvgCreateRasterizer();
        if (rast == nullptr)
            return false;
        out.pixels.assign(static_cast<size_t>(w) * h * 4, 0);
        nsvgRasterize(rast, image, 0.f, 0.f, scale, out.pixels.data(), w, h, w * 4);
        nsvgDeleteRasterizer(rast);
        out.width = w;
        out.height = h;
        out.generateMipmaps = false;
        out.valid = true;
        return true;
    }

    void Shutdown()
    {
        for (auto& [path, doc] : s_docs)
            if (doc.Image != nullptr)
                nsvgDelete(doc.Image);
        s_docs.clear();
    }
}
