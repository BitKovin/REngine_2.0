#include "BoneCloth.h"

#include "../Console/ConsoleRegister.h"
#include "../DebugDraw.hpp"
#include "../Logger.hpp"
#include "../Time.hpp"
#include "../World/WindField.h"

#include <algorithm>
#include <cmath>
#include <limits>

struct BoneCloth::RestVertex
{
	glm::vec3 position = glm::vec3(0.0f);   // rest pose, model space
	uint16_t bones[MAX_BONE_INFLUENCE] = {};
	float weights[MAX_BONE_INFLUENCE] = {};
	uint16_t dominant = INVALID_BONE_IDX;
};

namespace
{
	constexpr float SkinnedWeight = 0.05f;   // a bone with vertices above this weight carries geometry
	constexpr float TipWeight = 0.25f;       // vertices that reach a strip's end
	constexpr float JoinTolerance = 0.05f;   // a strip continues into one whose top is this far off its line, per length
	constexpr float SideTolerance = 0.02f;   // strip tops nearer the middle than this are neither left nor right
	constexpr float ProbeWeight = 0.5f;      // a joint's probes are among the vertices it carries this much of
	constexpr size_t MaxProbes = 6;          // per joint, spread over the cloth it carries
	constexpr int ProbeIterations = 3;       // turns a frame that bring a joint's deepest probe out of the surface colliders
	constexpr uint32_t MaxSurfaceColliders = 32;   // a probe's `ignore` has a bit per collider
	constexpr int ClipSampleCount = 17;      // samples taken evenly over each wind and turn clip
	constexpr float MaxDeltaTime = 1.0f / 15.0f;   // longer frames (hitches) swing this much
	constexpr float DebugLineTime = 0.001f;        // one frame

	glm::vec3 Perpendicular(const glm::vec3& v)
	{
		const glm::vec3 other = std::abs(v.x) < 0.9f ? glm::vec3(1.0f, 0.0f, 0.0f) : glm::vec3(0.0f, 1.0f, 0.0f);
		return glm::normalize(glm::cross(v, other));
	}

	float Median(std::vector<float>& values)
	{
		auto middle = values.begin() + static_cast<std::ptrdiff_t>(values.size() / 2);
		std::nth_element(values.begin(), middle, values.end());
		return *middle;
	}

	// smoothstep that also takes start == full (a step).
	float Ramp(float start, float full, float x)
	{
		return full > start ? glm::smoothstep(start, full, x) : (x >= start ? 1.0f : 0.0f);
	}

	float AxisParameter(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b)
	{
		const glm::vec3 axis = b - a;
		const float length2 = glm::dot(axis, axis);
		return length2 > 1e-12f ? glm::clamp(glm::dot(p - a, axis) / length2, 0.0f, 1.0f) : 0.0f;
	}

	// The clip's first and last frames as poses (local transforms).
	bool SampleClip(roj::SkinnedModel& model, const std::string& name, AnimationPose& first, AnimationPose& last)
	{
		const hashed_string key(name);
		auto clip = model.clips.find(key);
		if (clip == model.clips.end() || model.animations.find(key) == model.animations.end())
			return false;

		roj::Animator sampler(&model);
		sampler.set(key);
		sampler.m_currTime = 0.0f;
		sampler.UpdateAnimationPose();
		first.boneTransforms = sampler.GetBonePoseArray();
		sampler.m_currTime = clip->second->duration;
		sampler.UpdateAnimationPose();
		last.boneTransforms = sampler.GetBonePoseArray();
		first.boneTreeRoot = last.boneTreeRoot = &model.defaultRoot;
		return true;
	}

	// The tail kept at the joint's length from its head, within maxAngle of its rest direction.
	glm::vec3 Constrain(const glm::vec3& head, const glm::vec3& tail, const glm::vec3& restTail, float maxAngle)
	{
		const glm::vec3 rest = restTail - head;
		const float length = glm::length(rest);
		if (length < 1e-6f)
			return restTail;
		const glm::vec3 restDirection = rest / length;
		glm::vec3 direction = tail - head;
		const float current = glm::length(direction);
		direction = current > 1e-6f ? direction / current : restDirection;
		if (glm::dot(direction, restDirection) < std::cos(maxAngle))
		{
			glm::vec3 axis = glm::cross(restDirection, direction);
			const float axisLength = glm::length(axis);
			axis = axisLength > 1e-6f ? axis / axisLength : Perpendicular(restDirection);
			direction = glm::angleAxis(maxAngle, axis) * restDirection;
		}
		return head + direction * length;
	}
}

// ---- Build ----------------------------------------------------------------------------------------------------

