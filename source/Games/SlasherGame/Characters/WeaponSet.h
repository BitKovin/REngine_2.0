#pragma once

#include <Characters/CharacterAnimator.h>
#include <Combat/CombatTypes.h>
#include <Data/CharacterData.h>
#include <Data/WeaponData.h>

#include <Delay.hpp>
#include <SkeletalMesh.hpp>
#include <glm.h>

#include <array>
#include <memory>
#include <vector>

class CombatCharacter;
class WeaponMesh;
class particle_system_meleeTrail;
struct MoveSlot;

// The body's weapons (COMBAT_DESIGN.md §3, §7, §9.3): weapon meshes (Bone props or Rig models), melee blade
// sweeps for hit windows, weapon trails and firearm state. Member of CombatCharacter.
//
// Each slot holds a list of weapons; one is equipped (Equip, cycled by the player). The equipped Main weapon is in
// the hands unless the Secondary one is out and takes every hand it could be held with; the Secondary weapon is out
// from its first shot until its holsterDelay runs out (holsterAfterFire: until no fire move ran for holsterDelay). A
// sheathable Main weapon (the sword) is drawn by attacks and sheathed again by its Sheathe move; a firearm taking its
// sheath hand draws it, one taking its sword hand sheathes it.
//
// Threading: AsyncUpdate poses and places the meshes from the body's final pose (own entity only). Sweeps, firing
// and trail requests run in the body's parallel update (COMBAT_DESIGN.md §2.2) and read the socket transforms of the
// previous AsyncUpdate; hits go to the body's queue, shots and trail entities are created on the main thread
// (FlushShots from the body's flush, trails in LateUpdate).
class WeaponSet
{
public:
	// Where a weapon mesh is (WeaponMeshData's attachment states).
	enum class Presentation : uint8_t { Hand, Sheathed, Holster };

	void Initialize(CombatCharacter* owner);

	// LoadAssets path, after the body mesh loaded: creates weapon meshes (owned by the character's Drawables),
	// resolves Rig skeletons and rest poses, preloads weapon VFX.
	void LoadAssets(const CharacterData& data);
	void Destroy();

	// ---- Equipment -----------------------------------------------------------------------------------
	const WeaponData* GetWeapon(WeaponSlot slot) const;          // the equipped one
	bool HasWeapon(WeaponSlot slot) const { return GetWeapon(slot) != nullptr; }
	size_t GetWeaponCount(WeaponSlot slot) const { return Slot(slot).weapons.size(); }
	size_t GetEquippedIndex(WeaponSlot slot) const { return Slot(slot).equipped; }
	// Equips weapon `index` of `slot`: the previous one goes to its holster (its trail stops). A sheathable Main weapon
	// starts sheathed; a Secondary weapon that was out stays out as the new one unless that one only comes out to fire.
	// The body updates its move tables (CombatCharacter::EquipWeapon).
	void Equip(WeaponSlot slot, size_t index);

	// The weapon of `slot` is in the hands (drawn or sheathed for Main).
	bool IsInHands(WeaponSlot slot) const;
	// Puts the weapon of `slot` in the hands: Secondary comes out (its holsterDelay restarts); Main puts the Secondary
	// away. Attacks bring out the weapon of their first hit window.
	void BringOut(WeaponSlot slot);
	// Puts the Secondary weapon away. With fadeSeconds its mesh stays in the hand that long while its stance fades out
	// (the arm lowers it) - it no longer counts as in the hands meanwhile.
	void PutAwaySecondary(float fadeSeconds = 0.0f);
	// The Secondary weapon has a holsterDelay that passed since it came out or last fired.
	bool IsHolsterDue() const;
	// The Secondary weapon is out and leaves the Main one in the hands: it is layered over the Main stance (a pistol in
	// the off hand) instead of replacing it.
	bool IsSecondaryLayered() const { return secondaryOut && MainFitsBeside(); }

	// The Main weapon rests in its sheath (WeaponData::sheathedHands; false for weapons without one).
	bool IsSheathed() const { return mainSheathed; }
	void SetSheathed(bool sheathed);
	Presentation GetMainPresentation() const;

	// Animator stance keys of the equipped weapons (CharacterData::GetWeaponKey; GetSheathedStanceKey while the Main
	// weapon is sheathed), -1 without one.
	int GetStanceKey(WeaponSlot slot) const;

