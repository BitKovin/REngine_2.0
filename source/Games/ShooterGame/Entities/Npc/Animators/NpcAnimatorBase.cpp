#include "NpcAnimatorBase.h"

void NpcAnimatorBase::LoadAssets()
{
	idle = AddAnimation("GameData/models/npc/base.glb","idle",false);
	run_f = AddAnimation("GameData/models/npc/base.glb", "walk");
	pistol = AddAnimation("GameData/models/npc/base.glb", "idle", false);
	//inPain = AddAnimation("GameData/animations/npc/inPain.glb");
}

void NpcAnimatorBase::Update()
{

	if (loaded == false) return;

	std::string desiredAnimation = "idle";

	if (weapon_holds)
	{

		desiredAnimation = "pistol_hold";

		if (weapon_ready)
		{
			desiredAnimation = "pistol_idle";
		}

		if (weapon_aims)
		{
			desiredAnimation = "pistol_aim";
		}

	}

	if (pistol->currentAnimationData->animationName != desiredAnimation)
	{

		if (pistol->currentAnimationData->animationName == "idle")
		{
			pistol->PasteAnimationPose(lastPose);
		}

		pistol->PlayAnimation(desiredAnimation, false, 0.3);
	}

	Animator::Update();

}

AnimationPose NpcAnimatorBase::ProcessResultPose()
{
	auto idlePose = idle->GetAnimationPose();
	auto runFPose = run_f->GetAnimationPose();

	auto pistolPos = pistol->GetAnimationPose();

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

	if (weapon_holds)
	{

		hashed_string startBone = "clavicle_r";

		if (weapon_ready || weapon_aims)
		{
			startBone = "spine_01";
		}

		return AnimationPose::LayeredLerp(startBone, pistol->GetRootNode(), locomotion, pistolPos, true, 1);// AnimationPose::Lerp(locomotion, painPose, PainProgress);
	}
	else
	{
		return locomotion;
	}


}