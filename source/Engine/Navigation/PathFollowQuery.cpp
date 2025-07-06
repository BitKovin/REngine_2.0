#include "PathFollowQuery.h"

#include "../EngineMain.h"

#include "Navigation.hpp"
#include "../DebugDraw.hpp"

PathFollowQuery::PathFollowQuery()
{
}

PathFollowQuery::~PathFollowQuery()//since it's very rate that enemy will die and just get destroyed at the same time
{

	Canceled = true;

	while (Performing)
	{

	}
}

void PathFollowQuery::TryPerform()
{

	if (Performing) return;


	if (isPerformingDelay.Wait()) return;


	if (Canceled) return;

	Performing = true;

	EngineMain::MainInstance->MainThreadPool->QueueJob([this]() {this->CalculatePathOnThread(); });

	if (ThreadPool::Supported() == false)
	{
		isPerformingDelay.AddDelay(distance(desiredStart, desiredTarget)/300.0f+0.03);
	}
	else
	{
		isPerformingDelay.AddDelay(distance(desiredStart, desiredTarget) / 500.0f + 0.02);
	}

}

void PathFollowQuery::CalculatePathOnThread()
{

	vec3 s, t;


	s = desiredStart;
	t = desiredTarget;


	if (Canceled)
	{
		Performing = false;

		printf("I have doubt that it will ever trigger, so I print text to see if it ever happens. \n");

		return;
	}

	auto path = NavigationSystem::FindSimplePath(s, t);
	if (path.size())
	{
		CalculatedTargetLocation = path[0];
		FoundTarget = true;

		//DebugDraw::Path(path, 0.01f);

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
