#include "GoreCavity.h"

#include <ThreadPool.h>
#include <algorithm>
#include <cmath>
#include <unordered_map>

#include "GoreFlesh.h"
#include "GoreGpu.h"
#include "GoreProfile.h"
#include "GoreTriGrid.h"

namespace Gore
{
    namespace
    {
        constexpr float kPi = 3.14159265358979f;
        constexpr float kSearch = 0.45f;       // metres: a point farther than this from every face of the body is outside it
        constexpr float kMargin = 0.0005f;     // the surface ends this far under the skin: it meets the skin, so no strip of the garment's inside shows between them
        constexpr float kDepthRange = 0.07f;   // depth under the skin at which the wall is fully "deep"
        constexpr int   kBisect = 7;           // steps that place the rim of the surface on the skin (0.8 % of an edge)
        constexpr float kEdgeLength = 0.015f;  // metres between the samples of the ellipsoid

        // The bone weights of a point on a body triangle: the weights of its corners, mixed by the barycentric coordinates, the
        // four strongest kept.
        void TakeWeights(VertexData& out, const GridTri& tri, const glm::vec3& bary)
        {
            float bone[12], weight[12];
            int n = 0;
            for (int k = 0; k < 3; ++k)
            {
                for (int j = 0; j < 4; ++j)
                {
                    const float wt = tri.weight[k][j] * bary[k];
                    if (wt <= 0.0f) continue;
                    int at = -1;
                    for (int i = 0; i < n; ++i)
                        if (bone[i] == tri.bone[k][j]) at = i;
                    if (at < 0) { bone[n] = tri.bone[k][j]; weight[n] = wt; ++n; }
                    else weight[at] += wt;
                }
            }
            for (int i = 1; i < n; ++i)   // strongest first
                for (int j = i; j > 0 && weight[j] > weight[j - 1]; --j)
                {
                    std::swap(weight[j], weight[j - 1]);
                    std::swap(bone[j], bone[j - 1]);
                }

            out.BlendIndices[0] = out.BlendIndices[1] = out.BlendIndices[2] = out.BlendIndices[3] = 0.0f;
            out.BlendWeights = glm::vec4(0.0f);
            float sum = 0.0f;
            for (int i = 0; i < std::min(n, 4); ++i) sum += weight[i];
            if (sum <= 0.0f) { out.BlendWeights = glm::vec4(1, 0, 0, 0); return; }
            for (int i = 0; i < std::min(n, 4); ++i)
            {
                out.BlendIndices[i] = bone[i];
                out.BlendWeights[i] = weight[i] / sum;
            }
        }

        // How much of their skinning two vertices share (0 = no bone in common, 1 = the same weights). The wall of a cavity is
        // generated in the bind pose, where an arm lies against the trunk or the legs touch; a triangle between vertices that share
        // no bone would be stretched by every movement of the body into a slab that flies between the two parts, outside the mesh.
        float SharedSkinning(const VertexData& a, const VertexData& b)
        {
            float shared = 0.0f;
            for (int i = 0; i < 4; ++i)
                for (int j = 0; j < 4; ++j)
                    if (a.BlendWeights[i] > 0.0f && a.BlendIndices[i] == b.BlendIndices[j])
                        shared += std::min(a.BlendWeights[i], b.BlendWeights[j]);
            return shared;
        }
        constexpr float kMinSharedSkinning = 0.2f;   // a triangle needs at least this much skinning in common between each two of its corners

        // One wound as an ellipsoid in bind space.
        struct Shape
        {
            glm::vec3 center;
            glm::mat3 axes;
            glm::vec3 radii;

            glm::vec3 Point(const glm::vec3& u) const { return center + axes * (u * radii); }
            glm::vec3 Inward(const glm::vec3& u) const { return -glm::normalize(axes * (u / glm::max(radii, glm::vec3(1.0e-4f)))); }
        };

