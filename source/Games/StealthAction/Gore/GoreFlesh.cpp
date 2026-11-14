#include "GoreFlesh.h"

#include <chrono>
#include "GoreProfile.h"
#include "GoreTriGrid.h"

#include <Logger.hpp>

#include <algorithm>
#include <cmath>
#include <unordered_map>

namespace Gore
{
    namespace
    {
        constexpr float kPi = 3.14159265358979f;

        // ---- shells ----------------------------------------------------------------------------------------------------

        uint64_t WeldKey(const glm::vec3& p)
        {
            // 0.5 mm cells; 21 bits per axis (+-524 m).
            const auto q = [](float v) { return static_cast<uint64_t>(static_cast<int64_t>(std::llround(v * 2000.0f)) & 0x1FFFFF); };
            return q(p.x) | (q(p.y) << 21) | (q(p.z) << 42);
        }

        void BuildShells(const roj::SkinnedModel& model, const Profile& profile, TriGrid& grid, ModelFlesh& out)
        {
            const size_t meshCount = model.meshes.size();
            out.shellVertices.assign(meshCount, {});

            auto isVolume = [&](size_t m)
            {
                const SubmeshRole role = m < profile.submeshRoles.size() ? profile.submeshRoles[m] : SubmeshRole::Volume;
                return role == SubmeshRole::Volume;
            };

            // 1. Weld by position: vertices split by uv seams or hard edges must move together, or the shell cracks open.
            std::unordered_map<uint64_t, uint32_t> weld;
            std::vector<glm::vec3> position, normalSum;
            std::vector<std::vector<uint32_t>> weldId(meshCount);
            for (size_t m = 0; m < meshCount; ++m)
            {
                if (!isVolume(m)) continue;
                const roj::SkinnedMesh& mesh = model.meshes[m];
                weldId[m].resize(mesh.vertices.size());
                for (size_t v = 0; v < mesh.vertices.size(); ++v)
                {
                    const VertexData& vd = mesh.vertices[v];
                    const auto it = weld.emplace(WeldKey(vd.Position), static_cast<uint32_t>(position.size()));
                    if (it.second) { position.push_back(vd.Position); normalSum.push_back(glm::vec3(0)); }
                    weldId[m][v] = it.first->second;
                    normalSum[it.first->second] += vd.Normal;
                }
            }

            // 2. Inward direction and max inset per welded point: 40 % of the body thickness behind it, at most 7 cm.
            //    A sheet thinner than 2 cm has no flesh (max inset 0: the shell shader discards it).
            const size_t n = position.size();
            std::vector<glm::vec3> inward(n, glm::vec3(0));
            std::vector<float> inset(n, 0.0f);
            for (size_t i = 0; i < n; ++i)
            {
                const float len = glm::length(normalSum[i]);
                if (len < 1.0e-5f) continue;
                inward[i] = -normalSum[i] / len;
                const float t = grid.Cast(position[i] + inward[i] * 0.004f, inward[i], 0.6f);
                const float thickness = t < 0.0f ? 0.3f : t + 0.004f;
                inset[i] = thickness < 0.02f ? 0.0f : std::min(0.4f * thickness, 0.07f);
            }

            // 3. No spikes: a point never goes much deeper than its shallowest neighbour (one pass, from a copy).
            {
                std::vector<float> neighbourMin(n, 1.0e9f);
                for (size_t m = 0; m < meshCount; ++m)
                {
                    if (!isVolume(m)) continue;
                    const roj::SkinnedMesh& mesh = model.meshes[m];
                    for (size_t i = 0; i + 2 < mesh.indices.size(); i += 3)
                    {
                        const uint32_t id[3] = { weldId[m][mesh.indices[i]], weldId[m][mesh.indices[i + 1]], weldId[m][mesh.indices[i + 2]] };
                        for (int a = 0; a < 3; ++a)
                            for (int b = 0; b < 3; ++b)
                                if (id[a] != id[b]) neighbourMin[id[a]] = std::min(neighbourMin[id[a]], inset[id[b]]);
                    }
                }
                for (size_t i = 0; i < n; ++i)
                    if (neighbourMin[i] < 1.0e8f) inset[i] = std::min(inset[i], neighbourMin[i] * 1.5f + 0.004f);
            }

            // 4. Shell vertices.
            for (size_t m = 0; m < meshCount; ++m)
            {
                if (!isVolume(m)) continue;
                const roj::SkinnedMesh& mesh = model.meshes[m];
                std::vector<VertexData>& dst = out.shellVertices[m];
                dst = mesh.vertices;
                for (size_t v = 0; v < dst.size(); ++v)
                {
                    const uint32_t id = weldId[m][v];
                    dst[v].Tangent = mesh.vertices[v].Normal;          // the real normal (for lighting)
                    dst[v].Normal = inward[id];                        // the direction the shell moves in
                    dst[v].ShadowMapCoords = glm::vec2(inset[id], 0.0f);
                }
            }
        }

