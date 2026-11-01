#include "NpcAnimatorBase.h"

#include <cmath>
#include <limits>

// ── Asset loading ───────────────────────────────────────────────────────────

void NpcAnimatorBase::LoadAssets()
{
	locomotion      = AddAnimation("GameData/models/npc/base.glb", "idle");
	pistol          = AddAnimation("GameData/models/npc/base.glb", "idle", false);
	taskAnimation   = AddAnimation("GameData/models/npc/base.glb", "idle", false);
	actionAnimation = AddAnimation("GameData/models/npc/base.glb", "idle", false);

	taskAnimation->StopAnimation();
	actionAnimation->StopAnimation();
}

// ── Per-frame update ────────────────────────────────────────────────────────

void NpcAnimatorBase::Update()
{
	if (!loaded) return;

	// ── Ranged (pistol) layer state ─────────────────────────────────────────
	std::string desiredPistolAnim = scared ? "scared" : "idle";

	if (weapon_holds)
	{
		desiredPistolAnim = "pistol_hold";
		if (weapon_ready) desiredPistolAnim = "pistol_idle";
		if (weapon_aims)  desiredPistolAnim = "pistol_aim";

		if (weapon_pendingAttack)
		{
			pistol->PlayAnimation("pistol_fire", false, 0.1f);
			weapon_pendingAttack = false;
			desiredPistolAnim    = "pistol_fire";
		}
	}

	const bool pistolFiring = pistol->currentAnimationData->animationName == "pistol_fire"
	                       && pistol->IsAnimationPlaying();

	if (!pistolFiring && pistol->currentAnimationData->animationName != desiredPistolAnim)
	{
		if (pistol->currentAnimationData->animationName == "idle")
			pistol->PasteAnimationPose(lastPose);

		pistol->PlayAnimation(desiredPistolAnim, false, 0.3f);
	}

	// ── Locomotion layer state ──────────────────────────────────────────────
	const std::string desiredLocoAnim = PickLocomotionAnimation(false);

	if (locomotion->currentAnimationData->animationName != desiredLocoAnim)
		locomotion->PlayAnimation(desiredLocoAnim, true, 0.2f);

	Animator::Update();
}

// ── Locomotion clip selection ───────────────────────────────────────────────

// The sector the body is moving in relative to its facing. The current sector
// is kept until the movement angle leaves it by SectorStickiness, so a
// strafe that wobbles around 45 degrees doesn't flicker between clips.
void NpcAnimatorBase::UpdateMoveSector()
{
	constexpr float SectorStickiness = 10.0f; // degrees
	constexpr float MinDirectionSpeed = 0.5f; // below this the sector is kept

	if (std::abs(moveForward) + std::abs(moveRight) < MinDirectionSpeed) return;

	const float angle = glm::degrees(std::atan2(moveRight, moveForward)); // 0 forward, +90 right

	constexpr float centers[4] = { 0.0f, 90.0f, 180.0f, -90.0f }; // Forward, Right, Back, Left

	MoveSector best = moveSector;
	float bestDistance = std::numeric_limits<float>::max();

	for (int i = 0; i < 4; ++i)
	{
		float distance = std::abs(angle - centers[i]);
		if (distance > 180.0f) distance = 360.0f - distance;

		if (static_cast<MoveSector>(i) == moveSector) distance -= SectorStickiness;

		if (distance < bestDistance)
		{
			bestDistance = distance;
			best = static_cast<MoveSector>(i);
		}
	}

	moveSector = best;
}

std::string NpcAnimatorBase::PickLocomotionAnimation(bool swordVariants)
{
	UpdateMoveSector();

	const bool moving = movementSpeed > 1;
	const bool running = movementSpeed > 3;

	if (crouched)
	{
		if (!moving) return "crouch_idle";

		switch (moveSector)
		{
		case MoveSector::Right: return "crouch_r";
		case MoveSector::Left:  return "crouch_l";
		case MoveSector::Back:  return "crouch_b";
		default:                return "crouch_f";
		}
	}

	if (!moving)
	{
		if (scared) return "scared";
		return swordVariants ? "sword_idle" : "idle";
	}

	if (swordVariants && !running) return "sword_walk";

	const char* gait = running ? "run" : "walk";

	switch (moveSector)
	{
	case MoveSector::Right: return std::string(gait) + "_r";
	case MoveSector::Left:  return std::string(gait) + "_l";
	case MoveSector::Back:  return std::string(gait) + "_b";
	default:                return gait;
	}
}

