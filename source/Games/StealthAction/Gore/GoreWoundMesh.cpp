#include "GoreWoundMesh.h"

#include <ThreadPool.h>

#include <algorithm>
#include <cmath>

namespace Gore
{
    namespace
    {
        // A copy of one skin triangle (the worker never touches the model).
        struct TriRef
        {
            VertexData v[3];
        };

        // Up to four bones per vertex, like the model's own vertices.
        struct Influence
        {
            float bone[4] = { 0, 0, 0, 0 };
            float weight[4] = { 0, 0, 0, 0 };
        };

        struct Surface
        {
            bool hit = false;
            float t = 0.0f;
            glm::vec3 normal = glm::vec3(0, 0, 1);   // of the face that was hit
            glm::vec3 outward = glm::vec3(0, 0, 1);  // authored outside direction at the hit (interpolated vertex normals)
            float farLimit = 1.0e9f;   // distance from the surface to the next surface behind it (thin parts)
            Influence influence;
        };

        // A small bag of (bone, weight) pairs: sums duplicates, keeps the strongest four, normalises.
        struct Bag
        {
            float bone[16];
            float weight[16];
            int count = 0;

            void Add(float b, float w)
            {
                if (!(w > 0.0f)) return;
                for (int i = 0; i < count; ++i)
                    if (bone[i] == b) { weight[i] += w; return; }
                if (count < 16) { bone[count] = b; weight[count] = w; ++count; }
            }

            void Add(const Influence& inf, float scale)
            {
                for (int k = 0; k < 4; ++k) Add(inf.bone[k], inf.weight[k] * scale);
            }

            Influence Resolve() const
            {
                int order[16];
                for (int i = 0; i < count; ++i) order[i] = i;
                std::sort(order, order + count, [&](int a, int b) { return weight[a] > weight[b]; });

                Influence out;
                float sum = 0.0f;
                const int n = std::min(count, 4);
                for (int i = 0; i < n; ++i) { out.bone[i] = bone[order[i]]; out.weight[i] = weight[order[i]]; sum += out.weight[i]; }
                if (sum > 1.0e-6f)
                    for (int i = 0; i < n; ++i) out.weight[i] /= sum;
                else
                    out.weight[0] = 1.0f;
                return out;
            }
        };

        void AddVertexInfluence(Bag& bag, const VertexData& v, float scale)
        {
            for (int k = 0; k < 4; ++k)
                bag.Add(v.BlendIndices[k], v.BlendWeights[k] * scale);
        }

        // Moeller-Trumbore, both sides. Returns false on a miss; t along dir, barycentric u (towards b), v (towards c).
        bool RayTriangle(const glm::vec3& o, const glm::vec3& d, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c,
                         float& t, float& u, float& v)
        {
            const glm::vec3 e1 = b - a, e2 = c - a;
            const glm::vec3 p = glm::cross(d, e2);
            const float det = glm::dot(e1, p);
            if (std::abs(det) < 1.0e-9f) return false;
            const float inv = 1.0f / det;
            const glm::vec3 s = o - a;
            u = glm::dot(s, p) * inv;
            if (u < 0.0f || u > 1.0f) return false;
            const glm::vec3 q = glm::cross(s, e1);
            v = glm::dot(d, q) * inv;
            if (v < 0.0f || u + v > 1.0f) return false;
            t = glm::dot(e2, q) * inv;
            return t > 0.0f;
        }

        struct Scene
        {
            std::vector<TriRef> tris;

            const VertexData& V(const TriRef& t, int k) const { return t.v[k]; }

