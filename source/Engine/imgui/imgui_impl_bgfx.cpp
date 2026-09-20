// Derived from this Gist by Richard Gale:
//     https://gist.github.com/RichardGale/6e2b74bc42b3005e08397236e4be0fd0

// ImGui bgfx binding (Renderer backend), with optional Multi-Viewport support.
//
// Implemented features:
//  [X] Renderer: User texture binding. Use 'bgfx::TextureHandle::idx' as ImTextureID.
//  [X] Renderer: Multi-viewport support (multiple windows). OFF by default, opt-in per platform (see below).
//
// Usage:
//  - Init:      ImGui_Implbgfx_Init(view)          (after ImGui::CreateContext(), the platform backend init and bgfx::init())
//  - Each frame ImGui_Implbgfx_NewFrame()          (before ImGui::NewFrame())
//               ImGui::Render();
//               ImGui_Implbgfx_RenderDrawLists(ImGui::GetDrawData());
//               ImGui::UpdatePlatformWindows();    (only when ImGuiConfigFlags_ViewportsEnable is set)
//               ImGui::RenderPlatformWindowsDefault();
//               ... and then bgfx::frame() as usual. Everything above must happen BEFORE bgfx::frame().
//  - Shutdown:  ImGui_Implbgfx_Shutdown()          (before ImGui_ImplSDL2_Shutdown())
//
// Multi-viewport is OPT-IN. A platform main enables it explicitly with BOTH of:
//      io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
//      ImGui_Implbgfx_Init(view, /*multi_viewport=*/true);
// Platforms that can't open extra native windows (Android, Emscripten, ...) simply keep calling ImGui_Implbgfx_Init(view):
// no viewport handlers are installed, no swap chains and no extra view ids are ever used.
//
// Multi-viewport notes:
//  - Every secondary ImGui viewport (= OS window) gets its own bgfx swap chain (window frame buffer) and its own view id.
//    View ids are taken from a small pool right below the main ImGui view, counting downwards
//    (main view 255 -> secondary views 254, 253, ...). Make sure the engine's own views never use those ids.
//  - bgfx presents every swap chain inside bgfx::frame(), so there is no Renderer_SwapBuffers handler.
//  - Resizing is done in place with bgfx::updateSwapChain().
//  - Requires a bgfx renderer that reports BGFX_CAPS_SWAP_CHAIN (D3D11, D3D12, Vulkan, OpenGL/WGL...). Otherwise
//    ImGui silently falls back to single-viewport mode.
//  - The native window handle comes from ImGuiViewport::PlatformHandleRaw (HWND on Windows), which the SDL2 backend
//    fills in on Windows and macOS.

#include "imgui_impl_bgfx.h"
#include "imgui.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

// BGFX/BX
#include "bgfx/bgfx.h"
#include "bgfx/embedded_shader.h"
#include "bx/math.h"
#include "bx/timer.h"

#include <ShaderManager.h>

// Number of secondary viewports (floating OS windows) that can be rendered at the same time.
// Their view ids are (main view - 1) downwards, so with the main view at 255 and 8 windows, ids 247..254 are reserved.
#ifndef IMGUI_IMPL_BGFX_MAX_SECONDARY_VIEWS
#define IMGUI_IMPL_BGFX_MAX_SECONDARY_VIEWS 8
#endif

// Data
static uint8_t g_View = 255;
static bgfx::TextureHandle g_FontTexture = BGFX_INVALID_HANDLE;
static bgfx::ProgramHandle g_ShaderHandle = BGFX_INVALID_HANDLE;
static bgfx::UniformHandle g_AttribLocationTex = BGFX_INVALID_HANDLE;
static bgfx::VertexLayout g_VertexLayout;
static bool g_MultiViewport = false; // opt-in, see ImGui_Implbgfx_Init()
static bool g_SecondaryViewUsed[IMGUI_IMPL_BGFX_MAX_SECONDARY_VIEWS] = {};

// Per-viewport data, stored in ImGuiViewport::RendererUserData (secondary viewports only).
struct ImGui_Implbgfx_ViewportData
{
    bgfx::FrameBufferHandle Fbh;        // Window frame buffer (swap chain)
    int                     ViewId;     // bgfx view used to draw into Fbh (-1 = none)
    uint16_t                Width;      // Swap chain size in pixels
    uint16_t                Height;

