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

// Check if two triangles share an edge (exactly 2 common vertices)
static bool sharesEdge(uint32_t t1, uint32_t t2, const vector<uint32_t>& indices) {
    unordered_set<uint32_t> verts1 = { indices[3 * t1], indices[3 * t1 + 1], indices[3 * t1 + 2] };
    unordered_set<uint32_t> verts2 = { indices[3 * t2], indices[3 * t2 + 1], indices[3 * t2 + 2] };
    int shared = 0;
    for (uint32_t v : verts1) if (verts2.count(v)) shared++;
    return shared == 2;
}

// Determine if a 2D loop is CCW (outer boundary)
static bool IsCCW(const vector<pair<double, double>>& poly) {
    double sum = 0.0;
    for (size_t i = 0; i < poly.size(); ++i) {
        auto& p1 = poly[i];
        auto& p2 = poly[(i + 1) % poly.size()];
        sum += (p2.first - p1.first) * (p2.second + p1.second);
    }
    return sum < 0.0;
}

// Extract all boundary loops from unique edges (edges appearing exactly once)
vector<vector<uint32_t>> ExtractBoundaryLoops(const vector<MeshUtils::Edge>& edges) {
    unordered_multimap<uint32_t, uint32_t> adj;
    for (auto& e : edges) {
        adj.insert({ e.a, e.b });
        adj.insert({ e.b, e.a });
    }

    unordered_set<uint32_t> visited;
    vector<vector<uint32_t>> loops;

    for (auto& e : edges) {
        if (visited.count(e.a)) continue;
        vector<uint32_t> loop;
        uint32_t curr = e.a;
        uint32_t prev = UINT32_MAX;
        do {
            loop.push_back(curr);
            visited.insert(curr);
            auto range = adj.equal_range(curr);
            uint32_t next = UINT32_MAX;
            for (auto it = range.first; it != range.second; ++it) {
                if (it->second != prev && (!visited.count(it->second) || it->second == loop.front())) {
                    next = it->second;
                    break;
                }
            }
            prev = curr;
            curr = next;
        } while (curr != UINT32_MAX && curr != loop.front());

        if (loop.size() >= 3) {
            loops.push_back(loop);
        }
    }

    return loops;
}

// Simple ear-clipping triangulator for a single CCW loop
vector<uint32_t> TriangulateSimplePolygon(const vector<uint32_t>& loop,
    const vector<pair<double, double>>& pts2D) {
    vector<size_t> idx(loop.size());
    std::iota(idx.begin(), idx.end(), 0);
    vector<uint32_t> tris;

    auto valid = [&](size_t i, size_t j, size_t k) {
        auto& A = pts2D[i];
        auto& B = pts2D[j];
        auto& C = pts2D[k];
        double cross = (B.first - A.first) * (C.second - A.second)
            - (B.second - A.second) * (C.first - A.first);
        return cross > 0.0;
        };

    auto inside = [&](const pair<double, double>& P,
        const pair<double, double>& A,
        const pair<double, double>& B,
        const pair<double, double>& C) {
            auto sign = [&](const pair<double, double>& P1,
                const pair<double, double>& P2,
                const pair<double, double>& P3) {
                    return (P1.first - P3.first) * (P2.second - P3.second)
                        - (P2.first - P3.first) * (P1.second - P3.second);
                };
            double d1 = sign(P, A, B);
            double d2 = sign(P, B, C);
            double d3 = sign(P, C, A);
            bool hasNeg = (d1 < 0) || (d2 < 0) || (d3 < 0);
            bool hasPos = (d1 > 0) || (d2 > 0) || (d3 > 0);
            return !(hasNeg && hasPos);
        };

    while (idx.size() >= 3) {
        bool foundEar = false;
        for (size_t i = 0; i < idx.size(); ++i) {
            size_t j = (i + 1) % idx.size();
            size_t k = (i + 2) % idx.size();
            if (!valid(idx[i], idx[j], idx[k])) continue;
            bool ear = true;
            for (size_t m = 0; m < idx.size(); ++m) {
                if (m == i || m == j || m == k) continue;
                if (inside(pts2D[idx[m]], pts2D[idx[i]], pts2D[idx[j]], pts2D[idx[k]])) {
                    ear = false;
                    break;
                }
            }
            if (ear) {
                tris.push_back(loop[idx[i]]);
                tris.push_back(loop[idx[j]]);
                tris.push_back(loop[idx[k]]);
                idx.erase(idx.begin() + j);
                foundEar = true;
                break;
            }
        }
        if (!foundEar) break;
    }

    return tris;
}

