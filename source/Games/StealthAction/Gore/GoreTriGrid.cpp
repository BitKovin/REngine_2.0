#include "GoreTriGrid.h"

#include "GoreProfile.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <unordered_map>

namespace Gore
{
    namespace
    {
        // Share of the edges of a mesh (vertices welded by position) that belong to one triangle only. A closed surface has none;
        // a jacket with open cuffs and collar a few percent; hair cards most of them.
        float OpenEdgeShare(const roj::SkinnedMesh& mesh)
        {
            std::unordered_map<uint64_t, uint32_t> weld;
            auto key = [](const glm::vec3& p)
            {
                const auto q = [](float v) { return static_cast<uint64_t>(static_cast<int64_t>(std::llround(v * 2000.0f)) & 0x1FFFFF); };
                return q(p.x) | (q(p.y) << 21) | (q(p.z) << 42);
            };
            std::vector<uint32_t> id(mesh.vertices.size());
            for (size_t v = 0; v < mesh.vertices.size(); ++v)
                id[v] = weld.emplace(key(mesh.vertices[v].Position), static_cast<uint32_t>(weld.size())).first->second;

            std::map<std::pair<uint32_t, uint32_t>, int> edges;
            for (size_t i = 0; i + 2 < mesh.indices.size(); i += 3)
                for (int e = 0; e < 3; ++e)
                {
                    const uint32_t a = id[mesh.indices[i + e]], b = id[mesh.indices[i + (e + 1) % 3]];
                    ++edges[{ std::min(a, b), std::max(a, b) }];
                }
            if (edges.empty()) return 1.0f;
            size_t open = 0;
            for (const auto& kv : edges)
                if (kv.second == 1) ++open;
            return static_cast<float>(open) / static_cast<float>(edges.size());
        }

        constexpr float kClosedShare = 0.15f;   // a non-Volume mesh with fewer open edges than this encloses part of the body

        bool RayTri(const glm::vec3& o, const glm::vec3& d, const GridTri& t, float& out)
        {
            const glm::vec3 e1 = t.b - t.a, e2 = t.c - t.a;
            const glm::vec3 p = glm::cross(d, e2);
            const float det = glm::dot(e1, p);
            if (std::abs(det) < 1.0e-10f) return false;
            const float inv = 1.0f / det;
            const glm::vec3 s = o - t.a;
            const float u = glm::dot(s, p) * inv;
            if (u < 0.0f || u > 1.0f) return false;
            const glm::vec3 q = glm::cross(s, e1);
            const float v = glm::dot(d, q) * inv;
            if (v < 0.0f || u + v > 1.0f) return false;
            out = glm::dot(e2, q) * inv;
            return out > 1.0e-4f;
        }

        // The point of triangle (a, b, c) nearest to p, as barycentric coordinates (Ericson, Real-Time Collision Detection 5.1.5).
        glm::vec3 ClosestBary(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c)
        {
            const glm::vec3 ab = b - a, ac = c - a, ap = p - a;
            const float d1 = glm::dot(ab, ap), d2 = glm::dot(ac, ap);
            if (d1 <= 0.0f && d2 <= 0.0f) return glm::vec3(1, 0, 0);

            const glm::vec3 bp = p - b;
            const float d3 = glm::dot(ab, bp), d4 = glm::dot(ac, bp);
            if (d3 >= 0.0f && d4 <= d3) return glm::vec3(0, 1, 0);

            const float vc = d1 * d4 - d3 * d2;
            if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f)
            {
                const float v = d1 / (d1 - d3);
                return glm::vec3(1.0f - v, v, 0.0f);
            }

            const glm::vec3 cp = p - c;
            const float d5 = glm::dot(ab, cp), d6 = glm::dot(ac, cp);
            if (d6 >= 0.0f && d5 <= d6) return glm::vec3(0, 0, 1);

            const float vb = d5 * d2 - d1 * d6;
            if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f)
            {
                const float w = d2 / (d2 - d6);
                return glm::vec3(1.0f - w, 0.0f, w);
            }

