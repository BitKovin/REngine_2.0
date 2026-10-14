#include "MoveRunner.h"

#include <Characters/CombatCharacter.h>
#include <Characters/WeaponSet.h>
#include <Combat/CombatRules.h>

#include <Logger.hpp>
#include <Time.hpp>

#include <algorithm>
#include <cmath>

namespace
{
	constexpr float LocomotionCancelInput = 0.3f;   // move input magnitude that ends a move in a Locomotion window
}

void MoveRunner::Initialize(CombatCharacter* character)
{
	owner = character;
}

void MoveRunner::SetMoveSet(std::shared_ptr<const MoveSetData> newMoveSet)
{
	if (primary.IsActive() || overlay.IsActive())
		StopAll(MoveEndReason::Interrupted);

	recovery = RecoverySlot();   // points into the old moveset
	moveSet = std::move(newMoveSet);
	behaviors.clear();
	behaviors.resize(static_cast<size_t>(MoveBehaviorRegistry::Count()));
	cooldowns.assign(moveSet ? moveSet->moves.size() : 0, Delay());
	comboResolver.Reset();

	if (!moveSet)
		return;

	// One behaviour instance per type used by this moveset, created up front (no allocations mid-fight).
	for (const MoveData& move : moveSet->moves)
	{
		if (move.behaviorType < 0)
			continue;
		auto& instance = behaviors[static_cast<size_t>(move.behaviorType)];
		if (!instance)
			instance = MoveBehaviorRegistry::Get(move.behaviorType).Create();
	}
}

MoveBehavior* MoveRunner::GetBehavior(int type) const
{
	if (type < 0 || type >= static_cast<int>(behaviors.size()))
		return nullptr;
	return behaviors[static_cast<size_t>(type)].get();
}

MoveContext MoveRunner::MakeContext(MoveSlot& slot, float deltaTime)
{
	return MoveContext{ *owner, *this, *slot.move, slot.move->behaviorParams.get(), slot.time, slot.previousTime, slot.endTime, deltaTime };
}

float MoveRunner::ResolveEndTime(const MoveData& move) const
{
	if (!move.HasClip())
		return std::max(move.endTime, 0.0f);   // validated: clip-less moves always have an endTime

	const float clipLength = owner->GetClipDuration(move);
	float end = move.endTime >= 0.0f ? move.endTime : clipLength;
	if (clipLength > 0.0f)
		end = std::min(end, clipLength);

	if (end <= 0.0f)
	{
		Logger::Error("[MoveRunner] move '%s': clip '%s' is missing or empty and no endTime is set", move.id.c_str(), move.clip.c_str());
		end = 0.5f;
	}
	return end;
}

bool MoveRunner::IsMoveOnCooldown(int moveIndex) const
{
	if (moveIndex < 0 || moveIndex >= static_cast<int>(cooldowns.size()))
		return false;
	return cooldowns[static_cast<size_t>(moveIndex)].GetRemainTime() > 0.0;
}

bool MoveRunner::StartStateAllows(const MoveData& move) const
{
	switch (move.startState)
	{
	case MoveStartState::Ground: return owner->IsGrounded();
	case MoveStartState::Air:    return !owner->IsGrounded();
	default:                     return true;
	}
}

bool MoveRunner::CancelAllows(const MoveSlot& slot, TagMask tags) const
{
	if (!slot.IsActive() || slot.time >= slot.endTime)
		return true;

	for (const CancelWindowData& window : slot.move->cancels)
	{
		const float end = window.end < 0.0f ? slot.endTime : window.end;
		if (slot.time >= window.start && slot.time <= end && (window.intoMask & tags) != 0)
			return true;
	}
	return false;
}

bool MoveRunner::CanStartMove(int moveIndex, bool forced) const
{
	const MoveData* move = moveSet ? moveSet->GetMove(moveIndex) : nullptr;
	if (!move)
		return false;

	if (!forced)
	{
		if (locked || owner->IsBroken())
			return false;
		if (IsMoveOnCooldown(moveIndex) || !StartStateAllows(*move))
			return false;
	}

	if (const MoveBehavior* behavior = GetBehavior(move->behaviorType))
		if (!behavior->CanStart(*owner, *move))
			return false;

	if (forced)
		return true;

	if (move->channel == MoveChannel::Primary)
		return CancelAllows(primary, move->tagMask);

	// Overlay
	if (primary.IsActive() && primary.move->overlay != OverlayPolicy::Allow)
		return false;
	return CancelAllows(overlay, move->tagMask);
}