        // ---- anatomy ---------------------------------------------------------------------------------------------------

        struct Builder
        {
            std::vector<VertexData>& v;
            std::vector<uint32_t>& ix;
            std::vector<FleshPiece>& pieces;
            uint32_t pieceFirstIndex = 0, pieceFirstVertex = 0;

            void Begin() { pieceFirstIndex = static_cast<uint32_t>(ix.size()); pieceFirstVertex = static_cast<uint32_t>(v.size()); }

            void End(uint16_t skel)
            {
                if (ix.size() == pieceFirstIndex) return;
                FleshPiece p;
                p.first = pieceFirstIndex;
                p.count = static_cast<uint32_t>(ix.size()) - pieceFirstIndex;
                p.skel = skel;
                p.bbMin = glm::vec3(1.0e9f);
                p.bbMax = glm::vec3(-1.0e9f);
                for (size_t i = pieceFirstVertex; i < v.size(); ++i)
                {
                    p.bbMin = glm::min(p.bbMin, v[i].Position);
                    p.bbMax = glm::max(p.bbMax, v[i].Position);
                }
                pieces.push_back(p);
            }

            uint32_t Vert(const glm::vec3& pos, const glm::vec3& normal, const glm::vec2& uv, int skin, float material, float shade)
            {
                VertexData d;
                d.Position = pos;
                d.Normal = normal;
                d.TextureCoordinate = uv;
                d.BlendIndices[0] = static_cast<float>(skin);
                d.BlendWeights = glm::vec4(1, 0, 0, 0);
                d.Color = glm::vec4(material, shade, 0.0f, 1.0f);
                d.ShadowMapCoords = glm::vec2(1.0f, 0.0f);
                v.push_back(d);
                return static_cast<uint32_t>(v.size() - 1);
            }

