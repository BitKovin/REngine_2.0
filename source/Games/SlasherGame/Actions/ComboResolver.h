#pragma once

#include <Characters/CharacterInput.h>

#include <cstdint>
#include <limits>

enum class ComboDecision : uint8_t
{
	None,     // keep waiting
	Light,    // start the link's "next"
	Heavy,    // start the link's "alternative"
	Hold,     // start the hold opener (the launcher): the press was held for holdDelay
};

// What a pending Attack press outside a combo window starts when the slot has a hold opener (COMBAT_DESIGN.md §5.4).
// Pure logic.
enum class OpenerDecision : uint8_t
{
	Wait,     // still held, not yet for holdDelay: nothing starts yet
	Opener,   // released early (a tap): the combo opener, on release
	Hold,     // held for holdDelay (still held, or released after it): the hold opener (the launcher)
};

inline OpenerDecision DecideOpener(const ButtonState& button, double now, float holdDelay)
{
	const double heldFor = (button.held ? now : button.releaseTime) - button.pressTime;
	if (heldFor >= holdDelay)
		return OpenerDecision::Hold;
	return button.held ? OpenerDecision::Wait : OpenerDecision::Opener;
}

// How long a press must be held to become the hold opener (DecideOpener's holdDelay). The short holdDelay is only for a
// press made from rest whose hold opener hands back to the usual attack when released during its wind-up (the
// launcher's hold rule): a tap that outlasts the delay still gets the usual attack. A press made during a move, or one
// for a hold opener without that hand-over (the drop commits at once), needs the long holdDelayInMove - a hold the
// player means. A press made on the other side of the ground (in the air, and the body has landed since - or the
// reverse) never becomes this state's hold opener: it waits for its release, the usual attack.
inline float HoldOpenerDelay(bool pressedInMove, bool pressedInThisState, bool handsOver, float holdDelay, float holdDelayInMove)
{
	if (!pressedInThisState)
		return std::numeric_limits<float>::infinity();
	return pressedInMove || !handsOver ? holdDelayInMove : holdDelay;
}

struct ComboTiming
{
	float windowStart = 0.0f;
	float alternativeAt = 0.0f;
	float windowEnd = 0.0f;
	bool hasLight = false;
	bool hasHeavy = false;
	bool hasHold = false;        // the hold opener (the launcher) can start: a long hold becomes it
	float holdDelay = 0.6f;      // s a press must be held to become it
};

// Light/heavy follow-up selection for one running attack (COMBAT_DESIGN.md §5.4). Pure logic, no engine calls.
//
//   clip time: 0 ────── ws ════ LIGHT ════ a ════ HEAVY ════ we ──── end
//
//   tap (press + release) before ws ........ light at ws (released longer than inputBuffer before: dropped)
//   tap released inside [ws, a) ............ light on release
//   button held when the clip reaches a .... heavy at a (includes the hold that started the attack when
//                                            holdCarriesOver is on)
//   press inside [a, we] ................... heavy on press
//
// When a hold opener (the launcher) can start (hasHold), a hold may still become it, so a heavy waits for its press
// to be released: released before holdDelay - the heavy, on release; held for holdDelay (not before ws) - the hold
// opener instead. Meanwhile the link stays open past we (IsArmed). Lights are unaffected - they start on release anyway.
// If the link has only one branch, any qualifying input takes that branch.
class ComboResolver
{
public:
	// A new attack with a combo link started. `button` is the attack button right now; its current press is
	// the one that started the attack and never counts as a new press - with carryHold, held, it counts as held
	// (holdCarriesOver - except for a move the hold itself started: that hold is spent on it).
	void Begin(const ButtonState& button, bool carryHold)
	{
		lastPressId = button.pressId;
		carried = carryHold && button.held;
		pending = carried ? Pending::Held : Pending::None;
		pressClipTime = 0.0f;   // made before this attack
		pressGameTime = button.pressTime;
		releaseClipTime = 0.0f;
		releaseGameTime = button.releaseTime;
		heldPastEnd = false;
		active = true;
	}

