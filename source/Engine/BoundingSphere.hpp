#pragma once

#include "glm.h"
#include <vector>
#include <cmath>

#include "MathHelper.hpp"

#include "VertexData.h"

using namespace std;

class BoundingSphere
{
public:
    BoundingSphere() : offset(vec3(0.0f)), Radius(0.0f) {}
    BoundingSphere(vec3 pos, float radius)
    {
        offset = pos;
        Radius = radius;
    }
    ~BoundingSphere() {}

    vec3 offset = vec3(); // center of the sphere
    float Radius = 0; // sphere radius

    static BoundingSphere FromVertices(const vector<VertexData>& vertices)
    {
        vector<vec3> points;

        points.resize(vertices.size());

        for (VertexData vertex : vertices)
        {
            points.push_back(vertex.Position);
        }

        return FromPoints(points);

    }

    static BoundingSphere FromPoints(const vector<vec4>& pointsV4)
    {
        vector<vec3> points;

        for (auto v : pointsV4)
        {
            points.push_back(vec3(v.r, v.g, v.b));
        }

        return FromPoints(points);
    }


    // Computes a bounding sphere that encloses all given points.
    // Uses a variant of Ritter's algorithm.
    static BoundingSphere FromPoints(const vector<vec3>& points)
    {
        BoundingSphere sphere;

        if (points.empty())
            return sphere;

        // Step 1: Pick an arbitrary point p
        const vec3& p = points[0];

        // Step 2: Find the point q farthest from p
        size_t indexQ = 0;
        float maxDistSq = 0.0f;
        for (size_t i = 1; i < points.size(); i++)
        {
            float d = glm::length2(points[i] - p);
            if (d > maxDistSq)
            {
                maxDistSq = d;
                indexQ = i;
            }
        }
        const vec3& q = points[indexQ];

        // Step 3: Find the point r farthest from q
        size_t indexR = indexQ;
        maxDistSq = 0.0f;
        for (size_t i = 0; i < points.size(); i++)
        {
            float d = glm::length2(points[i] - q);
            if (d > maxDistSq)
            {
                maxDistSq = d;
                indexR = i;
            }
        }
        const vec3& r = points[indexR];

        // Step 4: Set the initial sphere to have its center at the midpoint of q and r, 
        // with radius half the distance between them.
        vec3 center = (q + r) * 0.5f;
        float radius = glm::sqrt(maxDistSq) * 0.5f;

        // Step 5: Expand the sphere to include all points.
        for (const vec3& pt : points)
        {
            vec3 diff = pt - center;
            float dist = glm::length(diff);
            if (dist > radius)
            {
                float newRadius = (radius + dist) * 0.5f;
                // Compute how much to move the center toward the point.
                float k = (newRadius - radius) / dist;
                center += diff * k;
                radius = newRadius;
            }
        }

        sphere.offset = center;
        sphere.Radius = radius;
        return sphere;
    }

    BoundingSphere Transform(vec3 translation,vec3 rotation = vec3(0), vec3 scale = vec3(1))
    {

        float s = glm::max(glm::max(scale.x, scale.y), scale.z);

        return BoundingSphere(MathHelper::RotateVector(offset, rotation) * s + translation, Radius * s);

    }

    BoundingSphere Transform(const glm::mat4& matrix)
    {
        // Transform the center (offset) as a point (w=1)
        glm::vec3 transformedCenter = glm::vec3(matrix * glm::vec4(offset, 1.0f));

        // Extract the scale from the matrix using column vectors of the upper-left 3x3 part
        float scaleX = glm::length(glm::vec3(matrix[0])); // X-axis
        float scaleY = glm::length(glm::vec3(matrix[1])); // Y-axis
        float scaleZ = glm::length(glm::vec3(matrix[2])); // Z-axis

        float maxScale = glm::max(scaleX, glm::max(scaleY, scaleZ));

        return BoundingSphere(transformedCenter, Radius * maxScale);
    }




private:

};