            // A tube along `points` with a radius per point. closed = the last point joins the first (a ring).
            // `side` is any vector not parallel to the tube (the rings are built around the tube with it).
            void Tube(const std::vector<glm::vec3>& points, const std::vector<float>& radii, int sides, bool closed,
                      const glm::vec3& side, int skin, float material, float shade, float uvPerMetre)
            {
                const int count = static_cast<int>(points.size());
                if (count < 2) return;
                const uint32_t base = static_cast<uint32_t>(v.size());
                float along = 0.0f;
                for (int i = 0; i < count; ++i)
                {
                    const glm::vec3& prev = points[closed ? (i + count - 1) % count : std::max(i - 1, 0)];
                    const glm::vec3& next = points[closed ? (i + 1) % count : std::min(i + 1, count - 1)];
                    glm::vec3 tangent = next - prev;
                    tangent = glm::length(tangent) > 1.0e-6f ? glm::normalize(tangent) : glm::vec3(0, 1, 0);
                    glm::vec3 x = glm::cross(side, tangent);
                    if (glm::length(x) < 1.0e-4f) x = glm::cross(glm::vec3(0.31f, 0.52f, 0.79f), tangent);
                    x = glm::normalize(x);
                    const glm::vec3 y = glm::cross(tangent, x);
                    if (i > 0) along += glm::distance(points[i], points[i - 1]);
                    for (int s = 0; s <= sides; ++s)
                    {
                        const float a = 2.0f * kPi * static_cast<float>(s) / sides;
                        const glm::vec3 nrm = x * std::cos(a) + y * std::sin(a);
                        Vert(points[i] + nrm * radii[i], nrm, glm::vec2(static_cast<float>(s) / sides, along * uvPerMetre), skin, material, shade);
                    }
                }
                const int ringStride = sides + 1;
                const int segments = closed ? count : count - 1;
                for (int i = 0; i < segments; ++i)
                {
                    const int j = (i + 1) % count;
                    for (int s = 0; s < sides; ++s)
                    {
                        const uint32_t a = base + i * ringStride + s, b = a + 1;
                        const uint32_t c = base + j * ringStride + s, d = c + 1;
                        ix.insert(ix.end(), { a, c, b, b, c, d });
                    }
                }
                if (!closed)   // end caps: a fan around each end point
                {
                    for (int end = 0; end < 2; ++end)
                    {
                        const int i = end == 0 ? 0 : count - 1;
                        const glm::vec3 dir = glm::normalize(end == 0 ? points[0] - points[1] : points[count - 1] - points[count - 2]);
                        const uint32_t centre = Vert(points[i] + dir * radii[i] * 0.4f, dir, glm::vec2(0.5f, 0.5f), skin, material, shade);
                        for (int s = 0; s < sides; ++s)
                        {
                            const uint32_t a = base + i * ringStride + s;
                            ix.insert(ix.end(), { centre, a, a + 1 });
                        }
                    }
                }
            }

            // A lumpy ellipsoid. axes: columns = the three axis directions (unit). lump: 0 = smooth, 0.2 = knobbly.
            void Blob(const glm::vec3& centre, const glm::mat3& axes, const glm::vec3& radii, float lump, float seed,
                      int segments, int rings, int skin, float material, float shade, float uvRepeat)
            {
                const uint32_t base = static_cast<uint32_t>(v.size());
                for (int r = 0; r <= rings; ++r)
                {
                    const float phi = kPi * static_cast<float>(r) / rings;
                    for (int s = 0; s <= segments; ++s)
                    {
                        const float theta = 2.0f * kPi * static_cast<float>(s) / segments;
                        const glm::vec3 unit(std::sin(phi) * std::cos(theta), std::cos(phi), std::sin(phi) * std::sin(theta));
                        // Periodic in theta (integer multiples), so the seam at s = 0 / segments closes.
                        const float bump = 1.0f + lump * (0.6f * std::sin(3.0f * theta + seed) * std::sin(2.0f * phi + seed * 1.7f)
                                                        + 0.4f * std::sin(5.0f * theta + seed * 2.3f) * std::sin(4.0f * phi + seed));
                        const glm::vec3 local = unit * radii * bump;
                        const glm::vec3 nrm = glm::normalize(axes * (unit / glm::max(radii, glm::vec3(1.0e-4f))));
                        Vert(centre + axes * local, nrm,
                             glm::vec2(static_cast<float>(s) / segments * uvRepeat, static_cast<float>(r) / rings * uvRepeat * 0.5f),
                             skin, material, shade);
                    }
                }
                const int stride = segments + 1;
                for (int r = 0; r < rings; ++r)
                    for (int s = 0; s < segments; ++s)
                    {
                        const uint32_t a = base + r * stride + s, b = a + 1, c = a + stride, d = c + 1;
                        ix.insert(ix.end(), { a, c, b, b, c, d });
                    }
            }
        };

        glm::vec3 JointBind(const roj::SkinnedModel& model, uint16_t skel)
        {
            return glm::vec3(glm::inverse(model.skeleton.bones[skel].invBind)[3]);
        }

        bool Contains(const std::vector<uint16_t>& list, uint16_t value)
        {
            return std::find(list.begin(), list.end(), value) != list.end();
        }

