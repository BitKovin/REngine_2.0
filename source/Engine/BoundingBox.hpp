#pragma once

#include "glm.h"
#include <vector>
#include <limits>

#include "MathHelper.hpp"
#include "VertexData.h"

using namespace std;

class BoundingBox
{
public:
    vec3 Min = vec3(0);  // minimum corner
    vec3 Max = vec3(0);  // maximum corner

    BoundingBox()
        : Min(std::numeric_limits<float>::max()),
        Max(std::numeric_limits<float>::lowest())
    {
    }

    BoundingBox(const vec3& minCorner, const vec3& maxCorner)
        : Min(minCorner),
        Max(maxCorner)
    {
    }

    ~BoundingBox() = default;

    // Create from a list of vertices
    static BoundingBox FromVertices(const vector<VertexData>& vertices)
    {
        vector<vec3> points;
        points.reserve(vertices.size());
        for (const auto& v : vertices)
        {
            points.push_back(v.Position);
        }
        return FromPoints(points);
    }

    // Create from a list of points
    static BoundingBox FromPoints(const vector<vec3>& points)
    {
        BoundingBox box;
        if (points.empty())
            return BoundingBox(vec3(0.0f), vec3(0.0f));

        // Initialize min and max
        vec3 minC = points[0];
        vec3 maxC = points[0];

        for (const auto& p : points)
        {
            minC.x = glm::min(minC.x, p.x);
            minC.y = glm::min(minC.y, p.y);
            minC.z = glm::min(minC.z, p.z);

            maxC.x = glm::max(maxC.x, p.x);
            maxC.y = glm::max(maxC.y, p.y);
            maxC.z = glm::max(maxC.z, p.z);
        }

        return BoundingBox(minC, maxC);
    }

    // Transform the box by translation, rotation (in radians), and scale
    BoundingBox Transform(const vec3& translation, const vec3& rotation = vec3(0.0f), const vec3& scale = vec3(1.0f)) const
    {
        // Generate all 8 corners of the original box
        vec3 corners[8];
        corners[0] = vec3(Min.x, Min.y, Min.z);
        corners[1] = vec3(Min.x, Min.y, Max.z);
        corners[2] = vec3(Min.x, Max.y, Min.z);
        corners[3] = vec3(Min.x, Max.y, Max.z);
        corners[4] = vec3(Max.x, Min.y, Min.z);
        corners[5] = vec3(Max.x, Min.y, Max.z);
        corners[6] = vec3(Max.x, Max.y, Min.z);
        corners[7] = vec3(Max.x, Max.y, Max.z);

        // Transform each corner and recompute min/max
        vec3 newMin(std::numeric_limits<float>::max());
        vec3 newMax(std::numeric_limits<float>::lowest());

        for (int i = 0; i < 8; ++i)
        {
            // Apply scale
            vec3 p = corners[i] * scale;
            // Apply rotation
            p = MathHelper::RotateVector(p, rotation);
            // Apply translation
            p += translation;

            // Update new bounds
            newMin.x = glm::min(newMin.x, p.x);
            newMin.y = glm::min(newMin.y, p.y);
            newMin.z = glm::min(newMin.z, p.z);

            newMax.x = glm::max(newMax.x, p.x);
            newMax.y = glm::max(newMax.y, p.y);
            newMax.z = glm::max(newMax.z, p.z);
        }

        return BoundingBox(newMin, newMax);
    }

    BoundingBox Transform(const glm::mat4& matrix) const
    {
        // Define the 8 corners of the box
        glm::vec3 corners[8] = {
            {Min.x, Min.y, Min.z},
            {Min.x, Min.y, Max.z},
            {Min.x, Max.y, Min.z},
            {Min.x, Max.y, Max.z},
            {Max.x, Min.y, Min.z},
            {Max.x, Min.y, Max.z},
            {Max.x, Max.y, Min.z},
            {Max.x, Max.y, Max.z}
        };

        BoundingBox result;
        // Initialize result's min and max to extremes
        result.Min = glm::vec3(std::numeric_limits<float>::max());
        result.Max = glm::vec3(std::numeric_limits<float>::lowest());

        // Transform each corner and update result's Min/Max
        for (int i = 0; i < 8; ++i)
        {
            glm::vec4 transformed = matrix * glm::vec4(corners[i], 1.0f);
            glm::vec3 pos = glm::vec3(transformed) / transformed.w;

            result.Min = glm::min(result.Min, pos);
            result.Max = glm::max(result.Max, pos);
        }

        return result;
    }

    // Check intersection with a sphere
    bool Intersects(const vec3 offset, const float radius) const
    {
        float distSq = 0.0f;
        const vec3& c = offset;

        if (c.x < Min.x)
            distSq += (Min.x - c.x) * (Min.x - c.x);
        else if (c.x > Max.x)
            distSq += (c.x - Max.x) * (c.x - Max.x);

        if (c.y < Min.y)
            distSq += (Min.y - c.y) * (Min.y - c.y);
        else if (c.y > Max.y)
            distSq += (c.y - Max.y) * (c.y - Max.y);

        if (c.z < Min.z)
            distSq += (Min.z - c.z) * (Min.z - c.z);
        else if (c.z > Max.z)
            distSq += (c.z - Max.z) * (c.z - Max.z);

        return distSq <= (radius * radius);
    }

    // Check intersection with another bounding box
    bool Intersects(const BoundingBox& other) const
    {
        return (Min.x <= other.Max.x && Max.x >= other.Min.x) &&
            (Min.y <= other.Max.y && Max.y >= other.Min.y) &&
            (Min.z <= other.Max.z && Max.z >= other.Min.z);
    }

    // Get center of the box
    vec3 Center() const
    {
        return (Min + Max) * 0.5f;
    }

    // Get extents (half-size) of the box
    vec3 Extents() const
    {
        return (Max - Min) * 0.5f;
    }
};
