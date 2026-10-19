#include "MoveSetEditor.h"

#ifndef DISTRIBUTION

#include <Actions/MoveBehavior.h>
#include <Combat/CombatTypes.h>
#include <Data/CombatData.h>

#include <imgui/imgui.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <iterator>
#include <map>
#include <string>
#include <vector>

using Json = nlohmann::json;

namespace
{
	const ImVec4 DimColor(0.55f, 0.55f, 0.6f, 1.0f);
	const ImVec4 ErrorColor(1.0f, 0.42f, 0.42f, 1.0f);
	const ImVec4 WarningColor(1.0f, 0.8f, 0.35f, 1.0f);
	const ImVec4 OkColor(0.45f, 0.9f, 0.5f, 1.0f);

	template<typename Enum>
	std::vector<std::string> EnumNames()
	{
		std::size_t count = 0;
		const auto* table = nlohmann::adl_serializer<Enum>::Table(count);
		std::vector<std::string> names;
		for (std::size_t i = 0; i < count; ++i)
			if (table[i].second != nullptr)
				names.emplace_back(table[i].second);
		return names;
	}

	template<std::size_t N>
	std::vector<std::string> NameList(const char* const (&names)[N])
	{
		return std::vector<std::string>(std::begin(names), std::end(names));
	}

