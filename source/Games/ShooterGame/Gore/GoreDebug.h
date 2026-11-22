#pragma once

namespace Gore
{
    // ImGui "Gore" window: shoot the NPC under the crosshair with a chosen source, draw wound volumes, check meshes.
    // Compiled only without DISTRIBUTION (an empty stub otherwise).
    void DrawDebugWindow();

    // Launch-argument test (-gore_look <npcClass> ...): call every frame from the game update.
    void UpdateLookTest();
}
