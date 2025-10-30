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

	std::unordered_map<hashed_string, mat4> resultPose;

	resultPose = b.boneTransforms;

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

static glm::mat4 ComputeGlobalTransform(
    const std::unordered_map<hashed_string, glm::mat4>& pose,
    const hashed_string& bone,
    const std::unordered_map<hashed_string, hashed_string>* parentMap)
{
    // if no parent map, assume local == global
    if (!parentMap) {
        auto it = pose.find(bone);
        return (it != pose.end()) ? it->second : glm::mat4(1.0f);
    }

    // accumulate from bone up to root: global = parentLocal * ... * boneLocal
    glm::mat4 global(1.0f);
    hashed_string cur = bone;
    auto it = pose.find(cur);
    if (it == pose.end()) return glm::mat4(1.0f);

    // start with local
    global = it->second;

    // walk up while we have parents in the map and in the pose
    while (true)
    {
        auto pit = parentMap->find(cur);
        if (pit == parentMap->end()) break;
        hashed_string parent = pit->second;
        auto parentPoseIt = pose.find(parent);
        if (parentPoseIt == pose.end()) break;
        // parentLocal * currentGlobal
        global = parentPoseIt->second * global;
        cur = parent;
    }
    return global;
}

AnimationPose AnimationPose::LayeredLerp(
	const hashed_string& startBoneName,
	roj::BoneNode* rootNode,
	const AnimationPose& poseA,
	const AnimationPose& poseB,
	bool UseWorldSpaceRotation,
	float progress)
{
	using glm::mat4;
	using glm::vec3;
	using glm::quat;

	// Fast paths
	if (!rootNode) return poseA;
	if (progress <= 0.0f) {
		// keep A but copy root from B if present (keeps previous behavior)
		AnimationPose out = poseA;
		auto rb = poseB.boneTransforms.find(hashed_string("root"));
		if (rb != poseB.boneTransforms.end()) out.boneTransforms[hashed_string("root")] = rb->second;
		return out;
	}
	if (progress >= 1.0f) return poseB;

	// --- helpers ---
	auto DecomposeLocal = [](const mat4& m, vec3& outT, vec3& outS, quat& outQ) {
		outT = vec3(m[3]);
		for (int i = 0; i < 3; ++i) outS[i] = glm::length(vec3(m[i]));
		glm::mat3 r;
		for (int i = 0; i < 3; ++i) {
			if (outS[i] != 0.0f) r[i] = vec3(m[i]) / outS[i];
			else r[i] = vec3(m[i]);
		}
		outQ = glm::quat_cast(r);
		};
	auto ComposeLocal = [](const vec3& t, const vec3& s, const quat& q) -> mat4 {
		glm::mat3 r = glm::mat3_cast(q);
		glm::mat3 sm(s.x, 0.0f, 0.0f,
			0.0f, s.y, 0.0f,
			0.0f, 0.0f, s.z);
		glm::mat3 rs = r * sm;
		mat4 out = mat4(rs);
		out[3] = glm::vec4(t, 1.0f);
		return out;
		};

	// --- 1) Build traversal order and bone->node lookup ---
	std::vector<hashed_string> traversal; traversal.reserve(256);
	std::unordered_map<hashed_string, roj::BoneNode*> nodeLookup; nodeLookup.reserve(256);
	traversal.clear();

	std::function<void(roj::BoneNode*)> walk = [&](roj::BoneNode* n) {
		if (!n) return;
		traversal.push_back(n->name);
		nodeLookup[n->name] = n;
		for (auto& c : n->children) walk(const_cast<roj::BoneNode*>(&c));
		};
	walk(rootNode);
	size_t N = traversal.size();
	if (N == 0) return poseA;

	// --- 2) Build bone->index map and parent index array ---
	std::unordered_map<hashed_string, int> indexMap; indexMap.reserve(N * 2);
	for (int i = 0; i < (int)N; ++i) indexMap[traversal[i]] = i;

	std::vector<int> parentIndex(N, -1);
	// fill parent index by visiting nodes again (less overhead than building parentMap earlier)
	std::function<void(roj::BoneNode*, int)> fillParent = [&](roj::BoneNode* n, int parentIdx) {
		int idx = indexMap[n->name];
		parentIndex[idx] = parentIdx;
		for (auto& c : n->children) fillParent(const_cast<roj::BoneNode*>(&c), idx);
		};
	fillParent(rootNode, -1);

	// --- 3) Preallocate arrays for local transforms and decomposed components ---
	std::vector<mat4> localA(N), localB(N);
	std::vector<vec3> tA(N), sA(N), tB(N), sB(N);
	std::vector<quat> qA(N), qB(N);

	for (int i = 0; i < (int)N; ++i) {
		const hashed_string& name = traversal[i];
		auto itA = poseA.boneTransforms.find(name);
		auto itB = poseB.boneTransforms.find(name);
		localA[i] = (itA != poseA.boneTransforms.end()) ? itA->second : mat4(1.0f);
		localB[i] = (itB != poseB.boneTransforms.end()) ? itB->second : mat4(1.0f);

		DecomposeLocal(localA[i], tA[i], sA[i], qA[i]);
		DecomposeLocal(localB[i], tB[i], sB[i], qB[i]);
	}

	// --- 4) Compute global (model) transforms for poseA and poseB in one pass parent-before-child ---
	std::vector<mat4> globalA(N), globalB(N);
	for (int i = 0; i < (int)N; ++i) {
		int p = parentIndex[i];
		if (p == -1) {
			globalA[i] = localA[i];
			globalB[i] = localB[i];
		}
		else {
			globalA[i] = globalA[p] * localA[i];
			globalB[i] = globalB[p] * localB[i];
		}
	}

	// --- 5) Identify subtree indices starting at startBoneName efficiently ---
	auto itStart = indexMap.find(startBoneName);
	if (itStart == indexMap.end()) {
		// start not found -> nothing to do, return poseA (copy root from poseB like prior behavior)
		AnimationPose out = poseA;
		auto rb = poseB.boneTransforms.find(hashed_string("root"));
		if (rb != poseB.boneTransforms.end()) out.boneTransforms[hashed_string("root")] = rb->second;
		return out;
	}
	int startIdx = itStart->second;

	std::vector<char> inSubtree(N, 0);
	std::vector<int> subtreeList; subtreeList.reserve(64);
	// iterative stack to avoid recursion
	std::vector<int> stack; stack.push_back(startIdx);
	while (!stack.empty()) {
		int cur = stack.back(); stack.pop_back();
		if (inSubtree[cur]) continue;
		inSubtree[cur] = 1;
		subtreeList.push_back(cur);
		// push children indices
		// to find children, we can iterate traversal and pick parentIndex == cur (but expensive O(N^2)); instead build adjacency once:
	}

	// Build adjacency list to push children only once (O(N))
	std::vector<std::vector<int>> children(N);
	for (int i = 0; i < (int)N; ++i) {
		int p = parentIndex[i];
		if (p >= 0) children[p].push_back(i);
	}
	// rebuild subtreeList using adjacency (faster)
	std::fill(inSubtree.begin(), inSubtree.end(), 0);
	subtreeList.clear();
	stack.clear();
	stack.push_back(startIdx);
	while (!stack.empty()) {
		int cur = stack.back(); stack.pop_back();
		if (inSubtree[cur]) continue;
		inSubtree[cur] = 1;
		subtreeList.push_back(cur);
		for (int ci : children[cur]) stack.push_back(ci);
	}

	// --- 6) Prepare result containers (local/result global) ---
	std::vector<mat4> resultLocal(N);
	std::vector<mat4> resultGlobal(N);

	// Set resultLocal to poseA local initially (bones outside subtree will remain A)
	for (int i = 0; i < (int)N; ++i) resultLocal[i] = localA[i];

	// Also fill resultGlobal for bones that are before the subtree or parents of subtree.
	// But simplest: compute result globals top-down; when processing subtree we will overwrite.
	// We'll iterate bones in traversal order and compute resultGlobal accordingly:
	for (int i = 0; i < (int)N; ++i) {
		// if bone is not in subtree, keep A's local; else skip (will be computed after)
		if (!inSubtree[i]) {
			int p = parentIndex[i];
			if (p == -1) resultGlobal[i] = resultLocal[i];
			else resultGlobal[i] = resultGlobal[p] * resultLocal[i];
		}
		else {
			// mark as unset; will compute during subtree pass
			resultGlobal[i] = mat4(0.0f);
		}
	}

	// --- 7) Process subtree top-down (parents before children) using subtreeList in topological order
	// Ensure subtreeList is in parent-before-child order: the adjacency push above preserves that if started from startIdx.
	// But to be safe, sort subtreeList by index of traversal (because traversal is parent-before-child)
	std::sort(subtreeList.begin(), subtreeList.end(), [&](int a, int b) { return a < b; });

	for (int idx : subtreeList) {
		// blended translation and scale
		vec3 blendedT = glm::mix(tA[idx], tB[idx], progress);
		vec3 blendedS = glm::mix(sA[idx], sB[idx], progress);

		quat finalLocalQ;
		if (!UseWorldSpaceRotation) {
			finalLocalQ = glm::slerp(qA[idx], qB[idx], progress);
		}
		else {
			// compute desired global rotation from globalB
			mat4 gB = globalB[idx];
			// extract rotation
			vec3 gS; for (int k = 0; k < 3; ++k) gS[k] = glm::length(vec3(gB[k]));
			glm::mat3 gRot;
			for (int k = 0; k < 3; ++k) gRot[k] = (gS[k] != 0.0f) ? vec3(gB[k]) / gS[k] : vec3(gB[k]);
			quat desiredGlobalQ = glm::quat_cast(gRot);

			// find parent result rotation
			int p = parentIndex[idx];
			quat parentResultQ = quat(1.0f, 0.0f, 0.0f, 0.0f);
			if (p != -1) {
				// ensure parent's resultGlobal is computed (it should be because traversal order puts parents first)
				mat4 pglob = resultGlobal[p];
				vec3 pS; for (int k = 0; k < 3; ++k) pS[k] = glm::length(vec3(pglob[k]));
				glm::mat3 pRot;
				for (int k = 0; k < 3; ++k) pRot[k] = (pS[k] != 0.0f) ? vec3(pglob[k]) / pS[k] : vec3(pglob[k]);
				parentResultQ = glm::quat_cast(pRot);
			}

			// local = inverse(parent) * desiredGlobal
			finalLocalQ = glm::inverse(parentResultQ) * desiredGlobalQ;
		}

		mat4 newLocal = ComposeLocal(blendedT, blendedS, finalLocalQ);
		resultLocal[idx] = newLocal;

		// compute resultGlobal for this bone
		int p = parentIndex[idx];
		if (p == -1) resultGlobal[idx] = newLocal;
		else resultGlobal[idx] = resultGlobal[p] * newLocal;
	}

	// --- 8) Compose result AnimationPose map from resultLocal (only fill bones we have in traversal) ---
	AnimationPose out;
	out.boneTransforms.reserve(resultLocal.size());
	for (int i = 0; i < (int)N; ++i) {
		out.boneTransforms[traversal[i]] = resultLocal[i];
	}

	// maintain previous behavior copying root from B if present
	auto rootItB = poseB.boneTransforms.find(hashed_string("root"));
	if (rootItB != poseB.boneTransforms.end()) out.boneTransforms[hashed_string("root")] = rootItB->second;

	return out;
}




