#include <AI/AIBrain.h>
#include <AI/AIController.h>
#include <Characters/CombatCharacter.h>

#include <RandomHelper.h>

#include <algorithm>

namespace
{
	// Continues a running combo the way a player does: a tap inside the light zone, or a hold through the
	// alternative point for the heavy follow-up. Decided once per link from comboChance / heavyChance.
	class ComboDriver
	{
	public:
		void Update(AIContext& context)
		{
			const MoveSlot& primary = context.Self.GetMoves().GetPrimary();
			if (!primary.IsActive() || !primary.move->combo.IsValid())
			{
				decidedSerial = 0;
				return;
			}

			const AiData& ai = context.Self.GetData()->ai;
			if (primary.serial != decidedSerial)
			{
				decidedSerial = primary.serial;
				continueCombo = RandomHelper::RandomFloat() < ai.comboChance && context.Controller.GetTarget() != nullptr;
				heavy = RandomHelper::RandomFloat() < ai.heavyChance;
				pressed = false;
			}

			if (!continueCombo || pressed)
				return;

			// A light follow-up is a tap in the light zone, a heavy one a tap in the heavy zone (a late press - never a
			// hold, which may become the launcher - COMBAT_DESIGN.md §5.4).
			const ComboLinkData& link = primary.move->combo;
			if (primary.time < (heavy ? link.alternativeAt : link.windowStart))
				return;

			context.Controller.Tap(InputButton::Attack);
			pressed = true;
		}

	private:
		uint32_t decidedSerial = 0;
		bool continueCombo = false;
		bool heavy = false;
		bool pressed = false;
	};

	// Shared fighting logic for melee / ranged styles.
	class EngageLogic
	{
	public:
		const char* state = "idle";

		// Returns true if the body is busy with an attack this frame (nothing else to decide).
		bool UpdateAttacking(AIContext& context)
		{
			combo.Update(context);
			if (context.Controller.IsBodyAttacking())
			{
				state = "attacking";
				return true;
			}
			return false;
		}

		void Idle(AIContext& context)
		{
			AIController& ai = context.Controller;
			if (ai.HasInvestigatePoint())
			{
				ai.MoveTo(ai.GetInvestigatePoint(), 0.6f, 1.0f);
				if (ai.IsMoveGoalReached())
					ai.ClearInvestigatePoint();
				state = "investigating";
				return;
			}
			ai.Stop();
			state = "idle";
		}

		void Fight(AIContext& context, Entity* target, bool ranged)
		{
			AIController& ai = context.Controller;
			const AiData& data = context.Self.GetData()->ai;

			if (!ai.IsTargetReady())
			{
				ai.Stop();
				state = "reacting";
				return;
			}

			if (!ai.IsTargetVisible())
			{
				ai.MoveTo(ai.GetLastKnownTargetPosition(), 1.0f, 0.75f);
				state = "searching";
				return;
			}

			const float distance = ai.GetTargetDistance();

			if (context.Now >= nextDecisionTime)
			{
				nextDecisionTime = context.Now + data.decisionInterval;
				if (RandomHelper::RandomFloat() < data.aggression && ai.TryStartAttack())
				{
					state = "attack";
					return;
				}
			}

			const float preferred = 0.5f * (data.preferredMinRange + data.preferredMaxRange);

			if (ranged && distance < data.preferredMinRange)
			{
				ai.MoveDirection(context.Self.Position - target->Position, 1.0f);
				ai.LookAt(target->Position);
				state = "backing off";
			}
			else if (distance > data.preferredMaxRange)
			{
				ai.MoveTo(target->Position, 1.0f, ranged ? data.preferredMaxRange * 0.9f : data.preferredMinRange);
				state = "approaching";
			}
			else
			{
				ai.Circle(target->Position, preferred, data.strafeSpeed);
				state = "circling";
			}
		}

	private:
		ComboDriver combo;
		double nextDecisionTime = 0.0;
	};
}

// "Melee": approach, circle at the preferred range while waiting for an attack token, attack by weight/range.
class MeleeBrain : public AIBrain
{
public:
	void Think(AIContext& context) override
	{
		if (engage.UpdateAttacking(context))
			return;

		if (Entity* target = context.Controller.GetTarget())
			engage.Fight(context, target, false);
		else
			engage.Idle(context);
	}

	const char* DescribeState() const override { return engage.state; }

private:
	EngageLogic engage;
};
REGISTER_AI_BRAIN("Melee", MeleeBrain)

// "Ranged": keeps a distance band, backs off when crowded, strafes, fires bursts (attacks with holdTime).
class RangedBrain : public AIBrain
{
public:
	void Think(AIContext& context) override
	{
		if (engage.UpdateAttacking(context))
			return;

		if (Entity* target = context.Controller.GetTarget())
			engage.Fight(context, target, true);
		else
			engage.Idle(context);
	}

	const char* DescribeState() const override { return engage.state; }

private:
	EngageLogic engage;
};
REGISTER_AI_BRAIN("Ranged", RangedBrain)

// "Ally": follows the body the player controls and engages enemies near it. Fights melee or ranged depending on
// the attacks in its data. Also drives the hero while the player possesses another body.
class AllyBrain : public AIBrain
{
public:
	void Think(AIContext& context) override
	{
		if (engage.UpdateAttacking(context))
			return;

		AIController& ai = context.Controller;
		const AiData& data = context.Self.GetData()->ai;
		CombatCharacter* leader = ai.GetLeader();

		// Only fight threats close to the leader; without a leader, defend itself.
		const glm::vec3 anchor = leader ? leader->Position : context.Self.Position;
		const float radius = leader ? data.engageRadius : data.sightRadius;
		Entity* target = ai.GetTarget();
		if (target == nullptr || glm::length(target->Position - anchor) > radius)
		{
			target = ai.FindVisibleHostileNear(anchor, radius);
			if (target)
				ai.SetTarget(target);
		}

		if (target && glm::length(target->Position - anchor) <= radius)
		{
			engage.Fight(context, target, ai.HasRangedAttacks());
			return;
		}

		if (leader)
		{
			const float distance = glm::length(MathHelper::XZ(leader->Position - context.Self.Position));
			if (distance > data.followDistance)
			{
				ai.MoveTo(leader->Position, distance > data.followDistance * 2.0f ? 1.0f : 0.6f, data.followDistance * 0.8f);
				engage.state = "following";
			}
			else
			{
				ai.Stop();
				engage.state = "guarding";
			}
			return;
		}

		engage.Idle(context);
	}

	const char* DescribeState() const override { return engage.state; }

private:
	EngageLogic engage;
};
REGISTER_AI_BRAIN("Ally", AllyBrain)

// "Idle": does nothing (training dummies, scripted bodies).
class IdleBrain : public AIBrain
{
public:
	void Think(AIContext& context) override
	{
		context.Controller.Stop();
	}

	const char* DescribeState() const override { return "idle"; }
};
REGISTER_AI_BRAIN("Idle", IdleBrain)