	// Hands the clips of the moves playing hold the weapon of `slot` with beyond its own (MoveData::grips): the parts
	// those hands carry (WeaponMeshData::carried) hide. Set by the body every pose frame, before AsyncUpdate.
	void SetGripHands(WeaponSlot slot, HandMask hands) { Slot(slot).gripHands = hands; }

	// ---- Melee (driven by MoveRunner hit windows) ----------------------------------------------------
	void BeginHitWindow(int windowIndex);
	void SweepHitWindow(MoveSlot& slot, int windowIndex);
	void EndHitWindow(int windowIndex);

	// The weapon in `slot` is `meters` longer while in the hands (the moves' reach, WeaponData::extend). Set by the
	// body every pose frame, before AsyncUpdate.
	void SetExtension(WeaponSlot slot, float meters);

	// Requests the weapon trail on or off; the trail entity follows in LateUpdate (main thread).
	void SetTrail(WeaponSlot slot, bool enabled);
	void StopAllTrails();

	// ---- Firearms ------------------------------------------------------------------------------------
	void Tick(float deltaTime);                                   // fire cooldown + reload timers
	bool IsReadyToFire(WeaponSlot slot) const;
	// Fires one shot (all pellets) toward aimPoint from the next muzzle. Returns the index of that muzzle in the
	// shot cycle (every muzzle of every mesh, FirearmData::muzzles), -1 if the weapon was not ready. Ammo and timers
	// change at once; projectiles, sound, muzzle flash and noise are created by FlushShots.
	int Fire(WeaponSlot slot, const glm::vec3& aimPoint);
	void FlushShots();                                            // main thread (the body's flush)
	bool StartReload(WeaponSlot slot);
	bool IsReloading(WeaponSlot slot) const;
	int GetMagazine(WeaponSlot slot) const;                      // -1 = infinite

	// ---- Visuals -------------------------------------------------------------------------------------
	// After the body pose was pasted: places Bone meshes on body bones, poses Rig meshes from the body pose and
	// the clips driving it. deltaTime is the body's local time.
	void AsyncUpdate(SkeletalMesh& body, const AnimationPose& bodyPose, const CharacterAnimator::WeaponDrive& drive, float deltaTime);
	void LateUpdate();                                            // move trail effects (main thread)
	void SetVisible(bool visible);

private:
	// A bone of a Rig model that the body skeleton does not have.
	struct RigBone
	{
		hashed_string name;
		hashed_string parent;                     // empty: no parent
		bool root = false;                        // its parent is a body bone (or none)
	};

	// Weapon-bone values of one attachment state, keyed by bone name (one entry per RigBone). Root bones are
	// relative to the follow bone when there is one; every other value is parent-local.
	struct RigRest
	{
		hashed_string follow;                     // empty: no follow bone
		AnimationPose values;
	};

	// One posed weapon model - or, for a weapon without meshes, the body itself (mesh == nullptr, data == nullptr).
	struct MeshInstance
	{
		WeaponMesh* mesh = nullptr;
		const WeaponMeshData* data = nullptr;
		// Per WeaponData::sockets entry (the bones blade / muzzle points use):
		std::vector<hashed_string> socketBones;   // empty name: the model lacks the bone
		std::vector<glm::mat4> socketBind;        // Bone mode: model-space socket in the (never animated) bind pose
		std::vector<glm::mat4> sockets;           // world space, written in AsyncUpdate
		glm::mat4 world = glm::mat4(1.0f);        // model to world, written in AsyncUpdate
		bool hasTransform = false;
		int hideState = -1;                       // Presentation whose `hide` list the mesh shows...
		HandMask hideBusyHands = 0;               // ...and the busy hands whose carried parts it hides

		// Rig mode
		bool rigReady = false;
		std::vector<RigBone> rigBones;            // may be empty: a model skinned to body bones only (a gun in the hand)
		std::array<RigRest, 3> rests;             // per Presentation
		hashed_string extendBone;                 // WeaponData::extend bone; empty: this model does not extend
		glm::vec3 extendAxis = glm::vec3(0.0f);   // in the bone's space
		glm::vec3 extendModelAxis = glm::vec3(0.0f);   // the axis in model space (bind pose): model units per bone unit
		AnimationPose current;                    // last output values, in the spaces of the current state
		AnimationPose fadeFrom;
		float fadeDuration = 0.0f;
		float fadeElapsed = 0.0f;
		uint32_t driveSerial = 0;
		int state = -1;                           // Presentation posed last, -1 = not posed yet
	};

