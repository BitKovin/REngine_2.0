#pragma once

#include "glm.h"

class RandomHelper
{
public:

    // Thread-safe (one generator per thread).
    static float RandomFloat();   // [0, 1)
    static int RandomInt();       // [0, INT_MAX]

    static glm::vec3 RandomPosition(float radius);
};

