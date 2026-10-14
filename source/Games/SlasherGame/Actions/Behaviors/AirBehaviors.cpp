#include <Actions/MoveBehavior.h>
#include <Actions/MoveRunner.h>
#include <Characters/CombatCharacter.h>

#include <Time.hpp>

#include <algorithm>

// "Jump": ground jump and air jumps (double jump). Started in the air it consumes one of maxAirJumps.
// The body keeps full air control while the jump move runs; the move ends on landing. Horizontal speed carries
// into the jump up to carrySpeed (a dash-jump leaves at run speed, not dash speed).
struct JumpParams : MoveBehaviorParams
{
	float velocity = 8.0f;
	int maxAirJumps = 1;             // only checked when the move starts in the air
	float airControl = 1.0f;         // locomotion scale during the move
	float carrySpeed = 1.0f;         // horizontal speed kept at takeoff, at most this fraction of the run speed

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(JumpParams, velocity, maxAirJumps, airControl, carrySpeed)

	bool Validate(const std::string& path, DataLoadReport& report) const
	{
		if (velocity <= 0.0f) report.Error(path + ".velocity", "must be > 0");
		if (maxAirJumps < 0) report.Error(path + ".maxAirJumps", "must be >= 0");
		if (carrySpeed < 0.0f) report.Error(path + ".carrySpeed", "must be >= 0");
		return report.Ok();
	}
};

class JumpBehavior : public MoveBehavior
{
public:
	bool CanStart(const CombatCharacter& character, const MoveData& move) const override
	{
		const JumpParams& params = static_cast<const JumpParams&>(*move.behaviorParams);
		return character.IsGrounded() || character.GetMovementResources().airJumpsUsed < params.maxAirJumps;
	}

	void OnStart(MoveContext& context) override
	{
		CombatCharacter& character = context.Character;
		const JumpParams& params = context.GetParams<JumpParams>();
		if (!character.IsGrounded())
			++character.GetMovementResources().airJumpsUsed;
		character.Launch(params.velocity);
		character.LimitHorizontalSpeed(params.carrySpeed * character.GetData()->movement.runSpeed * character.GetMoveSpeedMultiplier());
		airborneSeen = false;
	}

	void OnTick(MoveContext& context) override
	{
		context.Character.RequestLocomotion(context.GetParams<JumpParams>().airControl);
		airborneSeen = airborneSeen || !context.Character.IsGrounded();
	}

	bool WantsToEnd(const MoveContext& context) const override
	{
		return airborneSeen && context.Character.IsGrounded();
	}

private:
	bool airborneSeen = false;
};

REGISTER_MOVE_BEHAVIOR("Jump", JumpBehavior, JumpParams)

// "Glide": short glide while the jump button is held in the air - the move runner starts it by itself once the
// body falls with the button held for holdDelay (no second press). Limited total time per airtime (recharges on
// landing). The move sustains at its endTime while gliding.
struct GlideParams : MoveBehaviorParams
{
	float maxDuration = 1.5f;
	float gravityScale = 0.15f;
	float maxFallSpeed = 2.5f;
	float airControl = 0.8f;
	float holdDelay = 0.15f;         // seconds the jump button must be held before gliding

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(GlideParams, maxDuration, gravityScale, maxFallSpeed, airControl, holdDelay)

	bool Validate(const std::string& path, DataLoadReport& report) const
	{
		if (maxDuration <= 0.0f) report.Error(path + ".maxDuration", "must be > 0");
		if (gravityScale < 0.0f) report.Error(path + ".gravityScale", "must be >= 0");
		if (maxFallSpeed <= 0.0f) report.Error(path + ".maxFallSpeed", "must be > 0");
		if (holdDelay < 0.0f) report.Error(path + ".holdDelay", "must be >= 0");
		return report.Ok();
	}
};

class GlideBehavior : public MoveBehavior
{
public:
	bool CanStart(const CombatCharacter& character, const MoveData& move) const override
	{
		const GlideParams& params = static_cast<const GlideParams&>(*move.behaviorParams);
		const ButtonState& jump = character.GetInput().Get(InputButton::Jump);
		return !character.IsGrounded()
			&& jump.held && Time::GameTime - jump.pressTime >= params.holdDelay
			&& character.GetVelocity().y < 0.0f
			&& character.GetMovementResources().glideTimeUsed < params.maxDuration;
	}

	void OnTick(MoveContext& context) override
	{
		CombatCharacter& character = context.Character;
		const GlideParams& params = context.GetParams<GlideParams>();

		character.GetMovementResources().glideTimeUsed += context.DeltaTime;
		character.RequestGravityScale(params.gravityScale);
		character.RequestMaxFallSpeed(params.maxFallSpeed);
		character.RequestLocomotion(params.airControl);
		character.RequestGliding();
	}

	bool WantsToEnd(const MoveContext& context) const override
	{
		const CombatCharacter& character = context.Character;
		const GlideParams& params = context.GetParams<GlideParams>();
		return character.IsGrounded()
			|| !character.GetInput().Get(InputButton::Jump).held
			|| character.GetMovementResources().glideTimeUsed >= params.maxDuration;
	}

	bool Sustains(const MoveContext& context) const override
	{
		return !WantsToEnd(context);
	}
};

REGISTER_MOVE_BEHAVIOR("Glide", GlideBehavior, GlideParams)
