#pragma once

#include <Combat/CombatTypes.h>
#include <Data/DataLoadReport.h>
#include <Data/MoveSetData.h>
#include <Data/WeaponData.h>

#include <Helpers/JsonHelper.hpp>
#include <glm.h>
#include <utility/hashed_string.hpp>

#include <array>
#include <map>
#include <memory>
#include <string>
#include <vector>

// Data contract of *.character.json - see COMBAT_DESIGN.md §10.1.

struct CharacterMeshData
{
	std::string model = "GameData/models/npc/base.glb";
	std::string textures;            // empty = "<model>/"
	std::string meta;                // .skmm with hitboxes; empty = "<model>.skmm"
	float scale = 1.0f;
	float yawOffset = 0.0f;          // degrees, if the model does not face +Z
	bool hitboxes = true;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(CharacterMeshData, model, textures, meta, scale, yawOffset, hitboxes)
};

// Procedural strafing for bodies without directional run clips (runBackward / runLeft / runRight all empty): the
// legs run the forward clip turned toward the move direction (or the reversed clip turned away from it when moving
// backward) while the counter bones turn the torso back to the facing.
struct StrafeData
{
	bool enabled = true;
	std::string legsBone = "pelvis";
	std::vector<std::string> counterBones = { "spine_01", "spine_02", "spine_03" };
	float maxTwist = 75.0f;          // degrees the legs turn off the facing at most
	float turnRate = 600.0f;         // deg/s the legs follow a change of move direction
	float switchTime = 0.2f;         // seconds to cross-fade between running forward and backward

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(StrafeData, enabled, legsBone, counterBones, maxTwist, turnRate, switchTime)

	// Resolved
	hashed_string legsBoneHash;
	std::vector<hashed_string> counterBoneHashes;
};

// Procedural lean of the torso into acceleration on the ground (speeding up leans forward, turning leans in).
struct LeanData
{
	std::string bone = "spine_01";   // empty = no lean
	float forwardPerAcceleration = 0.5f;   // degrees per m/s^2 along the facing
	float sidePerAcceleration = 0.7f;      // degrees per m/s^2 across the facing
	float maxForward = 7.0f;               // degrees
	float maxSide = 9.0f;
	float sharpness = 6.0f;                // response (1/s); lower = heavier

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(LeanData, bone, forwardPerAcceleration, sidePerAcceleration, maxForward, maxSide, sharpness)

	// Resolved
	hashed_string boneHash;
};

struct LocomotionData
{
	std::string animation;           // clip source; empty = mesh model
	std::string idle = "idle";
	std::string runForward = "run";
	std::string runBackward = "run_b";
	std::string runLeft = "run_l";
	std::string runRight = "run_r";
	std::string fall;                // optional air pose (empty = idle)
	std::string glide;               // optional glide pose (empty = fall)
	std::string upperBodyBone = "spine_01";
	std::vector<std::string> legBones = { "thigh_l", "thigh_r" };   // keep locomotion under NoLegs moves
	float runClipSpeed = 5.5f;       // m/s at which the run clips play at 1x
	// Per-clip source files overriding `animation`, keyed by the field name ("idle": "<weapon model>.glb").
	std::map<std::string, std::string> sources;
	StrafeData strafe;
	LeanData lean;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(LocomotionData, animation, idle, runForward, runBackward, runLeft,
		runRight, fall, glide, upperBodyBone, legBones, runClipSpeed, sources, strafe, lean)

	// Resolved
	std::vector<hashed_string> legBoneHashes;
};

// Animation update rate by distance from the camera (COMBAT_DESIGN.md §2.4): far bodies evaluate their pose every
// 2nd / 4th frame (the move clock, movement and hitboxes still run every frame). The controlled body is always
// evaluated every frame.
struct AnimationLodData
{
	float halfRateDistance = 22.0f;  // m
	float quarterRateDistance = 45.0f;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(AnimationLodData, halfRateDistance, quarterRateDistance)
};

struct PoiseData
{
	bool enabled = true;
	float max = 40.0f;
	float regenDelay = 2.0f;
	float regenRate = 15.0f;         // per second
	float breakDuration = 3.0f;      // seconds the body stays Broken (finishable) after poise reaches 0
	float brokenDamageTaken = 1.5f;  // damage multiplier while Broken
	float firearmScale = 0.3f;       // poise damage multiplier for firearm body hits

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(PoiseData, enabled, max, regenDelay, regenRate, breakDuration,
		brokenDamageTaken, firearmScale)
};