bool MoveRunner::TryStartMove(int moveIndex)
{
	if (!CanStartMove(moveIndex))
		return false;

	const MoveData& move = *moveSet->GetMove(moveIndex);
	MoveSlot& slot = move.channel == MoveChannel::Overlay ? overlay : primary;
	EndSlot(slot, MoveEndReason::Cancelled);
	StartSlot(slot, move, MoveEntryKind::Fresh);
	return true;
}

bool MoveRunner::StartForcedMove(int moveIndex)
{
	if (!CanStartMove(moveIndex, true))
		return false;

	// An interruption (a reaction, death) ends whatever the Attack press was going to do: a button held through a
	// stagger never fires an attack or the launcher afterwards.
	ConsumePress(InputButton::Attack);

	const MoveData& move = *moveSet->GetMove(moveIndex);
	EndSlot(overlay, MoveEndReason::Interrupted);
	EndSlot(primary, MoveEndReason::Interrupted);
	StartSlot(primary, move, MoveEntryKind::Fresh);
	return true;
}

void MoveRunner::NotifyHitDealt(const HitResult& result)
{
	if (!primary.IsActive() || primary.behavior == nullptr || result.request == nullptr || result.request->moveIndex != primary.move->index)
		return;

	MoveContext context = MakeContext(primary, 0.0f);
	primary.behavior->OnHitDealt(context, result);
}

void MoveRunner::StopPrimary(MoveEndReason reason)
{
	EndSlot(primary, reason);
	PublishState();
}

void MoveRunner::StopOverlay(MoveEndReason reason)
{
	EndSlot(overlay, reason);
}

void MoveRunner::StopAll(MoveEndReason reason)
{
	EndSlot(overlay, reason);
	EndSlot(primary, reason);
	recovery = RecoverySlot();
	PublishState();
}

void MoveRunner::StartSlot(MoveSlot& slot, const MoveData& move, MoveEntryKind entry)
{
	const MoveEntryData& entryData = move.entry.Get(entry);

	slot.move = &move;
	slot.behavior = GetBehavior(move.behaviorType);
	slot.entry = entry;
	slot.clip = 0;
	slot.endTime = ResolveEndTime(move);
	slot.time = std::min(entryData.startTime, slot.endTime);
	slot.previousTime = slot.time;
	slot.serial = ++serialCounter;
	slot.sustaining = false;
	slot.firstTick = true;
	slot.holdCommitted = false;
	slot.hitVictims.resize(move.hits.size());
	for (auto& victims : slot.hitVictims) victims.clear();
	slot.hitWindowActive.assign(move.hits.size(), 0);

	if (move.cooldown > 0.0f && move.index >= 0 && move.index < static_cast<int>(cooldowns.size()))
		cooldowns[static_cast<size_t>(move.index)].AddDelay(move.cooldown);

	const bool isPrimary = IsPrimarySlot(slot);
	if (isPrimary)
	{
		recovery = RecoverySlot();   // the new move owns the whole body

		if (move.combo.IsValid())
			comboResolver.Begin(owner->GetInput().Get(InputButton::Attack), moveSet->holdCarriesOver);
		else
			comboResolver.Reset();
		comboArmed = false;

		if (move.overlay == OverlayPolicy::Block)
			EndSlot(overlay, MoveEndReason::Interrupted);
	}

	owner->OnMoveStarted(slot, isPrimary);

	if (slot.behavior)
	{
		MoveContext context = MakeContext(slot, 0.0f);
		slot.behavior->OnStart(context);
	}
}

void MoveRunner::RestartClip(const MoveData& move, int clip)
{
	MoveSlot* slot = primary.move == &move ? &primary : (overlay.move == &move ? &overlay : nullptr);
	if (slot == nullptr)
		return;

	slot->clip = clip;
	if (slot->firstTick)
		return;

	WeaponSet& weapons = owner->GetWeapons();
	for (size_t i = 0; i < slot->hitWindowActive.size(); ++i)
	{
		if (slot->hitWindowActive[i])
		{
			slot->hitWindowActive[i] = 0;
			weapons.EndHitWindow(static_cast<int>(i));
		}
	}
	for (auto& victims : slot->hitVictims)
		victims.clear();

	slot->time = 0.0f;
	slot->previousTime = 0.0f;
	slot->serial = ++serialCounter;   // a new clip instance for the animator
	slot->sustaining = false;
	slot->firstTick = true;

	if (IsPrimarySlot(*slot) && move.combo.IsValid())
		comboResolver.Begin(owner->GetInput().Get(InputButton::Attack), moveSet->holdCarriesOver);
}

