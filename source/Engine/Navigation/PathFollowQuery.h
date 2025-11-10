#pragma once

#include <mutex>

#include "../Delay.hpp"

#include "../glm.h"

class PathFollowQuery
{
public:
	PathFollowQuery();
	~PathFollowQuery();

	bool Performing = false;

	void TryPerform();

	void CalculatePathOnThread();

	void UpdateStartAndTarget(vec3 start, vec3 target);

	vec3 CalculatedTargetLocation = vec3();

	bool FoundTarget = false;

	bool reachedTarget = true;

	Delay isPerformingDelay;

	bool Async = true;

	bool CalculatedPath = true;

	float acceptanceRadius = 0.2f;

	void WaitToFinish();

	vec3 desiredStart;
	vec3 desiredTarget;

private:

	bool Canceled = false;

	std::recursive_mutex targetLocationsMutex;



};

