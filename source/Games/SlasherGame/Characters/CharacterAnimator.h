#pragma once

#include <Data/CharacterData.h>
#include <Data/MoveSetData.h>

#include <Animator.h>
#include <utility/hashed_string.hpp>

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
		float meshSpace = -1.0f;         // MoveData::LayerMeshSpace(): the layers' mesh-space share (< 0: by bone)
		bool movement = false;           // a Movement move (its clip carries no weapon: the firearms' hands hold them over it)
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
		float meshSpace = -1.0f;         // MoveData::LayerMeshSpace() of the upper-body hand-over
		bool movement = false;
	};

	glm::vec2 localVelocity = glm::vec2(0.0f);   // x = right, y = forward (m/s)
	bool grounded = true;
	bool gliding = false;
	bool aiming = false;                         // the stances' aim (sweep and yaw) applies (fades in / out)
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
// from its UpdatePose: Evaluate() advances the channels and samples every clip that shows, ComposePose() builds the
// pose in stages, each a method of its own, bottom to top:
//
//   ComposeLocomotion  idle (or the stance idles of the weapons in the hands) + run, phase-synced, playback rate from
//                      speed; procedural strafe (legs turned, torso turned back); lean on the ground; fall / glide in
//                      the air.
//   LayerHolds         the hold clip of each weapon in the hands on its holdBones subtrees - how it is carried while
//                      the body moves on its own. Layered stances (a weapon beside the one in the hands) on top.
//   ComposeMove        the primary move: a Full / NoLegs clip replaces everything above (its legs hand over to the
//                      locomotion / air pose by legWeight - NoLegs always, Full in the air), the recovery of an attack
//                      plays on and hands over, an UpperBody / Bones clip goes on its subtrees. An aiming weapon's
//                      hold goes back over such a clip with the aim (LayerAimHolds). A Movement move's clip carries no
//                      weapon: the firearms' hands (moveHoldBones) take their hold again over it.
//   ComposeOverlay     firing, drawing, reloading: an additive layer, then its mask (UpperBody / Bones; Full replaces
//                      the whole body, root included, and hands its legs over as the body runs or flies). It keeps its
//                      shape through its fade-out; one replacing another cross-fades the two results.
//   ApplyGrip          the hands holding a weapon take the grip of the clip posing it (local space).
//   ApplyAim           aiming weapons: the sweep clip at the aim pitch (its difference from the hold), then the yaw
//                      between facing and aim on the sweep's yaw bones - the weapon points at the aim at once.
//   transition         a primary change blends from the last output, frozen when it happened.
//   ApplyHitRecoil     on the final pose, outside transitions.
//
// Root bone: it belongs to whatever owns the whole body - locomotion, a full-body move, a Full overlay (the reload's
// spin). Partial layers and leg hand-overs never change it (the engine's partial blends take it from their layer; the
// helpers below put it back). Its translation is stripped at the end: root motion moves the capsule.
// Clips of models that lack some of the body's bones (a weapon model without the coat) leave those bones to the pose
// underneath.
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

	// Advances the channels and samples every contributing clip (deltaTime: local seconds since the previous call).
	// Then call ComposePose() and ConsumeRootMotion().
	void Evaluate(const AnimatorInput& input, float deltaTime);

	// Composes the evaluated clips into the animator's own pose and returns it (GetPose() until the next call). Every
	// blend works in place on storage kept between evaluations, so a steady composition does not allocate.
	// GetResultPose() returns a copy of the same pose.
	const AnimationPose& ComposePose();
	const AnimationPose& GetPose() const { return output; }

	// Model-space root translation of the primary clip since the previous Evaluate (horizontal).
	glm::vec3 ConsumeRootMotion();

	// After ComposePose(): what poses Rig weapon bones this frame.
	WeaponDrive GetWeaponDrive();

	// Clip length in seconds, 0 if the clip does not exist. Valid after LoadAssets.
	float GetClipDuration(int animation, const hashed_string& clip) const;

protected:
	void LoadAssets() override;
	AnimationPose ProcessResultPose() override;

