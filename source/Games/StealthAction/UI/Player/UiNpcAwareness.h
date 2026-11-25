#pragma once

// What NPCs know of the player, on the HUD (NPC_AI_DESIGN.md §18.1): over every visible NPC whose awareness meter for
// this player is above 0 a "?" that fills from white to yellow (Notice to Suspect) and turns into a red "!" at Identify,
// with the meter itself as a thin bar under it; for NPCs off the screen within 30 m an arrow at the screen edge in the
// same colour. The numbers shown are the meters themselves.

#include <UI/UiCanvas.hpp>
#include <UI/UiImage.hpp>
#include <UI/UiText.hpp>

class Player;

class UiNpcAwareness : public UiCanvas
{
public:

	// Rebuilds the markers for this frame (main thread, after the NPCs' LateUpdate).
	void Refresh(Player* player);

private:

	static constexpr float ArrowRange = 30.0f;
	static constexpr float ArrowMargin = 70.0f;

	void AddMarker(const glm::vec2& screen, float value, float distance);
	void AddArrow(const glm::vec2& viewport, const glm::vec2& direction, float value);
};
