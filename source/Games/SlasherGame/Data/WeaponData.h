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

// Parts of a weapon model held by a hand the weapon itself does not hold (the sword's scabbard in the left hand): hidden
// while that hand holds something else - another weapon, or this weapon's grip in a two-handed move (MoveData::grips).
struct WeaponCarriedData
{
	WeaponHand hand = WeaponHand::Left;
	std::vector<std::string> meshes;         // name prefixes (StaticMesh::MeshHideList)

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(WeaponCarriedData, hand, meshes)
};

// Attachment states (COMBAT_DESIGN.md §9.3): `hand` while the weapon is in the hands, `sheathed` while a sheathable
// weapon (WeaponData::sheathedHands) is held in its sheath, `holster` while it is put away (another weapon took the
// hands, or it is not the equipped weapon of its slot).
struct WeaponMeshData
{
	std::string model;
	std::string textures;                    // empty = "<model>/"
	WeaponAttachMode attach = WeaponAttachMode::Bone;
	float scale = 1.0f;                      // Bone mode (a Rig mesh uses the body's transform)
	WeaponAttachData hand;
	WeaponAttachData sheathed;
	WeaponAttachData holster;
	bool visibleWhenHolstered = true;
	std::vector<WeaponCarriedData> carried;  // in the hands only

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(WeaponMeshData, model, textures, attach, scale, hand, sheathed, holster,
		visibleWhenHolstered, carried)
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

// How the weapon gets longer for moves with a `reach` (COMBAT_DESIGN.md §9.3): this weapon bone of its Rig model
// moves out along `axis` - the part of the mesh skinned to it (the scythe's head) and the blade points on it go along.
struct WeaponExtendData
{
	std::string bone;                                // empty: the weapon keeps its length
	glm::vec3 axis = glm::vec3(0.0f, 1.0f, 0.0f);    // in the bone's own space

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(WeaponExtendData, bone, axis)

	// Resolved
	hashed_string boneHash;
	glm::vec3 resolvedAxis = glm::vec3(0.0f, 1.0f, 0.0f);   // normalized
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
	// with restartOnShot play clip n (0 = clip, n = variants[n - 1]) for the n-th of them (akimbo recoil). A Rig model
	// skinned straight to a hand (no weapon bones) puts its muzzle on that hand bone.
	std::vector<WeaponPointData> muzzles{ WeaponPointData{ "", glm::vec3(0.0f, 0.05f, 0.2f) } };
	std::string fireSound;
	std::string muzzleParticle;
	float noiseRadius = 30.0f;       // AI hearing
	ProjectileData projectile;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(FirearmData, fireRate, magazine, reloadTime, spread, pellets, muzzles,
		fireSound, muzzleParticle, noiseRadius, projectile)
};

// Aim sweep (COMBAT_DESIGN.md §9.1): a clip of the weapon aimed from one pitch to the other, level at its middle frame.
// The middle frame is the weapon's hold pose; while the body aims, the clip at the aim pitch is added on top of the
// pose (after the weapon's own clips - recoil, reload - and the grip) as the difference from the middle frame, on
// the `bones` subtrees. It turns spine, arms and coat the way the clip does: the coat no longer turns with the spine.
struct WeaponSweepData
{
	std::string clip;                        // empty: not an aiming weapon
	std::vector<std::string> bones = { "spine_01" };
	PoseSpace space = PoseSpace::Mesh;
	float startPitch = -90.0f;               // degrees (up > 0) the weapon aims at on the clip's first frame...
	float endPitch = 90.0f;                  // ...and on its last; between them time follows pitch linearly on each half

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(WeaponSweepData, clip, bones, space, startPitch, endPitch)

	// Resolved
	std::vector<hashed_string> boneHashes;