	// ---- What every field means: "<section>.<key>" (section = the move field it sits under, "params" for behaviour
	// parameters, "moveset" for the moveset's own fields), then "<key>" ----
	const std::map<std::string, const char*>& Help()
	{
		static const std::map<std::string, const char*> help = {
			// Moveset
			{ "moveset.inputBuffer", "Seconds an Attack press waits for a combo window or a cancel to open (combos queue up)." },
			{ "moveset.actionBuffer", "Seconds a Dash / Jump / Secondary / Finisher press waits: these should happen now or not at all." },
			{ "moveset.holdCarriesOver", "The hold that started an attack counts as held for its combo link: holding through an attack reaches its heavy." },
			{ "moveset.holdDelay", "Seconds Attack must be held from rest before the hold opener (the launcher) starts - it shows its wind-up\nand still falls back to the usual opener when released during it. Released earlier: the usual opener." },
			{ "moveset.holdDelayInMove", "Seconds Attack must be held during a move (a combo, a dash) before the hold opener (the launcher) starts;\nit commits at once. Longer than a heavy burst: released earlier, a press gives its light / heavy." },
			{ "moveset.blendOut", "Seconds a move blends back to locomotion when it ends, unless the move sets its own." },
			{ "moveset.combos", "The opener of each combo: Attack (ground, and the air too when there is no AirAttack opener), AirAttack." },
			{ "moveset.holdOpeners", "The move a long Attack hold starts (the launcher): from rest after holdDelay, during a move after holdDelayInMove." },
			{ "moveset.actions", "The move each button / situation starts (Dash, AirDash, Jump, AirJump, Glide, Secondary, Finisher, Draw, AirDrop)." },
			{ "moveset.reactions", "The move a hit reaction plays (Flinch, Stagger, Knockdown, Launch, PoiseBreak, FinisherVictim, Death)." },
			{ "moveset.animations", "Clip source files of the moveset, loaded with the body." },
			// General
			{ "behavior", "Behaviour running the move: Attack, Dash, Jump, Glide, Fire, Draw, Reaction, Death, Finisher, FinisherVictim, Charge." },
			{ "clip", "Clip name in the animation file. Empty: no clip of its own (needs endTime)." },
			{ "variants", "More clips sharing the move's timing; the behaviour picks one (Fire: one per muzzle)." },
			{ "animation", "Clip source file; empty: the moveset's only animation file." },
			{ "speed", "Playback rate of the clip. Every time of the move is in clip seconds, so all windows scale with it." },
			{ "loop", "While the behaviour sustains the move, the clip loops between loopStart and endTime." },
			{ "loopStart", "Clip time a sustained loop wraps back to." },
			{ "heavy", "A heavy attack: follow-ups use their afterHeavy entry; it staggers unarmored bodies a light one only flinches." },
			{ "tags", "What the move is - cancel windows ('into') and the AI pick moves by these." },
			{ "channel", "Primary: the body. Overlay: on top of the primary move (firing, drawing)." },
			{ "bodyMask", "Where the clip shows. Full: whole body. NoLegs: the legs keep the locomotion / air pose. UpperBody: from the spine up. Bones: only maskBones." },
			{ "maskBones", "bodyMask Bones: the bones whose subtrees the clip shows on." },
			{ "legWeight", "NoLegs, and Full moves in the air: share of the legs taken from the locomotion / air pose." },
			{ "airLegs", "Full moves in the air blend the legs toward the air pose. Off: the clip poses its own air legs (jumps, dashes)." },
			{ "overlay", "Block: overlays (firing) stop and aiming is suspended while this runs. Allow: they go on." },
			{ "startState", "Where the move may start: Ground, Air or Any." },
			{ "endTime", "Clip time the move ends at; -1: the clip's length." },
			{ "blendOut", "Seconds the move blends out when it ends; -1: the moveset's blendOut." },
			{ "cooldown", "Seconds before the move can start again." },
			// Entry
			{ "fresh", "Started on its own (not from a combo link): blendIn (s) the previous pose blends in, startTime (clip s) it starts at." },
			{ "afterLight", "Chained from a light attack's combo window." },
			{ "afterHeavy", "Chained from a heavy attack's combo window." },
			{ "afterHold", "Taking over from a hold move (the launcher) released during its wind-up." },
			{ "blendIn", "Seconds the previous pose blends into this clip." },
			{ "startTime", "Clip time the move starts at (skip a wind-up the previous move already did)." },
			// Hits
			{ "hits.start", "Clip time the window opens: the weapon deals damage from here..." },
			{ "hits.end", "...to here." },
			{ "weapon", "Weapon slot whose blade sweeps (Main, Secondary)." },
			{ "bone", "Instead of a blade: a sphere around this bone sweeps (kicks, charges)." },
			{ "radius", "Radius (m) of the bone sphere." },
			{ "source", "Damage source: Melee, Firearm, Finisher, Environment - weak points and poise treat them differently." },
			{ "damage", "Health damage." },
			{ "poiseDamage", "Poise lost by the victim; at 0 it is broken (finishable)." },
			{ "weakPointDamage", "Damage to a weak point hit; -1: the same as damage." },
			{ "reaction", "Reaction requested: Flinch, Stagger, Knockdown, Launch (armor may downgrade it)." },
			{ "knockback", "Speed (m/s) the victim is pushed away with; its reaction move decides for how long." },
			{ "launch", "Upward speed (m/s) of a Launch reaction." },
			{ "hitStop", "Seconds attacker and victim stop at the contact." },
			{ "hitStopScale", "Time scale while stopped (0.03: a near-freeze)." },
			{ "shake", "Meters the victim's mesh shakes during its hit-stop." },
			{ "camera", "View kick when the player lands the hit (impact units; 0: none - light hits)." },
			{ "impactScale", "Multiplies the victim's hit effect (blood pieces)." },
			{ "air", "What the hit does in the air." },
			{ "lift", "In the air: both fighters rise at least this fast (m/s) when the hit lands between two airborne bodies. < 0: the character's airCombat.hitLift." },
			{ "stall", "Seconds the victim hovers after the hit (every hit restarts it). < 0: the character's airCombat default; 0: no hover (a slam).\nA launch hangs at the top for this long." },
			{ "direct", "Hits the move's target directly without a sweep (finishers, grabs)." },
			{ "hitSound", "FMOD event; empty: the victim's hitEffect sound." },
			{ "hitParticle", "Particle system; empty: the victim's hitEffect particle." },
			// Combo
			{ "combo.windowStart", "LIGHT zone opens: a tap released from here (or made earlier in this attack) starts 'next'. -1: no combo link." },
			{ "combo.alternativeAt", "HEAVY zone opens: Attack still held here, or pressed in the zone, starts 'alternative' - at once, or on release\nwhere the launcher can start (a longer hold becomes the launcher). Keep it ~0.08 after windowStart: light taps are\nbuffered into the window start, heavy presses come slightly early." },
			{ "combo.windowEnd", "The combo window closes: nothing by here, the attack goes into recovery (a press still held keeps it open\nuntil it is released or becomes the launcher)." },
			{ "combo.next", "Light follow-up (tap)." },
			{ "combo.alternative", "Heavy follow-up (held through alternativeAt / late press)." },
			// Hold
			{ "hold.until", "Attack must stay held until this clip time (the wind-up); released earlier, 'release' takes over. -1: no hold rule." },
			{ "hold.release", "Move taking over when Attack is released during the wind-up (entered with its afterHold entry)." },
			// Cancels
			{ "cancels.start", "Clip time the window opens." },
			{ "cancels.end", "Clip time it closes; -1: the end of the move." },
			{ "into", "Moves with these tags may interrupt this one inside the window. Locomotion: pushing the stick ends it." },
			// Recovery
			{ "recovery.enabled", "Off: no recovery - the move plays to its end." },
			{ "recovery.start", "From here the move releases the body (moving or any action); its clip plays out until the body moves.\n-1: combo.windowEnd for moves with a combo link, otherwise no recovery." },
			{ "recovery.lowerBodyBlend", "Seconds the legs take to hand over to locomotion once the body moves." },
			{ "recovery.upperBodyBlend", "Seconds the upper body takes to hand over once the body moves; -1: until the clip ends." },
			// Root motion
			{ "rootMotion.enabled", "The clip's root bone moves the capsule." },
			{ "rootMotion.scale", "Scale of that motion (in the air also the character's airCombat.rootMotionScale)." },
			// Tracking
			{ "searchRange", "Target search radius (m), all around the body." },
			{ "tracking.until", "Turn toward the target until this clip time; -1: the first strike." },
			{ "startTurnRate", "deg/s the attack turns during its first startTurnTime: it lines up fast." },
			{ "startTurnTime", "Seconds of the fast turn at the start." },
			{ "turnRate", "deg/s it follows the target after that." },
			{ "turnSharpness", "1/s easing of the turns (higher: snappier)." },
			{ "magnetRange", "Only targets this close (m) are pulled toward; 0: no pull." },
			{ "idealDistance", "The pull stops here (m); nothing carries the body closer." },
			{ "magnetUntil", "Close the gap until this clip time; -1: the end of the last hit." },
			{ "maxMagnetSpeed", "m/s the pull adds to the clip's motion at most." },
			{ "advance", "m/s forward while the stick is held and there is no target." },
			// Reach
			{ "reach.length", "Meters the weapon gets longer (its extend bone moves out); 0: its own length." },
			{ "reach.start", "Clip time it is fully out; -1: the first blade hit's start." },
			{ "reach.end", "Clip time it starts going back; -1: the last blade hit's end." },
			{ "reach.blendIn", "Seconds it grows before start." },
			{ "reach.blendOut", "Seconds it shrinks back after end." },
			{ "reach.weapon", "Weapon slot that gets longer." },
			// Windows
			{ "armor.start", "Clip time armor begins." },
			{ "armor.end", "Clip time it ends; -1: the end of the move." },
			{ "poiseResist", "Share of incoming poise damage ignored, and how much reactions are downgraded (1: no reaction)." },
			{ "invulnerable.start", "Clip time the i-frames begin." },
			{ "invulnerable.end", "Clip time they end; -1: the end of the move." },
			// Cues
			{ "cues.time", "Clip time the cue fires." },
			{ "sound", "FMOD event to play." },
			{ "particle", "Particle system to spawn (at bone)." },
			{ "cues.bone", "Where sound / particle spawn; empty: the body's origin." },
			{ "trail", "Weapon trail: 1 on, 0 off, -1 unchanged." },
			{ "trailWeapon", "Weapon slot whose trail it switches." },
			{ "cameraShake", "Camera rumble (trauma 0..1) when the player's body plays it." },
			// Behaviour params
			{ "params.locomotion", "Share of run speed the stick may add during the attack (0: root motion only)." },
			{ "params.matchTargetHeight", "In the air, chase the target's height until a hit starts the hover." },
			{ "params.needsTarget", "In the air the move only starts with a target in reach (the launcher: a fresh hold-opener input falls back to AirDrop without one; combo follow-ups do not)." },
			{ "params.plunge", "Air version dives: from start at speed, the strike held from hold until landing." },
			{ "params.launchSelf", "The attacker leaps too: at time, or onHit when a hit launches its target." },
			{ "params.onHit", "Leap when one of the move's hits launches its target." },
			{ "params.verticalSpeed", "Upward speed (m/s) of the leap." },
			{ "params.requireHeld", "Only while Attack is still held." },
			{ "params.distance", "Distance (m) covered." },
			{ "params.duration", "Seconds the displacement takes." },
			{ "params.curve", "Speed curve of the dash: Linear or EaseOut." },
			{ "params.airCharges", "Times it can be used per airtime." },
			{ "params.exitSpeed", "Speed at the end of the dash, in run speeds (1: runs out of it)." },
			{ "params.tailLocomotion", "Share of run speed the stick steers with after the displacement." },
			{ "params.faceDirection", "The body turns to face the dash." },
			{ "params.velocity", "Upward speed (m/s) of the jump." },
			{ "params.maxAirJumps", "Jumps allowed in the air before landing." },
			{ "params.airControl", "Share of ground acceleration the stick has in the air." },
			{ "params.carrySpeed", "Horizontal speed carried into the jump at most, in run speeds." },
			{ "params.maxDuration", "Seconds of gliding per airtime." },
			{ "params.gravityScale", "Gravity while gliding." },
			{ "params.maxFallSpeed", "Fall speed cap (m/s) while gliding." },
			{ "params.holdDelay", "Seconds Jump must be held while falling before the glide starts." },
			{ "params.weapon", "Weapon slot the move uses." },
			{ "params.button", "Button driving the move." },
			{ "params.automatic", "Fires while held (otherwise one shot per press)." },
			{ "params.aimTurnRate", "deg/s the body turns to the aim while firing." },
			{ "params.restartOnShot", "Every shot restarts the clip (recoil per shot)." },
			{ "params.moveSpeed", "Share of run speed while firing." },
			{ "params.knockbackScale", "Multiplies the knockback received." },
			{ "params.knockbackTime", "Seconds the push lasts (it slows down to 0)." },
			{ "params.launchScale", "Multiplies the launch speed received." },
			{ "params.holdWhileBroken", "Hold the reaction while the poise is broken (finishable)." },
			{ "params.ragdoll", "Hand the body to ragdoll." },
			{ "params.ragdollDelay", "Clip seconds before the ragdoll." },
		};
		return help;
	}

