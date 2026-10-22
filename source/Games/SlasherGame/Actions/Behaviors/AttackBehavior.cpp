#include <Actions/MoveBehavior.h>
#include <Actions/MoveRunner.h>
#include <Characters/CombatCharacter.h>
#include <Combat/CombatRules.h>

#include <algorithm>
#include <cmath>
#include <vector>

// "Attack": melee attacks. Hit windows are processed by MoveRunner / WeaponSet; this behaviour is the target assist
// and the air combat of an attack (COMBAT_DESIGN.md §5.7):
//   * target: the controller's focus (AI), else the attack target all around the body - the one closest to the input
//     direction (facing without input), the previous attack's target favoured (CombatCharacter::FindAttackTarget)
//   * facing: turns toward the target (or the input direction) until tracking.until - never snaps; fast for the first
//     tracking.startTurnTime (startTurnRate), then at turnRate
//   * magnet: pulls the body to tracking.idealDistance on top of the clip's root motion until tracking.magnetUntil,
//     arriving by the next strike, so attacks without root motion still close in; nothing carries the body closer
//   * air (COMBAT_DESIGN.md §5.8, rules in Combat/AirJuggle.h): every attack works in the air too. The hits are what
//     matter there - a melee hit landed while the attacker is airborne starts its hover (almost no gravity,
//     CharacterData::airCombat) for as long as that attack runs and `hold` after it; swinging at air earns nothing.
//     The airborne victims of that attack hover as long as it runs too, and the attacker's `victimHold` after it: the
//     attacker's hits decide how long a juggled body stays up.
//     A successful launch that also launches the attacker gives the attacker the same hit-specific hover as its victim;
//     a time-triggered leap alone does not. The same hit lifts attacker and victim a little and - victim in the air too -
//     settles the victim once at the attack's ideal distance and the attacker's height
//     (CombatCharacter::SetAirPull; the destination is fixed at the hit, nothing follows the attacker). Before the
//     hover the attacker chases the target's height (rising with a launched target, coming down onto a grounded one);
//     hovering it keeps its own height. Landing, the move simply goes on on the ground. In the air the target is one
//     within targeting.airHeightRange above / below - an enemy standing far below is not chased.
//     A plunge (an attack whose air version dives - the drop) hops by plunge.rise as it starts in the air (the weapon
//     goes up), falls fast from plunge.start - steering onto the target below - and holds its strike from plunge.hold
//     until it lands; while it dives it is committed (MoveBehavior::Commits: no link, cancel or recovery in the air).
//     launchSelf: the attacker leaps too - at a clip time, or when one of the move's hits launches its target (the
//     launcher: attacker and target go up together). air.needsTarget: in the air the move only starts with something
//     to launch - an airborne target in reach (the launcher; without one a fresh hold falls back to AirDrop, combo
//     follow-ups never do); a launcher started in the air is weaker with every use since landing (airCombat.launch*).
struct AttackAirParams
{
	bool matchTargetHeight = true;   // before it hovers, chase the target's height while in the air
	bool needsTarget = false;        // in the air the move only starts with an airborne target in reach (the launcher)

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(AttackAirParams, matchTargetHeight, needsTarget)
};

struct AttackPlungeParams
{
	float start = -1.0f;             // clip time the plunge begins when the move runs in the air; < 0: no plunge
	float speed = 18.0f;             // m/s downward
	float hold = -1.0f;              // clip time the strike holds (hit window open) until landing; < 0: no hold
	float rise = 0.0f;               // m/s upward (at least) as the move starts in the air: the wind-up's hop before the dive

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(AttackPlungeParams, start, speed, hold, rise)

	bool IsSet() const { return start >= 0.0f; }
};

struct AttackSelfLaunchParams
{
	float time = -1.0f;              // clip time the attacker leaps; < 0: not by time
	bool onHit = false;              // leap when one of the move's hits launches its target (armor or a kill: no leap)
	float verticalSpeed = 10.0f;     // m/s; to rise level with the target: its launch speed * sqrt(own / its gravity)
	bool requireHeld = true;         // only while Attack is still held (tap = stay on the ground)

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(AttackSelfLaunchParams, time, onHit, verticalSpeed, requireHeld)