void SkeletalMesh::ApplyWorldSpaceBoneTransforms(std::unordered_map<hashed_string, mat4>& pose)
{

	mat4 world = GetWorldMatrix();

	for (auto& bonePose : pose)
	{
		pose[bonePose.first] = inverse(world) * pose[bonePose.first];
	}

	animator.ApplyLocalSpacePoseArray(GetAnimationPose().boneTransforms, pose);
	boneTransforms = animator.getBoneMatrices();

}

void SkeletalMesh::PlayAnimation(std::string name, bool Loop, float interpIn)
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
	bonePositions.reserve(hitboxBodies.size());

	for (auto hitboxBody : hitboxBodies)
	{
		bonePositions.push_back(FromPhysics(hitboxBody->GetPosition()));
	}

	auto box = BoundingBox::FromPoints(bonePositions);

	box.Max += vec3(1);
	box.Max -= vec3(1);

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
	return IsInFrustrum(Camera::frustum) && isVisible();
}

BoundingBox SkeletalMesh::GetBoundingBox()
{
	if (InRagdoll == false)
	{
		return StaticMesh::GetBoundingBox();
	}

	mat4 world = GetWorldMatrix();

	vector<vec3> bonePositions;
	bonePositions.reserve(hitboxBodies.size());

	for (auto hitboxBody : hitboxBodies)
	{

		bonePositions.push_back(FromPhysics(hitboxBody->GetPosition()));

	}

	auto box = BoundingBox::FromPoints(bonePositions);

	box.Min -= vec3(1);
	box.Max += vec3(1);

	//DebugDraw::Bounds(box.Min, box.Max, 0.01f);


	return box;
}

