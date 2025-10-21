#pragma once
#include <glm/vec3.hpp>
#include "Detour/DetourCrowd.h"
#include "Detour/DetourNavMeshQuery.h"

class CrowdAgent
{
public:

    CrowdAgent(){ }

    ~CrowdAgent();

    bool Init(glm::vec3 position, float height = 1.8f, float radius = 0.5f, float speed = 5);

    // Position
    glm::vec3 GetPosition() const;
    void SetPosition(const glm::vec3& pos);

    glm::vec3 GetVelocity() const;
    void SetVelocity(const glm::vec3& pos);

    float GetSpeed() const;
    void SetSpeed(float speed);

    // Target
    void SetTarget(const glm::vec3& target);
    glm::vec3 GetTarget() const { return targetPos; }
    bool HasTarget() const { return hasTarget; }

    // Acceptance radius
    void SetAcceptanceRadius(float r) { acceptanceRadius = r; }
    float GetAcceptanceRadius() const { return acceptanceRadius; }

    void UpdateStatus();

    // State flags
    bool GetSucceeded() const { return succeededAtTarget; }
    bool GetFailed() const { return failedToReachTarget; }

    // Check if reached target (projects both onto navmesh)
    bool IsAtTargetByProjection() const;
    bool CheckReachedByProjection();

    // Get agent index
    int GetAgentIndex() const { return agentIndex; }

private:
    bool findNearestPointOnNavmesh(const glm::vec3& pos,
        dtPolyRef& ref,
        float closest[3]) const;

private:
    dtCrowd* crowd = nullptr;
    int agentIndex = -1;

    glm::vec3 targetPos = glm::vec3(0.0f);
    bool hasTarget = false;
    float acceptanceRadius = 0.5f;

    bool succeededAtTarget = false;
    bool failedToReachTarget = false;
};