	const char* HelpFor(const std::string& section, const std::string& key)
	{
		const auto& help = Help();
		auto it = help.find(section + "." + key);
		if (it == help.end())
			it = help.find(key);
		return it == help.end() ? nullptr : it->second;
	}

	float DragSpeed(const std::string& key)
	{
		if (key == "damage" || key == "poiseDamage" || key == "weakPointDamage")
			return 0.1f;
		if (key.find("Rate") != std::string::npos)
			return 5.0f;
		if (key == "knockback" || key == "launch" || key == "distance" || key == "velocity" || key == "verticalSpeed" || key.find("Speed") != std::string::npos)
			return 0.05f;
		return 0.005f;
	}

	struct Context
	{
		const MoveSetData* moveSet = nullptr;
		std::vector<std::string> moveIds;
		std::string section;     // the move field being edited ("params" for behaviour parameters)
	};

	// Choices of a string field (nullptr: free text); allowNone: empty is a valid choice.
	const std::vector<std::string>* Options(const std::string& key, const Context& context, bool& allowNone)
	{
		static const std::vector<std::string> reactions = EnumNames<HitReaction>();
		static const std::vector<std::string> sources = EnumNames<DamageSource>();
		static const std::vector<std::string> weapons = EnumNames<WeaponSlot>();
		static const std::vector<std::string> channels = EnumNames<MoveChannel>();
		static const std::vector<std::string> masks = EnumNames<BodyMask>();
		static const std::vector<std::string> overlays = EnumNames<OverlayPolicy>();
		static const std::vector<std::string> states = EnumNames<MoveStartState>();
		static const std::vector<std::string> buttons = NameList(InputButtonNames);
		static std::vector<std::string> behaviors;
		if (behaviors.empty())
			for (int i = 0; i < MoveBehaviorRegistry::Count(); ++i)
				behaviors.push_back(MoveBehaviorRegistry::Get(i).Name);

		allowNone = false;
		if (key == "reaction") return &reactions;
		if (key == "source") return &sources;
		if (key == "weapon" || key == "trailWeapon") return &weapons;
		if (key == "channel") return &channels;
		if (key == "bodyMask") return &masks;
		if (key == "overlay") return &overlays;
		if (key == "startState") return &states;
		if (key == "behavior") return &behaviors;
		if (key == "button") return &buttons;
		if (key == "next" || key == "alternative" || key == "release")
		{
			allowNone = true;
			return &context.moveIds;
		}
		return nullptr;
	}