bool BoneCloth::Build(const BoneClothData& clothData, roj::SkinnedModel& clothModel)
{
	data = clothData;
	model = &clothModel;
	const roj::FlatSkeleton& skeleton = model->skeleton;
	if (skeleton.bones == nullptr || skeleton.boneCount == 0)
		return false;

	const uint16_t boneCount = skeleton.boneCount;
	std::vector<glm::mat4> restWorld(boneCount);
	for (uint16_t i = 0; i < boneCount; ++i)
	{
		const uint16_t parent = skeleton.bones[i].parentIdx;
		restWorld[i] = parent != INVALID_BONE_IDX && parent < i ? restWorld[parent] * skeleton.bones[i].localBind
			: skeleton.bones[i].localBind;
	}

	// The vertices at rest, skinned the way the animator draws them, so they sit where the rest-pose bones are.
	std::vector<int> skinToBone(MAX_SKINNED_BONES, -1);
	std::vector<glm::mat4> restSkin(boneCount);
	for (uint16_t i = 0; i < boneCount; ++i)
	{
		const int16_t skin = skeleton.bones[i].skinIdx;
		if (skin >= 0 && skin < MAX_SKINNED_BONES)
			skinToBone[static_cast<size_t>(skin)] = i;
		restSkin[i] = restWorld[i] * skeleton.bones[i].invBind;
	}

	std::vector<RestVertex> vertices;
	for (const roj::SkinnedMesh& mesh : model->meshes)
	{
		for (const VertexData& vertex : mesh.vertices)
		{
			RestVertex rest;
			glm::vec4 position(0.0f);
			float total = 0.0f;
			float strongest = 0.0f;
			for (int k = 0; k < MAX_BONE_INFLUENCE; ++k)
			{
				const float weight = vertex.BlendWeights[k];
				const int skin = static_cast<int>(vertex.BlendIndices[k]);
				if (weight <= 0.0f || skin < 0 || skin >= MAX_SKINNED_BONES || skinToBone[static_cast<size_t>(skin)] < 0)
					continue;
				const uint16_t bone = static_cast<uint16_t>(skinToBone[static_cast<size_t>(skin)]);
				rest.bones[k] = bone;
				rest.weights[k] = weight;
				position += restSkin[bone] * glm::vec4(vertex.Position, 1.0f) * weight;
				total += weight;
				if (weight > strongest)
				{
					strongest = weight;
					rest.dominant = bone;
				}
			}
			if (total <= 0.0f)
				continue;
			rest.position = glm::vec3(position) / total;
			vertices.push_back(rest);
		}
	}
	if (vertices.empty())
		return false;

	if (!BuildStrips(restWorld, vertices))
		return false;
	BuildColliders(restWorld, vertices);
	BuildProbes(restWorld, vertices);
	BuildWindLayer(data.openClip, openDifference, &Strip::opens);
	BuildWindLayer(data.volumeClip, volumeDifference, &Strip::flares);

	clothBones.clear();
	for (const Strip& strip : strips)
		for (const Joint& joint : strip.joints)
			clothBones.push_back(joint.bone);
	const std::string* clipNames[ClipCount] = { &data.windFront, &data.windLeft, &data.windBack, &data.windRight,
		&data.turnLeft, &data.turnRight };
	hasClips = false;
	for (int slot = 0; slot < ClipCount; ++slot)
	{
		BuildClip(*clipNames[slot], clips[static_cast<size_t>(slot)]);
		hasClips = hasClips || !clips[static_cast<size_t>(slot)].samples.empty();
	}
	blendRotation.resize(clothBones.size());
	blendTranslation.resize(clothBones.size());
	blendScale.resize(clothBones.size());
	clipChange.boneTreeRoot = &model->defaultRoot;

	std::vector<bool> carriesRoot(boneCount, false);
	for (uint16_t root : roots)
		for (uint16_t bone = skeleton.bones[root].parentIdx; bone != INVALID_BONE_IDX; bone = skeleton.bones[bone].parentIdx)
			carriesRoot[bone] = true;
	std::vector<int> spinSlot(boneCount, -1);
	spinBones.clear();
	spinParents.clear();
	for (uint16_t i = 0; i < boneCount; ++i)
	{
		if (!carriesRoot[i])
			continue;
		const uint16_t parent = skeleton.bones[i].parentIdx;
		spinSlot[i] = static_cast<int>(spinBones.size());
		spinBones.push_back(i);
		spinParents.push_back(parent != INVALID_BONE_IDX && parent < i ? spinSlot[parent] : -1);
	}
	spinTurns.resize(spinBones.size());
	previousSpinTurns.resize(spinBones.size());

	modelSpace.resize(boneCount);
	colliderStart.resize(colliders.size());
	colliderEnd.resize(colliders.size());
	needsReset = true;
	built = true;
	return true;
}

