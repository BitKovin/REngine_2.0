#include "SkillRaiseAlarm.h"

#include "../NpcBase.h"
#include "../Director/NpcDirector.h"

#include <cstdio>

float RaiseAlarmSkill::Want(const NpcContext& context) const
{
	NpcOrder order;
	std::string subject;

	if (NpcDirector::FindOrder(context.npc.Id, SituationKind::Search, order, subject) == false) return 0.0f;

	// The bell is the job, even in sight of the intruder: the others take care of that.
	return order.role == NpcRole::AlarmRunner && order.hasDestination ? 60.0f : 0.0f;
}

void RaiseAlarmSkill::Update(NpcContext& context)
{
	NpcOrder order;
	std::string subject;
	if (NpcDirector::FindOrder(context.npc.Id, SituationKind::Search, order, subject) == false) return;

	const NpcAlarmTuning& alarm = NpcSituationTuning::Get().alarm;
	const NpcPursuitTuning& pursuit = context.profile.pursuit;
	NpcBase& npc = context.npc;
	BodyIntent& intent = context.intent;

	intent.weapon = NpcWeaponState::Ready;
	intent.hasGaze = true;
	intent.gazePoint = order.look;

	const float distance = glm::length(MathHelper::XZ(npc.Position - order.destination));

	if (ringing == false && distance > alarm.arriveRadius)
	{
		intent.move = true;
		intent.destination = order.destination;
		intent.acceptRadius = alarm.arriveRadius * 0.8f;
		intent.gait = NpcGait::Run;
		intent.speed = pursuit.chaseSpeed;
		return;
	}

	if (ringing == false)
	{
		ringing = true;
		ringStartedAt = context.now;
		context.mind.Bark("alarm_raise");
	}

	intent.facing = BodyFacing::Point;
	intent.facingPoint = order.look;

	if (rung || context.now - ringStartedAt < alarm.ringTime) return;

	// Ringing: the director rings the bell and tells the area what the runner knows of the intruder (roughly).
	rung = true;

	if (const NpcRecord* known = context.mind.memory.Find(subject, NpcRecordKind::Person))
		context.mind.RequestAlarm(order.look, *known);
}

json RaiseAlarmSkill::Serialize(double now) const
{
	json data;
	data["ringing"] = ringing;
	data["rung"] = rung;
	data["ringAge"] = now - ringStartedAt;
	return data;
}

void RaiseAlarmSkill::Deserialize(const json& data, double now)
{
	ringing = data.value("ringing", false);
	rung = data.value("rung", false);
	ringStartedAt = now - data.value("ringAge", 0.0);
}

std::string RaiseAlarmSkill::Describe(double now) const
{
	char text[96];
	snprintf(text, sizeof(text), "%s", rung ? "rang the bell" : (ringing ? "ringing the bell" : "running to the bell"));
	return text;
}
