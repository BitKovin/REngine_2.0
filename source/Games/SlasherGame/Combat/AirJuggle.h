#pragma once

#include <algorithm>
#include <cmath>

// Air juggling (COMBAT_DESIGN.md §5.8): the pure rules. No engine types in here, so SelfTests (and a stand-alone
// harness) run exactly the code the game runs; CombatCharacter is the glue that feeds it bodies and hits.
//
// The model (Devil May Cry style):
//   * HOVER: a body that fights in the air has (almost) no gravity. Landing a hit starts it - swinging at air does not.
//     Falling, a hovering body has `fallGravityScale` of the gravity and sinks no faster than `maxFallSpeed`; rising it
//     keeps the normal arc. When the hover ends gravity fades back in over `gravityRamp`.
//   * HANG CLOCK: runs from a body's first hover of an airtime until it lands. Past `maxHangTime` gravity grows back
//     over `hangFade` however often the body is hit - no infinite juggles. Only the player has a limit of her own
//     (maxHangTime > 0): an enemy hangs as long as she keeps hitting it, so her limit is the juggle's.
//   * PULL: a melee hit in the air settles its victim once - toward the attacker's height (plus the slight rise of the
//     hit) and the attack's ideal distance. The destination is fixed at the hit: the victim never follows the
//     attacker afterwards.
// `Air` is CharacterData's AirCombatData (any type with the same members works).
namespace AirJuggle
{
	constexpr float FreeFallSpeed = 60.0f;   // m/s: the sink cap a hover grows to with the overtime (no cap in effect)

	inline float Lerp(float a, float b, float t) { return a + (b - a) * t; }

	// A hit-specific stall may ask for a longer hover than the body's normal one, never a shorter one (a juggled body
	// falls only when the hits stop); zero explicitly disables the hover (a slam).
	inline float ResolveHold(float requested, float fallback)
	{
		if (requested < 0.0f)
			return fallback;
		return requested > 0.0f ? std::max(requested, fallback) : 0.0f;
	}

	// The attacker's hover is earned only by a successful melee launcher configured to launch the attacker too.
	// Keep this gate pure so the edge cases can be covered by SelfTests.
	inline bool ShouldShareLaunchHover(bool launchedTarget, bool launchSelfOnHit, bool alreadySelfLaunched, bool meleeHit)
	{
		return launchedTarget && launchSelfOnHit && !alreadySelfLaunched && meleeHit;
	}

	// One step of a body's vertical speed while it hovers (hover > 0). hover in [0, 1]: 1 fully hovering, between:
	// gravity fading back in. overtime in [0, 1]: 0 until the hang clock passes maxHangTime, then growing to 1 - the
	// hover's gravity and sink grow to the normal ones.
	template <class Air>
	inline float StepVertical(float vertical, float gravity, float deltaTime, const Air& air, float hover, float overtime)
	{
		hover = std::clamp(hover, 0.0f, 1.0f);
		overtime = std::clamp(overtime, 0.0f, 1.0f);

		// Rising keeps the arc: a jump or a launch reaches its top, a hit's lift is a small hop.
		if (vertical > 0.0f)
			return vertical - gravity * Lerp(1.0f, air.riseGravityScale, hover) * deltaTime;

		const float scale = Lerp(1.0f, Lerp(air.fallGravityScale, 1.0f, overtime), hover);
		if (hover < 1.0f)
			return vertical - gravity * scale * deltaTime;   // gravity fading back in: no cap, it simply grows

		const float sink = Lerp(air.maxFallSpeed, FreeFallSpeed, overtime);
		if (vertical >= -sink)
			return std::max(vertical - gravity * scale * deltaTime, -sink);

		// Faster than the sink (a fall that entered the hover): brake to it without gravity pushing past it.
		return -sink + (vertical + sink) * std::exp(-air.fallEase * deltaTime);
	}

	// The hover clocks of one body (local seconds).
	struct Body
	{
		float remaining = 0.0f;   // full hover left
		float fade = 0.0f;        // gravity fading back in: left
		float clock = -1.0f;      // since the first hover of this airtime; < 0: none yet

		// Hover for at least `seconds` more: every hit restarts the timer, none shortens it.
		void Enter(float seconds) { remaining = std::max(remaining, seconds); }
		// No hover and no fade: gravity at once (a slam, a corpse).
		void End() { remaining = 0.0f; fade = 0.0f; }
		// Landing resets everything, the hang clock too.
		void Land() { remaining = 0.0f; fade = 0.0f; clock = -1.0f; }

