#include "AlarmPoint.h"

#include <LevelObjectFactory.h>
#include <MapData.h>
#include <Navigation/NavGrid.h>

#include "Brain/NpcData.h"

#include <algorithm>

AlarmPoint::AlarmPoint()
{
	ClassName = "alarm_point";
}

AlarmPoint::~AlarmPoint()
{
	alarms.erase(std::remove(alarms.begin(), alarms.end(), this), alarms.end());
}

void AlarmPoint::FromData(EntityData data)
{
	Entity::FromData(data);

	radius = data.GetPropertyFloat("radius", 0.0f);
}

void AlarmPoint::Start()
{
	Entity::Start();

	if (std::find(alarms.begin(), alarms.end(), this) == alarms.end())
		alarms.push_back(this);
}

float AlarmPoint::Radius() const
{
	return radius > 0.0f ? radius : NpcSituationTuning::Get().messages.alarm.radius;
}

glm::vec3 AlarmPoint::StandingPlace() const
{
	if (NavGrid::IsReady())
	{
		const int cell = NavGrid::FindCell(Position, 3.0f);
		if (cell >= 0)
			return NavGrid::GetCell(cell).floor;
	}

	return Position;
}

REGISTER_ENTITY(AlarmPoint, "alarm_point")
