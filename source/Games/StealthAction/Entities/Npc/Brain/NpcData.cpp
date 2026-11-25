#include "NpcData.h"

#include <AiPerception/Observer.h>
#include <FileSystem/FileSystem.h>
#include <Logger.hpp>

#include <mutex>

namespace
{
	std::mutex dataMutex;

	NpcPerceptionTuning perception;
	bool perceptionLoaded = false;

	NpcSituationTuning situations;
	bool situationsLoaded = false;

	NpcBarkTuning barkTuning;
	bool barksLoaded = false;

	std::map<std::string, NpcProfile> profiles;

	// Parses a JSON file into `value`; the defaults stay where the file or a key is missing.
	template <typename T>
	bool LoadJsonFile(const std::string& path, T& value)
	{
		const std::string text = FileSystemEngine::ReadFile(path);
		if (text.empty())
		{
			Logger::Warning("NPC data file not found, using defaults: %s", path.c_str());
			return false;
		}

		const nlohmann::json parsed = nlohmann::json::parse(text, nullptr, false);
		if (parsed.is_discarded())
		{
			Logger::Error("NPC data file is not valid JSON: %s", path.c_str());
			return false;
		}

		value = parsed.get<T>();
		return true;
	}
}

void NpcVisionTuning::ApplyTo(Observer& observer) const
{
	observer.useVisionProfile = true;

	VisionProfile& vision = observer.vision;
	vision.focusRange = focusRange;
	vision.focusHalfAngleDeg = focusHalfAngle;
	vision.focusWideRange = focusWideRange;
	vision.focusWideHalfAngleDeg = focusWideHalfAngle;
	vision.peripheralRange = peripheralRange;
	vision.peripheralHalfAngleDeg = peripheralHalfAngle;
	vision.peripheralFactor = peripheralFactor;
	vision.nearRange = nearRange;
	vision.nearFactor = nearFactor;
	vision.hysteresis = hysteresis;
	vision.closeDistance = closeDistance;
	vision.farFactor = farFactor;
	vision.darkFactor = darkFactor;
	vision.litLevel = litLevel;
}

void NpcPerceptionTuning::Load()
{
	std::lock_guard<std::mutex> lock(dataMutex);

	if (perceptionLoaded) return;
	perceptionLoaded = true;

	LoadJsonFile("GameData/ai/perception.json", perception);
}

const NpcPerceptionTuning& NpcPerceptionTuning::Get()
{
	Load();
	return perception;
}

void NpcSituationTuning::Load()
{
	std::lock_guard<std::mutex> lock(dataMutex);

	if (situationsLoaded) return;
	situationsLoaded = true;

	LoadJsonFile("GameData/ai/situations.json", situations);
}

const NpcSituationTuning& NpcSituationTuning::Get()
{
	Load();
	return situations;
}

const NpcSoundTuning& NpcPerceptionTuning::SoundFor(const std::string& name) const
{
	static const NpcSoundTuning fallback;

	const auto found = sounds.find(name);
	return found == sounds.end() ? fallback : found->second;
}

const NpcProfile& NpcProfile::Get(const std::string& id)
{
	std::lock_guard<std::mutex> lock(dataMutex);

	const auto found = profiles.find(id);
	if (found != profiles.end())
		return found->second;

	NpcProfile profile;
	profile.id = id;
	LoadJsonFile("GameData/ai/roles/" + id + ".json", profile);
	profile.id = id;

	return profiles.emplace(id, std::move(profile)).first->second;
}

void NpcBarkTuning::Load()
{
	std::lock_guard<std::mutex> lock(dataMutex);

	if (barksLoaded) return;
	barksLoaded = true;

	LoadJsonFile("GameData/ai/barks.json", barkTuning);
}

const NpcBarkTuning& NpcBarkTuning::Get()
{
	Load();
	return barkTuning;
}

std::string NpcBarkTuning::EventFor(const std::string& voice, const std::string& bark, int tier)
{
	return "event:/Character/" + voice + "/" + bark + "_t" + std::to_string(tier);
}

int NpcBarkTuning::TierForUse(int use) const
{
	if (use < tierUses[0]) return 1;
	if (use < tierUses[1]) return 2;
	return 3;
}

float NpcBarkTuning::PriorityOf(const std::string& bark) const
{
	const auto found = barks.find(bark);
	return found == barks.end() ? Bark().priority : found->second.priority;
}