            // First surface along the ray inside the t window, and the distance to the next one behind it.
            Surface Cast(const glm::vec3& o, const glm::vec3& d, float tLo, float tHi) const
            {
                Surface s;
                float bestU = 0, bestV = 0;
                const TriRef* best = nullptr;
                for (const TriRef& tr : tris)
                {
                    float t, u, v;
                    if (!RayTriangle(o, d, V(tr, 0).Position, V(tr, 1).Position, V(tr, 2).Position, t, u, v)) continue;
                    if (t < tLo || t > tHi) continue;
                    if (!s.hit || t < s.t) { s.hit = true; s.t = t; best = &tr; bestU = u; bestV = v; }
                }
                if (!s.hit) return s;

                Bag bag;
                AddVertexInfluence(bag, V(*best, 0), 1.0f - bestU - bestV);
                AddVertexInfluence(bag, V(*best, 1), bestU);
                AddVertexInfluence(bag, V(*best, 2), bestV);
                s.influence = bag.Resolve();
                {
                    const glm::vec3 vn = V(*best, 0).Normal * (1.0f - bestU - bestV) + V(*best, 1).Normal * bestU + V(*best, 2).Normal * bestV;
                    s.outward = glm::length(vn) > 1.0e-6f ? glm::normalize(vn) : glm::vec3(0, 0, 1);
                }
                s.normal = glm::normalize(glm::cross(V(*best, 1).Position - V(*best, 0).Position, V(*best, 2).Position - V(*best, 0).Position));

                for (const TriRef& tr : tris)
                {
                    float t, u, v;
                    if (!RayTriangle(o, d, V(tr, 0).Position, V(tr, 1).Position, V(tr, 2).Position, t, u, v)) continue;
                    if (t > s.t + 0.004f) s.farLimit = std::min(s.farLimit, t - s.t);
                }
                return s;
            }

            // Distance to the nearest face along a ray (1e9 when there is none).
            float Distance(const glm::vec3& o, const glm::vec3& d) const
            {
                float best = 1.0e9f;
                for (const TriRef& tr : tris)
                {
                    float t, u, v;
                    if (RayTriangle(o, d, V(tr, 0).Position, V(tr, 1).Position, V(tr, 2).Position, t, u, v) && t > 0.003f)
                        best = std::min(best, t);
                }
                return best;
            }

            // Weights of the vertex nearest to p (used where the ray finds no face, e.g. over a silhouette edge).
            Influence Nearest(const glm::vec3& p) const
            {
                float bestD = 1.0e30f;
                const VertexData* best = nullptr;
                for (const TriRef& tr : tris)
                    for (int k = 0; k < 3; ++k)
                    {
                        const VertexData& vd = V(tr, k);
                        const float d = glm::dot(vd.Position - p, vd.Position - p);
                        if (d < bestD) { bestD = d; best = &vd; }
                    }
                Bag bag;
                if (best != nullptr) AddVertexInfluence(bag, *best, 1.0f);
                return bag.Resolve();
            }
        };

        Influence Mix(const Influence& a, const Influence& b, float t)   // t = share of b
        {
            Bag bag;
            bag.Add(a, 1.0f - t);
            bag.Add(b, t);
            return bag.Resolve();
        }
    }

