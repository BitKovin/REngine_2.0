#include "SkillChase.h"

#include "../NpcBase.h"
#include "../Director/NpcDirector.h"

#include <cmath>
#include <cstdio>

void ChaseSkill::Reset()
{
	phase = Phase::Run;
	reported = false;
}

float ChaseSkill::Want(const NpcContext& context) const
{
	NpcOrder order;
	std::string subject;

	if (NpcDirector::FindOrder(context.npc.Id, SituationKind::Search, order, subject) == false || order.role != NpcRole::Chaser || order.hasDestination == false)
		return 0.0f;

	// In sight of it the NPC fights instead of running after news.
	return context.mind.SeesHostile() ? 0.0f : 50.0f;
}

void ChaseSkill::Update(NpcContext& context)
{
	NpcOrder order;
	std::string subject;
	if (NpcDirector::FindOrder(context.npc.Id, SituationKind::Search, order, subject) == false) return;

	const NpcPursuitTuning& tuning = context.profile.pursuit;
	NpcBase& npc = context.npc;
	BodyIntent& intent = context.intent;

	intent.weapon = NpcWeaponState::Ready;

	const float distance = glm::length(MathHelper::XZ(npc.Position - order.destination));

	// News that moved the chase on (the flood has ended, a new sighting) sends it running again.
	if (phase == Phase::Look && distance > tuning.arriveRadius * 2.0f)
		Reset();

	if (phase == Phase::Run)
	{
		intent.move = true;
		intent.destination = order.destination;
		intent.acceptRadius = tuning.arriveRadius;
		intent.gait = NpcGait::Run;
		intent.speed = tuning.chaseSpeed;

		if (context.body.Arrived() || distance <= tuning.arriveRadius + 0.3f)
		{
			phase = Phase::Look;
			lookStartedAt = context.now;

			const vec3 forward = MathHelper::XZ(MathHelper::GetForwardVector(npc.mesh->Rotation));
			lookBase = glm::length(forward) > 0.01f ? glm::normalize(forward) : vec3(0.0f, 0.0f, 1.0f);
		}
		return;
	}

	// Nothing here: a look round, then the situation hears about it.
	const float progress = static_cast<float>((context.now - lookStartedAt) / tuning.chaseLookTime);

	intent.hasGaze = true;
	intent.gazePoint = SweepGazePoint(context.body, lookBase, std::sin(progress * glm::two_pi<float>()) * tuning.chaseLookYaw);

	if (progress >= 1.0f && reported == false)
	{
		reported = true;
		context.mind.Request(NpcRequestType::Progress, SituationKind::Search, subject, NpcProgress::ChaseArrived);
		context.mind.Bark("give_up");
	}
}

json ChaseSkill::Serialize(double now) const
{
	json data;
	data["phase"] = static_cast<int>(phase);
	data["lookAge"] = now - lookStartedAt;
	data["reported"] = reported;
	data["lookBase"] = lookBase;
	return data;
}

void ChaseSkill::Deserialize(const json& data, double now)
{
	phase = static_cast<Phase>(data.value("phase", 0));
	lookStartedAt = now - data.value("lookAge", 0.0);
	reported = data.value("reported", false);
	lookBase = data.value("lookBase", vec3(0.0f, 0.0f, 1.0f));
}

std::string ChaseSkill::Describe(double now) const
{
	char text[96];
	snprintf(text, sizeof(text), "%s", phase == Phase::Run ? "running to the end of the chase" : "looking round at the end of the chase");
	return text;
}
