#include "NpcAnimatorBase.h"

void NpcAnimatorBase::LoadAssets()
{
	idle = AddAnimation("GameData/animations/npc/idle.glb");
	run_f = AddAnimation("GameData/animations/npc/walk.glb");
}

AnimationPose NpcAnimatorBase::ProcessResultPose()
{
	auto idlePose = idle->GetAnimationPose();
	auto runFPose = run_f->GetAnimationPose();

	return AnimationPose::Lerp(idlePose, runFPose, movementSpeed / 7);
}