bool BoneCloth::BuildStrips(const std::vector<glm::mat4>& restWorld, const std::vector<RestVertex>& vertices)
{
	const roj::FlatSkeleton& skeleton = model->skeleton;

	for (const std::string& name : data.roots)
	{
		const uint16_t bone = skeleton.findBone(hashed_string(name));
		if (bone == INVALID_BONE_IDX)
		{
			Logger::Warning("BoneCloth: the model has no root bone '%s'", name.c_str());
			return false;
		}
		roots.push_back(bone);
	}

	std::vector<std::vector<uint16_t>> children(skeleton.boneCount);
	for (uint16_t i = 0; i < skeleton.boneCount; ++i)
		if (skeleton.bones[i].parentIdx != INVALID_BONE_IDX)
			children[skeleton.bones[i].parentIdx].push_back(i);

	std::vector<int> skinned(skeleton.boneCount, 0);
	for (const RestVertex& vertex : vertices)
		for (int k = 0; k < MAX_BONE_INFLUENCE; ++k)
			if (vertex.weights[k] >= SkinnedWeight)
				++skinned[vertex.bones[k]];

	// A chain runs down from each child of a root that carries geometry, through the child carrying the most.
	std::vector<std::vector<uint16_t>> chains;
	for (uint16_t root : roots)
	{
		for (uint16_t top : children[root])
		{
			if (skinned[top] == 0)
				continue;
			std::vector<uint16_t> chain;
			for (uint16_t bone = top; bone != INVALID_BONE_IDX;)
			{
				chain.push_back(bone);
				uint16_t next = INVALID_BONE_IDX;
				int most = 0;
				for (uint16_t child : children[bone])
				{
					if (skinned[child] > most)
					{
						most = skinned[child];
						next = child;
					}
				}
				bone = next;
			}
			chains.push_back(std::move(chain));
		}
	}

	// A chain whose last bone points straight at another chain's top (along its local +Y - Blender bones point that
	// way) continues into it.
	for (bool joined = true; joined;)
	{
		joined = false;
		for (size_t a = 0; a < chains.size() && !joined; ++a)
		{
			const glm::mat4 toLast = glm::inverse(restWorld[chains[a].back()]);
			for (size_t b = 0; b < chains.size() && !joined; ++b)
			{
				const glm::vec3 top(toLast * restWorld[chains[b].front()][3]);
				if (a == b || top.y <= 0.0f || glm::length(glm::vec2(top.x, top.z)) > JoinTolerance * top.y)
					continue;
				chains[a].insert(chains[a].end(), chains[b].begin(), chains[b].end());
				chains.erase(chains.begin() + static_cast<std::ptrdiff_t>(b));
				joined = true;
			}
		}
	}
	if (chains.empty())
	{
		Logger::Warning("BoneCloth (%s): no bone under the roots has vertices skinned to it", Label());
		return false;
	}

	const glm::vec3 forward = glm::length(data.forward) > 1e-6f ? glm::normalize(data.forward) : glm::vec3(0.0f, 0.0f, 1.0f);
	const glm::vec3 left = glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), forward);
	leftAxis = glm::length(left) > 1e-6f ? glm::normalize(left) : glm::vec3(1.0f, 0.0f, 0.0f);
	glm::vec3 centre(0.0f);
	for (const auto& chain : chains)
		centre += glm::vec3(restWorld[chain.front()][3]) / static_cast<float>(chains.size());

	for (const auto& chain : chains)
	{
		Strip strip;
		const size_t count = chain.size();
		for (size_t k = 0; k < count; ++k)
		{
			Joint joint;
			joint.bone = chain[k];
			joint.above = k > 0 ? chain[k - 1] : skeleton.bones[chain[k]].parentIdx;
			const float along = count > 1 ? static_cast<float>(k) / static_cast<float>(count - 1) : 0.0f;
			joint.follow = std::max(glm::mix(data.followRoot, data.followTip, along), 0.0f);
			joint.maxAngle = glm::radians(glm::clamp(glm::mix(data.maxAngleRoot, data.maxAngleTip, along), 0.0f, 180.0f));
			if (k + 1 < count)
				joint.tipLocal = glm::vec3(glm::inverse(restWorld[chain[k]]) * restWorld[chain[k + 1]][3]);
			strip.joints.push_back(joint);
		}

		// The end: along the way the strip runs, as far as the last bone's vertices reach.
		Joint& last = strip.joints.back();
		glm::vec3 axisLocal(0.0f, 1.0f, 0.0f);
		if (count > 1 && glm::length(strip.joints[count - 2].tipLocal) > 1e-6f)
			axisLocal = glm::normalize(strip.joints[count - 2].tipLocal);   // the bone above's axis, in its frame
		glm::vec3 axis = glm::mat3(restWorld[last.bone]) * axisLocal;
		const float unit = glm::length(axis);
		if (unit < 1e-8f)
		{
			Logger::Warning("BoneCloth: bone '%s' has no size", skeleton.bones[last.bone].name.str().c_str());
			return false;
		}
		axis /= unit;
		const glm::vec3 head(restWorld[last.bone][3]);
		float length = data.tipLength;
		if (length <= 0.0f)
		{
			for (const RestVertex& vertex : vertices)
				for (int k = 0; k < MAX_BONE_INFLUENCE; ++k)
					if (vertex.bones[k] == last.bone && vertex.weights[k] >= TipWeight)
						length = std::max(length, glm::dot(vertex.position - head, axis));
			if (length < 0.01f)
				length = count > 1 ? glm::distance(head, glm::vec3(restWorld[chain[count - 2]][3])) : 0.1f;
		}
		last.tipLocal = axisLocal * (length / unit);

		const float sideways = glm::dot(glm::vec3(restWorld[chain.front()][3]) - centre, leftAxis);
		strip.side = sideways > SideTolerance ? 1 : (sideways < -SideTolerance ? -1 : 0);
		strips.push_back(std::move(strip));
	}
	return true;
}

void BoneCloth::BuildColliders(const std::vector<glm::mat4>& restWorld, const std::vector<RestVertex>& vertices)
{
	const roj::FlatSkeleton& skeleton = model->skeleton;

	// A fitted radius uses the limb between `bone` and `to`: the bones on the way from one to the other (`to` not
	// included) and the leaves hanging off them (twist bones) - not other branches.
	std::vector<bool> hasChildren(skeleton.boneCount, false);
	for (uint16_t i = 0; i < skeleton.boneCount; ++i)
		if (skeleton.bones[i].parentIdx != INVALID_BONE_IDX)
			hasChildren[skeleton.bones[i].parentIdx] = true;
	auto onTheWay = [&](uint16_t candidate, uint16_t bone, uint16_t to)
	{
		for (uint16_t current = skeleton.bones[to].parentIdx; current != INVALID_BONE_IDX; current = skeleton.bones[current].parentIdx)
		{
			if (current == candidate)
				return true;
			if (current == bone)
				break;
		}
		return false;
	};

	for (const BoneClothCollider& config : data.colliders)
	{
		Collider collider;
		collider.bone = skeleton.findBone(hashed_string(config.bone));
		collider.to = skeleton.findBone(hashed_string(config.to));
		if (collider.bone == INVALID_BONE_IDX || collider.to == INVALID_BONE_IDX)
		{
			Logger::Warning("BoneCloth (%s): collider %s-%s names a bone the model lacks", Label(), config.bone.c_str(),
				config.to.c_str());
			continue;
		}
		collider.radius = config.radius;
		collider.endRadius = config.endRadius;
		collider.surface = config.surface;
		const glm::vec3 start = glm::vec3(restWorld[collider.bone][3]) + config.offset;
		const glm::vec3 end = glm::vec3(restWorld[collider.to][3]) + config.offset;
		collider.startLocal = glm::vec3(glm::inverse(restWorld[collider.bone]) * glm::vec4(start, 1.0f));
		collider.endLocal = glm::vec3(glm::inverse(restWorld[collider.to]) * glm::vec4(end, 1.0f));

		if (config.radius <= 0.0f || config.endRadius <= 0.0f)
		{
			std::vector<float> nearStart, nearEnd, all;
			for (const RestVertex& vertex : vertices)
			{
				const uint16_t dominant = vertex.dominant;
				if (dominant == INVALID_BONE_IDX)
					continue;
				const uint16_t parent = skeleton.bones[dominant].parentIdx;
				if (!onTheWay(dominant, collider.bone, collider.to)
					&& (hasChildren[dominant] || parent == INVALID_BONE_IDX || !onTheWay(parent, collider.bone, collider.to)))
					continue;
				const float t = AxisParameter(vertex.position, start, end);
				const float distance = glm::distance(vertex.position, glm::mix(start, end, t));
				all.push_back(distance);
				if (t <= 0.3f)
					nearStart.push_back(distance);
				if (t >= 0.7f)
					nearEnd.push_back(distance);
			}
			const float fallback = all.empty() ? 0.06f : Median(all);
			if (config.radius <= 0.0f)
				collider.radius = nearStart.empty() ? fallback : Median(nearStart);
			if (config.endRadius <= 0.0f)
				collider.endRadius = nearEnd.empty() ? fallback : Median(nearEnd);
			Logger::Log("BoneCloth (%s): collider %s-%s fitted to radius %.3f..%.3f", Label(), config.bone.c_str(),
				config.to.c_str(), collider.radius, collider.endRadius);
		}
		colliders.push_back(collider);
	}
}

