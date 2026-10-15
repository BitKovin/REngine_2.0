#include "MoveSetData.h"

#include <Actions/MoveBehavior.h>
#include <Data/JsonTextPatch.h>

#include <glm.h>

#include <algorithm>

float ReachData::At(float time) const
{
	if (!IsActive())
		return 0.0f;

	float in = 1.0f;
	if (time < resolvedStart)
		in = blendIn > 0.0f ? glm::smoothstep(resolvedStart - blendIn, resolvedStart, time) : 0.0f;
	float out = 1.0f;
	if (time > resolvedEnd)
		out = blendOut > 0.0f ? 1.0f - glm::smoothstep(resolvedEnd, resolvedEnd + blendOut, time) : 0.0f;
	return length * std::min(in, out);
}

void MoveSetData::WriteJson(nlohmann::json& j, const MoveSetData& data)
{
	j = nlohmann::json::object();
	j["version"] = data.version;
	j["name"] = data.name;
	j["animations"] = data.animations;
	j["inputBuffer"] = data.inputBuffer;
	j["actionBuffer"] = data.actionBuffer;
	j["holdCarriesOver"] = data.holdCarriesOver;
	j["holdDelay"] = data.holdDelay;
	j["holdDelayInMove"] = data.holdDelayInMove;
	j["blendOut"] = data.blendOut;
	j["combos"] = data.combos;
	j["holdOpeners"] = data.holdOpeners;
	j["actions"] = data.actions;
	j["reactions"] = data.reactions;

	nlohmann::json moves = nlohmann::json::object();
	for (const MoveData& move : data.moves)
		moves[move.id] = move;
	j["moves"] = std::move(moves);
}

void MoveSetData::ReadJson(const nlohmann::json& j, MoveSetData& data)
{
	const MoveSetData defaults;
	data.version = j.value("version", defaults.version);
	data.name = j.value("name", defaults.name);
	data.animations = j.value("animations", defaults.animations);
	data.inputBuffer = j.value("inputBuffer", defaults.inputBuffer);
	data.actionBuffer = j.value("actionBuffer", defaults.actionBuffer);
	data.holdCarriesOver = j.value("holdCarriesOver", defaults.holdCarriesOver);
	data.holdDelay = j.value("holdDelay", defaults.holdDelay);
	data.holdDelayInMove = j.value("holdDelayInMove", defaults.holdDelayInMove);
	data.blendOut = j.value("blendOut", defaults.blendOut);
	data.combos = j.value("combos", defaults.combos);
	data.holdOpeners = j.value("holdOpeners", defaults.holdOpeners);
	data.actions = j.value("actions", defaults.actions);
	data.reactions = j.value("reactions", defaults.reactions);

	data.moves.clear();
	auto moves = j.find("moves");
	if (moves != j.end())
	{
		if (!moves->is_object())
			throw nlohmann::json::type_error::create(302, "'moves' must be an object of { \"id\": { move } }", &j);

		// nlohmann objects are key-sorted, so move indices are stable for a given file.
		for (auto it = moves->begin(); it != moves->end(); ++it)
		{
			if (!it.key().empty() && it.key()[0] == '_')
				continue;

			MoveData move = it->get<MoveData>();
			move.id = it.key();
			data.moves.push_back(std::move(move));
		}
	}
}

namespace
{
	bool ValidWindow(float start, float end) { return start >= 0.0f && (end < 0.0f || end >= start); }

	std::string MovePath(const MoveData& move) { return "moves." + move.id; }

	int ResolveMoveReference(const MoveSetData& data, const std::string& id, const std::string& path, DataLoadReport& report)
	{
		if (id.empty())
			return -1;

		const int index = data.FindMove(id);
		if (index < 0)
			report.Error(path, "unknown move '" + id + "'");
		return index;
	}

