#pragma once
#include <glm.h>
#include <bgfx/bgfx.h>
#include <cstdint>

namespace UiRender
{
    // Primitive data texture: RGBA32F, kDataWidth x kDataHeight texels.
    constexpr int kDataWidth    = 1024;
    constexpr int kDataHeight   = 256;
    constexpr int kPrimTexels   = 16;
    constexpr int kPrimsPerRow  = kDataWidth / kPrimTexels;        // 64
    constexpr int kPrimRows     = 224;                              // rows 0..223 hold primitives
    constexpr int kMaxPrims     = kPrimRows * kPrimsPerRow;         // 14336 per frame
    constexpr int kAuxFirstRow  = 224;                              // rows 224..255: auxiliary data [P3]

    // Values of texel 0 .x ("kind"). Must match fs_ui2.sc.
    enum class PrimKind : int
    {
        Image     = 0,   // texture * colour (legacy textured quads and legacy text)
        Box       = 1,   // SDF rounded box with fill and per-side border
        SdfGlyph  = 2,   // [P1]
        BoxShadow = 3,   // [P3]
        Arc       = 4,   // [P3]
        Solid     = 5,   // [P3] plain colour, for tessellated paths
    };

    // Bits of texel 0 .y ("flags"). Must match fs_ui2.sc uiFlag() calls.
    constexpr int kFlagClip       = 1;   // texels 9/10 hold a clip rect and radii in UI units
    constexpr int kFlagHasTexture = 2;   // Box: multiply fill by the batch texture

    // One primitive's data. Texel meaning:
    //  T0  kind, flags, unused, aux index (-1 = none) [P3]
    //  T1  fill / tint colour, straight RGBA
    //  T2  rect width, rect height, AA width (local units per target pixel; 1 when unscaled), param
    //  T3  corner radii: top-left, top-right, bottom-right, bottom-left
    //  T4  border widths: top, right, bottom, left
    //  T5  border colour top      T6 right      T7 bottom      T8 left   (straight RGBA)
    //  T9  clip rect in UI units: x0, y0, x1, y1
    //  T10 clip radii: top-left, top-right, bottom-right, bottom-left
    //  T11 gradient header [P3]   T12 gradient geometry [P3]
    //  T13 shadow / glyph params [P1/P3]   T14 second colour [P1/P3]   T15 mask [P3]
    struct PrimBlock
    {
        glm::vec4 T[kPrimTexels] = {};

        PrimBlock() { T[0] = glm::vec4(0.f, 0.f, 0.f, -1.f); }
        void SetKind(PrimKind kind) { T[0].x = static_cast<float>(kind); }
        void AddFlag(int flag) { T[0].y = static_cast<float>(static_cast<int>(T[0].y) | flag); }
    };

    // Batch-relevant render state. Two items batch together only when every field is equal.
    struct BatchKey
    {
        uint8_t  Program = 0;                       // index into UiGpu's program table; 0 = ui2/fs_ui2 (base). [P3] adds effect programs
        uint64_t State = 0;                         // full bgfx state flags
        uint16_t TextureIdx = bgfx::kInvalidHandle; // kInvalidHandle = use the white texture
        uint8_t  StencilRef = 0;                    // 0 = no stencil test

        bool operator==(const BatchKey& o) const
        {
            return Program == o.Program && State == o.State && TextureIdx == o.TextureIdx && StencilRef == o.StencilRef;
        }
        bool operator!=(const BatchKey& o) const { return !(*this == o); }
    };

    struct Vertex
    {
        float X, Y;     // position in UI units (after the full transform)
        float U, V;     // texture coordinates
        float LX, LY;   // local position in the primitive's own rect space (for SDF), UI units
        float Prim;     // primitive index (frame-global), as float
        float Pad;      // keep 32 bytes
    };
    static_assert(sizeof(Vertex) == 32, "UiRender::Vertex must stay 32 bytes");
}
