#pragma once

#include <Combat/CombatTypes.h>
#include <Data/DataLoadReport.h>

#include <Helpers/JsonHelper.hpp>
#include <glm.h>
#include <utility/hashed_string.hpp>

#include <string>
#include <vector>

// Data contract of *.weapon.json (COMBAT_DESIGN.md §9.3, §10.2). Rotations are in degrees, engine pitch/yaw/roll
// convention. Blade and muzzle points are weapon-model bones plus an offset in that bone's space.

enum class WeaponKind : uint8_t { Invalid, Melee, Firearm };
SLASHER_JSON_ENUM(WeaponKind, { { WeaponKind::Invalid, nullptr }, { WeaponKind::Melee, "Melee" }, { WeaponKind::Firearm, "Firearm" } })

// How a weapon mesh is posed.
//   Bone: a rigid prop attached to a body bone.
//   Rig:  the model is the full character rig plus weapon bones, without a character mesh. It is drawn with the
//         body's transform and final pose; its weapon-only bones follow the clip driving the body (attack clips
//         come from this model) and otherwise rest in the attachment's `clip` pose.
enum class WeaponAttachMode : uint8_t { Invalid, Bone, Rig };
SLASHER_JSON_ENUM(WeaponAttachMode, { { WeaponAttachMode::Invalid, nullptr }, { WeaponAttachMode::Bone, "Bone" }, { WeaponAttachMode::Rig, "Rig" } })

struct WeaponAttachData
{
	// Bone mode: the body bone carrying the mesh (empty = hidden in this state).
	// Rig mode:  optional follow bone. The weapon's root bones keep the transform relative to this body bone that
	//            the posing clip has, so the weapon stays in the hand through any body blend. Empty: weapon bones
	//            are posed in their parent's space (fine when the rig parents them to the hand).
	std::string bone;
	glm::vec3 offset = glm::vec3(0.0f);     // Bone mode, in the bone's space
	glm::vec3 rotation = glm::vec3(0.0f);   // Bone mode
	std::string clip;                        // Rig mode: weapon-model clip posing the weapon bones in this state
	float clipTime = 0.0f;                   // Rig mode: seconds into `clip`
	std::vector<std::string> hide;           // meshes of the model hidden in this state (name prefix), e.g. the
	                                         // holstered copy while in the hands and the in-hand copy while holstered

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(WeaponAttachData, bone, offset, rotation, clip, clipTime, hide)

	// Resolved
	hashed_string boneHash;
};

struct WeaponMeshData
{
	std::string model;
	std::string textures;                    // empty = "<model>/"
	WeaponAttachMode attach = WeaponAttachMode::Bone;
	float scale = 1.0f;                      // Bone mode (a Rig mesh uses the body's transform)
	WeaponAttachData hand;                   // while this weapon is the one in the hands
	WeaponAttachData holster;                // while the other slot is in the hands
	bool visibleWhenHolstered = true;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(WeaponMeshData, model, textures, attach, scale, hand, holster, visibleWhenHolstered)
};

// A point on the weapon: `offset` in the space of a weapon-model bone, or in model space without a bone (rigid
// Bone meshes only - the weapon bones of a Rig mesh move independently of its origin).
struct WeaponPointData
{
	std::string bone;
	glm::vec3 offset = glm::vec3(0.0f);

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(WeaponPointData, bone, offset)

	// Resolved
	int socket = -1;                 // index into WeaponData::sockets; -1 = model space
};

struct BladeData
{
	WeaponPointData base;
	WeaponPointData tip{ "", glm::vec3(0.0f, 0.0f, 1.0f) };
	float radius = 0.06f;
	int samples = 4;                 // sweep points from base to tip (>= 2)

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(BladeData, base, tip, radius, samples)
};

