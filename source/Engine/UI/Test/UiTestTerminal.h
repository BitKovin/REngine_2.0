#pragma once

// Engine test code for world surfaces (phase 5): the entity `ui_test_terminal` (a UiWorldSurface with two buttons) and the test commands
//   spawn_terminal [metres]                  1.5 m ahead of the camera, facing it
//   terminal_hover x y | terminal_unhover | terminal_click | terminal_focus enter|exit
//   expect_terminal hits|hits2|aimed|interactive|focused|renders|out_of_range <op> <value>
namespace UiTestTerminal
{
    void RegisterCommands();   // from UiSystem::Init
}
