#include "ExrLightmap.h"
#include <Compression/miniz.h>
#include <algorithm>
#include <cstring>

namespace {

uint32_t Get32(const uint8_t* p) { return uint32_t(p[0]) | uint32_t(p[1]) << 8 | uint32_t(p[2]) << 16 | uint32_t(p[3]) << 24; }
uint64_t Get64(const uint8_t* p) { return uint64_t(Get32(p)) | uint64_t(Get32(p + 4)) << 32; }

uint16_t FloatToHalf(float f) {
    uint32_t x;
    std::memcpy(&x, &f, 4);
    if ((x & 0x7fffffffu) > 0x7f800000u || (x >> 31)) return 0;  // NaN, negative: light is never negative
    const int e = int((x >> 23) & 0xff) - 127 + 15;
    if (e >= 31) return 0x7bff;                                  // saturate
    if (e <= 0) return 0;
    uint32_t m = x & 0x7fffff;
    uint16_t h = uint16_t((e << 10) | (m >> 13));
    if ((m & 0x1fff) > 0x1000 || ((m & 0x1fff) == 0x1000 && (h & 1))) ++h;
    return h;
}

}  // namespace

bool DecodeExrLightmap(const uint8_t* d, size_t n, ExrLightmapImage& out, std::string* error) {
    auto fail = [&](const char* why) { if (error) *error = why; return false; };
    if (n < 8 || Get32(d) != 20000630u) return fail("not an EXR file");
    if ((Get32(d + 4) & 0xff) != 2 || (Get32(d + 4) & 0x1a00) != 0) return fail("only single part scanline EXR files are supported");
    size_t p = 8;
    auto str = [&](std::string& s) {
        s.clear();
        while (p < n && d[p]) s.push_back(char(d[p++]));
        if (p >= n) return false;
        ++p;
        return true;
    };
    struct Channel { std::string name; int type = 0; };
    std::vector<Channel> chans;
    int compression = -1;
    int32_t xMin = 0, yMin = 0, xMax = -1, yMax = -1;
    for (;;) {
        std::string name, type;
        if (!str(name)) return fail("truncated header");
        if (name.empty()) break;
        if (!str(type) || p + 4 > n) return fail("truncated header");
        const uint32_t size = Get32(d + p);
        p += 4;
        if (p + size > n) return fail("truncated header");
        const uint8_t* v = d + p;
        if (name == "channels") {
            size_t q = 0;
            while (q < size && v[q]) {
                Channel c;
                while (q < size && v[q]) c.name.push_back(char(v[q++]));
                ++q;
                if (q + 16 > size) return fail("bad channel list");
                c.type = int(Get32(v + q));
                q += 16;
                chans.push_back(c);
            }
        } else if (name == "compression" && size >= 1) {
            compression = v[0];
        } else if (name == "dataWindow" && size >= 16) {
            xMin = int32_t(Get32(v)); yMin = int32_t(Get32(v + 4)); xMax = int32_t(Get32(v + 8)); yMax = int32_t(Get32(v + 12));
        }
        p += size;
    }
    if (compression != 0 && compression != 2 && compression != 3) return fail("unsupported EXR compression (none, ZIPS and ZIP are read)");
    const int w = xMax - xMin + 1, h = yMax - yMin + 1;
    if (w <= 0 || h <= 0 || w > 16384 || h > 16384) return fail("bad data window");
    size_t pixelBytes = 0;
    for (const auto& c : chans) {
        if (c.type != 1 && c.type != 2) return fail("only half and float channels are read");
        pixelBytes += c.type == 1 ? 2 : 4;
    }
    const int lines = compression == 3 ? 16 : 1;
    const int blocks = (h + lines - 1) / lines;
    if (p + size_t(blocks) * 8 > n) return fail("truncated offset table");
    out.width = w;
    out.height = h;
    out.rgbaHalf.assign(size_t(w) * h * 4, 0);
    for (size_t i = 3; i < out.rgbaHalf.size(); i += 4) out.rgbaHalf[i] = 0x3c00;  // alpha 1
    std::vector<uint8_t> buf, raw;
    for (int k = 0; k < blocks; ++k) {
        const uint64_t at = Get64(d + p + size_t(k) * 8);
        if (at + 8 > n) return fail("bad block offset");
        const int y0 = int32_t(Get32(d + at)) - yMin;
        const uint32_t len = Get32(d + at + 4);
        if (at + 8 + len > n || y0 < 0 || y0 >= h) return fail("bad block");
        const int y1 = std::min(h, y0 + lines);
        const size_t expect = size_t(y1 - y0) * w * pixelBytes;
        if (len == expect) {
            raw.assign(d + at + 8, d + at + 8 + len);
        } else {
            buf.resize(expect);
            mz_ulong got = mz_ulong(expect);
            if (mz_uncompress(buf.data(), &got, d + at + 8, len) != MZ_OK || got != expect) return fail("corrupt EXR block");
            for (size_t i = 1; i < buf.size(); ++i) buf[i] = uint8_t(int(buf[i - 1]) + int(buf[i]) - 128);  // undo the predictor
            raw.resize(buf.size());
            const size_t half = (buf.size() + 1) / 2;
            for (size_t i = 0; i < raw.size(); ++i) raw[i] = buf[(i & 1) ? half + i / 2 : i / 2];  // undo the byte split
        }
        size_t q = 0;
        for (int y = y0; y < y1; ++y)
            for (const auto& c : chans) {
                const int slot = c.name == "R" ? 0 : c.name == "G" ? 1 : c.name == "B" ? 2 : -1;
                for (int x = 0; x < w; ++x) {
                    uint16_t hv;
                    if (c.type == 1) { hv = uint16_t(raw[q] | raw[q + 1] << 8); q += 2; }
                    else { float f; uint32_t u = Get32(&raw[q]); std::memcpy(&f, &u, 4); hv = FloatToHalf(f); q += 4; }
                    if (slot >= 0) out.rgbaHalf[(size_t(y) * w + x) * 4 + slot] = hv;
                }
            }
    }
    return true;
}
