#include "UiFontLibrary.h"

#include <FileSystem/FileSystem.h>
#include <Logger.hpp>
#include <UI/Markup/UiMarkupLog.h>
#include <includedLibraries/stb_truetype.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <unordered_map>

namespace
{
    struct Face
    {
        std::string Family;   // lowercase
        int Weight = 400;
        bool Italic = false;
        std::string Path;
        std::vector<uint8_t> Bytes;
        stbtt_fontinfo Info{};
        float Scale = 1.f;
        UiFaceMetrics Metrics;
    };

    struct Page
    {
        bgfx::TextureHandle Texture = BGFX_INVALID_HANDLE;
        std::vector<uint8_t> Cpu;
        int ShelfX = 0, ShelfY = 0, ShelfH = 0;
        int DirtyY0 = 1 << 30, DirtyY1 = -1;
    };

    std::vector<Face>& Faces() { static std::vector<Face> faces; return faces; }
    std::vector<int>& Fallbacks() { static std::vector<int> fallbacks; return fallbacks; }
    std::vector<Page>& Pages() { static std::vector<Page> pages; return pages; }
    std::unordered_map<uint64_t, UiGlyph>& Cache() { static std::unordered_map<uint64_t, UiGlyph> cache; return cache; }
    bool g_loggedFull = false;
    bool g_ensuring = false;
    bool g_defaultFailedLogged = false;

    std::string Lower(std::string s)
    {
        for (char& c : s)
            if (c >= 'A' && c <= 'Z')
                c = static_cast<char>(c - 'A' + 'a');
        return s;
    }

    // Slot 0 is always the "default" face. It is reserved first so fallbacks registered before the file system is
    // ready cannot take its index; the font itself is loaded as soon as the file can be read.
    void EnsureDefault()
    {
        auto& faces = Faces();
        if (faces.empty())
        {
            Face placeholder;
            placeholder.Family = "default";
            faces.push_back(std::move(placeholder));
        }
        if (!faces[0].Bytes.empty() || g_ensuring)
            return;
        g_ensuring = true;
        Face loaded;
        loaded.Family = "default";
        loaded.Path = "GameData/fonts/NotoSans.ttf";
        loaded.Bytes = FileSystemEngine::ReadFileBinary(loaded.Path);
        const int offset = loaded.Bytes.empty() ? -1 : stbtt_GetFontOffsetForIndex(loaded.Bytes.data(), 0);
        if (offset >= 0 && stbtt_InitFont(&loaded.Info, loaded.Bytes.data(), offset))
        {
            loaded.Scale = stbtt_ScaleForMappingEmToPixels(&loaded.Info, UiFontLibrary::kBaseSize);
            int ascent = 0, descent = 0, lineGap = 0;
            stbtt_GetFontVMetrics(&loaded.Info, &ascent, &descent, &lineGap);
            loaded.Metrics = { ascent * loaded.Scale, descent * loaded.Scale, lineGap * loaded.Scale };
            faces[0] = std::move(loaded);
        }
        else if (!g_defaultFailedLogged)
        {
            g_defaultFailedLogged = true;
            Logger::Warning("[UI] the default UI font %s is not readable yet; text is not drawn until it is", loaded.Path.c_str());
        }
        g_ensuring = false;
    }

    // Returns the face holding the glyph, or -1.
    int FindFace(int face, uint32_t cp, int& glyphIndex)
    {
        auto& faces = Faces();
        auto has = [&](int f, int& gi) {
            if (f < 0 || f >= (int)faces.size() || faces[f].Bytes.empty())
                return false;
            gi = stbtt_FindGlyphIndex(&faces[f].Info, (int)cp);
            return gi != 0;
        };
        if (has(face, glyphIndex))
            return face;
        for (int fb : Fallbacks())
            if (has(fb, glyphIndex))
                return fb;
        return -1;
    }

    bool AllocatePage()
    {
        auto& pages = Pages();
        if ((int)pages.size() >= UiFontLibrary::kMaxPages)
            return false;
        Page page;
        const size_t bytes = (size_t)UiFontLibrary::kPageSize * UiFontLibrary::kPageSize;
        page.Cpu.assign(bytes, 0);
        page.Texture = bgfx::createTexture2D((uint16_t)UiFontLibrary::kPageSize, (uint16_t)UiFontLibrary::kPageSize, false, 1,
                                             bgfx::TextureFormat::R8, BGFX_SAMPLER_UVW_CLAMP);   // no initial data: a texture created with data is immutable and updateTexture2D would be ignored
        page.DirtyY0 = 0;               // the first flush uploads the whole (zeroed) page
        page.DirtyY1 = UiFontLibrary::kPageSize;
        pages.push_back(std::move(page));
        return true;
    }