	bool IsSet() const { return time >= 0.0f || onHit; }
};

struct AttackParams : MoveBehaviorParams
{
	float locomotion = 0.0f;         // fraction of run speed input may add during the attack (0 = root motion only)
	AttackAirParams air;
	AttackPlungeParams plunge;
	AttackSelfLaunchParams launchSelf;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(AttackParams, locomotion, air, plunge, launchSelf)

	bool Validate(const std::string& path, DataLoadReport& report) const
	{
		if (locomotion < 0.0f || locomotion > 1.0f) report.Error(path + ".locomotion", "must be in [0, 1]");
		if (plunge.IsSet() && plunge.speed <= 0.0f) report.Error(path + ".plunge.speed", "must be > 0");
		if (plunge.hold >= 0.0f && plunge.hold < plunge.start) report.Error(path + ".plunge.hold", "must not precede plunge.start");
		if (plunge.rise < 0.0f) report.Error(path + ".plunge.rise", "must be >= 0");
		if (launchSelf.IsSet() && launchSelf.verticalSpeed <= 0.0f) report.Error(path + ".launchSelf.verticalSpeed", "must be > 0");
		return report.Ok();
	}
};

class AttackBehavior : public MoveBehavior
{
public:
	bool CanStart(const CombatCharacter& character, const MoveData& move) const override
	{
		const AttackParams& params = static_cast<const AttackParams&>(*move.behaviorParams);
		if (!params.air.needsTarget || character.IsGrounded())
			return true;

		// Something to launch: the controller's focus (AI) or a hostile the search finds - in the air, in reach.
		EntityHandle focus = character.GetInput().focus;
		if (Entity* target = ResolveAlive(focus))
			return IsLaunchable(character, *target, move);
		return FindTarget(character, move, params) != nullptr;
	}

	void OnStart(MoveContext& context) override
	{
		CombatCharacter& character = context.Character;
		const MoveData& move = context.Move;
		const AttackParams& params = context.GetParams<AttackParams>();
		const CharacterInput& input = character.GetInput();

		const glm::vec3 inputDirection = MathHelper::XZ(input.move);
		const bool hasInput = glm::dot(inputDirection, inputDirection) > 0.01f;

		// A focus target from the controller (AI target, possession) wins over the search.
		EntityHandle focus = input.focus;
		Entity* target = ResolveAlive(focus);
		if (target == nullptr)
			target = FindTarget(character, move, params);
		character.SetSoftTarget(target);

		// The drop's wind-up: the weapon goes up with a small hop before the dive.
		if (params.plunge.IsSet() && params.plunge.rise > 0.0f && !character.IsGrounded())
			character.LiftTo(params.plunge.rise);

		turnDirection = target == nullptr && hasInput ? inputDirection : glm::vec3(0.0f);
		plunging = false;
		plunged = false;
		selfLaunched = false;
		hitInAir = false;
		airVictims.clear();

		// A launcher started in the air is weaker the more of them connected since landing (combo hits heal that). Its
		// strength is fixed now (the hits read it, CombatCharacter::GetLaunchScale); the use is counted when a
		// launching hit connects (OnHitDealt).
		bool launches = false;
		for (const HitWindowData& hit : move.hits)
			launches = launches || hit.reaction == HitReaction::Launch;
		airLauncher = launches && !character.IsGrounded();
		launchStrainTaken = false;
		launchScale = airLauncher ? character.PeekAirLaunchScale() : 1.0f;
		character.SetLaunchScale(launchScale);
	}

	void OnEnd(MoveContext& context, MoveEndReason reason) override
	{
		(void)reason;
		context.Character.SetLaunchScale(1.0f);
	}