// ── Task animation ──────────────────────────────────────────────────────────

void NpcAnimatorBase::PlayTaskAnimation(std::string animationName, bool loop)
{
	if (!taskAnimation->IsAnimationPlaying())
		taskAnimation->PasteAnimationPose(lastPose);

	taskAnimation->PlayAnimation(animationName, loop, 0.3f);
}

void NpcAnimatorBase::StopTaskAnimation()
{
	if (taskAnimation->currentAnimationData)
	{
		auto pose = taskAnimation->GetAnimationPose();
		locomotion->PasteAnimationPose(pose);
		pistol->PasteAnimationPose(pose);
		locomotion->PlayAnimation(locomotion->currentAnimationData->animationName, true, 0.3f);
		pistol->PlayAnimation(pistol->currentAnimationData->animationName,         true, 0.3f);
	}

	taskAnimation->StopAnimation();
}

// ── Action animation ────────────────────────────────────────────────────────

bool NpcAnimatorBase::IsPlayingActionAnimation() const
{
	return actionAnimation && actionAnimation->IsAnimationPlaying();
}

void NpcAnimatorBase::PlayActionAnimation(const std::string& animationName, bool loop,
                                          float blendIn, float blendOut)
{
	if (!actionAnimation->IsAnimationPlaying())
		actionAnimation->PasteAnimationPose(lastPose);

	actionAnimation->PlayAnimation(animationName, loop, blendIn);
	actionBlendOut = blendOut;
}

void NpcAnimatorBase::StopActionAnimation()
{
	if (actionAnimation->currentAnimationData)
	{
		auto pose = actionAnimation->GetAnimationPose();
		locomotion->PasteAnimationPose(pose);
		pistol->PasteAnimationPose(pose);
		locomotion->PlayAnimation(locomotion->currentAnimationData->animationName, true, 0.3f);
		pistol->PlayAnimation(pistol->currentAnimationData->animationName,         true, 0.3f);
	}

	actionAnimation->StopAnimation();
}

float NpcAnimatorBase::GetActionAnimationRemainingTime() const
{
	if (!IsPlayingActionAnimation()) return 0.0f;
	return actionAnimation->GetAnimationDuration() - actionAnimation->GetAnimationTime();
}

// ── Pose composition ────────────────────────────────────────────────────────

// Default combat pose: ranged upper-body layer blended over locomotion.
AnimationPose NpcAnimatorBase::BuildCombatPose()
{
	AnimationPose locomotionPose = locomotion->GetAnimationPose();
	if (!weapon_holds) return locomotionPose;

	auto pistolPose = pistol->GetAnimationPose();
	hashed_string startBone = "clavicle_r";

	if (weapon_ready || weapon_aims)
	{
		spineRotation.x = glm::clamp(spineRotation.x, -70.0f, 70.0f);
		spineRotation.y = glm::clamp(spineRotation.y, -30.0f, 30.0f);

		if (weapon_aims)
		{
			pistolPose.boneTransforms["spine_02"] = pistolPose.boneTransforms["spine_02"]
				* MathHelper::GetRotationMatrix(
				    vec3(spineRotation.y, spineRotation.y * 0.3f, -spineRotation.x * 0.75f));
		}

		startBone = "spine_01";
	}

	return AnimationPose::LayeredLerp(startBone, pistol->GetRootNode(),
	                                  locomotionPose, pistolPose, true, 1);
}

// Sealed.  Task and action overlays are applied here for all NPC types.
// Subclasses shape the result only through BuildCombatPose().
AnimationPose NpcAnimatorBase::ProcessResultPose()
{
	AnimationPose result = BuildCombatPose();

	// Task animation – full-body override (e.g. sitting, interacting)
	if (taskAnimation->IsAnimationPlaying())
		result = AnimationPose::Lerp(result, taskAnimation->GetAnimationPose(), 1.0f);

	// Action animation – highest priority, fades out as the clip ends
	if (actionAnimation && actionAnimation->IsAnimationPlaying())
	{
		const float blend = std::clamp(GetActionAnimationRemainingTime() / actionBlendOut,
		                               0.0f, 1.0f);
		result = AnimationPose::Lerp(actionAnimation->GetAnimationPose(), result, 1.0f - blend);
	}

	return result;
}