        // The child of b (a skin bone) that continues it best along `prefer` (unit). -1 if it has no skin-bone child.
        int ChildAlong(const roj::SkinnedModel& model, const Profile& profile, uint16_t b, const glm::vec3& prefer)
        {
            int best = -1;
            float bestDot = -2.0f;
            const glm::vec3 pb = JointBind(model, b);
            for (uint16_t c = b + 1; c < profile.subtreeEnd[b]; ++c)
            {
                if (profile.parent[c] != b || model.skeleton.bones[c].skinIdx < 0) continue;
                const glm::vec3 d = JointBind(model, c) - pb;
                const float len = glm::length(d);
                if (len < 1.0e-4f) continue;
                const float dt = glm::dot(d / len, prefer);
                if (dt > bestDot) { bestDot = dt; best = c; }
            }
            return best;
        }

        // Limb radius around a point: median of 6 rays square to `axis`. -1 when fewer than 4 rays hit within 0.3 m
        // (the point is not inside the body: a weapon bone, an IK bone).
        float RadiusAround(TriGrid& grid, const glm::vec3& point, const glm::vec3& axis)
        {
            glm::vec3 x = glm::cross(axis, std::abs(axis.y) < 0.9f ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0));
            x = glm::normalize(x);
            const glm::vec3 y = glm::cross(axis, x);
            float d[6];
            int hits = 0;
            for (int i = 0; i < 6; ++i)
            {
                const float a = 2.0f * kPi * static_cast<float>(i) / 6.0f;
                const float t = grid.Cast(point, x * std::cos(a) + y * std::sin(a), 0.3f);
                if (t > 0.0f) d[hits++] = t;
            }
            if (hits < 4) return -1.0f;
            std::sort(d, d + hits);
            return d[hits / 2];
        }

        void BuildAnatomy(const roj::SkinnedModel& model, const Profile& profile, TriGrid& grid, ModelFlesh& out)
        {
            const AnatomyConfig& cfg = profile.anatomy;
            const glm::vec3 up = cfg.up, forward = cfg.forward;
            const glm::vec3 right = glm::normalize(glm::cross(up, forward));
            const int boneCount = model.skeleton.boneCount;
            Builder B{ out.anatomyVertices, out.anatomyIndices, out.pieces };

            // 1. A bone inside every limb segment (parent joint -> child joint), sized by the limb around it.
            for (int b = 0; b < boneCount; ++b)
            {
                const int skin = model.skeleton.bones[b].skinIdx;
                if (skin < 0 || Contains(cfg.torso, static_cast<uint16_t>(b))) continue;
                const glm::vec3 pb = JointBind(model, static_cast<uint16_t>(b));
                for (uint16_t c = static_cast<uint16_t>(b + 1); c < profile.subtreeEnd[b]; ++c)
                {
                    if (profile.parent[c] != b || model.skeleton.bones[c].skinIdx < 0) continue;
                    const glm::vec3 pc = JointBind(model, c);
                    const float len = glm::distance(pb, pc);
                    if (len < cfg.minBoneLength) continue;
                    const glm::vec3 axis = (pc - pb) / len;
                    const float limb = RadiusAround(grid, (pb + pc) * 0.5f, axis);
                    if (limb < 0.0f || limb > 0.14f) continue;
                    const float r = glm::clamp(0.28f * limb, 0.006f, 0.03f);

                    static const float kT[7] = { 0.00f, 0.04f, 0.12f, 0.50f, 0.88f, 0.96f, 1.00f };
                    static const float kR[7] = { 1.20f, 1.70f, 1.00f, 0.88f, 1.00f, 1.70f, 1.20f };   // knobs at both ends
                    std::vector<glm::vec3> pts;
                    std::vector<float> radii;
                    for (int i = 0; i < 7; ++i) { pts.push_back(pb + axis * (len * kT[i])); radii.push_back(r * kR[i]); }

                    B.Begin();
                    B.Tube(pts, radii, 6, false, std::abs(axis.y) < 0.9f ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0), skin, 0.0f, 1.0f, 6.0f);
                    B.End(static_cast<uint16_t>(b));
                }
            }