	bool ComboString(const char* label, std::string& value, const std::vector<std::string>& options, bool allowNone)
	{
		bool changed = false;
		if (ImGui::BeginCombo(label, value.empty() ? "(none)" : value.c_str()))
		{
			if (allowNone && ImGui::Selectable("(none)", value.empty()))
			{
				value.clear();
				changed = true;
			}
			for (const std::string& option : options)
			{
				if (ImGui::Selectable(option.c_str(), option == value))
				{
					value = option;
					changed = true;
				}
			}
			ImGui::EndCombo();
		}
		return changed;
	}

	bool InputString(const char* label, std::string& value)
	{
		char buffer[256];
		std::snprintf(buffer, sizeof(buffer), "%s", value.c_str());
		if (!ImGui::InputText(label, buffer, sizeof(buffer)))
			return false;
		value = buffer;
		return true;
	}

	// Tooltip and "reset to default" on the last item.
	bool Decorate(const Context& context, const std::string& key, Json& value, const Json* defaults)
	{
		if (const char* help = HelpFor(context.section, key))
			ImGui::SetItemTooltip("%s", help);
		bool changed = false;
		if (defaults != nullptr && *defaults != value && ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Reset to default"))
			{
				value = *defaults;
				changed = true;
			}
			ImGui::EndPopup();
		}
		return changed;
	}

	std::string ElementLabel(const std::string& key, std::size_t index, const Json& element)
	{
		auto number = [&](const char* field) { return element.contains(field) && element[field].is_number() ? element[field].get<float>() : 0.0f; };
		char label[192];
		if (key == "hits")
			std::snprintf(label, sizeof(label), "hit %zu   %.2f - %.2f   %s", index, number("start"), number("end"), element.value("reaction", std::string()).c_str());
		else if (key == "cancels")
		{
			std::string into;
			if (element.contains("into") && element["into"].is_array())
				for (const Json& tag : element["into"])
					into += (into.empty() ? "" : ", ") + tag.get<std::string>();
			char end[16];
			if (number("end") < 0.0f)
				std::snprintf(end, sizeof(end), "end");
			else
				std::snprintf(end, sizeof(end), "%.2f", number("end"));
			std::snprintf(label, sizeof(label), "cancel %zu   %.2f - %s   %s", index, number("start"), end, into.c_str());
		}
		else if (key == "armor")
			std::snprintf(label, sizeof(label), "armor %zu   %.2f - %.2f   %.0f%%", index, number("start"), number("end"), number("poiseResist") * 100.0f);
		else if (key == "invulnerable")
			std::snprintf(label, sizeof(label), "i-frames %zu   %.2f - %.2f", index, number("start"), number("end"));
		else if (key == "cues")
			std::snprintf(label, sizeof(label), "cue %zu   at %.2f", index, number("time"));
		else
			std::snprintf(label, sizeof(label), "%s %zu", key.c_str(), index);
		return label;
	}

