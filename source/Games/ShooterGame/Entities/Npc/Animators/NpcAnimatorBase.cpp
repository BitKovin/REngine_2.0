#include "NpcAnimatorBase.h"

void NpcAnimatorBase::LoadAssets()
{
	locomotion = AddAnimation("GameData/models/npc/base.glb","idle");
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

		pistol->PlayAnimation(desiredAnimation, true, 0.3);
	}


	std::string desiredLocomotionAnimation = "idle";

	if (movementSpeed > 1)
	{
		desiredLocomotionAnimation = "walk";
	}
	if (movementSpeed > 3)
	{
		desiredLocomotionAnimation = "run";
	}

	if (locomotion->currentAnimationData->animationName != desiredLocomotionAnimation)
	{
		locomotion->PlayAnimation(desiredLocomotionAnimation, true, 0.2);
	}

	Animator::Update();

}

AnimationPose NpcAnimatorBase::ProcessResultPose()
{

	auto pistolPos = pistol->GetAnimationPose();

	//auto painPose = inPain->GetAnimationPose();

	AnimationPose locomotionPose = locomotion->GetAnimationPose();
	
	

	if (weapon_holds)
	{

		hashed_string startBone = "clavicle_r";

		if (weapon_ready || weapon_aims)
		{
			startBone = "spine_01";
		}

		return AnimationPose::LayeredLerp(startBone, pistol->GetRootNode(), locomotionPose, pistolPos, true, 1);// AnimationPose::Lerp(locomotion, painPose, PainProgress);
	}
	else
	{
		return locomotionPose;
	}


}