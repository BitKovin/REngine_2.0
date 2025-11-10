#pragma once

#include <string>
#include <glm.h>
#include <Helpers/JsonHelper.hpp>

struct TaskState
{
    std::string TaskName = "";

    bool AllowWeapon = true;

    bool HasToMoveToTarget = false;
    vec3 TargetLocation = vec3();
    float AcceptanceRadius = 0.05f;

    bool CanBeCanceled = true;

    bool DoingJob = false;

    bool HasToLookAtTarget = false;
    vec3 TargetOrientation = vec3();

    std::string TaskStage = "";

    float Timer1 = 0; // user data
    float Timer2 = 0; // user data
    float Timer3 = 0; // user data

    std::string UserData = ""; // user data

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(
        TaskState,
        AllowWeapon,
        TaskName,
        HasToMoveToTarget,
        TargetLocation,
        AcceptanceRadius,
        CanBeCanceled,
        HasToLookAtTarget,
        TargetOrientation,
        TaskStage,
        Timer1,
        Timer2,
        Timer3,
        UserData
    )
};