enum class MovementQuality : uint8_t { Invalid, Player, NpcGeneric, NpcLowQuality };
SLASHER_JSON_ENUM(MovementQuality, { { MovementQuality::Invalid, nullptr }, { MovementQuality::Player, "Player" },
	{ MovementQuality::NpcGeneric, "NpcGeneric" }, { MovementQuality::NpcLowQuality, "NpcLowQuality" } })

struct MovementData
{
	float walkSpeed = 2.5f;
	float runSpeed = 6.0f;
	float acceleration = 40.0f;      // m/s^2 on ground
	float deceleration = 50.0f;
	float airControl = 0.35f;        // fraction of ground acceleration available in air
	float gravity = 25.0f;
	float turnRate = 720.0f;         // deg/s at most
	float turnSharpness = 12.0f;     // 1/s: turns ease into the target direction instead of stopping dead
	float radius = 0.4f;
	float height = 1.8f;
	float mass = 60.0f;
	MovementQuality quality = MovementQuality::NpcGeneric;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(MovementData, walkSpeed, runSpeed, acceleration, deceleration,
		airControl, gravity, turnRate, turnSharpness, radius, height, mass, quality)
};

// Air combat (COMBAT_DESIGN.md §5.8, the rules in Combat/AirJuggle.h). A body HOVERS - almost no gravity - while it
// juggles in the air, and every character has its own numbers (the hero's gravity is small but not off and runs out
// sooner than an enemy's: the player starts to fall first).
//   * attacker: a melee hit landed in the air starts its hover (swinging at air does not, nor does the ground launcher);
//               it lasts while the attack that landed it runs and `hold` after it
//   * victim:   hovers `victimHold` after every melee hit (each hit restarts it; a hit's air.stall may ask for more),
//               `firearmHold` after a bullet (which also stops its vertical motion: it keeps its height)
//   * hover: falling, `fallGravityScale` of the gravity and never faster than `maxFallSpeed` (a faster fall brakes
//     to it, `fallEase`); rising keeps the normal arc, so jumps and launches reach their top
//   * the hover ends: gravity fades back in over `gravityRamp`
//   * no infinite air time: `maxHangTime` seconds after the body's first hover of an airtime its gravity grows back
//     over `hangFade`, whatever hits it. Landing resets it.
struct AirCombatData
{
	float riseGravityScale = 1.0f;   // rising: the arc of the jump / launch
	float fallGravityScale = 0.02f;  // falling while hovering: the share of the gravity (small, not zero)
	float maxFallSpeed = 0.15f;      // m/s: the sink of a hovering body
	float fallEase = 20.0f;          // 1/s: a faster fall brakes to the hover sink (no snap)
	float hold = 0.5f;               // s the attacker hovers after the attack that landed an air hit ends
	float victimHold = 0.5f;         // s a victim hovers after a melee hit (every hit restarts it)
	float firearmHold = 0.2f;        // s a victim hovers after a bullet
	float gravityRamp = 0.4f;        // s gravity takes to return once the hover ended
	float maxHangTime = 3.0f;        // s after the body's first hover of an airtime at which its gravity starts to return
	float hangFade = 2.0f;           // s the hover's gravity then takes to grow back to the normal one
	float hitLift = 2.5f;            // m/s: a melee hit in the air lifts victim and attacker (a slight rise, ~0.15 m)
	float rootMotionScale = 0.5f;    // root motion of moves in the air (ground lunges would carry the body off)
	float knockbackScale = 0.5f;     // horizontal knockback taken in the air (the attacker stays in reach)

	// What a melee hit in the air does to its victim's position (read from the VICTIM's data), once, at the hit:
	// within `pullTime` it is moved to the attacker's height (+ the hit's rise) and the attack's ideal distance. The
	// destination is fixed at the hit - dash away and nothing follows.
	float pullTime = 0.18f;          // s the settle takes
	float pullRange = 1.0f;          // m beyond the ideal distance up to which the victim is pulled in
	float pullMaxHeight = 1.0f;      // m of height difference up to which it is levelled with the attacker
	float pullMaxSpeed = 12.0f;      // m/s

