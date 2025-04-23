#include "SkeletalMesh.hpp"

void SkeletalMesh::PlayAnimation(string name, bool Loop, float interpIn)
{
	SetLooped(Loop);
	animator.set(name);
	PlayAnimation(interpIn);
	oldRootMotionPos = vec3();
	oldRootMotionRot = vec3();
	animator.totalRootMotionPosition = vec3();
	animator.totalRootMotionRotation = vec3();
}

MathHelper::Transform SkeletalMesh::PullRootMotion()
{
	vec3 rootMotionPos = animator.totalRootMotionPosition - oldRootMotionPos;
	vec3 rootMotionRot = animator.totalRootMotionRotation - oldRootMotionRot;

	oldRootMotionPos = animator.totalRootMotionPosition;
	oldRootMotionRot = animator.totalRootMotionRotation;

	positionOffset = -animator.totalRootMotionPosition;
	rotationOffset = -animator.totalRootMotionRotation;


	MathHelper::Transform transform;

	transform.Position = MathHelper::TransformVector(rootMotionPos, MathHelper::GetRotationQuaternion(-rotationOffset));
	transform.Rotation = rootMotionRot;

	transform.Position = MathHelper::TransformVector(transform.Position, MathHelper::GetRotationQuaternion(Rotation));

	return transform;
}

void SkeletalMesh::Update(float timeScale)
{

	animator.UpdatePose = UpdatePose;

	animator.update(Time::DeltaTimeF * timeScale);

	if (animator.m_currTime < oldAnimTime)
	{
		oldRootMotionPos = vec3();
		oldRootMotionRot = vec3();
		animator.totalRootMotionPosition = vec3();
		animator.totalRootMotionRotation = vec3();
	}
	oldAnimTime = animator.m_currTime;

	if (UpdatePose == false) return;

	float blendProgress = GetBlendInProgress();

	if (blendProgress < 0.995)
	{
		AnimationPose currentPose = GetAnimationPose();


		AnimationPose newPose = AnimationPose::Lerp(blendStartPose, currentPose, blendProgress);

		PasteAnimationPose(newPose);

	}

	boneTransforms = animator.getBoneMatrices();

}

mat4 SkeletalMesh::GetWorldMatrixNoOffsets()
{
	return translate(Position) * MathHelper::GetRotationMatrix(Rotation) * scale(Scale);
}

mat4 SkeletalMesh::GetBoneMatrix(string boneName)
{
	if (model == nullptr) return mat4();

	auto res = model->boneInfoMap.find(boneName);

	if(res == model->boneInfoMap.end()) return mat4();

	int id = res->second.id;

	mat4 invOffset = inverse(res->second.offset);

	return boneTransforms[id] * invOffset;

}

mat4 SkeletalMesh::GetBoneMatrixWorld(string boneName)
{
	return GetWorldMatrix() * GetBoneMatrix(boneName);
}

void SkeletalMesh::ClearHitboxes()
{
	std::lock_guard<std::recursive_mutex> lock(hitboxMutex);

	for (Body* body : hitboxBodies)
	{
		Physics::DestroyBody(body);
	}

	hitboxBodies.clear();

}

void SkeletalMesh::CreateHitbox(Entity* owner,HitboxData data)
{
	std::lock_guard<std::recursive_mutex> lock(hitboxMutex);

	Body* body = Physics::CreateHitBoxBody(owner, data.boneName, data.position, MathHelper::GetRotationQuaternion(data.rotation), data.size);

	hitboxBodies.push_back(body);

}

void SkeletalMesh::CreateHitboxesFromData(Entity* owner, SkeletalMeshMetaData data)
{

	ClearHitboxes();

	for (auto hitbox : data.hitboxes)
	{
		CreateHitbox(owner, hitbox);
	}

}

void SkeletalMesh::UpdateHitboxes()
{

	std::lock_guard<std::recursive_mutex> lock(hitboxMutex);

	for (Body* body : hitboxBodies)
	{
		string boneName = Physics::GetBodyData(body)->hitboxName;

		MathHelper::Transform boneTrans = MathHelper::DecomposeMatrix(GetBoneMatrixWorld(boneName));

		Physics::SetBodyPositionAndRotation(body, boneTrans.Position, boneTrans.Rotation);

	}

}