        // Where the ellipsoid of `w` lies inside the body, as triangles (appended to `out`).
        void BuildWound(CavityGeometry& out, const TriGrid& body, const Wound& w, int index)
        {
            // The skin hole has a ragged edge that reaches up to `roughness` (of a radius) beyond the nominal ellipsoid. The surface
            // is made that much larger, so it lies behind every part of the hole; where the hole is smaller the skin covers it.
            const glm::vec3 radii = w.radii * (1.0f + w.roughness);
            const Shape shape{ w.center, glm::mat3_cast(w.rotation), radii };

            const float mean = (radii.x + radii.y + radii.z) / 3.0f;
            const int cols = glm::clamp(static_cast<int>(std::ceil(2.0f * kPi * mean / kEdgeLength)), 12, 64);
            const int rings = std::max(6, cols / 2);
            const int total = (rings + 1) * cols;

            // The samples: how deep each lies inside the body (> 0 = inside, less the margin).
            std::vector<glm::vec3> unit(total);
            std::vector<float> depth(total);
            auto inside = [&](const glm::vec3& u)
            {
                const BodyQuery q = body.Closest(shape.Point(u), kSearch);
                return q.found ? q.signedDistance - kMargin : -kSearch;
            };
            bool any = false;
            for (int r = 0; r <= rings; ++r)
            {
                const float phi = kPi * static_cast<float>(r) / rings;
                for (int c = 0; c < cols; ++c)
                {
                    const float theta = 2.0f * kPi * static_cast<float>(c) / cols;
                    const int i = r * cols + c;
                    unit[i] = glm::vec3(std::sin(phi) * std::cos(theta), std::cos(phi), std::sin(phi) * std::sin(theta));
                    depth[i] = inside(unit[i]);
                    any = any || depth[i] > 0.0f;
                }
            }
            if (!any) return;   // the ellipsoid is nowhere inside the body

            auto makeVertex = [&](const glm::vec3& u) -> uint32_t
            {
                VertexData v;
                v.Position = shape.Point(u);
                v.Normal = shape.Inward(u);   // towards the hole: the side that is seen
                const BodyQuery q = body.Closest(v.Position, kSearch);
                if (q.found) TakeWeights(v, body.Tri(q.tri), q.bary);
                else v.BlendWeights = glm::vec4(1, 0, 0, 0);
                const float deep = q.found ? glm::clamp(q.signedDistance / kDepthRange, 0.0f, 1.0f) : 0.0f;
                v.Color = glm::vec4(static_cast<float>(index), deep, 0.0f, 1.0f);
                v.ShadowMapCoords = glm::vec2(1.0f, 0.0f);
                out.vertices.push_back(v);
                return static_cast<uint32_t>(out.vertices.size() - 1);
            };

            // A sample inside the body becomes one vertex, shared by its triangles; where an edge leaves the body a vertex is placed
            // on the ellipsoid, on the skin (bisection along the arc), shared by the two triangles at that edge.
            std::vector<int64_t> sampleVertex(total, -1);
            std::unordered_map<uint64_t, uint32_t> edgeVertex;
            auto sample = [&](int i) -> uint32_t
            {
                if (sampleVertex[i] < 0) sampleVertex[i] = makeVertex(unit[i]);
                return static_cast<uint32_t>(sampleVertex[i]);
            };
            auto crossing = [&](int i, int j) -> uint32_t
            {
                const int a = std::min(i, j), b = std::max(i, j);
                const uint64_t key = static_cast<uint64_t>(a) * static_cast<uint64_t>(total) + static_cast<uint64_t>(b);
                const auto it = edgeVertex.find(key);
                if (it != edgeVertex.end()) return it->second;

                float tIn = depth[a] > 0.0f ? 0.0f : 1.0f, tOut = 1.0f - tIn;
                for (int k = 0; k < kBisect; ++k)
                {
                    const float tm = 0.5f * (tIn + tOut);
                    if (inside(glm::normalize(glm::mix(unit[a], unit[b], tm))) > 0.0f) tIn = tm; else tOut = tm;
                }
                const uint32_t v = makeVertex(glm::normalize(glm::mix(unit[a], unit[b], tIn)));
                edgeVertex[key] = v;
                return v;
            };

            auto triangle = [&](int i0, int i1, int i2)
            {
                const int ids[3] = { i0, i1, i2 };
                uint32_t polygon[4];
                int n = 0;
                for (int e = 0; e < 3; ++e)
                {
                    const int cur = ids[e], next = ids[(e + 1) % 3];
                    const bool curIn = depth[cur] > 0.0f, nextIn = depth[next] > 0.0f;
                    if (curIn) polygon[n++] = sample(cur);
                    if (curIn != nextIn) polygon[n++] = crossing(cur, next);
                }
                for (int k = 1; k + 1 < n; ++k)
                {
                    const VertexData& v0 = out.vertices[polygon[0]];
                    const VertexData& v1 = out.vertices[polygon[k]];
                    const VertexData& v2 = out.vertices[polygon[k + 1]];
                    if (SharedSkinning(v0, v1) < kMinSharedSkinning || SharedSkinning(v1, v2) < kMinSharedSkinning ||
                        SharedSkinning(v0, v2) < kMinSharedSkinning)
                        continue;   // joins two parts that move apart (see SharedSkinning)
                    out.indices.insert(out.indices.end(), { polygon[0], polygon[k], polygon[k + 1] });
                }
            };

            for (int r = 0; r < rings; ++r)
            {
                for (int c = 0; c < cols; ++c)
                {
                    const int a = r * cols + c, b = r * cols + (c + 1) % cols;
                    const int d = (r + 1) * cols + c, e = (r + 1) * cols + (c + 1) % cols;
                    if (r > 0) triangle(a, d, b);            // at the first ring a and b are the same point
                    if (r + 1 < rings) triangle(b, d, e);    // at the last ring d and e are
                }
            }
        }
    }

