#include "GoreCutSolver.h"

#include <algorithm>
#include <cmath>

namespace Gore
{
    glm::mat4 BoneBindToWorld(const glm::mat4& world, const std::vector<glm::mat4>& palette, int skin)
    {
        if (skin < 0 || skin >= static_cast<int>(palette.size())) return world;
        return world * palette[skin];
    }

    void BoundCorners(const roj::BoneBound& b, glm::vec3 (&out)[8])
    {
        for (int i = 0; i < 8; ++i)
            out[i] = glm::vec3((i & 1) ? b.bbMax.x : b.bbMin.x, (i & 2) ? b.bbMax.y : b.bbMin.y, (i & 4) ? b.bbMax.z : b.bbMin.z);
    }

    namespace
    {
        glm::vec3 JointBind(const roj::SkinnedModel& model, uint16_t skel)
        {
            return glm::vec3(glm::inverse(model.skeleton.bones[skel].invBind)[3]);
        }

        float Volume(const roj::BoneBound& b)
        {
            const glm::vec3 e = glm::max(b.bbMax - b.bbMin, glm::vec3(0));
            return e.x * e.y * e.z;
        }

        // Union of the any-influence bounds over [first, last).
        roj::BoneBound SubtreeBounds(const Profile& profile, uint16_t first, uint16_t last)
        {
            roj::BoneBound out;
            for (uint16_t b = first; b < last && b < profile.anyBounds.size(); ++b)
            {
                const roj::BoneBound& s = profile.anyBounds[b];
                if (!s.hasData) continue;
                out.bbMin = glm::min(out.bbMin, s.bbMin);
                out.bbMax = glm::max(out.bbMax, s.bbMax);
                out.hasData = true;
            }
            return out;
        }

        struct Frame
        {
            const Profile* profile;
            uint16_t S;
            glm::vec3 joint, axis;
            float length;
        };

        // The plane is acceptable when every vertex that leaves with the part lies strictly beyond it and the bone keeps
        // some of its own vertices. (Bounds are the fallback for a model without vertex data.)
        bool Valid(const Frame& f, const SeverableBone& sev, const glm::vec3& n, const glm::vec3& o)
        {
            if (!sev.partPoints.empty())
            {
                for (const glm::vec3& p : sev.partPoints)
                    if (glm::dot(n, p - o) <= 0.003f) return false;
                for (const glm::vec3& p : sev.ownPoints)
                    if (glm::dot(n, p - o) < -0.003f) return true;
                return sev.ownPoints.empty();
            }

            const Profile& p = *f.profile;
            for (uint16_t b = f.S + 1; b < p.subtreeEnd[f.S]; ++b)
            {
                const roj::BoneBound& bb = p.anyBounds[b];
                if (!bb.hasData) continue;
                glm::vec3 c[8];
                BoundCorners(bb, c);
                for (int i = 0; i < 8; ++i)
                    if (glm::dot(n, c[i] - o) <= 0.002f) return false;
            }
            return true;
        }
    }