    ImGui_Implbgfx_ViewportData()
    {
        Fbh = BGFX_INVALID_HANDLE;
        ViewId = -1;
        Width = Height = 0;
    }
};

static int ImGui_Implbgfx_AllocSecondaryView()
{
    for (int i = 0; i < IMGUI_IMPL_BGFX_MAX_SECONDARY_VIEWS; i++)
    {
        const int view_id = (int)g_View - 1 - i;
        if (view_id < 0)
            break;
        if (!g_SecondaryViewUsed[i])
        {
            g_SecondaryViewUsed[i] = true;
            return view_id;
        }
    }
    return -1;
}

static void ImGui_Implbgfx_FreeSecondaryView(int view_id)
{
    const int i = (int)g_View - 1 - view_id;
    if (i >= 0 && i < IMGUI_IMPL_BGFX_MAX_SECONDARY_VIEWS)
        g_SecondaryViewUsed[i] = false;
}

static uint16_t ImGui_Implbgfx_ToPixels(float v)
{
    return (uint16_t)bx::clamp(v, 1.0f, 65535.0f);
}

static bgfx::SwapChain ImGui_Implbgfx_MakeSwapChain(void* nwh, uint16_t width, uint16_t height)
{
    bgfx::SwapChain swap_chain;
    swap_chain.nwh = nwh;
    swap_chain.width = width;
    swap_chain.height = height;
    swap_chain.flags = BGFX_SWAP_CHAIN_NONE; // vsync etc. are global (bgfx::reset flags), same as for the main window
    // formatColor / formatDepthStencil left at their defaults: default color format, no depth (ImGui doesn't need it).
    return swap_chain;
}

// Renders one ImDrawData (main viewport or secondary viewport) into 'view_id'.
// fb_width/fb_height: size in pixels of the frame buffer the view renders into.
static void ImGui_Implbgfx_RenderDrawData(ImDrawData* draw_data, bgfx::ViewId view_id, int fb_width, int fb_height)
{
    if (fb_width <= 0 || fb_height <= 0 || draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f)
        return;

    // Setup render state: alpha-blending enabled, no face culling,
    // no depth testing, scissor enabled
    const uint64_t state =
        BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_MSAA |
        BGFX_STATE_BLEND_FUNC(
            BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA);

    const bgfx::Caps* caps = bgfx::getCaps();

    // Setup viewport, orthographic projection matrix
    // Our visible imgui space lies from draw_data->DisplayPos (top left) to draw_data->DisplayPos + draw_data->DisplaySize (bottom right).
    // DisplayPos is (0,0) for single viewport apps, but with multi-viewports all coordinates are absolute (screen) coordinates.
    const float L = draw_data->DisplayPos.x;
    const float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
    const float T = draw_data->DisplayPos.y;
    const float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;

    float ortho[16];
    bx::mtxOrtho(ortho, L, R, B, T, 0.0f, 1000.0f, 0.0f, caps->homogeneousDepth);
    bgfx::setViewTransform(view_id, NULL, ortho);
    bgfx::setViewRect(view_id, 0, 0, (uint16_t)fb_width, (uint16_t)fb_height);

    // Will project scissor/clipping rectangles into framebuffer space
    const ImVec2 clip_off = draw_data->DisplayPos;
    const ImVec2 clip_scale(
        (float)fb_width / draw_data->DisplaySize.x,
        (float)fb_height / draw_data->DisplaySize.y);

    // Render command lists
    for (int n = 0; n < draw_data->CmdListsCount; n++) {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];

        bgfx::TransientVertexBuffer tvb;
        bgfx::TransientIndexBuffer tib;

        uint32_t numVertices = (uint32_t)cmd_list->VtxBuffer.size();
        uint32_t numIndices = (uint32_t)cmd_list->IdxBuffer.size();

        if ((numVertices != bgfx::getAvailTransientVertexBuffer(
                                numVertices, g_VertexLayout)) ||
            (numIndices != bgfx::getAvailTransientIndexBuffer(numIndices))) {
            // not enough space in transient buffer, quit drawing the rest...
            break;
        }

        bgfx::allocTransientVertexBuffer(&tvb, numVertices, g_VertexLayout);
        bgfx::allocTransientIndexBuffer(&tib, numIndices);

        ImDrawVert* verts = (ImDrawVert*)tvb.data;
        memcpy(
            verts, cmd_list->VtxBuffer.begin(),
            numVertices * sizeof(ImDrawVert));

        ImDrawIdx* indices = (ImDrawIdx*)tib.data;
        memcpy(
            indices, cmd_list->IdxBuffer.begin(),
            numIndices * sizeof(ImDrawIdx));

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];

            if (pcmd->UserCallback) {
                // ImDrawCallback_ResetRenderState is a special value asking us to reset our render state.
                // We set the full state on every draw call below, so there is nothing to reset.
                if (pcmd->UserCallback != ImDrawCallback_ResetRenderState)
                    pcmd->UserCallback(cmd_list, pcmd);
                continue;
            }

            // Project scissor/clipping rectangles into framebuffer space
            float clip_min_x = (pcmd->ClipRect.x - clip_off.x) * clip_scale.x;
            float clip_min_y = (pcmd->ClipRect.y - clip_off.y) * clip_scale.y;
            float clip_max_x = (pcmd->ClipRect.z - clip_off.x) * clip_scale.x;
            float clip_max_y = (pcmd->ClipRect.w - clip_off.y) * clip_scale.y;
            clip_min_x = bx::clamp(clip_min_x, 0.0f, (float)fb_width);
            clip_min_y = bx::clamp(clip_min_y, 0.0f, (float)fb_height);
            clip_max_x = bx::clamp(clip_max_x, 0.0f, (float)fb_width);
            clip_max_y = bx::clamp(clip_max_y, 0.0f, (float)fb_height);
            if (clip_max_x <= clip_min_x || clip_max_y <= clip_min_y)
                continue;

            const uint16_t xx = (uint16_t)clip_min_x;
            const uint16_t yy = (uint16_t)clip_min_y;
            bgfx::setScissor(
                xx, yy, (uint16_t)clip_max_x - xx, (uint16_t)clip_max_y - yy);

            bgfx::setState(state);
            bgfx::TextureHandle texture = {
                (uint16_t)((intptr_t)pcmd->GetTexID() & 0xffff)};
            bgfx::setTexture(0, g_AttribLocationTex, texture);
            bgfx::setVertexBuffer(0, &tvb, 0, numVertices);
            bgfx::setIndexBuffer(&tib, pcmd->IdxOffset, pcmd->ElemCount);
            bgfx::submit(view_id, g_ShaderHandle);
        }
    }
}

