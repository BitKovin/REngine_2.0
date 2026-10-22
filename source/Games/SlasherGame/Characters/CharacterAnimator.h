#pragma once

#include <Data/CharacterData.h>
#include <Data/MoveSetData.h>

#include <Animator.h>
#include <utility/hashed_string.hpp>

#include <array>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// What the animator needs for one evaluation. Built by CombatCharacter in UpdatePose from its own state.
struct AnimatorInput
{
	struct Layer
	{
		int animation = -1;              // index into the moveset's "animations"
		hashed_string clip;
		float time = 0.0f;               // authoritative move clock (clip seconds)
		uint32_t serial = 0;             // changes when a new move starts; 0 = no move
		float blendIn = 0.0f;
		float blendOut = 0.2f;
		BodyMask mask = BodyMask::Full;
		const std::vector<hashed_string>* maskBones = nullptr;   // mask Bones: the subtrees the clip shows on
		float legWeight = 1.0f;          // mask NoLegs (Full in the air): share of the legs taken from the locomotion / air pose
		bool airLegs = true;             // mask Full: in the air the legs blend toward the air pose (MoveData::airLegs)
		const MoveAdditiveData* additive = nullptr;   // overlay: the clip's additive layer (null or disabled = none)
		PoseSpace space = PoseSpace::Default;          // MoveData::layerSpace
		bool movement = false;           // a Movement move (its clip carries no weapon: holdOverMoves stances go on top)
		bool rootMotion = false;
	};

	// Continuation of an attack that ended in recovery (MoveRunner::GetRecovery(), COMBAT_DESIGN.md §5.5).
	struct Recovery
	{
		int animation = -1;
		hashed_string clip;
		float time = 0.0f;
		uint32_t serial = 0;             // serial of the attack it continues; 0 = no recovery
		bool released = false;           // the body moves: hand over to locomotion (until then the whole body plays)
		bool rootMotion = false;         // the clip moves the capsule until released
		float lowerBodyBlend = 0.2f;     // seconds for the legs to hand over once released
		float upperBodyBlend = 0.35f;    // seconds for the upper body to hand over once released; < 0: until the clip ends
		float blendOut = 0.2f;           // seconds for whatever still plays to hand over once the clip ends
		PoseSpace space = PoseSpace::Default;   // MoveData::layerSpace of the upper-body hand-over
		bool movement = false;
	};

	glm::vec2 localVelocity = glm::vec2(0.0f);   // x = right, y = forward (m/s)
	bool grounded = true;
	bool gliding = false;
	bool aiming = false;                         // the stances' aim sweeps apply (fade in / out)
	bool strafing = false;                       // facing is not driven by the movement (aiming, firing): run backward
	// Weapon stances shown (bit = CharacterData::GetWeaponKey, or GetSheathedStanceKey for a sheathed weapon): the
	// weapons in the hands. Layered ones go over the others instead of replacing them (a pistol in the off hand beside
	// the melee weapon).
	uint32_t stances = 0;
	uint32_t layeredStances = 0;
	glm::vec3 forward = glm::vec3(0.0f, 0.0f, 1.0f);       // body facing, model space
	glm::vec3 velocity = glm::vec3(0.0f);                  // horizontal, model space (m/s)
	glm::vec3 acceleration = glm::vec3(0.0f);              // horizontal, model space (m/s^2)
	glm::vec3 aimDirection = glm::vec3(0.0f, 0.0f, 1.0f);  // from the chest to the aim point, model space
	glm::vec3 recoil = glm::vec3(0.0f);                    // hit recoil: rotation vector (radians), model space
	Layer primary;
	Layer overlay;
	Recovery recovery;
};

