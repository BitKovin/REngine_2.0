#pragma once

#include <AI/AIBrain.h>
#include <Combat/CombatTypes.h>
#include <Control/Controller.h>

#include <Delay.hpp>
#include <EntityHandle.h>
#include <Navigation/PathFollowQuery.h>
#include <glm.h>

#include <memory>
#include <string>
#include <vector>

class CombatCharacter;
class Observer;

// The AI controller of one body (COMBAT_DESIGN.md §4.3, §8). Owned by the body; while the player possesses the
// body it is not ticked but keeps its memory. Provides perception (engine AiPerceptionSystem Observer), target
// tracking, navigation (PathFollowQuery), attack tokens and attack selection to its brain.
class AIController : public Controller
{
public:
	explicit AIController(CombatCharacter& body);
	~AIController() override;

	void SetBrain(const std::string& name);
	const std::string& GetBrainName() const { return brainName; }

	// ---- Controller ----
	void UpdateInput(CombatCharacter& body, CharacterInput& input, float deltaTime) override;
	void OnPossess(CombatCharacter& body) override;
	void OnUnpossess(CombatCharacter& body) override;

	// ---- Body notifications ----
	void NotifyDamaged(Entity* attacker);
	void OnBodyDied();
	void OnBodyDestroyed();

	// ---- Perception / targets (brain services) ----
	CombatCharacter& GetBody() { return body; }
	Entity* GetTarget();
	bool IsTargetVisible() const { return targetVisible; }
	bool IsTargetReady() const { return reactionDelay.GetRemainTime() <= 0.0; }   // reaction time after acquiring the target passed
	glm::vec3 GetLastKnownTargetPosition() const { return lastKnownTargetPosition; }
	float GetTargetDistance();
	bool HasInvestigatePoint() const { return hasInvestigatePoint; }
	glm::vec3 GetInvestigatePoint() const { return investigatePoint; }
	void ClearInvestigatePoint() { hasInvestigatePoint = false; }
	// The body the player currently controls if it is on this body's team (allies follow it).
	CombatCharacter* GetLeader() const;
	// Nearest hostile currently visible within maxDistance of `around`.
	Entity* FindVisibleHostileNear(const glm::vec3& around, float maxDistance);
	void SetTarget(Entity* newTarget);

	// ---- Movement (brain services; one request per frame) ----
	void MoveTo(const glm::vec3& point, float speedScale, float acceptanceRadius);
	void MoveDirection(const glm::vec3& direction, float speedScale);
	void Circle(const glm::vec3& center, float preferredDistance, float speedScale);
	void Stop();
	void LookAt(const glm::vec3& point);
	bool IsMoveGoalReached() const { return moveGoalReached; }

	// ---- Buttons ----
	void Tap(InputButton button);
	void HoldFor(InputButton button, float seconds);

	// ---- Attacks ----
	bool TryAcquireToken(AttackToken kind);
	void ReleaseToken();
	bool HoldsToken() const { return heldToken != AttackToken::None; }
	// Picks a valid attack from the character data (range, cooldown, sight, token, cancel rules) by weight and
	// starts it. Returns true if an attack started.
	bool TryStartAttack();
	bool IsBodyAttacking() const;
	bool HasRangedAttacks() const;
	float GetMaxAttackRange() const;

	void DrawDebugUI();

private:
	CombatCharacter& body;
	CharacterInput* input = nullptr;          // valid inside UpdateInput
	std::unique_ptr<AIBrain> brain;
	std::string brainName;
	bool brainNeedsActivation = true;

	std::shared_ptr<Observer> observer;
	EntityHandle target;
	bool targetVisible = false;
	Delay loseTargetDelay;                    // restarted while the target is visible; lost when it runs out
	Delay reactionDelay;
	glm::vec3 lastKnownTargetPosition = glm::vec3(0.0f);
	bool hasInvestigatePoint = false;
	glm::vec3 investigatePoint = glm::vec3(0.0f);

	PathFollowQuery path;
	bool moveGoalReached = true;
	float strafeSide = 1.0f;
	Delay strafeFlipDelay;

	EntityHandle tokenOwner;
	AttackToken heldToken = AttackToken::None;
	std::vector<Delay> attackCooldowns;       // per ai.attacks entry

	struct HeldButton { InputButton button; Delay release; };
	std::vector<HeldButton> heldButtons;

	void EnsureObserver();
	void RemoveObserver();
	void UpdatePerception(double now);
	void UpdateHeldButtons(double now);
	void AimAt(const glm::vec3& aimPoint, const glm::vec3& facePoint);
	bool IsHostile(Entity* entity) const;
	glm::vec3 EyePosition() const;
};
