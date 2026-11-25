#include "SkillInvestigate.h"

#include "../NpcBase.h"
#include "../Director/NpcDirector.h"
#include "../Spatial/PostQuery.h"

#include <Physics.h>

#include <cmath>
#include <cstdio>

namespace
{
	float FlatDistance(const vec3& a, const vec3& b)
	{
		return glm::length(MathHelper::XZ(a - b));
	}

	bool ClearView(const vec3& from, const vec3& to)
	{
		return Physics::LineTrace(from, to, BodyType::World).hasHit == false;
	}

	// A spot `min` to `max` from the point with a view of it, the nearest to `from`: the investigator looks from there
	// instead of standing on the spot. False when the grid is missing or no cell fits (the caller walks to the point).
	bool FindPost(const vec3& point, const vec3& from, float min, float max, vec3& post)
	{
		const std::vector<int> cells = PostQuery::CellsAround(point, max);
		return PostQuery::Find(cells, PostQuery::OpenPost(point, from, min, max, {}), post);
	}

	NpcWeaponState WeaponFor(NpcConcernType type, bool seesHostile)
	{
		if (type == NpcConcernType::Threat)
			return seesHostile ? NpcWeaponState::Aimed : NpcWeaponState::Ready;

		return NpcWeaponState::Held;
	}

	// The part of this NPC in the investigate situation of the subject (none when it has none).
	NpcRole RoleFor(const NpcContext& context, const std::string& subject)
	{
		NpcOrder order;
		std::string orderSubject;

		if (NpcDirector::FindOrder(context.npc.Id, SituationKind::Investigate, order, orderSubject) && orderSubject == subject)
			return order.role;
		return NpcRole::None;
	}
}

float InvestigateSkill::Want(const NpcContext& context) const
{
	const NpcConcern* concern = context.mind.TopConcern();
	if (concern == nullptr) return 0.0f;

	switch (concern->type)
	{
	case NpcConcernType::Threat:
		return 48.0f;

	case NpcConcernType::Curious:
	{
		// Only the ones the situation sent look into it; the rest glanced and carry on.
		const NpcRole role = RoleFor(context, concern->subject);
		return role == NpcRole::Investigator || role == NpcRole::Backup ? 40.0f : 0.0f;
	}

	default:
		return 0.0f; // an alarm is a search (SearchSkill, ChaseSkill)
	}
}

void InvestigateSkill::Begin(NpcContext& context, const NpcConcern& concern)
{
	active = true;
	subject = concern.subject;
	type = concern.type;
	role = RoleFor(context, subject);
	point = context.mind.ConcernPosition(concern);
	plannedPoint = point;
	startedAt = context.now;
	hasPost = false;

	if (type == NpcConcernType::Curious)
		context.mind.Bark("investigate_start");

	EnterPhase(context, Phase::React);
}

void InvestigateSkill::EnterPhase(NpcContext& context, Phase next)
{
	phase = next;
	phaseStartedAt = context.now;

	if (next == Phase::LookAround)
	{
		const vec3 toPoint = MathHelper::XZ(point - context.npc.Position);
		lookBase = glm::length(toPoint) > 0.1f
			? glm::normalize(toPoint)
			: MathHelper::XZ(MathHelper::GetForwardVector(context.npc.mesh->Rotation));
	}
}

// After the reaction: where to go. A hostile is closed on directly; anything else is looked at from a post (the backup's
// is farther off).
void InvestigateSkill::PlanApproach(NpcContext& context)
{
	const NpcInvestigateTuning& tuning = context.profile.investigate;

	plannedPoint = point;
	hasPost = false;

	if (type == NpcConcernType::Threat)
	{
		EnterPhase(context, Phase::Approach);
		return;
	}

	const bool backup = role == NpcRole::Backup;
	const float extra = backup ? tuning.backupExtraDistance : 0.0f;

	// Already standing where a post would be, with a view of it: just look.
	if (FlatDistance(context.npc.Position, point) <= tuning.postMax + extra + 0.5f && ClearView(context.body.EyePosition(), point))
	{
		EnterPhase(context, backup ? Phase::Cover : Phase::LookAround);
		return;
	}

	hasPost = FindPost(point, context.npc.Position, tuning.postMin + extra, tuning.postMax + extra, post);
	EnterPhase(context, Phase::Approach);
}

void InvestigateSkill::Finish(NpcContext& context)
{
	// The investigator tells its situation that the matter is settled; the others' concerns are settled with it.
	if (type == NpcConcernType::Curious && role == NpcRole::Investigator)
		context.mind.Request(NpcRequestType::Progress, SituationKind::Investigate, subject, NpcProgress::Resolved);

	if (type == NpcConcernType::Curious)
		context.mind.Bark("investigate_clear");

	context.mind.Resolve(subject);

	if (context.mind.TopConcern() == nullptr)
		context.mind.FinishSearch(context.now);

	active = false;
}