	bool EditValue(const std::string& key, Json& value, const Json* defaults, Context& context);

	bool EditFields(Json& object, const Json* defaults, Context& context)
	{
		bool changed = false;
		for (auto it = object.begin(); it != object.end(); ++it)
		{
			const Json* fieldDefault = defaults != nullptr && defaults->is_object() && defaults->contains(it.key()) ? &(*defaults)[it.key()] : nullptr;
			changed |= EditValue(it.key(), it.value(), fieldDefault, context);
		}
		return changed;
	}

	bool EditTags(const std::string& key, Json& value, Context& context)
	{
		static const std::vector<std::string> tags = EnumNames<MoveTag>();
		ImGui::TextUnformatted(key.c_str());
		if (const char* help = HelpFor(context.section, key))
			ImGui::SetItemTooltip("%s", help);

		bool changed = false;
		std::vector<std::string> selected;
		for (const Json& tag : value)
			if (tag.is_string())
				selected.push_back(tag.get<std::string>());

		ImGui::Indent();
		float lineWidth = 0.0f;
		const float available = ImGui::GetContentRegionAvail().x;
		for (const std::string& tag : tags)
		{
			if (key == "tags" && tag == "Locomotion")
				continue;   // a cancel target only
			bool on = std::find(selected.begin(), selected.end(), tag) != selected.end();
			const float width = ImGui::CalcTextSize(tag.c_str()).x + ImGui::GetFrameHeight() + 16.0f;
			if (lineWidth > 0.0f && lineWidth + width < available)
				ImGui::SameLine();
			else
				lineWidth = 0.0f;
			lineWidth += width;
			if (ImGui::Checkbox(tag.c_str(), &on))
			{
				changed = true;
				if (on)
					selected.push_back(tag);
				else
					selected.erase(std::remove(selected.begin(), selected.end(), tag), selected.end());
			}
		}
		ImGui::Unindent();

		if (changed)
		{
			Json ordered = Json::array();
			for (const std::string& tag : tags)
				if (std::find(selected.begin(), selected.end(), tag) != selected.end())
					ordered.push_back(tag);
			value = std::move(ordered);
		}
		return changed;
	}

	// The elements of an array: struct elements as tree nodes (remove inside), strings as rows; "+ add" appends one.
	bool EditArrayElements(const std::string& key, Json& value, Context& context)
	{
		bool changed = false;
		const Json* elementDefaults = MoveArrayElementDefaults(key);
		const bool structs = elementDefaults != nullptr || std::any_of(value.begin(), value.end(), [](const Json& e) { return e.is_object(); });

		for (std::size_t i = 0; i < value.size(); ++i)
		{
			ImGui::PushID(static_cast<int>(i));
			if (structs)
			{
				const bool open = ImGui::TreeNodeEx("##element", ImGuiTreeNodeFlags_SpanAvailWidth, "%s", ElementLabel(key, i, value[i]).c_str());
				if (open)
				{
					changed |= EditFields(value[i], elementDefaults, context);
					if (ImGui::SmallButton("Remove"))
					{
						value.erase(i);
						changed = true;
						ImGui::TreePop();
						ImGui::PopID();
						break;
					}
					ImGui::TreePop();
				}
			}
			else
			{
				std::string text = value[i].is_string() ? value[i].get<std::string>() : value[i].dump();
				if (InputString("##value", text))
				{
					value[i] = text;
					changed = true;
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("x"))
				{
					value.erase(i);
					changed = true;
					ImGui::PopID();
					break;
				}
			}
			ImGui::PopID();
		}

		if (ImGui::SmallButton(("+ add##" + key).c_str()))
		{
			value.push_back(elementDefaults != nullptr ? *elementDefaults : (structs && !value.empty() ? value.back() : Json(std::string())));
			changed = true;
		}
		return changed;
	}