            // 2. Trunk bones: a spine at the back of the body, made of vertebrae (alternating thick / thin).
            for (uint16_t b : cfg.torso)
            {
                const int skin = model.skeleton.bones[b].skinIdx;
                const int c = ChildAlong(model, profile, b, up);
                if (c < 0) continue;
                const glm::vec3 pb = JointBind(model, b), pc = JointBind(model, static_cast<uint16_t>(c));
                const float len = glm::distance(pb, pc);
                if (len < 0.02f) continue;
                const float back = grid.Cast((pb + pc) * 0.5f, -forward, 0.4f);
                const glm::vec3 offset = back > 0.0f ? -forward * (back * 0.6f) : glm::vec3(0);

                const int steps = std::max(2, static_cast<int>(len / 0.022f));
                std::vector<glm::vec3> pts;
                std::vector<float> radii;
                for (int i = 0; i <= steps; ++i)
                {
                    pts.push_back(glm::mix(pb, pc, static_cast<float>(i) / steps) + offset);
                    radii.push_back((i % 2 == 0) ? 0.024f : 0.013f);
                }
                B.Begin();
                B.Tube(pts, radii, 6, false, forward, skin, 0.0f, 0.95f, 8.0f);
                B.End(b);
            }

            // 3. Ribs: rings fitted to the chest cross-section, a breastbone at the front, dark organs inside.
            for (uint16_t b : cfg.ribs)
            {
                const int skin = model.skeleton.bones[b].skinIdx;
                const int c = ChildAlong(model, profile, b, up);
                if (c < 0) continue;
                const glm::vec3 pb = JointBind(model, b), pc = JointBind(model, static_cast<uint16_t>(c));
                const float len = glm::distance(pb, pc);
                if (len < 0.03f) continue;

                const int ringCount = std::max(1, static_cast<int>(len / 0.045f));
                constexpr int kDirs = 16;
                std::vector<glm::vec3> fronts;
                glm::vec3 meanRadii(0.0f);
                glm::vec3 meanCentre(0.0f);

                B.Begin();
                for (int r = 0; r < ringCount; ++r)
                {
                    const glm::vec3 centre = glm::mix(pb, pc, (static_cast<float>(r) + 0.5f) / ringCount);
                    float dist[kDirs];
                    float sorted[kDirs];
                    for (int i = 0; i < kDirs; ++i)
                    {
                        const float a = 2.0f * kPi * static_cast<float>(i) / kDirs;
                        const float t = grid.Cast(centre, right * std::cos(a) + forward * std::sin(a), 0.4f);
                        dist[i] = t > 0.0f ? t : 0.12f;
                        sorted[i] = dist[i];
                    }
                    std::sort(sorted, sorted + kDirs);
                    const float median = sorted[kDirs / 2];
                    for (int i = 0; i < kDirs; ++i) dist[i] = std::min(dist[i], median * 1.5f);   // a ray that ran into an arm

                    std::vector<glm::vec3> pts;
                    std::vector<float> radii;
                    for (int i = 0; i < kDirs; ++i)
                    {
                        const float a = 2.0f * kPi * static_cast<float>(i) / kDirs;
                        const float smooth = 0.5f * dist[i] + 0.25f * (dist[(i + 1) % kDirs] + dist[(i + kDirs - 1) % kDirs]);
                        const glm::vec3 dir = right * std::cos(a) + forward * std::sin(a);
                        // 80 % of the way to the skin; ribs slope down towards the front.
                        pts.push_back(centre + dir * (smooth * 0.80f) - up * (0.03f * std::max(0.0f, std::sin(a))));
                        radii.push_back(0.009f);
                    }
                    B.Tube(pts, radii, 4, true, up, skin, 0.0f, 1.0f, 6.0f);

                    fronts.push_back(pts[kDirs / 4]);   // a = 90 degrees = straight forward
                    meanRadii += glm::vec3(dist[0] + dist[kDirs / 2], 0.0f, dist[kDirs / 4] + dist[3 * kDirs / 4]) * 0.5f;
                    meanCentre += centre + forward * ((dist[kDirs / 4] - dist[3 * kDirs / 4]) * 0.5f);
                }
                if (fronts.size() >= 2)
                {
                    std::vector<float> radii(fronts.size(), 0.014f);
                    B.Tube(fronts, radii, 4, false, forward, skin, 0.0f, 1.0f, 6.0f);
                }
                B.End(b);

                // Chest organs (lungs / heart): one dark lumpy mass that fills the ribs.
                meanRadii /= static_cast<float>(ringCount);
                meanCentre /= static_cast<float>(ringCount);
                B.Begin();
                B.Blob(meanCentre, glm::mat3(right, up, forward), glm::vec3(meanRadii.x * 0.62f, len * 0.6f, meanRadii.z * 0.62f),
                       0.14f, static_cast<float>(b) * 1.3f, 10, 6, skin, 1.0f, 0.55f, 2.0f);
                B.End(b);
            }

