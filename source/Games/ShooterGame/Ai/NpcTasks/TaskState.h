#pragma once

#include <string>
#include <glm.h>

struct TaskState
{

	std::string TaskId;

	bool HasToMoveToTarget;
	vec3 TargetLocation;
	float AcceptanceRadius = 0.05f;

	bool HasToLookAtTarget;
	vec3 TargetOrientation;


	std::string TaskStage;

	float Timer1;//user data
	float Timer2;//user data
	float Timer3;//user data

	std::string UserData;//user data

};