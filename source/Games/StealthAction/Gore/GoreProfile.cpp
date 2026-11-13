#include "GoreProfile.h"
#include "GoreWoundMesh.h"
#include "GoreTypes.h"

#include <AssetRegistry.h>
#include <FileSystem/FileSystem.h>
#include <Helpers/Mesh/bone_partition.hpp>
#include <Logger.hpp>
#include <Helpers/StringHelper.h>
#include <json.hpp>

#include <algorithm>
#include <unordered_map>

namespace Gore
{
    namespace
    {
        std::unordered_map<std::string, Profile> g_profiles;
        std::unordered_map<std::string, bool> g_logged;   // model path -> already reported
        const Profile g_invalid;

        const Profile& Fail(const std::string& path, const char* why)
        {
            if (!g_logged[path])
            {
                g_logged[path] = true;
                Logger::Warning("Gore: no gore for '%s': %s", path.c_str(), why);
            }
            Profile p;
            p.valid = false;
            return g_profiles[path] = std::move(p);
        }

        Texture* LoadTexture(const nlohmann::json& textures, const char* key)
        {
            if (!textures.is_object() || !textures.contains(key) || !textures[key].is_string()) return nullptr;
            return AssetRegistry::GetTextureFromFile("GameData/textures/" + textures[key].get<std::string>());
        }

        SubmeshRole ParseRole(const std::string& role)
        {
            if (role == "shell") return SubmeshRole::Shell;
            if (role == "attachment") return SubmeshRole::Attachment;
            return SubmeshRole::Volume;
        }
    }

    float MaxExtent(const Profile& profile, uint16_t skel)
    {
        if (skel >= profile.anyBounds.size() || !profile.anyBounds[skel].hasData) return 0.0f;
        const glm::vec3 e = (profile.anyBounds[skel].bbMax - profile.anyBounds[skel].bbMin) * 0.5f;
        return std::max(e.x, std::max(e.y, e.z));
    }

    void ClearProfiles()
    {
        g_profiles.clear();
        g_logged.clear();
    }