void BoneCloth::BuildProbes(const std::vector<glm::mat4>& restWorld, const std::vector<RestVertex>& vertices)
{
	// Each joint's probes: the vertices it carries most of, spread out - the farthest from its head first, then each
	// next the farthest from those taken. A probe that rests inside a surface collider is left alone by that collider,
	// so the cloth keeps the shape it was made with.
	bool anySurface = false;
	for (size_t c = 0; c < colliders.size(); ++c)
	{
		if (!colliders[c].surface)
			continue;
		if (c >= MaxSurfaceColliders)
		{
			Logger::Warning("BoneCloth (%s): only the first %u colliders can be surface colliders", Label(), MaxSurfaceColliders);
			colliders[c].surface = false;
			continue;
		}
		anySurface = true;
	}
	if (!anySurface)
		return;

	std::vector<glm::vec3> candidates;
	std::vector<float> nearest;
	for (Strip& strip : strips)
	{
		for (Joint& joint : strip.joints)
		{
			candidates.clear();
			for (const RestVertex& vertex : vertices)
				for (int k = 0; k < MAX_BONE_INFLUENCE; ++k)
					if (vertex.bones[k] == joint.bone && vertex.weights[k] >= ProbeWeight)
						candidates.push_back(vertex.position);
			const glm::mat4 toBone = glm::inverse(restWorld[joint.bone]);
			const glm::vec3 head(restWorld[joint.bone][3]);
			nearest.assign(candidates.size(), std::numeric_limits<float>::max());
			for (size_t taken = 0; taken < MaxProbes && taken < candidates.size(); ++taken)
			{
				size_t best = 0;
				float farthest = -1.0f;
				for (size_t i = 0; i < candidates.size(); ++i)
				{
					const float distance = taken == 0 ? glm::distance(candidates[i], head) : nearest[i];
					if (distance > farthest)
					{
						farthest = distance;
						best = i;
					}
				}
				if (taken > 0 && farthest < 1e-4f)
					break;   // the rest sit on the probes taken
				const glm::vec3 position = candidates[best];
				for (size_t i = 0; i < candidates.size(); ++i)
					nearest[i] = std::min(nearest[i], glm::distance(candidates[i], position));

				Probe probe;
				probe.local = glm::vec3(toBone * glm::vec4(position, 1.0f));
				for (size_t c = 0; c < colliders.size(); ++c)
				{
					if (!colliders[c].surface)
						continue;
					const glm::vec3 start(restWorld[colliders[c].bone] * glm::vec4(colliders[c].startLocal, 1.0f));
					const glm::vec3 end(restWorld[colliders[c].to] * glm::vec4(colliders[c].endLocal, 1.0f));
					const float t = AxisParameter(position, start, end);
					if (glm::distance(position, glm::mix(start, end, t)) < glm::mix(colliders[c].radius, colliders[c].endRadius, t))
						probe.ignore |= 1u << c;
				}
				joint.probes.push_back(probe);
			}
		}
	}
}

void BoneCloth::BuildClip(const std::string& clip, ClipSamples& out)
{
	// The clip's change of each strip bone from its first frame (Local, like the wind layers), sampled evenly.
	out.samples.clear();
	if (clip.empty())
		return;
	const hashed_string key(clip);
	auto found = model->clips.find(key);
	if (found == model->clips.end() || model->animations.find(key) == model->animations.end())
	{
		Logger::Warning("BoneCloth (%s): the model has no clip '%s'; the strips keep their rest shape there", Label(),
			clip.c_str());
		return;
	}

	const roj::FlatSkeleton& skeleton = model->skeleton;
	roj::Animator sampler(model);
	sampler.set(key);
	AnimationPose first, sample, difference;
	first.boneTreeRoot = sample.boneTreeRoot = &model->defaultRoot;
	out.samples.assign(ClipSampleCount, std::vector<BoneChange>(clothBones.size()));
	for (int s = 0; s < ClipSampleCount; ++s)
	{
		sampler.m_currTime = found->second->duration * static_cast<float>(s) / static_cast<float>(ClipSampleCount - 1);
		sampler.UpdateAnimationPose();
		if (s == 0)
		{
			first.boneTransforms = sampler.GetBonePoseArray();
			continue;   // no change at the start
		}
		sample.boneTransforms = sampler.GetBonePoseArray();
		AnimationPose::MakeAdditive(difference, &model->defaultRoot, first, sample, AnimationPose::AdditiveSpace::Local);
		for (size_t b = 0; b < clothBones.size(); ++b)
		{
			auto change = difference.boneTransforms.find(skeleton.bones[clothBones[b]].name);
			if (change == difference.boneTransforms.end())
				continue;
			const AnimationPose::BoneTRS trs = AnimationPose::DecomposeBone(change->second);
			BoneChange& to = out.samples[static_cast<size_t>(s)][b];
			to.rotation = trs.rotation;
			to.translation = trs.translation;
			to.scale = trs.scale;
		}
	}
}

