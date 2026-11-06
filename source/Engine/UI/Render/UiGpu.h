#pragma once
#include <UI/Render/UiDrawList.h>
#include <UI/Render/UiPrimBuffer.h>

class Shader;

namespace UiGpu
{
    void Init();        // called from UiRenderer::Init()
    void Shutdown();    // called from UiRenderer::Shutdown()
    UiPrimBuffer& Prims();
    bgfx::TextureHandle WhiteTexture();

    // Adds a program to the table (phase 3 effect programs). Returns its index for BatchKey::Program, or 0 when it is not valid.
    uint8_t RegisterProgram(const char* vs, const char* fs);

    // Exact bgfx state values the legacy code produces (computed once in Init).
    uint64_t StateLegacyPremultiplied();   // SubmitQuad(): Premultiplied blend
    uint64_t StateLegacyAlpha();           // DrawTexturedRectRegion/9Slice: Alpha blend
    uint64_t StateLegacyText();            // DrawText: literal flags

    struct Projection { float Width = 0.f, Height = 0.f; };
    Projection CurrentProjection();        // same numbers SetShaderProjection uses (customViewport aware)

    // Submits the list into `viewId` and clears it.
    void Submit(UiDrawList& list, uint16_t viewId, const Projection& projection);

    struct Stats { uint32_t Passes, Prims, Vertices, GeometryDraws, Externals; uint32_t Breaks[6]; };
    const Stats& LastFrameStats();
    void OnBeforeBgfxFrame();              // copies running stats to "last frame", resets prims
}