	template<size_t N, size_t M>
	void ResolveSlotTable(const MoveSetData& data, const std::map<std::string, std::string>& table, const char* const (&names)[N],
		std::array<int, M>& out, const std::string& tableName, DataLoadReport& report)
	{
		for (const auto& [slotName, moveId] : table)
		{
			const std::string path = tableName + "." + slotName;
			const int slot = FindSlotIndex(names, slotName);
			if (slot < 0)
			{
				std::string allowed;
				for (const char* name : names) { if (!allowed.empty()) allowed += ", "; allowed += name; }
				report.Error(path, "unknown key (allowed: " + allowed + ")");
				continue;
			}

			out[static_cast<size_t>(slot)] = ResolveMoveReference(data, moveId, path, report);
		}
	}

	void ResolveMove(MoveSetData& data, MoveData& move, DataLoadReport& report)
	{
		const std::string path = MovePath(move);

		// Animation source + clips
		move.clipHash = hashed_string(move.clip);
		move.variantHashes.clear();
		for (size_t i = 0; i < move.variants.size(); ++i)
		{
			if (move.variants[i].empty())
				report.Error(path + ".variants[" + std::to_string(i) + "]", "empty clip name");
			move.variantHashes.emplace_back(move.variants[i]);
		}

		if (move.clip.empty())
		{
			move.animationIndex = -1;
			if (move.endTime < 0.0f)
				report.Error(path + ".endTime", "required when the move has no clip");
			if (!move.variants.empty())
				report.Error(path + ".variants", "require a clip (variants share the clip's source and timing)");
			if (!move.animation.empty())
				report.Warning(path + ".animation", "ignored: the move has no clip");
		}
		else if (move.animation.empty())
		{
			if (data.animations.size() == 1)
				move.animationIndex = 0;
			else
				report.Error(path + ".animation", "required when the moveset lists more than one entry in 'animations'");
		}
		else
		{
			auto it = std::find(data.animations.begin(), data.animations.end(), move.animation);
			if (it == data.animations.end())
				report.Error(path + ".animation", "'" + move.animation + "' is not listed in the moveset's 'animations' (it would not be preloaded)");
			else
				move.animationIndex = static_cast<int>(it - data.animations.begin());
		}

		if (move.speed <= 0.0f)
			report.Error(path + ".speed", "must be > 0");
		if (move.loop && move.loopStart < 0.0f)
			report.Error(path + ".loopStart", "must be >= 0");
		if (move.loop && move.endTime >= 0.0f && move.loopStart >= move.endTime)
			report.Error(path + ".loopStart", "must be before endTime");

		// Enums
		if (move.channel == MoveChannel::Invalid) report.Error(path + ".channel", "expected Primary or Overlay");
		if (move.bodyMask == BodyMask::Invalid) report.Error(path + ".bodyMask", "expected Full, NoLegs, UpperBody or Bones");
		move.maskBoneHashes.clear();
		for (size_t i = 0; i < move.maskBones.size(); ++i)
		{
			if (move.maskBones[i].empty())
				report.Error(path + ".maskBones[" + std::to_string(i) + "]", "empty bone name");
			move.maskBoneHashes.emplace_back(move.maskBones[i]);
		}
		if (move.legWeight < 0.0f || move.legWeight > 1.0f)
			report.Error(path + ".legWeight", "must be in [0, 1]");
		if (move.bodyMask == BodyMask::Bones && move.maskBones.empty())
			report.Error(path + ".maskBones", "required by bodyMask Bones");
		if (move.bodyMask != BodyMask::Bones && !move.maskBones.empty())
			report.Warning(path + ".maskBones", "ignored: bodyMask is not Bones");
		if (move.overlay == OverlayPolicy::Invalid) report.Error(path + ".overlay", "expected Block or Allow");
		if (move.startState == MoveStartState::Invalid) report.Error(path + ".startState", "expected Ground, Air or Any");

		// Tags
		move.tagMask = 0;
		for (size_t i = 0; i < move.tags.size(); ++i)
		{
			if (move.tags[i] == MoveTag::Invalid)
				report.Error(path + ".tags[" + std::to_string(i) + "]", "unknown tag");
			else if (move.tags[i] == MoveTag::Locomotion)
				report.Error(path + ".tags[" + std::to_string(i) + "]", "Locomotion is a cancel target (movement input), not a move tag");
			else
				move.tagMask |= TagBit(move.tags[i]);
		}
		if (move.heavy)
			move.tagMask |= TagBit(MoveTag::Heavy);

		// Entries
		const MoveEntryData* entries[] = { &move.entry.fresh, &move.entry.afterLight, &move.entry.afterHeavy };
		const char* entryNames[] = { "fresh", "afterLight", "afterHeavy" };
		for (int i = 0; i < 3; ++i)
		{
			if (entries[i]->blendIn < 0.0f) report.Error(path + ".entry." + entryNames[i] + ".blendIn", "must be >= 0");
			if (entries[i]->startTime < 0.0f) report.Error(path + ".entry." + entryNames[i] + ".startTime", "must be >= 0");
			if (move.endTime >= 0.0f && entries[i]->startTime >= move.endTime)
				report.Error(path + ".entry." + entryNames[i] + ".startTime", "must be before endTime");
		}

		move.resolvedBlendOut = move.blendOut < 0.0f ? data.blendOut : move.blendOut;

		// Hit windows
		for (size_t i = 0; i < move.hits.size(); ++i)
		{
			HitWindowData& hit = move.hits[i];
			const std::string hitPath = path + ".hits[" + std::to_string(i) + "]";
			if (hit.end < hit.start) report.Error(hitPath, "end must be >= start");
			if (hit.weapon == WeaponSlot::Count) report.Error(hitPath + ".weapon", "expected Main or Secondary");
			if (hit.source == DamageSource::Count) report.Error(hitPath + ".source", "unknown damage source");
			if (hit.reaction == HitReaction::Count) report.Error(hitPath + ".reaction", "unknown reaction");
			if (!hit.bone.empty() && hit.radius <= 0.0f) report.Error(hitPath + ".radius", "must be > 0");
			if (hit.direct && !hit.bone.empty()) report.Warning(hitPath + ".bone", "ignored for direct hits");
			if (hit.hitStop < 0.0f) report.Error(hitPath + ".hitStop", "must be >= 0");
			if (hit.hitStopScale <= 0.0f || hit.hitStopScale > 1.0f) report.Error(hitPath + ".hitStopScale", "must be in (0, 1]");
			if (hit.shake < 0.0f) report.Error(hitPath + ".shake", "must be >= 0");
			if (hit.camera < 0.0f) report.Error(hitPath + ".camera", "must be >= 0");
			if (hit.impactScale < 0.0f) report.Error(hitPath + ".impactScale", "must be >= 0");
			if (hit.air.stall < 0.0f) report.Error(hitPath + ".air.stall", "must be >= 0");
			hit.resolvedWeakPointDamage = hit.weakPointDamage < 0.0f ? hit.damage : hit.weakPointDamage;
			hit.boneHash = hashed_string(hit.bone);
		}

		// Combo link
		ComboLinkData& combo = move.combo;
		if (combo.windowStart >= 0.0f)
		{
			const std::string comboPath = path + ".combo";
			if (!(combo.windowStart <= combo.alternativeAt && combo.alternativeAt <= combo.windowEnd))
				report.Error(comboPath, "requires windowStart <= alternativeAt <= windowEnd");
			combo.nextIndex = ResolveMoveReference(data, combo.next, comboPath + ".next", report);
			combo.alternativeIndex = ResolveMoveReference(data, combo.alternative, comboPath + ".alternative", report);
			if (combo.next.empty() && combo.alternative.empty())
				report.Warning(comboPath, "window defined but neither 'next' nor 'alternative' is set");
		}
		else if (!combo.next.empty() || !combo.alternative.empty())
		{
			report.Error(path + ".combo.windowStart", "follow-ups are set but the window is missing");
		}

		// Hold (§5.4)
		HoldData& hold = move.hold;
		hold.releaseIndex = -1;
		if (hold.IsValid())
		{
			const std::string holdPath = path + ".hold";
			if (hold.release.empty())
				report.Error(holdPath + ".release", "required with hold.until (the move taking over on an early release)");
			else
				hold.releaseIndex = ResolveMoveReference(data, hold.release, holdPath + ".release", report);
			if (move.channel != MoveChannel::Primary)
				report.Error(holdPath, "only Primary moves can hold");
			for (const HitWindowData& hit : move.hits)
			{
				if (hit.start < hold.until)
				{
					report.Warning(holdPath + ".until", "a hit window opens before the move commits - an early release would cut a landed strike");
					break;
				}
			}
		}
		else if (!hold.release.empty())
		{
			report.Error(path + ".hold.until", "release is set but until is missing");
		}

		// Cancel windows
		for (size_t i = 0; i < move.cancels.size(); ++i)
		{
			CancelWindowData& cancel = move.cancels[i];
			const std::string cancelPath = path + ".cancels[" + std::to_string(i) + "]";
			if (!ValidWindow(cancel.start, cancel.end)) report.Error(cancelPath, "invalid start/end");
			cancel.intoMask = 0;
			for (MoveTag tag : cancel.into)
			{
				if (tag == MoveTag::Invalid) report.Error(cancelPath + ".into", "unknown tag");
				else cancel.intoMask |= TagBit(tag);
			}
			if (cancel.intoMask == 0) report.Warning(cancelPath + ".into", "empty - this window allows nothing");
			if ((cancel.intoMask & TagBit(MoveTag::Locomotion)) && move.combo.IsValid() && cancel.start < move.combo.alternativeAt)
				report.Warning(cancelPath, "a Locomotion cancel inside the light zone ends the combo while the stick steers it - open it at combo.alternativeAt or later");
		}

		for (size_t i = 0; i < move.armor.size(); ++i)
		{
			const ArmorWindowData& armor = move.armor[i];
			const std::string armorPath = path + ".armor[" + std::to_string(i) + "]";
			if (!ValidWindow(armor.start, armor.end)) report.Error(armorPath, "invalid start/end");
			if (armor.poiseResist < 0.0f || armor.poiseResist > 1.0f) report.Error(armorPath + ".poiseResist", "must be in [0, 1]");
		}

		for (size_t i = 0; i < move.invulnerable.size(); ++i)
			if (!ValidWindow(move.invulnerable[i].start, move.invulnerable[i].end))
				report.Error(path + ".invulnerable[" + std::to_string(i) + "]", "invalid start/end");

		for (size_t i = 0; i < move.cues.size(); ++i)
		{
			const CueData& cue = move.cues[i];
			const std::string cuePath = path + ".cues[" + std::to_string(i) + "]";
			if (cue.trail < -1 || cue.trail > 1) report.Error(cuePath + ".trail", "expected -1, 0 or 1");
			if (cue.trailWeapon == WeaponSlot::Count) report.Error(cuePath + ".trailWeapon", "expected Main or Secondary");
		}

		// Tracking: windows default to the hit windows (turn until the first strike, pull until the last one ends).
		TrackingData& tracking = move.tracking;
		if (tracking.turnRate < 0.0f) report.Error(path + ".tracking.turnRate", "must be >= 0");
		if (tracking.startTurnRate < 0.0f) report.Error(path + ".tracking.startTurnRate", "must be >= 0");
		if (tracking.startTurnTime < 0.0f) report.Error(path + ".tracking.startTurnTime", "must be >= 0");
		if (tracking.turnSharpness <= 0.0f) report.Error(path + ".tracking.turnSharpness", "must be > 0");
		if (tracking.searchRange < 0.0f) report.Error(path + ".tracking.searchRange", "must be >= 0");
		if (tracking.magnetRange < 0.0f) report.Error(path + ".tracking.magnetRange", "must be >= 0");
		if (tracking.idealDistance < 0.0f) report.Error(path + ".tracking.idealDistance", "must be >= 0");
		if (tracking.maxMagnetSpeed < 0.0f) report.Error(path + ".tracking.maxMagnetSpeed", "must be >= 0");
		if (tracking.advance < 0.0f) report.Error(path + ".tracking.advance", "must be >= 0");
		float firstHit = -1.0f;
		float lastHit = -1.0f;
		for (const HitWindowData& hit : move.hits)
		{
			firstHit = firstHit < 0.0f ? hit.start : std::min(firstHit, hit.start);
			lastHit = std::max(lastHit, hit.end);
		}
		tracking.resolvedUntil = tracking.until >= 0.0f ? tracking.until : std::max(firstHit, 0.0f);
		tracking.resolvedMagnetUntil = tracking.magnetUntil >= 0.0f ? tracking.magnetUntil : std::max(lastHit, tracking.resolvedUntil);

		// Reach: fully out over the blade hits of its weapon by default.
		ReachData& reach = move.reach;
		const std::string reachPath = path + ".reach";
		if (reach.length < 0.0f) report.Error(reachPath + ".length", "must be >= 0");
		if (reach.blendIn < 0.0f) report.Error(reachPath + ".blendIn", "must be >= 0");
		if (reach.blendOut < 0.0f) report.Error(reachPath + ".blendOut", "must be >= 0");
		if (reach.weapon == WeaponSlot::Count) report.Error(reachPath + ".weapon", "expected Main or Secondary");
		float firstBlade = -1.0f;
		float lastBlade = -1.0f;
		for (const HitWindowData& hit : move.hits)
		{
			if (!hit.bone.empty() || hit.direct || hit.weapon != reach.weapon)
				continue;
			firstBlade = firstBlade < 0.0f ? hit.start : std::min(firstBlade, hit.start);
			lastBlade = std::max(lastBlade, hit.end);
		}
		reach.resolvedStart = reach.start >= 0.0f ? reach.start : std::max(firstBlade, 0.0f);
		reach.resolvedEnd = reach.end >= 0.0f ? reach.end : std::max(lastBlade, reach.resolvedStart);
		if (reach.IsActive())
		{
			if ((reach.start < 0.0f || reach.end < 0.0f) && firstBlade < 0.0f)
				report.Error(reachPath, "no blade hit of its weapon to time it by - set start and end");
			else if (reach.resolvedEnd < reach.resolvedStart)
				report.Error(reachPath, "end must be >= start");
		}

		// Recovery
		move.recoveryStart = -1.0f;
		if (move.recovery.enabled)
		{
			if (move.recovery.start >= 0.0f)
				move.recoveryStart = move.recovery.start;
			else if (move.combo.IsValid())
				move.recoveryStart = move.combo.windowEnd;
		}
		if (move.recovery.lowerBodyBlend < 0.0f)
			report.Error(path + ".recovery.lowerBodyBlend", "must be >= 0");
		if (move.recoveryStart >= 0.0f)
		{
			if (!move.HasClip())
				report.Error(path + ".recovery", "a move without a clip has nothing to recover with - disable recovery");
			if (move.channel != MoveChannel::Primary)
				report.Error(path + ".recovery", "only Primary moves can recover");
			if (move.combo.IsValid() && move.recoveryStart < move.combo.windowEnd)
				report.Error(path + ".recovery.start", "must not be before combo.windowEnd (the combo window would be cut short)");
			if (move.endTime >= 0.0f && move.recoveryStart >= move.endTime)
				report.Warning(path + ".recovery.start", "is at or after endTime - the move ends before recovering");
		}

		// Behaviour
		move.behaviorType = MoveBehaviorRegistry::Find(move.behavior);
		if (move.behaviorType < 0)
		{
			report.Error(path + ".behavior", "unknown behaviour '" + move.behavior + "' (known: " + MoveBehaviorRegistry::KnownNames() + ")");
		}
		else
		{
			const MoveBehaviorType& type = MoveBehaviorRegistry::Get(move.behaviorType);
			move.behaviorParams = type.ParseParams(move.params, path + ".params", report);
		}
	}
}