    const Profile& GetProfile(const std::string& modelPath, const roj::SkinnedModel& model)
    {
        auto cached = g_profiles.find(modelPath);
        if (cached != g_profiles.end())
            return cached->second;

        // Geometry has not landed yet: try again next time, do not cache a failure.
        if (model.meshes.empty())
            return g_invalid;

        const std::string text = FileSystemEngine::ReadFile(modelPath + ".gore");
        if (text.empty())
            return Fail(modelPath, "no .gore file next to the model");

        nlohmann::json j = nlohmann::json::parse(text, nullptr, false);
        if (j.is_discarded() || !j.is_object())
            return Fail(modelPath, ".gore file is not valid JSON");

        const int boneCount = model.skeleton.boneCount;
        if (boneCount == 0 || boneCount > kMaxSkeletonBones)
            return Fail(modelPath, "skeleton is empty or has too many bones");

        const roj::BonePartitionContext ctx = roj::BuildBonePartitionContext(model.skeleton);
        if (!ctx.valid)
            return Fail(modelPath, "skeleton is not in parent-before-child order");

        for (const roj::SkinnedMesh& mesh : model.meshes)
            if (mesh.boneIndexFirst.size() != static_cast<size_t>(boneCount) + 1)
                return Fail(modelPath, "mesh is not bone-partitioned (engine load path)");

        Profile p;

        const nlohmann::json textures = j.value("textures", nlohmann::json::object());
        p.flesh = LoadTexture(textures, "flesh");
        p.fleshNormal = LoadTexture(textures, "fleshNormal");
        p.blood = LoadTexture(textures, "blood");
        p.edgeNoise = LoadTexture(textures, "edgeNoise");
        p.boneT = LoadTexture(textures, "bone");

        p.fleshScale = j.value("fleshScale", 12.0f);
        if (j.contains("bloodColor") && j["bloodColor"].is_array() && j["bloodColor"].size() == 3)
            p.bloodColor = glm::vec3(j["bloodColor"][0].get<float>(), j["bloodColor"][1].get<float>(), j["bloodColor"][2].get<float>());

        // Submesh roles by name prefix (first matching prefix wins, default volume).
        p.submeshRoles.assign(model.meshes.size(), SubmeshRole::Volume);
        if (j.contains("submeshes") && j["submeshes"].is_object())
        {
            for (size_t i = 0; i < model.meshes.size(); ++i)
            {
                for (auto it = j["submeshes"].begin(); it != j["submeshes"].end(); ++it)
                {
                    if (!it.value().is_string()) continue;
                    if (StringHelper::StartsWith(model.meshes[i].name, it.key()))
                    {
                        p.submeshRoles[i] = ParseRole(it.value().get<std::string>());
                        break;
                    }
                }
            }
        }

        // Severable bones.
        p.severableBySkel.assign(boneCount, -1);
        if (j.contains("severable") && j["severable"].is_array())
        {
            for (const nlohmann::json& s : j["severable"])
            {
                if (p.severable.size() >= 16)
                {
                    Logger::Error("Gore: '%s' has more than 16 severable bones, extra ones ignored", modelPath.c_str());
                    break;
                }

                SeverableBone b;
                b.bone = s.value("bone", std::string());
                const uint16_t skel = model.skeleton.findBone(b.bone);
                if (skel == INVALID_BONE_IDX)
                {
                    Logger::Error("Gore: '%s' severable bone '%s' is not in the skeleton", modelPath.c_str(), b.bone.c_str());
                    continue;
                }
                b.skel = skel;
                b.skin = model.skeleton.bones[skel].skinIdx;
                if (b.skin < 0)
                {
                    Logger::Error("Gore: '%s' severable bone '%s' has no skin index", modelPath.c_str(), b.bone.c_str());
                    continue;
                }

                if (s.contains("range") && s["range"].is_array() && s["range"].size() == 2)
                    b.range = glm::vec2(s["range"][0].get<float>(), s["range"][1].get<float>());
                b.maxTiltDeg = s.value("maxTilt", b.maxTiltDeg);
                b.boneRadius = s.value("boneRadius", b.boneRadius);
                b.gib = s.value("gib", std::string("rigid")) == "ragdoll" ? SeverableBone::GibKind::Ragdoll : SeverableBone::GibKind::Rigid;
                b.canDestroy = s.value("canDestroy", false);
                b.lethal = s.value("lethal", false);
                b.boneStub = s.value("boneStub", std::string());
                if (s.contains("stump") && s["stump"].is_object())
                {
                    b.spurtSeconds = s["stump"].value("spurtSeconds", b.spurtSeconds);
                    b.pulseHz = s["stump"].value("pulseHz", b.pulseHz);
                }

                p.severableBySkel[skel] = static_cast<int16_t>(p.severable.size());
                p.severable.push_back(std::move(b));
            }
        }

        // Vertex samples for the cut solver.
        for (SeverableBone& sev : p.severable)
        {
            const uint16_t end = ctx.subtreeEnd[sev.skel];
            for (const roj::SkinnedMesh& mesh : model.meshes)
            {
                for (const VertexData& v : mesh.vertices)
                {
                    float below = 0.0f, own = 0.0f;
                    for (int k = 0; k < MAX_BONE_INFLUENCE; ++k)
                    {
                        const float w = v.BlendWeights[k];
                        if (!(w > 0.0f)) continue;
                        const int skin = static_cast<int>(v.BlendIndices[k]);
                        if (skin < 0 || skin >= MAX_SKINNED_BONES) continue;
                        const uint16_t skel = ctx.skinToSkel[skin];
                        if (skel == INVALID_BONE_IDX || skel < sev.skel || skel >= end) continue;
                        if (skel == sev.skel) own += w; else below += w;
                    }
                    if (below >= 0.5f) sev.partPoints.push_back(v.Position);
                    else if (own >= 0.5f) sev.ownPoints.push_back(v.Position);
                }
            }

            // Keep it light: at most ~384 samples each (even stride).
            auto thin = [](std::vector<glm::vec3>& pts)
            {
                if (pts.size() <= 384) return;
                const size_t stride = (pts.size() + 383) / 384;
                size_t out = 0;
                for (size_t i = 0; i < pts.size(); i += stride) pts[out++] = pts[i];
                pts.resize(out);
            };
            thin(sev.partPoints);
            thin(sev.ownPoints);
        }

        // Union of the per-mesh any-influence bounds.
        p.anyBounds.assign(boneCount, roj::BoneBound{});
        for (const roj::SkinnedMesh& mesh : model.meshes)
        {
            if (mesh.boneAnyBounds.size() != static_cast<size_t>(boneCount)) continue;
            for (int b = 0; b < boneCount; ++b)
            {
                const roj::BoneBound& src = mesh.boneAnyBounds[b];
                if (!src.hasData) continue;
                roj::BoneBound& dst = p.anyBounds[b];
                dst.bbMin = glm::min(dst.bbMin, src.bbMin);
                dst.bbMax = glm::max(dst.bbMax, src.bbMax);
                dst.hasData = true;
            }
        }

        p.parent = ctx.parent;
        p.subtreeEnd = ctx.subtreeEnd;
        p.triGrid = BuildTriangleGrid(model, p);
        p.valid = true;

        return g_profiles[modelPath] = std::move(p);
    }
}
