#pragma once

#include <Animator.h>

class NpcAnimatorBase : public Animator
{
public:

	NpcAnimatorBase(Entity* ownerEntity) : Animator(ownerEntity) {}

	float movementSpeed = 0;

protected:

	Animation* idle = nullptr;
	Animation* run_f = nullptr;

	void LoadAssets();

	AnimationPose ProcessResultPose();

};
