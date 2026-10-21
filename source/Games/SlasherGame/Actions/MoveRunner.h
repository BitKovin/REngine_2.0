#pragma once

#include <Actions/ComboResolver.h>
#include <Actions/MoveBehavior.h>
#include <Data/MoveSetData.h>

#include <Delay.hpp>
#include <entt/entt.hpp>

#include <array>
#include <cstdint>
#include <memory>
#include <vector>

class CombatCharacter;

// One running move on a channel. `time` is the authoritative clip time: the animator samples the clip at it.
struct MoveSlot
{
	const MoveData* move = nullptr;
	MoveBehavior* behavior = nullptr;
	float time = 0.0f;
	float previousTime = 0.0f;
	float endTime = 0.0f;
	MoveEntryKind entry = MoveEntryKind::Fresh;
	int clip = 0;                              // clip index (MoveData::GetClipHash)
	uint32_t serial = 0;                       // unique per start / clip restart; 0 = never started
	bool sustaining = false;
	bool firstTick = true;
	bool holdCommitted = false;                // started by a long hold: no hold.until release hand-over
	bool committed = false;                    // MoveBehavior::Commits this frame: no link, cancel or recovery

	std::vector<std::vector<entt::entity>> hitVictims;   // per hit window: hit-once list (versioned ids)
	std::vector<uint8_t> hitWindowActive;

	bool IsActive() const { return move != nullptr; }
	float GetBlendIn() const { return move ? move->entry.Get(entry).blendIn : 0.0f; }
};

// The tail of an attack whose combo window closed unused (COMBAT_DESIGN.md §5.5). The move itself has ended -
// the body is free to move and act - while the clip keeps playing on the upper body so the weapon returns to
// its idle pose. Only cues still fire; any new primary move clears it.
struct RecoverySlot
{
	const MoveData* move = nullptr;
	float time = 0.0f;
	float previousTime = 0.0f;
	float endTime = 0.0f;
	int clip = 0;                              // clip index of the attack it continues
	uint32_t serial = 0;                       // serial of the attack it continues (the animator keeps its clip)
	bool released = false;                     // the body moved: locomotion takes over, root motion stops

	bool IsActive() const { return move != nullptr; }
};

// Runs a body's moves from its moveset (COMBAT_DESIGN.md §5). Member of CombatCharacter; main thread only.
class MoveRunner
{
public:
	void Initialize(CombatCharacter* owner);

	// Creates behaviour instances for the moveset. Only at a move boundary (no primary move running).
	void SetMoveSet(std::shared_ptr<const MoveSetData> moveSet);
	const MoveSetData* GetMoveSet() const { return moveSet.get(); }

	// Per frame, after the controller wrote input: advances moves, resolves combos, routes buttons.
	void Tick(float deltaTime);

	bool CanStartMove(int moveIndex, bool forced = false) const;
	// All rules apply (AI and code). startedByHold: the Attack hold started it (a hold opener) - the hold is spent on it
	// and does not carry into its combo link.
	bool TryStartMove(int moveIndex, bool startedByHold = false);
	bool StartForcedMove(int moveIndex);         // reactions, death, finisher victim: ignores cancel windows
	void StopPrimary(MoveEndReason reason);
	void StopOverlay(MoveEndReason reason);
	void StopAll(MoveEndReason reason);

	// Plays the running move's clip again from the start, as clip index `clip` (MoveData::GetClipHash): windows,
	// cues and the animation restart, the behaviour keeps its state. Before the move's first tick only the clip
	// changes (nothing was advanced or shown yet). Used by behaviours, e.g. Fire's per-shot recoil clips.
	void RestartClip(const MoveData& move, int clip);

	// A hit this body dealt was resolved (main thread): the primary move that dealt it reacts (air attacks keep the
	// attacker up).
	void NotifyHitDealt(const HitResult& result);

	// While locked (dead) only forced moves can start.
	void SetLocked(bool value) { locked = value; }
	bool IsLocked() const { return locked; }

	const MoveSlot& GetPrimary() const { return primary; }
	const MoveSlot& GetOverlay() const { return overlay; }
	const RecoverySlot& GetRecovery() const { return recovery; }
	bool IsPrimaryActive() const { return primary.IsActive(); }
	bool IsRecovering() const { return recovery.IsActive(); }
	// The body moves (input, airborne): the recovery clip hands over to locomotion (COMBAT_DESIGN.md §5.5).
	void ReleaseRecovery() { recovery.released = recovery.IsActive(); }
	bool IsMoveOnCooldown(int moveIndex) const;

	// Windows of the primary move at its current time.
	bool IsInvulnerableNow() const;
	float GetArmorResistNow() const;

	// Debug
	const ComboResolver& GetComboResolver() const { return comboResolver; }
	const char* GetLastComboDecision() const { return lastComboDecision; }

private:
	CombatCharacter* owner = nullptr;
	std::shared_ptr<const MoveSetData> moveSet;
	std::vector<std::unique_ptr<MoveBehavior>> behaviors;   // indexed by MoveBehaviorRegistry type
	std::vector<Delay> cooldowns;                           // per move index
	std::array<uint32_t, static_cast<size_t>(InputButton::Count)> consumedPressId{};

	MoveSlot primary;
	MoveSlot overlay;
	RecoverySlot recovery;
	ComboResolver comboResolver;
	uint32_t serialCounter = 0;
	bool locked = false;
	bool comboArmed = false;                   // a press for the running combo link is held: the link stays open
	uint32_t observedAttackPressId = 0;        // the Attack press seen last, and whether a primary move ran / the body
	bool attackPressInMove = false;            // stood on the ground when it was made (they decide the hold delay of a
	bool attackPressGrounded = true;           // press no combo window owns, HoldOpenerDelay)
	const char* lastComboDecision = "-";

	void StartSlot(MoveSlot& slot, const MoveData& move, MoveEntryKind entry, bool startedByHold = false);
	void EndSlot(MoveSlot& slot, MoveEndReason reason);
	void AdvanceSlot(MoveSlot& slot, float deltaTime);
	bool ResolveCombo();
	bool ReleaseHold();
	void StartOpener(bool grounded);
	int GetStartableHoldOpener(bool allowAirDropFallback) const; // a startable launcher; AirDrop fallback only for a fresh attack input
	bool CanStartAsHoldOpener(int moveIndex) const;
	bool ShouldRecover(const MoveSlot& slot) const;
	void BeginRecovery();
	void AdvanceRecovery(float deltaTime);
	void ProcessHitWindows(MoveSlot& slot);
	void ProcessCues(const MoveData& move, float time, float previousTime, bool firstTick);
	void HandleButtons();
	void HandleLocomotionCancel();
	bool TryStartFromButton(InputButton button, int moveIndex, bool startedByHold = false);
	bool HasPendingPress(InputButton button) const;
	void ConsumePress(InputButton button);
	void PublishState();

	bool CancelAllows(const MoveSlot& slot, TagMask tags) const;
	bool StartStateAllows(const MoveData& move) const;
	bool ComboCanContinue() const;               // the primary's combo has a follow-up that can start in this state
	float ResolveEndTime(const MoveData& move) const;
	MoveBehavior* GetBehavior(int type) const;
	MoveContext MakeContext(MoveSlot& slot, float deltaTime);
	bool IsPrimarySlot(const MoveSlot& slot) const { return &slot == &primary; }
};
