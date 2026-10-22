#pragma once

#include <Data/JsonEnum.h>
#include <EntityHandle.h>

#include <cstdint>

// Shared combat vocabulary. Enums serialize as strings in data files; each enum's first serialized entry is
// "Invalid" (or the safest default) so a typo in JSON is detected by the validation pass instead of silently
// becoming a real value.

// ---- Teams ------------------------------------------------------------------------------------------

enum class Team : uint8_t
{
	Neutral,
	Player,
	Enemy,
	Count
};

SLASHER_JSON_ENUM(Team, {
	{ Team::Count, nullptr },
	{ Team::Neutral, "Neutral" },
	{ Team::Player, "Player" },
	{ Team::Enemy, "Enemy" },
})

enum class Attitude : uint8_t
{
	Friendly,
	Neutral,
	Hostile
};

inline Attitude GetAttitude(Team a, Team b)
{
	if (a == Team::Neutral || b == Team::Neutral) return Attitude::Neutral;
	return a == b ? Attitude::Friendly : Attitude::Hostile;
}

// ---- Damage -----------------------------------------------------------------------------------------

enum class DamageSource : uint8_t
{
	Melee,
	Firearm,
	Finisher,
	Environment,
	Explosion,
	Count
};

SLASHER_JSON_ENUM(DamageSource, {
	{ DamageSource::Count, nullptr },
	{ DamageSource::Melee, "Melee" },
	{ DamageSource::Firearm, "Firearm" },
	{ DamageSource::Finisher, "Finisher" },
	{ DamageSource::Environment, "Environment" },
	{ DamageSource::Explosion, "Explosion" },
})

using DamageSourceMask = uint8_t;
inline constexpr DamageSourceMask SourceBit(DamageSource s) { return static_cast<DamageSourceMask>(1u << static_cast<uint8_t>(s)); }

// What the attacker asks for; the hit resolution decides what is actually played (armor, stagger, death).
enum class HitReaction : uint8_t
{
	None,
	Flinch,
	Stagger,
	Knockdown,
	Launch,
	Count
};

SLASHER_JSON_ENUM(HitReaction, {
	{ HitReaction::Count, nullptr },
	{ HitReaction::None, "None" },
	{ HitReaction::Flinch, "Flinch" },
	{ HitReaction::Stagger, "Stagger" },
	{ HitReaction::Knockdown, "Knockdown" },
	{ HitReaction::Launch, "Launch" },
})

// Keys of a moveset's "reactions" table: hit reactions plus forced states.
enum class ReactionSlot : uint8_t
{
	Flinch,
	Stagger,
	Knockdown,
	Launch,
	PoiseBreak,       // optional; falls back to Stagger
	FinisherVictim,
	Death,
	Count
};

inline constexpr const char* ReactionSlotNames[] = { "Flinch", "Stagger", "Knockdown", "Launch", "PoiseBreak", "FinisherVictim", "Death" };

// ---- Input ------------------------------------------------------------------------------------------

// Virtual buttons a controller presses on a body.
enum class InputButton : uint8_t
{
	Attack,
	Secondary,
	Dash,
	Jump,
	Finisher,
	Interact,
	SwitchMain,      // cycles the Main (melee) weapons
	SwitchSecondary, // cycles the Secondary (firearm) weapons
	Count
};

inline constexpr const char* InputButtonNames[] = { "Attack", "Secondary", "Dash", "Jump", "Finisher", "Interact", "SwitchMain", "SwitchSecondary" };

// Keys of a moveset's "combos" table (combo openers).
enum class ComboSlot : uint8_t
{
	Attack,
	AirAttack,
	Count
};

inline constexpr const char* ComboSlotNames[] = { "Attack", "AirAttack" };

// Keys of a moveset's "actions" table.
enum class ActionSlot : uint8_t
{
	Dash,
	AirDash,
	Jump,
	AirJump,
	Glide,
	Secondary,
	AirSecondary,
	Finisher,
	Draw,            // brings the Main weapon back once another weapon's holsterDelay ran out
	AirDrop,         // the drop attack: what holding Attack in the air starts when there is nothing to launch (a plunge)
	Sheathe,         // a sheathable Main weapon (WeaponData::sheathedHands) goes back into its sheath once its combo ended,
	                 // standing (a full-body move: the clip steps from the attack stance into the idle)
	MovingSheathe,   // the same while moving or in the air (over the legs); falls back to Sheathe
	Reload,          // the Secondary weapon reloads (starts by itself while its magazine refills)
	Count
};