		template <class Air>
		float Hover(const Air& air) const
		{
			if (remaining > 0.0f)
				return 1.0f;
			return air.gravityRamp > 0.0f ? std::clamp(fade / air.gravityRamp, 0.0f, 1.0f) : 0.0f;
		}

		template <class Air>
		float Overtime(const Air& air) const
		{
			if (air.maxHangTime <= 0.0f)
				return 0.0f;   // no limit of its own
			return clock > air.maxHangTime ? std::clamp((clock - air.maxHangTime) / air.hangFade, 0.0f, 1.0f) : 0.0f;
		}

		// The hover can be felt: there is some, and the clock has not run it out.
		template <class Air>
		bool IsHovering(const Air& air) const { return Hover(air) > 0.0f && Overtime(air) < 1.0f; }

		// Once per frame, after the vertical step. The clock starts with the first hover in the air and then runs
		// whether or not the body keeps hovering (pausing the fight does not reset it).
		template <class Air>
		void Advance(float deltaTime, bool grounded, const Air& air)
		{
			if (!grounded)
			{
				if (clock >= 0.0f)
					clock += deltaTime;
				else if (remaining > 0.0f)
					clock = 0.0f;
			}
			if (remaining > 0.0f)
				fade = air.gravityRamp;
			else
				fade = std::max(0.0f, fade - deltaTime);
			remaining = std::max(0.0f, remaining - deltaTime);
		}
	};

	// The height a vertical speed lifts a body by under `gravity` (the slight rise of a hit).
	inline float RiseHeight(float upwardSpeed, float gravity)
	{
		return gravity > 1e-3f ? upwardSpeed * upwardSpeed / (2.0f * gravity) : 0.0f;
	}

	// What a melee hit in the air does to its victim's position: the displacement it covers. Horizontal: closing the
	// gap to the attack's ideal distance - only a gap up to pullRange (a victim further off than that is out of the
	// attack's reach: nothing pulls it), never pushing it away. Vertical: to the attacker's height plus the hit's
	// `rise` - or just the rise if the heights are more than pullMaxHeight apart.
	struct Pull
	{
		float dx = 0.0f;
		float dy = 0.0f;
		float dz = 0.0f;
		bool horizontal = false;
	};

	template <class Air>
	inline Pull PlanPull(float attackerX, float attackerY, float attackerZ, float victimX, float victimY, float victimZ,
		float idealDistance, float rise, const Air& air)
	{
		Pull pull;
		const float toAttackerX = attackerX - victimX;
		const float toAttackerZ = attackerZ - victimZ;
		const float distance = std::sqrt(toAttackerX * toAttackerX + toAttackerZ * toAttackerZ);
		const float gap = distance - idealDistance;
		if (distance > 1e-3f && gap > 0.0f && gap <= air.pullRange)
		{
			pull.dx = toAttackerX / distance * gap;
			pull.dz = toAttackerZ / distance * gap;
			pull.horizontal = true;
		}
		pull.dy = std::fabs(attackerY - victimY) <= air.pullMaxHeight ? attackerY + rise - victimY : rise;
		return pull;
	}

	// The velocity that covers what is left of a pull (error = destination - position) by its end. Capped, so a long
	// way is not a teleport.
	struct Velocity
	{
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
	};

	inline Velocity PullVelocity(float errorX, float errorY, float errorZ, float timeLeft, float deltaTime, float maxSpeed)
	{
		const float time = std::max(timeLeft, std::max(deltaTime, 1e-3f));
		Velocity velocity{ errorX / time, errorY / time, errorZ / time };
		const float speed = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y + velocity.z * velocity.z);
		if (speed > maxSpeed && speed > 1e-6f)
		{
			const float scale = maxSpeed / speed;
			velocity.x *= scale;
			velocity.y *= scale;
			velocity.z *= scale;
		}
		return velocity;
	}

	// The launcher thrown in the air: weaker than the ground launch, weaker with every one that connected since
	// landing (strain), never below the floor. Melee hits landed in an air combo heal the strain (launchRecover each).
	template <class Air>
	inline float LaunchScale(float strain, const Air& air)
	{
		return std::clamp(air.launchScale * std::pow(air.launchDecay, std::max(strain, 0.0f)), air.launchMinScale, 1.0f);
	}
}
