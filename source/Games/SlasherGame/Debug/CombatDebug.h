#pragma once

class PlayerController;

#ifndef DISTRIBUTION
// ImGui combat panel (COMBAT_DESIGN.md §11): body inspector with the move timeline, live move tuning with
// Apply (validated, adopted at the next move boundary) and Save to JSON, data reload, god mode, possession.
// Drawn from PlayerController::UpdateDebugUI while the engine debug UI is enabled.
void DrawCombatDebugPanel(PlayerController& player);
#endif