	void Reset()
	{
		pending = Pending::None;
		carried = false;
		active = false;
	}

	bool IsActive() const { return active; }

	// Records this frame's button state. Call every frame before Evaluate.
	void Observe(const ButtonState& button, const ComboTiming& timing, float clipTime)
	{
		if (!active)
			return;

		if (button.pressId != lastPressId)
		{
			lastPressId = button.pressId;
			carried = false;   // a new press supersedes the carried hold
			heldPastEnd = false;
			pending = Pending::Held;
			pressClipTime = clipTime;
			pressGameTime = button.pressTime;

			if (!button.held)
			{
				pending = Pending::Released;   // pressed and released within one frame: a tap
				releaseClipTime = clipTime;
				releaseGameTime = button.releaseTime;
			}
		}
		else if (pending == Pending::Held && !button.held)
		{
			// The carried hold let go before the heavy point was only the hold that started the attack: nothing.
			pending = carried && clipTime < timing.alternativeAt ? Pending::None : Pending::Released;
			releaseClipTime = clipTime;
			releaseGameTime = button.releaseTime;
		}

		if (pending == Pending::Held && clipTime > timing.windowEnd)
			heldPastEnd = true;
	}

	ComboDecision Evaluate(const ComboTiming& timing, float clipTime, double now, float inputBuffer) const
	{
		if (!active || pending == Pending::None || clipTime < timing.windowStart || pressClipTime > timing.windowEnd)
			return ComboDecision::None;
		if (clipTime > timing.windowEnd && !heldPastEnd)
			return ComboDecision::None;   // the window closed; only a press still held then (IsArmed) keeps it open

		if (pending == Pending::Held)
		{
			if (timing.hasHold)
				return now - pressGameTime >= timing.holdDelay ? ComboDecision::Hold : ComboDecision::None;
			if (pressClipTime < timing.alternativeAt && clipTime < timing.alternativeAt)
				return ComboDecision::None;   // released before a it is a light, held to a a heavy
			return Pick(ComboDecision::Heavy, timing);
		}

		// Released: pressed late, or still held when the clip reached a - heavy; else a light, unless stale.
		const bool heavy = pressClipTime >= timing.alternativeAt || releaseClipTime >= timing.alternativeAt;
		if (!heavy && now - releaseGameTime > inputBuffer)
			return ComboDecision::None;
		return Pick(heavy ? ComboDecision::Heavy : ComboDecision::Light, timing);
	}

	// A press for this link is held and may still become the hold opener: the attack keeps the link open (no recovery
	// past windowEnd) until it is released or becomes the hold.
	bool IsArmed(const ComboTiming& timing, double now) const
	{
		return active && timing.hasHold && pending == Pending::Held && pressClipTime <= timing.windowEnd
			&& now - pressGameTime < timing.holdDelay;
	}

	// Debug description of the pending input.
	const char* DescribePending() const
	{
		switch (pending)
		{
		case Pending::Held:     return carried ? "carried hold" : "held";
		case Pending::Released: return "released";
		default:                return "none";
		}
	}

private:
	enum class Pending : uint8_t { None, Held, Released };

	static ComboDecision Pick(ComboDecision wanted, const ComboTiming& timing)
	{
		if (wanted == ComboDecision::Light && !timing.hasLight)
			return timing.hasHeavy ? ComboDecision::Heavy : ComboDecision::None;
		if (wanted == ComboDecision::Heavy && !timing.hasHeavy)
			return timing.hasLight ? ComboDecision::Light : ComboDecision::None;
		return wanted;
	}

	Pending pending = Pending::None;
	bool carried = false;          // the pending press is the hold that started the attack (holdCarriesOver)
	bool heldPastEnd = false;      // still held when the window closed: its release still gets the follow-up
	float pressClipTime = 0.0f;
	float releaseClipTime = 0.0f;
	double pressGameTime = 0.0;
	double releaseGameTime = 0.0;
	uint32_t lastPressId = 0;
	bool active = false;
};