void InvestigateSkill::Update(NpcContext& context)
{
	const NpcInvestigateTuning& tuning = context.profile.investigate;
	NpcBase& npc = context.npc;
	BodyIntent& intent = context.intent;

	const NpcConcern* top = context.mind.TopConcern();
	if (top == nullptr)
	{
		active = false;
		return;
	}

	if (active == false || top->subject != subject)
	{
		Begin(context, *top);
	}
	else
	{
		type = top->type;
		point = context.mind.ConcernPosition(*top);

		// The part changed (the backup was sent in, or the situation was won): plan again.
		const NpcRole current = RoleFor(context, subject);
		if (current != role)
		{
			role = current;
			if (phase != Phase::React)
				PlanApproach(context);
		}

		// News of somewhere else: plan the approach again (no new reaction).
		if (phase != Phase::React && FlatDistance(point, plannedPoint) > tuning.retargetDistance)
			PlanApproach(context);
	}

	if (context.now - startedAt > tuning.giveUpAfter)
	{
		Finish(context);
		return;
	}

	const bool seesHostile = context.mind.SeesHostile();

	intent.hasGaze = true;
	intent.gazePoint = point;
	intent.weapon = WeaponFor(type, seesHostile);

	switch (phase)
	{
	case Phase::React:
		if (context.now - phaseStartedAt >= tuning.reactionTime)
			PlanApproach(context);
		break;

	case Phase::Approach:
	{
		// A hostile in view: close on it to the hold distance and watch it.
		if (type == NpcConcernType::Threat && seesHostile && FlatDistance(npc.Position, point) <= tuning.holdDistance)
		{
			intent.facing = BodyFacing::Point;
			intent.facingPoint = point;
			break;
		}

		const bool closingOnHostile = type == NpcConcernType::Threat && seesHostile;

		intent.move = true;
		intent.destination = hasPost ? post : point;
		intent.acceptRadius = hasPost ? 0.8f : (closingOnHostile ? tuning.holdDistance : tuning.arriveRadius);
		intent.speed = type == NpcConcernType::Threat ? tuning.threatSpeed : tuning.speed;

		const bool near = FlatDistance(npc.Position, intent.destination) <= intent.acceptRadius + 0.3f;

		if (context.body.Arrived() || near)
			EnterPhase(context, role == NpcRole::Backup ? Phase::Cover : Phase::LookAround);
		break;
	}

	case Phase::LookAround:
	{
		if (type == NpcConcernType::Threat && seesHostile)
		{
			EnterPhase(context, Phase::Approach);
			break;
		}

		const float progress = static_cast<float>((context.now - phaseStartedAt) / tuning.lookAroundTime);
		intent.gazePoint = SweepGazePoint(context.body, lookBase, std::sin(progress * glm::two_pi<float>()) * tuning.lookAroundYaw);

		if (progress >= 1.0f)
			Finish(context);
		break;
	}

	case Phase::Cover:
		// Watches the point from here until the investigator is done (then the situation settles the concern).
		break;
	}
}

json InvestigateSkill::Serialize(double now) const
{
	json data;
	data["active"] = active;
	data["subject"] = subject;
	data["type"] = type;
	data["role"] = role;
	data["point"] = point;
	data["plannedPoint"] = plannedPoint;
	data["phase"] = static_cast<int>(phase);
	data["startedAge"] = now - startedAt;
	data["phaseAge"] = now - phaseStartedAt;
	data["hasPost"] = hasPost;
	data["post"] = post;
	data["lookBase"] = lookBase;
	return data;
}

void InvestigateSkill::Deserialize(const json& data, double now)
{
	active = data.value("active", false);
	subject = data.value("subject", std::string());
	type = data.value("type", NpcConcernType::Curious);
	role = data.value("role", NpcRole::None);
	point = data.value("point", vec3());
	plannedPoint = data.value("plannedPoint", vec3());
	phase = static_cast<Phase>(data.value("phase", 0));
	startedAt = now - data.value("startedAge", 0.0);
	phaseStartedAt = now - data.value("phaseAge", 0.0);
	hasPost = data.value("hasPost", false);
	post = data.value("post", vec3());
	lookBase = data.value("lookBase", vec3(0.0f, 0.0f, 1.0f));
}

std::string InvestigateSkill::Describe(double now) const
{
	if (active == false) return "idle";

	const char* phaseName = phase == Phase::React ? "react" : (phase == Phase::Approach ? "approach" : (phase == Phase::LookAround ? "look around" : "cover"));
	const char* typeName = type == NpcConcernType::Threat ? "threat" : (type == NpcConcernType::Alarm ? "alarm" : "curious");

	char text[160];
	snprintf(text, sizeof(text), "%s of %s (%s, %s), %.1fs%s", phaseName, subject.c_str(), typeName, NpcRoleName(role), now - startedAt, hasPost ? ", from a post" : "");
	return text;
}
