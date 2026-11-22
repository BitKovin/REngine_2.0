#include "GoreCore.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace Gore
{
    namespace
    {
        constexpr float kMinScale = 0.6f;   // a core bone is never scaled below or above these (a character with odd proportions
        constexpr float kMaxScale = 1.7f;   // would turn the core into a spike or a blob)

        glm::mat4 BindWorld(const roj::SkinnedModel& model, uint16_t bone)
        {
            return glm::inverse(model.skeleton.bones[bone].invBind);
        }

        glm::vec3 JointOf(const roj::SkinnedModel& model, uint16_t bone)
        {
            return glm::vec3(BindWorld(model, bone)[3]);
        }

        // One vertex of the core on the character: position and normal blended by the weights, the weights moved to the
        // character's skin indices.
        struct Moved
        {
            glm::vec3 p = glm::vec3(0), n = glm::vec3(0, 1, 0);
            float bone[4] = {};
            glm::vec4 weight = glm::vec4(0);
        };
    }

    std::shared_ptr<const TriGrid> BuildFittedCore(const roj::SkinnedModel& core, const roj::SkinnedModel& body,
                                                   const std::unordered_map<std::string, std::string>& boneMap)
    {
        const int coreCount = core.skeleton.boneCount;
        if (coreCount == 0 || core.meshes.empty() || body.skeleton.boneCount == 0) return nullptr;

        // The character's bone for each core bone: by name, through the bone map. -1 = the character has no such bone.
        std::vector<int> direct(coreCount, -1);
        int matched = 0;
        for (int c = 0; c < coreCount; ++c)
        {
            std::string name = core.skeleton.bones[c].name.str();
            const auto mapped = boneMap.find(name);
            if (mapped != boneMap.end()) name = mapped->second;
            const uint16_t found = body.skeleton.findBone(name);
            if (found != INVALID_BONE_IDX && body.skeleton.bones[found].skinIdx >= 0)
            {
                direct[c] = found;
                ++matched;
            }
        }
        if (matched == 0) return nullptr;

        // The core bone that carries each core bone: itself, or its nearest ancestor that the character has a bone for.
        std::vector<int> carrier(coreCount, -1);
        for (int c = 0; c < coreCount; ++c)
        {
            if (direct[c] >= 0) { carrier[c] = c; continue; }
            const uint16_t parent = core.skeleton.bones[c].parentIdx;
            if (parent != INVALID_BONE_IDX && parent < c) carrier[c] = carrier[parent];
        }

        // The scale of each matched bone: the length of the bone to its first matched child in the character over that in the core.
        std::vector<float> scale(coreCount, 0.0f);
        for (int c = 0; c < coreCount; ++c)
        {
            if (direct[c] < 0) continue;
            for (int k = c + 1; k < coreCount; ++k)
            {
                if (core.skeleton.bones[k].parentIdx != c || direct[k] < 0) continue;
                const float lengthCore = glm::distance(JointOf(core, static_cast<uint16_t>(c)), JointOf(core, static_cast<uint16_t>(k)));
                const float lengthBody = glm::distance(JointOf(body, static_cast<uint16_t>(direct[c])), JointOf(body, static_cast<uint16_t>(direct[k])));
                if (lengthCore > 1.0e-3f && lengthBody > 1.0e-3f)
                {
                    scale[c] = glm::clamp(lengthBody / lengthCore, kMinScale, kMaxScale);
                    break;
                }
            }
        }
        for (int c = 0; c < coreCount; ++c)   // ends of chains (hands, feet, head) and bones without a usable child: like the parent
        {
            if (direct[c] < 0 || scale[c] > 0.0f) continue;
            const uint16_t parent = core.skeleton.bones[c].parentIdx;
            scale[c] = (parent != INVALID_BONE_IDX && parent < c && carrier[parent] >= 0 && scale[carrier[parent]] > 0.0f)
                     ? scale[carrier[parent]] : 1.0f;
        }

        // Core model space -> the character's bind space, for each core bone: the character's bone frame times the scale times the
        // inverse of the core bone's own frame. Linear blend skinning with these puts every core vertex where its bones are.
        std::vector<glm::mat4> place(coreCount, glm::mat4(1.0f));
        std::vector<int> bodySkin(coreCount, -1);   // the character's skin index that carries each core bone
        for (int c = 0; c < coreCount; ++c)
        {
            const int own = carrier[c];
            if (own < 0) continue;
            place[c] = BindWorld(body, static_cast<uint16_t>(direct[own])) * glm::scale(glm::mat4(1.0f), glm::vec3(scale[own]))
                     * core.skeleton.bones[own].invBind;
            bodySkin[c] = body.skeleton.bones[direct[own]].skinIdx;
        }

        // Core skin index -> core bone.
        std::vector<int> skinToBone(MAX_SKINNED_BONES, -1);
        for (int c = 0; c < coreCount; ++c)
        {
            const int skin = core.skeleton.bones[c].skinIdx;
            if (skin >= 0 && skin < MAX_SKINNED_BONES) skinToBone[skin] = c;
        }

        auto move = [&](const VertexData& v) -> Moved
        {
            Moved out;
            glm::vec3 p(0.0f), n(0.0f);
            float total = 0.0f;
            int count = 0;
            for (int i = 0; i < MAX_BONE_INFLUENCE; ++i)
            {
                const float w = v.BlendWeights[i];
                if (!(w > 0.0f)) continue;
                const int skin = static_cast<int>(v.BlendIndices[i]);
                const int bone = (skin >= 0 && skin < MAX_SKINNED_BONES) ? skinToBone[skin] : -1;
                if (bone < 0 || bodySkin[bone] < 0) continue;
                p += w * glm::vec3(place[bone] * glm::vec4(v.Position, 1.0f));
                n += w * glm::vec3(place[bone] * glm::vec4(v.Normal, 0.0f));
                total += w;
                int slot = -1;
                for (int s = 0; s < count; ++s)
                    if (static_cast<int>(out.bone[s]) == bodySkin[bone]) slot = s;
                if (slot < 0 && count < 4) { slot = count++; out.bone[slot] = static_cast<float>(bodySkin[bone]); }
                if (slot >= 0) out.weight[slot] += w;
            }
            if (total <= 0.0f)
            {
                out.p = v.Position;   // carried by no bone the character has: stays where it is
                out.n = v.Normal;
                out.weight = glm::vec4(1, 0, 0, 0);
                return out;
            }
            out.p = p / total;
            out.n = glm::length(n) > 1.0e-6f ? glm::normalize(n) : v.Normal;
            out.weight /= std::max(out.weight.x + out.weight.y + out.weight.z + out.weight.w, 1.0e-6f);
            return out;
        };

        std::vector<GridTri> triangles;
        for (size_t m = 0; m < core.meshes.size(); ++m)
        {
            const roj::SkinnedMesh& mesh = core.meshes[m];
            for (size_t i = 0; i + 2 < mesh.indices.size(); i += 3)
            {
                const Moved corners[3] = { move(mesh.vertices[mesh.indices[i]]), move(mesh.vertices[mesh.indices[i + 1]]),
                                           move(mesh.vertices[mesh.indices[i + 2]]) };
                GridTri t;
                t.a = corners[0].p; t.b = corners[1].p; t.c = corners[2].p;
                t.na = corners[0].n; t.nb = corners[1].n; t.nc = corners[2].n;
                t.mesh = static_cast<uint16_t>(m);
                for (int k = 0; k < 3; ++k)
                {
                    for (int j = 0; j < 4; ++j) t.bone[k][j] = corners[k].bone[j];
                    t.weight[k] = corners[k].weight;
                }
                triangles.push_back(t);
            }
        }
        if (triangles.empty()) return nullptr;

        auto grid = std::make_shared<TriGrid>();
        grid->BuildFromTriangles(std::move(triangles));
        return grid;
    }
}
