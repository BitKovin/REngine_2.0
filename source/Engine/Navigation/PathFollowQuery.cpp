#include "PathFollowQuery.h"

#include "../EngineMain.h"

#include "Navigation.hpp"
#include "../DebugDraw.hpp"

#include "../Physics.h"

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

	if (Async)
	{
		EngineMain::MainInstance->MainThreadPool->QueueJob([this]() {this->CalculatePathOnThread(); });
	}
	else
	{
		CalculatePathOnThread();
	}

	if (ThreadPool::Supported() == false)
	{
		isPerformingDelay.AddDelay(distance(desiredStart, desiredTarget) / 300.0f + 0.03);
	}
	else
	{
		isPerformingDelay.AddDelay(distance(desiredStart, desiredTarget) / 500.0f + 0.02);
	}

}

void PathFollowQuery::CalculatePathOnThread()
{

	targetLocationsMutex.lock();

	vec3 s, t;


	s = desiredStart;
	t = desiredTarget;


	if (Canceled)
	{
		Performing = false;

		Logger::Log("I have HUGE doubt that it will ever trigger, so I print text to see if it ever happens. \n");

		targetLocationsMutex.unlock();
		return;
	}

	auto path = NavigationSystem::FindSimplePath(s, t, acceptanceRadius, &reachedTarget);

	if (path.size())
	{

		if (path.size() > 1)
		{

			const vec3 firstElem = path[0];

			if (distance2(s, firstElem) < 9) //if distance is less then 3m
			{
				if (Physics::SphereTrace(s, firstElem + vec3(0, 1, 0), 0.3f, BodyType::World).hasHit == false)
				{
					path.erase(path.begin());
				}
			}


		}

		CalculatedTargetLocation = path[0];
		FoundTarget = true;

		//DebugDraw::Path(path, 0.01f);

	}
	
	Performing = false;

	targetLocationsMutex.unlock();
	
}

void PathFollowQuery::UpdateStartAndTarget(vec3 start, vec3 target)
{
	targetLocationsMutex.lock();
	desiredStart = start;
	desiredTarget = target;
	targetLocationsMutex.unlock();
}
