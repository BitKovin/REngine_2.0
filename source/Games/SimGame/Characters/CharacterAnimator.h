#pragma once

// The locomotion of every CharacterBody: idle blended with a forward-axis clip and a side-axis clip, chosen from the state alone (movement
// relative to the body, stance). Written once for citizens, guards and the player (it replaces SimNpcAnimator, and is the shape of
// ShooterGame's PlayerBodyAnimator without its weapon layers and slide).
//
// A model with the directional clips (CharacterModel::directional) strafes, backs up and crouches with animations. One without them plays its
// forward clip whatever the direction and has no crouch.

#include "CharacterModels.h"

#include <Animator.h>

class CharacterAnimator : public Animator
{
public:
	CharacterAnimator(Entity* owner, const CharacterModel& model) : Animator(owner), model(model) {}

	glm::vec2 relativeMovement{ 0.0f }; // m/s, x right, y forward, relative to where the body faces
	bool crouching = false;

	void Update() override;

	// The clip names the animator would play for this input (the tests walk the table without loading anything).
	struct Choice
	{
		const char* forward;
		const char* side;
		const char* idle;
	};
	static Choice Choose(const CharacterModel& model, glm::vec2 relativeMovement, bool crouching, bool running);

	// Speed (m/s) above which a mover is running. Hysteresis: it keeps running until it is slower than RunBelow.
	static constexpr float RunAbove = 3.2f;
	static constexpr float RunBelow = 2.8f;
	// Movement (m/s) at which the locomotion clips have fully replaced the idle pose.
	static constexpr float FullMoveSpeed = 1.0f;

protected:
	void LoadAssets() override;
	AnimationPose ProcessResultPose() override;

private:
	const CharacterModel& model;
	Animation* forwardAnim = nullptr;
	Animation* sideAnim = nullptr;
	Animation* idleAnim = nullptr;
	bool running = false;
	float phase = 0.0f; // shared position in the gait cycle of the two moving clips, so they do not drift apart
};
