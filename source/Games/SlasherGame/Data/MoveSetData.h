#pragma once

#include <Combat/CombatTypes.h>
#include <Data/DataLoadReport.h>

#include <Helpers/JsonHelper.hpp>
#include <utility/hashed_string.hpp>

#include <array>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

struct MoveBehaviorParams;

// Data contract of *.moveset.json - see COMBAT_DESIGN.md §5.2. All times are clip seconds (unscaled).
// Serialized fields use NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT (missing keys keep defaults). Fields
// below a "Resolved" comment are filled by ResolveMoveSet() and are never serialized.

enum class MoveTag : uint8_t
{
	Invalid,
	Attack,
	Melee,
	Ranged,
	Heavy,
	Dash,
	Jump,
	Glide,
	Fire,
	Reaction,
	Finisher,
	Death,
	Movement,
	Special,
	Locomotion,      // cancel target only: movement input ends the move (COMBAT_DESIGN.md §5.6)
	Count
};

SLASHER_JSON_ENUM(MoveTag, {
	{ MoveTag::Invalid, nullptr },
	{ MoveTag::Attack, "Attack" },
	{ MoveTag::Melee, "Melee" },
	{ MoveTag::Ranged, "Ranged" },
	{ MoveTag::Heavy, "Heavy" },
	{ MoveTag::Dash, "Dash" },
	{ MoveTag::Jump, "Jump" },
	{ MoveTag::Glide, "Glide" },
	{ MoveTag::Fire, "Fire" },
	{ MoveTag::Reaction, "Reaction" },
	{ MoveTag::Finisher, "Finisher" },
	{ MoveTag::Death, "Death" },
	{ MoveTag::Movement, "Movement" },
	{ MoveTag::Special, "Special" },
	{ MoveTag::Locomotion, "Locomotion" },
})

using TagMask = uint32_t;
inline constexpr TagMask TagBit(MoveTag tag) { return 1u << static_cast<uint8_t>(tag); }

enum class MoveChannel : uint8_t { Invalid, Primary, Overlay };
SLASHER_JSON_ENUM(MoveChannel, { { MoveChannel::Invalid, nullptr }, { MoveChannel::Primary, "Primary" }, { MoveChannel::Overlay, "Overlay" } })

// Where a move's clip shows:
//   Full       the whole body (an Overlay: its legs hand over to the pose underneath by legWeight as the body runs
//              or flies - a full-body reload keeps its footwork standing and runs on the move)
//   NoLegs     the whole body - hips included - with the legs (locomotion.legBones) blended toward the locomotion
//              pose by the move's legWeight, relative to the clip's hips: air attacks swing the body over the air pose
//   UpperBody  from locomotion.upperBodyBone up (mesh space), the hips and legs keep locomotion: overlays while running
//   Bones      only the subtrees of the move's `maskBones` (local space): a draw on one arm
enum class BodyMask : uint8_t { Invalid, Full, NoLegs, UpperBody, Bones };
SLASHER_JSON_ENUM(BodyMask, { { BodyMask::Invalid, nullptr }, { BodyMask::Full, "Full" }, { BodyMask::NoLegs, "NoLegs" },
	{ BodyMask::UpperBody, "UpperBody" }, { BodyMask::Bones, "Bones" } })

enum class OverlayPolicy : uint8_t { Invalid, Block, Allow };
SLASHER_JSON_ENUM(OverlayPolicy, { { OverlayPolicy::Invalid, nullptr }, { OverlayPolicy::Block, "Block" }, { OverlayPolicy::Allow, "Allow" } })

enum class MoveStartState : uint8_t { Invalid, Ground, Air, Any };
SLASHER_JSON_ENUM(MoveStartState, { { MoveStartState::Invalid, nullptr }, { MoveStartState::Ground, "Ground" }, { MoveStartState::Air, "Air" }, { MoveStartState::Any, "Any" } })

// Which entry variant a move starts with (R5). AfterHold: a hold move released during its wind-up handed over to it.
enum class MoveEntryKind : uint8_t { Fresh, AfterLight, AfterHeavy, AfterHold };

struct MoveEntryData
{
	float blendIn = 0.1f;
	float startTime = 0.0f;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(MoveEntryData, blendIn, startTime)
};