	// The launcher in the air (read from the ATTACKER's data): weaker than on the ground and weaker with every one
	// that connected since landing; melee hits of the combo between heal it - so combos keep the juggle going.
	float launchScale = 0.5f;        // strength of the first launcher of an airtime (fraction of the ground launch)
	float launchDecay = 0.6f;        // each earlier unhealed launcher multiplies the strength by this
	float launchRecover = 0.25f;     // strain one melee hit on an airborne victim heals (a launcher = 1)
	float launchMinScale = 0.15f;    // the strength never drops below this

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(AirCombatData, riseGravityScale, fallGravityScale, maxFallSpeed, fallEase,
		hold, victimHold, firearmHold, gravityRamp, maxHangTime, hangFade, hitLift, rootMotionScale, knockbackScale,
		pullTime, pullRange, pullMaxHeight, pullMaxSpeed, launchScale, launchDecay, launchRecover, launchMinScale)
};

// Melee target selection (COMBAT_DESIGN.md §5.7): every hostile within the move's tracking.searchRange, all around the
// body, scored by its angle from the input direction (the facing without input) plus distance; lowest score wins.
// Score = angle / 180 + distanceWeight * distance / searchRange - (keepBonus for the previous attack's target).
// In the air (§5.8) only bodies within airHeightRange above / below count: an enemy standing far below is not what an
// air swing goes for (the drop dives onto it).
struct TargetingData
{
	float distanceWeight = 0.5f;     // how much a closer body outweighs a better-aligned one
	float keepBonus = 0.2f;          // the previous attack's target wins near ties (= 36 degrees of angle)
	float inputThreshold = 0.3f;     // move input magnitude that counts as a direction
	float airHeightRange = 2.0f;     // m: an airborne attacker's targets are at most this far above / below it

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(TargetingData, distanceWeight, keepBonus, inputThreshold, airHeightRange)
};

// What this body shows where it is hit (COMBAT_DESIGN.md §9.4): blood, sparks, sand - a property of the victim. A hit
// window's hitParticle / hitSound override it; its impactScale multiplies `scale`.
struct HitEffectData
{
	std::string particle;            // registered particle system; spawned along the strike, scale.x = `scale`
	std::string sound;               // FMOD event
	float scale = 1.0f;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(HitEffectData, particle, sound, scale)
};

struct HitRecoilBoneData
{
	std::string bone;
	float weight = 1.0f;             // share of the recoil angle this bone takes (the chain adds up)

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(HitRecoilBoneData, bone, weight)

	// Resolved
	hashed_string boneHash;
};

// Procedural hit recoil (COMBAT_DESIGN.md §9.1): a hit bends the upper body away from the blow at once - the
// contact frame and the hit-stop already show it - and a damped spring brings it back in the body's local time, on
// top of whatever plays (reaction clips, attacks under armor).
struct HitRecoilData
{
	std::vector<HitRecoilBoneData> bones = { { "spine_01", 0.3f }, { "spine_02", 0.3f }, { "spine_03", 0.25f }, { "neck_01", 0.15f } };
	float degreesPerKnockback = 3.0f;   // recoil angle per m/s of the hit's knockback
	float maxAngle = 22.0f;             // degrees
	float frequency = 3.5f;             // Hz of the spring bringing the body back
	float damping = 0.55f;              // damping ratio (< 1: it sways through once)

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(HitRecoilData, bones, degreesPerKnockback, maxAngle, frequency, damping)
};

enum class WeakPointEffect : uint8_t { Invalid, BreakPoise, Kill, DamageBonus };
SLASHER_JSON_ENUM(WeakPointEffect, { { WeakPointEffect::Invalid, nullptr }, { WeakPointEffect::BreakPoise, "BreakPoise" },
	{ WeakPointEffect::Kill, "Kill" }, { WeakPointEffect::DamageBonus, "DamageBonus" } })

