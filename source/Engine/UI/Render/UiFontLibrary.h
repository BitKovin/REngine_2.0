#pragma once
#include <bgfx/bgfx.h>
#include <cstdint>
#include <string>
#include <vector>

struct UiGlyph
{
    bool Valid = false;          // false: no bitmap (space) or missing
    uint16_t Page = 0;
    float U0 = 0, V0 = 0, U1 = 0, V1 = 0;
    float Width = 0, Height = 0;    // bitmap size in atlas pixels (includes the 8 px SDF padding)
    float OffsetX = 0, OffsetY = 0; // bitmap top-left relative to pen/baseline, atlas pixels
    float Advance = 0;              // atlas pixels
    int FaceIndex = 0;              // face the glyph came from (fallbacks differ)
};

struct UiFaceMetrics { float Ascent = 0, Descent = 0, LineGap = 0; }; // atlas pixels at kBaseSize, Descent negative

class UiFontLibrary
{
public:
    static constexpr float kBaseSize = 48.f;    // glyphs are generated at 48 px per em
    static constexpr int   kPadding = 8;        // SDF spread in atlas pixels
    static constexpr int   kPageSize = 2048;
    static constexpr int   kMaxPages = 4;

    static void Init();
    static void Shutdown();

    // @font-face registration. Returns false and logs if the file can't be read or parsed.
    static bool RegisterFace(const std::string& family, int weight, bool italic, const std::string& path);
    static void AddFallback(const std::string& path);   // also called from UiRenderer::AddFallbackFont
    // Picks the closest face: exact family (case-insensitive), then nearest weight, then italic match.
    // Unknown family falls back to "default". Sets `syntheticBold`/`syntheticItalic` when no matching face exists.
    static int ResolveFace(const std::string& family, int weight, bool italic, bool& syntheticBold, bool& syntheticItalic);
    static bool HasFamily(const std::string& family);

    static const UiFaceMetrics& Metrics(int face);
    static const UiGlyph& Glyph(int face, uint32_t codepoint);     // generates on demand
    static bool HasCodepoint(int face, uint32_t codepoint);        // primary face only, no fallbacks
    static float Kerning(int face, uint32_t left, uint32_t right); // atlas pixels; 0 across faces
    static bgfx::TextureHandle PageTexture(uint16_t page);
    // Uploads dirty atlas regions. Called by UiSystem::Finalize (main thread).
    static void FlushUploads();
    static size_t FaceCount();
    static size_t GlyphCount();
};
