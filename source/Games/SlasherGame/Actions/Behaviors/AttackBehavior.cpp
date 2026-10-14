#include <Actions/MoveBehavior.h>
#include <Actions/MoveRunner.h>
#include <Characters/CombatCharacter.h>
#include <Combat/CombatRules.h>

#include <algorithm>

// "Attack": melee attacks. Hit windows are processed by MoveRunner / WeaponSet; this behaviour is the target assist
// and the air combat of an attack (COMBAT_DESIGN.md §5.7):
//   * target: the controller's focus (AI), else the attack target all around the body - the one closest to the input
//     direction (facing without input), the previous attack's target favoured (CombatCharacter::FindAttackTarget)
//   * facing: turns toward the target (or the input direction) until tracking.until - never snaps; fast for the first
//     tracking.startTurnTime (startTurnRate), then at turnRate
//   * magnet: pulls the body to tracking.idealDistance on top of the clip's root motion until tracking.magnetUntil,
//     arriving by the next strike, so attacks without root motion still close in; nothing carries the body closer
//   * air (COMBAT_DESIGN.md §5.8): every attack works in the air too. Run in the air it keeps the body in air combat
//     (it rises on its arc, then hangs and sinks slowly - CharacterData::airCombat), follows the target's vertical
//     motion (rising and hanging with a launched target, coming down onto a grounded one), and each hit it lands on an
//     airborne victim lifts the attacker as much as the victim. Landing, the move simply goes on on the ground. A
//     plunge (an attack whose air version dives) falls fast from plunge.start and holds its strike from plunge.hold
//     until it lands. launchSelf: the attacker leaps too - at a clip time, or when one of the move's hits launches its
//     target (the launcher: attacker and target go up together and hang for the hit's air stall).
struct AttackAirParams
{
	bool matchTargetHeight = true;   // follow the target's vertical motion while in the air

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(AttackAirParams, matchTargetHeight)
};

struct AttackPlungeParams
{
	float start = -1.0f;             // clip time the plunge begins when the move runs in the air; < 0: no plunge
	float speed = 18.0f;             // m/s downward
	float hold = -1.0f;              // clip time the strike holds (hit window open) until landing; < 0: no hold

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(AttackPlungeParams, start, speed, hold)
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
		if (plunge.start >= 0.0f && plunge.speed <= 0.0f) report.Error(path + ".plunge.speed", "must be > 0");
		if (plunge.hold >= 0.0f && plunge.hold < plunge.start) report.Error(path + ".plunge.hold", "must not precede plunge.start");
		if (launchSelf.IsSet() && launchSelf.verticalSpeed <= 0.0f) report.Error(path + ".launchSelf.verticalSpeed", "must be > 0");
		return report.Ok();
	}
};

class AttackBehavior : public MoveBehavior
{
public:
	void OnStart(MoveContext& context) override
	{
		CombatCharacter& character = context.Character;
		const MoveData& move = context.Move;
		const CharacterInput& input = character.GetInput();

		const glm::vec3 inputDirection = MathHelper::XZ(input.move);
		const bool hasInput = glm::dot(inputDirection, inputDirection) > 0.01f;

		// A focus target from the controller (AI target, possession) wins over the search.
		EntityHandle focus = input.focus;
		Entity* target = ResolveAlive(focus);
		if (target == nullptr)
			target = character.FindAttackTarget(move.tracking.searchRange);
		character.SetSoftTarget(target);

		turnDirection = target == nullptr && hasInput ? inputDirection : glm::vec3(0.0f);
		plunging = false;
		plunged = false;
		selfLaunched = false;
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
			TryLaunchSelf(context, character.GetData()->airCombat.hold);

		// A plunge dives once, from the air; landing ends it and the move goes on on the ground.
		if (!plunged && params.plunge.start >= 0.0f && context.Time >= params.plunge.start && airborne)
			plunging = plunged = true;
		if (plunging && !airborne)
			plunging = false;

		if (plunging)
			character.RequestVelocity(glm::vec3(0.0f, -params.plunge.speed, 0.0f), true);
		else if (airborne)
			character.EnterAirCombat(character.GetData()->airCombat.hold);

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

		const bool closeGap = tracking.magnetRange > 0.0f && distance <= tracking.magnetRange && context.Time <= tracking.resolvedMagnetUntil && !plunging;
		character.RequestMagnet(target->Position, tracking.idealDistance, arrival, tracking.maxMagnetSpeed, closeGap);

		// In the air, move with the target: its vertical speed (a launched target rises, a hit one hangs, a grounded one
		// stands) plus a pull toward its height - attacker and target stay level, or the attacker comes down onto it.
		if (airborne && !plunging && params.air.matchTargetHeight && distance <= tracking.searchRange)
		{
			auto* body = dynamic_cast<CombatCharacter*>(target);
			const float follow = body != nullptr ? body->GetVelocity().y : 0.0f;
			const float pull = std::clamp((target->Position.y - character.Position.y) * HeightMatchRate, -MaxHeightMatchSpeed, MaxHeightMatchSpeed);
			character.SetVerticalSpeed(follow + pull);
		}
	}

	void OnHitDealt(MoveContext& context, const HitResult& result) override
	{
		const AttackParams& params = context.GetParams<AttackParams>();
		if (params.launchSelf.onHit && !selfLaunched && result.reaction == HitReaction::Launch)
		{
			TryLaunchSelf(context, result.request ? result.request->airStall : 0.0f);
			return;
		}

		// A hit on an airborne victim lifts it; an airborne attacker gets the same lift, so the two stay level.
		auto* victim = dynamic_cast<CombatCharacter*>(result.victim);
		if (!plunging && result.request && !context.Character.IsGrounded() && victim != nullptr && !victim->IsGrounded())
			context.Character.LiftTo(result.request->airLift);
	}

	// A plunge holds its strike (and the open hit window) while it falls.
	bool HoldsClock(const MoveContext& context) const override
	{
		const AttackParams& params = context.GetParams<AttackParams>();
		return plunging && params.plunge.hold >= 0.0f && context.Time >= params.plunge.hold && !context.Character.IsGrounded();
	}

private:
	// Once per move; with requireHeld only while Attack is still held. The attacker hangs as long as its target.
	void TryLaunchSelf(MoveContext& context, float airStall)
	{
		selfLaunched = true;
		const AttackParams& params = context.GetParams<AttackParams>();
		CombatCharacter& character = context.Character;
		if (params.launchSelf.requireHeld && !character.GetInput().Get(InputButton::Attack).held)
			return;
		character.Launch(params.launchSelf.verticalSpeed);
		character.EnterAirCombat(std::max(airStall, character.GetData()->airCombat.hold));
	}

	static constexpr float MagnetFollowTime = 0.1f;   // s: arrival time once the strikes run
	static constexpr float MinArrivalTime = 0.05f;
	static constexpr float HeightMatchRate = 6.0f;    // 1/s
	static constexpr float MaxHeightMatchSpeed = 4.0f;

	glm::vec3 turnDirection = glm::vec3(0.0f);        // input direction at the start (no target)
	bool plunging = false;                            // diving now
	bool plunged = false;                             // the plunge happened (once per move)
	bool selfLaunched = false;
};

REGISTER_MOVE_BEHAVIOR("Attack", AttackBehavior, AttackParams)
