#include <Actions/MoveBehavior.h>
#include <Actions/MoveRunner.h>
#include <Characters/CombatCharacter.h>
#include <Combat/CombatEvents.h>
#include <Data/CombatData.h>

#include <Logger.hpp>

#include <Time.hpp>

// "Finisher" (attacker) + "FinisherVictim": paired moves on a poise-broken body (COMBAT_DESIGN.md §6.4).
// The attacker aligns the victim, both are flagged inFinisher (invulnerable to everyone else), damage comes
// from the attacker's `direct` hit windows, and on completion the attacker receives the reward effect.
// Pairing and releasing touch the victim, so they run on the main thread (CombatCharacter::RunOnMainThread); when
// two attackers start a finisher on the same victim in one frame, the first in body order gets it and the other
// finisher is interrupted.
struct FinisherParams : MoveBehaviorParams
{
	float distance = 1.4f;           // victim is placed this far in front of the attacker
	std::string reward;              // status effect id; empty = the attacker data's finisher.reward
	float slowMotionDuration = 0.35f;
	float slowMotionScale = 0.3f;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(FinisherParams, distance, reward, slowMotionDuration, slowMotionScale)

	bool Validate(const std::string& path, DataLoadReport& report) const
	{
		if (distance < 0.0f) report.Error(path + ".distance", "must be >= 0");
		if (slowMotionScale <= 0.0f || slowMotionScale > 1.0f) report.Error(path + ".slowMotionScale", "must be in (0, 1]");
		return report.Ok();
	}
};

class FinisherBehavior : public MoveBehavior
{
public:
	bool CanStart(const CombatCharacter& character, const MoveData& move) const override
	{
		(void)move;
		return character.FindFinisherTarget() != nullptr;
	}

	void OnStart(MoveContext& context) override
	{
		CombatCharacter& attacker = context.Character;
		const FinisherParams& params = context.GetParams<FinisherParams>();

		victimHandle = EntityHandle();
		paired = false;
		auto* victim = dynamic_cast<CombatCharacter*>(attacker.FindFinisherTarget());
		if (victim == nullptr)
			return;

		victimHandle = EntityHandle::FromEntity(victim);
		attacker.SetSoftTarget(victim);

		glm::vec3 direction = MathHelper::XZ(victim->Position - attacker.Position);
		direction = glm::dot(direction, direction) > 1e-6f ? glm::normalize(direction) : attacker.GetForward();
		attacker.SnapFacing(direction);
		attacker.GetCombatState()->inFinisher = true;

		glm::vec3 victimPosition = attacker.Position + direction * params.distance;
		victimPosition.y = victim->Position.y;

		// The runner and this behaviour outlive the flush that runs this (same frame); the move serial tells whether
		// the finisher is still the running move by then.
		const uint32_t serial = context.Runner.GetPrimary().serial;
		attacker.RunOnMainThread([this, &attacker, serial, victimPosition, direction, params]()
		{
			if (attacker.GetMoves().GetPrimary().serial != serial)
				return;   // the finisher already ended (interrupted by a hit resolved earlier)

			auto* body = dynamic_cast<CombatCharacter*>(ResolveAlive(victimHandle));
			if (body == nullptr || body->IsDead() || body->GetCombatState()->inFinisher)
			{
				attacker.GetMoves().StopPrimary(MoveEndReason::Interrupted);   // claimed by another finisher
				return;
			}

			paired = true;
			body->Teleport(victimPosition);
			body->SnapFacing(-direction);
			body->SetSoftTarget(&attacker);
			body->GetCombatState()->inFinisher = true;

			if (const MoveSetData* victimMoves = body->GetMoves().GetMoveSet())
			{
				const int victimMove = victimMoves->GetReactionMove(ReactionSlot::FinisherVictim);
				if (victimMove >= 0)
					body->GetMoves().StartForcedMove(victimMove);
			}

			if (params.slowMotionDuration > 0.0f && params.slowMotionScale < 1.0f)
				Time::AddTimeScaleEffect(params.slowMotionDuration, params.slowMotionScale, true, "finisher", 0.05f, 0.15f);

			EcsScheduler::Emit(FinisherEvent{ EntityHandle::FromEntity(&attacker), victimHandle, false });
		});
	}

	void OnEnd(MoveContext& context, MoveEndReason reason) override
	{
		CombatCharacter& attacker = context.Character;
		const FinisherParams& params = context.GetParams<FinisherParams>();

		attacker.GetCombatState()->inFinisher = false;

		const bool completed = paired && (reason == MoveEndReason::Completed || reason == MoveEndReason::Chained
			|| reason == MoveEndReason::Cancelled || reason == MoveEndReason::Recovered);
		if (completed)
		{
			const std::string& reward = params.reward.empty() ? attacker.GetData()->finisher.reward : params.reward;
			if (!reward.empty() && !attacker.GetStatusEffects()->Add(reward))
				Logger::Warning("[Finisher] reward effect '%s' not found in %s", reward.c_str(), CombatData::StatusEffectsPath);
		}

		if (paired)
		{
			const EntityHandle attackerHandle = EntityHandle::FromEntity(&attacker);
			attacker.RunOnMainThread([victim = victimHandle, attackerHandle, completed]() mutable
			{
				if (auto* body = dynamic_cast<CombatCharacter*>(ResolveAlive(victim)))
				{
					body->GetCombatState()->inFinisher = false;
					if (!body->IsDead())
						body->GetPoise()->EndBroken();
				}
				if (completed)
					EcsScheduler::Emit(FinisherEvent{ attackerHandle, victim, true });
			});
		}

		victimHandle = EntityHandle();
		paired = false;
	}

private:
	EntityHandle victimHandle;
	bool paired = false;             // the victim was aligned and flagged (main thread)
};

REGISTER_MOVE_BEHAVIOR("Finisher", FinisherBehavior, FinisherParams)

class FinisherVictimBehavior : public MoveBehavior
{
public:
	void OnStart(MoveContext& context) override
	{
		context.Character.GetCombatState()->inFinisher = true;
	}

	void OnTick(MoveContext& context) override
	{
		CombatCharacter& victim = context.Character;
		victim.RequestVelocity(glm::vec3(0.0f), false);
		if (Entity* attacker = victim.GetSoftTarget())
			victim.RequestFacing(attacker->Position - victim.Position, 720.0f);
	}

	bool WantsToEnd(const MoveContext& context) const override
	{
		auto state = context.Character.GetCombatState();
		return !state || !state->inFinisher;   // the attacker's finisher ended
	}

	bool Sustains(const MoveContext& context) const override
	{
		return !WantsToEnd(context);
	}
};

REGISTER_MOVE_BEHAVIOR("FinisherVictim", FinisherVictimBehavior, NoMoveParams)