            // 4. Guts: a knobbly mass in the belly.
            for (uint16_t b : cfg.guts)
            {
                const int skin = model.skeleton.bones[b].skinIdx;
                const int c = ChildAlong(model, profile, b, up);
                const glm::vec3 pb = JointBind(model, b);
                const glm::vec3 pc = c >= 0 ? JointBind(model, static_cast<uint16_t>(c)) : pb + up * 0.12f;
                const float len = std::max(glm::distance(pb, pc), 0.08f);
                glm::vec3 centre = (pb + pc) * 0.5f;

                auto reach = [&](const glm::vec3& d) { const float t = grid.Cast(centre, d, 0.4f); return t > 0.0f ? t : 0.12f; };
                const float rx = std::min(reach(right), reach(-right));
                const float front = reach(forward), backD = reach(-forward);
                centre += forward * ((front - backD) * 0.5f);

                B.Begin();
                B.Blob(centre, glm::mat3(right, up, forward), glm::vec3(rx * 0.62f, len * 0.70f, (front + backD) * 0.5f * 0.62f),
                       0.20f, static_cast<float>(b) * 2.1f + 0.7f, 10, 6, skin, 1.0f, 1.0f, 3.0f);
                B.End(b);
            }

            // 5. Skull: fitted into the head.
            if (cfg.head >= 0 && cfg.head < static_cast<int>(profile.anyBounds.size()) && profile.anyBounds[cfg.head].hasData)
            {
                const uint16_t h = static_cast<uint16_t>(cfg.head);
                const int skin = model.skeleton.bones[h].skinIdx;
                const roj::BoneBound& bb = profile.anyBounds[h];
                const glm::vec3 joint = JointBind(model, h);
                // The head bone also carries part of the neck: the skull sits in the upper part of its bounds.
                glm::vec3 centre = (bb.bbMin + bb.bbMax) * 0.5f;
                const float top = glm::dot(bb.bbMax - joint, glm::abs(up));       // rough height of the head above its joint
                centre = joint + up * (std::max(top, 0.1f) * 0.55f) + (centre - joint - up * glm::dot(centre - joint, up));

                auto reach = [&](const glm::vec3& d, float fallback) { const float t = grid.Cast(centre, d, 0.3f); return t > 0.0f ? t : fallback; };
                const float rx = 0.5f * (reach(right, 0.08f) + reach(-right, 0.08f));
                const float ry = reach(up, 0.1f);
                const float front = reach(forward, 0.1f), backD = reach(-forward, 0.1f);
                centre += forward * ((front - backD) * 0.5f);

                B.Begin();
                B.Blob(centre, glm::mat3(right, up, forward), glm::vec3(rx, ry, (front + backD) * 0.5f) * 0.80f,
                       0.03f, 0.4f, 12, 8, skin, 0.0f, 1.0f, 2.0f);
                B.End(h);
            }
        }
    }

    void ModelFlesh::EnsureGpu()
    {
        if (gpuReady) return;
        gpuReady = true;

        const bgfx::VertexLayout layout = VertexData::Declaration();
        shellVbh.assign(shellVertices.size(), BGFX_INVALID_HANDLE);
        for (size_t i = 0; i < shellVertices.size(); ++i)
        {
            if (shellVertices[i].empty()) continue;
            shellVbh[i] = bgfx::createVertexBuffer(
                bgfx::copy(shellVertices[i].data(), static_cast<uint32_t>(sizeof(VertexData) * shellVertices[i].size())), layout);
        }
        if (!anatomyVertices.empty() && !anatomyIndices.empty())
        {
            anatomyVbh = bgfx::createVertexBuffer(
                bgfx::copy(anatomyVertices.data(), static_cast<uint32_t>(sizeof(VertexData) * anatomyVertices.size())), layout);
            anatomyIbh = bgfx::createIndexBuffer(
                bgfx::copy(anatomyIndices.data(), static_cast<uint32_t>(sizeof(uint32_t) * anatomyIndices.size())), BGFX_BUFFER_INDEX32);
        }
    }

    std::shared_ptr<ModelFlesh> BuildModelFlesh(const roj::SkinnedModel& model, const Profile& profile)
    {
        const auto buildStart = std::chrono::steady_clock::now();
        auto flesh = std::make_shared<ModelFlesh>();

        // What the wound cavities test points against (const queries only): the flesh meshes and the closed clothes around them.
        auto body = std::make_shared<TriGrid>();
        body->Build(model, profile, TriGrid::Mode::Body);
        if (!body->Empty()) flesh->body = body;

        auto grid = std::make_shared<TriGrid>();
        grid->Build(model, profile, TriGrid::Mode::Volume);
        if (grid->Empty())
        {
            flesh->shellVertices.assign(model.meshes.size(), {});
            return flesh;   // no Volume submesh: wounds are holes only (and cavities, if clothes enclose the body)
        }

        BuildShells(model, profile, *grid, *flesh);
        BuildAnatomy(model, profile, *grid, *flesh);

        Logger::Log("Gore: wound cavities test against " + std::to_string(body->Size()) + " body triangles (" + std::to_string(grid->Size())
                    + " of them in Volume submeshes)");
        Logger::Log("Gore: flesh built: " + std::to_string(flesh->pieces.size()) + " anatomy pieces, "
                    + std::to_string(flesh->anatomyVertices.size()) + " anatomy vertices, "
                    + std::to_string(std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - buildStart).count()) + " ms");
        return flesh;
    }

    void BuildChunkMeshes(std::vector<VertexData>& vertices, std::vector<uint32_t>& indices, std::vector<FleshPiece>& variants)
    {
        Builder B{ vertices, indices, variants };
        const glm::mat3 I(1.0f);

        // Meat lumps: material 1 (guts texture), different shapes and brightness.
        B.Begin(); B.Blob(glm::vec3(0), I, glm::vec3(1.0f, 0.70f, 0.85f), 0.30f, 0.3f, 7, 5, 0, 1.0f, 1.0f, 1.0f); B.End(0);
        B.Begin(); B.Blob(glm::vec3(0), I, glm::vec3(1.3f, 0.50f, 0.70f), 0.35f, 1.9f, 7, 5, 0, 1.0f, 0.8f, 1.0f); B.End(0);
        B.Begin(); B.Blob(glm::vec3(0), I, glm::vec3(0.8f, 0.80f, 0.60f), 0.25f, 4.2f, 6, 4, 0, 1.0f, 1.1f, 1.0f); B.End(0);

        // A bone splinter: material 0 (bone texture), thick in the middle, pointed at one end.
        const std::vector<glm::vec3> points = { glm::vec3(-1.2f, 0.0f, 0.0f), glm::vec3(-0.4f, 0.1f, 0.0f),
                                                glm::vec3(0.5f, -0.05f, 0.0f), glm::vec3(1.3f, 0.1f, 0.0f) };
        const std::vector<float> radii = { 0.12f, 0.30f, 0.26f, 0.05f };
        B.Begin(); B.Tube(points, radii, 5, false, glm::vec3(0, 1, 0), 0, 0.0f, 1.0f, 1.0f); B.End(0);
    }
}
