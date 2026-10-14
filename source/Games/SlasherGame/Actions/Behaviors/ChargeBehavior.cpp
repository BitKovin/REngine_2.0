#include <Actions/MoveBehavior.h>
#include <Actions/MoveRunner.h>
#include <Characters/CombatCharacter.h>

#include <Physics.h>

#include <algorithm>

// "Charge": long-range rush (enemy heavy). Windup until chargeStart (tracking from the move data turns toward
// the target), then runs forward with limited homing until it hits something, crashes into a wall or times
// out. Damage comes from the move's hit windows (e.g. a long bone-sphere window). Use "loop": true on the move
// so the run clip loops while charging.
struct ChargeParams : MoveBehaviorParams
{
	float speed = 10.0f;
	float chargeStart = 0.3f;        // clip time when the rush begins
	float maxDuration = 1.5f;        // local seconds of rushing
	float homingRate = 60.0f;        // deg/s toward the target while rushing
	bool stopOnHit = true;
	bool crashStagger = true;        // a wall crash plays this body's Stagger reaction

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ChargeParams, speed, chargeStart, maxDuration, homingRate, stopOnHit, crashStagger)

	bool Validate(const std::string& path, DataLoadReport& report) const
	{
		if (speed <= 0.0f) report.Error(path + ".speed", "must be > 0");
		if (chargeStart < 0.0f) report.Error(path + ".chargeStart", "must be >= 0");
		if (maxDuration <= 0.0f) report.Error(path + ".maxDuration", "must be > 0");
		return report.Ok();
	}
};

class ChargeBehavior : public MoveBehavior
{
public:
	void OnStart(MoveContext& context) override
	{
		(void)context;
		rushing = false;
		finished = false;
		elapsed = 0.0f;
	}

	void OnTick(MoveContext& context) override
	{
		CombatCharacter& character = context.Character;
		const ChargeParams& params = context.GetParams<ChargeParams>();

		if (finished || context.Time < params.chargeStart)
			return;

		rushing = true;
		elapsed += context.DeltaTime;

		if (Entity* target = character.GetSoftTarget(); target && params.homingRate > 0.0f)
			character.RequestFacing(target->Position - character.Position, params.homingRate);

		const glm::vec3 direction = character.GetForward();

		const glm::vec3 chest = character.GetTargetPoint();
		const float probe = params.speed * context.DeltaTime + character.GetData()->movement.radius + 0.1f;
		const bool crashed = Physics::LineTrace(chest, chest + direction * probe, BodyType::World, {}, { &character }).hasHit;

		bool landedHit = false;
		for (const auto& victims : context.Runner.GetPrimary().hitVictims)
			landedHit = landedHit || !victims.empty();

		if (crashed || (params.stopOnHit && landedHit) || elapsed >= params.maxDuration)
		{
			Finish();
			if (crashed && params.crashStagger)
			{
				const MoveSetData* moveSet = character.GetMoves().GetMoveSet();
				const int stagger = moveSet ? moveSet->GetReactionMove(ReactionSlot::Stagger) : -1;
				if (stagger >= 0)
					character.GetMoves().StartForcedMove(stagger);   // replaces this move
			}
			return;
		}

		character.RequestVelocity(direction * params.speed, false);
	}

	bool Sustains(const MoveContext& context) const override
	{
		const ChargeParams& params = context.GetParams<ChargeParams>();
		return !finished && (rushing || context.Time < params.chargeStart);
	}

private:
	bool rushing = false;
	bool finished = false;
	float elapsed = 0.0f;

	void Finish()
	{
		finished = true;
		rushing = false;
	}
};

REGISTER_MOVE_BEHAVIOR("Charge", ChargeBehavior, ChargeParams)
