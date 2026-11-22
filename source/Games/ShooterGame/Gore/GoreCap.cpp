#include "GoreCap.h"

#include <ThreadPool.h>
#include <algorithm>
#include <map>
#include <tuple>
#include "GoreFlesh.h"
#include "GoreGpu.h"
#include "GoreProfile.h"

#include <cmath>

namespace Gore
{
    namespace
    {
        struct Crossing
        {
            glm::vec3 p = glm::vec3(0);
            const VertexData* weights = nullptr;   // the model vertex whose bone weights this point takes
        };

        struct Segment
        {
            Crossing a, b;
            int layer = 0;   // 0 skin, 1..3 shells, 4 bone, 6 organ (a gut mass)
            int ring = 0;    // segments that share end points form a ring
            glm::vec3 fan = glm::vec3(0.0f);   // the centre its disc is drawn from
        };

        // Where the plane dot(n, p) = w crosses a triangle: two points on its edges. False when it does not cross.
        bool SliceTriangle(const glm::vec3* p, const VertexData* const* src, const glm::vec3& n, float w, Crossing* out)
        {
            const float d[3] = { glm::dot(n, p[0]) - w, glm::dot(n, p[1]) - w, glm::dot(n, p[2]) - w };
            int count = 0;
            for (int k = 0; k < 3 && count < 2; ++k)
            {
                const int j = (k + 1) % 3;
                if ((d[k] > 0.0f) == (d[j] > 0.0f)) continue;      // this edge stays on one side
                const float t = d[k] / (d[k] - d[j]);               // never 0 / 0: the two signs differ
                out[count].p = glm::mix(p[k], p[j], t);
                out[count].weights = t < 0.5f ? src[k] : src[j];
                ++count;
            }
            return count == 2;
        }

        // Look of each disc, outside to inside. tint multiplies the texture; material 0 = flesh texture, 1 = bone texture,
        // 2 = gut texture; shrink scales the disc around its centre (the marrow is the bone outline at 55 %).
        struct Look { glm::vec3 tint; float material; float shrink; };
        const Look kLooks[7] =
        {
            { glm::vec3(1.25f, 0.95f, 0.80f), 0.0f, 1.00f },   // 0 skin outline: pale skin and fat ring
            { glm::vec3(1.05f, 0.60f, 0.55f), 0.0f, 1.00f },   // 1 shell 1: light muscle
            { glm::vec3(0.90f, 0.42f, 0.40f), 0.0f, 1.00f },   // 2 shell 2: muscle
            { glm::vec3(0.62f, 0.28f, 0.28f), 0.0f, 1.00f },   // 3 shell 3: deep muscle
            { glm::vec3(1.00f, 0.94f, 0.84f), 1.0f, 1.00f },   // 4 bone
            { glm::vec3(0.45f, 0.13f, 0.10f), 1.0f, 0.55f },   // 5 marrow
            { glm::vec3(1.20f, 0.82f, 0.78f), 2.0f, 1.00f },   // 6 organ: pale pink gut wall against the dark muscle
        };
        constexpr float kLift = 0.0005f;   // metres between two discs
    }

    void CapGeometry::EnsureGpu() const
    {
        if (gpuReady) return;
        gpuReady = true;
        if (vertices.empty() || indices.empty()) return;
        vbh = bgfx::createVertexBuffer(bgfx::copy(vertices.data(), static_cast<uint32_t>(sizeof(VertexData) * vertices.size())),
                                       VertexData::Declaration());
        ibh = bgfx::createIndexBuffer(bgfx::copy(indices.data(), static_cast<uint32_t>(sizeof(uint32_t) * indices.size())),
                                      BGFX_BUFFER_INDEX32);
    }

    CapGeometry::~CapGeometry()
    {
        DestroyBuffersOnMainThread(vbh, ibh);   // not here: this may run on the game thread or the worker
    }