bool ResolveMoveSet(MoveSetData& data, DataLoadReport& report)
{
	if (data.version != 1)
		report.Warning("version", "expected 1, got " + std::to_string(data.version));

	if (data.inputBuffer < 0.0f) report.Error("inputBuffer", "must be >= 0");
	if (data.actionBuffer < 0.0f) report.Error("actionBuffer", "must be >= 0");
	if (data.holdDelay < 0.0f) report.Error("holdDelay", "must be >= 0");
	if (data.holdDelayInMove < data.holdDelay) report.Error("holdDelayInMove", "must be >= holdDelay");
	if (data.blendOut < 0.0f) report.Error("blendOut", "must be >= 0");

	data.moveIndexById.clear();
	for (size_t i = 0; i < data.moves.size(); ++i)
	{
		data.moves[i].index = static_cast<int>(i);
		data.moveIndexById[data.moves[i].id] = static_cast<int>(i);
	}

	if (data.moves.empty())
		report.Error("moves", "a moveset needs at least one move");

	for (MoveData& move : data.moves)
		ResolveMove(data, move, report);

	data.comboOpeners.fill(-1);
	data.holdOpenerMoves.fill(-1);
	data.actionMoves.fill(-1);
	data.reactionMoves.fill(-1);
	ResolveSlotTable(data, data.combos, ComboSlotNames, data.comboOpeners, "combos", report);
	ResolveSlotTable(data, data.holdOpeners, ComboSlotNames, data.holdOpenerMoves, "holdOpeners", report);
	for (size_t i = 0; i < data.holdOpenerMoves.size(); ++i)
		if (data.holdOpenerMoves[i] >= 0 && data.comboOpeners[i] < 0)
			report.Error(std::string("holdOpeners.") + ComboSlotNames[i], "needs a combo opener in the same slot (a press released early starts it)");
	ResolveSlotTable(data, data.actions, ActionSlotNames, data.actionMoves, "actions", report);
	ResolveSlotTable(data, data.reactions, ReactionSlotNames, data.reactionMoves, "reactions", report);

	// Channel sanity for table entries
	for (size_t i = 0; i < data.reactionMoves.size(); ++i)
	{
		const MoveData* move = data.GetMove(data.reactionMoves[i]);
		if (move && move->channel != MoveChannel::Primary)
			report.Error(std::string("reactions.") + ReactionSlotNames[i], "reaction moves must use the Primary channel");
	}

	return report.Ok();
}