    // The cavity is built from the model's own faces:
    //  * every triangle of the model that overlaps the wound ellipsoid is subdivided into a barycentric grid (the same
    //    grid density on every triangle, so neighbouring triangles agree on shared edges: no cracks);
    //  * a grid point that lies inside the wound is pushed inward, against the authored outside of the model (interpolated
    //    vertex normals), by the depth of the wound ellipsoid under it (limited by the body behind it); points outside
    //    stay on the skin, so the bowl joins the surface exactly along the edge of the hole;
    //  * every point takes the bone indices/weights interpolated from the three vertices of its triangle (barycentric,
    //    the strongest four bones), so it is skinned exactly like the skin it came from.
    // Cloth over a body, entry and exit sides, ridges and joints are all handled the same way, because nothing depends on
    // the shot direction.
    namespace
    {
    std::shared_ptr<const CavityGeometry> BuildCavityFromTriangles(const Wound& wound, std::vector<TriRef>&& triangles)
    {
        const float grow = 1.0f + wound.roughness + 0.1f;   // beyond the noisy hole edge, hidden under the skin
        const glm::vec3 radii = wound.radii * grow;
        const float reachXY = std::max(radii.x, radii.y);
        const glm::mat3 R = glm::mat3_cast(wound.rotation);
        const glm::mat3 Rinv = glm::transpose(R);
        const glm::vec3 X = R[0], Y = R[1];

        Scene scene;
        scene.tris = std::move(triangles);
        if (scene.tris.empty()) return nullptr;

        // Wound-space helpers.
        auto toUnit = [&](const glm::vec3& p) { return (Rinv * (p - wound.center)) / radii; };

        // Distance from p, going along dir (unit), to the far side of the wound ellipsoid; 0 when p is outside it.
        auto ellipsoidDepth = [&](const glm::vec3& p, const glm::vec3& dir)
        {
            const glm::vec3 o = toUnit(p);
            if (glm::dot(o, o) >= 1.0f) return 0.0f;
            const glm::vec3 d = (Rinv * dir) / radii;
            const float a = glm::dot(d, d);
            const float b = glm::dot(o, d);
            const float c = glm::dot(o, o) - 1.0f;
            const float disc = b * b - a * c;
            if (disc <= 0.0f || a < 1.0e-12f) return 0.0f;
            return std::max(0.0f, (-b + std::sqrt(disc)) / a);
        };

        const float depthMax = glm::clamp(wound.radii.z * 2.0f * grow, 0.004f, 0.5f);

        auto cavity = std::make_shared<CavityGeometry>();
        std::vector<VertexData>& vs = cavity->vertices;
        std::vector<uint32_t>& ix = cavity->indices;

        constexpr size_t kMaxTriangles = 3000;
        const float cell = std::max(reachXY / 3.5f, 0.004f);   // target grid cell size on the skin

        std::vector<int> gridIndex;   // per triangle: grid point -> output vertex (or -1)

        for (const TriRef& tr : scene.tris)
        {
            const VertexData& va = scene.V(tr, 0);
            const VertexData& vb = scene.V(tr, 1);
            const VertexData& vc = scene.V(tr, 2);

            // Overlap of the triangle with the wound ellipsoid, roughly: the triangle's bounds against the ellipsoid bounds
            // were tested above; here skip triangles whose plane misses the ellipsoid entirely.
            const glm::vec3 fn = glm::cross(vb.Position - va.Position, vc.Position - va.Position);
            if (glm::length(fn) < 1.0e-10f) continue;

            const float longest = std::max(glm::distance(va.Position, vb.Position), std::max(glm::distance(vb.Position, vc.Position), glm::distance(vc.Position, va.Position)));
            const int N = glm::clamp(static_cast<int>(std::ceil(longest / cell)), 1, 64);

            const int points = (N + 1) * (N + 2) / 2;
            gridIndex.assign(static_cast<size_t>(points), -1);
            auto gp = [&](int i, int j) { return j * (N + 1) - j * (j - 1) / 2 + i; };   // (i,j), i + j <= N

            struct GridPoint { glm::vec3 position; float rho; };
            std::vector<GridPoint> pts(static_cast<size_t>(points));
            bool any = false;
            for (int j = 0; j <= N; ++j)
                for (int i = 0; i + j <= N; ++i)
                {
                    const float u = static_cast<float>(i) / N, v = static_cast<float>(j) / N;
                    GridPoint& g = pts[static_cast<size_t>(gp(i, j))];
                    g.position = va.Position * (1.0f - u - v) + vb.Position * u + vc.Position * v;
                    const glm::vec3 q = toUnit(g.position);
                    g.rho = std::sqrt(glm::dot(q, q));
                    any |= g.rho < 1.0f;
                }
            // A triangle that is entirely inside one grid cell of the ellipsoid is still caught by the cell test below, but
            // when no grid point is inside, no sub-triangle can be kept either (the grid is finer than the wound).
            if (!any) continue;

            auto emit = [&](int i, int j) -> uint32_t
            {
                int& slot = gridIndex[static_cast<size_t>(gp(i, j))];
                if (slot >= 0) return static_cast<uint32_t>(slot);

                const float u = static_cast<float>(i) / N, v = static_cast<float>(j) / N, w = 1.0f - u - v;
                const GridPoint& g = pts[static_cast<size_t>(gp(i, j))];

                glm::vec3 outward = va.Normal * w + vb.Normal * u + vc.Normal * v;
                outward = glm::length(outward) > 1.0e-6f ? glm::normalize(outward) : glm::normalize(fn);

                Bag bag;
                AddVertexInfluence(bag, va, w);
                AddVertexInfluence(bag, vb, u);
                AddVertexInfluence(bag, vc, v);
                const Influence inf = bag.Resolve();

                // Depth: the ellipsoid's depth under this point along the inward direction, never more than 60 % of the body
                // behind it (a thin part must not be dug through).
                const glm::vec3 inward = -outward;
                float depth = 0.0f;
                if (g.rho < 1.0f)
                {
                    depth = std::min(ellipsoidDepth(g.position, inward), depthMax);
                    if (depth > 0.0f)
                    {
                        const glm::vec3 faceInward = glm::dot(fn, inward) < 0.0f ? -fn : fn;
                        const float thick = scene.Distance(g.position + glm::normalize(faceInward) * 0.002f, inward);
                        depth = std::min(depth, 0.6f * thick);
                    }
                }
                depth += 0.003f;   // just under the skin so the rim never z-fights with it

                VertexData out;
                out.Position = g.position + inward * depth;
                out.Normal = outward;
                const glm::vec3 local = Rinv * (g.position - wound.center);
                out.TextureCoordinate = glm::vec2(0.5f) + glm::vec2(local.x / radii.x, local.y / radii.y) * 0.5f;
                out.Tangent = X;
                out.BiTangent = Y;
                for (int k = 0; k < 4; ++k)
                {
                    out.BlendIndices[k] = inf.bone[k];
                    out.BlendWeights[k] = inf.weight[k];
                }
                const float f = glm::clamp(g.rho, 0.0f, 1.0f);
                out.ShadowMapCoords = glm::vec2(f, glm::clamp(depth / depthMax, 0.0f, 1.0f));   // lateral fraction (1 = edge), depth fraction (read as texcoord1)
                out.Color = glm::vec4(1.0f);

                slot = static_cast<int>(vs.size());
                vs.push_back(out);
                return static_cast<uint32_t>(slot);
            };

            auto rho = [&](int i, int j) { return pts[static_cast<size_t>(gp(i, j))].rho; };
            auto keep = [&](int i0, int j0, int i1, int j1, int i2, int j2)
            {
                if (ix.size() / 3 >= kMaxTriangles) return;
                if (rho(i0, j0) >= 1.0f && rho(i1, j1) >= 1.0f && rho(i2, j2) >= 1.0f) return;
                const uint32_t a = emit(i0, j0), b = emit(i1, j1), c = emit(i2, j2);
                ix.push_back(a); ix.push_back(b); ix.push_back(c);
            };

            for (int j = 0; j < N; ++j)
                for (int i = 0; i + j < N; ++i)
                {
                    keep(i, j, i + 1, j, i, j + 1);
                    if (i + j < N - 1) keep(i + 1, j, i + 1, j + 1, i, j + 1);
                }
        }

        if (ix.empty()) return nullptr;

        // Normals from the bowl itself (smooth, facing the opening).
        std::vector<glm::vec3> acc(vs.size(), glm::vec3(0.0f));
        for (size_t i = 0; i + 2 < ix.size(); i += 3)
        {
            const glm::vec3 f = glm::cross(vs[ix[i + 1]].Position - vs[ix[i]].Position, vs[ix[i + 2]].Position - vs[ix[i]].Position);
            for (int k = 0; k < 3; ++k)
            {
                const glm::vec3 hint = vs[ix[i + k]].Normal;   // authored outside
                acc[ix[i + k]] += glm::dot(f, hint) < 0.0f ? -f : f;
            }
        }
        for (size_t i = 0; i < vs.size(); ++i)
            if (glm::length(acc[i]) > 1.0e-12f) vs[i].Normal = glm::normalize(acc[i]);

        return cavity;
    }
    }   // namespace

