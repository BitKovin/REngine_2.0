#pragma once

#include <glm.h>

namespace Gore
{
    // Queues `count` chunks flying from `point` roughly along `direction` (world space). speed in m/s, size = radius of
    // a typical chunk in metres. Safe to call from anywhere on the game thread; nothing is created until ChunkPool::Tick.
    void SpawnChunks(const glm::vec3& point, const glm::vec3& direction, int count, float speed, float size);

    namespace ChunkPool
    {
        void Tick();   // GameStart::Update, next to GibPool::Tick(): creates the system entity on first use, spawns the queue
    }
}