	void OnTick(MoveContext& context) override
	{
		CombatCharacter& character = context.Character;
		const MoveData& move = context.Move;
		const AttackParams& params = context.GetParams<AttackParams>();
		const TrackingData& tracking = move.tracking;
		const bool airborne = !character.IsGrounded();

		if (params.locomotion > 0.0f)
			character.RequestLocomotion(params.locomotion);

		if (params.launchSelf.time >= 0.0f && !selfLaunched && context.Time >= params.launchSelf.time)
			TryLaunchSelf(context);

		// A plunge dives once, from the air; landing ends it and the move goes on on the ground.
		if (!plunged && params.plunge.IsSet() && context.Time >= params.plunge.start && airborne)
			plunging = plunged = true;
		if (plunging && !airborne)
			plunging = false;

		// The hover is earned by a landed hit (OnHitDealt) and carried by the attack that landed it: only then does the
		// swing keep the body up. An attack that has not hit anything in the air gets nothing for being in the air.
		if (plunging)
			character.RequestVelocity(glm::vec3(0.0f, -params.plunge.speed, 0.0f), true);
		else if (airborne && hitInAir)
		{
			character.EnterAirCombat(character.GetData()->airCombat.hold);
			CarryVictimHovers();
		}

		const float turnRate = context.Time < tracking.startTurnTime ? std::max(tracking.startTurnRate, tracking.turnRate) : tracking.turnRate;

		Entity* target = character.GetSoftTarget();
		if (target == nullptr)
		{
			if (context.Time <= tracking.resolvedUntil && glm::dot(turnDirection, turnDirection) > 1e-6f)
				character.RequestFacing(turnDirection, turnRate, tracking.turnSharpness);

			const glm::vec3 inputDirection = MathHelper::XZ(character.GetInput().move);
			if (tracking.advance > 0.0f && context.Time <= tracking.resolvedMagnetUntil && glm::dot(inputDirection, inputDirection) > 0.01f)
				character.RequestAdditiveVelocity(character.GetForward() * tracking.advance);
			return;
		}

		const glm::vec3 offset = MathHelper::XZ(target->Position - character.Position);
		const float distance = glm::length(offset);
		if (distance > 1e-3f && context.Time <= tracking.resolvedUntil)
			character.RequestFacing(offset, turnRate, tracking.turnSharpness);

		// Arrive by the next strike (local seconds); once the strikes run, keep up within a tenth of a second.
		const float playback = std::max(move.speed * character.GetAttackSpeedMultiplier(), 0.01f);
		float nextStrike = -1.0f;
		for (const HitWindowData& hit : move.hits)
			if (hit.start > context.Time && (nextStrike < 0.0f || hit.start < nextStrike))
				nextStrike = hit.start;
		const float arrival = nextStrike >= 0.0f ? std::max((nextStrike - context.Time) / playback, MinArrivalTime) : MagnetFollowTime;

		// Diving, the magnet steers the plunge onto the target below (the dive itself has no horizontal speed).
		const bool closeGap = tracking.magnetRange > 0.0f && distance <= tracking.magnetRange && context.Time <= tracking.resolvedMagnetUntil;
		character.RequestMagnet(target->Position, tracking.idealDistance, arrival, tracking.maxMagnetSpeed, closeGap);

		// In the air, chase the target's height until a hit starts the hover: its vertical speed (a launched target
		// rises, a hanging one stays, a grounded one stands) plus a pull toward its height - the attacker reaches an
		// enemy that hangs above it, or comes down onto a grounded one. Hovering, the attacker keeps its own height (the
		// small gravity is its own, it falls before the enemies it juggles); the hits settle the enemy level with it.
		// A plunge move's height is its own: the hop, then the dive.
		if (airborne && !params.plunge.IsSet() && params.air.matchTargetHeight && distance <= tracking.searchRange)
		{
			auto* body = dynamic_cast<CombatCharacter*>(target);
			const bool targetAirborne = body != nullptr && !body->IsGrounded();
			if (!(character.IsAirHovering() && targetAirborne))
			{
				const float follow = body != nullptr ? body->GetVelocity().y : 0.0f;
				const float pull = std::clamp((target->Position.y - character.Position.y) * HeightMatchRate, -MaxHeightMatchSpeed, MaxHeightMatchSpeed);
				character.SetVerticalSpeed(follow + pull);
			}
		}
	}

