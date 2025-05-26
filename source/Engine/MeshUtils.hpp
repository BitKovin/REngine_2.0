#pragma once

#include "glm.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <cmath>
#include <numeric>
#include <functional>

#include "VertexData.h"

using namespace std;

class MeshUtils
{
public:
    struct PositionVerticesIndices
    {
        vector<vec3>     vertices;
        vector<uint32_t> indices;
    };

    struct VerticesIndices
    {
        vector<VertexData> vertices;
        vector<uint32_t>   indices;
    };

    /// Merge a bunch of meshes together
    static PositionVerticesIndices MergeMeshes(const vector<PositionVerticesIndices>& meshes)
    {
        PositionVerticesIndices merged;
        uint32_t vertexOffset = 0;
        for (auto const& mesh : meshes)
        {
            merged.vertices.insert(merged.vertices.end(), mesh.vertices.begin(), mesh.vertices.end());
            for (auto idx : mesh.indices)
                merged.indices.push_back(idx + vertexOffset);
            vertexOffset += uint32_t(mesh.vertices.size());
        }
        return merged;
    }

    static VerticesIndices MergeMeshes(const vector<VerticesIndices>& meshes)
    {
        VerticesIndices merged;
        uint32_t vertexOffset = 0;
        for (auto const& mesh : meshes)
        {
            merged.vertices.insert(merged.vertices.end(), mesh.vertices.begin(), mesh.vertices.end());
            for (auto idx : mesh.indices)
                merged.indices.push_back(idx + vertexOffset);
            vertexOffset += uint32_t(mesh.vertices.size());
        }
        return merged;
    }

    // --------------------------------------------------
    // RemoveDegenerates: weld very-close vertices & drop degenerate triangles
    // --------------------------------------------------

    static PositionVerticesIndices RemoveDegenerates(const PositionVerticesIndices& mesh,
        float distanceThreshold,
        float areaEpsilon)
    {
        if (mesh.vertices.empty())
            return PositionVerticesIndices{ {}, {} };

        // --- Merge close vertices via union-find ---
        vector<size_t> parent(mesh.vertices.size());
        iota(parent.begin(), parent.end(), 0);
        function<size_t(size_t)> findRoot = [&](size_t i) -> size_t {
            return parent[i] == i ? i : (parent[i] = findRoot(parent[i]));
            };
        auto unionSets = [&](size_t a, size_t b) {
            size_t ra = findRoot(a), rb = findRoot(b);
            if (ra != rb) parent[ra] = rb;
            };

        // Spatial sort by X
        vector<size_t> sorted(mesh.vertices.size());
        iota(sorted.begin(), sorted.end(), 0);
        sort(sorted.begin(), sorted.end(), [&](size_t a, size_t b) {
            return mesh.vertices[a].x < mesh.vertices[b].x;
            });

        for (size_t i = 0; i < sorted.size(); ++i)
        {
            size_t ai = sorted[i];
            for (size_t j = i + 1; j < sorted.size(); ++j)
            {
                size_t bi = sorted[j];
                if (mesh.vertices[bi].x > mesh.vertices[ai].x + distanceThreshold)
                    break;
                if (length(mesh.vertices[ai] - mesh.vertices[bi]) < distanceThreshold)
                    unionSets(ai, bi);
            }
        }

        // Build new vertex groups
        vector<size_t> root(mesh.vertices.size());
        for (size_t i = 0; i < root.size(); ++i)
            root[i] = findRoot(i);

        unordered_map<size_t, vector<size_t>> groups;
        for (size_t i = 0; i < root.size(); ++i)
            groups[root[i]].push_back(i);

        vector<vec3> newVerts;
        newVerts.reserve(groups.size());
        vector<size_t> oldToNew(mesh.vertices.size());
        for (auto& kv : groups)
        {
            vec3 avg(0.0f);
            for (auto idx : kv.second)
                avg += mesh.vertices[idx];
            avg /= float(kv.second.size());
            size_t newIdx = newVerts.size();
            newVerts.push_back(avg);
            for (auto idx : kv.second)
                oldToNew[idx] = newIdx;
        }

        // Remove tiny/degenerate triangles
        vector<uint32_t> newIdxs;
        for (size_t i = 0; i + 2 < mesh.indices.size(); i += 3)
        {
            size_t i0 = oldToNew[mesh.indices[i]];
            size_t i1 = oldToNew[mesh.indices[i + 1]];
            size_t i2 = oldToNew[mesh.indices[i + 2]];
            if (i0 == i1 || i1 == i2 || i2 == i0)
                continue;
            vec3 A = newVerts[i0];
            vec3 B = newVerts[i1];
            vec3 C = newVerts[i2];
            float area = 0.5f * length(cross(B - A, C - A));
            if (area > areaEpsilon)
            {
                newIdxs.push_back(uint32_t(i0));
                newIdxs.push_back(uint32_t(i1));
                newIdxs.push_back(uint32_t(i2));
            }
        }

        return PositionVerticesIndices{ newVerts, newIdxs };
    }

    static PositionVerticesIndices ExpandHorizontalTriangles(const PositionVerticesIndices& mesh, float radius);

    // --------------------------------------------------
    // MergeCoplanarRegions: flood-fill + re-triangulate planar patches
    // --------------------------------------------------

    struct Edge
    {
        uint32_t a, b;
        Edge(uint32_t x, uint32_t y) noexcept : a(glm::min(x, y)), b(glm::max(x, y)) {}
        bool operator==(Edge const& o) const noexcept { return a == o.a && b == o.b; }
    };


	struct EdgeHash { size_t operator()(Edge const& e) const noexcept { return (uint64_t(e.a) << 32) ^ e.b; } };


    struct EdgeEq   
    {
        bool operator()(Edge const& l, Edge const& r) const noexcept
        {
            return l.a == r.a && l.b == r.b;
        }
    };


    static PositionVerticesIndices MergeCoplanarRegions(const PositionVerticesIndices& mesh, float normalTolerance = 0.99f);



private:
    // Simple boundary chaining: pick a start and walk matching edges
    static vector<uint32_t> ChainBoundary(const vector<Edge>& edges);
};