// This is the main rendering function that you have to implement and call after
// ImGui::Render(). Pass ImGui::GetDrawData() to this function.
// It renders the MAIN viewport into the back buffer. Secondary viewports are rendered by
// ImGui::RenderPlatformWindowsDefault(), through the Renderer_RenderWindow handler installed by ImGui_Implbgfx_Init().
// Note: If text or lines are blurry when integrating ImGui into your engine,
// in your Render function, try translating your projection matrix by
// (0.5f,0.5f) or (0.375f,0.375f)
void ImGui_Implbgfx_RenderDrawLists(ImDrawData* draw_data)
{
    if (draw_data == NULL) {
        return;
    }

    // Avoid rendering when minimized, scale coordinates for retina displays
    // (screen coordinates != framebuffer coordinates)
    int fb_width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
    int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
    if (fb_width == 0 || fb_height == 0) {
        return;
    }

    ImGui_Implbgfx_RenderDrawData(draw_data, g_View, fb_width, fb_height);
}

//--------------------------------------------------------------------------------------------------------
// MULTI-VIEWPORT / PLATFORM INTERFACE SUPPORT
// This is an _advanced_ and _optional_ feature, allowing the backend to create and handle multiple viewports simultaneously.
// If you are new to dear imgui or creating a new binding for dear imgui, it is recommended that you completely ignore this section first..
//--------------------------------------------------------------------------------------------------------