const nlohmann::json& DefaultMoveJson()
{
	static const nlohmann::json move = nlohmann::json(MoveData());
	return move;
}

const nlohmann::json* MoveArrayElementDefaults(const std::string& field)
{
	static const std::map<std::string, nlohmann::json> defaults = {
		{ "hits", nlohmann::json(HitWindowData()) },
		{ "cancels", nlohmann::json(CancelWindowData()) },
		{ "armor", nlohmann::json(ArmorWindowData()) },
		{ "invulnerable", nlohmann::json(TimeWindowData()) },
		{ "cues", nlohmann::json(CueData()) },
	};
	const auto it = defaults.find(field);
	return it == defaults.end() ? nullptr : &it->second;
}

namespace
{
	const nlohmann::json* ElementDefaults(const std::string& field)
	{
		return MoveArrayElementDefaults(field);
	}

	nlohmann::json StripDefaults(const nlohmann::json& value, const nlohmann::json& defaults)
	{
		if (!value.is_object() || !defaults.is_object())
			return value;

		nlohmann::json out = nlohmann::json::object();
		for (auto it = value.begin(); it != value.end(); ++it)
		{
			const auto fallback = defaults.find(it.key());
			if (fallback != defaults.end() && *fallback == it.value())
				continue;

			const nlohmann::json* elements = it.value().is_array() ? ElementDefaults(it.key()) : nullptr;
			if (elements)
			{
				nlohmann::json array = nlohmann::json::array();
				for (const nlohmann::json& element : it.value())
					array.push_back(StripDefaults(element, *elements));
				out[it.key()] = std::move(array);
			}
			else if (fallback != defaults.end() && fallback->is_object())
			{
				out[it.key()] = StripDefaults(it.value(), *fallback);
			}
			else
			{
				out[it.key()] = it.value();
			}
		}
		return out;
	}

