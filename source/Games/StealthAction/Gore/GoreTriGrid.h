#pragma once

#include <glm.h>
#include <skinned_model.hpp>

#include <cstdint>
#include <vector>

namespace Gore
{
    struct Profile;

    struct GridTri
    {
        glm::vec3 a, b, c;           // bind-space corners
        glm::vec3 na, nb, nc;        // the model's normals at the corners
        uint16_t mesh = 0;           // model->meshes index (which closed surface this face belongs to)
        // The bone weights of the three corners, copied at build time: a worker thread that reads them never touches the model, which
        // the asset system may release or replace at any moment.
        float bone[3][4] = {};
        glm::vec4 weight[3] = { glm::vec4(0), glm::vec4(0), glm::vec4(0) };
    };

    // The nearest point of the body to a point (TriGrid::Closest).
    struct BodyQuery
    {
        bool found = false;
        float signedDistance = 0.0f;   // metres from the nearest surface point: positive inside the body, negative outside
        uint32_t tri = 0;              // index into TriGrid::Tri()
        glm::vec3 bary = glm::vec3(0); // of the nearest point in that triangle
    };

    // Uniform grid over the submeshes of a model, in bind space. Ray casts (Cast) use a stamp array: game thread or one worker at
    // a time. Closest never writes: any number of threads may call it once the grid is built.
    class TriGrid
    {
    public:
        enum class Mode : uint8_t
        {
            Volume,   // the Volume submeshes: the body proper (flesh thickness, limb radius)
            Body,     // everything that encloses the body: the Volume submeshes and every other submesh that is a closed surface
                      // (a jacket or trousers on a body that has no flesh mesh under its clothes; hair is open and stays out)
        };

        void Build(const roj::SkinnedModel& model, const Profile& profile, Mode mode);

        bool Empty() const { return tris.empty(); }
        size_t Size() const { return tris.size(); }
        const GridTri& Tri(uint32_t i) const { return tris[i]; }

        // Distance to the nearest face along the ray (any side), -1 when nothing is hit within tMax.
        float Cast(const glm::vec3& o, const glm::vec3& d, float tMax);

        // The nearest face within maxDist of p, and whether p lies inside the body (behind the nearest face's normal).
        BodyQuery Closest(const glm::vec3& p, float maxDist) const;

    private:
        glm::ivec3 CellOf(const glm::vec3& p) const;
        size_t Index(int x, int y, int z) const { return (static_cast<size_t>(z) * dims.y + y) * dims.x + x; }

        std::vector<GridTri> tris;
        glm::vec3 origin = glm::vec3(0);
        float cell = 0.06f;
        glm::ivec3 dims = glm::ivec3(1);
        std::vector<uint32_t> cellStart, cellTris, stamp;
        uint32_t stampId = 0;
    };
}
