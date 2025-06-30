#pragma once
#include "Animation.h"

class Animator
{
public:

	float Speed = 1;
	bool UpdatePose = true; 

	Animator(Entity* ownerEntity);
	virtual ~Animator();

	void Update();
	AnimationPose GetResultPose();

	void LoadAssetsIfNeeded();

protected:

	vector<Animation*> animationsToUpdate;

	vector<AnimationEvent> pendingAnimationEvents;

	Entity* owner = nullptr;

	bool loaded = false;

	Animation* AddAnimation(string path, string name = "", bool loop = true);

	virtual AnimationPose ProcessResultPose();

	virtual void LoadAssets(){}

};

