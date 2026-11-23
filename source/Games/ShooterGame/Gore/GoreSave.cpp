#include "GoreSave.h"

#include "GoreComponent.h"
#include "GoreRenderState.h"

#include <Entity.h>
#include <Time.hpp>

#include <algorithm>

namespace Gore
{
    namespace
    {
        using json = nlohmann::json;

        json Vec3(const glm::vec3& v) { return json::array({ v.x, v.y, v.z }); }
        json Vec4(const glm::vec4& v) { return json::array({ v.x, v.y, v.z, v.w }); }

        glm::vec3 ReadVec3(const json& j, const char* key, const glm::vec3& fallback)
        {
            if (!j.contains(key) || !j[key].is_array() || j[key].size() != 3) return fallback;
            return glm::vec3(j[key][0].get<float>(), j[key][1].get<float>(), j[key][2].get<float>());
        }

        glm::vec4 ReadVec4(const json& j, const char* key, const glm::vec4& fallback)
        {
            if (!j.contains(key) || !j[key].is_array() || j[key].size() != 4) return fallback;
            return glm::vec4(j[key][0].get<float>(), j[key][1].get<float>(), j[key][2].get<float>(), j[key][3].get<float>());
        }

        template <typename E>
        E ReadEnum(const json& j, const char* key, E fallback, int maxValue)
        {
            const int v = j.value(key, static_cast<int>(fallback));
            return (v >= 0 && v <= maxValue) ? static_cast<E>(v) : fallback;
        }

        json WoundToJson(const Wound& w, double clockNow)
        {
            json j;
            j["center"] = Vec3(w.center);
            j["rotation"] = Vec4(glm::vec4(w.rotation.x, w.rotation.y, w.rotation.z, w.rotation.w));
            j["radii"] = Vec3(w.radii);
            j["anchorSkin"] = w.anchorSkin;
            j["anchorSkel"] = w.anchorSkel;
            j["kind"] = static_cast<int>(w.kind);
            j["roughness"] = w.roughness;
            j["rimWidth"] = w.rimWidth;
            j["seed"] = w.seed;
            j["reach"] = w.reach;
            j["healStart"] = w.healStart - clockNow;   // regen clock, relative to now
            j["healDuration"] = w.healDuration;
            j["order"] = w.order;
            j["taper"] = w.taper;
            j["limitSkel"] = w.limitSkel;
            return j;
        }

        Wound WoundFromJson(const json& j, double clockNow)
        {
            Wound w;
            w.center = ReadVec3(j, "center", w.center);
            const glm::vec4 q = ReadVec4(j, "rotation", glm::vec4(0, 0, 0, 1));
            w.rotation = glm::normalize(glm::quat(q.w, q.x, q.y, q.z));
            w.radii = ReadVec3(j, "radii", w.radii);
            w.anchorSkin = j.value("anchorSkin", w.anchorSkin);
            w.anchorSkel = j.value("anchorSkel", w.anchorSkel);
            w.kind = ReadEnum(j, "kind", w.kind, static_cast<int>(WoundKind::Crush));
            w.roughness = j.value("roughness", w.roughness);
            w.rimWidth = j.value("rimWidth", w.rimWidth);
            w.seed = j.value("seed", w.seed);
            w.reach = j.value("reach", w.reach);
            w.healStart = clockNow + j.value("healStart", 0.0);
            w.healDuration = j.value("healDuration", w.healDuration);
            w.order = j.value("order", w.order);
            w.taper = j.value("taper", w.taper);
            w.limitSkel = j.value("limitSkel", w.limitSkel);
            return w;
        }

        json CutToJson(const Cut& c, double clockNow, double now)
        {
            json j;
            j["plane"] = Vec4(c.plane);
            j["center"] = Vec3(c.center);
            j["influenceRadius"] = c.influenceRadius;
            j["boneRadius"] = c.boneRadius;
            j["outlineRadius"] = c.outlineRadius;
            j["roughness"] = c.roughness;
            j["seed"] = c.seed;
            j["boneSkel"] = c.boneSkel;
            j["boneSkin"] = c.boneSkin;
            j["phase"] = static_cast<int>(c.phase);
            j["severedAt"] = c.severedAt - now;          // game time, relative
            j["regrowStart"] = c.regrowStart - clockNow;  // regen clock, relative
            j["regrowDuration"] = c.regrowDuration;
            j["regrowMaxRadius"] = c.regrowMaxRadius;
            j["hitboxesBack"] = c.hitboxesBack;
            j["charred"] = c.charred;
            j["fxNext"] = c.fxNext - now;
            j["clean"] = c.clean;
            j["hasCap"] = c.cap != nullptr;               // the face itself is built again after the load
            j["stragglers"] = json::array({ c.stragglers[0], c.stragglers[1] });
            return j;
        }