	bool EditValue(const std::string& key, Json& value, const Json* defaults, Context& context)
	{
		bool changed = false;
		const bool isDefault = defaults != nullptr && *defaults == value;
		ImGui::PushID(key.c_str());

		if (value.is_array() && (key == "tags" || key == "into"))
		{
			changed = EditTags(key, value, context);
		}
		else if (value.is_array())
		{
			if (isDefault)
				ImGui::PushStyleColor(ImGuiCol_Text, DimColor);
			const bool open = ImGui::TreeNodeEx("##array", ImGuiTreeNodeFlags_SpanAvailWidth, "%s (%zu)", key.c_str(), value.size());
			if (isDefault)
				ImGui::PopStyleColor();
			changed |= Decorate(context, key, value, defaults);
			if (open)
			{
				changed |= EditArrayElements(key, value, context);
				ImGui::TreePop();
			}
		}
		else if (value.is_object())
		{
			const bool open = ImGui::TreeNodeEx("##object", ImGuiTreeNodeFlags_SpanAvailWidth, "%s", key.c_str());
			changed |= Decorate(context, key, value, defaults);
			if (open)
			{
				changed |= EditFields(value, defaults, context);
				ImGui::TreePop();
			}
		}
		else
		{
			if (isDefault)
				ImGui::PushStyleColor(ImGuiCol_Text, DimColor);

			if (value.is_boolean())
			{
				bool flag = value.get<bool>();
				if (ImGui::Checkbox(key.c_str(), &flag))
				{
					value = flag;
					changed = true;
				}
			}
			else if (value.is_number_float())
			{
				float number = value.get<float>();
				if (ImGui::DragFloat(key.c_str(), &number, DragSpeed(key), 0.0f, 0.0f, "%.3f"))
				{
					value = number;
					changed = true;
				}
			}
			else if (value.is_number())
			{
				int number = value.get<int>();
				if (ImGui::DragInt(key.c_str(), &number, 0.1f))
				{
					value = number;
					changed = true;
				}
			}
			else if (value.is_string())
			{
				std::string text = value.get<std::string>();
				bool allowNone = false;
				const std::vector<std::string>* options = Options(key, context, allowNone);
				if (options ? ComboString(key.c_str(), text, *options, allowNone) : InputString(key.c_str(), text))
				{
					value = text;
					changed = true;
				}
			}
			else
			{
				ImGui::TextDisabled("%s: %s", key.c_str(), value.dump().c_str());
			}

			if (isDefault)
				ImGui::PopStyleColor();
			changed |= Decorate(context, key, value, defaults);
		}

		ImGui::PopID();
		return changed;
	}

	// The behaviour's parameters, all of them (defaults dimmed); stores back only what was written or now differs.
	bool EditParams(Json& params, const MoveData& move, Context& context)
	{
		MoveData probe = move;
		probe.params = params;
		const int type = MoveBehaviorRegistry::Find(move.behavior);
		const Json defaults = type >= 0 && MoveBehaviorRegistry::Get(type).DefaultParams ? MoveBehaviorRegistry::Get(type).DefaultParams() : Json::object();
		Json edited = MoveParamsWithDefaults(probe);
		if (edited.empty())
		{
			ImGui::TextDisabled("'%s' has no parameters.", move.behavior.c_str());
			return false;
		}

		context.section = "params";
		if (!EditFields(edited, &defaults, context))
			return false;
		params = MoveParamsToStore(edited, probe);
		return true;
	}

	bool SlotTable(const char* title, std::map<std::string, std::string>& table, const std::vector<std::string>& slots, const Context& context)
	{
		bool changed = false;
		if (!ImGui::TreeNodeEx(title, ImGuiTreeNodeFlags_SpanAvailWidth))
		{
			if (const char* help = HelpFor("moveset", title))
				ImGui::SetItemTooltip("%s", help);
			return false;
		}
		if (const char* help = HelpFor("moveset", title))
			ImGui::SetItemTooltip("%s", help);
		for (const std::string& slot : slots)
		{
			const auto found = table.find(slot);
			std::string move = found == table.end() ? std::string() : found->second;
			if (ComboString(slot.c_str(), move, context.moveIds, true))
			{
				if (move.empty())
					table.erase(slot);
				else
					table[slot] = move;
				changed = true;
			}
		}
		ImGui::TreePop();
		return changed;
	}

	Context MakeContext(const MoveSetData& moveSet)
	{
		Context context;
		context.moveSet = &moveSet;
		for (const MoveData& move : moveSet.moves)
			context.moveIds.push_back(move.id);
		return context;
	}
}

void MoveSetEditor::Reset()
{
	editing.reset();
	base.reset();
	selected = 0;
	errors.clear();
	warnings.clear();
}

void MoveSetEditor::Validate()
{
	errors.clear();
	warnings.clear();
	DataLoadReport report;
	MoveSetData check;
	const Json json = *editing;
	if (ConvertDataJson(json, check, "", report))
		ResolveMoveSet(check, report);
	errors = report.Errors;
	warnings = report.Warnings;
}

void MoveSetEditor::Draw(const MoveSetData& live)
{
	if (!ImGui::CollapsingHeader("Moveset editor"))
		return;

	if (!editing || editing->sourcePath != live.sourcePath)
	{
		editing = std::make_shared<MoveSetData>(live);
		base = std::make_shared<MoveSetData>(live);
		selected = 0;
		Validate();
	}

	ImGui::TextUnformatted(editing->sourcePath.c_str());
	ImGui::TextDisabled("Times are clip seconds. Dimmed = default. Right-click a field to reset it; hover for what it does.");

	bool changed = false;
	ImGui::PushID("moveset");
	changed |= DrawMoveSetFields();
	ImGui::PopID();

	ImGui::SeparatorText("Move");
	changed |= DrawMoveList();
	if (!editing->moves.empty())
	{
		selected = std::clamp(selected, 0, static_cast<int>(editing->moves.size()) - 1);
		ImGui::PushID("move");
		changed |= DrawMove(editing->moves[static_cast<size_t>(selected)]);
		ImGui::PopID();
	}

	if (changed)
		Validate();

	ImGui::Separator();
	if (errors.empty())
		ImGui::TextColored(OkColor, warnings.empty() ? "Valid." : "Valid, with warnings:");
	for (const std::string& error : errors)
		ImGui::TextColored(ErrorColor, "error: %s", error.c_str());
	for (const std::string& warning : warnings)
		ImGui::TextColored(WarningColor, "warning: %s", warning.c_str());

	DrawActions();
}