inline constexpr const char* ActionSlotNames[] = { "Dash", "AirDash", "Jump", "AirJump", "Glide", "Secondary", "AirSecondary", "Finisher", "Draw", "AirDrop",
	"Sheathe", "MovingSheathe", "Reload" };

// Finds the index of name in a names table, -1 if absent.
template<size_t N>
int FindSlotIndex(const char* const (&names)[N], const std::string& name)
{
	for (size_t i = 0; i < N; ++i)
		if (name == names[i]) return static_cast<int>(i);
	return -1;
}

// ---- Weapons ----------------------------------------------------------------------------------------

enum class WeaponSlot : uint8_t
{
	Main,        // melee
	Secondary,   // firearm
	Count
};

SLASHER_JSON_ENUM(WeaponSlot, {
	{ WeaponSlot::Count, nullptr },
	{ WeaponSlot::Main, "Main" },
	{ WeaponSlot::Secondary, "Secondary" },
})

inline constexpr const char* WeaponSlotNames[] = { "Main", "Secondary" };

// The weapon slot whose equipped weapon's moves fill an action slot (MoveSetData::BuildTables): firing and reloading
// belong to the firearm, everything else (the draw, the drop, the finisher, sheathing) to the melee weapon.
inline WeaponSlot ActionWeaponSlot(ActionSlot slot)
{
	return slot == ActionSlot::Secondary || slot == ActionSlot::AirSecondary || slot == ActionSlot::Reload
		? WeaponSlot::Secondary : WeaponSlot::Main;
}

// Hands a weapon holds (WeaponData::hands): a firearm taking a hand the melee weapon needs puts it away or into
// its sheath (COMBAT_DESIGN.md §9.3).
enum class WeaponHand : uint8_t
{
	Right,
	Left,
	Count
};

SLASHER_JSON_ENUM(WeaponHand, {
	{ WeaponHand::Count, nullptr },
	{ WeaponHand::Right, "Right" },
	{ WeaponHand::Left, "Left" },
})

using HandMask = uint8_t;
inline constexpr HandMask HandBit(WeaponHand hand) { return static_cast<HandMask>(1u << static_cast<uint8_t>(hand)); }

// ---- Animation --------------------------------------------------------------------------------------

// Space a layer turns bones in (AnimationPose::AdditiveSpace, LayeredLerp's start bone): Local - each bone in its
// parent's space; Mesh - in model space, so the turn holds whatever the pose underneath does with the parents (aim
// sweeps). Default: what the layer does without a choice (the field's documentation says which).
enum class PoseSpace : uint8_t
{
	Invalid,
	Default,
	Local,
	Mesh
};

SLASHER_JSON_ENUM(PoseSpace, {
	{ PoseSpace::Invalid, nullptr },
	{ PoseSpace::Default, "Default" },
	{ PoseSpace::Local, "Local" },
	{ PoseSpace::Mesh, "Mesh" },
})

// ---- AI ---------------------------------------------------------------------------------------------

enum class AttackToken : uint8_t
{
	None,
	Melee,
	Ranged,
	Heavy,
	Count
};

SLASHER_JSON_ENUM(AttackToken, {
	{ AttackToken::Count, nullptr },
	{ AttackToken::None, "None" },
	{ AttackToken::Melee, "Melee" },
	{ AttackToken::Ranged, "Ranged" },
	{ AttackToken::Heavy, "Heavy" },
})

// ---- Entity references ------------------------------------------------------------------------------

// Resolves a handle and treats an entity that is already Destroyed (removed later this frame) as gone.
inline Entity* ResolveAlive(EntityHandle& handle)
{
	Entity* entity = handle.Resolve();
	return (entity != nullptr && !entity->Destroyed) ? entity : nullptr;
}
