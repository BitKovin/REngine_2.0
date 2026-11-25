#pragma once

// The sounds NPCs listen for (NPC_AI_DESIGN.md §5.4). The game emits them through NpcSounds::Emit; the radius and
// what a sound means come from the "sounds" table of GameData/ai/perception.json.

#include <AiPerception/AiPerceptionSystem.h>

#include "NpcData.h"
#include "../Investigations/InvestigationBase.h"

enum class NpcSoundType : int
{
	Unknown = 0,

	// The value the old NPC code reads from a heard shot (InvestigationReason::WeaponFire).
	Gunshot = 3,

	Footstep = 101,
	Landing,
	Door,
	ThrownObject,
	MeleeClash,
	SuppressedShot,
	Explosion,
	BodyFall,
	Scream,
	FeedingStealth,
	FeedingCombat,
	BreakingGlass
};

static_assert(static_cast<int>(NpcSoundType::Gunshot) == static_cast<int>(InvestigationReason::WeaponFire),
	"Gunshot must keep the value old NPCs read as a weapon shot");

namespace NpcSounds
{
	// The key of a sound type in perception.json.
	inline const char* Name(NpcSoundType type)
	{
		switch (type)
		{
		case NpcSoundType::Gunshot: return "gunshot";
		case NpcSoundType::Footstep: return "footstep";
		case NpcSoundType::Landing: return "landing";
		case NpcSoundType::Door: return "door";
		case NpcSoundType::ThrownObject: return "thrown_object";
		case NpcSoundType::MeleeClash: return "melee_clash";
		case NpcSoundType::SuppressedShot: return "suppressed_shot";
		case NpcSoundType::Explosion: return "explosion";
		case NpcSoundType::BodyFall: return "body_fall";
		case NpcSoundType::Scream: return "scream";
		case NpcSoundType::FeedingStealth: return "feeding_stealth";
		case NpcSoundType::FeedingCombat: return "feeding_combat";
		case NpcSoundType::BreakingGlass: return "breaking_glass";
		default: return "unknown";
		}
	}

	// Emits a sound at a position. `loudness` overrides the table's radius when above 0 (a weapon with its own noise
	// radius). `minor`: footsteps, a landing, a door, a thrown object: listeners off every player's screen hear less.
	inline void Emit(NpcSoundType type, const glm::vec3& position, const std::string& causerId, float loudness = 0.0f, bool minor = false)
	{
		const NpcSoundTuning& tuning = NpcPerceptionTuning::Get().SoundFor(Name(type));

		SoundEvent sound;
		sound.position = position;
		sound.loudness = loudness > 0.0f ? loudness : tuning.radius;
		sound.type = static_cast<int>(type);
		sound.causerId = causerId;
		sound.minorPlayerSound = minor;

		AiPerceptionSystem::EmitSound(sound);
	}
}
