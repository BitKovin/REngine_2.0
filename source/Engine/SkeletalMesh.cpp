#include "SkeletalMesh.hpp"
#include <algorithm>

void SkeletalMesh::PlayAnimation(string name, bool Loop, float interpIn)
{

	if (model == nullptr) return;

	SetLooped(Loop);
	animator.set(name);
	PlayAnimation(interpIn);
	oldRootMotionPos = vec3();
	oldRootMotionRot = vec3();
	animator.totalRootMotionPosition = vec3();
	animator.totalRootMotionRotation = vec3();
	rootMotionBasisQuat = quat();
	currentAnimationData = GetAnimationDataFromName(name);	
	Update(0);
	//animator.UpdateAnimationPose();
	boneTransforms = animator.getBoneMatrices();
}

// In your SkeletalMesh:
MathHelper::Transform SkeletalMesh::PullRootMotion()
{

	// 1) grab the raw deltas from the animator
	glm::vec3 deltaPos = animator.totalRootMotionPosition - oldRootMotionPos;
	glm::vec3 deltaRot = animator.totalRootMotionRotation - oldRootMotionRot;

	// 2) stash totals for next frame:
	oldRootMotionPos = animator.totalRootMotionPosition;
	oldRootMotionRot = animator.totalRootMotionRotation;

	// 3) apply the offsets back to the skeleton (unchanged)
	positionOffset = -animator.totalRootMotionPosition;
	rotationOffset = -animator.totalRootMotionRotation;

	// 4) build the output transform:
	MathHelper::Transform t;

	rootMotionBasisQuat = MathHelper::GetRotationQuaternion(Rotation + rotationOffset);

	// — USE the fixed “basis” quat for _all_ translation:
	//   this freezes the walk-direction in world-space
	glm::quat basis = true
		? rootMotionBasisQuat
		: MathHelper::GetRotationQuaternion(Rotation);

	t.Position = MathHelper::TransformVector(deltaPos, basis);

	// — but still spin each frame by whatever the animator gave you:
	t.Rotation = deltaRot;

	return t;
}


void SkeletalMesh::Update(float timeScale)
{

	animator.UpdatePose = UpdatePose;

	animator.update(Time::DeltaTimeF * timeScale);
	UpdateAnimationEvents();

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

float SkeletalMesh::GetAnimationDuration()
{
	if (model == nullptr) return 0;

	if (animator.m_currAnim == nullptr) return 0;


	return animator.m_currAnim->duration;

}

float SkeletalMesh::GetAnimationTime()
{
	if (model == nullptr) return 0;

	if (animator.m_currAnim == nullptr) return 0;

	return animator.m_currTime;
}

void SkeletalMesh::SetAnimationTime(float time)
{
	if (model == nullptr) return;

	if (animator.m_currAnim == nullptr) return;

	animator.m_currTime = time;

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

void SkeletalMesh::CreateHitboxes(Entity* owner)
{

	ClearHitboxes();

	for (auto hitbox : metaData.hitboxes)
	{
		CreateHitbox(owner, hitbox);
	}

	UpdateHitboxes();

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

void SkeletalMesh::UpdateAnimationEvents()
{
	if (currentAnimationData == nullptr) return;

	float currentAnimationTime = GetAnimationTime();

	if (currentAnimationTime == oldAnimationEventTime) return;

	if (oldAnimationEventTime > currentAnimationTime)
	{
		oldAnimationEventTime = 0;
	}

	vector<AnimationEvent> pendingEvents;

	for (const auto& event : currentAnimationData->animationEvents)
	{
		if (event.time > oldAnimationEventTime && event.time <= currentAnimationTime)
		{
			pendingEvents.push_back(event);
		}
	}

	// Sort events from earliest to latest by time
	std::sort(pendingEvents.begin(), pendingEvents.end(), [](const AnimationEvent& a, const AnimationEvent& b) {
		return a.time < b.time;
		});

	// Push sorted events to the queue
	for (const auto& event : pendingEvents)
	{
		pendingAnimationEvents.push_back(event);
	}

	oldAnimationEventTime = currentAnimationTime;
}

vector<AnimationEvent> SkeletalMesh::PullAnimationEvents()
{
	vector<AnimationEvent> result = pendingAnimationEvents;
	pendingAnimationEvents.clear();
	return result;

}

unordered_map<string, SkeletalMeshMetaData> loaded_metas;

void SkeletalMesh::ClearMetaDataCache()
{
	loaded_metas.clear();
}

void SkeletalMesh::SaveMetaToFile()
{
	if (model == nullptr) return;

	string metaFilePath = filePath + ".skmm";

	json jsonData = json(metaData);
	string content = jsonData.dump(4);

	try {
		auto parent = std::filesystem::path(metaFilePath).parent_path();
		if (!parent.empty() && !std::filesystem::exists(parent)) {
			std::filesystem::create_directories(parent);
		}
	}
	catch (const std::exception& e) {
		// log or throw
	}
	std::ofstream ofs(metaFilePath, std::ios::out 
		| std::ios::binary 
		| std::ios::trunc);
	if (!ofs) return;
	ofs.write(content.data(), content.size());

}

void SkeletalMesh::LoadMetaFromFile()
{
	if (model == nullptr) return;

	string metaFilePath = filePath + ".skmm";

	auto foundData = loaded_metas.find(metaFilePath);

	SkeletalMeshMetaData data;

	if (foundData != loaded_metas.end())
	{
		data = foundData->second;
	}
	else
	{
		string file = AssetRegistry::ReadFileToString(metaFilePath);

		if (file.size() < 3)
		{
			return;
		}
			

		json jsonData = json::parse(file);

		data = jsonData.get<SkeletalMeshMetaData>();

		loaded_metas[metaFilePath] = data;

	}
	
	metaData = data;


}

AnimationData* SkeletalMesh::GetAnimationDataFromName(string name)
{
	for (AnimationData& data : metaData.animations)
	{
		if (data.animationName == name)
		{
			return &data;
		}
	}

	metaData.animations.push_back(AnimationData{ name });
	return GetAnimationDataFromName(name);
}
