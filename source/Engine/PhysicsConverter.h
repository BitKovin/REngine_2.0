#pragma once

#include <Jolt/Jolt.h>
#include "glm.h"

inline glm::vec3 FromPhysics(JPH::Vec3 v)
{
	return vec3(v.GetX(), v.GetY(), v.GetZ());
}

inline JPH::Vec3 ToPhysics(glm::vec3 v)
{
	return JPH::Vec3(v.x, v.y, v.z);
}

inline glm::quat FromPhysics(JPH::Quat q)
{
	return quat(q.GetW(), q.GetX(), q.GetY(), q.GetZ());
}

inline JPH::Quat ToPhysics(glm::quat q)
{
	return JPH::Quat(q.x, q.y, q.z, q.w);
}

inline JPH::Mat44 ToPhysics(glm::mat4 m)
{
    using JPH::Vec4;
    using JPH::Mat44;

    // GLM is column-major by default: m[col][row]
    return Mat44(
        Vec4(m[0][0], m[1][0], m[2][0], m[3][0]), // column 0
        Vec4(m[0][1], m[1][1], m[2][1], m[3][1]), // column 1
        Vec4(m[0][2], m[1][2], m[2][2], m[3][2]), // column 2
        Vec4(m[0][3], m[1][3], m[2][3], m[3][3])  // column 3 (translation)
    );
}

inline glm::mat4 FromPhysics(const JPH::Mat44& m)
{
    glm::mat4 out;
    out[0][0] = m(0, 0); out[0][1] = m(0, 1); out[0][2] = m(0, 2); out[0][3] = m(0, 3);
    out[1][0] = m(1, 0); out[1][1] = m(1, 1); out[1][2] = m(1, 2); out[1][3] = m(1, 3);
    out[2][0] = m(2, 0); out[2][1] = m(2, 1); out[2][2] = m(2, 2); out[2][3] = m(2, 3);
    out[3][0] = m(3, 0); out[3][1] = m(3, 1); out[3][2] = m(3, 2); out[3][3] = m(3, 3);
    return out;
}