void BoneCloth::BuildWindLayer(const std::string& clip, AnimationPose& difference, bool Strip::* uses)
{
	difference.boneTransforms.clear();
	if (clip.empty())
		return;

	AnimationPose first, last;
	if (!SampleClip(*model, clip, first, last))
	{
		Logger::Warning("BoneCloth (%s): the model has no clip '%s'; that wind layer is off", Label(), clip.c_str());
		return;
	}
	AnimationPose::MakeAdditive(difference, &model->defaultRoot, first, last, AnimationPose::AdditiveSpace::Local);

	// A strip uses the layer when the clip turns any of its bones.
	const roj::FlatSkeleton& skeleton = model->skeleton;
	for (Strip& strip : strips)
	{
		for (const Joint& joint : strip.joints)
		{
			auto change = difference.boneTransforms.find(skeleton.bones[joint.bone].name);
			if (change == difference.boneTransforms.end())
				continue;
			const glm::quat turn = glm::quat_cast(glm::mat3(change->second));
			if (2.0f * std::acos(std::min(std::abs(turn.w), 1.0f)) >= glm::radians(0.5f))
				strip.*uses = true;
		}
	}
}

// ---- Step -----------------------------------------------------------------------------------------------------

void BoneCloth::Step(AnimationPose& pose, const glm::mat4& world, float deltaTime)
{
	if (!built)
		return;

	deltaTime = glm::clamp(deltaTime, 0.0f, MaxDeltaTime);
	const glm::vec3 origin(world[3]);
	if (needsReset)
		previousOrigin = origin;
	const bool snap = needsReset || glm::distance(origin, previousOrigin) > data.teleportDistance;

	UpdateWindWeights(pose, world, deltaTime);
	RestShape(pose);
	ComputeModelSpace(pose);
	Swing(world, deltaTime, snap);
	WriteBones(pose, world);
	if (DebugDrawEnabled)
		DrawDebug(world);

	previousOrigin = origin;
	needsReset = false;
}

float BoneCloth::SpinRate(const AnimationPose& pose, const glm::mat4& world, float deltaTime)
{
	// The fastest turn about the world's up, radians/s (> 0: turning left), of the body bones the roots hang from and
	// their ancestors - the character's turn and the animation's pelvis and spine together, so a pelvis twist the spine
	// counters still counts. Bones the pose lacks are at rest.
	const roj::FlatSkeleton& skeleton = model->skeleton;
	float fastest = 0.0f;
	for (size_t i = 0; i < spinBones.size(); ++i)
	{
		const uint16_t bone = spinBones[i];
		auto it = pose.boneTransforms.find(skeleton.bones[bone].name);
		const glm::mat3 local(it != pose.boneTransforms.end() ? it->second : skeleton.bones[bone].localBind);
		spinTurns[i] = (spinParents[i] >= 0 ? spinTurns[static_cast<size_t>(spinParents[i])] : glm::mat3(world)) * local;

		glm::mat3 turn = spinTurns[i];
		for (int axis = 0; axis < 3; ++axis)
			turn[axis] = glm::length(turn[axis]) > 1e-8f ? glm::normalize(turn[axis]) : turn[axis];
		const glm::quat now = glm::normalize(glm::quat_cast(turn));
		if (!needsReset && deltaTime > 0.0f)
		{
			glm::quat change = now * glm::inverse(previousSpinTurns[i]);
			if (change.w < 0.0f)
				change = -change;
			const float angle = 2.0f * std::acos(std::min(change.w, 1.0f));
			const glm::vec3 axis(change.x, change.y, change.z);
			const float axisLength = glm::length(axis);
			const float rate = axisLength > 1e-6f ? angle * axis.y / axisLength / deltaTime : 0.0f;
			if (std::abs(rate) > std::abs(fastest))
				fastest = rate;
		}
		previousSpinTurns[i] = now;
	}
	return fastest;
}