void MoveRunner::EndSlot(MoveSlot& slot, MoveEndReason reason)
{
	if (!slot.IsActive())
		return;

	const bool isPrimary = IsPrimarySlot(slot);

	if (slot.behavior)
	{
		MoveContext context = MakeContext(slot, 0.0f);
		slot.behavior->OnEnd(context, reason);
	}

	WeaponSet& weapons = owner->GetWeapons();
	for (size_t i = 0; i < slot.hitWindowActive.size(); ++i)
		if (slot.hitWindowActive[i])
			weapons.EndHitWindow(static_cast<int>(i));
	weapons.StopAllTrails();

	owner->OnMoveEnded(slot, reason, isPrimary);

	if (isPrimary)
	{
		comboResolver.Reset();
		comboArmed = false;
	}

	slot.move = nullptr;
	slot.behavior = nullptr;
	slot.sustaining = false;
}

void MoveRunner::Tick(float deltaTime)
{
	if (!moveSet)
		return;

	// Where a new Attack press was made - inside a move or from rest - before this frame can end that move.
	const ButtonState& attack = owner->GetInput().Get(InputButton::Attack);
	if (attack.pressId != observedAttackPressId)
	{
		observedAttackPressId = attack.pressId;
		attackPressInMove = primary.IsActive();
	}

	// Before the primary: a recovery that begins this frame starts advancing next frame.
	if (recovery.IsActive())
		AdvanceRecovery(deltaTime);
	if (primary.IsActive())
		AdvanceSlot(primary, deltaTime);
	if (overlay.IsActive())
		AdvanceSlot(overlay, deltaTime);

	if (!locked)
	{
		HandleButtons();
		HandleLocomotionCancel();
	}

	PublishState();
}

void MoveRunner::AdvanceSlot(MoveSlot& slot, float deltaTime)
{
	const MoveData& move = *slot.move;
	const uint32_t serial = slot.serial;

	slot.previousTime = slot.time;
	const bool clockHeld = !slot.firstTick && slot.behavior && slot.behavior->HoldsClock(MakeContext(slot, deltaTime));
	if (!slot.firstTick && !clockHeld && (!slot.sustaining || move.loop))
		slot.time += deltaTime * move.speed * owner->GetAttackSpeedMultiplier();

	// A sustained looping move wraps inside [loopStart, endTime]; windows and cues of the loop run again.
	// Once the behaviour stops sustaining, the move finishes at endTime below.
	if (slot.sustaining && move.loop && slot.time >= slot.endTime)
	{
		const MoveContext probe = MakeContext(slot, deltaTime);
		if (slot.behavior && slot.behavior->Sustains(probe))
		{
			const float loopLength = std::max(slot.endTime - move.loopStart, 0.01f);
			slot.time = move.loopStart + std::fmod(slot.time - move.loopStart, loopLength);
			slot.previousTime = move.loopStart;
		}
		else
		{
			slot.sustaining = false;
		}
	}

	if (IsPrimarySlot(slot) && (ReleaseHold() || ResolveCombo()))
		return;

	ProcessHitWindows(slot);
	ProcessCues(move, slot.time, slot.previousTime, slot.firstTick);
	slot.firstTick = false;

	if (IsPrimarySlot(slot) && ShouldRecover(slot))
	{
		BeginRecovery();
		return;
	}

	if (slot.behavior)
	{
		MoveContext context = MakeContext(slot, deltaTime);
		slot.behavior->OnTick(context);

		// The tick may have replaced or ended this move (e.g. a dash hitting a wall).
		if (!slot.IsActive() || slot.serial != serial)
			return;

		if (slot.behavior->WantsToEnd(context))
		{
			EndSlot(slot, MoveEndReason::Released);
			return;
		}

		if (slot.time >= slot.endTime && slot.behavior->Sustains(context))
		{
			if (!move.loop)
				slot.time = slot.endTime;
			slot.sustaining = true;
			return;
		}
		if (slot.sustaining && slot.behavior->Sustains(context))
			return;   // looping sustain still inside the loop range
	}

	if (slot.time >= slot.endTime)
		EndSlot(slot, MoveEndReason::Completed);
}