	// One weapon of a slot.
	struct WeaponState
	{
		std::shared_ptr<const WeaponData> data;
		int stanceKey = -1;                       // CharacterData::GetWeaponKey
		int sheathedStanceKey = -1;               // CharacterData::GetSheathedStanceKey
		std::vector<MeshInstance> meshes;
		float fireCooldown = 0.0f;
		int magazine = 0;
		float reloadRemaining = 0.0f;
		int nextMuzzle = 0;                       // position in the shot cycle
		particle_system_meleeTrail* trail = nullptr;
		bool trailWanted = false;                 // requested by cues; the entity follows in SyncTrails
		float extension = 0.0f;                   // m (SetExtension)
	};

	struct SlotState
	{
		std::vector<WeaponState> weapons;
		size_t equipped = 0;
		HandMask gripHands = 0;                   // SetGripHands
	};

	struct PendingShot
	{
		std::shared_ptr<const WeaponData> weapon;
		glm::vec3 muzzle = glm::vec3(0.0f);
		glm::vec3 direction = glm::vec3(0.0f, 0.0f, 1.0f);
	};

	struct HitWindowState
	{
		std::vector<glm::vec3> previousSamples;
		bool hasPrevious = false;
	};

	CombatCharacter* owner = nullptr;
	std::array<SlotState, static_cast<size_t>(WeaponSlot::Count)> slots;
	bool secondaryOut = false;                    // the Secondary weapon is in the hands
	float secondaryLeaving = 0.0f;                // s its mesh still shows while its stance fades (PutAwaySecondary)
	bool mainSheathed = false;                    // the Main weapon rests in its sheath
	Delay holster;                                // Secondary holsterDelay, restarted on coming out and on every shot
	std::vector<HitWindowState> windows;
	std::vector<glm::vec3> sampleScratch;
	std::vector<PendingShot> pendingShots;
	bool visible = true;

	SlotState& Slot(WeaponSlot slot) { return slots[static_cast<size_t>(slot)]; }
	const SlotState& Slot(WeaponSlot slot) const { return slots[static_cast<size_t>(slot)]; }
	WeaponState* Equipped(WeaponSlot slot);
	const WeaponState* Equipped(WeaponSlot slot) const;

	// Hands the out Secondary weapon takes (0 when it is away).
	HandMask BusyHands() const;
	// Hands holding something other than the weapon of `slot` (the other slot's weapon in the hand, its own two-handed
	// grip): the parts of its model those hands carry hide.
	HandMask CarryingHandsBusy(WeaponSlot slot) const;
	// The Main weapon has an in-hand state (drawn, or sheathed) whose hands the Secondary weapon leaves free.
	bool MainFitsBeside() const;
	void FitMainToHands();
	Presentation GetPresentation(WeaponSlot slot, size_t index) const;

	void SetupRig(MeshInstance& instance, const WeaponData& weapon, const roj::FlatSkeleton& bodySkeleton);
	bool SampleRest(MeshInstance& instance, const WeaponData& weapon, const WeaponAttachData& attach,
		const roj::FlatSkeleton& bodySkeleton, const AnimationPose& bindPose, RigRest& rest);
	// Rig values of `clip` (a pose of the weapon model). Bones the clip lacks take their value from `fallback`.
	static AnimationPose RigValues(const MeshInstance& instance, const AnimationPose& clip, const hashed_string& follow,
		const AnimationPose* fallback);
	void PlaceBoneMesh(MeshInstance& instance, const WeaponAttachData& attach, SkeletalMesh& body);
	static void PoseBuiltIn(MeshInstance& instance, SkeletalMesh& body, const AnimationPose& bodyPose);
	void PoseRigMesh(MeshInstance& instance, Presentation presentation, int stanceKey, float extension, SkeletalMesh& body,
		const AnimationPose& bodyPose, const CharacterAnimator::WeaponDrive& drive, float deltaTime);

	// World position of a weapon point on a posed mesh; false if the mesh is not posed or lacks the point's bone.
	static bool GetPoint(const MeshInstance& instance, const WeaponPointData& point, glm::vec3& out);
	void ComputeSamples(const HitWindowData& window, std::vector<glm::vec3>& out) const;
	void SyncTrails();                                            // main thread
	static void PlaceTrail(WeaponState& state);
	void EmitHit(const MoveSlot& slot, const HitWindowData& window, Entity* victim, const glm::vec3& point,
		const glm::vec3& normal, const glm::vec3& sweepDirection, const std::string& hitbox);
};