	// New moveset content at `path` without the fields that only repeat defaults.
	nlohmann::json StripMoveSetDefaults(const JsonTextPatch::Path& path, const nlohmann::json& value)
	{
		const nlohmann::json& move = DefaultMoveJson();
		if (path.size() < 2 || path[0] != "moves")
			return value;
		if (path.size() == 2)
			return StripDefaults(value, move);

		const std::string& field = path[2];
		const nlohmann::json* elements = ElementDefaults(field);
		if (path.size() == 3)
		{
			if (elements && value.is_array())
			{
				nlohmann::json array = nlohmann::json::array();
				for (const nlohmann::json& element : value)
					array.push_back(StripDefaults(element, *elements));
				return array;
			}
			const auto fallback = move.find(field);
			return fallback != move.end() ? StripDefaults(value, *fallback) : value;
		}
		if (path.size() == 4 && elements)
			return StripDefaults(value, *elements);
		return value;
	}
}

namespace
{
	nlohmann::json BehaviorDefaultParams(const std::string& behavior)
	{
		const int type = MoveBehaviorRegistry::Find(behavior);
		if (type < 0 || MoveBehaviorRegistry::Get(type).DefaultParams == nullptr)
			return nlohmann::json::object();
		nlohmann::json defaults = MoveBehaviorRegistry::Get(type).DefaultParams();
		return defaults.is_object() ? defaults : nlohmann::json::object();
	}