private:
	struct Clip
	{
		Animation* animation = nullptr;
		float duration = 0.0f;
		float weight = 0.0f;
	};

	// WeaponStanceData of one weapon (index = CharacterData::GetWeaponKey).
	struct Stance
	{
		Clip idle;
		float idleTime = -1.0f;                           // >= 0: the idle is this frame (a ready stance); < 0: loops
		Clip hold;                                        // may share idle's Animation (same clip)
		float holdTime = -1.0f;                           // >= 0: the hold is this frame (a sweep's middle); < 0: loops
		PoseSpace holdSpace = PoseSpace::Default;
		std::vector<hashed_string> holdBones;
		std::vector<hashed_string> moveHoldBones;         // over Movement moves: the hands holding the weapon
		std::vector<hashed_string> handBones;             // take the weapon's grip in local space
		std::vector<std::string> sources;                 // files whose clips pose this weapon (its models, stance source)
		const WeaponSweepData* sweepData = nullptr;       // aiming weapon
		Clip sweep;                                       // its own Animation: sampled at the aim pitch
		AnimationPose sweepDifference;                    // the sweep at the aim pitch relative to the hold (its middle)
		float blend = 0.2f;
		float fade = 0.0f;                                // 0..1, linear
		float weight = 0.0f;                              // smoothstep(fade)
		bool layered = false;                             // over the other stances (latched while shown)
	};

	// How a layer clip lies over the pose beneath it. The overlay keeps it from its start until it has faded out (once
	// the move ended the input describes no overlay).
	struct LayerShape
	{
		BodyMask mask = BodyMask::UpperBody;              // UpperBody, Bones or Full
		std::vector<hashed_string> maskBones;             // BodyMask::Bones
		float meshSpace = -1.0f;                          // the mask layers' mesh-space share (MoveData::LayerMeshSpace)
		float legWeight = 1.0f;                           // Full: share of the legs handed over while moving / flying
		bool airLegs = true;
		std::vector<hashed_string> additiveBones;         // empty: no additive layer
		PoseSpace additiveSpace = PoseSpace::Local;
	};

	// A pose frozen when what it showed was replaced, blended into the new composition over `duration`.
	struct PoseFade
	{
		AnimationPose pose;
		float duration = 0.0f;
		float elapsed = 0.0f;
		bool active = false;

		void Advance(float deltaTime);
		float Progress() const;                           // 0 = all frozen pose, 1 = all new composition
	};

	struct PrimaryChannel
	{
		Animation* clip = nullptr;
		uint32_t serial = 0;
		float blendOut = 0.2f;                            // of the move playing: its end blends over this
	};

	// The clip of an attack that ended in recovery, on the primary layer's Animation (recovery and primary never overlap).
	struct RecoveryChannel
	{
		Animation* clip = nullptr;
		uint32_t inputSerial = 0;                         // input serial last seen (0 once the clip ended)
		uint32_t moveSerial = 0;                          // serial of the attack it continues, kept while fading
		bool released = false;                            // the body moved: the clip hands over
		bool fading = false;                              // the clip ended: whatever still plays hands over
		float releaseElapsed = 0.0f;
		float fadeElapsed = 0.0f;
		float lowerBodyBlend = 0.2f;
		float upperBodyBlend = 0.35f;
		float blendOut = 0.2f;
		float lowerWeight = 0.0f;
		float upperWeight = 0.0f;
	};

	struct OverlayChannel
	{
		Animation* clip = nullptr;
		uint32_t serial = 0;
		LayerShape shape;
		AnimationPose reference;                          // additive layer: the clip at its reference time
		AnimationPose difference;                         // additive layer: this frame's difference
		float weight = 0.0f;
		float fadeRate = 10.0f;
		float blendOut = 0.2f;
	};

	// The overlay another overlay clip replaced while visible: its last pose (the fade's), shape, difference and
	// weight, cross-faded into the new one over its blendIn.
	struct OverlayHandOver
	{
		PoseFade fade;
		LayerShape shape;
		AnimationPose difference;
		float weight = 0.0f;
	};

	std::shared_ptr<const CharacterData> data;
	const roj::FlatSkeleton* bodySkeleton = nullptr;
	std::vector<std::string> loadedSources;               // moveset animation files, by index
	std::unordered_set<const Animation*> incompleteClips; // clip models lacking body bones (completed before blending)
	std::unordered_map<const Animation*, int> clipWeapons;  // move clip layer -> weapon (stance key) whose model it comes from

	// ---- Clips
	Clip idle, runForward, runBackward, runLeft, runRight, fall, glide;
	Clip runReverse;                                      // procedural strafe: the forward run played backward
	bool proceduralStrafe = false;
	std::vector<Stance> stances;                          // per weapon (CharacterData::GetWeaponKey)
	bool stancesStarted = false;
	std::vector<Animation*> primaryLayer;                 // one Animation per source file
	std::vector<Animation*> overlayLayer;
	std::vector<std::unordered_map<hashed_string, float>> clipDurations;

	// ---- Channels
	AnimatorInput current;
	PrimaryChannel primary;
	RecoveryChannel recovery;
	OverlayChannel overlay;
	OverlayHandOver overlayHandOver;
	PoseFade transition;                                  // a primary change: from the last output
	float driveTransition = 0.0f;

	// ---- Locomotion state
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

	// ---- Composition storage, kept between evaluations (ComposePose)
	AnimationPose output;                                 // the composed pose
	AnimationPose lastOutput;                             // the composition before hit recoil (transition snapshots)
	bool hasLastOutput = false;
	AnimationPose completion;                             // a clip completed from the pose under it
	AnimationPose blendScratch;                           // a turned run, a blend computed apart then swapped in
	AnimationPose layerScratch;                           // the locomotion under a full-body move, an outgoing overlay
	AnimationPose fullScratch;                            // a full-body overlay with its legs handed over

	glm::vec3 rootMotion = glm::vec3(0.0f);

	// ---- Loading
	void LoadClip(Clip& clip, const std::string& source, const std::string& name, bool loop);
	void LoadMoveSetSource(const std::string& source);
	void LoadStances();
	void LoadStance(Stance& stance, const WeaponStanceData& stanceData, const WeaponData& weapon);
	void FindIncompleteClips();
	void MapClipWeapons();
	bool IsComplete(const Animation* animation) const { return incompleteClips.find(animation) == incompleteClips.end(); }

	// ---- Evaluation
	static void Sample(Animation* animation, float time);
	void StartLayerClip(std::vector<Animation*>& layer, const AnimatorInput::Layer& input, Animation*& active, bool extractRootMotion);
	void BeginTransition(float duration);
	void UpdateRecovery(const AnimatorInput::Recovery& input);
	void BeginRecovery(const AnimatorInput::Recovery& input);
	void UpdateRecoveryWeights(float deltaTime);
	void DropRecovery();
	void UpdatePrimary(const AnimatorInput::Layer& input);
	void UpdateOverlay(const AnimatorInput::Layer& input, float deltaTime);
	void UpdateLocomotionWeights(float deltaTime);
	void UpdateStrafe(float deltaTime, float run);
	void UpdateLean(float deltaTime);
	void UpdateStanceWeights(float deltaTime);
	void UpdateSweeps();

	// ---- Composition stages (ComposePose, in this order)
	void ComposeLocomotion(AnimationPose& pose, roj::BoneNode* rootNode);
	void LayerHolds(AnimationPose& pose, roj::BoneNode* rootNode, bool overMove);
	void ComposeMove(AnimationPose& pose, roj::BoneNode* rootNode);
	void LayerAimHolds(AnimationPose& pose, roj::BoneNode* rootNode);
	void ComposeOverlay(AnimationPose& pose, roj::BoneNode* rootNode);
	void ApplyGrip(AnimationPose& pose, roj::BoneNode* rootNode);
	void ApplyAim(AnimationPose& pose, roj::BoneNode* rootNode);
	void ApplyHitRecoil(AnimationPose& pose, roj::BoneNode* rootNode) const;

	// ---- Composition building blocks
	bool IsFullBodyPrimary(const roj::BoneNode* rootNode) const;
	bool IsMovementClipShown() const;
	const AnimationPose& Blendable(Animation* animation, const AnimationPose& base);
	void LerpFrom(AnimationPose& pose, const AnimationPose& from, float progress);
	void LayerBranch(AnimationPose& target, roj::BoneNode* rootNode, const hashed_string& bone, const AnimationPose& layer,
		bool layerComplete, float weight, float meshSpace);
	void LayerMask(AnimationPose& target, roj::BoneNode* rootNode, const std::vector<hashed_string>* maskBones,
		const AnimationPose& layer, bool layerComplete, float weight, float meshSpace);
	void HandLegsTo(AnimationPose& pose, roj::BoneNode* rootNode, const AnimationPose& legs, float weight) const;
	void LayerOverlay(AnimationPose& target, roj::BoneNode* rootNode, const LayerShape& shape, const AnimationPose& clip,
		bool complete, const AnimationPose& difference, float weight);
	void ApplyLegTwist(AnimationPose& pose, roj::BoneNode* rootNode, float angle) const;
	void ApplyLean(AnimationPose& pose, roj::BoneNode* rootNode) const;
	static void ShapeFromInput(LayerShape& shape, const AnimatorInput::Layer& input);
};