struct MoveEntriesData
{
	MoveEntryData fresh;
	MoveEntryData afterLight;
	MoveEntryData afterHeavy;
	MoveEntryData afterHold;

	const MoveEntryData& Get(MoveEntryKind kind) const
	{
		switch (kind)
		{
		case MoveEntryKind::AfterLight: return afterLight;
		case MoveEntryKind::AfterHeavy: return afterHeavy;
		case MoveEntryKind::AfterHold:  return afterHold;
		default:                        return fresh;
		}
	}

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(MoveEntriesData, fresh, afterLight, afterHeavy, afterHold)
};

// What a hit does in the air (COMBAT_DESIGN.md §5.8). A negative value takes the default of the character's airCombat
// (melee: hitLift and victimHold; bullets: firearmHold).
//   lift   a melee hit between two airborne fighters lifts both at least this fast (m/s) - the pair rises a little
//   stall  seconds the victim hovers after the hit (every hit restarts it). 0: no hover, the victim falls at once - a
//          plunge slams it down. A launching hit keeps its `launch` speed and hangs its victim at the top for
//          this long; if `launchSelf.onHit` also launches the attacker, the attacker shares this duration.
struct HitAirData
{
	float lift = -1.0f;
	float stall = -1.0f;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(HitAirData, lift, stall)
};

struct HitWindowData
{
	float start = 0.0f;
	float end = 0.0f;
	WeaponSlot weapon = WeaponSlot::Main;   // blade of this weapon slot...
	std::string bone;                       // ...or, if set, a sphere around this bone (kicks, shoulder charges)
	float radius = 0.25f;                   // bone sphere radius
	DamageSource source = DamageSource::Melee;
	float damage = 10.0f;
	float poiseDamage = 5.0f;
	float weakPointDamage = -1.0f;          // < 0: same as damage
	HitReaction reaction = HitReaction::Flinch;
	float knockback = 1.5f;                 // horizontal push speed (m/s)
	float launch = 0.0f;                    // upward velocity for Launch reactions (m/s)
	float hitStop = 0.05f;                  // seconds attacker and victim stop at the contact (easing back at the end)
	float hitStopScale = 0.03f;             // time scale while stopped (a near-freeze)
	float shake = 0.04f;                    // m the victim's mesh shakes along the hit direction during its hit-stop
	float camera = 0.0f;                    // view kick when the player lands it (impact units; 0 = none: light hits)
	float impactScale = 1.0f;               // multiplies the victim's hit effect (blood pieces)
	HitAirData air;
	bool direct = false;                    // hit the move's target directly, no sweep (finishers, grabs)
	std::string hitSound;                   // empty: the victim's hitEffect sound
	std::string hitParticle;                // empty: the victim's hitEffect particle

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(HitWindowData, start, end, weapon, bone, radius, source, damage,
		poiseDamage, weakPointDamage, reaction, knockback, launch, hitStop, hitStopScale, shake, camera, impactScale, air,
		direct, hitSound, hitParticle)

	// Resolved
	hashed_string boneHash;
	float resolvedWeakPointDamage = 0.0f;
};

struct ComboLinkData
{
	float windowStart = -1.0f;   // < 0: this move has no combo link
	float alternativeAt = -1.0f;
	float windowEnd = -1.0f;
	std::string next;            // light follow-up
	std::string alternative;     // heavy follow-up

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ComboLinkData, windowStart, alternativeAt, windowEnd, next, alternative)

	// Resolved
	int nextIndex = -1;
	int alternativeIndex = -1;
	bool IsValid() const { return windowStart >= 0.0f && (nextIndex >= 0 || alternativeIndex >= 0); }
};

struct CancelWindowData
{
	float start = 0.0f;
	float end = -1.0f;           // < 0: until the move ends
	std::vector<MoveTag> into;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(CancelWindowData, start, end, into)

	// Resolved
	TagMask intoMask = 0;
};

struct ArmorWindowData
{
	float start = 0.0f;
	float end = -1.0f;
	float poiseResist = 0.5f;    // 1 = hyper armor

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ArmorWindowData, start, end, poiseResist)
};

struct TimeWindowData
{
	float start = 0.0f;
	float end = -1.0f;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(TimeWindowData, start, end)
};

