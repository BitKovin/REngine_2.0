#pragma once

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl2.h"
#include "imgui/imgui_impl_bgfx.h"
#include "Input.h"
#include <imgui/ImGuizmo.h>
#include "gl.h"

#include <SDL2/SDL.h>

// True between ImGui::NewFrame() (in ImStartFrame) and the end of RenderImGui().
inline bool& ImFrameIsOpen()
{
    static bool open = false;
    return open;
}

// With multi-viewports ImGui requires ImGui::UpdatePlatformWindows() after every frame it began.
// If a frame was started but never rendered (e.g. the debug UI got switched off between ImStartFrame() and
// RenderImGui()), close it properly so the next ImGui::NewFrame() doesn't assert.
inline void ImAbandonFrame()
{
    if (!ImFrameIsOpen())
        return;

    ImFrameIsOpen() = false;
    ImGui::EndFrame();
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        ImGui::UpdatePlatformWindows();
}

inline void ImStartFrame()
{
    ImAbandonFrame();

    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    if (Input::LockCursor)
    {
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoKeyboard;
    }
    else
    {
        ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
        ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoKeyboard;
    }

    // Start the Dear ImGui frame
    ImGui_ImplSDL2_NewFrame();      // ← stays the same
    ImGui_Implbgfx_NewFrame();      // ← was ImGui_ImplOpenGL3_NewFrame()


    ImGui::NewFrame();
    ImFrameIsOpen() = true;
    ImGuizmo::BeginFrame();

    ImGui::PushStyleColor(ImGuiCol_DockingEmptyBg, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
    ImGui::PopStyleColor(2);
}

// Must run before bgfx::frame(): every viewport's draw calls are submitted to bgfx here.
inline void RenderImGui()
{
    if (!ImFrameIsOpen())
        return; // no matching ImStartFrame()
    ImFrameIsOpen() = false;

    // Rendering
    ImGui::Render();

    // Main viewport (the game window)
    ImGui_Implbgfx_RenderDrawLists(ImGui::GetDrawData());

    // Floating windows: create/move/resize/destroy the OS windows, then render into them
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

// Floating ImGui windows are real OS windows. While the debug UI is switched off nothing updates or renders them anymore,
// so hide them, and show them again once it comes back. Cheap: only does work when the state changes.
// Call once per frame, before the `if (DebugUiEnabled) ImStartFrame();` in EngineMain::MainLoop():
//     ImSetFloatingWindowsVisible(DebugUiEnabled);
// Does nothing on platforms where multi-viewport is not enabled (Android, Emscripten, ...): there are no floating windows.
inline void ImSetFloatingWindowsVisible(bool visible)
{
    if (ImGui::GetCurrentContext() == nullptr || !(ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable))
        return;

    static bool s_visible = true;
    if (s_visible == visible)
        return;
    s_visible = visible;

    ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
    for (int i = 1; i < platform_io.Viewports.Size; i++) // 0 = main viewport (the game window)
    {
        SDL_Window* sdl_window = SDL_GetWindowFromID((Uint32)(intptr_t)platform_io.Viewports[i]->PlatformHandle);
        if (sdl_window == nullptr)
            continue;
        if (visible)
        {
#if SDL_VERSION_ATLEAST(2,0,18)
            SDL_SetHint(SDL_HINT_WINDOW_NO_ACTIVATION_WHEN_SHOWN, "1"); // don't steal focus from the game window
#endif
            SDL_ShowWindow(sdl_window);
#if SDL_VERSION_ATLEAST(2,0,18)
            SDL_SetHint(SDL_HINT_WINDOW_NO_ACTIVATION_WHEN_SHOWN, "0");
#endif
        }
        else
        {
            SDL_HideWindow(sdl_window);
        }
    }
}