	// What a landed hit means in the air (§5.8). Order matters: the victim's own part (hover, lift) already ran.
	void OnHitDealt(MoveContext& context, const HitResult& result) override
	{
		const AttackParams& params = context.GetParams<AttackParams>();
		CombatCharacter& character = context.Character;
		const bool launching = result.reaction == HitReaction::Launch;

		// A launcher started in the air: one more used up (healed by the combo's melee hits).
		if (airLauncher && !launchStrainTaken && launching)
		{
			launchStrainTaken = true;
			character.AddAirLaunchStrain();
		}

		auto* victim = dynamic_cast<CombatCharacter*>(result.victim);
		if (result.request != nullptr
			&& AirJuggle::ShouldShareLaunchHover(launching, params.launchSelf.onHit, selfLaunched,
				result.request->source == DamageSource::Melee)
			&& TryLaunchSelf(context))
		{
			// The victim receives this same stall in CombatCharacter::ApplyAirHit. Give the attacker the same hover
			// duration only when the launch actually lifted both bodies; a whiff, blocked launch, or held-input failure
			// must not grant it. The same hold as the victim's (CombatCharacter::ApplyAirHit): the attacker's victimHold.
			const float hold = AirJuggle::ResolveHold(result.request->airStall, character.GetData()->airCombat.victimHold);
			if (hold > 0.0f)
				character.EnterAirCombat(hold);
			else
				character.EndAirCombat();
		}

		// The remaining handling is for melee hits landed while the attacker is airborne. The launcher above also earns
		// hover when it successfully launches both bodies, even though its hit begins while the attacker is grounded.
		if (plunging || victim == nullptr || result.request == nullptr || result.request->source != DamageSource::Melee || character.IsGrounded())
			return;

		// The hit starts the hover; the attack that landed it carries it (OnTick).
		hitInAir = true;
		character.EnterAirCombat(character.GetData()->airCombat.hold);

		if (launching || result.killed || victim->IsGrounded())
			return;   // a launch keeps its own speeds; a corpse or a grounded victim has nothing to settle

		// The attack carries the victim's hover as it carries its own (OnTick): the victim hangs while the attack runs
		// and its hold after, so the next attack's wind-up has something to hit.
		const float victimHold = AirJuggle::ResolveHold(result.request->airStall, character.GetData()->airCombat.victimHold);
		if (victimHold > 0.0f)
			airVictims.push_back({ EntityHandle::FromEntity(victim), victimHold });

		// Both rise a little - and the victim is settled at the attack's distance and the attacker's height, once.
		const AirCombatData& air = character.GetData()->airCombat;
		const float lift = result.request->airLift >= 0.0f ? result.request->airLift : air.hitLift;
		character.LiftTo(lift);
		character.RecoverAirLaunch();

		const float rise = AirJuggle::RiseHeight(lift, character.GetData()->movement.gravity);
		const AirJuggle::Pull pull = AirJuggle::PlanPull(character.Position.x, character.Position.y, character.Position.z,
			victim->Position.x, victim->Position.y, victim->Position.z, context.Move.tracking.idealDistance, rise, victim->GetData()->airCombat);
		victim->SetAirPull(glm::vec3(pull.dx, pull.dy, pull.dz), pull.horizontal);
	}

	// A plunge holds its strike (and the open hit window) while it falls.
	bool HoldsClock(const MoveContext& context) const override
	{
		const AttackParams& params = context.GetParams<AttackParams>();
		return plunging && params.plunge.hold >= 0.0f && context.Time >= params.plunge.hold && !context.Character.IsGrounded();
	}