bool SkeletalMesh::IsInFrustrum(Frustum frustrum)
{

	if (InRagdoll == false)
	{
		return StaticMesh::IsInFrustrum(frustrum);
	}

	mat4 world = GetWorldMatrix();

	vector<vec3> bonePositions;
	bonePositions.reserve(hitboxBodies.size());
	
	for (auto hitboxBody : hitboxBodies)
	{

		bonePositions.push_back(FromPhysics(hitboxBody->GetPosition()));

	}

	auto box = BoundingBox::FromPoints(bonePositions);

	box.Min -= vec3(1);
	box.Max += vec3(1);

	//DebugDraw::Bounds(box.Min, box.Max, 0.01f);


	return frustrum.IsBoxVisible(box.Min, box.Max);

}

LightVolPointData SkeletalMesh::GetLightVolData()
{
	if (InRagdoll == false)
		return StaticMesh::GetLightVolData();

	mat4 world = GetWorldMatrix();

	vector<vec3> bonePositions;
	bonePositions.reserve(model->boneInfoMap.size());

	for (auto hitboxBody : hitboxBodies)
	{

		bonePositions.push_back(FromPhysics(hitboxBody->GetPosition()));

	}

	auto box = BoundingBox::FromPoints(bonePositions);

	if (model == nullptr ||
		Level::Current->BspData.lightVols.size() == 0) return LightVolPointData{ vec3(0),vec3(1),vec3(0) };

	vec3 samplePos = box.Center() + vec3(0, 0.5, 0);


	auto light = Level::Current->BspData.GetLightvolColorPoint(samplePos * MAP_SCALE, true);
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

bool SkeletalMesh::IsAnimationPlaying()
{
	return animator.m_playing;
}

void SkeletalMesh::Update(float timeScale)
{

	if (model == nullptr) return;

	animator.UpdatePose = UpdatePose;

	animator.InterpolatePosition = InterpolatePosition;
	animator.InterpolateRotation = InterpolateRotation;
	animator.InterpolateScale = InterpolateScale;

	if (UpdatePoseOnlyWhenRendered)
	{
		animator.UpdatePose = WasRended;
	}

	animator.update(Time::DeltaTimeF * timeScale);
	UpdateAnimationEvents();

	if (animator.m_currTime != oldAnimTime)
	{
		if (animator.UpdatePose)
		{
			dirtyPose = true;
		}
	}

	oldAnimTime = animator.m_currTime;

	//if (UpdatePose == false) return;

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


	return animator.m_currAnim->duration / animator.m_currAnim->ticksPerSec;

}

float SkeletalMesh::GetAnimationTime()
{
	if (model == nullptr) return 0;

	if (animator.m_currAnim == nullptr) return 0;

	return animator.m_currTime / animator.m_currAnim->ticksPerSec;
}

void SkeletalMesh::SetAnimationTime(float time)
{
	if (model == nullptr) return;

	if (animator.m_currAnim == nullptr) return;

	animator.m_currTime = time * animator.m_currAnim->ticksPerSec;

}

roj::BoneNode* SkeletalMesh::GetNodeFromName(const hashed_string& name)
{

	if (model == nullptr) return nullptr;

	auto nodeResult = model->boneNodesMap.find(name);

	if (nodeResult != model->boneNodesMap.end())
	{
		return &nodeResult->second;
	}

	return nullptr;
}

roj::BoneNode* SkeletalMesh::GetRootNode()
{
	if (model == nullptr) return nullptr;

	return & model->defaultRoot;

}

std::unordered_map<hashed_string, hashed_string>* SkeletalMesh::GetNodeParentMap()
{
	if (model == nullptr) return nullptr;

	return &model->parentMap;
}

void SkeletalMesh::StartedRendering()
{

	if (model == nullptr) return;

	Update(0);

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

		vec3 linearVel = vec3();
		vec3 angularVel = vec3();

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


		Physics::SetLinearVelocity(hitbox, linearVel/1.5f);
		Physics::SetAngularVelocity(hitbox, angularVel/1.5f);

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

	defaultBoneScale.clear();

	hitboxBodies.clear();
	hitboxConstraints.clear();
}

void SkeletalMesh::CreateHitbox(Entity* owner,HitboxData data)
{
	std::lock_guard<std::recursive_mutex> lock(hitboxMutex);

	Body* body = Physics::CreateHitBoxBody(owner, this, data.boneName, data.position, MathHelper::GetRotationQuaternion(data.rotation), data.size);

	defaultBoneScale[data.boneName] = MathHelper::DecomposeMatrix(GetBoneMatrixWorld(data.boneName)).Scale;

	hitboxBodies.push_back(body);

}

void SkeletalMesh::CreateHitboxes(Entity* owner)
{

	auto oldPose = GetAnimationPose();

	animator.ApplyBonePoseArray(std::unordered_map<hashed_string, mat4>{}); //applying rest pose without visual update

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
		
		Physics::ConfigureSwingTwistMotor(constraint);

		constraint->SetEnabled(false);

		hitboxConstraints[hitbox.boneName] = constraint;

	}

	animator.ApplyBonePoseArray(oldPose.boneTransforms); //restoring old pose just in case

}