    CutResult SolveCut(const Profile& profile, const roj::SkinnedModel& model, const glm::mat4& world,
                       const std::vector<glm::mat4>& palette, const CutRequest& req, Rng& rng)
    {
        CutResult result;
        if (req.severableIndex >= profile.severable.size()) return result;

        const SeverableBone& sev = profile.severable[req.severableIndex];
        const uint16_t S = sev.skel;
        if (S >= profile.subtreeEnd.size() || S >= profile.anyBounds.size()) return result;
        if (sev.skin < 0 || sev.skin >= static_cast<int>(palette.size())) return result;

        Frame f;
        f.profile = &profile;
        f.S = S;
        f.joint = JointBind(model, S);

        // Axis: towards the child whose subtree is biggest, else towards the far corner of the bone's bounds.
        glm::vec3 child = f.joint;
        float bestVolume = -1.0f;
        for (uint16_t c = S + 1; c < profile.subtreeEnd[S]; ++c)
        {
            if (profile.parent[c] != S) continue;
            if (model.skeleton.bones[c].skinIdx < 0) continue;
            const float v = Volume(SubtreeBounds(profile, c, profile.subtreeEnd[c]));
            if (v > bestVolume) { bestVolume = v; child = JointBind(model, c); }
        }
        if (bestVolume < 0.0f)
        {
            const roj::BoneBound& own = profile.anyBounds[S];
            if (!own.hasData) return result;
            glm::vec3 corners[8];
            BoundCorners(own, corners);
            float far = -1.0f;
            for (int i = 0; i < 8; ++i)
            {
                const float d = glm::distance(corners[i], f.joint);
                if (d > far) { far = d; child = corners[i]; }
            }
        }

        f.length = glm::distance(child, f.joint);
        if (f.length < 0.01f) return result;
        f.axis = (child - f.joint) / f.length;

        const glm::mat4 M = BoneBindToWorld(world, palette, sev.skin);
        const glm::mat4 invM = glm::inverse(M);
        const glm::vec3 p = glm::vec3(invM * glm::vec4(req.worldPoint, 1.0f));

        glm::vec3 n = glm::transpose(glm::mat3(M)) * req.worldNormal;
        if (glm::length(n) < 1.0e-5f) n = f.axis;
        n = glm::normalize(n);
        if (glm::dot(n, f.axis) < 0.0f) n = -n;

        // Tilt clamp.
        const float maxTilt = glm::radians(sev.maxTiltDeg);
        const float cosT = glm::dot(n, f.axis);
        if (cosT < std::cos(maxTilt))
        {
            glm::vec3 perp = n - f.axis * cosT;
            perp = glm::length(perp) < 1.0e-5f ? glm::vec3(0) : glm::normalize(perp);
            n = glm::normalize(f.axis * std::cos(maxTilt) + perp * std::sin(maxTilt));
        }

        const float s = glm::clamp(glm::dot(p - f.joint, f.axis) / f.length, sev.range.x, sev.range.y);
        glm::vec3 o = f.joint + f.axis * (s * f.length);

        // Search: the wanted plane first, then planes closer to the bone axis and shifted along the allowed range.
        bool valid = Valid(f, sev, n, o);
        for (int ti = 0; ti <= 4 && !valid; ++ti)
        {
            const glm::vec3 nk = glm::normalize(glm::mix(n, f.axis, static_cast<float>(ti) * 0.25f));
            for (int ui = 0; ui < 9 && !valid; ++ui)
            {
                // 0, then alternately towards the upper and the lower end of the range.
                const float u = static_cast<float>((ui + 1) / 2) * 0.25f;
                const float target = (ui % 2 == 1) ? sev.range.y : sev.range.x;
                const float sk = ui == 0 ? s : glm::mix(s, target, u);
                const glm::vec3 ok = f.joint + f.axis * (sk * f.length);
                if (Valid(f, sev, nk, ok)) { n = nk; o = ok; valid = true; }
            }
        }
        if (!valid) return result;

        Cut& cut = result.cut;
        cut.plane = glm::vec4(n, glm::dot(n, o));
        cut.center = o;
        cut.boneRadius = sev.boneRadius;
        cut.roughness = 0.012f;
        cut.seed = rng.Next01();
        cut.boneSkel = S;
        cut.boneSkin = sev.skin;

        float maxDist = 0.0f;
        for (uint16_t b = S; b < profile.subtreeEnd[S]; ++b)
        {
            const roj::BoneBound& bb = profile.anyBounds[b];
            if (!bb.hasData) continue;
            glm::vec3 c[8];
            BoundCorners(bb, c);
            for (int i = 0; i < 8; ++i) maxDist = std::max(maxDist, glm::distance(o, c[i]));
        }
        // Cross-section radius: mean distance from the cut centre of the vertices near the plane.
        {
            float sum = 0.0f;
            int count = 0;
            auto accumulate = [&](const std::vector<glm::vec3>& pts)
            {
                for (const glm::vec3& v : pts)
                {
                    const glm::vec3 d = v - o;
                    const float along = glm::dot(n, d);
                    if (std::abs(along) > 0.05f) continue;
                    sum += glm::length(d - n * along);
                    ++count;
                }
            };
            accumulate(sev.partPoints);
            accumulate(sev.ownPoints);
            cut.outlineRadius = count >= 6 ? glm::clamp(sum / static_cast<float>(count), 1.5f * sev.boneRadius, 0.25f) : 2.6f * sev.boneRadius;
        }

        cut.influenceRadius = 1.05f * maxDist + 0.02f;
        cut.regrowMaxRadius = cut.influenceRadius;

        result.ok = true;
        return result;
    }
}
