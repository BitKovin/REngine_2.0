#include "SkeletalMesh.hpp"
#include <algorithm>
#include "Level.hpp"

AnimationPose AnimationPose::Lerp(AnimationPose a, AnimationPose b, float progress)
{

	if (progress > 0.995) // not doing same with small value since root bone should always be blended to B
		return b;

	if (progress < 0.005) // not doing same with small value since root bone should always be blended to B
	{

		if (a.boneTransforms.find("root") != a.boneTransforms.end() &&
			b.boneTransforms.find("root") != b.boneTransforms.end())
		{
			a.boneTransforms["root"] = b.boneTransforms["root"];
			return a;
		}


	}

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

		glm::vec3 aTrans = glm::vec3(aMat[3]);
		glm::vec3 bTrans = glm::vec3(bMat[3]);

		// Extract scale (length of each column)
		glm::vec3 aScale, bScale;
		for (int i = 0; i < 3; i++) {
			aScale[i] = glm::length(aMat[i]);
			bScale[i] = glm::length(bMat[i]);
		}

		// Extract rotation (normalize columns)
		glm::mat3 aRotMat, bRotMat;
		for (int i = 0; i < 3; i++) {
			aRotMat[i] = aMat[i] / aScale[i];
			bRotMat[i] = bMat[i] / bScale[i];
		}

		// Convert to quaternions
		glm::quat aQuat = glm::quat_cast(aRotMat);
		glm::quat bQuat = glm::quat_cast(bRotMat);

		// Interpolate
		glm::vec3 trans = glm::mix(aTrans, bTrans, progress);
		glm::vec3 scale = glm::mix(aScale, bScale, progress);
		glm::quat quat = glm::slerp(aQuat, bQuat, progress);

		// Build result matrix
		glm::mat3 rotMat = glm::mat3_cast(quat);
		glm::mat3 scaleMat(scale.x, 0, 0, 0, scale.y, 0, 0, 0, scale.z);
		glm::mat3 rsMat = rotMat * scaleMat; // Rotation * Scale
		mat4 resultMat = glm::mat4(rsMat);         // Embed 3x3 into 4x4
		resultMat[3] = glm::vec4(trans, 1.0f); // Set translation

		resultPose[bonePose.first] = resultMat;
	}

	AnimationPose result;

	result.boneTransforms = resultPose;

	return result;

}

void SkeletalMesh::ApplyWorldSpaceBoneTransforms(std::unordered_map<std::string, mat4>& pose)
{

	mat4 world = GetWorldMatrix();

	for (auto& bonePose : pose)
	{
		pose[bonePose.first] = inverse(world) * pose[bonePose.first];
	}

	animator.ApplyLocalSpacePoseArray(GetAnimationPose().boneTransforms, pose);
	boneTransforms = animator.getBoneMatrices();

}

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
	animator.oldRootBoneTransform = MathHelper::Transform();
	rootMotionBasisQuat = quat();
	currentAnimationData = GetAnimationDataFromName(name);	
	Update(0);
	//animator.UpdateAnimationPose();
	boneTransforms = animator.getBoneMatrices();
}

void SkeletalMesh::SetAnimationPaused(bool value)
{

	animator.m_playing = !value;

}

bool SkeletalMesh::GetAnimationPaused()
{
	return !animator.m_playing;
}

bool SkeletalMesh::IsCameraVisible()
{
	if (InRagdoll == false)
	{
		return StaticMesh::IsCameraVisible();
	}

	mat4 world = GetWorldMatrix();

	vector<vec3> bonePositions;
	bonePositions.reserve(model->boneInfoMap.size());

	for (auto bone : model->boneInfoMap)
	{
		mat4 boneMatrix = world * GetBoneMatrix(bone.first);

		vec3 pos = vec3(boneMatrix[3]);
		bonePositions.push_back(pos);


	}

	auto box = BoundingBox::FromPoints(bonePositions);

	if (Level::Current->BspData.m_numOfVerts)
	{

		int cameraC = Level::Current->BspData.FindClusterAtPosition(Camera::finalizedPosition);
		int targetC = Level::Current->BspData.FindClusterAtPosition(box.Center() + vec3(0,1,0));

		

		if (Level::Current->BspData.IsClusterVisible(cameraC, targetC) == false)
		{

			//DebugDraw::Line(box.Center(), Camera::position - vec3(0, 1, 0));

			return false;
		}
	}
	return true;
}