void BoneCloth::UpdateWindWeights(const AnimationPose& pose, const glm::mat4& world, float deltaTime)
{
	const bool layers = !openDifference.boneTransforms.empty() || !volumeDifference.boneTransforms.empty();
	if (!layers && !hasClips)
		return;

	const float blend = deltaTime > 0.0f ? 1.0f - std::exp(-deltaTime / std::max(data.layerSmoothing, 1e-3f)) : 0.0f;

	// Spin: the body's turn rate since the last frame. It rises like the wind and settles slower, so a quick turn leaves
	// the coat open a moment after it. The turn clips keep its direction.
	const bool turnClips = !clips[TurnLeft].samples.empty() || !clips[TurnRight].samples.empty();
	if (data.spinOpen > 0.0f || data.spinVolume > 0.0f || turnClips)
	{
		const float rate = SpinRate(pose, world, deltaTime);
		if (needsReset)
		{
			spinWeight = 0.0f;
			turnAmount = 0.0f;
		}
		else if (deltaTime > 0.0f)
		{
			const float wanted = Ramp(glm::radians(data.spinStart), glm::radians(data.spinFull), std::abs(rate));
			const float release = 1.0f - std::exp(-deltaTime / std::max(data.spinRelease, 1e-3f));
			spinWeight += (wanted - spinWeight) * (wanted > spinWeight ? blend : release);
			const float turn = rate >= 0.0f ? wanted : -wanted;
			const bool growing = std::abs(turn) > std::abs(turnAmount) && turn * turnAmount >= 0.0f;
			turnAmount += (turn - turnAmount) * (growing ? blend : release);
		}
	}

	// The apparent wind in model space: the world's wind less the character's own velocity.
	const glm::vec3 origin(world[3]);
	const glm::vec3 bodyVelocity = deltaTime > 0.0f ? (origin - previousOrigin) / deltaTime : glm::vec3(0.0f);
	glm::mat3 facing(world);
	for (int axis = 0; axis < 3; ++axis)
		facing[axis] = glm::length(facing[axis]) > 1e-8f ? glm::normalize(facing[axis]) : facing[axis];
	const glm::vec3 wind = glm::transpose(facing) * (WindField::Sample(origin, Time::GameTime) - bodyVelocity);
	const glm::vec3 forward = glm::length(data.forward) > 1e-6f ? glm::normalize(data.forward) : glm::vec3(0.0f, 0.0f, 1.0f);
	const float headwind = std::max(0.0f, -glm::dot(wind, forward));
	const float leftward = glm::dot(wind, leftAxis);   // > 0: the air moves to the left, so it comes from the right

	if (hasClips)
	{
		const glm::vec2 air(glm::dot(wind, forward), leftward);
		windAir = needsReset ? air : windAir + (air - windAir) * blend;
		windSpeed = needsReset ? glm::length(air) : windSpeed + (glm::length(air) - windSpeed) * blend;
	}
	if (!layers)
		return;

	volumeWeight += (Ramp(data.volumeWindStart, data.volumeWindFull, glm::length(wind)) - volumeWeight) * blend;
	for (Strip& strip : strips)
	{
		const float windward = std::max(0.0f, -static_cast<float>(strip.side) * leftward);
		const float wanted = Ramp(data.openWindStart, data.openWindFull, headwind + data.windwardBias * windward);
		strip.openWeight += (wanted - strip.openWeight) * blend;
	}
}

void BoneCloth::AddClip(ClipSlot slot, float position, float weight)
{
	// The clip's change at `position` (0..1 through it), between the two nearest samples, added by `weight`. No clip:
	// no change (the rest shape).
	if (weight <= 0.0f)
		return;
	const ClipSamples& clip = clips[static_cast<size_t>(slot)];
	if (clip.samples.empty())
	{
		for (size_t b = 0; b < clothBones.size(); ++b)
		{
			blendRotation[b].w += weight;
			blendScale[b] += glm::vec3(weight);
		}
		return;
	}
	const float at = glm::clamp(position, 0.0f, 1.0f) * static_cast<float>(clip.samples.size() - 1);
	const size_t lower = std::min(static_cast<size_t>(at), clip.samples.size() - 2);
	const float upper = at - static_cast<float>(lower);
	for (size_t side = 0; side < 2; ++side)
	{
		const float share = weight * (side == 0 ? 1.0f - upper : upper);
		if (share <= 0.0f)
			continue;
		const std::vector<BoneChange>& changes = clip.samples[lower + side];
		for (size_t b = 0; b < clothBones.size(); ++b)
		{
			glm::quat rotation = changes[b].rotation;
			if (rotation.w < 0.0f)
				rotation = -rotation;   // the short way round, like every other one added
			blendRotation[b] += glm::vec4(rotation.x, rotation.y, rotation.z, rotation.w) * share;
			blendTranslation[b] += changes[b].translation * share;
			blendScale[b] += changes[b].scale * share;
		}
	}
}