static void ImGui_Implbgfx_CreateWindow(ImGuiViewport* viewport)
{
    ImGui_Implbgfx_ViewportData* vd = IM_NEW(ImGui_Implbgfx_ViewportData)();
    viewport->RendererUserData = vd;

    void* nwh = viewport->PlatformHandleRaw;
    if (nwh == NULL) {
        fprintf(stderr, "imgui_impl_bgfx: viewport has no native window handle (PlatformHandleRaw), it won't be rendered.\n");
        return;
    }

    vd->ViewId = ImGui_Implbgfx_AllocSecondaryView();
    if (vd->ViewId < 0) {
        fprintf(stderr, "imgui_impl_bgfx: out of view ids for secondary viewports (IMGUI_IMPL_BGFX_MAX_SECONDARY_VIEWS = %d), viewport won't be rendered.\n", IMGUI_IMPL_BGFX_MAX_SECONDARY_VIEWS);
        return;
    }

    const ImVec2 fb_scale = ImGui::GetIO().DisplayFramebufferScale;
    vd->Width = ImGui_Implbgfx_ToPixels(viewport->Size.x * fb_scale.x);
    vd->Height = ImGui_Implbgfx_ToPixels(viewport->Size.y * fb_scale.y);
    vd->Fbh = bgfx::createFrameBuffer(ImGui_Implbgfx_MakeSwapChain(nwh, vd->Width, vd->Height));
    bgfx::setViewName((bgfx::ViewId)vd->ViewId, "ImGui viewport");
}

static void ImGui_Implbgfx_DestroyWindow(ImGuiViewport* viewport)
{
    // The main viewport doesn't have RendererUserData (it renders into the back buffer).
    if (ImGui_Implbgfx_ViewportData* vd = (ImGui_Implbgfx_ViewportData*)viewport->RendererUserData) {
        if (bgfx::isValid(vd->Fbh)) {
            // Don't leave the view pointing at a dead frame buffer. bgfx tolerates the OS window
            // going away before the swap chain is released (Present errors are ignored / the surface is recreated).
            bgfx::setViewFrameBuffer((bgfx::ViewId)vd->ViewId, BGFX_INVALID_HANDLE);
            bgfx::destroy(vd->Fbh);
        }
        if (vd->ViewId >= 0) {
            ImGui_Implbgfx_FreeSecondaryView(vd->ViewId);
        }
        IM_DELETE(vd);
    }
    viewport->RendererUserData = NULL;
}

static void ImGui_Implbgfx_SetWindowSize(ImGuiViewport* viewport, ImVec2 size)
{
    ImGui_Implbgfx_ViewportData* vd = (ImGui_Implbgfx_ViewportData*)viewport->RendererUserData;
    if (vd == NULL || !bgfx::isValid(vd->Fbh)) {
        return;
    }

    const ImVec2 fb_scale = ImGui::GetIO().DisplayFramebufferScale;
    const uint16_t width = ImGui_Implbgfx_ToPixels(size.x * fb_scale.x);
    const uint16_t height = ImGui_Implbgfx_ToPixels(size.y * fb_scale.y);
    if (width == vd->Width && height == vd->Height) {
        return;
    }

    vd->Width = width;
    vd->Height = height;
    bgfx::updateSwapChain(vd->Fbh, ImGui_Implbgfx_MakeSwapChain(viewport->PlatformHandleRaw, width, height));
}

static void ImGui_Implbgfx_RenderWindow(ImGuiViewport* viewport, void*)
{
    ImGui_Implbgfx_ViewportData* vd = (ImGui_Implbgfx_ViewportData*)viewport->RendererUserData;
    if (vd == NULL || !bgfx::isValid(vd->Fbh) || viewport->DrawData == NULL) {
        return;
    }

    const bgfx::ViewId view_id = (bgfx::ViewId)vd->ViewId;
    bgfx::setViewFrameBuffer(view_id, vd->Fbh);
    bgfx::setViewMode(view_id, bgfx::ViewMode::Sequential);
    if (viewport->Flags & ImGuiViewportFlags_NoRendererClear) {
        bgfx::setViewClear(view_id, BGFX_CLEAR_NONE);
    } else {
        bgfx::setViewClear(view_id, BGFX_CLEAR_COLOR, 0x000000ff, 1.0f, 0);
    }

    // Make sure the view is cleared (and the swap chain presented) even when the draw data is empty.
    bgfx::setViewRect(view_id, 0, 0, vd->Width, vd->Height);
    bgfx::touch(view_id);

    ImGui_Implbgfx_RenderDrawData(viewport->DrawData, view_id, vd->Width, vd->Height);
}

