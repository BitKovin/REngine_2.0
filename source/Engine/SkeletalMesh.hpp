#pragma once


#include <vector>
#include <string>

#include "ShaderManager.h"

#include "IDrawMesh.h"

#include "VertexData.h"

#include "MathHelper.hpp"

#include "skinned_model.hpp"

#include "glm.h"

#include "model_animator.hpp"

#include "StaticMesh.h"

#include "Time.hpp"

#include "json.hpp"
#include "Helpers/JsonHelper.hpp"

#include "Physics.h"



using namespace std;

struct AnimationPose 
{
	std::unordered_map<std::string, mat4> boneTransforms;

	static AnimationPose Lerp(AnimationPose a, AnimationPose b, float progress);

	void SetBoneTransform(std::string bone, mat4 transform)
	{
		boneTransforms[bone] = transform;
	}

	void SetBoneTransform(std::string bone, MathHelper::Transform transform)
	{
		boneTransforms[bone] = transform.ToMatrix();
	}

	void SetBoneTransformEuler(std::string bone, MathHelper::Transform transform)
	{
		boneTransforms[bone] = transform.ToMatrixEuler();
	}

	MathHelper::Transform GetBoneTransform(std::string bone)
	{
		auto res = boneTransforms.find(bone);
		if (res != boneTransforms.end())
		{
			return MathHelper::DecomposeMatrix(res->second);
		}

		return MathHelper::Transform();
	}

};

struct AnimationState
{
	bool playing;
	string animationName;
	float animationTime;
	float oldAnimationEventTime;
	bool looping;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(AnimationState, playing, animationName, animationTime, oldAnimationEventTime,looping)

};

struct AnimationEvent
{
	string eventName;
	float time;
	string userData1;
	string userData2;
	string userData3;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(AnimationEvent, eventName, time, userData1, userData2, userData3)
};

struct AnimationData
{
	string animationName;
	vector<AnimationEvent> animationEvents;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(AnimationData, animationName, animationEvents)
};

struct HitboxData
{
	string boneName = "";
	vec3 size = vec3(0.2);
	vec3 rotation = vec3(0);
	vec3 position = vec3(0);
	float damageMultiplier = 1;

	string parentBone = "";

	mat4 constraintLocal1 = mat4();
	mat4 constraintLocal2 = mat4();

	vec3 twistParameters = vec3();
	vec3 constraintRotation = vec3();

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(HitboxData, boneName, size, rotation, position, damageMultiplier, parentBone, constraintLocal1, constraintLocal2, twistParameters, constraintRotation)
};

struct SkeletalMeshMetaData
{
	vector<HitboxData> hitboxes;
	vector<AnimationData> animations;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(SkeletalMeshMetaData, hitboxes, animations)


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

	quat rootMotionBasisQuat;

	float oldAnimationEventTime = 0;

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
	std::vector<Constraint*> hitboxConstraints; //to do: destroy them

	std::recursive_mutex hitboxMutex;

	std::vector<AnimationEvent> pendingAnimationEvents;

protected:

	void ApplyAdditionalShaderParams(ShaderProgram* shader_program)
	{
		shader_program->SetUniform("finalBonesMatrices", finalizedBoneTransforms);
	}

	static inline unordered_map<string, SkeletalMeshMetaData> loaded_metas;

public:

	SkeletalMeshMetaData metaData;
	AnimationData* currentAnimationData = nullptr;

	bool UpdatePose = true;
	bool UpdatePoseOnlyWhenRendered = false;

	bool InRagdoll = false;

	static inline int skelMeshes = 0;

	SkeletalMesh(Entity* owner) : StaticMesh(owner)
	{
		CastDetailShadows = true;
		skelMeshes++;
	}

	~SkeletalMesh()
	{
		ClearHitboxes();
		skelMeshes--;
		
	}

	AnimationPose GetAnimationPose()
	{
		if (model == nullptr) return AnimationPose();
		AnimationPose pose;
		pose.boneTransforms = animator.GetBonePoseArray();

		return pose;
	}

	void PasteAnimationPose(AnimationPose pose)
	{
		animator.ApplyBonePoseArray(pose.boneTransforms);
		boneTransforms = animator.getBoneMatrices();
	}

	void ApplyWorldSpaceBoneTransforms(std::unordered_map<std::string, mat4>& pose);

	void PlayAnimation(string name, bool Loop = false, float interpIn = 0.12);

	bool IsInFrustrum(Frustum frustrum);

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

	float GetAnimationDuration();
	float GetAnimationTime();
	void SetAnimationTime(float time);

	void StartedRendering();

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

	float GetHitboxDamageMultiplier(string boneName);

	void StartRagdoll();
	void StopRagdoll();

	void ClearHitboxes();

	void CreateHitbox(Entity* owner, HitboxData data);

	void CreateHitboxes(Entity* owner);

	Body* FindHitboxByName(string name);

	void UpdateHitboxes();

	void UpdateAnimationEvents();
	vector<AnimationEvent> PullAnimationEvents();

	void SaveMetaToFile();
	void LoadMetaFromFile();

	static void ClearMetaDataCache();

	AnimationData* GetAnimationDataFromName(string name);

	void SetAnimationState(const AnimationState& animationState)
	{
		PlayAnimation(animationState.animationName, animationState.looping, 0);
		animator.m_currTime = animationState.animationTime;
		animator.UpdateAnimationPose();
		animator.m_playing = animationState.playing;
		oldAnimationEventTime = animationState.oldAnimationEventTime;
		Update(0);
		PullAnimationEvents();
	}



	AnimationState GetAnimationState()
	{
		AnimationState animationState;

		animationState.looping = animator.Loop;
		animationState.animationName = animator.currentAnimationName;
		animationState.animationTime = animator.m_currTime;
		animationState.playing = animator.m_playing;
		animationState.oldAnimationEventTime = oldAnimationEventTime;

		return animationState;
	}



};