    namespace
    {
    // Worker thread. False when the plane does not cross the skin near the cut centre.
    bool FillCap(CapGeometry& cap, const roj::SkinnedModel& model, const Profile& profile, const Cut& cut)
    {
        if (!profile.flesh) return false;
        const ModelFlesh& flesh = *profile.flesh;

        const size_t boneCount = profile.subtreeEnd.size();
        const uint16_t S = cut.boneSkel;
        if (S >= boneCount || profile.parent.size() != boneCount) return false;

        const glm::vec3 n = glm::normalize(glm::vec3(cut.plane));
        const float w = cut.plane.w;
        const glm::vec3 centre = cut.center;
        const float limit = 1.3f * cut.outlineRadius + 0.01f;   // ignore anything the plane crosses far from the cut

        // Triangles that can cross the plane near the cut: the cut bone, everything below it, and its parent
        // (vertices near the cut may be weighted to the parent bone).
        std::vector<uint16_t> bones;
        // Triangles are stored under the common ancestor of the bones they touch, so a triangle that spans the cut bone and a
        // neighbour (shoulder, hip) sits in an ancestor's range: take the whole chain up to the root.
        for (uint16_t a = profile.parent[S]; a < boneCount && bones.size() < boneCount; a = profile.parent[a])   // a root's parent is INVALID_BONE_IDX (large)
            bones.push_back(a);
        for (uint16_t b = S; b < profile.subtreeEnd[S]; ++b) bones.push_back(b);

        std::vector<Segment> segments;
        auto add = [&](const glm::vec3* p, const VertexData* const* src, int layer)
        {
            Crossing c[2];
            if (!SliceTriangle(p, src, n, w, c)) return false;
            if (glm::distance(c[0].p, centre) > limit || glm::distance(c[1].p, centre) > limit) return false;
            if (glm::distance(c[0].p, c[1].p) < 1.0e-5f) return false;
            Segment s;
            s.a = c[0];
            s.b = c[1];
            s.layer = layer;
            segments.push_back(s);
            return true;
        };

        int skinSegments = 0;
        for (size_t m = 0; m < model.meshes.size(); ++m)
        {
            const SubmeshRole role = m < profile.submeshRoles.size() ? profile.submeshRoles[m] : SubmeshRole::Volume;
            if (role != SubmeshRole::Volume) continue;      // cloth is a thin sheet: clipped by the plane, no cross-section

            const roj::SkinnedMesh& mesh = model.meshes[m];
            if (mesh.boneIndexFirst.size() != boneCount + 1) continue;

            const std::vector<VertexData>* shell = nullptr;
            if (m < flesh.shellVertices.size() && flesh.shellVertices[m].size() == mesh.vertices.size())
                shell = &flesh.shellVertices[m];

            for (uint16_t b : bones)
            {
                const uint32_t end = mesh.boneIndexFirst[b + 1];
                for (uint32_t i = mesh.boneIndexFirst[b]; i + 2 < end; i += 3)
                {
                    const uint32_t id[3] = { mesh.indices[i], mesh.indices[i + 1], mesh.indices[i + 2] };
                    const VertexData* src[3] = { &mesh.vertices[id[0]], &mesh.vertices[id[1]], &mesh.vertices[id[2]] };

                    glm::vec3 p[3] = { src[0]->Position, src[1]->Position, src[2]->Position };
                    if (add(p, src, 0)) ++skinSegments;

                    if (shell == nullptr) continue;
                    const VertexData* sv[3] = { &(*shell)[id[0]], &(*shell)[id[1]], &(*shell)[id[2]] };
                    if (sv[0]->ShadowMapCoords.x < 0.004f || sv[1]->ShadowMapCoords.x < 0.004f || sv[2]->ShadowMapCoords.x < 0.004f)
                        continue;                               // no flesh under this triangle (same rule as the shell shader)

                    for (int k = 0; k < kShellLayers; ++k)
                    {
                        for (int v = 0; v < 3; ++v)             // the same move the shell vertex shader does
                            p[v] = sv[v]->Position + sv[v]->Normal * (sv[v]->ShadowMapCoords.x * kShellFraction[k]);
                        add(p, src, k + 1);
                    }
                }
            }
        }
        if (skinSegments < 3) return false;

        // The bone inside the cut bone, and the organs (gut masses) of the cut bone and its ancestors: a waist cut goes through
        // the belly, whose guts hang on the pelvis and the lowest spine bone.
        {
            std::vector<uint8_t> onChain(boneCount, 0);
            onChain[S] = 1;
            for (uint16_t a = profile.parent[S]; a < boneCount; a = profile.parent[a]) onChain[a] = 1;

            for (const FleshPiece& piece : flesh.pieces)
            {
                if (piece.skel >= boneCount || !onChain[piece.skel]) continue;
                for (uint32_t i = piece.first; i + 2 < piece.first + piece.count; i += 3)
                {
                    const VertexData* src[3] = { &flesh.anatomyVertices[flesh.anatomyIndices[i]],
                                                 &flesh.anatomyVertices[flesh.anatomyIndices[i + 1]],
                                                 &flesh.anatomyVertices[flesh.anatomyIndices[i + 2]] };
                    const bool organ = src[0]->Color.r > 0.5f;   // anatomy material: 0 bone, 1 guts
                    if (!organ && piece.skel != S) continue;     // only the cut bone's own bone is cut
                    const glm::vec3 p[3] = { src[0]->Position, src[1]->Position, src[2]->Position };
                    add(p, src, organ ? 6 : 4);
                }
            }
        }

        // The plane also crosses other parts near the cut (shoulders under a collar, neighbouring bones). Segments that share end
        // points form rings; per layer only the ring nearest to the cut centre is the cross-section (organs: every ring, each a
        // separate mass).
        {
            std::vector<int> parent(segments.size() * 2);
            for (size_t i = 0; i < parent.size(); ++i) parent[i] = static_cast<int>(i);
            auto find = [&](int x) { while (parent[x] != x) x = parent[x] = parent[parent[x]]; return x; };
            std::map<std::tuple<long, long, long>, int> seen;    // end point (0.2 mm grid) -> first end point index there
            auto key = [](const glm::vec3& p) { return std::make_tuple(std::lround(p.x * 5000.0f), std::lround(p.y * 5000.0f), std::lround(p.z * 5000.0f)); };
            for (size_t i = 0; i < segments.size(); ++i)
            {
                parent[find(static_cast<int>(i) * 2)] = find(static_cast<int>(i) * 2 + 1);
                for (int e = 0; e < 2; ++e)
                {
                    const auto k = key(e == 0 ? segments[i].a.p : segments[i].b.p);
                    auto it = seen.find(k);
                    if (it == seen.end()) seen[k] = static_cast<int>(i) * 2 + e;
                    else parent[find(static_cast<int>(i) * 2 + e)] = find(it->second);
                }
            }

            // Per layer and ring: the mean end point distance from the centre.
            std::map<std::pair<int, int>, std::pair<float, int>> ringSum;   // (layer, ring) -> (sum, count)
            for (size_t i = 0; i < segments.size(); ++i)
            {
                auto& r = ringSum[{ segments[i].layer, find(static_cast<int>(i) * 2) }];
                r.first += glm::distance(segments[i].a.p, centre) + glm::distance(segments[i].b.p, centre);
                r.second += 2;
            }
            std::map<int, std::pair<int, float>> nearest;                   // layer -> (ring, mean distance)
            for (const auto& kv : ringSum)
            {
                const float mean = kv.second.first / static_cast<float>(kv.second.second);
                auto it = nearest.find(kv.first.first);
                if (it == nearest.end() || mean < it->second.second) nearest[kv.first.first] = { kv.first.second, mean };
            }
            // A shell lies inside the skin: a shell ring that is wider than the skin ring belongs to something else (a suit's collar).
            const bool haveSkin = nearest.count(0) > 0;
            const float skinMean = haveSkin ? nearest[0].second : 0.0f;
            std::vector<Segment> kept;
            for (size_t i = 0; i < segments.size(); ++i)
            {
                const Segment& sg = segments[i];
                if (sg.layer > 0 && sg.layer < 4 && (!haveSkin || nearest[sg.layer].second > skinMean * 1.02f)) continue;
                const int ring = find(static_cast<int>(i) * 2);
                if (sg.layer != 6 && nearest[sg.layer].first != ring) continue;
                kept.push_back(sg);
                kept.back().ring = ring;
            }
            segments.swap(kept);
        }

        // Each disc is drawn as a fan from one point: the cut centre for skin and flesh, the middle of its own ring for bone
        // and organs (the bone need not sit on the cut centre, and every organ mass is a separate ring).
        {
            std::map<std::pair<int, int>, std::pair<glm::vec3, int>> sums;   // (layer, ring) -> (sum of end points, count)
            for (const Segment& sg : segments)
            {
                auto& s = sums[{ sg.layer, sg.ring }];
                s.first += sg.a.p + sg.b.p;
                s.second += 2;
            }
            for (Segment& sg : segments)
            {
                if (sg.layer == 4 || sg.layer == 6)
                {
                    const auto& s = sums[{ sg.layer, sg.ring }];
                    sg.fan = s.first / static_cast<float>(s.second);
                }
                else
                {
                    sg.fan = centre;
                }
            }
        }

        // Texture axes on the plane.
        const glm::vec3 upRef = std::abs(n.y) < 0.99f ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);
        const glm::vec3 T = glm::normalize(glm::cross(n, upRef));
        const glm::vec3 B = glm::cross(n, T);

