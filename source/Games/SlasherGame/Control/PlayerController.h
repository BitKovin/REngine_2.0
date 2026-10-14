#pragma once

#include <Control/Controller.h>
#include <Control/PlayerCamera.h>
#include <Data/PlayerConfigData.h>

#include <Delay.hpp>
#include <Entity.h>
#include <EntityHandle.h>

#include <memory>
#include <string>

class CombatCharacter;
class CombatHud;

// The local player (COMBAT_DESIGN.md §4.2, §4.4). An engine entity that persists while the player hops between
// bodies: maps InputActions to the controlled body's virtual buttons (camera relative), runs the third-person
// camera and the HUD, and performs possession / release.
class PlayerController : public Entity, public Controller
{
public:
	PlayerController();
	~PlayerController() override;

	static PlayerController* GetActive();

	// ---- Entity ----
	void Start() override;
	void Update() override;
	void LateUpdate() override;
	void Destroy() override;
	void UpdateDebugUI() override;

	// ---- Controller ----
	void UpdateInput(CombatCharacter& body, CharacterInput& input, float deltaTime) override;
	void OnPossess(CombatCharacter& body) override;
	void OnUnpossess(CombatCharacter& body) override;
	bool IsPlayerController() const override { return true; }

	// ---- Bodies ----
	void SetHero(CombatCharacter* hero);          // the player's own body; possessed immediately
	CombatCharacter* GetHero();
	CombatCharacter* GetPossessedBody();
	const PlayerConfigData* GetConfig() const { return config.get(); }

	// ---- Feedback (COMBAT_DESIGN.md §9.4, main thread) ----
	PlayerCamera& GetCamera() { return camera; }
	const PlayerFeedbackData& GetFeedback() const;

	// Takes over another body. Returns false (with a reason) if not allowed.
	bool Possess(CombatCharacter* target, std::string* reason = nullptr);
	// Returns control to the hero; the released body becomes an ally or reverts to its own team / brain.
	void Release(PossessionReleaseMode mode);
	// Possessable body under the crosshair, or nullptr.
	CombatCharacter* FindPossessionCandidate();

	// Dev builds: debug inspector target (the soft target, else the controlled body).
	CombatCharacter* GetDebugFocus();

protected:
	void LoadAssets() override;

private:
	std::shared_ptr<const PlayerConfigData> config;
	PlayerCamera camera;
	std::shared_ptr<CombatHud> hud;

	EntityHandle hero;
	EntityHandle possessed;
	bool playerDied = false;                  // the controlled body and the hero are both dead
	Delay respawnDelay;
	bool restartRequested = false;

	void BindBody(CombatCharacter* body);
	void UpdateDeathAndRelease();
};