	bool IsEnabled() const { return !clip.empty(); }
	// Clip time for an aim pitch (degrees), clamped to the clip.
	float TimeAt(float pitchDegrees, float duration) const;
};

// How the body holds the weapon while it is in the hands (COMBAT_DESIGN.md §9.1). Clips loop on the body's idle
// clock (or hold one frame: idleTime / holdTime) and cross-fade over `blend` when the weapon enters or leaves the
// hands.
struct WeaponStanceData
{
	std::string animation;                   // clip source; empty = the model of the weapon's first mesh
	std::string idle;                        // full-body idle replacing the locomotion idle; empty = keep it
	float idleTime = -1.0f;                  // >= 0: the idle is this frame of its clip (a ready stance)
	std::string hold;                        // layered over locomotion on each holdBones subtree (move clips pose
	std::vector<std::string> holdBones;      // the arms themselves); with a sweep: its middle frame (hold stays empty)
	float holdTime = -1.0f;                  // >= 0: the hold is this frame of its clip
	// Space the hold's start bones blend in; Default: mesh space at locomotion.upperBodyBone, local for
	// limbs. Mesh at a limb (the musket's left arm) keeps the arm aimed whatever the run does with the chest.
	PoseSpace holdSpace = PoseSpace::Default;
	// The hold goes over movement moves too (jumps, glides, dashes: their clips do not carry this weapon): a firearm
	// stays held in the air.
	bool holdOverMoves = false;
	// Aiming weapon: while it is in the hands the body faces the aim direction (strafes, turning at aimTurnRate) and
	// the sweep aims the pose at the aim point.
	WeaponSweepData sweep;
	float aimTurnRate = 900.0f;              // deg/s the body turns toward the aim direction
	// Hands holding the weapon. Their subtrees (hand + fingers) always take the grip of the clip posing the weapon -
	// this weapon's move / overlay clip while one drives it, its hold clip otherwise - in local space, after every
	// other layer: blends move the arm, never the grip.
	std::vector<std::string> handBones;
	float blend = 0.2f;                      // seconds

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(WeaponStanceData, animation, idle, idleTime, hold, holdBones, holdTime,
		holdSpace, holdOverMoves, sweep, aimTurnRate, handBones, blend)

	// Resolved
	std::string resolvedAnimation;
	std::string resolvedHold;                // the hold clip: `hold`, or the sweep clip (sampled at its middle)

	bool IsAiming() const { return sweep.IsEnabled(); }
};

struct WeaponData
{
	int version = 1;
	std::string name;                        // also the key of its moves in a moveset (MoveSetData::weaponMoves)
	std::string hudLabel;                    // shown on the HUD; empty = name
	WeaponKind kind = WeaponKind::Melee;
	// Usually one; two for akimbo props (shots alternate between them). None: the weapon is part of the character
	// model and its blade / muzzle points use body bones.
	std::vector<WeaponMeshData> meshes;
	WeaponStanceData stance;
	// Hands the weapon holds in the hands (COMBAT_DESIGN.md §9.3). A Secondary weapon taking a hand the Main weapon's
	// state needs moves the Main weapon to its other in-hand state (drawn <-> sheathed) if that one's hands are free,
	// else puts it away - and only then replaces its stance; one that leaves those hands free (a pistol in the off
	// hand) is layered over the Main weapon, which stays in the hands.
	std::vector<WeaponHand> hands = { WeaponHand::Right };
	// Melee weapons with a sheath held in the hands (a katana in its scabbard): the hands holding it sheathed. Such a
	// weapon rests sheathed; attacks draw it, the moveset's Sheathe action puts it back once its combo ended (a cue's
	// `sheathe` switches the meshes on the frame the blade goes in). Empty = no sheathed state.
	std::vector<WeaponHand> sheathedHands;
	// The stance while sheathed (`stance` is the drawn one); empty fields keep the locomotion clips (a plain run).
	WeaponStanceData sheathedStance;
	// Seconds after the weapon entered the hands or last fired before a non-Main weapon is put away: the body
	// plays the moveset's Draw action (or switches instantly without one). 0 = it stays in the hands.
	float holsterDelay = 0.0f;
	// Firearm without an idle in the hands: it is put away once no Fire move has run for holsterDelay seconds; its
	// stance fades out over stance.blend before the mesh goes (the arm lowers the weapon).
	bool holsterAfterFire = false;
	BladeData blade;
	WeaponExtendData extend;
	std::string trailParticle = "meleeTrail";
	float trailStart = 0.0f;                 // where along the blade the swing trail begins (0 = base, 1 = tip)
	FirearmData firearm;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(WeaponData, version, name, hudLabel, kind, meshes, stance, hands, sheathedHands,
		sheathedStance, holsterDelay, holsterAfterFire, blade, extend, trailParticle, trailStart, firearm)

	// Resolved
	std::string sourcePath;
	std::vector<std::string> sockets;     // distinct weapon-model bones used by blade / muzzle points
	HandMask handMask = 0;
	HandMask sheathedHandMask = 0;

	bool HasRigMesh() const;
	bool CanSheathe() const { return sheathedHandMask != 0; }
	// The weapon has its own sheathed stance (CharacterData::GetSheathedStanceKey).
	bool HasSheathedStance() const { return CanSheathe(); }
};

bool ResolveWeapon(WeaponData& data, DataLoadReport& report);