// Melee target assist (COMBAT_DESIGN.md §5.7, Attack). The target is picked when the move starts, all around the body
// (CharacterData::targeting: the one closest to the input direction - the facing without input - distance breaking
// near ties, the previous attack's target favoured); the body turns toward it - never snaps - and is pulled to
// `idealDistance` on top of the clip's root motion, so attacks without root motion still close in.
// Times are clip seconds; < 0 derives them from the hit windows.
struct TrackingData
{
	float searchRange = 6.0f;    // target search radius when the move starts (0 = no target)
	float until = -1.0f;         // turn toward the target (or the input direction) until this time; < 0: first hit start
	float startTurnRate = 1440.0f;   // deg/s during the first startTurnTime: the attack lines up fast...
	float startTurnTime = 0.12f;
	float turnRate = 720.0f;     // ...then follows the target at this rate (deg/s)
	float turnSharpness = 24.0f; // 1/s easing of the attack's turns (the body's own movement.turnSharpness is softer)
	float magnetRange = 3.0f;    // only targets this close are pulled toward; 0 = no magnetism
	float idealDistance = 1.4f;  // the pull stops here; root motion never carries the body closer
	float magnetUntil = -1.0f;   // close the gap until this time; < 0: end of the last hit window
	float maxMagnetSpeed = 9.0f; // m/s added to root motion at most
	float advance = 0.0f;        // m/s forward while move input is held and there is no target (until magnetUntil)

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(TrackingData, searchRange, until, startTurnRate, startTurnTime,
		turnRate, turnSharpness, magnetRange, idealDistance, magnetUntil, maxMagnetSpeed, advance)

	// Resolved
	float resolvedUntil = 0.0f;
	float resolvedMagnetUntil = 0.0f;
};

// Weapon reach (COMBAT_DESIGN.md §5.2, §9.3): the weapon in `weapon` is `length` m longer - fully from `start` to
// `end`, growing over blendIn before and shrinking back over blendOut after (the weapon's extend bone moves out). A
// move taking over blends the weapon to its own reach over its blendIn, like the clip's weapon bones.
// Times are clip seconds; < 0 derives them from the move's blade hits of that weapon.
struct ReachData
{
	float length = 0.0f;         // m added to the weapon; 0 = its own length
	float start = -1.0f;         // fully out from here; < 0: the first blade hit's start
	float end = -1.0f;           // going back from here; < 0: the last blade hit's end
	float blendIn = 0.12f;       // seconds it grows before start
	float blendOut = 0.2f;       // seconds it shrinks after end
	WeaponSlot weapon = WeaponSlot::Main;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ReachData, length, start, end, blendIn, blendOut, weapon)

	// Resolved
	float resolvedStart = 0.0f;
	float resolvedEnd = 0.0f;

	bool IsActive() const { return length > 0.0f; }
	float At(float time) const;  // m the weapon is longer at clip time `time`
};

struct RootMotionData
{
	bool enabled = true;
	float scale = 1.0f;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(RootMotionData, enabled, scale)
};

// Recovery (COMBAT_DESIGN.md §5.5): from `start` (default: the end of the combo window) the move releases the
// body - locomotion, facing and every other move are available immediately - while the rest of the clip keeps
// playing (the weapon returning to idle): on the whole body, moving the capsule, until the body moves; then the legs
// hand over to locomotion over lowerBodyBlend and the upper body over upperBodyBlend.
struct RecoveryData
{
	bool enabled = true;
	float start = -1.0f;          // < 0: combo.windowEnd if the move has a combo link, otherwise no recovery
	float lowerBodyBlend = 0.2f;  // seconds for the legs to hand over once the body moves
	float upperBodyBlend = 0.35f; // seconds for the upper body to hand over once the body moves; < 0: until the clip ends

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(RecoveryData, enabled, start, lowerBodyBlend, upperBodyBlend)
};

// Hold move (COMBAT_DESIGN.md §5.4): Attack must stay held through the wind-up, until `until` (clip seconds). Released
// earlier, the move hands over to `release` (entered with its entry.afterHold) - the launcher falls back to the usual
// attack.
struct HoldData
{
	float until = -1.0f;         // < 0: no hold rule
	std::string release;         // move taking over on an early release

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(HoldData, until, release)

	// Resolved
	int releaseIndex = -1;

	bool IsValid() const { return until >= 0.0f; }
};

