#pragma once


#include <vector>
#include <string>

#include "ShaderManager.h"

#include "IDrawMesh.h"

#include "VertexData.h"

#include "MathHelper.hpp"

#include "skinned_model.hpp"

#include "glm.h"

#include "animator.hpp"

#include "StaticMesh.hpp"

#include "Time.hpp"

#include "json.hpp"
#include "Helpers/JsonHelper.hpp"

#include "Physics.h"

using namespace std;

struct AnimationPose 
{
	std::unordered_map<std::string, mat4> boneTransforms;

	static AnimationPose Lerp(AnimationPose a, AnimationPose b, float progress)
	{
		if (progress < 0.002)
			return a;

		if (progress > 0.995)
			return b;

		std::unordered_map<std::string, mat4> resultPose;

		for (auto bonePose : a.boneTransforms)
		{

			

			mat4 aMat = a.boneTransforms[bonePose.first];
			mat4 bMat = b.boneTransforms[bonePose.first];

			if (bonePose.first == "root")
			{
				resultPose[bonePose.first] = bMat;
				continue;
			}

			auto aTrans = MathHelper::DecomposeMatrix(aMat);
			auto bTrans = MathHelper::DecomposeMatrix(bMat);

			auto resultTrans = MathHelper::Transform::Lerp(aTrans, bTrans, progress);

			resultPose[bonePose.first] = resultTrans.ToMatrix();

		}

		AnimationPose result;

		result.boneTransforms = resultPose;

		return result;

	}

};

struct AnimationState
{
	bool playing;
	string animationName;
	float animationTime;
	bool looping;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(AnimationState, playing, animationName, animationTime, looping)

};

struct AnimationEvent
{
	string eventName;
	float time;
	string userData1;
	string userData2;
	string userData3;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(AnimationEvent, eventName, time, userData1, userData2, userData3)
};

struct AnimationData
{
	string animationName;
	vector<AnimationData> animationEvents;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(AnimationData, animationName, animationEvents)
};

struct HitboxData
{
	string boneName = "";
	vec3 size = vec3(0.2);
	vec3 rotation = vec3(0);
	vec3 position = vec3(0);

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(HitboxData, boneName, size, rotation, position)
};

struct SkeletalMeshMetaData
{
	vector<HitboxData> hitboxes;
	vector<AnimationData> animations;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(SkeletalMeshMetaData, hitboxes, animations)


};

class Entity;

class SkeletalMesh : public StaticMesh
{
private:

	roj::Animator animator;

	std::vector<mat4> boneTransforms;

	std::vector<mat4> finalizedBoneTransforms;

	double blendStartTime = 0;
	double blendEndTime = 0;

	bool firstAnimation = true;

	vec3 oldRootMotionPos = vec3();
	vec3 oldRootMotionRot = vec3();

	float oldAnimTime = 0;

	string filePath = "";

	float GetBlendInProgress()
	{
		double currentTime = Time::GameTime;

		if (blendEndTime <= blendStartTime)
			return 1.0f; // Avoid division by zero or invalid range

		if (currentTime <= blendStartTime)
			return 0.0f;

		if (currentTime >= blendEndTime)
			return 1.0f;

		double progress = (currentTime - blendStartTime) / (blendEndTime - blendStartTime);
		return static_cast<float>(progress);
	}
	AnimationPose blendStartPose;

	std::vector<Body*> hitboxBodies;

	std::recursive_mutex hitboxMutex;

protected:

	void ApplyAdditionalShaderParams(ShaderProgram* shader_program)
	{
		for (int i = 0; i < finalizedBoneTransforms.size(); ++i)
			shader_program->SetUniform("finalBonesMatrices[" + std::to_string(i) + "]", finalizedBoneTransforms[i]);
	}

public:

	SkeletalMeshMetaData metaData;

	bool UpdatePose = true;

	AnimationPose GetAnimationPose()
	{

		AnimationPose pose;
		pose.boneTransforms = animator.GetBonePoseArray();

		return pose;
	}

	void PasteAnimationPose(AnimationPose pose)
	{
		animator.ApplyBonePoseArray(pose.boneTransforms);
		boneTransforms = animator.getBoneMatrices();
	}

	void PlayAnimation(string name, bool Loop = false, float interpIn = 0.12);

	void FinalizeFrameData()
	{
		StaticMesh::FinalizeFrameData();
		finalizedBoneTransforms = boneTransforms;
	}

	MathHelper::Transform PullRootMotion();

	void PlayAnimation(float interpIn = 0.12)
	{

		if (firstAnimation || animator.m_currAnim == nullptr)
		{
			interpIn = 0;
			blendStartTime = -1;
			blendEndTime = 0;
			firstAnimation = false;
		}

		if (interpIn > 0.01)
		{
			blendStartPose = GetAnimationPose();
			blendStartTime = Time::GameTime;
			blendEndTime = Time::GameTime + interpIn;
		}

		animator.play();
	}

	void Update(float timeScale = 1);

	void SetLooped(bool looped)
	{
		animator.Loop = looped;
	}

	bool GetLooped()
	{
		return animator.Loop;
	}

	void LoadFromFile(string path)
	{

		StaticMesh::LoadFromFile(path);
		filePath = path;

		boneTransforms.resize(model->boneInfoMap.size());

		for (int i = 0; i < boneTransforms.size(); i++)
		{
			boneTransforms[i] = glm::identity<mat4>();
		}

		animator = roj::Animator(model);

		LoadMetaFromFile();
	}

	mat4 GetWorldMatrixNoOffsets();

	mat4 GetBoneMatrix(string boneName);

	mat4 GetBoneMatrixWorld(string boneName);

	void ClearHitboxes();

	void CreateHitbox(Entity* owner, HitboxData data);

	void CreateHitboxes(Entity* owner);

	void UpdateHitboxes();

	void SaveMetaToFile();
	void LoadMetaFromFile();

	void SetAnimationState(const AnimationState& animationState)
	{
		animator.Loop = animationState.looping;
		animator.set(animationState.animationName);
		animator.play();
		animator.m_currTime = animationState.animationTime;
		animator.UpdateAnimationPose();
		animator.m_playing = animationState.playing;
		Update(0);
	}



	AnimationState GetAnimationState()
	{
		AnimationState animationState;

		animationState.looping = animator.Loop;
		animationState.animationName = animator.currentAnimationName;
		animationState.animationTime = animator.m_currTime;
		animationState.playing = animator.m_playing;

		return animationState;
	}



};