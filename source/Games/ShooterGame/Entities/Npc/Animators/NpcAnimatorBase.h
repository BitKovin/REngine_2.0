#pragma once

#include <Animator.h>

class NpcAnimatorBase : public Animator
{
public:

	NpcAnimatorBase(Entity* ownerEntity) : Animator(ownerEntity) {}

	float movementSpeed = 0;

	float PainProgress = 0;

	bool weapon_holds = false;
	bool weapon_ready = false;
	bool weapon_aims = false;

	void Update();

protected:

	Animation* idle = nullptr;
	Animation* run_f = nullptr;
	Animation* pistol = nullptr;

	Animation* inPain = nullptr;

	void LoadAssets();

	AnimationPose ProcessResultPose();

};