bool SkeletalMesh::IsInFrustrum(Frustum frustrum)
{

	if (InRagdoll == false)
	{
		return StaticMesh::IsInFrustrum(frustrum);
	}

	mat4 world = GetWorldMatrix();

	vector<vec3> bonePositions;
	bonePositions.reserve(model->boneInfoMap.size());
	
	for (auto bone : model->boneInfoMap)
	{
		mat4 boneMatrix = world * GetBoneMatrix(bone.first);

		vec3 pos = vec3(boneMatrix[3]);
		bonePositions.push_back(pos);


	}

	auto box = BoundingBox::FromPoints(bonePositions);

	box.Min -= vec3(1);
	box.Max += vec3(1);


	return frustrum.IsBoxVisible(box.Min, box.Max);

}

LightVolPointData SkeletalMesh::GetLightVolData()
{
	if (InRagdoll == false)
		return StaticMesh::GetLightVolData();

	mat4 world = GetWorldMatrix();

	vector<vec3> bonePositions;
	bonePositions.reserve(model->boneInfoMap.size());

	for (auto bone : model->boneInfoMap)
	{
		mat4 boneMatrix = world * GetBoneMatrix(bone.first);

		vec3 pos = vec3(boneMatrix[3]);
		bonePositions.push_back(pos);

	}

	auto box = BoundingBox::FromPoints(bonePositions);

	if (model == nullptr ||
		Level::Current->BspData.lightVols.size() == 0) return LightVolPointData{ vec3(0),vec3(1),vec3(0) };

	vec3 samplePos = box.Center() + vec3(0, 0.5, 0);

	auto light = Level::Current->BspData.GetLightvolColor(samplePos * MAP_SCALE);
	return light;
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
	rotationOffset = -animator.totalRootMotionRotation ;

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

	if (UpdatePoseOnlyWhenRendered)
	{
		animator.UpdatePose = WasRended;
	}

	animator.update(Time::DeltaTimeF * timeScale);
	UpdateAnimationEvents();


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

void SkeletalMesh::StartedRendering()
{

	if (model == nullptr) return;
	animator.update(0);
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

float SkeletalMesh::GetHitboxDamageMultiplier(string boneName)
{
	for (auto& hitbox : metaData.hitboxes)
	{
		if (hitbox.boneName == boneName)
			return hitbox.damageMultiplier;
	}

	return 1.0f;
}

void SkeletalMesh::StartRagdoll()
{

	InRagdoll = true;

	for (auto& hitbox : hitboxBodies)
	{

		Physics::SetMotionType(hitbox, JPH::EMotionType::Dynamic);
		hitbox->SetMotionType(JPH::EMotionType::Dynamic);

		Physics::SetCollisionMask(hitbox, BodyType::World);

		const string boneName = Physics::GetBodyData(hitbox)->hitboxName;

		vec3 linearVel = vec3(0,0,0);
		quat angularVel = quat();

		auto linearRes = boneLinearVel.find(boneName);
		auto angularRes = boneAngularVel.find(boneName);

		if (linearRes != boneLinearVel.end())
		{
			linearVel = linearRes->second;
		}

		if (angularRes != boneAngularVel.end())
		{
			angularVel = angularRes->second;
		}

		auto constraint = GetConstraintByHitboxName(boneName);

		if(constraint != nullptr)
		{
			constraint->SetEnabled(true);
		}

		Physics::SetLinearVelocity(hitbox, linearVel/3.0f);
		Physics::SetAngularVelocity(hitbox, MathHelper::ToYawPitchRoll(angularVel)/3.0f);

	}

}

void SkeletalMesh::StopRagdoll()
{
	InRagdoll = false;

	for (auto& hitbox : hitboxBodies)
	{

		Physics::SetMotionType(hitbox, JPH::EMotionType::Kinematic);

		Physics::SetCollisionMask(hitbox, BodyType::None);


	}

	for (auto constraint : hitboxConstraints)
	{
		constraint.second->SetEnabled(false);
	}

}

void SkeletalMesh::ClearHitboxes()
{
	std::lock_guard<std::recursive_mutex> lock(hitboxMutex);

	for (Body* body : hitboxBodies)
	{
		Physics::DestroyBody(body);
	}

	for (auto constraint : hitboxConstraints)
	{
		Physics::DestroyConstraint(constraint.second);
	}

	hitboxBodies.clear();
	hitboxConstraints.clear();
}

void SkeletalMesh::CreateHitbox(Entity* owner,HitboxData data)
{
	std::lock_guard<std::recursive_mutex> lock(hitboxMutex);

	Body* body = Physics::CreateHitBoxBody(owner, data.boneName, data.position, MathHelper::GetRotationQuaternion(data.rotation), data.size);

	hitboxBodies.push_back(body);

}

void SkeletalMesh::CreateHitboxes(Entity* owner)
{

	auto oldPose = GetAnimationPose();

	animator.ApplyBonePoseArray(std::unordered_map<std::string, mat4>{}); //applying rest pose without visual update

	ClearHitboxes();

	for (auto hitbox : metaData.hitboxes)
	{
		CreateHitbox(owner, hitbox);
	}

	UpdateHitboxes();

	for (auto hitbox : metaData.hitboxes)
	{

		if (hitbox.parentBone == "") continue;
		
		Body* parentBody = FindHitboxByName(hitbox.parentBone);
		Body* currentBody = FindHitboxByName(hitbox.boneName);

		if (parentBody == nullptr || currentBody == nullptr) continue;

		auto constraint = Physics::CreateRagdollConstraint(parentBody, currentBody, hitbox.twistParameters.x, hitbox.twistParameters.y, hitbox.twistParameters.z, ToPhysics(MathHelper::GetRotationQuaternion(hitbox.constraintRotation)));
		
		constraint->SetEnabled(false);

		hitboxConstraints[hitbox.boneName] = constraint;

	}

	animator.ApplyBonePoseArray(oldPose.boneTransforms); //restoring old pose just in case

}

Constraint* SkeletalMesh::GetConstraintByHitboxName(string name)
{

	auto res = hitboxConstraints.find(name);

	if (res != hitboxConstraints.end())
	{
		return res->second;
	}

	return nullptr;
}

Body* SkeletalMesh::FindHitboxByName(string name)
{
	for (auto hitbox : hitboxBodies)
	{

		if (Physics::GetBodyData(hitbox)->hitboxName == name)
		{
			return hitbox;
		}

	}

	return nullptr;
}

void SkeletalMesh::UpdateHitboxes()
{

	if (InRagdoll)
	{

		std::unordered_map<std::string, mat4> pose;

		for (Body* body : hitboxBodies)
		{

			vec3 pos = FromPhysics(body->GetPosition());
			quat rot = FromPhysics(body->GetRotation());

			const auto& boneName = Physics::GetBodyData(body)->hitboxName;

			auto res = animator.currentPose.find(boneName);

			vec3 scale = MathHelper::DecomposeMatrix(GetBoneMatrixWorld(boneName)).Scale;



			MathHelper::Transform boneTrans;

			boneTrans.Position = pos;
			boneTrans.RotationQuaternion = rot;
			boneTrans.Scale = scale;

			pose[boneName] = boneTrans.ToMatrix();


		}

		ApplyWorldSpaceBoneTransforms(pose);

		return;
	}

	std::lock_guard<std::recursive_mutex> lock(hitboxMutex);

	for (Body* body : hitboxBodies)
	{
		string boneName = Physics::GetBodyData(body)->hitboxName;

		MathHelper::Transform boneTrans = MathHelper::DecomposeMatrix(GetBoneMatrixWorld(boneName));

		vec3 oldPos = FromPhysics(body->GetPosition());
		vec3 newPos = boneTrans.Position;

		quat oldRot = FromPhysics(body->GetRotation());
		quat newRot = boneTrans.RotationQuaternion;

		vec3 linearVelocity = (newPos - oldPos) / Time::DeltaTimeF;

		quat deltaRot = newRot * glm::inverse(oldRot);
		deltaRot = glm::normalize(deltaRot);

		quat angularVelocityQuat = quat(
			deltaRot.w - 1.0f,
			deltaRot.x,
			deltaRot.y,
			deltaRot.z
		) * (2.0f / Time::DeltaTimeF);

		boneLinearVel[boneName] = linearVelocity;
		boneAngularVel[boneName] = angularVelocityQuat;

		Physics::SetBodyPositionAndRotation(body, boneTrans.Position, boneTrans.RotationQuaternion);

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