    // Shelf packer; returns false when the atlas is full.
    bool Pack(int w, int h, uint16_t& pageIndex, int& x, int& y)
    {
        auto& pages = Pages();
        if (pages.empty() && !AllocatePage())
            return false;
        for (;;)
        {
            Page& page = pages.back();
            if (page.ShelfX + w > UiFontLibrary::kPageSize)
            {
                page.ShelfY += page.ShelfH + 1;
                page.ShelfX = 0;
                page.ShelfH = 0;
            }
            if (page.ShelfY + h > UiFontLibrary::kPageSize)
            {
                if (!AllocatePage())
                    return false;
                continue;
            }
            x = page.ShelfX;
            y = page.ShelfY;
            page.ShelfX += w + 1;
            page.ShelfH = std::max(page.ShelfH, h);
            pageIndex = (uint16_t)(pages.size() - 1);
            return true;
        }
    }
}

void UiFontLibrary::Init()
{
    EnsureDefault();
}

void UiFontLibrary::Shutdown()
{
    for (Page& page : Pages())
        if (bgfx::isValid(page.Texture))
            bgfx::destroy(page.Texture);
    Pages().clear();
    Cache().clear();
    Faces().clear();
    Fallbacks().clear();
}

bool UiFontLibrary::RegisterFace(const std::string& family, int weight, bool italic, const std::string& path)
{
    EnsureDefault();
    const std::string fam = Lower(family);
    for (const Face& f : Faces())
    {
        if (f.Family == fam && f.Weight == weight && f.Italic == italic && !f.Bytes.empty())
        {
            if (f.Path != path)
                UiMarkupLog::Warning(path, 0, 0, "font face '" + family + "' is already registered with '" + f.Path + "'; the first one wins");
            return true;
        }
    }

    Face face;
    face.Family = fam;
    face.Weight = weight;
    face.Italic = italic;
    face.Path = path;
    face.Bytes = FileSystemEngine::ReadFileBinary(path);
    if (face.Bytes.empty())
    {
        UiMarkupLog::Error(path, 0, 0, "cannot read font file");
        return false;
    }
    const int offset = stbtt_GetFontOffsetForIndex(face.Bytes.data(), 0);
    if (offset < 0 || !stbtt_InitFont(&face.Info, face.Bytes.data(), offset))
    {
        UiMarkupLog::Error(path, 0, 0, "cannot parse font file");
        return false;
    }
    face.Scale = stbtt_ScaleForMappingEmToPixels(&face.Info, kBaseSize);
    int ascent = 0, descent = 0, lineGap = 0;
    stbtt_GetFontVMetrics(&face.Info, &ascent, &descent, &lineGap);
    face.Metrics = { ascent * face.Scale, descent * face.Scale, lineGap * face.Scale };
    // Moving a Face is fine: stbtt_fontinfo points at the byte vector's heap buffer, which a move keeps.
    Faces().push_back(std::move(face));
    return true;
}

void UiFontLibrary::AddFallback(const std::string& path)
{
    EnsureDefault();
    const std::string family = "__fallback" + std::to_string(Fallbacks().size());
    if (RegisterFace(family, 400, false, path))
        Fallbacks().push_back((int)Faces().size() - 1);
}

bool UiFontLibrary::HasFamily(const std::string& family)
{
    EnsureDefault();
    const std::string fam = Lower(family);
    for (const Face& f : Faces())
        if (f.Family == fam)
            return true;
    return false;
}

int UiFontLibrary::ResolveFace(const std::string& family, int weight, bool italic, bool& syntheticBold, bool& syntheticItalic)
{
    EnsureDefault();
    auto& faces = Faces();
    std::string fam = Lower(family);
    if (!HasFamily(fam))
        fam = "default";

    int best = -1, bestScore = 1 << 30;
    for (int i = 0; i < (int)faces.size(); ++i)
    {
        if (faces[i].Family != fam)
            continue;
        const int score = std::abs(faces[i].Weight - weight) * 4 + (faces[i].Italic != italic ? 1 : 0);
        if (score < bestScore)
        {
            bestScore = score;
            best = i;
        }
    }
    if (best < 0)
        best = 0;
    syntheticBold = weight >= 600 && faces[best].Weight < 600;
    syntheticItalic = italic && !faces[best].Italic;
    return best;
}

const UiFaceMetrics& UiFontLibrary::Metrics(int face)
{
    EnsureDefault();
    if (face < 0 || face >= (int)Faces().size())
        face = 0;
    return Faces()[face].Metrics;
}

