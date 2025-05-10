#include "MeshUtils.hpp"

MeshUtils::PositionVerticesIndices MeshUtils::ExpandHorizontalTriangles(const PositionVerticesIndices& mesh, float radius)
{
    PositionVerticesIndices result;
    size_t triCount = mesh.indices.size() / 3;

    // Accumulate new positions and count contributions per vertex
    std::vector<vec3> accum(mesh.vertices.size(), vec3(0.0f));
    std::vector<int> count(mesh.vertices.size(), 0);

    float startRadius = radius;

    for (size_t t = 0; t < triCount; ++t) {
        // Extract triangle indices and vertices
        uint32_t i0 = mesh.indices[3 * t];
        uint32_t i1 = mesh.indices[3 * t + 1];
        uint32_t i2 = mesh.indices[3 * t + 2];
        vec3 A = mesh.vertices[i0];
        vec3 B = mesh.vertices[i1];
        vec3 C = mesh.vertices[i2];

        // Compute the triangle's plane normal and basis vectors
        vec3 N = normalize(cross(B - A, C - A));
        vec3 U = normalize(B - A);
        vec3 V = cross(N, U);

        if (abs(N.y) > 0.1f)
        {
            radius = 0.0001f;
        }
        else
        {
            radius = startRadius;
        }

        // Project vertices to 2D plane
        auto project = [&](vec3 P) -> std::pair<float, float> {
            return { dot(P - A, U), dot(P - A, V) };
            };
        std::pair<float, float> P0 = project(A);
        std::pair<float, float> P1 = project(B);
        std::pair<float, float> P2 = project(C);

        // Compute outward normals in 2D (90° clockwise rotation)
        auto computeNormal = [](std::pair<float, float> D) -> std::pair<float, float> {
            return { D.second, -D.first };
            };

        std::pair<float, float> D01 = { P1.first - P0.first, P1.second - P0.second };
        std::pair<float, float> N01 = computeNormal(D01);
        float len01 = std::sqrt(N01.first * N01.first + N01.second * N01.second);
        N01.first /= len01; N01.second /= len01;

        std::pair<float, float> D12 = { P2.first - P1.first, P2.second - P1.second };
        std::pair<float, float> N12 = computeNormal(D12);
        float len12 = std::sqrt(N12.first * N12.first + N12.second * N12.second);
        N12.first /= len12; N12.second /= len12;

        std::pair<float, float> D20 = { P0.first - P2.first, P0.second - P2.second };
        std::pair<float, float> N20 = computeNormal(D20);
        float len20 = std::sqrt(N20.first * N20.first + N20.second * N20.second);
        N20.first /= len20; N20.second /= len20;

        // Offset edges outward by radius
        std::pair<float, float> P0_offset01 = { P0.first + radius * N01.first, P0.second + radius * N01.second };
        std::pair<float, float> P1_offset01 = { P1.first + radius * N01.first, P1.second + radius * N01.second };
        std::pair<float, float> P1_offset12 = { P1.first + radius * N12.first, P1.second + radius * N12.second };
        std::pair<float, float> P2_offset12 = { P2.first + radius * N12.first, P2.second + radius * N12.second };
        std::pair<float, float> P2_offset20 = { P2.first + radius * N20.first, P2.second + radius * N20.second };
        std::pair<float, float> P0_offset20 = { P0.first + radius * N20.first, P0.second + radius * N20.second };

        // Compute new vertex positions as intersections of offset lines
        auto intersectLines = [](std::pair<float, float> A, std::pair<float, float> B,
            std::pair<float, float> C, std::pair<float, float> D) -> std::pair<float, float> {
                float Vx = B.first - A.first;
                float Vy = B.second - A.second;
                float Wx = D.first - C.first;
                float Wy = D.second - C.second;
                float Px = C.first - A.first;
                float Py = C.second - A.second;
                float det = Vx * Wy - Vy * Wx;
                if (std::abs(det) < 1e-6f) return { NAN, NAN }; // Handle parallel lines (rare)
                float t = (Px * Wy - Py * Wx) / det;
                return { A.first + t * Vx, A.second + t * Vy };
            };

        std::pair<float, float> A_new = intersectLines(P2_offset20, P0_offset20, P0_offset01, P1_offset01);
        std::pair<float, float> B_new = intersectLines(P0_offset01, P1_offset01, P1_offset12, P2_offset12);
        std::pair<float, float> C_new = intersectLines(P1_offset12, P2_offset12, P2_offset20, P0_offset20);

        // Unproject back to 3D
        auto unproject = [&](std::pair<float, float> P2D) -> vec3 {
            return A + U * P2D.first + V * P2D.second;
            };
        vec3 A_exp = unproject(A_new);
        vec3 B_exp = unproject(B_new);
        vec3 C_exp = unproject(C_new);

        // Accumulate new positions
        accum[i0] += A_exp;
        count[i0]++;
        accum[i1] += B_exp;
        count[i1]++;
        accum[i2] += C_exp;
        count[i2]++;
    }

    // Compute averaged new vertex positions
    std::vector<vec3> newVertices(mesh.vertices.size());
    for (size_t i = 0; i < mesh.vertices.size(); ++i) {
        if (count[i] > 0) {
            newVertices[i] = accum[i] / float(count[i]);
        }
        else {
            newVertices[i] = mesh.vertices[i]; // Unreferenced vertices (unlikely in a valid mesh)
        }
    }

    // Set result
    result.vertices = newVertices;
    result.indices = mesh.indices; // Reuse original indices with new vertex positions
    return result;
}

