#pragma once
#include <UI/Render/UiRenderTypes.h>
#include <vector>

class UiPrimBuffer
{
public:
    void Init();      // creates the RGBA32F texture (point sampling, clamp); logs an error if unsupported
    void Shutdown();
    // Returns the frame-global primitive index, or -1 when the frame is full (logs once per frame).
    int  Add(const UiRender::PrimBlock& block);
    // Uploads every row touched since the last upload. Call right before submitting draws that use them.
    void UploadPending();
    // Starts a new frame. Only UiRenderer::OnBeforeBgfxFrame() may call this (conventions §3).
    void ResetFrame();
    bgfx::TextureHandle Texture() const { return m_texture; }
    int Count() const { return m_count; }

private:
    bgfx::TextureHandle m_texture = BGFX_INVALID_HANDLE;
    std::vector<glm::vec4> m_cpu;   // kDataWidth * kDataHeight texels, mirror of the whole texture
    int m_count = 0;                // primitives added this frame
    int m_uploadedCount = 0;        // primitives already covered by an upload this frame
    bool m_loggedFull = false;
};