bool MoveSetEditor::DrawMoveSetFields()
{
	MoveSetData& moveSet = *editing;
	Context context = MakeContext(moveSet);
	context.section = "moveset";
	bool changed = false;

	if (!ImGui::CollapsingHeader("Moveset: buffers, slots, files"))
		return false;

	auto drag = [&](const char* name, float& value, float speed)
	{
		if (ImGui::DragFloat(name, &value, speed, 0.0f, 10.0f, "%.3f"))
			changed = true;
		if (const char* help = HelpFor("moveset", name))
			ImGui::SetItemTooltip("%s", help);
	};
	drag("inputBuffer", moveSet.inputBuffer, 0.01f);
	drag("actionBuffer", moveSet.actionBuffer, 0.01f);
	if (ImGui::Checkbox("holdCarriesOver", &moveSet.holdCarriesOver))
		changed = true;
	ImGui::SetItemTooltip("%s", HelpFor("moveset", "holdCarriesOver"));
	drag("holdDelay", moveSet.holdDelay, 0.005f);
	drag("holdDelayInMove", moveSet.holdDelayInMove, 0.01f);
	drag("blendOut", moveSet.blendOut, 0.005f);

	changed |= SlotTable("combos", moveSet.combos, NameList(ComboSlotNames), context);
	changed |= SlotTable("holdOpeners", moveSet.holdOpeners, NameList(ComboSlotNames), context);
	changed |= SlotTable("actions", moveSet.actions, NameList(ActionSlotNames), context);
	changed |= SlotTable("reactions", moveSet.reactions, NameList(ReactionSlotNames), context);

	Json animations = moveSet.animations;
	if (EditValue("animations", animations, nullptr, context))
	{
		moveSet.animations = animations.get<std::vector<std::string>>();
		changed = true;
	}
	return changed;
}

