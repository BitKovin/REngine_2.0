#pragma once

// A bell or a siren a guard can run to and ring (NPC_AI_DESIGN.md §17.11). Map entity "alarm_point", keyvalues:
//   radius   the area the alarm reaches, in metres (default: the alarm message of GameData/ai/situations.json)
// The runner is chosen by a search situation; ringing alarms the area (NpcDirector) and tells it roughly where the intruder is.

#include <Entity.h>

#include <vector>

class AlarmPoint : public Entity
{
public:

	AlarmPoint();
	~AlarmPoint();

	void FromData(EntityData data) override;
	void Start() override;

	// Every alarm point of the level.
	static const std::vector<AlarmPoint*>& All() { return alarms; }

	// The area it reaches.
	float Radius() const;

	// Where the runner stands to ring it: the floor under the point on the navigation grid (the point itself when there is none).
	glm::vec3 StandingPlace() const;

private:

	static inline std::vector<AlarmPoint*> alarms;

	float radius = 0.0f;
};
