#pragma once
#include <cstdint>
#include <string>
#include <vector>

// Lightmap pages of rBSP v2 maps (written by the level editor, mapeditor DESIGN §12): OpenEXR, scanline, R G B half (or float) channels,
// no compression / ZIPS / ZIP. Only that subset is read. Values are linear and used as stored (colour = albedo x value).
struct ExrLightmapImage {
    int width = 0, height = 0;
    std::vector<uint16_t> rgbaHalf;  // width * height * 4 half floats, row 0 = top, alpha 1.0: ready for an RGBA16F texture
};

bool DecodeExrLightmap(const uint8_t* data, size_t size, ExrLightmapImage& out, std::string* error = nullptr);
