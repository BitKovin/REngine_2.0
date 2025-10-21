#include "CrowdAgent.h"
#include "../glm.h"
#include <cmath>

#include "Navigation.hpp"



CrowdAgent::~CrowdAgent()
{
    if (crowd && agentIndex >= 0)
        crowd->removeAgent(agentIndex);


}

bool CrowdAgent::Init(glm::vec3 position, float height, float radius, float speed)
{

    if (crowd || agentIndex >= 0)
    {
        return false; // Already initialized
    }

    crowd = NavigationSystem::g_crowd;
    if (!crowd)
    {
        return false;
    }

    dtCrowdAgentParams params{};
    params.radius = radius;
    params.height = height;
    params.maxAcceleration = 8.0f;
    params.maxSpeed = speed;
    params.collisionQueryRange = params.radius * 12.0f;
    params.pathOptimizationRange = params.radius * 30.0f;
    params.separationWeight = 2.0f;
    params.updateFlags =
        //DT_CROWD_ANTICIPATE_TURNS |
        //DT_CROWD_OPTIMIZE_VIS |
        //DT_CROWD_OPTIMIZE_TOPO |
        //DT_CROWD_OBSTACLE_AVOIDANCE |
        //DT_CROWD_SEPARATION;
    params.obstacleAvoidanceType = 0;
    params.queryFilterType = 0;
    params.userData = nullptr;

    acceptanceRadius = 0.1f;



    const dtNavMeshQuery* crowdQuery = crowd->getNavMeshQuery();
    if (!crowdQuery)
    {
        return false;
    }

    float p[3] = { position.x, position.y, position.z };
    dtPolyRef ref;
    float closest[3];

    findNearestPointOnNavmesh(position, ref, closest);

    agentIndex = crowd->addAgent(closest, &params);
    if (agentIndex < 0)
    {
        return false;
    }

    return true;
}

glm::vec3 CrowdAgent::GetPosition() const
{
    if (!crowd || agentIndex < 0)
        return glm::vec3(0.0f);

    const dtCrowdAgent* a = crowd->getAgent(agentIndex);
    if (!a)
        return glm::vec3(0.0f);

    return glm::vec3(a->npos[0], a->npos[1], a->npos[2]);
}

void CrowdAgent::SetPosition(const glm::vec3& pos)
{
    if (!crowd || agentIndex < 0)
        return;

    dtCrowdAgent* a = crowd->getEditableAgent(agentIndex);
    if (!a)
        return;

    a->npos[0] = pos.x;
    a->npos[1] = pos.y;
    a->npos[2] = pos.z;
}

glm::vec3 CrowdAgent::GetVelocity() const
{
    if (!crowd) return glm::vec3(0.0f);
    const dtCrowdAgent* ag = crowd->getAgent(agentIndex);
    if (!ag) return glm::vec3(0.0f);
    return glm::vec3(ag->vel[0], ag->vel[1], ag->vel[2]);
}

void CrowdAgent::SetVelocity(const glm::vec3& vel)
{
    if (!crowd) return;
    dtCrowdAgent* ag = const_cast<dtCrowdAgent*>(crowd->getAgent(agentIndex));
    if (!ag) return;

    ag->vel[0] = vel.x;
    ag->vel[1] = vel.y;
    ag->vel[2] = vel.z;
}

float CrowdAgent::GetSpeed() const
{
    if (!crowd || agentIndex < 0)
        return 0.0f;

    const dtCrowdAgent* ag = crowd->getAgent(agentIndex);
    if (!ag)
        return 0.0f;

    return ag->params.maxSpeed;
}

void CrowdAgent::SetSpeed(float speed)
{
    if (!crowd || agentIndex < 0)
        return;

    dtCrowdAgent* ag = crowd->getEditableAgent(agentIndex);
    if (!ag)
        return;

    ag->params.maxSpeed = speed;
}

void CrowdAgent::SetTarget(const glm::vec3& target)
{
    if (!crowd || agentIndex < 0)
        return;

    targetPos = target;
    hasTarget = true;
    succeededAtTarget = false;
    failedToReachTarget = false;

    dtPolyRef ref;
    float closest[3];

    if (findNearestPointOnNavmesh(target, ref, closest))
    {
        dtStatus status = crowd->requestMoveTarget(agentIndex, ref, closest);
        if (dtStatusFailed(status))
        {
            failedToReachTarget = true;
            hasTarget = false;
        }
    }
    else
    {
        failedToReachTarget = true;
        hasTarget = false;
    }



}

bool CrowdAgent::findNearestPointOnNavmesh(const glm::vec3& pos,
                                                 dtPolyRef& ref,
                                                 float closest[3]) const
{


    auto navQuery = dtAllocNavMeshQuery();
    if (dtStatusFailed(navQuery->init(NavigationSystem::navMesh, 2048)))
    {
        dtFreeNavMeshQuery(navQuery);
        navQuery = nullptr;
        return false;
    }

    float p[3] = { pos.x, pos.y, pos.z };
    const float ext[3] = { 2.0f, 4.0f, 2.0f }; // search extents

    dtQueryFilter filter;

    dtStatus s = navQuery->findNearestPoly(p, ext, &filter, &ref, closest);

    dtFreeNavMeshQuery(navQuery);

    return dtStatusSucceed(s) && ref != 0;
}

void CrowdAgent::UpdateStatus()
{

    if (!hasTarget || !crowd || agentIndex < 0)
        return;

    const dtCrowdAgent* ag = crowd->getAgent(agentIndex);
    if (!ag)
        return;

    if (ag->targetState == DT_CROWDAGENT_TARGET_FAILED)
    {
        failedToReachTarget = true;
        hasTarget = false;
        crowd->resetMoveTarget(agentIndex);
        return;
    }

    if (IsAtTargetByProjection())
    {
        succeededAtTarget = true;
        hasTarget = false;
        crowd->resetMoveTarget(agentIndex);
        return;
    }

}

bool CrowdAgent::IsAtTargetByProjection() const
{
    if (!hasTarget)
        return false;

    if (distance(GetPosition(), targetPos) > 3 + acceptanceRadius) return false;

    dtPolyRef aRef, tRef;
    float agentClosest[3], targetClosest[3];

    glm::vec3 agentWorld = GetPosition();
    if (!findNearestPointOnNavmesh(agentWorld, aRef, agentClosest))
        return false;

    if (!findNearestPointOnNavmesh(targetPos, tRef, targetClosest))
        return false;

    float dx = agentClosest[0] - targetClosest[0];
    float dy = agentClosest[1] - targetClosest[1];
    float dz = agentClosest[2] - targetClosest[2];
    float dist = std::sqrt(dx * dx + dy * dy + dz * dz);

    return dist <= acceptanceRadius;
}

bool CrowdAgent::CheckReachedByProjection()
{
    if (!hasTarget)
        return false;

    if (IsAtTargetByProjection())
    {

        if (crowd && agentIndex >= 0)
            crowd->resetMoveTarget(agentIndex);

        return true;
    }

    return false;
}