#include "PathFollowQuery.h"

#include "../EngineMain.h"

#include "Navigation.hpp"

PathFollowQuery::PathFollowQuery()
{
}

PathFollowQuery::~PathFollowQuery()
{
}

void PathFollowQuery::TryPerform()
{
	if (Performing) return;

	Performing = true;

	EngineMain::MainInstance->MainThreadPool.QueueJob([this]() {this->CalculatePathOnThread(); });

}

void PathFollowQuery::CalculatePathOnThread()
{

	vec3 s, t;

	targetLocationsMutex.lock();
	s = desiredStart;
	t = desiredTarget;
	targetLocationsMutex.unlock();

	auto path = NavigationSystem::FindSimplePath(s, t);
	if (path.size())
	{
		CalculatedTargetLocation = path[0];
		FoundTarget = true;
	}
	
	Performing = false;
	
}

void PathFollowQuery::UpdateStartAndTarget(vec3 start, vec3 target)
{
	targetLocationsMutex.lock();
	desiredStart = start;
	desiredTarget = target;
	targetLocationsMutex.unlock();
}
