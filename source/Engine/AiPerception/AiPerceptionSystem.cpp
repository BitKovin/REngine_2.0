#include "AiPerceptionSystem.h"
#include "Observer.h"
#include "ObservationTarget.h"
#include <algorithm>
#include "../EngineMain.h"

std::vector<std::shared_ptr<Observer>> AiPerceptionSystem::observers;
std::vector<std::shared_ptr<ObservationTarget>> AiPerceptionSystem::targets;

std::shared_ptr<Observer> AiPerceptionSystem::CreateObserver(const glm::vec3& position, const glm::vec3& forward, float fovDeg)
{
    auto observer = std::make_shared<Observer>(position, forward, fovDeg);
    observer->id = nextId;
    nextId++;
    observers.push_back(observer);
    return observer;
}

std::shared_ptr<ObservationTarget> AiPerceptionSystem::CreateTarget(const glm::vec3& position, const std::string& ownerId, const std::vector<std::string>& tags)
{
    auto target = std::make_shared<ObservationTarget>(position, tags);
    target->ownerId = ownerId;
    targets.push_back(target);
    return target;
}

void AiPerceptionSystem::RemoveObserver(const std::shared_ptr<Observer>& observer)
{
    observers.erase(std::remove(observers.begin(), observers.end(), observer), observers.end());
}

void AiPerceptionSystem::RemoveTarget(const std::shared_ptr<ObservationTarget>& target)
{
    targets.erase(std::remove(targets.begin(), targets.end(), target), targets.end());
}

void AiPerceptionSystem::RemoveAll()
{
    observers.clear();
    targets.clear();
}

void AiPerceptionSystem::Update()
{

    const int frameDistrib = 4;

    for (auto& observer : observers)
    {

        if (observer->id % frameDistrib == EngineMain::MainInstance->frame % frameDistrib)
        {
            observer->UpdateVisibility(targets);
        }

    }

}

std::vector<std::shared_ptr<Observer>> AiPerceptionSystem::GetObserversInRadius(const glm::vec3& position, float radius)
{
    std::vector<std::shared_ptr<Observer>> result;
    if (observers.empty() || radius <= 0.0f)
        return result;

    float radiusSq = radius * radius;

    for (auto& obs : observers)
    {
        float distSq = glm::length2(obs->position - position);
        if (distSq <= radiusSq)
        {
            result.push_back(obs);
        }
    }

    return result;
}