	// A plunge cannot be left while it dives: it ends on the ground (combo link, cancels and recovery wait for that).
	bool Commits(const MoveContext& context) const override
	{
		return plunging && !context.Character.IsGrounded();
	}

private:
	// The target this move picks now (COMBAT_DESIGN.md §5.7, §5.8). On the ground every hostile in searchRange. In the
	// air one within targeting.airHeightRange above / below - except for a plunge, which dives onto what is below; and a
	// move that needs a target there (the launcher) wants something to launch: an airborne hostile within its reach.
	static Entity* FindTarget(const CombatCharacter& character, const MoveData& move, const AttackParams& params)
	{
		const TrackingData& tracking = move.tracking;
		if (character.IsGrounded() || params.plunge.IsSet())
			return character.FindAttackTarget(tracking.searchRange);
		if (params.air.needsTarget)
			return character.FindAttackTarget(LaunchReach(move), character.GetAttackHeightRange(), true);
		return character.FindAttackTarget(tracking.searchRange, character.GetAttackHeightRange());
	}

	// How far a target may be for a launcher in the air: the gap the attack closes on its own (the magnet).
	static float LaunchReach(const MoveData& move)
	{
		return std::max(move.tracking.magnetRange, move.tracking.idealDistance);
	}

	// The same test for one given body (the controller's focus): airborne, in reach, within the air height range.
	static bool IsLaunchable(const CombatCharacter& character, const Entity& target, const MoveData& move)
	{
		const auto* body = dynamic_cast<const CombatCharacter*>(&target);
		if (body == nullptr || body->IsGrounded())
			return false;
		const glm::vec3 offset = target.Position - character.Position;
		const float height = character.GetAttackHeightRange();
		return glm::length(MathHelper::XZ(offset)) <= LaunchReach(move) && (height < 0.0f || std::abs(offset.y) <= height);
	}

	// Once per move; with requireHeld only while Attack is still held. A time-triggered leap alone follows a normal
	// jump arc. When onHit successfully launches a target, OnHitDealt also grants the attacker the target's stall hover.
	// A weaker (air) launcher lifts its attacker less, in step with its victim.
	bool TryLaunchSelf(MoveContext& context)
	{
		selfLaunched = true;
		const AttackParams& params = context.GetParams<AttackParams>();
		CombatCharacter& character = context.Character;
		if (params.launchSelf.requireHeld && !character.GetInput().Get(InputButton::Attack).held)
			return false;
		character.Launch(params.launchSelf.verticalSpeed * launchScale);
		return true;
	}

	// Keeps the airborne victims of this attack hovering while it runs. Only a hover still felt is extended: a victim
	// slammed since (stall 0), landed or out of hang time is left to fall.
	void CarryVictimHovers()
	{
		for (AirVictim& carried : airVictims)
		{
			auto* victim = dynamic_cast<CombatCharacter*>(ResolveAlive(carried.handle));
			if (victim != nullptr && !victim->IsGrounded() && victim->IsAirHovering())
				victim->EnterAirCombat(carried.hold);
		}
	}

	struct AirVictim
	{
		EntityHandle handle;
		float hold = 0.0f;                            // s it hovers after this attack ends
	};

	static constexpr float MagnetFollowTime = 0.1f;   // s: arrival time once the strikes run
	static constexpr float MinArrivalTime = 0.05f;
	static constexpr float HeightMatchRate = 6.0f;    // 1/s
	static constexpr float MaxHeightMatchSpeed = 4.0f;

	glm::vec3 turnDirection = glm::vec3(0.0f);        // input direction at the start (no target)
	bool plunging = false;                            // diving now
	bool plunged = false;                             // the plunge happened (once per move)
	bool selfLaunched = false;
	bool hitInAir = false;                            // this attack landed a melee hit while airborne: it carries the hover
	std::vector<AirVictim> airVictims;                // airborne bodies it hit: it carries their hover too
	float launchScale = 1.0f;                         // strength of this move's launching hits (air launcher strain)
	bool airLauncher = false;                         // started in the air with a launching hit
	bool launchStrainTaken = false;                   // its launch connected and was counted
};

REGISTER_MOVE_BEHAVIOR("Attack", AttackBehavior, AttackParams)
