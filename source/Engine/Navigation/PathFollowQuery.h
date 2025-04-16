#pragma once

#include <mutex>

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

	vec3 CalculatedTargetLocation = vec3(0);

	bool FoundTarget = false;



private:

	bool Canceled = false;

	std::recursive_mutex targetLocationsMutex;

	vec3 desiredStart;
	vec3 desiredTarget;

};