struct CueData
{
	float time = 0.0f;
	std::string sound;           // FMOD event path
	std::string particle;        // GlobalParticleSystem name
	std::string bone;            // where sound / particle spawn (empty = character origin)
	int trail = -1;              // -1 unchanged, 0 stop weapon trail, 1 start weapon trail
	WeaponSlot trailWeapon = WeaponSlot::Main;
	float cameraShake = 0.0f;    // intensity, 0 = none
	int sheathe = -1;            // -1 unchanged, 1 the Main weapon goes into its sheath (the frame the blade is in), 0 out

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(CueData, time, sound, particle, bone, trail, trailWeapon, cameraShake, sheathe)
};

// A window where the clip holds its weapon with more hands than the weapon's own (the sword's two-handed swings): parts
// other hands carry (WeaponMeshData::carried, the scabbard) are hidden meanwhile. Clip seconds; runs on while the move
// plays out in recovery.
struct GripWindowData
{
	float start = 0.0f;
	float end = -1.0f;               // < 0: until the clip ends
	std::vector<WeaponHand> hands = { WeaponHand::Left };
	WeaponSlot weapon = WeaponSlot::Main;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(GripWindowData, start, end, hands, weapon)

	// Resolved
	HandMask handMask = 0;
};

// An additive layer of the move's clip (COMBAT_DESIGN.md §9.1): the clip's difference from its own pose at
// `referenceTime` is added to the `bones` subtrees (the root bone keeps the pose's) - before the bodyMask layer, which
// then replaces its own bones with the clip. Overlay moves only. The musket's shot: the left arm takes the clip, the
// whole body gets its kick (spine, coat).
struct MoveAdditiveData
{
	std::vector<std::string> bones;  // empty: no additive layer
	PoseSpace space = PoseSpace::Local;
	float referenceTime = 0.0f;      // clip seconds

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(MoveAdditiveData, bones, space, referenceTime)

	// Resolved
	std::vector<hashed_string> boneHashes;

	bool IsEnabled() const { return !bones.empty(); }
};

struct MoveData
{
	std::string behavior = "Attack";
	std::string clip;            // empty: no animation of its own (locomotion poses show), endTime required
	std::vector<std::string> variants;   // more clips from the same source sharing the move's timing; behaviours
	                                     // pick one (clip index n: 0 = clip, n = variants[n - 1])
	std::string animation;       // clip source file; empty = the moveset's only "animations" entry
	float speed = 1.0f;
	bool loop = false;           // while the behaviour sustains the move, the clip loops [loopStart, endTime]
	float loopStart = 0.0f;
	bool heavy = false;
	std::vector<MoveTag> tags;
	MoveChannel channel = MoveChannel::Primary;
	BodyMask bodyMask = BodyMask::Full;
	std::vector<std::string> maskBones;  // bodyMask Bones: the clip shows on these bones' subtrees (local space)
	float legWeight = 0.85f;             // bodyMask NoLegs (and Full in the air): how much the legs take the locomotion /
	                                     // air pose (the rest keeps the clip's own leg motion)
	bool airLegs = true;                 // bodyMask Full: in the air the legs blend toward the air pose by legWeight
	                                     // (false: the clip poses its own air legs - jumps, air dashes)
	MoveAdditiveData additive;
	// Space the clip's partial layers blend in - its UpperBody / Bones mask and the upper body of its recovery
	// hand-over. Default: mesh space at locomotion.upperBodyBone (the torso keeps the clip's facing), local at
	// limbs. Local keeps the torso on the running hips (a side-on sword stance must not twist off the legs).
	PoseSpace layerSpace = PoseSpace::Default;
	OverlayPolicy overlay = OverlayPolicy::Block;   // Block also suspends aiming (the body faces the move, not the aim)
	bool suspendsAim = false;                       // an Overlay that poses the weapon itself (a spinning reload): no sweep
	MoveStartState startState = MoveStartState::Ground;
	MoveEntriesData entry;
	float endTime = -1.0f;       // < 0: clip length
	float blendOut = -1.0f;      // < 0: moveset default
	float cooldown = 0.0f;       // seconds before this move can start again
	std::vector<HitWindowData> hits;
	ComboLinkData combo;
	HoldData hold;
	std::vector<CancelWindowData> cancels;
	std::vector<ArmorWindowData> armor;
	std::vector<TimeWindowData> invulnerable;
	std::vector<GripWindowData> grips;
	TrackingData tracking;
	ReachData reach;
	RootMotionData rootMotion;
	RecoveryData recovery;
	std::vector<CueData> cues;
	nlohmann::json params = nlohmann::json::object();

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(MoveData, behavior, clip, variants, animation, speed, loop, loopStart, heavy, tags, channel,
		bodyMask, maskBones, legWeight, airLegs, additive, layerSpace, overlay, suspendsAim, startState, entry, endTime, blendOut,
		cooldown, hits, combo, hold, cancels, armor, invulnerable, grips, tracking, reach, rootMotion, recovery, cues, params)

