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


    static PositionVerticesIndices MergeCoplanarRegions(const PositionVerticesIndices& mesh, float normalTolerance = 0.99f)
    {
        PositionVerticesIndices result;
        result.vertices = mesh.vertices;
        size_t triCount = mesh.indices.size() / 3;

        // Build edge->tris map
        unordered_map<Edge, vector<uint32_t>, EdgeHash, EdgeEq> edgeMap;
        edgeMap.reserve(triCount * 3);
        for (uint32_t t = 0; t < triCount; ++t) {
            auto v0 = mesh.indices[3 * t], v1 = mesh.indices[3 * t + 1], v2 = mesh.indices[3 * t + 2];
            edgeMap[Edge(v0, v1)].push_back(t);
            edgeMap[Edge(v1, v2)].push_back(t);
            edgeMap[Edge(v2, v0)].push_back(t);
        }

        vector<bool> used(triCount, false);
        auto getNormal = [&](uint32_t t) { vec3 A = mesh.vertices[mesh.indices[3 * t]]; vec3 B = mesh.vertices[mesh.indices[3 * t + 1]]; vec3 C = mesh.vertices[mesh.indices[3 * t + 2]]; return normalize(cross(B - A, C - A)); };

        for (uint32_t seed = 0; seed < triCount; ++seed)
        {
            if (used[seed]) continue;
            // only merge if surface is near-horizontal
            vec3 baseN = getNormal(seed);
            if (fabs(baseN.y) < 0.8) continue;  // skip non-horizontal regions

            // flood-fill coplanar region
            vector<uint32_t> region;
            queue<uint32_t> q;
            used[seed] = true; q.push(seed);
            while (!q.empty()) {
                uint32_t t = q.front(); q.pop(); region.push_back(t);
                auto verts = array<uint32_t, 3>{ mesh.indices[3 * t], mesh.indices[3 * t + 1], mesh.indices[3 * t + 2] };
                for (int e = 0; e < 3; ++e) {
                    Edge ed(verts[e], verts[(e + 1) % 3]);
                    for (uint32_t nbr : edgeMap[ed]) {
                        if (!used[nbr] && dot(baseN, getNormal(nbr)) >= normalTolerance) { used[nbr] = true; q.push(nbr); }
                    }
                }
            }

            // Collect boundary vertices set
            unordered_map<Edge, int, EdgeHash, EdgeEq> edgeCount;
            for (uint32_t t : region) {
                auto v0 = mesh.indices[3 * t], v1 = mesh.indices[3 * t + 1], v2 = mesh.indices[3 * t + 2];
                edgeCount[Edge(v0, v1)]++;
                edgeCount[Edge(v1, v2)]++;
                edgeCount[Edge(v2, v0)]++;
            }
            vector<uint32_t> boundaryVerts;
            unordered_set<uint32_t> seen;
            for (auto& kv : edgeCount) { if (kv.second == 1) { if (seen.insert(kv.first.a).second) boundaryVerts.push_back(kv.first.a); if (seen.insert(kv.first.b).second) boundaryVerts.push_back(kv.first.b); } }

            if (boundaryVerts.size() < 3) continue;

            // Project boundary to 2D for hull
            vec3 u = fabs(baseN.x) > fabs(baseN.z) ? normalize(cross(vec3(0, 1, 0), baseN)) : normalize(cross(vec3(1, 0, 0), baseN));
            vec3 v = cross(baseN, u);
            vector<pair<double, double>> pts2D(boundaryVerts.size());
            for (size_t i = 0; i < boundaryVerts.size(); ++i) { auto P = result.vertices[boundaryVerts[i]]; pts2D[i] = { dot(P,u), dot(P,v) }; }

            // Compute convex hull (Monotone chain)
            vector<size_t> idx(boundaryVerts.size()); iota(idx.begin(), idx.end(), 0);
            sort(idx.begin(), idx.end(), [&](size_t a, size_t b) { auto& A = pts2D[a], & B = pts2D[b]; return A.first < B.first || (A.first == B.first && A.second < B.second); });
            vector<size_t> hull;
            // lower
            for (size_t i : idx) { while (hull.size() >= 2) { auto& p1 = pts2D[hull[hull.size() - 2]], & p2 = pts2D[hull.back()], & p3 = pts2D[i]; double cr = (p2.first - p1.first) * (p3.second - p1.second) - (p2.second - p1.second) * (p3.first - p1.first); if (cr <= 0) hull.pop_back(); else break; } hull.push_back(i); }
            // upper
            size_t lowerSize = hull.size();
            for (auto it = idx.rbegin(); it != idx.rend(); ++it) { size_t i = *it; while (hull.size() > lowerSize) { auto& p1 = pts2D[hull[hull.size() - 2]], & p2 = pts2D[hull.back()], & p3 = pts2D[i]; double cr = (p2.first - p1.first) * (p3.second - p1.second) - (p2.second - p1.second) * (p3.first - p1.first); if (cr <= 0) hull.pop_back(); else break; } hull.push_back(i); }
            hull.pop_back(); // last equals first

            // Fan triangulate hull in 3D
            for (size_t i = 1; i + 1 < hull.size(); ++i) { result.indices.push_back(boundaryVerts[hull[0]]); result.indices.push_back(boundaryVerts[hull[i]]); result.indices.push_back(boundaryVerts[hull[i + 1]]); }
        }
        return result;
    }

private:
    // Simple boundary chaining: pick a start and walk matching edges
    static vector<uint32_t> ChainBoundary(const vector<Edge>& edges)
    {
        vector<uint32_t> loop;
        if (edges.empty()) return loop;
        // build multimap: vertex->adjacent vertices
        unordered_multimap<uint32_t, uint32_t> adj;
        for (auto& e : edges)
        {
            adj.insert({ e.a, e.b });
            adj.insert({ e.b, e.a });
        }
        // start at an a
        uint32_t start = edges[0].a;
        loop.push_back(start);
        uint32_t curr = start;
        uint32_t prev = UINT32_MAX;
        do
        {
            auto range = adj.equal_range(curr);
            uint32_t next = UINT32_MAX;
            for (auto it = range.first; it != range.second; ++it)
            {
                if (it->second != prev)
                {
                    next = it->second;
                    break;
                }
            }
            if (next == UINT32_MAX || next == start) break;
            loop.push_back(next);
            prev = curr;
            curr = next;
        } while (true);
        return loop;
    }
};
