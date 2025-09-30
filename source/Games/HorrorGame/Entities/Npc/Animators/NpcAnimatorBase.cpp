#include "NpcAnimatorBase.h"

void NpcAnimatorBase::LoadAssets()
{
	idle = AddAnimation("GameData/animations/npc/idle.glb");
	run_f = AddAnimation("GameData/animations/npc/walk.glb");
	inPain = AddAnimation("GameData/animations/npc/inPain.glb");
}

AnimationPose NpcAnimatorBase::ProcessResultPose()
{
	auto idlePose = idle->GetAnimationPose();
	auto runFPose = run_f->GetAnimationPose();

	auto painPose = inPain->GetAnimationPose();

	auto locomotion = AnimationPose::Lerp(idlePose, runFPose, movementSpeed / 7);

	return AnimationPose::Lerp(locomotion, painPose, PainProgress);
}