struct WeakPointData
{
	std::string name;
	std::vector<std::string> hitboxes;
	float health = 30.0f;
	std::vector<DamageSource> sources = { DamageSource::Firearm };
	float multiplier = 1.5f;         // health-damage multiplier for hits on this point while intact
	std::vector<WeakPointEffect> onBreak = { WeakPointEffect::BreakPoise };
	float bonusDamageTaken = 1.5f;   // DamageBonus: body damage multiplier while broken
	float regrow = 0.0f;             // seconds; 0 = stays broken

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(WeakPointData, name, hitboxes, health, sources, multiplier, onBreak,
		bonusDamageTaken, regrow)

	// Resolved
	DamageSourceMask sourceMask = 0;
	std::vector<hashed_string> hitboxHashes;
	bool breakPoise = false, kill = false, damageBonus = false;
};

struct AiAttackData
{
	std::string move;
	// Instead of a move: a combo slot ("Attack") or action slot ("Secondary") - the move of the equipped weapons in it
	// (MoveTables), so a body that switches weapons attacks with the one it holds.
	std::string slot;
	float minRange = 0.0f;
	float maxRange = 2.0f;
	float weight = 1.0f;
	float cooldown = 1.0f;
	AttackToken token = AttackToken::Melee;
	bool requireLineOfSight = true;
	float holdTime = 0.0f;           // seconds to hold the move's button after starting it (fire bursts, charges)
	InputButton holdButton = InputButton::Secondary;

	friend void to_json(nlohmann::json& j, const AiAttackData& a)
	{
		j = nlohmann::json{ { "move", a.move }, { "slot", a.slot }, { "minRange", a.minRange }, { "maxRange", a.maxRange }, { "weight", a.weight },
			{ "cooldown", a.cooldown }, { "token", a.token }, { "requireLineOfSight", a.requireLineOfSight },
			{ "holdTime", a.holdTime }, { "holdButton", InputButtonNames[static_cast<size_t>(a.holdButton)] } };
	}

	friend void from_json(const nlohmann::json& j, AiAttackData& a)
	{
		const AiAttackData d;
		a.move = j.value("move", d.move);
		a.slot = j.value("slot", d.slot);
		a.minRange = j.value("minRange", d.minRange);
		a.maxRange = j.value("maxRange", d.maxRange);
		a.weight = j.value("weight", d.weight);
		a.cooldown = j.value("cooldown", d.cooldown);
		a.token = j.value("token", d.token);
		a.requireLineOfSight = j.value("requireLineOfSight", d.requireLineOfSight);
		a.holdTime = j.value("holdTime", d.holdTime);
		const int button = FindSlotIndex(InputButtonNames, j.value("holdButton", std::string(InputButtonNames[static_cast<size_t>(d.holdButton)])));
		a.holdButton = button < 0 ? InputButton::Count : static_cast<InputButton>(button);
	}

	// Resolved
	int moveIndex = -1;              // `move`; -1 with a slot
	int comboSlot = -1;              // `slot` naming a ComboSlot
	int actionSlot = -1;             // `slot` naming an ActionSlot
};

struct AiData
{
	std::string brain = "Melee";
	float sightRadius = 25.0f;
	float fov = 140.0f;
	float loseTargetTime = 6.0f;     // seconds without sight before giving up
	float preferredMinRange = 1.5f;
	float preferredMaxRange = 3.0f;
	float aggression = 0.6f;         // 0..1, chance per decision to attack when allowed
	float reactionTime = 0.25f;      // seconds before reacting to a new target
	float decisionInterval = 0.3f;   // seconds between attack / strafe decisions
	float comboChance = 0.5f;        // chance to continue a combo at each link
	float heavyChance = 0.3f;        // chance a continued link is the heavy (alternative) follow-up
	float aimError = 2.0f;           // degrees of random aim error for firearms
	float strafeSpeed = 0.45f;       // fraction of run speed while circling
	float followDistance = 4.0f;     // allies: distance kept from the leader
	float engageRadius = 15.0f;      // allies: engage enemies within this distance of the leader
	std::vector<AiAttackData> attacks;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(AiData, brain, sightRadius, fov, loseTargetTime, preferredMinRange,
		preferredMaxRange, aggression, reactionTime, decisionInterval, comboChance, heavyChance, aimError, strafeSpeed,
		followDistance, engageRadius, attacks)
};

struct PossessionData
{
	bool allowed = true;
	std::string idleBrain = "Ally";  // brain driving this body while the player controls a different one

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(PossessionData, allowed, idleBrain)
};