bool MoveRunner::ResolveCombo()
{
	if (!comboResolver.IsActive())
		return false;

	const MoveData& move = *primary.move;
	const ComboLinkData& link = move.combo;

	ComboTiming timing;
	timing.windowStart = link.windowStart;
	timing.alternativeAt = link.alternativeAt;
	timing.windowEnd = link.windowEnd;
	timing.hasLight = link.nextIndex >= 0 && StartStateAllows(*moveSet->GetMove(link.nextIndex));
	timing.hasHeavy = link.alternativeIndex >= 0 && StartStateAllows(*moveSet->GetMove(link.alternativeIndex));
	const int holdOpener = GetStartableHoldOpener();
	timing.hasHold = holdOpener >= 0;
	timing.holdDelay = moveSet->holdDelayInMove;

	comboResolver.Observe(owner->GetInput().Get(InputButton::Attack), timing, primary.time);
	comboArmed = comboResolver.IsArmed(timing, Time::GameTime);
	const ComboDecision decision = comboResolver.Evaluate(timing, primary.time, Time::GameTime, moveSet->inputBuffer);
	if (decision == ComboDecision::None)
		return false;

	int nextIndex = holdOpener;
	if (decision == ComboDecision::Light)
		nextIndex = link.nextIndex;
	else if (decision == ComboDecision::Heavy)
		nextIndex = link.alternativeIndex;
	const MoveData* next = moveSet->GetMove(nextIndex);
	if (!next || IsMoveOnCooldown(nextIndex) || locked)
		return false;

	if (const MoveBehavior* behavior = GetBehavior(next->behaviorType))
		if (!behavior->CanStart(*owner, *next))
			return false;

	lastComboDecision = decision == ComboDecision::Light ? "light" : (decision == ComboDecision::Heavy ? "heavy" : "hold");
	ConsumePress(InputButton::Attack);

	// The hold opener (launcher) after a long hold is a fresh, committed start: the hold already showed the intent.
	const MoveEntryKind entry = decision == ComboDecision::Hold ? MoveEntryKind::Fresh : (move.heavy ? MoveEntryKind::AfterHeavy : MoveEntryKind::AfterLight);
	EndSlot(primary, MoveEndReason::Chained);
	StartSlot(primary, *next, entry);
	primary.holdCommitted = decision == ComboDecision::Hold;
	return true;
}

int MoveRunner::GetStartableHoldOpener() const
{
	const int holdOpener = moveSet->GetHoldOpener(moveSet->GetOpenerSlot(owner->IsGrounded()));
	const MoveData* move = moveSet->GetMove(holdOpener);
	if (move == nullptr || IsMoveOnCooldown(holdOpener) || !StartStateAllows(*move))
		return -1;
	if (const MoveBehavior* behavior = GetBehavior(move->behaviorType))
		if (!behavior->CanStart(*owner, *move))
			return -1;
	return holdOpener;
}

// A hold move whose button was let go before hold.until (its wind-up) hands over to its release move - the launcher
// falls back to the usual attack (COMBAT_DESIGN.md §5.4).
bool MoveRunner::ReleaseHold()
{
	const MoveData& move = *primary.move;
	const HoldData& hold = move.hold;
	if (!hold.IsValid() || primary.holdCommitted || primary.time >= hold.until || locked || owner->GetInput().Get(InputButton::Attack).held)
		return false;

	const MoveData* next = moveSet->GetMove(hold.releaseIndex);
	if (!next || IsMoveOnCooldown(hold.releaseIndex) || !StartStateAllows(*next))
		return false;
	if (const MoveBehavior* behavior = GetBehavior(next->behaviorType))
		if (!behavior->CanStart(*owner, *next))
			return false;

	EndSlot(primary, MoveEndReason::Chained);
	StartSlot(primary, *next, MoveEntryKind::AfterHold);
	return true;
}

bool MoveRunner::ComboCanContinue() const
{
	if (!primary.IsActive())
		return false;
	const ComboLinkData& link = primary.move->combo;
	const MoveData* next = moveSet->GetMove(link.nextIndex);
	const MoveData* alternative = moveSet->GetMove(link.alternativeIndex);
	return (next && StartStateAllows(*next)) || (alternative && StartStateAllows(*alternative));
}

bool MoveRunner::ShouldRecover(const MoveSlot& slot) const
{
	// Runs after ResolveCombo: past recoveryStart (>= combo.windowEnd) the combo can no longer fire - unless a press for
	// the link is still held (armed): its release still gets the follow-up, so the attack waits for it.
	const MoveData& move = *slot.move;
	return move.recoveryStart >= 0.0f
		&& slot.time >= move.recoveryStart
		&& slot.time < slot.endTime
		&& !slot.sustaining
		&& !comboArmed;
}