	// Resolved
	std::string id;
	int index = -1;
	int animationIndex = 0;
	hashed_string clipHash;
	std::vector<hashed_string> variantHashes;
	std::vector<hashed_string> maskBoneHashes;
	TagMask tagMask = 0;
	int behaviorType = -1;
	std::shared_ptr<const MoveBehaviorParams> behaviorParams;
	float resolvedBlendOut = 0.2f;
	float recoveryStart = -1.0f;  // clip time recovery begins; < 0: none

	bool HasTag(MoveTag tag) const { return (tagMask & TagBit(tag)) != 0; }
	bool HasClip() const { return !clip.empty(); }
	int GetClipCount() const { return HasClip() ? 1 + static_cast<int>(variantHashes.size()) : 0; }
	// Clip index n (wrapping): 0 = clip, n = variants[n - 1].
	const hashed_string& GetClipHash(int index) const
	{
		const int count = GetClipCount();
		const int wrapped = count > 0 ? ((index % count) + count) % count : 0;
		return wrapped == 0 ? clipHash : variantHashes[static_cast<size_t>(wrapped - 1)];
	}
};

// The moves a body starts from its combo and action slots (COMBAT_DESIGN.md §5.2): the moveset's own tables, or - for
// a body - those with its equipped weapons' entries over them (MoveSetData::BuildTables). -1 = nothing.
struct MoveTables
{
	std::array<int, static_cast<size_t>(ComboSlot::Count)> comboOpeners;
	std::array<int, static_cast<size_t>(ComboSlot::Count)> holdOpeners;
	std::array<int, static_cast<size_t>(ActionSlot::Count)> actions;

	MoveTables() { comboOpeners.fill(-1); holdOpeners.fill(-1); actions.fill(-1); }

	int GetComboOpener(ComboSlot slot) const { return comboOpeners[static_cast<size_t>(slot)]; }
	// The slot whose opener a press starts: AirAttack in the air if there is one, else Attack (one combo for both - its
	// moves start in the air too, COMBAT_DESIGN.md §5.8).
	ComboSlot GetOpenerSlot(bool grounded) const
	{
		return !grounded && GetComboOpener(ComboSlot::AirAttack) >= 0 ? ComboSlot::AirAttack : ComboSlot::Attack;
	}
	int GetHoldOpener(ComboSlot slot) const { return holdOpeners[static_cast<size_t>(slot)]; }
	int GetActionMove(ActionSlot slot) const { return actions[static_cast<size_t>(slot)]; }
};

// Moves of one weapon, by its WeaponData::name: while it is equipped its entries replace the moveset's. Combos and
// hold openers belong to the Main weapon, actions to the slot that owns them (ActionWeaponSlot): the melee weapon's
// draw, drop, finisher and sheathe, the firearm's fire and reload.
struct WeaponMovesData
{
	std::map<std::string, std::string> combos;
	std::map<std::string, std::string> holdOpeners;
	std::map<std::string, std::string> actions;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(WeaponMovesData, combos, holdOpeners, actions)

	// Resolved (-1 = keep the moveset's)
	MoveTables tables;
};