// Body animation (COMBAT_DESIGN.md §9.1), built on the engine Animator. Owned by CombatCharacter and only used
// from its UpdatePose. Composition, bottom to top:
//
//   locomotion: idle (or the stance idle of the weapon in the hands) + 4-direction run, phase-synced, playback
//               rate from speed. Without strafe / backward clips the run is procedural: the legs run the forward
//               clip turned toward the move direction (the reversed clip when strafing backward) while the torso
//               turns back to the facing. Lean into acceleration on the ground; fall / glide poses in air.
//   stance hold: the hold clip of the weapon in the hands on its holdBones subtrees, over locomotion only (move
//               clips pose the arms themselves); layered stances (a weapon beside the one in the hands) on top
//   full-body primary: replaces all of the above (NoLegs: except the legs, which keep the locomotion / air pose
//               under the clip's hips; a Full move does the same in the air, faded in as the body leaves the ground -
//               ground attacks performed in the air). A Movement move (jump, glide, dash) gets the holds of the weapons
//               whose stance asks for it (holdOverMoves: firearms) on top - its clip does not carry them
//   recovery:   the clip of an attack that ended in recovery keeps playing on the same Animation (no restart, no
//               transition) - on the whole body, moving the capsule, until the body moves; then the legs hand over
//               to locomotion over lowerBodyBlend and the upper body over upperBodyBlend
//   upper-body / bone-masked primary, then overlay (firing, drawing) - from upperBodyBone or the move's maskBones, in
//               the move's layerSpace; an overlay with an additive layer first adds the clip's difference from its
//               reference frame (the musket's kick through spine and coat). A Full overlay replaces the whole body (the
//               rifle's spinning reload) and hands its legs to the pose underneath as the body runs or flies. The
//               overlay keeps its shape through its fade-out, and one replacing another cross-fades the two results
//   grip:       the stance's hand bones take the grip of the clip posing the weapon (local space), so the weapon
//               stays tight in the hand through every blend
//   aim sweep:  weapons with a sweep add the sweep clip at the aim pitch as its difference from its middle frame (the
//               hold pose) - mesh space by default, faded by `aiming` - on top of their own clips and the grip
//   hit recoil: the hitRecoil bones bend along the blow (CharacterData::hitRecoil, spring state from the body) - on top
//               of the final pose, outside transitions (a reaction starting mid-recoil does not freeze it in)
//   transitions: any primary change (start / end / interruption) freezes the last output pose and blends it
//                into the new composition over the move's blendIn (or the ended move's blendOut); stances
//                cross-fade over their blend when the weapon in the hands changes
//   root motion: delta of the "root" bone of the primary (or unreleased recovery) clip; root translation is
//                stripped from the pose. Layers never change the root bone (it belongs to the legs).
//
// Clips of models that lack some of the body's bones (a weapon model without the coat) leave those bones to the
// pose underneath; clips with every body bone blend without that copy. Layers rooted at upperBodyBone blend that
// bone's rotation in mesh space (the torso keeps the layer's facing whatever the legs do); layers rooted elsewhere
// (an arm) blend in local space.
class CharacterAnimator : public Animator
{
public:
	// Clip poses that drive the own bones of Rig weapons (COMBAT_DESIGN.md §9.3). Attack clips come from the
	// weapon model, so they carry the weapon bones. Pointers are valid until the next Evaluate.
	struct WeaponDrive
	{
		const AnimationPose* move = nullptr;   // primary move clip, or the recovery clip
		float moveWeight = 0.0f;
		uint32_t moveSerial = 0;               // changes when another clip takes over (recovery keeps its attack's)
		float moveTransition = 0.0f;           // seconds to cross-fade the weapon bones after moveSerial changed
		int moveWeapon = -1;                   // weapon (stance key) whose model the clip comes from; -1 = none
		const AnimationPose* overlay = nullptr;
		float overlayWeight = 0.0f;
		int overlayWeapon = -1;
	};

	explicit CharacterAnimator(Entity* owner) : Animator(owner) {}

	// Must be called before LoadAssetsIfNeeded(). bodySkeleton: the body model's skeleton (which bones a clip must
	// have to blend without completion); null = complete every clip.
	void Configure(std::shared_ptr<const CharacterData> data, const roj::FlatSkeleton* bodySkeleton);

#ifndef DISTRIBUTION
	// Dev reload: loads clip sources a reloaded moveset added (explicit developer action only).
	void EnsureMoveSetAnimations(const MoveSetData& moveSet);
#endif

	// Samples every contributing clip for this evaluation (deltaTime: local seconds since the previous one). Then
	// call ComposePose() and ConsumeRootMotion().
	void Evaluate(const AnimatorInput& input, float deltaTime);

	// Composes the evaluated clips into the animator's own pose and returns it (GetPose() until the next call). Every
	// blend works in place on storage kept between evaluations, so a steady composition does not allocate.
	// GetResultPose() returns a copy of the same pose.
	const AnimationPose& ComposePose();
	const AnimationPose& GetPose() const { return output; }

