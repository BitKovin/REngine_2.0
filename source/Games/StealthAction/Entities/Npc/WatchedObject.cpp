#include "WatchedObject.h"

#include <AiPerception/AiPerceptionSystem.h>
#include <Entity.h>
#include <Time.hpp>

#include "Brain/NpcData.h"

WatchedObject::~WatchedObject()
{
	Stop();
}

void WatchedObject::Start(const Entity& owner)
{
	Stop();

	target = AiPerceptionSystem::CreateTarget(owner.Position, owner.Id, { "object" });
	target->moving = false;
	target->motion = TargetMotion::Still;

	shown.clear();
	wanted.clear();
}

void WatchedObject::Stop()
{
	AiPerceptionSystem::RemoveTarget(target);
	target = nullptr;
}

void WatchedObject::Show(const std::string& state)
{
	if (shown.empty() == false)
		target->tags.erase(hashed_string("state:" + shown));

	shown = state;
	target->tags.insert(hashed_string("state:" + shown));
}

void WatchedObject::Update(const glm::vec3& sightPoint, const std::string& state, bool settle)
{
	if (target == nullptr) return;

	target->position = sightPoint;

	const double now = Time::GameTime;

	if (state != wanted)
	{
		wanted = state;
		wantedSince = now;
	}

	if (wanted == shown) return;

	if (shown.empty() || settle == false || now - wantedSince >= NpcPerceptionTuning::Get().objects.settleTime)
		Show(wanted);
}