void MoveRunner::BeginRecovery()
{
	recovery.move = primary.move;
	recovery.time = primary.time;
	recovery.previousTime = primary.time;
	recovery.endTime = primary.endTime;
	recovery.clip = primary.clip;
	recovery.serial = primary.serial;

	EndSlot(primary, MoveEndReason::Recovered);
}

void MoveRunner::AdvanceRecovery(float deltaTime)
{
	const MoveData& move = *recovery.move;

	recovery.previousTime = recovery.time;
	recovery.time = std::min(recovery.time + deltaTime * move.speed * owner->GetAttackSpeedMultiplier(), recovery.endTime);
	ProcessCues(move, recovery.time, recovery.previousTime, false);

	if (recovery.time >= recovery.endTime)
		recovery = RecoverySlot();
}

void MoveRunner::ProcessHitWindows(MoveSlot& slot)
{
	const MoveData& move = *slot.move;
	if (move.hits.empty())
		return;

	WeaponSet& weapons = owner->GetWeapons();

	for (size_t i = 0; i < move.hits.size(); ++i)
	{
		const HitWindowData& window = move.hits[i];
		const bool activeNow = slot.time >= window.start && slot.previousTime <= window.end;

		if (activeNow)
		{
			if (!slot.hitWindowActive[i])
			{
				slot.hitWindowActive[i] = 1;
				slot.hitVictims[i].clear();
				weapons.BeginHitWindow(static_cast<int>(i));
			}
			weapons.SweepHitWindow(slot, static_cast<int>(i));
		}
		else if (slot.hitWindowActive[i])
		{
			slot.hitWindowActive[i] = 0;
			weapons.EndHitWindow(static_cast<int>(i));
		}
	}
}

void MoveRunner::ProcessCues(const MoveData& move, float time, float previousTime, bool firstTick)
{
	for (const CueData& cue : move.cues)
	{
		const bool crossed = firstTick
			? cue.time == time
			: (cue.time > previousTime && cue.time <= time);

		if (crossed)
			owner->PlayCue(cue);
	}
}

// Attack presses wait inputBuffer after their release (combos queue up) - a press still held does not expire, it may be
// becoming a hold (the launcher; an interruption spends it, StartForcedMove); every other action waits actionBuffer
// after its press - a dash pressed long before its cancel window opens is not wanted any more when it would fire.
bool MoveRunner::HasPendingPress(InputButton button) const
{
	const ButtonState& state = owner->GetInput().Get(button);
	if (state.pressId == consumedPressId[static_cast<size_t>(button)])
		return false;
	if (button == InputButton::Attack)
		return state.held || Time::GameTime - state.releaseTime <= moveSet->inputBuffer;
	return Time::GameTime - state.pressTime <= moveSet->actionBuffer;
}

void MoveRunner::ConsumePress(InputButton button)
{
	consumedPressId[static_cast<size_t>(button)] = owner->GetInput().Get(button).pressId;
}

bool MoveRunner::TryStartFromButton(InputButton button, int moveIndex)
{
	if (moveIndex < 0 || !TryStartMove(moveIndex))
		return false;
	ConsumePress(button);
	return true;
}

void MoveRunner::HandleButtons()
{
	const bool grounded = owner->IsGrounded();

	if (HasPendingPress(InputButton::Attack))
	{
		// While a combo window can still use the press, it belongs to the combo resolver. A combo whose follow-ups
		// cannot start here (a ground combo after the body left the ground) leaves it to the air opener.
		const bool comboOwnsPress = comboResolver.IsActive() && primary.IsActive()
			&& (primary.time <= primary.move->combo.windowEnd || comboArmed) && ComboCanContinue();
		if (!comboOwnsPress)
			StartOpener(grounded);
	}

	if (HasPendingPress(InputButton::Secondary))
	{
		int move = grounded ? -1 : moveSet->GetActionMove(ActionSlot::AirSecondary);
		if (move < 0) move = moveSet->GetActionMove(ActionSlot::Secondary);
		TryStartFromButton(InputButton::Secondary, move);
	}

	if (HasPendingPress(InputButton::Dash))
	{
		int move = grounded ? -1 : moveSet->GetActionMove(ActionSlot::AirDash);
		if (move < 0) move = moveSet->GetActionMove(ActionSlot::Dash);
		TryStartFromButton(InputButton::Dash, move);
	}

	if (HasPendingPress(InputButton::Jump))
		TryStartFromButton(InputButton::Jump, moveSet->GetActionMove(grounded ? ActionSlot::Jump : ActionSlot::AirJump));

	// Holding jump while falling glides - no second press (the Glide behaviour checks the hold and the fall).
	if (!grounded && owner->GetInput().Get(InputButton::Jump).held)
	{
		const int glide = moveSet->GetActionMove(ActionSlot::Glide);
		if (glide >= 0 && primary.move != moveSet->GetMove(glide))
			TryStartMove(glide);
	}

	if (HasPendingPress(InputButton::Finisher))
		TryStartFromButton(InputButton::Finisher, moveSet->GetActionMove(ActionSlot::Finisher));
}

