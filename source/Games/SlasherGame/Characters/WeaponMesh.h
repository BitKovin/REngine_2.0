#pragma once

#include <SkeletalMesh.hpp>

#include <algorithm>
#include <string>

// Mesh of a weapon model. Never plays clips at runtime (its pose is pasted every frame); Rig weapons sample
// their rest poses from their own clips while loading.
class WeaponMesh : public SkeletalMesh
{
public:
	explicit WeaponMesh(Entity* owner) : SkeletalMesh(owner) {}

	// Bone-local pose of `clip` at `time` seconds (clamped to the clip). False if the model has no such clip.
	bool SampleClipPose(const std::string& clip, float time, AnimationPose& out)
	{
		if (model == nullptr || model->animations.find(hashed_string(clip)) == model->animations.end())
			return false;

		PlayAnimation(clip, false, 0.0f);
		SetAnimationTime(std::clamp(time, 0.0f, GetAnimationDuration()));
		Update(0.0f);
		dirtyPose = true;   // the pose snapshot only refreshes by itself when the clip time changed
		out = GetAnimationPose();

		StopAnimation();    // nothing may re-evaluate the clip over the pasted poses
		return true;
	}
};
