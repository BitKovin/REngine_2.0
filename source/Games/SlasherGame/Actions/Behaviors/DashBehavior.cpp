#include <Actions/MoveBehavior.h>
#include <Actions/MoveRunner.h>
#include <Characters/CombatCharacter.h>

#include <Physics.h>

#include <algorithm>

enum class DashCurve : uint8_t { Invalid, Linear, EaseOut };
SLASHER_JSON_ENUM(DashCurve, { { DashCurve::Invalid, nullptr }, { DashCurve::Linear, "Linear" }, { DashCurve::EaseOut, "EaseOut" } })

// "Dash": fixed-distance displacement along the input direction (facing without input). Gravity is suspended
// while moving; i-frames come from the move's "invulnerable" windows. The displacement slows down to exitSpeed
// (the run speed by default), so the body runs out of the dash instead of stopping; during the rest of the clip
// input steers the body (tailLocomotion) until the move's cancel windows or recovery cut it.
struct DashParams : MoveBehaviorParams
{
	float distance = 6.0f;
	float duration = 0.25f;          // local seconds of displacement
	DashCurve curve = DashCurve::EaseOut;
	int airCharges = 1;              // dashes allowed per airtime
	float exitSpeed = 1.0f;          // speed when the displacement ends, as a fraction of the run speed
	float tailLocomotion = 1.0f;     // after the displacement input moves the body at this fraction of the run speed
	bool faceDirection = true;       // snap facing to the dash direction

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(DashParams, distance, duration, curve, airCharges, exitSpeed, tailLocomotion, faceDirection)

	bool Validate(const std::string& path, DataLoadReport& report) const
	{
		if (distance <= 0.0f) report.Error(path + ".distance", "must be > 0");
		if (duration <= 0.0f) report.Error(path + ".duration", "must be > 0");
		if (curve == DashCurve::Invalid) report.Error(path + ".curve", "expected Linear or EaseOut");
		if (airCharges < 0) report.Error(path + ".airCharges", "must be >= 0");
		if (exitSpeed < 0.0f) report.Error(path + ".exitSpeed", "must be >= 0");
		if (tailLocomotion < 0.0f || tailLocomotion > 1.0f) report.Error(path + ".tailLocomotion", "must be in [0, 1]");
		return report.Ok();
	}
};

class DashBehavior : public MoveBehavior
{
public:
	bool CanStart(const CombatCharacter& character, const MoveData& move) const override
	{
		const DashParams& params = static_cast<const DashParams&>(*move.behaviorParams);
		return character.IsGrounded() || character.GetMovementResources().airDashesUsed < params.airCharges;
	}

	void OnStart(MoveContext& context) override
	{
		CombatCharacter& character = context.Character;
		const DashParams& params = context.GetParams<DashParams>();

		const glm::vec3 inputDirection = MathHelper::XZ(character.GetInput().move);
		direction = glm::dot(inputDirection, inputDirection) > 0.01f ? glm::normalize(inputDirection) : character.GetForward();
		elapsed = 0.0f;
		moving = true;

		if (params.faceDirection)
			character.SnapFacing(direction);
		if (!character.IsGrounded())
			++character.GetMovementResources().airDashesUsed;
	}

	void OnTick(MoveContext& context) override
	{
		CombatCharacter& character = context.Character;
		const DashParams& params = context.GetParams<DashParams>();

		if (!moving)
		{
			character.RequestLocomotion(params.tailLocomotion);
			return;
		}

		// EaseOut: speed falls linearly from 2 * average - exit to exit, so the distance stays `distance` and the
		// body leaves the dash at the exit speed. The exit never exceeds the average (that would speed up).
		elapsed += context.DeltaTime;
		const float t = std::clamp(elapsed / params.duration, 0.0f, 1.0f);
		const float averageSpeed = params.distance / params.duration;
		const float exitSpeed = std::min(params.exitSpeed * character.GetData()->movement.runSpeed * character.GetMoveSpeedMultiplier(), averageSpeed);
		const float speed = params.curve == DashCurve::EaseOut ? glm::mix(2.0f * averageSpeed - exitSpeed, exitSpeed, t) : averageSpeed;

		// Stop at walls instead of grinding along them.
		const glm::vec3 chest = character.GetTargetPoint();
		const float probe = std::max(speed * context.DeltaTime, 0.05f) + character.GetData()->movement.radius;
		const Physics::HitResult wall = Physics::LineTrace(chest, chest + direction * probe, BodyType::World, {}, { &character });

		if (t >= 1.0f || wall.hasHit)
		{
			moving = false;
			character.RequestVelocity(direction * (wall.hasHit ? 0.0f : exitSpeed), true);
			return;
		}

		character.RequestVelocity(direction * speed, true);   // vertical 0: gravity suspended
	}

private:
	glm::vec3 direction = glm::vec3(0.0f, 0.0f, 1.0f);
	float elapsed = 0.0f;
	bool moving = false;
};

REGISTER_MOVE_BEHAVIOR("Dash", DashBehavior, DashParams)