    std::shared_ptr<const TriangleGrid> BuildTriangleGrid(const roj::SkinnedModel& model, const Profile& profile)
    {
        auto grid = std::make_shared<TriangleGrid>();
        glm::vec3 lo(1.0e30f), hi(-1.0e30f);
        for (size_t m = 0; m < model.meshes.size(); ++m)
        {
            if (m < profile.submeshRoles.size() && profile.submeshRoles[m] == SubmeshRole::Attachment) continue;
            const roj::SkinnedMesh& mesh = model.meshes[m];
            for (size_t i = 0; i + 2 < mesh.indices.size(); i += 3)
            {
                const uint32_t i0 = mesh.indices[i], i1 = mesh.indices[i + 1], i2 = mesh.indices[i + 2];
                if (i0 >= mesh.vertices.size() || i1 >= mesh.vertices.size() || i2 >= mesh.vertices.size()) continue;
                grid->tris.push_back({ static_cast<uint16_t>(m), i0, i1, i2 });
                for (uint32_t k : { i0, i1, i2 })
                {
                    lo = glm::min(lo, mesh.vertices[k].Position);
                    hi = glm::max(hi, mesh.vertices[k].Position);
                }
            }
        }
        if (grid->tris.empty()) return grid;

        const glm::vec3 extent = hi - lo;
        grid->cell = std::max(std::max(extent.x, std::max(extent.y, extent.z)) / 40.0f, 0.02f);
        grid->origin = lo;
        grid->dims = glm::max(glm::ivec3(extent / grid->cell) + glm::ivec3(1), glm::ivec3(1));

        const size_t cells = static_cast<size_t>(grid->dims.x) * grid->dims.y * grid->dims.z;
        auto cellRange = [&](const glm::vec3& a, const glm::vec3& b, glm::ivec3& c0, glm::ivec3& c1)
        {
            c0 = glm::clamp(glm::ivec3((a - grid->origin) / grid->cell), glm::ivec3(0), grid->dims - glm::ivec3(1));
            c1 = glm::clamp(glm::ivec3((b - grid->origin) / grid->cell), glm::ivec3(0), grid->dims - glm::ivec3(1));
        };
        auto triBounds = [&](const TriangleGrid::Tri& t, glm::vec3& a, glm::vec3& b)
        {
            const roj::SkinnedMesh& mesh = model.meshes[t.mesh];
            const glm::vec3& p0 = mesh.vertices[t.i0].Position;
            const glm::vec3& p1 = mesh.vertices[t.i1].Position;
            const glm::vec3& p2 = mesh.vertices[t.i2].Position;
            a = glm::min(p0, glm::min(p1, p2));
            b = glm::max(p0, glm::max(p1, p2));
        };

        std::vector<uint32_t> counts(cells + 1, 0);
        for (const TriangleGrid::Tri& t : grid->tris)
        {
            glm::vec3 a, b; glm::ivec3 c0, c1;
            triBounds(t, a, b);
            cellRange(a, b, c0, c1);
            for (int z = c0.z; z <= c1.z; ++z)
                for (int y = c0.y; y <= c1.y; ++y)
                    for (int x = c0.x; x <= c1.x; ++x)
                        ++counts[(static_cast<size_t>(z) * grid->dims.y + y) * grid->dims.x + x + 1];
        }
        grid->cellStart.assign(cells + 1, 0);
        for (size_t c = 0; c < cells; ++c) grid->cellStart[c + 1] = grid->cellStart[c] + counts[c + 1];
        grid->cellTris.assign(grid->cellStart[cells], 0);
        std::vector<uint32_t> fill(grid->cellStart.begin(), grid->cellStart.end() - 1);
        for (uint32_t ti = 0; ti < grid->tris.size(); ++ti)
        {
            glm::vec3 a, b; glm::ivec3 c0, c1;
            triBounds(grid->tris[ti], a, b);
            cellRange(a, b, c0, c1);
            for (int z = c0.z; z <= c1.z; ++z)
                for (int y = c0.y; y <= c1.y; ++y)
                    for (int x = c0.x; x <= c1.x; ++x)
                        grid->cellTris[fill[(static_cast<size_t>(z) * grid->dims.y + y) * grid->dims.x + x]++] = ti;
        }
        return grid;
    }

