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
// Threading: AsyncUpdate poses and places the meshes from the body's final pose (own entity only). Sweeps, firing
// and trail requests run in the body's parallel update (COMBAT_DESIGN.md §2.2) and read the socket transforms of the
// previous AsyncUpdate; hits go to the body's queue, shots and trail entities are created on the main thread
// (FlushShots from the body's flush, trails in LateUpdate).
class WeaponSet
{
public:
	void Initialize(CombatCharacter* owner);

	// LoadAssets path, after the body mesh loaded: creates weapon meshes (owned by the character's Drawables),
	// resolves Rig skeletons and rest poses, preloads weapon VFX.
	void LoadAssets(const CharacterData& data);
	void Destroy();

	const WeaponData* GetWeapon(WeaponSlot slot) const;
	bool HasWeapon(WeaponSlot slot) const { return GetWeapon(slot) != nullptr; }

	// The weapon in the hands; the other one is holstered. The last used slot stays in the hands until its
	// holsterDelay runs out (IsHolsterDue).
	WeaponSlot GetActiveSlot() const { return activeSlot; }
	void SetActiveSlot(WeaponSlot slot);
	// The weapon in `slot` has a holsterDelay that passed since it entered the hands or last fired.
	bool IsHolsterDue(WeaponSlot slot) const;

	// ---- Melee (driven by MoveRunner hit windows) ----------------------------------------------------
	void BeginHitWindow(int windowIndex);
	void SweepHitWindow(MoveSlot& slot, int windowIndex);
	void EndHitWindow(int windowIndex);

	// The weapon in `slot` is `meters` longer while in the hands (the moves' reach, WeaponData::extend). Set by the
	// body every pose frame, before AsyncUpdate.
	void SetExtension(WeaponSlot slot, float meters) { Slot(slot).extension = meters; }

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
		int hideState = -1;                       // attachment state whose `hide` list the mesh shows

		// Rig mode
		bool rigReady = false;
		std::vector<RigBone> rigBones;
		RigRest handRest;
		RigRest holsterRest;
		hashed_string extendBone;                 // WeaponData::extend bone; empty: this model does not extend
		glm::vec3 extendAxis = glm::vec3(0.0f);   // in the bone's space
		glm::vec3 extendModelAxis = glm::vec3(0.0f);   // the axis in model space (bind pose): model units per bone unit
		AnimationPose current;                    // last output values, in the spaces of the current state
		AnimationPose fadeFrom;
		float fadeDuration = 0.0f;
		float fadeElapsed = 0.0f;
		uint32_t driveSerial = 0;
		int state = -1;                           // 1 = in the hands, 0 = holstered, -1 = not posed yet
	};

	struct SlotState
	{
		std::shared_ptr<const WeaponData> data;
		std::vector<MeshInstance> meshes;
		float fireCooldown = 0.0f;
		int magazine = 0;
		float reloadRemaining = 0.0f;
		int nextMuzzle = 0;                       // position in the shot cycle
		Delay holster;                            // holsterDelay, restarted on entering the hands and on every shot
		particle_system_meleeTrail* trail = nullptr;
		bool trailWanted = false;                 // requested by cues; the entity follows in SyncTrails
		float extension = 0.0f;                   // m (SetExtension)
	};

	struct PendingShot
	{
		WeaponSlot slot = WeaponSlot::Secondary;
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
	WeaponSlot activeSlot = WeaponSlot::Main;
	std::vector<HitWindowState> windows;
	std::vector<glm::vec3> sampleScratch;
	std::vector<PendingShot> pendingShots;
	bool visible = true;

	SlotState& Slot(WeaponSlot slot) { return slots[static_cast<size_t>(slot)]; }
	const SlotState& Slot(WeaponSlot slot) const { return slots[static_cast<size_t>(slot)]; }

	void SetupRig(MeshInstance& instance, const WeaponData& weapon, const roj::FlatSkeleton& bodySkeleton);
	bool SampleRest(MeshInstance& instance, const WeaponData& weapon, const WeaponAttachData& attach,
		const roj::FlatSkeleton& bodySkeleton, const AnimationPose& bindPose, RigRest& rest);
	// Rig values of `clip` (a pose of the weapon model). Bones the clip lacks take their value from `fallback`.
	static AnimationPose RigValues(const MeshInstance& instance, const AnimationPose& clip, const hashed_string& follow,
		const AnimationPose* fallback);
	void PlaceBoneMesh(MeshInstance& instance, const WeaponAttachData& attach, SkeletalMesh& body);
	static void PoseBuiltIn(MeshInstance& instance, SkeletalMesh& body, const AnimationPose& bodyPose);
	void PoseRigMesh(MeshInstance& instance, bool inHands, float extension, SkeletalMesh& body, const AnimationPose& bodyPose,
		const CharacterAnimator::WeaponDrive& drive, float deltaTime);

	// World position of a weapon point on a posed mesh; false if the mesh is not posed or lacks the point's bone.
	static bool GetPoint(const MeshInstance& instance, const WeaponPointData& point, glm::vec3& out);
	void ComputeSamples(const HitWindowData& window, std::vector<glm::vec3>& out) const;
	void SyncTrails();                                            // main thread
	static void PlaceTrail(SlotState& state);
	void EmitHit(const MoveSlot& slot, const HitWindowData& window, Entity* victim, const glm::vec3& point,
		const glm::vec3& normal, const glm::vec3& sweepDirection, const std::string& hitbox);
};
