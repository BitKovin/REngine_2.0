#include "SkillSearch.h"

#include "../NpcBase.h"
#include "../Director/NpcDirector.h"

#include <cmath>
#include <cstdio>

namespace
{
	bool IsSearchRole(NpcRole role)
	{
		return role == NpcRole::Checker || role == NpcRole::Searcher || role == NpcRole::Watcher;
	}
}

void SearchSkill::Reset()
{
	role = NpcRole::None;
	standingSince = -1.0;
	reported = false;
}

float SearchSkill::Want(const NpcContext& context) const
{
	NpcOrder order;
	std::string subject;

	if (NpcDirector::FindOrder(context.npc.Id, SituationKind::Search, order, subject) == false || IsSearchRole(order.role) == false)
		return 0.0f;

	// The one who checks the place of a body looks after the body (TendBodySkill).
	if (order.role == NpcRole::Checker)
	{
		const NpcDirector* director = NpcDirector::Get();
		const SearchSituation* search = director ? dynamic_cast<const SearchSituation*>(director->FindSituation(SituationKind::Search, subject)) : nullptr;
		if (search != nullptr && search->BodySubject())
			return 0.0f;
	}

	// In sight of it the NPC fights instead of searching.
	return context.mind.SeesHostile() ? 0.0f : 50.0f;
}

void SearchSkill::Update(NpcContext& context)
{
	NpcOrder order;
	std::string subject;
	if (NpcDirector::FindOrder(context.npc.Id, SituationKind::Search, order, subject) == false) return;

	const NpcPursuitTuning& pursuit = context.profile.pursuit;
	NpcBase& npc = context.npc;
	BodyIntent& intent = context.intent;

	if (order.role != role)
	{
		role = order.role;
		standingSince = -1.0;
		reported = false;
	}

	intent.weapon = NpcWeaponState::Ready;

	bool standing = true;

	if (order.hasDestination)
	{
		const float distance = glm::length(MathHelper::XZ(npc.Position - order.destination));
		const float accept = order.role == NpcRole::Watcher ? 0.6f : pursuit.arriveRadius;

		standing = distance <= accept + 0.3f;

		if (standing == false)
		{
			intent.move = true;
			intent.destination = order.destination;
			intent.acceptRadius = accept;
			intent.gait = NpcGait::Walk;
			intent.speed = pursuit.searchSpeed;
		}
	}

	if (standing)
	{
		if (standingSince < 0.0)
			standingSince = context.now;
	}
	else
	{
		standingSince = -1.0;
	}

	if (order.hasLook == false) return;

	intent.hasGaze = true;
	intent.gazePoint = order.look;

	if (standing == false || order.role == NpcRole::Watcher) return;

	// Standing: the head sweeps a little round the point it looks at, so the NPC is not a statue.
	const vec3 toLook = MathHelper::XZ(order.look - npc.Position);
	if (glm::length(toLook) < 0.1f) return;

	const float period = order.role == NpcRole::Checker ? context.profile.investigate.lookAroundTime : pursuit.searchLookPeriod;
	const float yawLimit = order.role == NpcRole::Checker ? context.profile.investigate.lookAroundYaw : pursuit.searchLookYaw;
	const float progress = static_cast<float>((context.now - standingSince) / period);

	intent.gazePoint = SweepGazePoint(context.body, glm::normalize(toLook), std::sin(progress * glm::two_pi<float>()) * yawLimit);

	// The checker has looked once the sweep has been round.
	if (order.role == NpcRole::Checker && progress >= 1.0f && reported == false)
	{
		reported = true;
		context.mind.Request(NpcRequestType::Progress, SituationKind::Search, subject, NpcProgress::Checked);
	}
}

json SearchSkill::Serialize(double now) const
{
	json data;
	data["role"] = role;
	data["standingAge"] = standingSince < 0.0 ? -1.0 : now - standingSince;
	data["reported"] = reported;
	return data;
}

void SearchSkill::Deserialize(const json& data, double now)
{
	role = data.value("role", NpcRole::None);
	const double age = data.value("standingAge", -1.0);
	standingSince = age < 0.0 ? -1.0 : now - age;
	reported = data.value("reported", false);
}

std::string SearchSkill::Describe(double now) const
{
	char text[96];
	snprintf(text, sizeof(text), "%s%s", NpcRoleName(role), standingSince >= 0.0 ? " (standing)" : " (walking)");
	return text;
}