        Cut CutFromJson(const json& j, double clockNow, double now)
        {
            Cut c;
            c.plane = ReadVec4(j, "plane", c.plane);
            c.center = ReadVec3(j, "center", c.center);
            c.influenceRadius = j.value("influenceRadius", c.influenceRadius);
            c.boneRadius = j.value("boneRadius", c.boneRadius);
            c.outlineRadius = j.value("outlineRadius", c.outlineRadius);
            c.roughness = j.value("roughness", c.roughness);
            c.seed = j.value("seed", c.seed);
            c.boneSkel = j.value("boneSkel", c.boneSkel);
            c.boneSkin = j.value("boneSkin", c.boneSkin);
            c.phase = ReadEnum(j, "phase", c.phase, static_cast<int>(CutPhase::Regrowing));
            c.severedAt = now + j.value("severedAt", 0.0);
            c.regrowStart = clockNow + j.value("regrowStart", 0.0);
            c.regrowDuration = j.value("regrowDuration", c.regrowDuration);
            c.regrowMaxRadius = j.value("regrowMaxRadius", c.regrowMaxRadius);
            c.hitboxesBack = j.value("hitboxesBack", c.hitboxesBack);
            c.charred = j.value("charred", c.charred);
            c.fxNext = now + j.value("fxNext", 0.0);
            c.clean = j.value("clean", c.clean);
            if (j.contains("stragglers") && j["stragglers"].is_array() && j["stragglers"].size() == 2)
            {
                c.stragglers[0] = j["stragglers"][0].get<uint64_t>();
                c.stragglers[1] = j["stragglers"][1].get<uint64_t>();
            }
            return c;
        }

        json SplatToJson(const Splat& s)
        {
            json j;
            j["center"] = Vec3(s.center);
            j["radius"] = s.radius;
            j["strength"] = s.strength;
            j["order"] = s.order;
            j["direction"] = Vec3(s.direction);
            j["kind"] = static_cast<int>(s.kind);
            return j;
        }

        Splat SplatFromJson(const json& j)
        {
            Splat s;
            s.center = ReadVec3(j, "center", s.center);
            s.radius = j.value("radius", s.radius);
            s.strength = j.value("strength", s.strength);
            s.order = j.value("order", s.order);
            s.direction = ReadVec3(j, "direction", s.direction);
            s.kind = ReadEnum(j, "kind", s.kind, static_cast<int>(DecalKind::Bruise));
            return s;
        }
    }

    void SaveGoreState(Entity* entity, json& target)
    {
        if (entity == nullptr || !entity->HasComponent<GoreComponent>()) return;
        const GoreComponent& gore = *entity->GetComponent<GoreComponent>();
        if (gore.isGib) return;
        if (gore.woundCount == 0 && gore.cutCount == 0 && gore.splatCount == 0 && gore.hitCounter == 0) return;

        const double now = Time::GameTime;
        const double clockNow = RegenClock(gore.frozenClock, gore.pausedUntil, now);

        json j;
        j["wounds"] = json::array();
        for (int i = 0; i < gore.woundCount; ++i) j["wounds"].push_back(WoundToJson(gore.wounds[i], clockNow));
        j["cuts"] = json::array();
        for (int i = 0; i < gore.cutCount; ++i) j["cuts"].push_back(CutToJson(gore.cuts[i], clockNow, now));
        j["splats"] = json::array();
        for (int i = 0; i < gore.splatCount; ++i) j["splats"].push_back(SplatToJson(gore.splats[i]));

        j["hitCounter"] = gore.hitCounter;
        j["orderCounter"] = gore.orderCounter;
        j["paused"] = std::max(gore.pausedUntil - now, 0.0);   // how long the healing is still suppressed
        j["severDamage"] = gore.severDamage;
        json times = json::array();
        for (double t : gore.severDamageTime) times.push_back(t - now);
        j["severDamageTime"] = times;

        target["gore"] = std::move(j);
    }

    void LoadGoreState(Entity* entity, const json& source)
    {
        if (entity == nullptr || !source.contains("gore") || !source["gore"].is_object()) return;
        const json& j = source["gore"];

        if (!entity->HasComponent<GoreComponent>()) entity->AddComponent<GoreComponent>();
        GoreComponent& gore = *entity->GetComponent<GoreComponent>();
        gore = GoreComponent();

        const double now = Time::GameTime;
        const double paused = j.value("paused", 0.0);
        gore.frozenClock = now;                        // the regen clock restarts at "now"; saved clock times are relative to it
        gore.pausedUntil = paused > 0.0 ? now + paused : now;
        const double clockNow = now;

        if (j.contains("wounds") && j["wounds"].is_array())
            for (const json& w : j["wounds"])
                if (gore.woundCount < kMaxWounds) gore.wounds[gore.woundCount++] = WoundFromJson(w, clockNow);
        if (j.contains("cuts") && j["cuts"].is_array())
            for (const json& c : j["cuts"])
            {
                if (gore.cutCount >= kMaxCuts) break;
                gore.capPending[gore.cutCount] = c.value("hasCap", false);
                gore.cuts[gore.cutCount++] = CutFromJson(c, clockNow, now);
            }
        if (j.contains("splats") && j["splats"].is_array())
            for (const json& s : j["splats"])
                if (gore.splatCount < kMaxSplats) gore.splats[gore.splatCount++] = SplatFromJson(s);

        gore.hitCounter = j.value("hitCounter", 0u);
        gore.orderCounter = j.value("orderCounter", 0u);
        if (j.contains("severDamage") && j["severDamage"].is_array())
            for (size_t i = 0; i < gore.severDamage.size() && i < j["severDamage"].size(); ++i)
                gore.severDamage[i] = j["severDamage"][i].get<float>();
        if (j.contains("severDamageTime") && j["severDamageTime"].is_array())
            for (size_t i = 0; i < gore.severDamageTime.size() && i < j["severDamageTime"].size(); ++i)
                gore.severDamageTime[i] = now + j["severDamageTime"][i].get<double>();

        gore.version = 2;                // the mesh starts at 0: it draws this state
        gore.restorePending = gore.cutCount > 0;   // caps and hitboxes, applied by the gore tick once the model is ready
    }
}
