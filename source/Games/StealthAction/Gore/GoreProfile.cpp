#include "GoreProfile.h"
#include "GoreFlesh.h"
#include "GoreTypes.h"

#include <AssetRegistry.h>
#include <FileSystem/FileSystem.h>
#include <Helpers/Mesh/bone_partition.hpp>
#include <Logger.hpp>
#include <Helpers/StringHelper.h>
#include <json.hpp>

#include <ThreadPool.h>
#include <algorithm>
#include <atomic>
#include <unordered_map>

namespace Gore
{
    namespace
    {
        std::unordered_map<std::string, Profile> g_profiles;
        std::unordered_map<std::string, bool> g_logged;   // model path -> already reported
        std::unordered_map<std::string, int> g_coreWaits;   // model path -> GetProfile calls spent waiting for the core body
        constexpr int kCoreWaitCalls = 3000;
        const Profile g_invalid;
        const Profile g_loading = [] { Profile p; p.loading = true; return p; }();

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

        // Registry file name of a texture of the profile's "textures" object, "" when it names none.
        std::string TexturePath(const nlohmann::json& textures, const char* key)
        {
            if (!textures.is_object() || !textures.contains(key) || !textures[key].is_string()) return std::string();
            return "GameData/textures/" + textures[key].get<std::string>();
        }

        // Asks the registry for the textures of the profile (see Profile::fleshTexture). Game thread.
        void ResolveTextures(Profile& p)
        {
            auto resolve = [](const std::string& path, Texture*& slot)
            {
                Texture* texture = path.empty() ? nullptr : AssetRegistry::GetTextureFromFile(path);
                if (texture != slot) slot = texture;
            };
            resolve(p.fleshPath, p.fleshTexture);
            resolve(p.bloodPath, p.blood);
            resolve(p.decalPath, p.decal);
            resolve(p.edgeNoisePath, p.edgeNoise);
            resolve(p.bonePath, p.boneT);
            resolve(p.gutsPath, p.guts);
        }

        SubmeshRole ParseRole(const std::string& role)
        {
            if (role == "shell") return SubmeshRole::Shell;
            if (role == "attachment") return SubmeshRole::Attachment;
            if (role == "prop") return SubmeshRole::Prop;
            return SubmeshRole::Volume;
        }
    }

    struct Profile::PendingFlesh
    {
        std::atomic<bool> done{ false };
        std::shared_ptr<ModelFlesh> result;
    };

    ThreadPool& BuildPool()
    {
        static ThreadPool pool("GoreBuild");
        static bool started = false;
        if (!started)
        {
            started = true;
            if (ThreadPool::Supported()) pool.Start(1);
        }
        return pool;
    }

    float MaxExtent(const Profile& profile, uint16_t skel)
    {
        if (skel >= profile.anyBounds.size() || !profile.anyBounds[skel].hasData) return 0.0f;
        const glm::vec3 e = (profile.anyBounds[skel].bbMax - profile.anyBounds[skel].bbMin) * 0.5f;
        return std::max(e.x, std::max(e.y, e.z));
    }

    void ClearProfiles()
    {
        BuildPool().WaitForFinish();   // a job may hold references into the profiles and the models
        g_profiles.clear();
        g_logged.clear();
    }