bool MoveSetEditor::DrawMoveList()
{
	std::vector<MoveData>& moves = editing->moves;
	bool changed = false;

	const char* current = moves.empty() ? "(no moves)" : moves[static_cast<size_t>(std::clamp(selected, 0, static_cast<int>(moves.size()) - 1))].id.c_str();
	if (ImGui::BeginCombo("move", current))
	{
		for (int i = 0; i < static_cast<int>(moves.size()); ++i)
			if (ImGui::Selectable(moves[static_cast<size_t>(i)].id.c_str(), i == selected))
				selected = i;
		ImGui::EndCombo();
	}

	ImGui::SetNextItemWidth(160.0f);
	ImGui::InputText("##newMoveId", newMoveId, sizeof(newMoveId));
	ImGui::SetItemTooltip("Id for a new or duplicated move.");
	const std::string id = newMoveId;
	const bool idFree = !id.empty() && std::none_of(moves.begin(), moves.end(), [&](const MoveData& move) { return move.id == id; });

	ImGui::SameLine();
	ImGui::BeginDisabled(!idFree);
	if (ImGui::Button("New"))
	{
		MoveData move;
		move.id = id;
		move.endTime = 0.5f;   // a clip-less move needs an end; give it a clip and drop this
		moves.push_back(move);
		selected = static_cast<int>(moves.size()) - 1;
		changed = true;
	}
	ImGui::SetItemTooltip("Adds an empty Attack move with this id.");
	ImGui::SameLine();
	if (ImGui::Button("Duplicate") && !moves.empty())
	{
		MoveData copy = moves[static_cast<size_t>(selected)];
		copy.id = id;
		moves.push_back(copy);
		selected = static_cast<int>(moves.size()) - 1;
		changed = true;
	}
	ImGui::SetItemTooltip("Copies the selected move under this id.");
	ImGui::EndDisabled();

	ImGui::SameLine();
	ImGui::BeginDisabled(moves.empty());
	if (ImGui::Button("Delete"))
		ImGui::OpenPopup("Delete move?");
	ImGui::EndDisabled();
	ImGui::SetItemTooltip("Removes the selected move (references to it are reported below).");
	if (ImGui::BeginPopupModal("Delete move?", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Delete '%s'?", moves.empty() ? "" : moves[static_cast<size_t>(selected)].id.c_str());
		if (ImGui::Button("Delete") && !moves.empty())
		{
			moves.erase(moves.begin() + selected);
			selected = std::max(0, selected - 1);
			changed = true;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}
	return changed;
}

bool MoveSetEditor::DrawMove(MoveData& move)
{
	Json json = move;
	const Json& defaults = DefaultMoveJson();
	Context context = MakeContext(*editing);
	bool changed = false;

	struct Section
	{
		const char* title;
		std::vector<const char*> keys;
		bool open;
		const char* help;
	};
	static const std::vector<Section> sections = {
		{ "General", { "behavior", "clip", "variants", "animation", "speed", "loop", "loopStart", "heavy", "tags", "channel", "bodyMask",
			"maskBones", "legWeight", "airLegs", "overlay", "startState", "endTime", "blendOut", "cooldown" }, true,
			"What the move is, what it plays and where." },
		{ "Entry", { "entry" }, false, "How the move begins, depending on what came before it." },
		{ "Hits", { "hits" }, true, "Damage windows: when and how the weapon deals damage." },
		{ "Combo link", { "combo" }, true, "When Attack chains into the next attack: LIGHT zone (tap) and HEAVY zone (hold / late press)." },
		{ "Hold (wind-up)", { "hold" }, false, "Attack must stay held through the wind-up, or another move takes over." },
		{ "Cancels", { "cancels" }, true, "Which actions may interrupt the move, and from when." },
		{ "Recovery", { "recovery" }, true, "When the move releases the body while its clip plays out." },
		{ "Root motion", { "rootMotion" }, false, "Whether the clip's root motion moves the capsule." },
		{ "Tracking (target assist)", { "tracking" }, false, "Target search, turning toward it and pulling in." },
		{ "Reach", { "reach" }, false, "How much longer the weapon gets during the move (heavy attacks reach further)." },
		{ "Armor", { "armor" }, false, "Windows where hits stagger less." },
		{ "Invulnerable", { "invulnerable" }, false, "Windows where hits do not land (i-frames)." },
		{ "Cues", { "cues" }, false, "Sounds, particles, weapon trails and camera rumble at clip times." },
	};

	std::vector<std::string> shown = { "params" };
	for (const Section& section : sections)
	{
		ImGui::PushID(section.title);
		const bool open = ImGui::CollapsingHeader(section.title, section.open ? ImGuiTreeNodeFlags_DefaultOpen : 0);
		ImGui::SetItemTooltip("%s", section.help);
		for (const char* key : section.keys)
		{
			shown.push_back(key);
			if (!open || !json.contains(key))
				continue;
			context.section = key;
			Json& value = json[key];
			const Json* fieldDefault = defaults.contains(key) ? &defaults[key] : nullptr;
			if (value.is_object())
				changed |= EditFields(value, fieldDefault, context);    // section contents directly, no extra tree level
			else if (value.is_array() && MoveArrayElementDefaults(key) != nullptr)
				changed |= EditArrayElements(key, value, context);
			else
			{
				context.section.clear();
				changed |= EditValue(key, value, fieldDefault, context);
			}
		}
		ImGui::PopID();
	}

	MoveData behaviorProbe;
	behaviorProbe.behavior = json.value("behavior", std::string("Attack"));
	ImGui::PushID("params");
	if (ImGui::CollapsingHeader(("Behaviour params (" + behaviorProbe.behavior + ")").c_str()))
		changed |= EditParams(json["params"], behaviorProbe, context);
	ImGui::SetItemTooltip("Parameters of the move's behaviour - all of them, defaults dimmed.");
	ImGui::PopID();

	// Fields added to the data later, not placed in a section yet.
	std::vector<std::string> other;
	for (auto it = json.begin(); it != json.end(); ++it)
		if (std::find(shown.begin(), shown.end(), it.key()) == shown.end())
			other.push_back(it.key());
	if (!other.empty() && ImGui::CollapsingHeader("Other"))
	{
		context.section.clear();
		for (const std::string& key : other)
			changed |= EditValue(key, json[key], defaults.contains(key) ? &defaults[key] : nullptr, context);
	}

	if (!changed)
		return false;

	MoveData updated;
	DataLoadReport report;
	if (!ConvertDataJson(json, updated, move.id, report))
	{
		status = "edit rejected: " + (report.Errors.empty() ? std::string("invalid value") : report.Errors.front());
		return false;
	}
	updated.id = move.id;
	move = std::move(updated);
	return true;
}

void MoveSetEditor::DrawActions()
{
	std::string error;
	ImGui::BeginDisabled(!errors.empty());
	if (ImGui::Button("Apply"))
		status = CombatData::ReplaceMoveSet(*editing, error) ? "applied - bodies adopt it at their next move boundary" : "rejected: " + error;
	ImGui::SetItemTooltip("Makes the edited moveset live (validated); bodies adopt it at their next move boundary.");
	ImGui::SameLine();
	if (ImGui::Button("Apply + Save"))
	{
		if (CombatData::ReplaceMoveSet(*editing, error) && CombatData::SaveMoveSet(*base, *editing, error))
		{
			*base = *editing;
			status = "saved " + editing->sourcePath;
		}
		else
		{
			status = "failed: " + error;
		}
	}
	ImGui::SetItemTooltip("Also writes the edits into the moveset file - only the edited values change in the text.");
	ImGui::EndDisabled();
	ImGui::SameLine();
	if (ImGui::Button("Revert"))
	{
		Reset();
		status = "reverted to the live data";
	}
	ImGui::SetItemTooltip("Drops every edit since the last Apply + Save (or since the editor opened).");

	if (!status.empty())
		ImGui::TextWrapped("%s", status.c_str());
}

#endif
