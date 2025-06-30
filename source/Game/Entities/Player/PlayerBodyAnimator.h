#pragma once
#include <Animator.h>

class PlayerBodyAnimator : public Animator
{
public:
	
	PlayerBodyAnimator(Entity* ownerEntity) : Animator(ownerEntity){}

	float movementSpeed = 0;

protected:

	Animation* idle = nullptr;
	Animation* run_f = nullptr;

	void LoadAssets()
	{
		idle = AddAnimation("GameData/animations/player/body/idle.glb");
		run_f = AddAnimation("GameData/animations/player/body/run_f.glb");
	}

	AnimationPose ProcessResultPose()
	{
		auto idlePose = idle->GetAnimationPose();
		auto runFPose = run_f->GetAnimationPose();

		return AnimationPose::Lerp(idlePose, runFPose, movementSpeed/7);
	}

};
