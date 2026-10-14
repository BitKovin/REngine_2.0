#pragma once

#include <Combat/CombatTypes.h>

#include <EntityHandle.h>
#include <glm.h>

#include <array>
#include <cstdint>

// One virtual button. Times are Time::GameTime (so they freeze with pause, like moves do).
struct ButtonState
{
	bool held = false;
	bool pressed = false;        // edge this frame
	bool released = false;       // edge this frame
	double pressTime = -1000.0;
	double releaseTime = -1000.0;
	uint32_t pressId = 0;        // increments on every press; lets consumers mark a press as used
};

// What a controller wants a body to do this frame (COMBAT_DESIGN.md §4.1). Written by the controller at the
// start of the body's Update, read by the move runner, movement and behaviours.
struct CharacterInput
{
	glm::vec3 move = glm::vec3(0.0f);                 // world space, horizontal, length <= 1
	glm::vec3 aimOrigin = glm::vec3(0.0f);            // where aiming starts (camera for the player)
	glm::vec3 aimDirection = glm::vec3(0.0f, 0.0f, 1.0f);
	EntityHandle focus;                               // preferred target (AI target / player override)
	std::array<ButtonState, static_cast<size_t>(InputButton::Count)> buttons;

	const ButtonState& Get(InputButton button) const { return buttons[static_cast<size_t>(button)]; }

	// Clears per-frame edges. Called by the body before asking its controller for input.
	void BeginFrame()
	{
		for (ButtonState& b : buttons)
		{
			b.pressed = false;
			b.released = false;
		}
	}

	// Feeds the button's physical state for this frame.
	void SetButton(InputButton button, bool down, double now)
	{
		ButtonState& b = buttons[static_cast<size_t>(button)];
		if (down && !b.held)
		{
			b.held = true;
			b.pressed = true;
			b.pressTime = now;
			++b.pressId;
		}
		else if (!down && b.held)
		{
			b.held = false;
			b.released = true;
			b.releaseTime = now;
		}
	}

	// A press and release that both happened within this frame (very short tap, or an AI tap).
	void Tap(InputButton button, double now)
	{
		SetButton(button, true, now);
		SetButton(button, false, now);
	}

	// Releases every button (possession change, death, pause).
	void ReleaseAll(double now)
	{
		for (size_t i = 0; i < buttons.size(); ++i)
			SetButton(static_cast<InputButton>(i), false, now);
		move = glm::vec3(0.0f);
	}
};
