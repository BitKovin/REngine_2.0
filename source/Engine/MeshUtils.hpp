#pragma once

#include "glm.h"

#include <vector>
#include <unordered_map>
#include <cmath>

#include "VertexData.h"

using namespace std;

class MeshUtils
{
public:
    struct PositionVerticesIndices
    {
        vector<vec3>    vertices;
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
    // RemoveDegenerates: weld very‐close vertices & drop degenerate triangles
    // --------------------------------------------------

    static PositionVerticesIndices RemoveDegenerates(const PositionVerticesIndices& mesh,
        float distanceThreshold,
        float areaEpsilon) {
        // Handle empty mesh case
        if (mesh.vertices.empty()) {
            return PositionVerticesIndices{ {}, {} };
        }

        // --- Step 1: Merge close vertices ---

        // Initialize union-find parent array
        vector<size_t> parent(mesh.vertices.size());
        iota(parent.begin(), parent.end(), 0); // Each vertex starts as its own root

        // Union-Find helper: Find root with path compression
        std::function<size_t(size_t)> find = [&parent, &find](size_t i) -> size_t {
            if (parent[i] != i) {
                parent[i] = find(parent[i]); // Path compression
            }
            return parent[i];
        };

        // Union-Find helper: Union two sets
        auto unionSets = [&parent, &find](size_t x, size_t y) {
            size_t rootX = find(x);
            size_t rootY = find(y);
            if (rootX != rootY) {
                parent[rootX] = rootY; // Simple union (could use rank for optimization)
            }
            };

        // Sort vertices by x-coordinate for efficient spatial grouping
        vector<size_t> sortedIndices(mesh.vertices.size());
        iota(sortedIndices.begin(), sortedIndices.end(), 0);
        sort(sortedIndices.begin(), sortedIndices.end(),
            [&mesh](size_t a, size_t b) {
                return mesh.vertices[a].x < mesh.vertices[b].x;
            });

        // Group vertices within distanceThreshold
        for (size_t m = 0; m < sortedIndices.size(); ++m) {
            size_t idx = sortedIndices[m];
            float x = mesh.vertices[idx].x;
            size_t n = m + 1;
            // Check subsequent vertices until x-distance exceeds threshold
            while (n < sortedIndices.size() &&
                mesh.vertices[sortedIndices[n]].x < x + distanceThreshold) {
                size_t idx2 = sortedIndices[n];
                // Full 3D distance check
                if (length(mesh.vertices[idx] - mesh.vertices[idx2]) < distanceThreshold) {
                    unionSets(idx, idx2);
                }
                ++n;
            }
        }

        // Compute roots for all vertices
        vector<size_t> roots(mesh.vertices.size());
        for (size_t i = 0; i < mesh.vertices.size(); ++i) {
            roots[i] = find(i);
        }

        // Group vertices by root and compute new vertex positions
        map<size_t, vector<size_t>> rootToVertices;
        for (size_t i = 0; i < mesh.vertices.size(); ++i) {
            rootToVertices[roots[i]].push_back(i);
        }

        vector<vec3> newVertices;
        vector<size_t> oldToNew(mesh.vertices.size());
        for (const auto& pair : rootToVertices) {
            const vector<size_t>& group = pair.second;
            vec3 averagePos(0.0f, 0.0f, 0.0f);
            // Compute average position for the group
            for (size_t j : group) {
                averagePos += mesh.vertices[j];
            }
            averagePos /= static_cast<float>(group.size());
            newVertices.push_back(averagePos);
            size_t newIndex = newVertices.size() - 1;
            // Map all old vertices in this group to the new index
            for (size_t j : group) {
                oldToNew[j] = newIndex;
            }
        }

        // --- Step 2: Remove degenerate triangles ---

        vector<uint32_t> newIndices;
        // Process triangles (assuming indices are in groups of 3)
        for (size_t i = 0; i + 2 < mesh.indices.size(); i += 3) {
            size_t i0 = mesh.indices[i];
            size_t i1 = mesh.indices[i + 1];
            size_t i2 = mesh.indices[i + 2];

            // Map to new vertex indices
            size_t ni0 = oldToNew[i0];
            size_t ni1 = oldToNew[i1];
            size_t ni2 = oldToNew[i2];

            // Check if vertices are distinct
            if (ni0 != ni1 && ni0 != ni2 && ni1 != ni2) {
                vec3 a = newVertices[ni0];
                vec3 b = newVertices[ni1];
                vec3 c = newVertices[ni2];

                // Compute triangle area using cross product
                vec3 ab = b - a;
                vec3 ac = c - a;
                float area = 0.5f * length(cross(ab, ac));

                // Keep triangle if area exceeds epsilon
                if (area > areaEpsilon) {
                    newIndices.push_back(static_cast<uint32_t>(ni0));
                    newIndices.push_back(static_cast<uint32_t>(ni1));
                    newIndices.push_back(static_cast<uint32_t>(ni2));
                }
            }
        }

        // Return the cleaned mesh
        return PositionVerticesIndices{ newVertices, newIndices };
    }
};