bool UiFontLibrary::HasCodepoint(int face, uint32_t cp)
{
    EnsureDefault();
    if (face < 0 || face >= (int)Faces().size() || Faces()[face].Bytes.empty())
        return false;
    return stbtt_FindGlyphIndex(&Faces()[face].Info, (int)cp) != 0;
}

const UiGlyph& UiFontLibrary::Glyph(int face, uint32_t cp)
{
    EnsureDefault();
    if (face < 0 || face >= (int)Faces().size())
        face = 0;
    const uint64_t key = (uint64_t)face * 0x110000ull + cp;
    auto& cache = Cache();
    const auto found = cache.find(key);
    if (found != cache.end())
        return found->second;

    UiGlyph glyph;
    int glyphIndex = 0;
    int source = FindFace(face, cp, glyphIndex);
    if (source < 0)
    {
        source = face;                 // tofu box of the primary face
        glyphIndex = 0;
    }
    glyph.FaceIndex = source;
    Face& f = Faces()[source];
    if (f.Bytes.empty())
        return cache[key] = glyph;

    int advance = 0, lsb = 0;
    stbtt_GetGlyphHMetrics(&f.Info, glyphIndex, &advance, &lsb);
    glyph.Advance = advance * f.Scale;

    int w = 0, h = 0, xoff = 0, yoff = 0;
    unsigned char* bitmap = stbtt_GetGlyphSDF(&f.Info, f.Scale, glyphIndex, kPadding, 128, 16.0f, &w, &h, &xoff, &yoff);
    if (bitmap != nullptr && w > 0 && h > 0)
    {
        uint16_t pageIndex = 0;
        int x = 0, y = 0;
        if (Pack(w, h, pageIndex, x, y))
        {
            Page& page = Pages()[pageIndex];
            for (int row = 0; row < h; ++row)
                std::memcpy(&page.Cpu[(size_t)(y + row) * kPageSize + x], bitmap + (size_t)row * w, (size_t)w);
            page.DirtyY0 = std::min(page.DirtyY0, y);
            page.DirtyY1 = std::max(page.DirtyY1, y + h);
            glyph.Valid = true;
            glyph.Page = pageIndex;
            glyph.U0 = (float)x / kPageSize;
            glyph.V0 = (float)y / kPageSize;
            glyph.U1 = (float)(x + w) / kPageSize;
            glyph.V1 = (float)(y + h) / kPageSize;
            glyph.Width = (float)w;
            glyph.Height = (float)h;
            glyph.OffsetX = (float)xoff;
            glyph.OffsetY = (float)yoff;
        }
        else if (!g_loggedFull)
        {
            g_loggedFull = true;
            Logger::Error("[UI] font atlas is full (%d pages); further glyphs are not drawn", kMaxPages);
        }
    }
    if (bitmap != nullptr)
        stbtt_FreeSDF(bitmap, nullptr);
    return cache[key] = glyph;
}

float UiFontLibrary::Kerning(int face, uint32_t left, uint32_t right)
{
    EnsureDefault();
    if (face < 0 || face >= (int)Faces().size() || Faces()[face].Bytes.empty())
        return 0.f;
    Face& f = Faces()[face];
    const int a = stbtt_FindGlyphIndex(&f.Info, (int)left);
    const int b = stbtt_FindGlyphIndex(&f.Info, (int)right);
    if (a == 0 || b == 0)
        return 0.f;
    return stbtt_GetGlyphKernAdvance(&f.Info, a, b) * f.Scale;
}

bgfx::TextureHandle UiFontLibrary::PageTexture(uint16_t page)
{
    if (page >= Pages().size())
        return BGFX_INVALID_HANDLE;
    return Pages()[page].Texture;
}

void UiFontLibrary::FlushUploads()
{
    for (Page& page : Pages())
    {
        if (page.DirtyY1 < 0 || !bgfx::isValid(page.Texture))
            continue;
        const int rows = page.DirtyY1 - page.DirtyY0;
        const bgfx::Memory* mem = bgfx::copy(&page.Cpu[(size_t)page.DirtyY0 * kPageSize], (uint32_t)((size_t)rows * kPageSize));
        bgfx::updateTexture2D(page.Texture, 0, 0, 0, (uint16_t)page.DirtyY0, (uint16_t)kPageSize, (uint16_t)rows, mem, (uint16_t)kPageSize);
        page.DirtyY0 = 1 << 30;
        page.DirtyY1 = -1;
    }
}

size_t UiFontLibrary::FaceCount() { return Faces().size(); }
size_t UiFontLibrary::GlyphCount() { return Cache().size(); }