struct FinisherData
{
	float range = 2.5f;              // attacker side: max distance to a finishable target
	float angle = 70.0f;             // attacker side: max angle off facing / input direction
	std::string reward;              // status effect granted on completion

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(FinisherData, range, angle, reward)
};

struct AttackTokenCapacityData
{
	int melee = 2;
	int ranged = 2;
	int heavy = 1;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(AttackTokenCapacityData, melee, ranged, heavy)
};

struct CharacterData
{
	int version = 1;
	std::string name;
	Team team = Team::Enemy;
	CharacterMeshData mesh;
	LocomotionData locomotion;
	AnimationLodData animationLod;
	std::string moveset;
	float health = 100.0f;
	PoiseData poise;
	MovementData movement;
	std::vector<WeakPointData> weakPoints;
	// Slot name -> weapon files. The first is equipped at start; the player cycles them (InputButton::SwitchMain /
	// SwitchSecondary, COMBAT_DESIGN.md §9.3).
	std::map<std::string, std::vector<std::string>> weapons;
	AiData ai;
	PossessionData possession;
	FinisherData finisher;
	AttackTokenCapacityData attackTokens;
	TargetingData targeting;
	AirCombatData airCombat;
	HitEffectData hitEffect;
	HitRecoilData hitRecoil;
	float corpseLifetime = 15.0f;    // seconds a dead body stays before it is removed; <= 0 keeps it

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(CharacterData, version, name, team, mesh, locomotion, animationLod, moveset,
		health, poise, movement, weakPoints, weapons, ai, possession, finisher, attackTokens, targeting, airCombat, hitEffect,
		hitRecoil, corpseLifetime)

	// Resolved
	std::string sourcePath;
	std::shared_ptr<const MoveSetData> moveSet;
	std::array<std::vector<std::shared_ptr<const WeaponData>>, static_cast<size_t>(WeaponSlot::Count)> weaponData;

	static constexpr size_t MaxWeapons = 16;   // all slots together (the animator keeps one stance per weapon)

	size_t GetWeaponCount(WeaponSlot slot) const { return weaponData[static_cast<size_t>(slot)].size(); }
	const WeaponData* GetWeapon(WeaponSlot slot, size_t index = 0) const
	{
		const auto& list = weaponData[static_cast<size_t>(slot)];
		return index < list.size() ? list[index].get() : nullptr;
	}
	// One index per weapon of every slot (Main's first): the animator's stance of a weapon.
	int GetWeaponKey(WeaponSlot slot, size_t index) const
	{
		size_t key = index;
		for (size_t s = 0; s < static_cast<size_t>(slot); ++s)
			key += weaponData[s].size();
		return index < GetWeaponCount(slot) ? static_cast<int>(key) : -1;
	}
	// The sheathed stance of a sheathable weapon (WeaponData::sheathedStance): keys after every weapon's, in the same
	// order; -1 for a weapon without one.
	int GetSheathedStanceKey(WeaponSlot slot, size_t index) const
	{
		const WeaponData* weapon = GetWeapon(slot, index);
		if (weapon == nullptr || !weapon->HasSheathedStance())
			return -1;
		size_t key = 0;
		for (const auto& list : weaponData)
			key += list.size();
		for (size_t s = 0; s < weaponData.size(); ++s)
		{
			for (size_t i = 0; i < weaponData[s].size(); ++i)
			{
				if (s == static_cast<size_t>(slot) && i == index)
					return static_cast<int>(key);
				if (weaponData[s][i]->HasSheathedStance())
					++key;
			}
		}
		return -1;
	}
	int FindWeakPointByHitbox(const hashed_string& hitbox) const
	{
		for (size_t i = 0; i < weakPoints.size(); ++i)
			for (const hashed_string& name : weakPoints[i].hitboxHashes)
				if (name == hitbox) return static_cast<int>(i);
		return -1;
	}
};

// Validates fields and fills resolved values that do not need other files (moveSet / weaponData are filled
// by CombatData, which then calls ResolveCharacterReferences).
bool ResolveCharacter(CharacterData& data, DataLoadReport& report);
bool ResolveCharacterReferences(CharacterData& data, DataLoadReport& report);