// Movement input ends a primary move inside a Locomotion cancel window (COMBAT_DESIGN.md §5.6) - unless the player is
// still feeding the combo (Attack held, or a press waiting). A move with a clip hands over like a released recovery:
// the clip keeps playing while the legs and then the upper body blend to locomotion.
void MoveRunner::HandleLocomotionCancel()
{
	if (!primary.IsActive() || !CancelAllows(primary, TagBit(MoveTag::Locomotion)))
		return;

	const CharacterInput& input = owner->GetInput();
	if (glm::length(MathHelper::XZ(input.move)) < LocomotionCancelInput)
		return;
	if (input.Get(InputButton::Attack).held || HasPendingPress(InputButton::Attack))
		return;

	if (primary.move->HasClip() && primary.move->recovery.enabled)
	{
		BeginRecovery();
		ReleaseRecovery();
	}
	else
	{
		EndSlot(primary, MoveEndReason::Cancelled);
	}
}

// A fresh Attack press outside a combo window starts the slot's combo opener - or, when a hold opener can start, waits:
// released early it starts the opener (on release), held long enough the hold opener (§5.4). A press made from rest
// needs holdDelay (the launcher shows its wind-up and still falls back to the opener if released during it); one made
// during a move (a dash, an attack past its combo window) holdDelayInMove, and the launcher commits at once. Where the
// press was made counts, not where the body is now: a press held through the end of a dash is still the dash's. A
// press that cannot start anything yet stays pending and decides when it can.
void MoveRunner::StartOpener(bool grounded)
{
	const int opener = moveSet->GetComboOpener(moveSet->GetOpenerSlot(grounded));
	const int holdOpener = GetStartableHoldOpener();   // a ground launcher in the air: none, the press starts the opener
	if (holdOpener < 0)
	{
		TryStartFromButton(InputButton::Attack, opener);
		return;
	}

	const bool inMove = attackPressInMove;
	const float delay = inMove ? moveSet->holdDelayInMove : moveSet->holdDelay;
	switch (DecideOpener(owner->GetInput().Get(InputButton::Attack), Time::GameTime, delay))
	{
	case OpenerDecision::Wait:
		break;
	case OpenerDecision::Opener:
		TryStartFromButton(InputButton::Attack, opener);
		break;
	case OpenerDecision::Hold:
		if (TryStartFromButton(InputButton::Attack, holdOpener))
			primary.holdCommitted = inMove;
		else if (!inMove)
			TryStartFromButton(InputButton::Attack, opener);   // from rest the press still gets the usual attack
		break;   // during a move the hold waits until the move lets the launcher in
	}
}

bool MoveRunner::IsInvulnerableNow() const
{
	if (!primary.IsActive())
		return false;
	for (const TimeWindowData& window : primary.move->invulnerable)
	{
		const float end = window.end < 0.0f ? primary.endTime : window.end;
		if (primary.time >= window.start && primary.time <= end)
			return true;
	}
	return false;
}

float MoveRunner::GetArmorResistNow() const
{
	float resist = 0.0f;
	if (!primary.IsActive())
		return resist;
	for (const ArmorWindowData& window : primary.move->armor)
	{
		const float end = window.end < 0.0f ? primary.endTime : window.end;
		if (primary.time >= window.start && primary.time <= end)
			resist = std::max(resist, window.poiseResist);
	}
	return resist;
}

void MoveRunner::PublishState()
{
	if (auto state = owner->GetCombatState())
	{
		state->invulnerable = IsInvulnerableNow() || state->inFinisher;
		state->activeMoveTags = primary.IsActive() ? primary.move->tagMask : 0;
	}
	if (auto poise = owner->GetPoise())
		poise->armorResist = GetArmorResistNow();
}
