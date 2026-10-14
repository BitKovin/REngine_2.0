#include <Actions/MoveBehavior.h>
#include <Actions/MoveRunner.h>
#include <Characters/CombatCharacter.h>

#include <SkeletalMesh.hpp>

#include <algorithm>

// "Reaction": hit reactions. Applies the knockback handed over by the hit funnel (decaying horizontal push,
// launch upward). With holdWhileBroken the move sustains while the body is poise-broken, so a short stun clip
// covers the whole finishable window.
struct ReactionParams : MoveBehaviorParams
{
	float knockbackScale = 1.0f;
	float knockbackTime = 0.25f;     // seconds over which the horizontal push decays to 0
	float launchScale = 1.0f;
	bool holdWhileBroken = false;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ReactionParams, knockbackScale, knockbackTime, launchScale, holdWhileBroken)

	bool Validate(const std::string& path, DataLoadReport& report) const
	{
		if (knockbackScale < 0.0f) report.Error(path + ".knockbackScale", "must be >= 0");
		if (knockbackTime <= 0.0f) report.Error(path + ".knockbackTime", "must be > 0");
		if (launchScale < 0.0f) report.Error(path + ".launchScale", "must be >= 0");
		return report.Ok();
	}
};

class ReactionBehavior : public MoveBehavior
{
public:
	void OnStart(MoveContext& context) override
	{
		const ReactionParams& params = context.GetParams<ReactionParams>();
		const glm::vec3 knockback = context.Character.TakePendingKnockback();

		push = MathHelper::XZ(knockback) * params.knockbackScale;
		elapsed = 0.0f;
		if (knockback.y > 0.0f)
			context.Character.Launch(knockback.y * params.launchScale);
	}

	void OnTick(MoveContext& context) override
	{
		const ReactionParams& params = context.GetParams<ReactionParams>();
		elapsed += context.DeltaTime;
		if (elapsed < params.knockbackTime)
			context.Character.RequestVelocity(push * (1.0f - elapsed / params.knockbackTime), false);
	}

	bool Sustains(const MoveContext& context) const override
	{
		return context.GetParams<ReactionParams>().holdWhileBroken && context.Character.IsBroken();
	}

private:
	glm::vec3 push = glm::vec3(0.0f);
	float elapsed = 0.0f;
};

REGISTER_MOVE_BEHAVIOR("Reaction", ReactionBehavior, ReactionParams)

// "Death": the death clip, held on its last frame; optionally hands the body to ragdoll.
struct DeathParams : MoveBehaviorParams
{
	bool ragdoll = false;
	float ragdollDelay = 0.0f;       // clip seconds before switching to ragdoll

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(DeathParams, ragdoll, ragdollDelay)

	bool Validate(const std::string& path, DataLoadReport& report) const
	{
		if (ragdollDelay < 0.0f) report.Error(path + ".ragdollDelay", "must be >= 0");
		return report.Ok();
	}
};

class DeathBehavior : public MoveBehavior
{
public:
	void OnStart(MoveContext& context) override
	{
		ragdolled = false;
		TryRagdoll(context);
	}

	void OnTick(MoveContext& context) override
	{
		context.Character.RequestVelocity(glm::vec3(0.0f), false);
		TryRagdoll(context);
	}

	bool Sustains(const MoveContext& context) const override
	{
		(void)context;
		return true;   // the dead body keeps its final pose
	}

private:
	bool ragdolled = false;

	void TryRagdoll(MoveContext& context)
	{
		const DeathParams& params = context.GetParams<DeathParams>();
		if (ragdolled || !params.ragdoll || context.Time < params.ragdollDelay)
			return;
		ragdolled = true;

		// Creates physics bodies: main thread.
		CombatCharacter& character = context.Character;
		character.RunOnMainThread([&character]()
		{
			if (SkeletalMesh* mesh = character.GetMesh())
				mesh->StartRagdoll();
		});
	}
};

REGISTER_MOVE_BEHAVIOR("Death", DeathBehavior, DeathParams)
