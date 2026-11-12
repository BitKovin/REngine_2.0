#include "GoreSource.h"

#include <FileSystem/FileSystem.h>
#include <Logger.hpp>
#include <json.hpp>

#include <unordered_map>
#include <vector>

namespace Gore
{
    namespace
    {
        std::vector<Source> g_sources;
        std::unordered_map<std::string, uint16_t> g_idToIndex;
        std::unordered_map<std::string, uint16_t> g_classToIndex;
        std::vector<std::string> g_tags;

        SourceType ParseType(const std::string& t)
        {
            if (t == "heavyBullet") return SourceType::HeavyBullet;
            if (t == "pellet") return SourceType::Pellet;
            if (t == "slash") return SourceType::Slash;
            if (t == "blunt") return SourceType::Blunt;
            if (t == "blast") return SourceType::Blast;
            return SourceType::Bullet;
        }

        void EnsureDefault()
        {
            if (!g_sources.empty()) return;
            Source s;
            s.id = "default";
            g_sources.push_back(s);
            g_idToIndex["default"] = 0;
        }
    }

    uint32_t TagBit(const std::string& tag)
    {
        for (size_t i = 0; i < g_tags.size(); ++i)
            if (g_tags[i] == tag) return 1u << i;

        if (g_tags.size() >= 32)
        {
            Logger::Error("Gore: more than 32 source tags, '%s' ignored", tag.c_str());
            return 0;
        }

        g_tags.push_back(tag);
        return 1u << (g_tags.size() - 1);
    }

    void LoadSources()
    {
        g_sources.clear();
        g_idToIndex.clear();
        g_classToIndex.clear();
        g_tags.clear();

        // Index 0 is always "default".
        EnsureDefault();

        const std::string text = FileSystemEngine::ReadFile("GameData/gore/sources.json");
        nlohmann::json j = nlohmann::json::parse(text, nullptr, false);
        if (j.is_discarded() || !j.is_object())
        {
            Logger::Error("Gore: GameData/gore/sources.json missing or invalid, only the default source exists");
            return;
        }

        if (j.contains("sources") && j["sources"].is_object())
        {
            for (auto it = j["sources"].begin(); it != j["sources"].end(); ++it)
            {
                const nlohmann::json& v = it.value();
                Source s;
                s.id = it.key();
                s.type = ParseType(v.value("type", std::string("bullet")));
                s.woundRadius = v.value("woundRadius", s.woundRadius);
                s.refDamage = v.value("refDamage", s.refDamage);
                s.clusterRadius = v.value("clusterRadius", s.clusterRadius);
                s.gashLength = v.value("gashLength", s.gashLength);
                s.severPower = v.value("severPower", s.severPower);
                s.exit = v.value("exit", s.exit);
                if (s.refDamage <= 0.0f) s.refDamage = 20.0f;

                if (v.contains("tags") && v["tags"].is_array())
                    for (const auto& tag : v["tags"])
                        if (tag.is_string()) s.tagBits |= TagBit(tag.get<std::string>());

                if (s.id == "default")
                {
                    g_sources[0] = s;
                    continue;
                }

                g_idToIndex[s.id] = static_cast<uint16_t>(g_sources.size());
                g_sources.push_back(s);
            }
        }

        if (j.contains("classMap") && j["classMap"].is_object())
        {
            for (auto it = j["classMap"].begin(); it != j["classMap"].end(); ++it)
            {
                if (!it.value().is_string()) continue;
                auto id = g_idToIndex.find(it.value().get<std::string>());
                if (id == g_idToIndex.end())
                {
                    Logger::Error("Gore: classMap '%s' points to unknown source '%s'", it.key().c_str(), it.value().get<std::string>().c_str());
                    continue;
                }
                g_classToIndex[it.key()] = id->second;
            }
        }
    }

    uint16_t SourceIndexForClass(const std::string& className)
    {
        auto it = g_classToIndex.find(className);
        return it != g_classToIndex.end() ? it->second : 0;
    }

    const Source& GetSource(uint16_t index)
    {
        EnsureDefault();
        return index < g_sources.size() ? g_sources[index] : g_sources[0];
    }
}