            const float va = d3 * d6 - d5 * d4;
            if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f)
            {
                const float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
                return glm::vec3(0.0f, 1.0f - w, w);
            }

            const float denom = 1.0f / (va + vb + vc);
            const float v = vb * denom, w = vc * denom;
            return glm::vec3(1.0f - v - w, v, w);
        }
    }

    void TriGrid::Build(const roj::SkinnedModel& model, const Profile& profile, Mode mode)
    {
        glm::vec3 lo(1.0e9f), hi(-1.0e9f);
        for (size_t m = 0; m < model.meshes.size(); ++m)
        {
            const SubmeshRole role = m < profile.submeshRoles.size() ? profile.submeshRoles[m] : SubmeshRole::Volume;
            if (role == SubmeshRole::Attachment || role == SubmeshRole::Prop) continue;
            if (role != SubmeshRole::Volume && (mode == Mode::Volume || OpenEdgeShare(model.meshes[m]) > kClosedShare)) continue;
            const roj::SkinnedMesh& mesh = model.meshes[m];
            for (size_t i = 0; i + 2 < mesh.indices.size(); i += 3)
            {
                const VertexData& va = mesh.vertices[mesh.indices[i]];
                const VertexData& vb = mesh.vertices[mesh.indices[i + 1]];
                const VertexData& vc = mesh.vertices[mesh.indices[i + 2]];
                GridTri t;
                t.a = va.Position; t.b = vb.Position; t.c = vc.Position;
                t.na = va.Normal; t.nb = vb.Normal; t.nc = vc.Normal;
                t.mesh = static_cast<uint16_t>(m);
                const VertexData* corners[3] = { &va, &vb, &vc };
                for (int k = 0; k < 3; ++k)
                {
                    for (int j = 0; j < 4; ++j) t.bone[k][j] = corners[k]->BlendIndices[j];
                    t.weight[k] = corners[k]->BlendWeights;
                }
                tris.push_back(t);
                lo = glm::min(lo, glm::min(t.a, glm::min(t.b, t.c)));
                hi = glm::max(hi, glm::max(t.a, glm::max(t.b, t.c)));
            }
        }
        if (tris.empty()) return;

        origin = lo - glm::vec3(0.01f);
        const glm::vec3 size = hi - origin + glm::vec3(0.01f);
        for (int k = 0; k < 3; ++k)
            dims[k] = glm::clamp(static_cast<int>(std::ceil(size[k] / cell)), 1, 128);

        const size_t cells = static_cast<size_t>(dims.x) * dims.y * dims.z;
        std::vector<uint32_t> counts(cells, 0);
        auto forCells = [&](const GridTri& t, auto&& fn)
        {
            const glm::ivec3 c0 = CellOf(glm::min(t.a, glm::min(t.b, t.c)));
            const glm::ivec3 c1 = CellOf(glm::max(t.a, glm::max(t.b, t.c)));
            for (int z = c0.z; z <= c1.z; ++z)
                for (int y = c0.y; y <= c1.y; ++y)
                    for (int x = c0.x; x <= c1.x; ++x)
                        fn(Index(x, y, z));
        };
        for (const GridTri& t : tris) forCells(t, [&](size_t c) { ++counts[c]; });

        cellStart.assign(cells + 1, 0);
        for (size_t c = 0; c < cells; ++c) cellStart[c + 1] = cellStart[c] + counts[c];
        cellTris.assign(cellStart[cells], 0);
        std::fill(counts.begin(), counts.end(), 0);
        for (uint32_t i = 0; i < tris.size(); ++i)
            forCells(tris[i], [&](size_t c) { cellTris[cellStart[c] + counts[c]++] = i; });

        stamp.assign(tris.size(), 0);
    }

    float TriGrid::Cast(const glm::vec3& o, const glm::vec3& d, float tMax)
    {
        if (tris.empty()) return -1.0f;
        ++stampId;
        float best = -1.0f;
        long long last = -1;
        const float step = cell * 0.25f;
        for (float s = 0.0f; s <= tMax + step; s += step)
        {
            if (best >= 0.0f && best <= s - cell) break;   // every cell up to the hit was visited
            const glm::vec3 p = o + d * std::min(s, tMax);
            const glm::vec3 f = (p - origin) / cell;
            const int x = static_cast<int>(std::floor(f.x)), y = static_cast<int>(std::floor(f.y)), z = static_cast<int>(std::floor(f.z));
            if (x < 0 || y < 0 || z < 0 || x >= dims.x || y >= dims.y || z >= dims.z) continue;
            const long long c = static_cast<long long>(Index(x, y, z));
            if (c == last) continue;
            last = c;
            for (uint32_t k = cellStart[c]; k < cellStart[c + 1]; ++k)
            {
                const uint32_t ti = cellTris[k];
                if (stamp[ti] == stampId) continue;
                stamp[ti] = stampId;
                float t;
                if (RayTri(o, d, tris[ti], t) && t <= tMax && (best < 0.0f || t < best)) best = t;
            }
        }
        return best;
    }

    BodyQuery TriGrid::Closest(const glm::vec3& p, float maxDist) const
    {
        BodyQuery result;
        if (tris.empty()) return result;

        const glm::vec3 f = (p - origin) / cell;
        const glm::ivec3 centre(static_cast<int>(std::floor(f.x)), static_cast<int>(std::floor(f.y)), static_cast<int>(std::floor(f.z)));
        const int maxRing = static_cast<int>(std::ceil(maxDist / cell)) + 1;

        float best = maxDist;
        bool found = false;
        uint32_t bestTri = 0;
        glm::vec3 bestBary(0.0f);

        // The body is the union of its meshes (a jacket, the shirt under it, trousers: each a closed surface that encloses part of
        // it), so a point is inside when it is inside any one of them. The nearest face alone is not enough: between the jacket
        // and the shirt it may belong to the shirt, which the point is outside of. So the nearest face of every mesh is kept.
        constexpr int kSlots = 32;                                  // meshes beyond this share the last slot
        constexpr float kUnionSlack = 0.05f;                        // the search goes this far past the nearest face to find the other meshes
        constexpr float kNone = std::numeric_limits<float>::max();
        float meshBest[kSlots];
        uint32_t meshTri[kSlots];
        glm::vec3 meshBary[kSlots];
        for (int s = 0; s < kSlots; ++s) meshBest[s] = kNone;

        for (int ring = 0; ring <= maxRing; ++ring)
        {
            if (found && best + kUnionSlack <= static_cast<float>(ring - 1) * cell) break;   // every cell not yet visited is farther than that

            for (int z = centre.z - ring; z <= centre.z + ring; ++z)
            {
                if (z < 0 || z >= dims.z) continue;
                for (int y = centre.y - ring; y <= centre.y + ring; ++y)
                {
                    if (y < 0 || y >= dims.y) continue;
                    for (int x = centre.x - ring; x <= centre.x + ring; ++x)
                    {
                        if (x < 0 || x >= dims.x) continue;
                        if (std::max(std::abs(x - centre.x), std::max(std::abs(y - centre.y), std::abs(z - centre.z))) != ring) continue;

                        const size_t c = Index(x, y, z);
                        for (uint32_t k = cellStart[c]; k < cellStart[c + 1]; ++k)   // a triangle in several cells is tested again: harmless
                        {
                            const GridTri& t = tris[cellTris[k]];
                            const glm::vec3 bary = ClosestBary(p, t.a, t.b, t.c);
                            const glm::vec3 point = t.a * bary.x + t.b * bary.y + t.c * bary.z;
                            const float d = glm::distance(p, point);
                            const int slot = std::min<int>(t.mesh, kSlots - 1);
                            if (d <= maxDist && d < meshBest[slot])
                            {
                                meshBest[slot] = d;
                                meshTri[slot] = cellTris[k];
                                meshBary[slot] = bary;
                            }
                            if (d < best || (!found && d <= best))
                            {
                                best = d;
                                found = true;
                                bestTri = cellTris[k];
                                bestBary = bary;
                            }
                        }
                    }
                }
            }
        }
        if (!found) return result;

        // Signed distance to each mesh found (behind its nearest face = inside); the body's is the largest: inside any mesh wins.
        float signedBest = -kNone;
        for (int s = 0; s < kSlots; ++s)
        {
            if (meshBest[s] == kNone) continue;
            const GridTri& t = tris[meshTri[s]];
            const glm::vec3 point = t.a * meshBary[s].x + t.b * meshBary[s].y + t.c * meshBary[s].z;
            glm::vec3 n = t.na * meshBary[s].x + t.nb * meshBary[s].y + t.nc * meshBary[s].z;
            if (glm::length(n) < 1.0e-5f) n = glm::cross(t.b - t.a, t.c - t.a);
            signedBest = std::max(signedBest, glm::dot(p - point, n) < 0.0f ? meshBest[s] : -meshBest[s]);
        }

        result.found = true;
        result.tri = bestTri;           // the nearest face of all: its bone weights are the ones to copy
        result.bary = bestBary;
        result.signedDistance = signedBest;
        return result;
    }

    glm::ivec3 TriGrid::CellOf(const glm::vec3& p) const
    {
        const glm::vec3 f = (p - origin) / cell;
        return glm::ivec3(glm::clamp(static_cast<int>(std::floor(f.x)), 0, dims.x - 1),
                          glm::clamp(static_cast<int>(std::floor(f.y)), 0, dims.y - 1),
                          glm::clamp(static_cast<int>(std::floor(f.z)), 0, dims.z - 1));
    }
}
