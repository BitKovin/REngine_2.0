#pragma once
#include <UI/Render/UiRenderTypes.h>
#include <vector>

struct UiRampStop
{
    glm::vec4 Color;   // straight RGBA
    float Pos;         // 0..1, non-decreasing
};

// One 256-entry row per distinct stop list, in a 256 x 256 RGBA8 texture. The CPU keeps every stop list under a stable id
// (what recorded primitives store); the id turns into a row only when a primitive is added to a frame (Resolve), so rows can be
// reused by least-recent use without invalidating recorded draw data.
namespace UiGradientRamps
{
    void Init();
    void Shutdown();

    // Stable id for a stop list (identical lists share an id). -1 when the stop list is empty or the id table is full.
    int Id(const std::vector<UiRampStop>& stops);
    // The texture row holding the ramp of `id`, uploading it on first use; the row cannot be reused during this frame.
    // -1 when all 256 rows are in use this frame (logs once per frame).
    int Resolve(int id);

    void UploadPending();            // before submitting draws that sample the texture
    void OnBeforeBgfxFrame();        // new frame: rows touched before may be reused
    bgfx::TextureHandle Texture();
    int ResidentRows();
}
