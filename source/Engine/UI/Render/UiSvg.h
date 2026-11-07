#pragma once
#include <glm.h>
#include <Texture.hpp>
#include <string>

// SVG icons for the UI image atlas: nanosvg parses the file, the rasteriser draws it at a requested size (straight alpha RGBA8).
namespace UiSvg
{
    bool IsSvgPath(const std::string& path);
    // Intrinsic size of the drawing in UI units (its width/height, 96 dpi px). False when the file cannot be read or parsed.
    bool Load(const std::string& path, float& intrinsicW, float& intrinsicH);
    // Rasterises so that the longer side is `longSide` texels. Parsing is cached per path.
    bool Rasterize(const std::string& path, int longSide, Texture::Decoded& out);
    void Shutdown();
}
