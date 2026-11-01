#pragma once

#include "TacticHelpers.h"
#include "../CoverMap.h"
#include "../../NpcBase.h"
#include "../CombatTuning.h"

#include <Physics.h>
#include <RandomHelper.h>

#include <algorithm>

// Footwork shared by the open-ground gun tactics (TacticStrafe, TacticKite).
namespace CombatMoves
{
	constexpr float MaxStepHeight = 1.5f;

	// Ground under `wanted` at the NPC's centre height, if it is walkable from
	// where the NPC stands (no wall or prop in the way, no big drop or climb).
	inline bool WalkablePoint(const NpcBase* body, const vec3& wanted, vec3& out)
	{
		const float centerHeight = body->GetCenterHeight();
		const float groundY = body->Position.y - centerHeight;

		const auto ground = Physics::LineTrace(wanted + vec3(0, 1.0f, 0), wanted - vec3(0, 3.0f, 0), BodyType::World);
		if (!ground.hasHit || std::abs(ground.position.y - groundY) > MaxStepHeight) return false;

		const vec3 candidate = ground.position + vec3(0, centerHeight, 0);

		if (Physics::CylinderTrace(body->Position, candidate, 0.5f, 0.8f, BodyType::World | BodyType::MainBody).hasHit) return false;

		out = candidate;
		return true;
	}

	inline bool SeesFrom(const vec3& point, float centerHeight, const vec3& targetEye)
	{
		const vec3 eye = point - vec3(0, centerHeight, 0) + vec3(0, 1.5f, 0);
		return !Physics::LineTrace(eye, targetEye, CoverMap::BlockerMask()).hasHit;
	}

	// A sidestep of `step` metres that also corrects the range into
	// [minDistance, maxDistance] and keeps a line of sight on the target. Tries
	// `sign`'s side first and flips `sign` so the next step goes the other way.
	inline bool PickSidestep(const NpcBase* body, const vec3& target, const vec3& targetEye,
		float minDistance, float maxDistance, float step, float& sign, vec3& out)
	{
		vec3 radial = MathHelper::Normalized(MathHelper::XZ(body->Position - target));
		if (glm::length(radial) < 0.001f) radial = vec3(1, 0, 0);

		const vec3 side = MathHelper::Normalized(glm::cross(radial, vec3(0, 1, 0)));
		const float distance = TacticHelpers::DistanceXZ(body->Position, target);

		// One step corrects the range by at most a step: a long straight jump
		// across the room would fail the walkable check and leave it standing.
		const float correction = glm::clamp(glm::clamp(distance, minDistance, maxDistance) - distance, -step, step);

		for (float trySign : { sign, -sign })
		{
			vec3 candidate;
			const vec3 wantedPoint = body->Position + side * (trySign * step) + radial * correction;

			if (!WalkablePoint(body, wantedPoint, candidate)) continue;
			if (!SeesFrom(candidate, body->GetCenterHeight(), targetEye)) continue;

			out = candidate;
			sign = -trySign;
			return true;
		}

		return false;
	}

	// Where a squad member heads when it closes in on a target: `holdDistance`
	// out from it on this NPC's side of it, plus a stable lateral offset of up
	// to `spread` per NPC so a squad fans out instead of queuing up.
	inline vec3 ApproachPoint(const NpcBase* body, const vec3& target, float holdDistance, float spread)
	{
		vec3 radial = MathHelper::Normalized(MathHelper::XZ(body->Position - target));
		if (glm::length(radial) < 0.001f) radial = vec3(1, 0, 0);

		const vec3 side = MathHelper::Normalized(glm::cross(radial, vec3(0, 1, 0)));
		const float lateral = (TacticHelpers::StableRandom01(body->Id, 11) * 2.0f - 1.0f) * spread;

		return target + radial * holdDistance + side * lateral;
	}

	// Closing on a far target and still fighting (COMBAT_AI_DESIGN.md §11.3):
	// sprint a leg, stop (kneeling half the time) and shoot, sprint again.
	// Without a line of sight there is nothing to stop for: it just sprints.
	struct BoundingApproach
	{

		bool sprinting = true;
		bool kneel = false;
		double phaseEnd = 0.0;

		void Reset()
		{
			sprinting = true;
			kneel = false;
			phaseEnd = 0.0;
		}

		// Sets the attack position and fills the order for this tick.
		void Update(NpcBase* body, const vec3& goal, const vec3& lookAt, bool canShoot, CombatMoveOrder& order)
		{
			const double now = Time::GameTime;

			if (!canShoot)
			{
				sprinting = true;
				phaseEnd = std::max(phaseEnd, now);
			}
			else if (now >= phaseEnd)
			{
				sprinting = !sprinting;
				phaseEnd = now + (sprinting ? Between(CombatTuning::Get().bounding.sprintMin, CombatTuning::Get().bounding.sprintMax) : Between(CombatTuning::Get().bounding.stopMin, CombatTuning::Get().bounding.stopMax));
				kneel = !sprinting && RandomHelper::RandomFloat() < CombatTuning::Get().bounding.kneelChance;
			}

			if (sprinting)
			{
				body->SetAttackPosition(goal);
				order.speed = CombatTuning::Get().movement.runSpeed;
				order.facing = NpcFacing::Movement;
				order.stance = NpcStance::Standing;
			}
			else
			{
				body->SetAttackPosition(body->Position);
				order.speed = CombatTuning::Get().movement.strafeSpeed;
				order.facing = NpcFacing::Point;
				order.lookAt = lookAt;
				order.stance = kneel ? NpcStance::Crouched : NpcStance::Standing;
				order.careful = true;
			}
		}

	private:
		static double Between(double min, double max) { return min + RandomHelper::RandomFloat() * (max - min); }
	};
}