	// Model-space root translation of the primary clip since the previous Evaluate (horizontal).
	glm::vec3 ConsumeRootMotion();

	// After GetResultPose(): what poses Rig weapon bones this frame.
	WeaponDrive GetWeaponDrive();

	// Clip length in seconds, 0 if the clip does not exist. Valid after LoadAssets.
	float GetClipDuration(int animation, const hashed_string& clip) const;

protected:
	void LoadAssets() override;
	AnimationPose ProcessResultPose() override;

private:
	struct LocomotionClip
	{
		Animation* animation = nullptr;
		float duration = 0.0f;
		float weight = 0.0f;
	};

	// WeaponStanceData of one weapon (index = CharacterData::GetWeaponKey).
	struct Stance
	{
		LocomotionClip idle;
		float idleTime = -1.0f;                           // >= 0: the idle is this frame (a ready stance); < 0: loops
		LocomotionClip hold;                              // may share idle's Animation (same clip)
		float holdTime = -1.0f;                           // >= 0: the hold is this frame (a sweep's middle); < 0: loops
		PoseSpace holdSpace = PoseSpace::Default;
		bool holdOverMoves = false;
		std::vector<hashed_string> holdBones;
		std::vector<hashed_string> handBones;             // take the weapon's grip in local space
		std::vector<std::string> sources;                 // files whose clips pose this weapon (its models, stance source)
		const WeaponSweepData* sweepData = nullptr;       // aiming weapon
		LocomotionClip sweep;                             // its own Animation: sampled at the aim pitch
		AnimationPose sweepDifference;                    // the sweep at the aim pitch relative to the hold (its middle)
		float blend = 0.2f;
		float fade = 0.0f;                                // 0..1, linear
		float weight = 0.0f;                              // smoothstep(fade)
		bool layered = false;                             // over the other stances (latched while shown)
	};

	// How the overlay clip is layered, kept from its start until it has faded out.
	struct OverlayShape
	{
		BodyMask mask = BodyMask::UpperBody;              // UpperBody, Bones or Full
		std::vector<hashed_string> maskBones;             // BodyMask::Bones
		PoseSpace space = PoseSpace::Default;             // the mask layers' space (MoveData::layerSpace)
		float legWeight = 1.0f;                           // Full: share of the legs handed over while moving / flying
		bool airLegs = true;
		std::vector<hashed_string> additiveBones;         // empty: no additive layer
		PoseSpace additiveSpace = PoseSpace::Local;
	};

	std::shared_ptr<const CharacterData> data;
	const roj::FlatSkeleton* bodySkeleton = nullptr;
	std::vector<std::string> loadedSources;               // moveset animation files, by index
	std::unordered_set<const Animation*> incompleteClips; // clip models lacking body bones (completed before blending)
	std::unordered_map<const Animation*, int> clipWeapons;  // move clip layer -> weapon (stance key) whose model it comes from

	LocomotionClip idle, runForward, runBackward, runLeft, runRight, fall, glide;
	LocomotionClip runReverse;                            // procedural strafe: the forward run played backward
	bool proceduralStrafe = false;
	std::vector<Stance> stances;                          // per weapon (CharacterData::GetWeaponKey)
	bool stancesStarted = false;
	std::vector<Animation*> primaryLayer;                 // one Animation per source file
	std::vector<Animation*> overlayLayer;
	std::vector<std::unordered_map<hashed_string, float>> clipDurations;

	AnimatorInput current;
	Animation* activePrimary = nullptr;
	Animation* activeOverlay = nullptr;
	uint32_t primarySerial = 0;
	uint32_t overlaySerial = 0;
	float pendingBlendOut = 0.2f;
	float driveTransition = 0.0f;

	Animation* activeRecovery = nullptr;                  // one of primaryLayer (recovery and primary never overlap)
	uint32_t recoverySerial = 0;                          // input serial last seen (0 once the clip ended)
	uint32_t recoveryMoveSerial = 0;                      // serial of the attack it continues, kept while fading
	bool recoveryReleased = false;                        // the body moved: the clip hands over
	float recoveryReleaseElapsed = 0.0f;
	float recoveryFadeElapsed = 0.0f;
	float recoveryLowerBodyBlend = 0.2f;
	float recoveryUpperBodyBlend = 0.35f;
	float recoveryBlendOut = 0.2f;
	float recoveryLowerWeight = 0.0f;
	float recoveryUpperWeight = 0.0f;
	bool recoveryFading = false;

