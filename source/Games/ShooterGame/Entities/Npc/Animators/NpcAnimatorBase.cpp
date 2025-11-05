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

		if (weapon_pendingAttack)
		{
			desiredAnimation = "pistol_fire";
			pistol->PlayAnimation("pistol_fire",false,0.1f);
			weapon_pendingAttack = false;
		}
	}

	if ((pistol->currentAnimationData->animationName == "pistol_fire" && pistol->IsAnimationPlaying()) == false)
	{

		if (pistol->currentAnimationData->animationName != desiredAnimation)
		{

			if (pistol->currentAnimationData->animationName == "idle")
			{
				pistol->PasteAnimationPose(lastPose);
			}

			pistol->PlayAnimation(desiredAnimation, false, 0.3);
		}

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

			spineRotation.x = glm::clamp(spineRotation.x, -70.0f, 70.0f);
			spineRotation.y = glm::clamp(spineRotation.y, -30.0f, 30.0f);

			pistolPos.boneTransforms["spine_02"] = pistolPos.boneTransforms["spine_02"] * MathHelper::GetRotationMatrix(vec3(spineRotation.y, spineRotation.y * 0.3f, -spineRotation.x * 0.75f));


			startBone = "spine_01";
		}

		return AnimationPose::LayeredLerp(startBone, pistol->GetRootNode(), locomotionPose, pistolPos, true, 1);// AnimationPose::Lerp(locomotion, painPose, PainProgress);
	}
	else
	{
		return locomotionPose;
	}


}