void BoneCloth::RestShape(AnimationPose& pose)
{
	// The roots and the strips keep their rest transforms on the body (the animation's keys on them are not used);
	// the wind and turn clips, then the wind layers turn the strips, on each run of a strip within one branch of the
	// skeleton (a joined strip hangs from two).
	const roj::FlatSkeleton& skeleton = model->skeleton;
	for (uint16_t root : roots)
		pose.boneTransforms.insert_or_assign(skeleton.bones[root].name, skeleton.bones[root].localBind);
	for (const Strip& strip : strips)
		for (const Joint& joint : strip.joints)
			pose.boneTransforms.insert_or_assign(skeleton.bones[joint.bone].name, skeleton.bones[joint.bone].localBind);

	roj::BoneNode* rootNode = &model->defaultRoot;
	auto applyToRuns = [&](const AnimationPose& difference, auto weightOf)
	{
		for (const Strip& strip : strips)
		{
			const float weight = weightOf(strip);
			if (weight <= 0.0f)
				continue;
			for (size_t k = 0; k < strip.joints.size(); ++k)
			{
				if (k > 0 && skeleton.bones[strip.joints[k].bone].parentIdx == strip.joints[k - 1].bone)
					continue;
				AnimationPose::ApplyAdditiveInPlace(skeleton.bones[strip.joints[k].bone].name, rootNode, pose, difference,
					AnimationPose::AdditiveSpace::Local, weight);
			}
		}
	};

	// The clips in two layers: the wind's - the two sides nearest to where it comes from, as far through as the wind is
	// strong - and the turn's on top of it, so a turn in the wind shows both. A spin opens the front: as the body spins
	// the wind's side matters less (it sweeps round the body faster than cloth can follow) and its speed, with spinOpen
	// of windFull at a full spin, counts as headwind instead.
	auto applyBlend = [&](auto addClips)
	{
		std::fill(blendRotation.begin(), blendRotation.end(), glm::vec4(0.0f));
		std::fill(blendTranslation.begin(), blendTranslation.end(), glm::vec3(0.0f));
		std::fill(blendScale.begin(), blendScale.end(), glm::vec3(0.0f));
		addClips();
		for (size_t b = 0; b < clothBones.size(); ++b)
		{
			const glm::vec4& sum = blendRotation[b];
			AnimationPose::BoneTRS change;   // the shares add up to 1
			if (glm::length(sum) > 1e-6f)
				change.rotation = glm::normalize(glm::quat(sum.w, sum.x, sum.y, sum.z));
			change.translation = blendTranslation[b];
			change.scale = blendScale[b];
			clipChange.boneTransforms.insert_or_assign(skeleton.bones[clothBones[b]].name, AnimationPose::ComposeBone(change));
		}
		applyToRuns(clipChange, [](const Strip&) { return 1.0f; });
	};
	const glm::vec2 air = windAir * (1.0f - spinWeight)
		- glm::vec2(spinWeight * (windSpeed + data.spinOpen * data.windFull), 0.0f);
	const float windPosition = hasClips ? Ramp(data.windStart, data.windFull, glm::length(air)) : 0.0f;
	if (windPosition > 0.0f)
	{
		applyBlend([&]
		{
			// Where the wind comes from, in quarter turns from the front toward the left (the slot order).
			float around = std::atan2(-air.y, -air.x) / glm::half_pi<float>();
			if (around < 0.0f)
				around += 4.0f;
			const int side = std::min(static_cast<int>(around), 3);
			const float toNext = around - static_cast<float>(side);
			const float strength[4] = { data.windStrengthFront, data.windStrengthSides, data.windStrengthBack,
				data.windStrengthSides };   // the slot order: front, left, back, right
			AddClip(static_cast<ClipSlot>(side), windPosition * strength[side], 1.0f - toNext);
			AddClip(static_cast<ClipSlot>((side + 1) % 4), windPosition * strength[(side + 1) % 4], toNext);
		});
	}
	const float turnPosition = hasClips ? std::abs(turnAmount) : 0.0f;
	if (turnPosition > 1e-4f)
		applyBlend([&] { AddClip(turnAmount > 0.0f ? TurnLeft : TurnRight, turnPosition * data.spinStrength, 1.0f); });

	if (!volumeDifference.boneTransforms.empty())
		applyToRuns(volumeDifference, [&](const Strip& strip)
			{ return strip.flares ? data.volume + volumeWeight + data.spinVolume * spinWeight : 0.0f; });
	if (!openDifference.boneTransforms.empty())
		applyToRuns(openDifference, [&](const Strip& strip)
			{ return strip.opens ? strip.openWeight + data.spinOpen * spinWeight : 0.0f; });
}

void BoneCloth::ComputeModelSpace(const AnimationPose& pose)
{
	// Like the animator: bones the pose lacks keep their rest transform.
	const roj::FlatSkeleton& skeleton = model->skeleton;
	for (uint16_t i = 0; i < skeleton.boneCount; ++i)
	{
		auto it = pose.boneTransforms.find(skeleton.bones[i].name);
		const glm::mat4& local = it != pose.boneTransforms.end() ? it->second : skeleton.bones[i].localBind;
		const uint16_t parent = skeleton.bones[i].parentIdx;
		modelSpace[i] = parent != INVALID_BONE_IDX && parent < i ? modelSpace[parent] * local : local;
	}
}

void BoneCloth::Swing(const glm::mat4& world, float deltaTime, bool snap)
{
	// Each tail eases toward where the bone would hang - its rest direction on the joint above as that joint hangs now -
	// a share of the way each frame (faster near the top), so the strip trails the body and catches up. No speed is
	// kept: nothing overshoots. The tails first move with the character but for the share they trail (all of it when
	// no time passes, so in hitstop the strips keep their shape on the body).
	const glm::vec3 moved = glm::vec3(world[3]) - previousOrigin;
	const glm::vec3 carry = moved * (deltaTime > 0.0f ? 1.0f - glm::clamp(data.trail, 0.0f, 1.0f) : 1.0f);
	for (size_t c = 0; c < colliders.size(); ++c)
	{
		colliderStart[c] = ColliderPoint(world, colliders[c].bone, colliders[c].startLocal);
		colliderEnd[c] = ColliderPoint(world, colliders[c].to, colliders[c].endLocal);
	}

	for (Strip& strip : strips)
	{
		glm::vec3 head(world * modelSpace[strip.joints.front().bone][3]);
		glm::mat3 aboveRest(1.0f);
		glm::mat3 aboveNow(1.0f);
		for (size_t k = 0; k < strip.joints.size(); ++k)
		{
			Joint& joint = strip.joints[k];
			const glm::mat3 restModel(modelSpace[joint.bone]);
			const glm::mat3 restBasis = k == 0 ? glm::mat3(world) * restModel
				: aboveNow * glm::inverse(aboveRest) * restModel;   // its rest bend on the joint above
			const glm::vec3 restTail = head + restBasis * joint.tipLocal;

			glm::vec3 tail = snap ? restTail
				: glm::mix(joint.tail + carry, restTail, 1.0f - std::exp(-joint.follow * deltaTime));
			tail = Constrain(head, tail, restTail, joint.maxAngle);
			for (size_t c = 0; c < colliders.size(); ++c)
			{
				if (colliders[c].surface)
					continue;
				const float t = AxisParameter(tail, colliderStart[c], colliderEnd[c]);
				const glm::vec3 nearest = glm::mix(colliderStart[c], colliderEnd[c], t);
				const float radius = glm::mix(colliders[c].radius, colliders[c].endRadius, t) + data.radius;
				const glm::vec3 offset = tail - nearest;
				const float distance = glm::length(offset);
				if (distance < radius && distance > 1e-6f)
					tail = nearest + offset * (radius / distance);
			}
			joint.tail = Constrain(head, tail, restTail, joint.maxAngle);

			// The bone turned from its rest direction to the tail.
			const glm::vec3 from = restTail - head;
			const glm::vec3 to = joint.tail - head;
			joint.basis = restBasis;
			if (glm::dot(from, from) > 1e-12f && glm::dot(to, to) > 1e-12f)
				joint.basis = glm::mat3_cast(glm::rotation(glm::normalize(from), glm::normalize(to))) * restBasis;

			// The cloth it carries kept out of the surface colliders: the joint turned about its head, each time just
			// enough to bring its deepest probe to the capsule's surface. The swing limit does not hold this back.
			for (int iteration = 0; iteration < ProbeIterations && !joint.probes.empty(); ++iteration)
			{
				float deepest = 0.0f;
				glm::vec3 probeFrom(0.0f), probeTo(0.0f);
				for (const Probe& probe : joint.probes)
				{
					const glm::vec3 point = head + joint.basis * probe.local;
					for (size_t c = 0; c < colliders.size(); ++c)
					{
						if (!colliders[c].surface || (probe.ignore & (1u << c)) != 0)
							continue;
						const float t = AxisParameter(point, colliderStart[c], colliderEnd[c]);
						const glm::vec3 nearest = glm::mix(colliderStart[c], colliderEnd[c], t);
						const float radius = glm::mix(colliders[c].radius, colliders[c].endRadius, t);
						const glm::vec3 offset = point - nearest;
						const float distance = glm::length(offset);
						if (radius - distance > deepest && distance > 1e-6f)
						{
							deepest = radius - distance;
							probeFrom = point - head;
							probeTo = nearest + offset * (radius / distance) - head;
						}
					}
				}
				if (deepest <= 0.0f || glm::dot(probeFrom, probeFrom) < 1e-12f || glm::dot(probeTo, probeTo) < 1e-12f)
					break;
				const glm::mat3 turn = glm::mat3_cast(glm::rotation(glm::normalize(probeFrom), glm::normalize(probeTo)));
				joint.basis = turn * joint.basis;
				joint.tail = head + turn * (joint.tail - head);
			}
			joint.head = head;

			aboveRest = restModel;
			aboveNow = joint.basis;
			head = joint.tail;
		}
	}
}