// Helper: Check if a triangle is valid (oriented correctly)
static bool IsTriangleValid(const std::vector<std::pair<double, double>>& pts2D, size_t i, size_t j, size_t k) {
    auto& p1 = pts2D[i], & p2 = pts2D[j], & p3 = pts2D[k];
    double cross = (p2.first - p1.first) * (p3.second - p1.second) - (p2.second - p1.second) * (p3.first - p1.first);
    return cross > 0; // Ensure consistent orientation (CCW)
}

// Helper: Check if a point is inside a triangle (for ear clipping)
static bool IsPointInTriangle(const std::pair<double, double>& p, const std::pair<double, double>& a,
    const std::pair<double, double>& b, const std::pair<double, double>& c) {
    auto sign = [](const std::pair<double, double>& p1, const std::pair<double, double>& p2,
        const std::pair<double, double>& p3) {
            return (p1.first - p3.first) * (p2.second - p3.second) - (p2.first - p3.first) * (p1.second - p3.second);
        };
    double d1 = sign(p, a, b);
    double d2 = sign(p, b, c);
    double d3 = sign(p, c, a);
    bool has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    bool has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);
    return !(has_neg && has_pos);
}

// Helper: Check if a triangle is an ear (no points inside)
static bool IsEar(const std::vector<std::pair<double, double>>& pts2D, const std::vector<size_t>& indices,
    size_t i, size_t j, size_t k) {
    if (!IsTriangleValid(pts2D, indices[i], indices[j], indices[k])) return false;
    for (size_t m = 0; m < indices.size(); ++m) {
        if (m != i && m != j && m != k) {
            if (IsPointInTriangle(pts2D[indices[m]], pts2D[indices[i]], pts2D[indices[j]], pts2D[indices[k]])) {
                return false;
            }
        }
    }
    return true;
}

