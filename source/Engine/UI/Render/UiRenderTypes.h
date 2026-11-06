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
    constexpr int kAuxFirstRow  = 224;                              // rows 224..255: auxiliary texels
    constexpr int kAuxTexels    = (kDataHeight - kAuxFirstRow) * kDataWidth;   // 32768 per frame

    // Values of texel 0 .x ("kind"). Must match fs_ui2.sc.
    enum class PrimKind : int
    {
        Image     = 0,   // texture * colour (legacy textured quads and legacy text)
        Box       = 1,   // SDF rounded box with fill and per-side border
        SdfGlyph  = 2,
        BoxShadow = 3,   // only in the shadow program (ui2/fs_ui2_shadow)
        Arc       = 4,
        Solid     = 5,   // plain colour; coverage comes from the vertex (v_local.x), for tessellated paths
    };

    // Bits of texel 0 .y ("flags"). Must match fs_ui2.sc uiFlag() calls.
    constexpr int kFlagClip       = 1;   // texels 9/10 hold a clip rect and radii in UI units
    constexpr int kFlagHasTexture = 2;   // Box: multiply fill by the batch texture
    constexpr int kFlagMask       = 4;   // T15 = (type, ramp, aux index, ...): linear or radial mask
    constexpr int kFlagClipShape  = 8;   // T15.w = aux index of a circle / ellipse clip-path
    constexpr int kFlagPixelated  = 16;  // Image / Box: nearest sampling; T13.xy = texture size in texels

    // One primitive's data. Texel meaning:
    //  T0  kind, flags, unused, colour-matrix aux index (-1 = none)
    //  T1  fill / tint colour, straight RGBA
    //  T2  rect width, rect height, AA width (local units per target pixel; 1 when unscaled), param
    //  T3  corner radii: top-left, top-right, bottom-right, bottom-left
    //  T4  border widths: top, right, bottom, left
    //  T5  border colour top      T6 right      T7 bottom      T8 left   (straight RGBA)
    //  T9  clip rect in UI units: x0, y0, x1, y1
    //  T10 clip radii: top-left, top-right, bottom-right, bottom-left
    //  T11 gradient header (type 0 none / 1 linear / 2 radial / 3 conic, ramp id then row, repeat or tile axes, tiled 0/1)
    //  T12 gradient geometry, or the texture sub-rectangle of a tiled fill (u0, v0, u1, v1)
    //  T13 glyph (scale, dilation, softness, stroke width), BoxShadow (.z = inset), Arc (start, sweep, 0, round caps), pixelated texture size
    //  T14 glyph stroke colour   T15 mask (type, ramp id then row, aux index, clip-shape aux index)
    struct PrimBlock
    {
        glm::vec4 T[kPrimTexels] = {};

        PrimBlock() { T[0] = glm::vec4(0.f, 0.f, 0.f, -1.f); T[15] = glm::vec4(0.f, 0.f, 0.f, -1.f); }
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
