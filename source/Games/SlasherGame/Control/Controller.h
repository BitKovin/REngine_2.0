#pragma once

class CombatCharacter;
struct CharacterInput;

// Decides what a body does (COMBAT_DESIGN.md §4.1). The body calls UpdateInput at the start of its own
// Update, so input is always fresh regardless of entity update order. Main thread only.
class Controller
{
public:
	virtual ~Controller() = default;

	virtual void UpdateInput(CombatCharacter& body, CharacterInput& input, float deltaTime) = 0;

	virtual void OnPossess(CombatCharacter& body) { (void)body; }
	virtual void OnUnpossess(CombatCharacter& body) { (void)body; }

	virtual bool IsPlayerController() const { return false; }
};