	// Objects merge key by key (a partial authored object keeps the other defaults); a number written as 18 stays a
	// float where the parameter is one.
	void MergeParams(nlohmann::json& target, const nlohmann::json& source)
	{
		if (!source.is_object())
			return;
		for (auto it = source.begin(); it != source.end(); ++it)
		{
			const bool known = target.contains(it.key());
			if (known && target[it.key()].is_object() && it.value().is_object())
				MergeParams(target[it.key()], it.value());
			else if (known && target[it.key()].is_number_float() && it.value().is_number())
				target[it.key()] = it.value().get<double>();
			else
				target[it.key()] = it.value();
		}
	}

	nlohmann::json KeepAuthored(const nlohmann::json& edited, const nlohmann::json& defaults, const nlohmann::json& authored)
	{
		nlohmann::json out = nlohmann::json::object();
		for (auto it = edited.begin(); it != edited.end(); ++it)
		{
			const bool written = authored.is_object() && authored.contains(it.key());
			const bool hasDefault = defaults.is_object() && defaults.contains(it.key());
			if (it.value().is_object() && hasDefault && defaults[it.key()].is_object())
			{
				nlohmann::json inner = KeepAuthored(it.value(), defaults[it.key()], written ? authored[it.key()] : nlohmann::json::object());
				if (!inner.empty() || written)
					out[it.key()] = std::move(inner);
			}
			else if (written || !hasDefault || defaults[it.key()] != it.value())
			{
				out[it.key()] = it.value();
			}
		}
		return out;
	}
}