    namespace
    {
        ThreadPool& CavityPool()
        {
            static ThreadPool pool("GoreCavity");
            static bool started = false;
            if (!started)
            {
                started = true;
                if (ThreadPool::Supported()) pool.Start(1);
            }
            return pool;
        }
    }

    void RequestCavity(Wound& wound, const roj::SkinnedModel& model, const Profile& profile)
    {
        wound.cavity.reset();
        wound.cavityFailed = false;
        wound.cavityTicket.reset();

        const TriangleGrid* grid = profile.triGrid.get();
        if (grid == nullptr || grid->tris.empty()) { wound.cavityFailed = true; return; }

        // Triangles around the wound, deep enough for the thickness probes.
        const float grow = 1.0f + wound.roughness + 0.1f;
        const float reachXY = std::max(wound.radii.x, wound.radii.y) * grow;
        const float depthCap = glm::clamp(wound.radii.z * 2.0f * grow, 0.004f, 0.5f);
        const float half = std::max(reachXY, wound.radii.z * grow) + depthCap + 0.01f;
        const glm::vec3 lo = wound.center - glm::vec3(half), hi = wound.center + glm::vec3(half);

        const glm::ivec3 c0 = glm::clamp(glm::ivec3((lo - grid->origin) / grid->cell), glm::ivec3(0), grid->dims - glm::ivec3(1));
        const glm::ivec3 c1 = glm::clamp(glm::ivec3((hi - grid->origin) / grid->cell), glm::ivec3(0), grid->dims - glm::ivec3(1));
        std::vector<uint32_t> ids;
        for (int z = c0.z; z <= c1.z; ++z)
            for (int y = c0.y; y <= c1.y; ++y)
                for (int x = c0.x; x <= c1.x; ++x)
                {
                    const size_t c = (static_cast<size_t>(z) * grid->dims.y + y) * grid->dims.x + x;
                    ids.insert(ids.end(), grid->cellTris.begin() + grid->cellStart[c], grid->cellTris.begin() + grid->cellStart[c + 1]);
                }
        std::sort(ids.begin(), ids.end());
        ids.erase(std::unique(ids.begin(), ids.end()), ids.end());

        std::vector<TriRef> local;
        local.reserve(ids.size());
        for (uint32_t id : ids)
        {
            const TriangleGrid::Tri& t = grid->tris[id];
            const roj::SkinnedMesh& mesh = model.meshes[t.mesh];
            const glm::vec3& a = mesh.vertices[t.i0].Position;
            const glm::vec3& b = mesh.vertices[t.i1].Position;
            const glm::vec3& c = mesh.vertices[t.i2].Position;
            const glm::vec3 tMin = glm::min(a, glm::min(b, c)), tMax = glm::max(a, glm::max(b, c));
            if (tMax.x < lo.x || tMin.x > hi.x || tMax.y < lo.y || tMin.y > hi.y || tMax.z < lo.z || tMin.z > hi.z) continue;
            TriRef r;
            r.v[0] = mesh.vertices[t.i0];
            r.v[1] = mesh.vertices[t.i1];
            r.v[2] = mesh.vertices[t.i2];
            local.push_back(r);
        }
        if (local.empty()) { wound.cavityFailed = true; return; }

        auto ticket = std::make_shared<CavityTicket>();
        wound.cavityTicket = ticket;

        Wound copy;   // only the shape is needed by the worker
        copy.center = wound.center;
        copy.rotation = wound.rotation;
        copy.radii = wound.radii;
        copy.kind = wound.kind;
        copy.roughness = wound.roughness;

        CavityPool().QueueJob([copy, ticket, tris = std::move(local)]() mutable
        {
            ticket->result = BuildCavityFromTriangles(copy, std::move(tris));
            ticket->state.store(1, std::memory_order_release);
        });
    }

    bool PollCavity(Wound& wound)
    {
        if (!wound.cavityTicket || wound.cavityTicket->state.load(std::memory_order_acquire) == 0) return false;
        wound.cavity = wound.cavityTicket->result;
        wound.cavityFailed = !wound.cavity;
        wound.cavityTicket.reset();
        return true;
    }
}
