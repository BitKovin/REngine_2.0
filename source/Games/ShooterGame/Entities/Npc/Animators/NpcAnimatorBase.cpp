#include "NpcAnimatorBase.h"

void NpcAnimatorBase::LoadAssets()
{
	idle = AddAnimation("GameData/models/npc/base.glb","idle",true);
	run_f = AddAnimation("GameData/models/npc/base.glb", "walk");
	//inPain = AddAnimation("GameData/animations/npc/inPain.glb");
}

AnimationPose NpcAnimatorBase::ProcessResultPose()
{
	auto idlePose = idle->GetAnimationPose();
	auto runFPose = run_f->GetAnimationPose();

	//auto painPose = inPain->GetAnimationPose();

	AnimationPose locomotion;
	
	if (movementSpeed > 0.2)
	{
		locomotion = AnimationPose::Lerp(idlePose, runFPose, movementSpeed / 2.0f);
	}
	else
	{
		locomotion = idlePose;
	}

	return locomotion;// AnimationPose::Lerp(locomotion, painPose, PainProgress);
}