	float overlayWeight = 0.0f;
	float overlayFadeRate = 10.0f;
	float overlayBlendOut = 0.2f;
	// The overlay's shape, kept from its start until it has faded out: once the move ended the input describes no
	// overlay (its default mask would put the fading clip on the whole upper body).
	OverlayShape overlayShape;
	AnimationPose overlayReference;                       // additive layer: the clip at its reference time
	AnimationPose overlayDifference;                      // additive layer: this frame's difference
	// The overlay another overlay clip replaced while visible: its last pose, shape, difference and weight,
	// cross-faded into the new one over its blendIn.
	AnimationPose overlaySnapshot;
	OverlayShape overlaySnapshotShape;
	AnimationPose overlaySnapshotDifference;
	float overlaySnapshotWeight = 0.0f;
	float overlaySnapshotDuration = 0.0f;
	float overlaySnapshotElapsed = 0.0f;
	bool overlaySnapshotActive = false;
	float airWeight = 0.0f;
	float glideWeight = 0.0f;
	float aimWeight = 0.0f;
	float idleTime = 0.0f;
	float locomotionPhase = 0.0f;

	// Procedural strafe (radians, relative to the facing)
	float strafeMoveAngle = 0.0f;                         // smoothed move direction
	float strafeForwardTwist = 0.0f;                      // legs of the forward run
	float strafeBackwardTwist = 0.0f;                     // legs of the reversed run
	float strafeBackwardBlend = 0.0f;                     // 0 = forward run, 1 = reversed run (linear)
	bool strafeBackward = false;

	// Lean (radians)
	float leanForward = 0.0f;
	float leanSide = 0.0f;

	AnimationPose snapshot;
	float snapshotDuration = 0.0f;
	float snapshotElapsed = 0.0f;
	bool snapshotActive = false;
	AnimationPose lastOutput;                             // the composition before hit recoil (transition snapshots)
	bool hasLastOutput = false;

	// Composition storage, kept between evaluations (ComposePose).
	AnimationPose output;                                 // the composed pose
	AnimationPose completion;                             // a clip completed from the pose under it
	AnimationPose blendScratch;                           // a turned run, a blend computed apart then swapped in
	AnimationPose layerScratch;                           // the locomotion under a full-body move, a completed overlay
	AnimationPose fullScratch;                            // a full-body overlay with its legs handed over

	glm::vec3 rootMotion = glm::vec3(0.0f);

	void LoadLocomotionClip(LocomotionClip& clip, const std::string& source, const std::string& name, bool loop);
	void LoadMoveSetSource(const std::string& source);
	void LoadStances();
	void LoadStance(Stance& stance, const WeaponStanceData& stanceData, const WeaponData& weapon);
	void FindIncompleteClips();
	void MapClipWeapons();
	bool IsComplete(const Animation* animation) const { return incompleteClips.find(animation) == incompleteClips.end(); }
	void StartLayerClip(std::vector<Animation*>& layer, const AnimatorInput::Layer& input, Animation*& active, bool extractRootMotion);
	static void Sample(Animation* animation, float time);
	void BeginSnapshot(float duration);
	void BeginRecovery(const AnimatorInput::Recovery& recovery);
	void UpdateRecoveryWeights(float deltaTime);
	void DropRecovery();
	void UpdateLocomotionWeights(float deltaTime);
	void UpdateStrafe(float deltaTime, float run);
	void UpdateLean(float deltaTime);
	void UpdateStanceWeights(float deltaTime);
	void ApplyLegTwist(AnimationPose& pose, roj::BoneNode* rootNode, float angle) const;
	void ApplyLean(AnimationPose& pose, roj::BoneNode* rootNode) const;
	void UpdateSweeps();
	void ApplySweeps(AnimationPose& pose, roj::BoneNode* rootNode);
	void ApplyGrip(AnimationPose& pose, roj::BoneNode* rootNode);
	void ApplyHitRecoil(AnimationPose& pose, roj::BoneNode* rootNode) const;
};