void SkeletalMesh::ApplyImpulseToAllHitboxes(vec3 impulse, bool scaleWithMass)
{

	if (scaleWithMass)
	{
		for (Body* body : hitboxBodies)
		{

			float invMass = body->GetMotionProperties()->GetInverseMass();
			float mass = (invMass > 0.0f) ? 1.0f / invMass : FLT_MAX;

			Physics::AddImpulse(body, impulse * mass);
		}
	}
	else
	{
		for (Body* body : hitboxBodies)
		{
			Physics::AddImpulse(body, impulse);
		}
	}

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

		

		if (hitboxConstraints.size() > 0)
		{




			std::unordered_map<hashed_string, mat4> animationPose;

			std::unordered_map<hashed_string, quat> hitboxRelativePose;

			bool hasInvalidTransform = false;

			for (const HitboxData& data : metaData.hitboxes)
			{

				if (data.parentBone == "") continue;

				const auto& boneName = data.boneName;

				if (hitboxConstraints.contains(boneName))
					hitboxConstraints[boneName]->SetEnabled(true);

				mat4 relativeTransform = mat4();

				if (RagdollPoseFollowStrength > 0)
				{
					mat4 parentBone = GetBoneMatrix(data.parentBone);
					mat4 childBone = GetBoneMatrix(data.boneName);

					relativeTransform = inverse(parentBone) * childBone;
				}

				quat resultQuat = normalize(quat_cast(relativeTransform));
				
				// 1) Ensure no NaNs / infinities
				if (!std::isfinite(resultQuat.x) || !std::isfinite(resultQuat.y) ||
					!std::isfinite(resultQuat.z) || !std::isfinite(resultQuat.w))
				{
					hasInvalidTransform = true;
				}

				hitboxRelativePose[boneName] = resultQuat;

			}

			if (hasInvalidTransform == false)
			{
				for (auto relHitboxPos : hitboxRelativePose)
				{

					Physics::UpdateSwingTwistMotor(hitboxConstraints[relHitboxPos.first], relHitboxPos.second, RagdollPoseFollowStrength);
				}
			}


		}
		

		std::unordered_map<hashed_string, mat4> pose;

		

		for (Body* body : hitboxBodies)
		{

			vec3 pos = FromPhysics(body->GetPosition());
			quat rot = FromPhysics(body->GetRotation());

			const auto& boneName = Physics::GetBodyData(body)->hitboxName;

			if (RagdollPoseFollowStrength > 0)
			{
				Physics::Activate(body);
			}

			auto res = animator.currentPose.find(boneName);

			vec3 scale = defaultBoneScale[boneName];



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

	mat4 world = GetWorldMatrix();

	for (Body* body : hitboxBodies)
	{
		string boneName = Physics::GetBodyData(body)->hitboxName;

		if(hitboxConstraints.contains(boneName))
			hitboxConstraints[boneName]->SetEnabled(false);

		MathHelper::Transform boneTrans = MathHelper::DecomposeMatrix(world * GetBoneMatrix(boneName));

		vec3 oldPos = FromPhysics(body->GetPosition());
		vec3 newPos = boneTrans.Position;

		quat oldRot = FromPhysics(body->GetRotation());
		quat newRot = boneTrans.RotationQuaternion;

		vec3 linearVelocity = (newPos - oldPos) / Time::DeltaTimeF;

		quat deltaRot = newRot * glm::inverse(oldRot);
		deltaRot = glm::normalize(deltaRot);

		vec3 angularVelocity = (2.0f / Time::DeltaTimeF) * vec3(deltaRot.x, deltaRot.y, deltaRot.z);

		boneLinearVel[boneName] = linearVelocity;
		boneAngularVel[boneName] = angularVelocity;

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

AnimationData* SkeletalMesh::GetAnimationDataFromName(std::string name)
{
	for (AnimationData& data : metaData.animations)
	{
		if (data.animationName == name)
		{
			return &data;
		}
	}

	metaData.animations.push_back(AnimationData{ name});
	return GetAnimationDataFromName(name);
}

void SkeletalMesh::SetAnimationState(const AnimationState& animationState)
{

	if (animationState.animationName != "")
	{
		PlayAnimation(animationState.animationName, animationState.looping, 0);
	}
	else
	{
		PasteAnimationPose(AnimationPose()); //setting to rest pose
	}

	animator.m_currTime = animationState.animationTime;
	animator.UpdateAnimationPose();
	animator.m_playing = animationState.playing;
	oldAnimationEventTime = animationState.oldAnimationEventTime;
	Update(0);
	PullAnimationEvents();

	InRagdoll = animationState.inRagdoll;

	if (InRagdoll)
	{

		StartRagdoll();

		for (auto hitboxBody : hitboxBodies)
		{


			Physics::SetLinearVelocity(hitboxBody, vec3());
			Physics::SetAngularVelocity(hitboxBody, vec3());

			string hitboxName = Physics::GetBodyData(hitboxBody)->hitboxName;

			auto posRes = animationState.ragdollHitboxPositions.find(hitboxName);
			auto rotRes = animationState.ragdollHitboxRotations.find(hitboxName);

			auto posVelRes = animationState.ragdollHitboxLinearVelocty.find(hitboxName);
			auto rotVelRes = animationState.ragdollHitboxAngularVelocty.find(hitboxName);

			if (posRes != animationState.ragdollHitboxPositions.end())
			{
				Physics::SetBodyPosition(hitboxBody, posRes->second);
			}

			if (rotRes != animationState.ragdollHitboxRotations.end())
			{
				Physics::SetBodyRotation(hitboxBody, rotRes->second);
			}

			if (posVelRes != animationState.ragdollHitboxLinearVelocty.end())
			{
				Physics::SetLinearVelocity(hitboxBody, posVelRes->second);
			}

			if (rotVelRes != animationState.ragdollHitboxAngularVelocty.end())
			{
				Physics::SetAngularVelocity(hitboxBody, rotVelRes->second);
			}


		}
	}

}

AnimationState SkeletalMesh::GetAnimationState()
{
	AnimationState animationState;

	animationState.looping = animator.Loop;
	animationState.animationName = animator.currentAnimationName;
	animationState.animationTime = animator.m_currTime;
	animationState.playing = animator.m_playing;
	animationState.oldAnimationEventTime = oldAnimationEventTime;

	unordered_map<string, vec3> hitboxPositions;
	unordered_map<string, quat> hitboxRotations;

	unordered_map<string, vec3> hitboxPositionsVel;
	unordered_map<string, vec3> hitboxRotationsVel;

	if (InRagdoll)
	{
		for (auto hitboxBody : hitboxBodies)
		{

			string hitboxName = Physics::GetBodyData(hitboxBody)->hitboxName;

			hitboxPositions[hitboxName] = FromPhysics(hitboxBody->GetPosition());
			hitboxRotations[hitboxName] = FromPhysics(hitboxBody->GetRotation());

			hitboxPositionsVel[hitboxName] = FromPhysics(hitboxBody->GetLinearVelocity());
			hitboxRotationsVel[hitboxName] = FromPhysics(hitboxBody->GetAngularVelocity());

		}
	}

	animationState.inRagdoll = InRagdoll;
	animationState.ragdollHitboxPositions = hitboxPositions;
	animationState.ragdollHitboxRotations = hitboxRotations;

	animationState.ragdollHitboxLinearVelocty = hitboxPositionsVel;
	animationState.ragdollHitboxAngularVelocty = hitboxRotationsVel;

	return animationState;
}
