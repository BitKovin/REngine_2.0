#include "SkillTendBody.h"

#include "../NpcBase.h"
#include "../Director/NpcDirector.h"

#include <cstdio>

namespace
{
	// The order of this NPC in the search of a body, when it is the one that checks the place.
	bool TendsBody(const NpcContext& context, NpcOrder& order, std::string& subject)
	{
		NpcDirector::Membership membership;
		const NpcDirector* director = NpcDirector::Get();

		if (director == nullptr || director->FindMembership(context.npc.Id, SituationKind::Search, membership) == false) return false;
		if (membership.order.role != NpcRole::Checker) return false;

		const SearchSituation* search = dynamic_cast<const SearchSituation*>(membership.situation);
		if (search == nullptr || search->BodySubject() == false) return false;

		order = membership.order;
		subject = membership.situation->Subject();
		return true;
	}
}

float TendBodySkill::Want(const NpcContext& context) const
{
	NpcOrder order;
	std::string subject;

	if (TendsBody(context, order, subject) == false) return 0.0f;

	// In sight of a hostile the NPC fights instead.
	return context.mind.SeesHostile() ? 0.0f : 52.0f;
}

void TendBodySkill::Update(NpcContext& context)
{
	NpcOrder order;
	std::string subject;
	if (TendsBody(context, order, subject) == false) return;

	const NpcTendTuning& tuning = context.profile.tend;
	NpcBase& npc = context.npc;
	BodyIntent& intent = context.intent;

	// Where it lies and what state it is in, as this NPC knows it.
	const NpcRecord* record = context.mind.memory.Find(subject, NpcRecordKind::Body);
	const vec3 place = record ? record->position : order.look;
	const bool unconscious = record != nullptr && NpcBodyInfo::StateOfDetail(record->detail) == NpcBodyState::Unconscious;

	intent.weapon = NpcWeaponState::Held;
	intent.hasGaze = true;
	intent.gazePoint = place;

	const float distance = glm::length(MathHelper::XZ(npc.Position - place));

	if (phase == Phase::Walk)
	{
		if (distance > tuning.arriveRadius + 0.3f)
		{
			intent.move = true;
			intent.destination = place;
			intent.acceptRadius = tuning.arriveRadius;
			intent.gait = NpcGait::Walk;
			intent.speed = tuning.speed;
			return;
		}

		phase = Phase::Tend;
		tendStartedAt = context.now;

		if (unconscious)
			context.mind.Bark("body_unconscious");
	}

	intent.crouch = phase == Phase::Tend;
	intent.facing = BodyFacing::Point;
	intent.facingPoint = place;

	if (phase != Phase::Tend) return;

	if (context.now - tendStartedAt < (unconscious ? tuning.unconsciousTime : tuning.deadTime)) return;

	phase = Phase::Done;

	if (unconscious)
		context.mind.RequestWake(subject);
	else
		context.mind.RequestAreaEvent(NpcAreaEvent::BodyFound, place, context.now);

	context.mind.Request(NpcRequestType::Progress, SituationKind::Search, subject, NpcProgress::Checked);
}

json TendBodySkill::Serialize(double now) const
{
	json data;
	data["phase"] = static_cast<int>(phase);
	data["tendAge"] = now - tendStartedAt;
	return data;
}

void TendBodySkill::Deserialize(const json& data, double now)
{
	phase = static_cast<Phase>(data.value("phase", 0));
	tendStartedAt = now - data.value("tendAge", 0.0);
}

std::string TendBodySkill::Describe(double now) const
{
	char text[96];
	snprintf(text, sizeof(text), "%s", phase == Phase::Walk ? "walking to the body" : (phase == Phase::Tend ? "kneeling by the body" : "done with the body"));
	return text;
}
