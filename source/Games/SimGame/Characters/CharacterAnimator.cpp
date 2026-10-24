// Time.hpp first: engine headers below use Time without including it.
#include <Time.hpp>

#include "CharacterAnimator.h"

#include <cmath>

CharacterAnimator::Choice CharacterAnimator::Choose(const CharacterModel& model, glm::vec2 relativeMovement, bool crouching, bool running)
{
	const bool forward = relativeMovement.y >= 0.0f;
	const bool right = relativeMovement.x >= 0.0f;
	if (!model.directional)
		return { running ? "run" : "walk", running ? "run" : "walk", "idle" };
	if (crouching)
		return { forward ? "crouch_f" : "crouch_b", right ? "crouch_r" : "crouch_l", "crouch_idle" };
	if (running)
		return { forward ? "run" : "run_b", right ? "run_r" : "run_l", "idle" };
	return { forward ? "walk" : "walk_b", right ? "walk_r" : "walk_l", "idle" };
}

void CharacterAnimator::LoadAssets()
{
	// Three clip slots, each switching clips with a crossfade as the input changes; only these are evaluated per frame.
	forwardAnim = AddAnimation(model.clips, "walk");
	sideAnim = AddAnimation(model.clips, "walk_r");
	idleAnim = AddAnimation(model.clips, "idle");
}

void CharacterAnimator::Update()
{
	if (!loaded)
		return;
	constexpr float BlendTime = 0.2f;
	const float speed = glm::length(relativeMovement);
	running = running ? speed > RunBelow : speed > RunAbove;

	const Choice choice = Choose(model, relativeMovement, crouching, running);
	if (forwardAnim->GetAnimationName() != choice.forward)
		forwardAnim->PlayAnimation(choice.forward, true, BlendTime);
	if (sideAnim->GetAnimationName() != choice.side)
		sideAnim->PlayAnimation(choice.side, true, BlendTime);
	if (idleAnim->GetAnimationName() != choice.idle)
		idleAnim->PlayAnimation(choice.idle, true, BlendTime);

	// One gait cycle for both moving clips; standing still resets it.
	const float duration = forwardAnim->GetAnimationDuration();
	if (speed < 0.1f)
		phase = 0.0f;
	else if (duration > 0.0f)
	{
		phase = std::fmod(phase + static_cast<float>(Time::DeltaTime) / duration, 1.0f);
		forwardAnim->SetAnimationTime(phase * duration);
		sideAnim->SetAnimationTime(phase * sideAnim->GetAnimationDuration());
	}
	Animator::Update();
}

AnimationPose CharacterAnimator::ProcessResultPose()
{
	const float speed = glm::length(relativeMovement);
	const float moving = glm::clamp(speed / FullMoveSpeed, 0.0f, 1.0f);
	const float axes = std::fabs(relativeMovement.x) + std::fabs(relativeMovement.y);
	const float forwardWeight = axes > 1e-4f ? moving * std::fabs(relativeMovement.y) / axes : 0.0f;
	const float sideWeight = model.directional && axes > 1e-4f ? moving * std::fabs(relativeMovement.x) / axes : 0.0f;
	// A model without directional clips has only the forward axis: all of the movement goes to it.
	const float forward = model.directional ? forwardWeight : moving;

	AnimationPose pose = idleAnim->GetAnimationPose();
	float accumulated = 1.0f - forward - sideWeight;
	auto blendIn = [&](Animation* animation, float weight)
	{
		if (weight <= 0.0f)
			return;
		const float total = accumulated + weight;
		pose = AnimationPose::Lerp(pose, animation->GetAnimationPoseRef(), weight / total);
		accumulated = total;
	};
	blendIn(forwardAnim, forward);
	blendIn(sideAnim, sideWeight);
	return pose;
}
