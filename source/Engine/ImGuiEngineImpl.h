#pragma once

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl2.h"
#include "imgui/imgui_impl_opengl3.h"
#include "Input.h"
#include "gl.h"

inline void ImStartFrame()
{

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
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();

    ImGui::NewFrame();

    ImGui::PushStyleColor(ImGuiCol_DockingEmptyBg, ImVec4(0,0,0,0));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
    ImGui::DockSpaceOverViewport();
    ImGui::PopStyleColor(2);
}

inline void RenderImGui()
{
    // Rendering
    ImGui::Render();

    glUseProgram(0); // You may want this if using this code in an OpenGL 3+ context where shaders may be bound
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}