struct MoveSetData
{
	int version = 1;
	std::string name;
	std::vector<std::string> animations;
	float inputBuffer = 0.4f;        // s an Attack press waits for a combo window / cancel (combos queue up)
	float actionBuffer = 0.15f;      // s a Dash / Jump / Secondary / Finisher press waits: they happen now or not at all
	bool holdCarriesOver = true;     // the hold that started an attack counts as held for its combo link
	float holdDelay = 0.1f;          // s Attack held from rest before the hold opener starts (released earlier: the opener)
	float holdDelayInMove = 0.6f;    // s Attack held during a move (a combo, a dash) before the hold opener starts
	float blendOut = 0.2f;
	std::map<std::string, std::string> combos;
	std::map<std::string, std::string> holdOpeners;   // combo slot -> move started by holding Attack from neutral
	std::map<std::string, std::string> actions;
	std::map<std::string, std::string> reactions;
	std::map<std::string, WeaponMovesData> weaponMoves;   // weapon name -> its moves (over the tables above)
	std::vector<MoveData> moves;      // serialized as an object { "id": { ... } }

	// Resolved
	std::string sourcePath;
	std::unordered_map<std::string, int> moveIndexById;
	MoveTables tables;                // combos, holdOpeners, actions
	std::array<int, static_cast<size_t>(ReactionSlot::Count)> reactionMoves;

	MoveSetData() { reactionMoves.fill(-1); }

	int FindMove(const std::string& id) const
	{
		auto it = moveIndexById.find(id);
		return it == moveIndexById.end() ? -1 : it->second;
	}

	const MoveData* GetMove(int index) const
	{
		return (index >= 0 && index < static_cast<int>(moves.size())) ? &moves[static_cast<size_t>(index)] : nullptr;
	}

	// The moveset's own tables (without any weapon's moves).
	int GetComboOpener(ComboSlot slot) const { return tables.GetComboOpener(slot); }
	ComboSlot GetOpenerSlot(bool grounded) const { return tables.GetOpenerSlot(grounded); }
	int GetHoldOpener(ComboSlot slot) const { return tables.GetHoldOpener(slot); }
	int GetActionMove(ActionSlot slot) const { return tables.GetActionMove(slot); }
	int GetReactionMove(ReactionSlot slot) const { return reactionMoves[static_cast<size_t>(slot)]; }

	// The tables of a body holding these weapons (WeaponData::name; empty or unknown = none): the moveset's, with the
	// main weapon's combos, hold openers and Main-owned actions and the secondary weapon's Secondary-owned actions over
	// them.
	MoveTables BuildTables(const std::string& mainWeapon, const std::string& secondaryWeapon) const;

	// Hidden friends (found by ADL only), like the NLOHMANN macros generate. "moves" is an object keyed by id.
	friend void to_json(nlohmann::json& j, const MoveSetData& data) { WriteJson(j, data); }
	friend void from_json(const nlohmann::json& j, MoveSetData& data) { ReadJson(j, data); }
	static void WriteJson(nlohmann::json& j, const MoveSetData& data);
	static void ReadJson(const nlohmann::json& j, MoveSetData& data);
};

// Validates the parsed data and fills every "Resolved" field (indices, masks, hashes, behaviour params).
// Returns false if the moveset must not be used.
bool ResolveMoveSet(MoveSetData& data, DataLoadReport& report);

// Defaults of a move's serialized form, and of the elements of its arrays of structs (hits, cancels, armor,
// invulnerable, cues; nullptr for any other field) - what new content leaves out, what editors show as unset.
const nlohmann::json& DefaultMoveJson();
const nlohmann::json* MoveArrayElementDefaults(const std::string& field);

// A move's behaviour parameters for editing: every parameter of its behaviour at its default, overridden by what the
// move's "params" block sets. Back from editing: the block to store - what the author wrote, plus what now differs
// from the defaults (defaults are never written out).
nlohmann::json MoveParamsWithDefaults(const MoveData& move);
nlohmann::json MoveParamsToStore(const nlohmann::json& edited, const MoveData& move);

// Parse + convert + resolve a moveset file's text (no file access). `error`: the first problem.
bool LoadMoveSetText(const std::string& text, MoveSetData& out, std::string& error);

// Dev save (COMBAT_DESIGN.md §10.4): `text` - the moveset file as authored - rewritten so that it means `edited` where
// `edited` differs from `base`; only those values change in the text (JsonTextPatch), new content is written without
// default fields. Fails, leaving `out` unusable, when the file does not load, or when the result would not load back
// to exactly the file's data plus those edits.
bool PatchMoveSetText(const std::string& text, const MoveSetData& base, const MoveSetData& edited, std::string& out, std::string& error);
