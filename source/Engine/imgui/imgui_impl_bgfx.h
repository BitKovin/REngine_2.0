// Derived from this Gist by Richard Gale:
//     https://gist.github.com/RichardGale/6e2b74bc42b3005e08397236e4be0fd0

// ImGui BGFX binding (Renderer backend), with Multi-Viewport support (see imgui_impl_bgfx.cpp).

// You can copy and use unmodified imgui_impl_* files in your project. See
// main.cpp for an example of using this. If you use this binding you'll need to
// call 4 functions: ImGui_ImplXXXX_Init(), ImGui_ImplXXXX_NewFrame(),
// ImGui::Render() and ImGui_ImplXXXX_Shutdown(). If you are new to ImGui, see
// examples/README.txt and documentation at the top of imgui.cpp.
// https://github.com/ocornut/imgui

#pragma once

struct ImDrawData;

// 'view' is the bgfx view id used to draw the main viewport (drawn on top of everything, e.g. 255).
// Call after ImGui::CreateContext() and after the platform backend init (e.g. ImGui_ImplSDL2_InitForOther()).
//
// 'multi_viewport' (default: false) opts in to Multi-Viewport support: ImGui windows can be dragged out of the main window
// into their own OS windows, each drawn into its own bgfx swap chain. Only pass true from a platform main that can create
// extra native windows (desktop), and set ImGuiConfigFlags_ViewportsEnable there too. Android, Emscripten, ... keep the default.
// Secondary viewports use the view ids right below 'view', see IMGUI_IMPL_BGFX_MAX_SECONDARY_VIEWS.
void ImGui_Implbgfx_Init(int view, bool multi_viewport = false);
void ImGui_Implbgfx_Shutdown();
void ImGui_Implbgfx_NewFrame();

// Renders the main viewport. Secondary viewports are rendered by ImGui::RenderPlatformWindowsDefault().
void ImGui_Implbgfx_RenderDrawLists(struct ImDrawData* draw_data);

// Use if you want to reset your rendering device without losing ImGui state.
void ImGui_Implbgfx_InvalidateDeviceObjects();
bool ImGui_Implbgfx_CreateDeviceObjects();
