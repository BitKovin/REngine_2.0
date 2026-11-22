#include "NpcMeleeAnimator.h"

void NpcMeleeAnimator::LoadAssets()
{
	// Inherits: locomotion, pistol, taskAnimation, actionAnimation.
	// Adds blockAnimation on top.
	NpcAnimatorBase::LoadAssets();
	blockAnimation = AddAnimation("GameData/models/npc/base.glb", "sword_block", false);
}

void NpcMeleeAnimator::Update()
{
	if (!loaded) return;

	// ── Locomotion ────────────────────────────────────────────────────────────
	const std::string desiredLocoAnim = PickLocomotionAnimation(weapon_ready);

	if (locomotion->currentAnimationData->animationName != desiredLocoAnim)
		locomotion->PlayAnimation(desiredLocoAnim, true, 0.3f);

	// ── Block layer ───────────────────────────────────────────────────────────
	const std::string desiredBlockAnim = blocking ? "sword_block" : "sword_idle";
	if (blockAnimation->currentAnimationData->animationName != desiredBlockAnim)
	{
		blockAnimation->PasteAnimationPose(lastPose);
		blockAnimation->PlayAnimation(desiredBlockAnim, true, 0.3f);
	}

	Animator::Update();
}

AnimationPose NpcMeleeAnimator::BuildCombatPose()
{
	AnimationPose locomotionPose = locomotion->GetAnimationPose();
	if (!weapon_holds) return locomotionPose;

	// Blend block/sword pose from spine_01 upward
	return AnimationPose::LayeredLerp("spine_01", locomotion->GetRootNode(),
	                                  locomotionPose, blockAnimation->GetAnimationPose(), 1, 1);
}