        cap.vertices.reserve(segments.size() * 8);
        cap.indices.reserve(segments.size() * 8);

        auto vertex = [&](const glm::vec3& pos, const glm::vec3& normal, const Look& look, const VertexData* weights)
        {
            VertexData v;
            v.Position = pos;
            v.Normal = normal;
            const glm::vec3 r = pos - centre;
            v.TextureCoordinate = glm::vec2(glm::dot(r, T), glm::dot(r, B)) * 10.0f;   // 10 texture repeats per metre
            if (weights != nullptr)
            {
                for (int k = 0; k < 4; ++k) v.BlendIndices[k] = weights->BlendIndices[k];
                v.BlendWeights = weights->BlendWeights;
            }
            else
            {
                v.BlendIndices[0] = static_cast<float>(cut.boneSkin);
                v.BlendWeights = glm::vec4(1, 0, 0, 0);
            }
            v.Color = glm::vec4(look.tint, 1.0f);
            v.ShadowMapCoords = glm::vec2(1.0f, look.material);
            cap.indices.push_back(static_cast<uint32_t>(cap.vertices.size()));
            cap.vertices.push_back(v);
        };

        // sign +1: seen from the removed side (the body's cap). sign -1: seen from the kept side (the gib's cap).
        auto emit = [&](float sign)
        {
            const glm::vec3 normal = n * sign;
            for (const Segment& s : segments)
            {
                const int discs = s.layer == 4 ? 2 : 1;        // a bone segment makes the bone disc and the marrow disc
                for (int k = 0; k < discs; ++k)
                {
                    const int lookIndex = s.layer + k;
                    const Look& look = kLooks[lookIndex];
                    const glm::vec3& c = s.fan;
                    const glm::vec3 lift = normal * (kLift * static_cast<float>(lookIndex));   // deeper discs lie nearer the viewer
                    vertex(c + lift, normal, look, nullptr);
                    vertex(c + (s.a.p - c) * look.shrink + lift, normal, look, s.a.weights);
                    vertex(c + (s.b.p - c) * look.shrink + lift, normal, look, s.b.weights);
                }
            }
        };

        cap.bodyFirst = 0;
        emit(1.0f);
        cap.bodyCount = static_cast<uint32_t>(cap.indices.size());
        cap.gibFirst = cap.bodyCount;
        emit(-1.0f);
        cap.gibCount = static_cast<uint32_t>(cap.indices.size()) - cap.gibFirst;

        return true;
    }
    }

    std::shared_ptr<const CapGeometry> RequestCap(const roj::SkinnedModel& model, const Profile& profile, const Cut& cut)
    {
        auto cap = std::make_shared<CapGeometry>();
        Cut copy = cut;
        copy.cap.reset();
        const roj::SkinnedModel* modelPtr = &model;
        const Profile* profilePtr = &profile;
        BuildPool().QueueJob([cap, copy, modelPtr, profilePtr]()
        {
            const bool ok = FillCap(*cap, *modelPtr, *profilePtr, copy);
            cap->state.store(ok ? CapGeometry::State::Ready : CapGeometry::State::Failed, std::memory_order_release);
        });
        return cap;
    }
}