glm::vec3 BoneCloth::ColliderPoint(const glm::mat4& world, uint16_t bone, const glm::vec3& local) const
{
	return glm::vec3(world * (modelSpace[bone] * glm::vec4(local, 1.0f)));
}

void BoneCloth::WriteBones(AnimationPose& pose, const glm::mat4& world)
{
	// Each joint's swung transform, made local to its skeleton parent (itself swung first when it is the joint above).
	const roj::FlatSkeleton& skeleton = model->skeleton;
	const glm::mat4 toModel = glm::inverse(world);
	for (const Strip& strip : strips)
	{
		for (const Joint& joint : strip.joints)
		{
			glm::mat4 transform(joint.basis);
			transform[3] = glm::vec4(joint.head, 1.0f);
			transform = toModel * transform;
			const uint16_t parent = skeleton.bones[joint.bone].parentIdx;
			const glm::mat4 parentModel = parent != INVALID_BONE_IDX ? modelSpace[parent] : glm::mat4(1.0f);
			pose.boneTransforms.insert_or_assign(skeleton.bones[joint.bone].name, glm::inverse(parentModel) * transform);
			modelSpace[joint.bone] = transform;
		}
	}
}

void BoneCloth::DrawDebug(const glm::mat4& world) const
{
	for (const Strip& strip : strips)
	{
		for (const Joint& joint : strip.joints)
		{
			DebugDraw::Line(joint.head, joint.tail, DebugLineTime, 0.01f, DebugColor::Yellow);
			for (const Probe& probe : joint.probes)
				DebugDraw::Point(joint.head + joint.basis * probe.local, DebugLineTime, 0.008f, DebugColor::Cyan);
		}
	}

	for (const Collider& collider : colliders)
	{
		const glm::vec3 start = ColliderPoint(world, collider.bone, collider.startLocal);
		const glm::vec3 end = ColliderPoint(world, collider.to, collider.endLocal);
		const uint32_t color = collider.surface ? DebugColor::Cyan : DebugColor::Orange;
		const glm::vec3 axis = end - start;
		const glm::vec3 direction = glm::length(axis) > 1e-6f ? glm::normalize(axis) : glm::vec3(0.0f, 1.0f, 0.0f);
		const glm::vec3 u = Perpendicular(direction);
		const glm::vec3 v = glm::cross(direction, u);
		DebugDraw::Line(start, end, DebugLineTime, 0.01f, color);
		constexpr int Segments = 8;
		for (int s = 0; s < Segments; ++s)
		{
			const float angle0 = glm::two_pi<float>() * static_cast<float>(s) / Segments;
			const float angle1 = glm::two_pi<float>() * static_cast<float>(s + 1) / Segments;
			const glm::vec3 ring0 = u * std::cos(angle0) + v * std::sin(angle0);
			const glm::vec3 ring1 = u * std::cos(angle1) + v * std::sin(angle1);
			DebugDraw::Line(start + ring0 * collider.radius, start + ring1 * collider.radius, DebugLineTime, 0.01f, color);
			DebugDraw::Line(end + ring0 * collider.endRadius, end + ring1 * collider.endRadius, DebugLineTime, 0.01f, color);
			DebugDraw::Line(start + ring0 * collider.radius, end + ring0 * collider.endRadius, DebugLineTime, 0.01f, color);
		}
	}
}

CONSOLE_FUNC("cloth_debug", "cloth_debug [0|1] - draws bone cloth strips (yellow), colliders (orange), surface colliders and probes (cyan)")
{
	BoneCloth::DebugDrawEnabled = args.empty() ? !BoneCloth::DebugDrawEnabled : Console::ArgBool(args, 0);
}

CONSOLE_FUNC("cloth_reload", "cloth_reload - re-reads the bone cloth settings of every skeletal mesh's .skmm")
{
	SkeletalMesh::ReloadClothData();
}