MeshUtils::PositionVerticesIndices MeshUtils::MergeCoplanarRegions(const PositionVerticesIndices& mesh, float normalTolerance)
{
    PositionVerticesIndices result;
    result.vertices = mesh.vertices;
    size_t triCount = mesh.indices.size() / 3;

    // Build edge->tris map
    std::unordered_map<Edge, std::vector<uint32_t>, EdgeHash, EdgeEq> edgeMap;
    edgeMap.reserve(triCount * 3);
    for (uint32_t t = 0; t < triCount; ++t) {
        auto v0 = mesh.indices[3 * t], v1 = mesh.indices[3 * t + 1], v2 = mesh.indices[3 * t + 2];
        edgeMap[Edge(v0, v1)].push_back(t);
        edgeMap[Edge(v1, v2)].push_back(t);
        edgeMap[Edge(v2, v0)].push_back(t);
    }

    std::vector<bool> used(triCount, false);
    auto getNormal = [&](uint32_t t) {
        vec3 A = mesh.vertices[mesh.indices[3 * t]];
        vec3 B = mesh.vertices[mesh.indices[3 * t + 1]];
        vec3 C = mesh.vertices[mesh.indices[3 * t + 2]];
        return normalize(cross(B - A, C - A));
        };

    for (uint32_t seed = 0; seed < triCount; ++seed) {
        if (used[seed]) continue;
        vec3 baseN = getNormal(seed);
        if (fabs(baseN.y) < 0.8) continue; // Skip non-horizontal regions

        // Flood-fill coplanar region
        std::vector<uint32_t> region;
        std::queue<uint32_t> q;
        used[seed] = true;
        q.push(seed);
        while (!q.empty()) {
            uint32_t t = q.front();
            q.pop();
            region.push_back(t);
            auto verts = std::array<uint32_t, 3>{ mesh.indices[3 * t], mesh.indices[3 * t + 1], mesh.indices[3 * t + 2] };
            for (int e = 0; e < 3; ++e) {
                Edge ed(verts[e], verts[(e + 1) % 3]);
                for (uint32_t nbr : edgeMap[ed]) {
                    if (!used[nbr] && dot(baseN, getNormal(nbr)) >= normalTolerance) {
                        used[nbr] = true;
                        q.push(nbr);
                    }
                }
            }
        }

        // Collect boundary edges
        std::unordered_map<Edge, int, EdgeHash, EdgeEq> edgeCount;
        for (uint32_t t : region) {
            auto v0 = mesh.indices[3 * t], v1 = mesh.indices[3 * t + 1], v2 = mesh.indices[3 * t + 2];
            edgeCount[Edge(v0, v1)]++;
            edgeCount[Edge(v1, v2)]++;
            edgeCount[Edge(v2, v0)]++;
        }
        std::vector<Edge> boundaryEdges;
        for (auto& kv : edgeCount) {
            if (kv.second == 1) {
                boundaryEdges.push_back(kv.first);
            }
        }

        if (boundaryEdges.size() < 3) continue;

        // Chain boundary into a closed loop
        std::vector<uint32_t> boundaryVerts = ChainBoundary(boundaryEdges);
        if (boundaryVerts.size() < 3) continue;

        // Project boundary to 2D
        vec3 u = fabs(baseN.x) > fabs(baseN.z) ? normalize(cross(vec3(0, 1, 0), baseN))
            : normalize(cross(vec3(1, 0, 0), baseN));
        vec3 v = cross(baseN, u);
        std::vector<std::pair<double, double>> pts2D(boundaryVerts.size());
        for (size_t i = 0; i < boundaryVerts.size(); ++i) {
            auto P = result.vertices[boundaryVerts[i]];
            pts2D[i] = { dot(P, u), dot(P, v) };
        }

        // Ensure the polygon is oriented counter-clockwise
        double area = 0;
        for (size_t i = 0; i < pts2D.size(); ++i) {
            auto& p1 = pts2D[i], & p2 = pts2D[(i + 1) % pts2D.size()];
            area += p1.first * p2.second - p2.first * p1.second;
        }
        if (area < 0) {
            std::reverse(boundaryVerts.begin(), boundaryVerts.end());
            std::reverse(pts2D.begin(), pts2D.end());
        }

        // Triangulate using ear clipping
        std::vector<size_t> indices(pts2D.size());
        std::iota(indices.begin(), indices.end(), 0);
        std::vector<uint32_t> triIndices;
        while (indices.size() >= 3) {
            bool foundEar = false;
            for (size_t i = 0; i < indices.size(); ++i) {
                size_t j = (i + 1) % indices.size();
                size_t k = (i + 2) % indices.size();
                if (IsEar(pts2D, indices, i, j, k)) {
                    // Add triangle
                    triIndices.push_back(boundaryVerts[indices[i]]);
                    triIndices.push_back(boundaryVerts[indices[j]]);
                    triIndices.push_back(boundaryVerts[indices[k]]);
                    // Remove vertex j (the ear)
                    indices.erase(indices.begin() + j);
                    foundEar = true;
                    break;
                }
            }
            if (!foundEar) break; // Malformed polygon (e.g., self-intersecting)
        }

        // Add triangles to result
        result.indices.insert(result.indices.end(), triIndices.begin(), triIndices.end());
    }

    return result;
}