    const Profile& GetProfile(const std::string& modelPath, const roj::SkinnedModel& model)
    {
        auto cached = g_profiles.find(modelPath);
        if (cached != g_profiles.end())
        {
            Profile& c = cached->second;
            if (c.pendingFlesh)
            {
                if (!c.pendingFlesh->done.load(std::memory_order_acquire))
                    return g_loading;   // the flesh is still being built on the worker: hits are kept, gore starts when it is done
                c.flesh = std::move(c.pendingFlesh->result);
                c.pendingFlesh.reset();
                c.valid = true;
            }
            ResolveTextures(c);   // the registry may have freed them since (a level change)
            return c;
        }

        // Geometry has not landed yet: try again next time, do not cache a failure.
        if (model.meshes.empty())
            return g_invalid;

        // Geometry is there but not partitioned yet (the load is still finishing on another thread): not a failure, try again
        // next time. A mesh that stays unpartitioned is reported once and never gets gore.
        const int meshBones = model.skeleton.boneCount;
        for (const roj::SkinnedMesh& mesh : model.meshes)
        {
            if (mesh.boneIndexFirst.size() != static_cast<size_t>(meshBones) + 1)
            {
                if (!g_logged[modelPath])
                {
                    g_logged[modelPath] = true;
                    Logger::Warning("Gore: '%s' is not bone-partitioned (yet); gore starts once it is", modelPath.c_str());
                }
                return g_loading;
            }
        }

        const std::string text = FileSystemEngine::ReadFile(modelPath + ".gore");
        if (text.empty())
            return Fail(modelPath, "no .gore file next to the model");

        nlohmann::json j = nlohmann::json::parse(text, nullptr, false);
        if (j.is_discarded() || !j.is_object())
            return Fail(modelPath, ".gore file is not valid JSON");

        // "extends": a shared profile (same body layout on many models). The model's own keys replace the shared ones.
        if (j.contains("extends") && j["extends"].is_string())
        {
            const std::string basePath = j["extends"].get<std::string>();
            nlohmann::json base = nlohmann::json::parse(FileSystemEngine::ReadFile(basePath), nullptr, false);
            if (base.is_discarded() || !base.is_object())
                return Fail(modelPath, "the shared profile it extends is missing or not valid JSON");
            for (auto it = j.begin(); it != j.end(); ++it)
                if (it.key() != "extends") base[it.key()] = it.value();
            j = std::move(base);
        }

        // "boneMap": the shared profile names bones like the common humanoid rig; a model with another rig maps them.
        std::unordered_map<std::string, std::string> boneMap;
        if (j.contains("boneMap") && j["boneMap"].is_object())
            for (auto it = j["boneMap"].begin(); it != j["boneMap"].end(); ++it)
                if (it.value().is_string()) boneMap[it.key()] = it.value().get<std::string>();
        auto mapBone = [&boneMap](const std::string& name)
        {
            const auto found = boneMap.find(name);
            return found == boneMap.end() ? name : found->second;
        };

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
        p.fleshPath = TexturePath(textures, "flesh");
        p.bloodPath = TexturePath(textures, "blood");
        p.decalPath = TexturePath(textures, "decal");
        p.edgeNoisePath = TexturePath(textures, "edgeNoise");
        p.bonePath = TexturePath(textures, "bone");
        p.gutsPath = TexturePath(textures, "guts");
        ResolveTextures(p);

        p.corePath = j.value("core", std::string());
        p.boneMap = boneMap;

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
                b.bone = mapBone(s.value("bone", std::string()));
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
                b.detachDamage = s.value("detach", b.detachDamage);
                b.boneRadius = s.value("boneRadius", b.boneRadius);
                b.coreRadius = s.value("coreRadius", b.coreRadius);
                b.clipAncestors = s.value("clipAncestors", b.clipAncestors);
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

        // Bones a living enemy keeps whole (the head): the listed bones and everything below them.
        p.protectedWhileAlive.assign(boneCount, 0);
        if (j.contains("protectedWhileAlive") && j["protectedWhileAlive"].is_array())
        {
            for (const nlohmann::json& n : j["protectedWhileAlive"])
            {
                if (!n.is_string()) continue;
                const std::string name = mapBone(n.get<std::string>());
                const uint16_t skel = model.skeleton.findBone(name);
                if (skel == INVALID_BONE_IDX)
                {
                    Logger::Warning("Gore: '%s' protectedWhileAlive bone '%s' is not in the skeleton", modelPath.c_str(), name.c_str());
                    continue;
                }
                for (uint16_t b = skel; b < ctx.subtreeEnd[skel]; ++b)
                    p.protectedWhileAlive[b] = 1;
            }
        }
        for (SeverableBone& sev : p.severable)
            for (uint16_t b = sev.skel; b < ctx.subtreeEnd[sev.skel] && !sev.holdsProtected; ++b)
                sev.holdsProtected = p.protectedWhileAlive[b] != 0;

        // Anatomy layout (all optional; missing bones are skipped with a warning).
        {
            const nlohmann::json a = j.value("anatomy", nlohmann::json::object());
            auto readVec3 = [](const nlohmann::json& v, glm::vec3& out)
            {
                if (v.is_array() && v.size() == 3) out = glm::vec3(v[0].get<float>(), v[1].get<float>(), v[2].get<float>());
            };
            auto boneOf = [&](const std::string& name) -> int
            {
                const uint16_t skel = model.skeleton.findBone(mapBone(name));
                if (skel == INVALID_BONE_IDX || model.skeleton.bones[skel].skinIdx < 0)
                {
                    Logger::Warning("Gore: '%s' anatomy bone '%s' is missing or has no skin index", modelPath.c_str(), name.c_str());
                    return -1;
                }
                return skel;
            };
            auto readList = [&](const char* key, std::vector<uint16_t>& out)
            {
                if (!a.contains(key) || !a[key].is_array()) return;
                for (const nlohmann::json& n : a[key])
                {
                    if (!n.is_string()) continue;
                    const int skel = boneOf(n.get<std::string>());
                    if (skel >= 0) out.push_back(static_cast<uint16_t>(skel));
                }
            };

            if (a.contains("forward")) readVec3(a["forward"], p.anatomy.forward);
            if (a.contains("up")) readVec3(a["up"], p.anatomy.up);
            if (glm::length(p.anatomy.forward) < 1.0e-4f) p.anatomy.forward = glm::vec3(0, 0, 1);
            if (glm::length(p.anatomy.up) < 1.0e-4f) p.anatomy.up = glm::vec3(0, 1, 0);
            p.anatomy.forward = glm::normalize(p.anatomy.forward);
            p.anatomy.up = glm::normalize(p.anatomy.up);
            readList("torso", p.anatomy.torso);
            readList("ribs", p.anatomy.ribs);
            readList("guts", p.anatomy.guts);
            if (a.contains("head") && a["head"].is_string()) p.anatomy.head = boneOf(a["head"].get<std::string>());
            p.anatomy.minBoneLength = a.value("minBoneLength", p.anatomy.minBoneLength);
        }

        // Vertex samples for the cut solver.
        for (SeverableBone& sev : p.severable)
        {
            const uint16_t end = ctx.subtreeEnd[sev.skel];
            for (const roj::SkinnedMesh& mesh : model.meshes)
            {
                for (const VertexData& v : mesh.vertices)
                {
                    float below = 0.0f, own = 0.0f, bestWeight = 0.0f;
                    uint16_t bestBone = sev.skel;
                    for (int k = 0; k < MAX_BONE_INFLUENCE; ++k)
                    {
                        const float w = v.BlendWeights[k];
                        if (!(w > 0.0f)) continue;
                        const int skin = static_cast<int>(v.BlendIndices[k]);
                        if (skin < 0 || skin >= MAX_SKINNED_BONES) continue;
                        const uint16_t skel = ctx.skinToSkel[skin];
                        if (skel == INVALID_BONE_IDX || skel < sev.skel || skel >= end) continue;
                        if (skel == sev.skel) own += w; else { below += w; if (w > bestWeight) { bestWeight = w; bestBone = skel; } }
                    }
                    if (below >= 0.5f) { sev.partPoints.push_back(v.Position); sev.partPointBones.push_back(bestBone); }
                    else if (own >= 0.5f) sev.ownPoints.push_back(v.Position);
                }
            }

            // Keep it light: at most ~384 samples each (even stride).
            auto thin = [](std::vector<glm::vec3>& pts, std::vector<uint16_t>* owners)
            {
                if (pts.size() <= 384) return;
                const size_t stride = (pts.size() + 383) / 384;
                size_t out = 0;
                for (size_t i = 0; i < pts.size(); i += stride)
                {
                    pts[out] = pts[i];
                    if (owners != nullptr) (*owners)[out] = (*owners)[i];
                    ++out;
                }
                pts.resize(out);
                if (owners != nullptr) owners->resize(out);
            };
            thin(sev.partPoints, &sev.partPointBones);
            thin(sev.ownPoints, nullptr);
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

        // The core body (Profile::corePath) is a model like any other: it loads on its own thread and is requested here, on the game
        // thread. Until its geometry is there the profile is not cached (hits are kept). A core that never loads is given up after
        // a while: the cavities then use the model's own mesh.
        const roj::SkinnedModel* coreModel = nullptr;
        if (!p.corePath.empty())
        {
            const roj::SkinnedModel* loading = AssetRegistry::GetSkinnedModelFromFile(p.corePath);
            const bool ready = loading != nullptr && !loading->meshes.empty() && loading->skeleton.boneCount > 0
                && std::all_of(loading->meshes.begin(), loading->meshes.end(), [&](const roj::SkinnedMesh& m)
                   { return m.boneIndexFirst.size() == static_cast<size_t>(loading->skeleton.boneCount) + 1; });
            if (ready)
            {
                coreModel = loading;
            }
            else if (++g_coreWaits[modelPath] < kCoreWaitCalls)
            {
                return g_loading;
            }
            else
            {
                Logger::Warning("Gore: the core body '%s' did not load, '%s' fills its holes from its own mesh", p.corePath.c_str(), modelPath.c_str());
            }
        }

        // The flesh (shells and anatomy) takes a few milliseconds on a detailed model: build it on the worker. The worker reads
        // a snapshot of the profile and the model's meshes, which are not modified once the model is loaded and partitioned.
        auto pending = std::make_shared<Profile::PendingFlesh>();
        auto snapshot = std::make_shared<const Profile>(p);
        const roj::SkinnedModel* modelPtr = &model;
        p.pendingFlesh = pending;
        BuildPool().QueueJob([pending, snapshot, modelPtr, coreModel]()
        {
            pending->result = BuildModelFlesh(*modelPtr, *snapshot, coreModel);
            pending->done.store(true, std::memory_order_release);
        });

        g_profiles[modelPath] = std::move(p);
        return g_loading;
    }
}