nlohmann::json MoveParamsWithDefaults(const MoveData& move)
{
	nlohmann::json params = BehaviorDefaultParams(move.behavior);
	MergeParams(params, move.params);
	return params;
}

nlohmann::json MoveParamsToStore(const nlohmann::json& edited, const MoveData& move)
{
	return KeepAuthored(edited, BehaviorDefaultParams(move.behavior), move.params);
}

bool LoadMoveSetText(const std::string& text, MoveSetData& out, std::string& error)
{
	DataLoadReport report;
	nlohmann::json json;
	const bool ok = ParseDataJson(text, json, report) && ConvertDataJson(json, out, "", report) && ResolveMoveSet(out, report);
	if (!ok)
		error = report.Errors.empty() ? std::string("invalid moveset") : report.Errors.front();
	return ok;
}

bool PatchMoveSetText(const std::string& text, const MoveSetData& base, const MoveSetData& edited, std::string& out, std::string& error)
{
	MoveSetData current;
	if (!LoadMoveSetText(text, current, error))
	{
		error = "the file on disk does not load (" + error + ") - fix it by hand first";
		return false;
	}

	const nlohmann::json baseJson = base;
	const nlohmann::json editedJson = edited;
	if (!JsonTextPatch::Apply(text, baseJson, editedJson, StripMoveSetDefaults, out, error))
		return false;

	// The saved file has to mean what the file means now plus exactly the edits - checked by loading it.
	nlohmann::json expected = current;
	JsonTextPatch::Merge(expected, baseJson, editedJson);
	MoveSetData saved;
	if (!LoadMoveSetText(out, saved, error))
	{
		error = "the patched file would not load (" + error + ") - not saved";
		return false;
	}
	if (nlohmann::json(saved) != expected)
	{
		error = "the patched file would not load back to the edited data - not saved";
		return false;
	}
	return true;
}