static void ImGui_Implbgfx_InitMultiViewportSupport()
{
    ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
    platform_io.Renderer_CreateWindow = ImGui_Implbgfx_CreateWindow;
    platform_io.Renderer_DestroyWindow = ImGui_Implbgfx_DestroyWindow;
    platform_io.Renderer_SetWindowSize = ImGui_Implbgfx_SetWindowSize;
    platform_io.Renderer_RenderWindow = ImGui_Implbgfx_RenderWindow;
    // No Renderer_SwapBuffers: bgfx presents all swap chains in bgfx::frame().
}

static void ImGui_Implbgfx_ShutdownMultiViewportSupport()
{
    // Calls Renderer_DestroyWindow (and Platform_DestroyWindow) for every viewport.
    if (ImGui::GetCurrentContext() != NULL) {
        ImGui::DestroyPlatformWindows();
    }
}

//--------------------------------------------------------------------------------------------------------

bool ImGui_Implbgfx_CreateFontsTexture()
{
    // Build texture atlas
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    // Upload texture to graphics system
    g_FontTexture = bgfx::createTexture2D(
        (uint16_t)width, (uint16_t)height, false, 1, bgfx::TextureFormat::BGRA8,
        0, bgfx::copy(pixels, width * height * 4));

    // Store our identifier
    io.Fonts->TexID = (intptr_t)g_FontTexture.idx;

    return true;
}


bool ImGui_Implbgfx_CreateDeviceObjects()
{
    g_ShaderHandle = ShaderManager::GetShaderProgram("imgui/vs_ocornut_imgui", "imgui/fs_ocornut_imgui")->GetProgram();

    g_VertexLayout.begin()
        .add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
        .end();

    g_AttribLocationTex =
        bgfx::createUniform("g_AttribLocationTex", bgfx::UniformType::Sampler);

    ImGui_Implbgfx_CreateFontsTexture();

    return true;
}

void ImGui_Implbgfx_InvalidateDeviceObjects()
{
    //handled by shader manager
    //bgfx::destroy(g_AttribLocationTex);
    //bgfx::destroy(g_ShaderHandle);

    if (isValid(g_FontTexture)) {
        bgfx::destroy(g_FontTexture);
        ImGui::GetIO().Fonts->TexID = 0;
        g_FontTexture.idx = bgfx::kInvalidHandle;
    }
}

void ImGui_Implbgfx_Init(int view, bool multi_viewport)
{
    g_View = (uint8_t)(view & 0xff);
    g_MultiViewport = multi_viewport;

    ImGuiIO& io = ImGui::GetIO();
    io.BackendRendererName = "imgui_impl_bgfx";

    // Opt-in only: without it no viewport handlers are installed and ImGuiBackendFlags_RendererHasViewports is never set,
    // so ImGui can't create any secondary viewport, even if some platform main sets ImGuiConfigFlags_ViewportsEnable by mistake.
    // (ImGuiBackendFlags_RendererHasViewports is set in ImGui_Implbgfx_NewFrame(), depending on what the bgfx renderer supports.)
    if (g_MultiViewport) {
        ImGui_Implbgfx_InitMultiViewportSupport();
    }
}

void ImGui_Implbgfx_Shutdown()
{
    if (g_MultiViewport) {
        ImGui_Implbgfx_ShutdownMultiViewportSupport();
    }
    ImGui_Implbgfx_InvalidateDeviceObjects();

    if (ImGui::GetCurrentContext() != NULL) {
        ImGuiIO& io = ImGui::GetIO();
        io.BackendRendererName = NULL;
        io.BackendFlags &= ~ImGuiBackendFlags_RendererHasViewports;
    }
    g_MultiViewport = false;
}

void ImGui_Implbgfx_NewFrame()
{
    // Multi-viewport (opt-in) also needs multiple-window (swap chain) support from the bgfx renderer that is in use.
    // ImGui disables ViewportsEnable on its own when this flag is missing.
    if (g_MultiViewport) {
        ImGuiIO& io = ImGui::GetIO();
        if (bgfx::getCaps()->supported & BGFX_CAPS_SWAP_CHAIN) {
            io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;
        } else {
            io.BackendFlags &= ~ImGuiBackendFlags_RendererHasViewports;
        }
    }

    if (!isValid(g_FontTexture)) {
        ImGui_Implbgfx_CreateDeviceObjects();
    }
}