MeshUtils::PositionVerticesIndices MeshUtils::MergeCoplanarRegions(const MeshUtils::PositionVerticesIndices& mesh,
    float normalTolerance) {
    MeshUtils::PositionVerticesIndices result;
    result.vertices = mesh.vertices;
    size_t triCount = mesh.indices.size() / 3;

    // Build triangle adjacency by edges
    unordered_map<MeshUtils::Edge, vector<uint32_t>, MeshUtils::EdgeHash, MeshUtils::EdgeEq> edgeMap;
    edgeMap.reserve(triCount * 3);
    for (uint32_t t = 0; t < triCount; ++t) {
        uint32_t v0 = mesh.indices[3 * t];
        uint32_t v1 = mesh.indices[3 * t + 1];
        uint32_t v2 = mesh.indices[3 * t + 2];
        edgeMap[MeshUtils::Edge(v0, v1)].push_back(t);
        edgeMap[MeshUtils::Edge(v1, v2)].push_back(t);
        edgeMap[MeshUtils::Edge(v2, v0)].push_back(t);
    }

    vector<bool> used(triCount, false);
    auto getNormal = [&](uint32_t t) {
        vec3 A = mesh.vertices[mesh.indices[3 * t]];
        vec3 B = mesh.vertices[mesh.indices[3 * t + 1]];
        vec3 C = mesh.vertices[mesh.indices[3 * t + 2]];
        return normalize(cross(B - A, C - A));
        };

    for (uint32_t seed = 0; seed < triCount; ++seed) {
        if (used[seed]) continue;
        vec3 baseN = getNormal(seed);
        // Keep only approximately horizontal
        if (std::fabs(baseN.y) < 0.8f) continue;

        // Flood-fill coplanar region
        vector<uint32_t> region;
        std::queue<uint32_t> q;
        used[seed] = true;
        q.push(seed);
        while (!q.empty()) {
            uint32_t t = q.front(); q.pop();
            region.push_back(t);
            std::array<uint32_t, 3> verts = { { mesh.indices[3 * t], mesh.indices[3 * t + 1], mesh.indices[3 * t + 2] } };
            for (int e = 0; e < 3; ++e) {
                MeshUtils::Edge ed(verts[e], verts[(e + 1) % 3]);
                for (uint32_t nbr : edgeMap[ed]) {
                    if (!used[nbr] && dot(baseN, getNormal(nbr)) >= normalTolerance
                        && sharesEdge(t, nbr, mesh.indices)) {
                        used[nbr] = true;
                        q.push(nbr);
                    }
                }
            }
        }

        // Count boundary edges
        unordered_map<MeshUtils::Edge, int, MeshUtils::EdgeHash, MeshUtils::EdgeEq> edgeCount;
        for (uint32_t t : region) {
            uint32_t v0 = mesh.indices[3 * t];
            uint32_t v1 = mesh.indices[3 * t + 1];
            uint32_t v2 = mesh.indices[3 * t + 2];
            edgeCount[MeshUtils::Edge(v0, v1)]++;
            edgeCount[MeshUtils::Edge(v1, v2)]++;
            edgeCount[MeshUtils::Edge(v2, v0)]++;
        }
        vector<MeshUtils::Edge> boundary;
        for (auto& kv : edgeCount) {
            if (kv.second == 1) boundary.push_back(kv.first);
        }
        if (boundary.size() < 3) continue;

        // Extract loops and triangulate only the outer ones
        auto loops = ExtractBoundaryLoops(boundary);
        for (auto& loop : loops) {
            // Project loop to 2D plane of region
            vec3 u = std::fabs(baseN.x) > std::fabs(baseN.z)
                ? normalize(cross(vec3(0, 1, 0), baseN))
                : normalize(cross(vec3(1, 0, 0), baseN));
            vec3 v = cross(baseN, u);
            vector<pair<double, double>> pts2D(loop.size());
            for (size_t i = 0; i < loop.size(); ++i) {
                vec3 P = result.vertices[loop[i]];
                pts2D[i] = { dot(P, u), dot(P, v) };
            }
            if (!IsCCW(pts2D)) continue;  // skip holes

            auto tris = TriangulateSimplePolygon(loop, pts2D);
            result.indices.insert(result.indices.end(), tris.begin(), tris.end());
        }
    }

    return result;
}