struct ProjectileData
{
	float speed = 120.0f;
	float radius = 0.03f;
	float gravity = 0.0f;
	float maxDistance = 120.0f;
	float damage = 8.0f;
	float poiseDamage = 1.0f;
	float weakPointDamage = -1.0f;   // < 0: same as damage
	HitReaction reaction = HitReaction::None;
	float knockback = 0.0f;
	float hitStop = 0.0f;
	float hitStopScale = 0.06f;      // deepest time scale of the hit-stop
	std::string tracer = "bullet_trail";
	std::string impactParticle = "hit_flesh";
	std::string impactSound;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ProjectileData, speed, radius, gravity, maxDistance, damage, poiseDamage,
		weakPointDamage, reaction, knockback, hitStop, hitStopScale, tracer, impactParticle, impactSound)
};

struct FirearmData
{
	float fireRate = 6.0f;           // shots per second
	int magazine = 0;                // 0 = infinite, no reload
	float reloadTime = 1.2f;
	float spread = 0.6f;             // degrees (cone half-angle)
	int pellets = 1;
	// Shots cycle through every muzzle of every mesh (meshes in order, muzzles in order within a mesh); Fire moves
	// with restartOnShot play clip n (0 = clip, n = variants[n - 1]) for the n-th of them (akimbo recoil).
	std::vector<WeaponPointData> muzzles{ WeaponPointData{ "", glm::vec3(0.0f, 0.05f, 0.2f) } };
	std::string fireSound;
	std::string muzzleParticle;
	float noiseRadius = 30.0f;       // AI hearing
	ProjectileData projectile;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(FirearmData, fireRate, magazine, reloadTime, spread, pellets, muzzles,
		fireSound, muzzleParticle, noiseRadius, projectile)
};

// How the body holds the weapon while it is in the hands (COMBAT_DESIGN.md §9.1). Clips loop on the body's idle
// clock and cross-fade over `blend` when the weapon enters or leaves the hands.
struct WeaponStanceData
{
	std::string animation;                   // clip source; empty = the model of the weapon's first mesh
	std::string idle;                        // full-body idle replacing the locomotion idle; empty = keep it
	std::string hold;                        // layered over locomotion on each holdBones subtree (move clips pose
	std::vector<std::string> holdBones;      // the arms themselves)
	// Aiming weapon: while it is in the hands the body faces the aim direction (strafes) and these bones turn the
	// hold pose toward the aim point, parent first (spine aim offset). Empty = not an aiming weapon.
	std::vector<std::string> aimBones;
	float aimMaxPitch = 70.0f;               // degrees
	float aimMaxYaw = 60.0f;                 // degrees off the body's facing (covers the body still turning)
	float aimTurnRate = 900.0f;              // deg/s the body turns toward the aim direction
	// Hands holding the weapon. Their subtrees (hand + fingers) always take the grip of the clip posing the weapon -
	// this weapon's move / overlay clip while one drives it, its hold clip otherwise - in local space, after every
	// other layer: blends move the arm, never the grip.
	std::vector<std::string> handBones;
	float blend = 0.2f;                      // seconds

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(WeaponStanceData, animation, idle, hold, holdBones, aimBones, aimMaxPitch,
		aimMaxYaw, aimTurnRate, handBones, blend)

	// Resolved
	std::string resolvedAnimation;
};

struct WeaponData
{
	int version = 1;
	std::string name;
	WeaponKind kind = WeaponKind::Melee;
	// Usually one; two for akimbo props (shots alternate between them). None: the weapon is part of the character
	// model and its blade / muzzle points use body bones.
	std::vector<WeaponMeshData> meshes;
	WeaponStanceData stance;
	// Seconds after the weapon entered the hands or last fired before a non-Main weapon is put away: the body
	// plays the moveset's Draw action (or switches instantly without one). 0 = it stays in the hands.
	float holsterDelay = 0.0f;
	BladeData blade;
	std::string trailParticle = "meleeTrail";
	FirearmData firearm;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(WeaponData, version, name, kind, meshes, stance, holsterDelay, blade,
		trailParticle, firearm)

	// Resolved
	std::string sourcePath;
	std::vector<std::string> sockets;     // distinct weapon-model bones used by blade / muzzle points

	bool HasRigMesh() const;
};

bool ResolveWeapon(WeaponData& data, DataLoadReport& report);