    void CavityGeometry::EnsureGpu() const
    {
        if (gpuReady) return;
        gpuReady = true;
        if (vertices.empty() || indices.empty()) return;
        vbh = bgfx::createVertexBuffer(bgfx::copy(vertices.data(), static_cast<uint32_t>(sizeof(VertexData) * vertices.size())),
                                       VertexData::Declaration());
        ibh = bgfx::createIndexBuffer(bgfx::copy(indices.data(), static_cast<uint32_t>(sizeof(uint32_t) * indices.size())),
                                      BGFX_BUFFER_INDEX32);
    }

    CavityGeometry::~CavityGeometry()
    {
        DestroyBuffersOnMainThread(vbh, ibh);   // not here: this may run on the game thread, the worker or a PreFinalize pool thread
    }

    bool HasCavity(const Wound& wound)
    {
        return wound.limitSkel < 0 && wound.healDuration <= 0.0f;
    }

    bool SameWounds(const std::array<Wound, kMaxWounds>& a, int countA, const std::array<Wound, kMaxWounds>& b, int countB)
    {
        if (countA != countB) return false;
        for (int i = 0; i < countA; ++i)
        {
            const Wound& x = a[i];
            const Wound& y = b[i];
            if (x.center != y.center || x.rotation != y.rotation || x.radii != y.radii || x.limitSkel != y.limitSkel
                || x.healDuration != y.healDuration || x.kind != y.kind)
                return false;
        }
        return true;
    }

    std::shared_ptr<const CavityGeometry> RequestCavity(const Profile& profile, const std::array<Wound, kMaxWounds>& wounds, int count)
    {
        if (!profile.flesh || !profile.flesh->body) return nullptr;
        bool any = false;
        for (int i = 0; i < count; ++i) any = any || HasCavity(wounds[i]);
        if (!any) return nullptr;

        auto cavity = std::make_shared<CavityGeometry>();
        cavity->wounds = wounds;
        cavity->woundCount = static_cast<uint8_t>(count);

        // The job holds the body grid (immutable, with its own bone weights) and the wounds: nothing of the model, which the game
        // thread may release while the worker runs.
        const std::shared_ptr<const TriGrid> body = profile.flesh->body;
        BuildPool().QueueJob([cavity, body]()
        {
            for (int i = 0; i < cavity->woundCount; ++i)
                if (HasCavity(cavity->wounds[i]))
                    BuildWound(*cavity, *body, cavity->wounds[i], i);
            cavity->state.store(CavityGeometry::State::Ready, std::memory_order_release);
        });
        return cavity;
    }
}
