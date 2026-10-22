#include "CharacterAnimator.h"

#include <Logger.hpp>

#include <algorithm>
#include <cmath>

namespace
{
	constexpr float AimBlendTime = 0.15f;          // s for the aim to fade in / out
	constexpr float StrafeMinSpeed = 0.3f;         // m/s below which the legs keep their direction
	constexpr float StrafeHysteresis = 20.0f;      // deg around 90 between running forward and backward

	const glm::vec3 Up(0.0f, 1.0f, 0.0f);
	const hashed_string RootBone("root");

	float Approach(float value, float target, float rate, float deltaTime)
	{
		const float step = rate * deltaTime;
		if (value < target) return std::min(target, value + step);
		return std::max(target, value - step);
	}

	float LoopTime(float time, float duration)
	{
		return duration > 0.0f ? std::fmod(time, duration) : 0.0f;
	}

	float FadeOut(float elapsed, float duration)
	{
		return duration > 0.0f ? 1.0f - glm::smoothstep(0.0f, 1.0f, elapsed / duration) : 0.0f;
	}

	float WrapAngle(float radians)
	{
		return std::remainder(radians, glm::two_pi<float>());
	}

	// Signed angle about up from `from` to `to` (horizontal model-space directions): the rotation angleAxis(angle, Up)
	// turns `from` onto `to`.
	float YawBetween(const glm::vec3& from, const glm::vec3& to)
	{
		return std::atan2(glm::dot(Up, glm::cross(from, to)), glm::dot(from, to));
	}

	glm::vec3 FlatDirection(const glm::vec3& v)
	{
		const glm::vec3 flat(v.x, 0.0f, v.z);
		const float length = glm::length(flat);
		return length > 1e-4f ? flat / length : glm::vec3(0.0f, 0.0f, 1.0f);
	}

	// Puts the pose's root bone back when it goes out of scope: the engine's partial blends take the root from their
	// layer, but the root belongs to whatever owns the whole body.
	class RootGuard
	{
	public:
		explicit RootGuard(AnimationPose& pose) : pose(pose)
		{
			auto it = pose.boneTransforms.find(RootBone);
			hasRoot = it != pose.boneTransforms.end();
			if (hasRoot)
				root = it->second;
		}
		~RootGuard()
		{
			if (hasRoot)
				pose.boneTransforms[RootBone] = root;
		}
		RootGuard(const RootGuard&) = delete;
		RootGuard& operator=(const RootGuard&) = delete;

	private:
		AnimationPose& pose;
		glm::mat4 root = glm::mat4(1.0f);
		bool hasRoot = false;
	};

	// A clip of a model that lacks some of the body's bones (a weapon model without the coat) must leave them to the
	// pose underneath: Lerp's fast path would drop them and LayeredLerp would collapse them to identity. out = the
	// layer's bones (its root included) plus the bones only base has, written over out's storage; out may be base.
	void CompleteInto(AnimationPose& out, const AnimationPose& layer, const AnimationPose& base)
	{
		auto& bones = out.boneTransforms;
		const bool aliased = &out == &base;
		const size_t baseSize = base.boneTransforms.size();
		if (!aliased)
			for (const auto& [bone, transform] : base.boneTransforms)
				bones.insert_or_assign(bone, transform);

		size_t layerOnly = 0;
		for (const auto& [bone, transform] : layer.boneTransforms)
		{
			const bool inserted = bones.insert_or_assign(bone, transform).second;
			if (aliased ? inserted : base.boneTransforms.find(bone) == base.boneTransforms.end())
				++layerOnly;
		}

		// Bones a previous completion left that neither pose has.
		if (bones.size() != baseSize + layerOnly)
		{
			for (auto it = bones.begin(); it != bones.end();)
			{
				const bool keep = base.boneTransforms.find(it->first) != base.boneTransforms.end()
					|| layer.boneTransforms.find(it->first) != layer.boneTransforms.end();
				it = keep ? std::next(it) : bones.erase(it);
			}
		}
	}

	// to = from, over to's storage.
	void CopyPose(AnimationPose& to, const AnimationPose& from)
	{
		AnimationPose::LerpInPlace(to, from, 1.0f);
	}

	// target = Lerp(target, layer, weight) with the root blended by weight too (Lerp takes the root from its second
	// pose at any weight). `layer` is scratch: its root is changed.
	void LerpWithRoot(AnimationPose& target, AnimationPose& layer, float weight)
	{
		auto targetRoot = target.boneTransforms.find(RootBone);
		auto layerRoot = layer.boneTransforms.find(RootBone);
		if (targetRoot != target.boneTransforms.end() && layerRoot != layer.boneTransforms.end())
			layerRoot->second = MathHelper::Transform::Lerp(MathHelper::DecomposeMatrix(targetRoot->second),
				MathHelper::DecomposeMatrix(layerRoot->second), weight).ToMatrix();
		AnimationPose::LerpInPlace(target, layer, weight);
	}

	// Turns `bone` about its own origin by `rotation` (model space): new local = local * model^-1 * turned.
	void RotateInModelSpace(AnimationPose& pose, roj::BoneNode* rootNode, const hashed_string& bone, const glm::mat3& rotation)
	{
		auto it = pose.boneTransforms.find(bone);
		if (it == pose.boneTransforms.end())
			return;

		const glm::mat4 model = AnimationPose::GetModelSpaceTransform(bone, rootNode, pose);
		const glm::mat3 basis = rotation * glm::mat3(model);
		glm::mat4 turned = model;
		turned[0] = glm::vec4(basis[0], 0.0f);
		turned[1] = glm::vec4(basis[1], 0.0f);
		turned[2] = glm::vec4(basis[2], 0.0f);
		it->second = it->second * glm::inverse(model) * turned;
	}

	AnimationPose::AdditiveSpace ToAdditiveSpace(PoseSpace space)
	{
		return space == PoseSpace::Mesh ? AnimationPose::AdditiveSpace::Mesh : AnimationPose::AdditiveSpace::Local;
	}

	// Adds `difference` to `bone`'s subtree by weight; the root bone keeps its value.
	void AddBranch(AnimationPose& pose, roj::BoneNode* rootNode, const hashed_string& bone, const AnimationPose& difference,
		PoseSpace space, float weight)
	{
		RootGuard root(pose);
		AnimationPose::ApplyAdditiveInPlace(bone, rootNode, pose, difference, ToAdditiveSpace(space), weight);
	}
}

// ---- PoseFade ---------------------------------------------------------------------------------------------

void CharacterAnimator::PoseFade::Advance(float deltaTime)
{
	if (!active)
		return;
	elapsed += deltaTime;
	if (elapsed >= duration)
		active = false;
}

float CharacterAnimator::PoseFade::Progress() const
{
	return duration > 0.0f ? glm::smoothstep(0.0f, 1.0f, elapsed / duration) : 1.0f;
}

// ---- Loading ----------------------------------------------------------------------------------------------

void CharacterAnimator::Configure(std::shared_ptr<const CharacterData> characterData, const roj::FlatSkeleton* skeleton)
{
	data = std::move(characterData);
	bodySkeleton = skeleton;
}

void CharacterAnimator::LoadClip(Clip& clip, const std::string& source, const std::string& name, bool loop)
{
	clip = Clip();
	if (name.empty())
		return;

	clip.animation = AddAnimation(source, name, loop);
	clip.duration = clip.animation ? clip.animation->GetAnimationDuration() : 0.0f;
	if (clip.duration <= 0.0f)
	{
		Logger::Error("[CharacterAnimator] '%s': clip '%s' not found in '%s'", data->sourcePath.c_str(), name.c_str(), source.c_str());
		clip.animation = nullptr;
	}
}

// One stance per weapon of every slot (CharacterData::GetWeaponKey order), then the sheathed stance of every sheathable
// weapon (GetSheathedStanceKey order).
void CharacterAnimator::LoadStances()
{
	stances.clear();
	for (const auto& slot : data->weaponData)
		for (const auto& weapon : slot)
			LoadStance(stances.emplace_back(), weapon->stance, *weapon);
	for (const auto& slot : data->weaponData)
		for (const auto& weapon : slot)
			if (weapon->HasSheathedStance())
				LoadStance(stances.emplace_back(), weapon->sheathedStance, *weapon);

	stancesStarted = false;
}

void CharacterAnimator::LoadStance(Stance& stance, const WeaponStanceData& stanceData, const WeaponData& weapon)
{
	stance.blend = stanceData.blend;

	LoadClip(stance.idle, stanceData.resolvedAnimation, stanceData.idle, true);
	stance.idleTime = stanceData.idleTime;
	const bool sameClip = !stanceData.resolvedHold.empty() && stanceData.resolvedHold == stanceData.idle;
	if (!stanceData.IsAiming() && sameClip && stanceData.holdTime == stanceData.idleTime)
		stance.hold = stance.idle;
	else
		LoadClip(stance.hold, stanceData.resolvedAnimation, stanceData.resolvedHold, true);
	stance.holdTime = stanceData.holdTime;
	stance.holdSpace = stanceData.holdSpace;

	// An aiming weapon holds its sweep's middle frame; a second instance of the clip follows the aim pitch.
	if (stanceData.IsAiming())
	{
		LoadClip(stance.sweep, stanceData.resolvedAnimation, stanceData.sweep.clip, false);
		if (stance.hold.animation && stance.sweep.animation)
		{
			stance.sweepData = &stanceData.sweep;
			stance.holdTime = stance.hold.duration * 0.5f;
		}
	}

	for (const std::string& bone : stanceData.holdBones)
		stance.holdBones.emplace_back(bone);
	for (const std::string& bone : stanceData.moveHoldBones)
		stance.moveHoldBones.emplace_back(bone);
	for (const std::string& bone : stanceData.handBones)
		stance.handBones.emplace_back(bone);
	stance.sources.push_back(stanceData.resolvedAnimation);
	for (const WeaponMeshData& mesh : weapon.meshes)
		stance.sources.push_back(mesh.model);
}

void CharacterAnimator::LoadMoveSetSource(const std::string& source)
{
	Animation* primaryClip = AddAnimation(source, "", false);
	Animation* overlayClip = AddAnimation(source, "", false);

	std::unordered_map<hashed_string, float> durations;
	if (primaryClip && primaryClip->model)
	{
		for (const auto& [name, clip] : primaryClip->model->animations)
			if (clip.ticksPerSec > 0.0)
				durations[name] = static_cast<float>(clip.duration / clip.ticksPerSec);
	}
	else
	{
		Logger::Error("[CharacterAnimator] could not load clip source '%s'", source.c_str());
	}

	loadedSources.push_back(source);
	primaryLayer.push_back(primaryClip);
	overlayLayer.push_back(overlayClip);
	clipDurations.push_back(std::move(durations));
}

// A clip blends without completion only if its model has every bone of the body (poses carry their model's bones).
void CharacterAnimator::FindIncompleteClips()
{
	incompleteClips.clear();
	for (const Animation* animation : animationsToUpdate)
	{
		if (animation == nullptr || animation->model == nullptr)
			continue;
		if (bodySkeleton == nullptr)
		{
			incompleteClips.insert(animation);
			continue;
		}

		const auto& bones = animation->model->skeleton.nameToIdx;
		for (const auto& entry : bodySkeleton->nameToIdx)
		{
			if (bones.find(entry.first) == bones.end())
			{
				incompleteClips.insert(animation);
				break;
			}
		}
	}
}

// Which weapon a move clip poses: the one whose models (or stance source) the clip's file is.
void CharacterAnimator::MapClipWeapons()
{
	clipWeapons.clear();
	for (size_t source = 0; source < loadedSources.size(); ++source)
	{
		for (size_t key = 0; key < stances.size(); ++key)
		{
			const std::vector<std::string>& files = stances[key].sources;
			if (std::find(files.begin(), files.end(), loadedSources[source]) == files.end())
				continue;
			if (primaryLayer[source])
				clipWeapons[primaryLayer[source]] = static_cast<int>(key);
			if (overlayLayer[source])
				clipWeapons[overlayLayer[source]] = static_cast<int>(key);
			break;
		}
	}
}

void CharacterAnimator::LoadAssets()
{
	if (!data)
	{
		Logger::Error("[CharacterAnimator] LoadAssets without Configure()");
		return;
	}

	const LocomotionData& locomotion = data->locomotion;
	const std::string defaultSource = locomotion.animation.empty() ? data->mesh.model : locomotion.animation;
	auto source = [&](const char* field)
	{
		auto it = locomotion.sources.find(field);
		return it != locomotion.sources.end() ? it->second : defaultSource;
	};

	LoadClip(idle, source("idle"), locomotion.idle, true);
	LoadClip(runForward, source("runForward"), locomotion.runForward, true);
	LoadClip(runBackward, source("runBackward"), locomotion.runBackward, true);
	LoadClip(runLeft, source("runLeft"), locomotion.runLeft, true);
	LoadClip(runRight, source("runRight"), locomotion.runRight, true);
	LoadClip(fall, source("fall"), locomotion.fall, true);
	LoadClip(glide, source("glide"), locomotion.glide, true);

	// No directional clips at all: strafe procedurally with the forward run (a second instance plays it backward).
	proceduralStrafe = locomotion.strafe.enabled && runForward.animation
		&& !runBackward.animation && !runLeft.animation && !runRight.animation;
	if (proceduralStrafe)
		LoadClip(runReverse, source("runForward"), locomotion.runForward, true);
	else
		runReverse = Clip();

	if (!idle.animation)
		Logger::Error("[CharacterAnimator] '%s' has no valid idle clip - the body cannot be posed", data->sourcePath.c_str());

	if (data->moveSet)
	{
		for (const std::string& file : data->moveSet->animations)
			LoadMoveSetSource(file);

		for (const MoveData& move : data->moveSet->moves)
		{
			for (int clip = 0; clip < move.GetClipCount(); ++clip)
			{
				if (GetClipDuration(move.animationIndex, move.GetClipHash(clip)) <= 0.0f)
					Logger::Error("[CharacterAnimator] %s: move '%s' uses clip '%s' which does not exist in '%s'",
						data->moveSet->sourcePath.c_str(), move.id.c_str(), move.GetClipHash(clip).str().c_str(),
						data->moveSet->animations[static_cast<size_t>(move.animationIndex)].c_str());
			}
		}
	}

	LoadStances();
	FindIncompleteClips();
	MapClipWeapons();
}

#ifndef DISTRIBUTION
void CharacterAnimator::EnsureMoveSetAnimations(const MoveSetData& moveSet)
{
	// Indices must match the moveset's "animations" order; rebuild the layers if it changed.
	if (moveSet.animations == loadedSources)
		return;

	primary = PrimaryChannel();
	DropRecovery();
	recovery.inputSerial = 0;
	overlay.clip = nullptr;
	overlay.serial = 0;
	overlay.weight = 0.0f;
	overlayHandOver.fade.active = false;
	loadedSources.clear();
	primaryLayer.clear();
	overlayLayer.clear();
	clipDurations.clear();

	for (const std::string& file : moveSet.animations)
		LoadMoveSetSource(file);
	FindIncompleteClips();
	MapClipWeapons();
}
#endif

float CharacterAnimator::GetClipDuration(int animation, const hashed_string& clip) const
{
	if (animation < 0 || animation >= static_cast<int>(clipDurations.size()))
		return 0.0f;

	const auto& durations = clipDurations[static_cast<size_t>(animation)];
	auto it = durations.find(clip);
	return it == durations.end() ? 0.0f : it->second;
}

// ---- Evaluation -------------------------------------------------------------------------------------------

void CharacterAnimator::Sample(Animation* animation, float time)
{
	animation->SetAnimationTime(time);
	animation->Update(0.0f);   // re-evaluates the pose (and root motion) at the set time
}

void CharacterAnimator::StartLayerClip(std::vector<Animation*>& layer, const AnimatorInput::Layer& input, Animation*& active, bool extractRootMotion)
{
	active = nullptr;
	if (input.animation < 0 || input.animation >= static_cast<int>(layer.size()) || layer[static_cast<size_t>(input.animation)] == nullptr)
		return;

	Animation* animation = layer[static_cast<size_t>(input.animation)];
	animation->PlayAnimation(input.clip.str(), false, 0.0f);
	Sample(animation, input.time);

	if (extractRootMotion)
		animation->PullRootMotion();   // discard the jump from clip start to the entry start time

	active = animation;
}

void CharacterAnimator::BeginTransition(float duration)
{
	if (!hasLastOutput || duration <= 0.001f)
	{
		transition.active = false;
		return;
	}

	CopyPose(transition.pose, lastOutput);
	transition.duration = duration;
	transition.elapsed = 0.0f;
	transition.active = true;
}

void CharacterAnimator::Evaluate(const AnimatorInput& input, float deltaTime)
{
	if (!loaded)
		return;

	current = input;
	rootMotion = glm::vec3(0.0f);

	UpdateRecovery(input.recovery);   // before the primary: it takes over the clip of the attack that just ended
	UpdatePrimary(input.primary);
	UpdateOverlay(input.overlay, deltaTime);
	aimWeight = Approach(aimWeight, input.aiming ? 1.0f : 0.0f, 1.0f / AimBlendTime, deltaTime);
	UpdateRecoveryWeights(deltaTime);

	UpdateLocomotionWeights(deltaTime);
	UpdateLean(deltaTime);
	UpdateStanceWeights(deltaTime);
	UpdateSweeps();

	transition.Advance(deltaTime);
}

void CharacterAnimator::UpdateRecovery(const AnimatorInput::Recovery& input)
{
	if (input.serial != recovery.inputSerial)
	{
		if (input.serial != 0)
			BeginRecovery(input);
		else if (recovery.clip)
			recovery.fading = true;   // the clip ended (or a new move replaced it - dropped by the primary)
		recovery.inputSerial = input.serial;
	}
	else if (recovery.clip && recovery.inputSerial != 0)
	{
		Sample(recovery.clip, input.time);
		const MathHelper::Transform delta = recovery.clip->PullRootMotion();
		if (input.rootMotion && !input.released && !recovery.released)
			rootMotion += delta.Position;   // released: the capsule follows input instead
	}
}

void CharacterAnimator::BeginRecovery(const AnimatorInput::Recovery& input)
{
	DropRecovery();

	if (primary.clip && input.serial == primary.serial)
	{
		// The attack that just ended keeps playing: same Animation, same clip, no restart and no transition.
		recovery.clip = primary.clip;
		primary.clip = nullptr;
		primary.serial = 0;   // consumed - the primary layer must not blend out
		Sample(recovery.clip, input.time);
		const MathHelper::Transform delta = recovery.clip->PullRootMotion();
		if (input.rootMotion && !input.released)
			rootMotion += delta.Position;
	}
	else
	{
		// Not continuous with what is shown (body spawned mid-move, dev reload): start the clip and blend to it.
		AnimatorInput::Layer layer;
		layer.animation = input.animation;
		layer.clip = input.clip;
		layer.time = input.time;
		BeginTransition(input.lowerBodyBlend);
		StartLayerClip(primaryLayer, layer, recovery.clip, true);
		driveTransition = input.lowerBodyBlend;
	}

	if (!recovery.clip)
		return;

	recovery.moveSerial = input.serial;
	recovery.lowerBodyBlend = input.lowerBodyBlend;
	recovery.upperBodyBlend = input.upperBodyBlend;
	recovery.blendOut = input.blendOut;
	recovery.lowerWeight = 1.0f;
	recovery.upperWeight = 1.0f;
}

void CharacterAnimator::DropRecovery()
{
	const uint32_t inputSerial = recovery.inputSerial;
	recovery = RecoveryChannel();
	recovery.inputSerial = inputSerial;
}

void CharacterAnimator::UpdateRecoveryWeights(float deltaTime)
{
	if (!recovery.clip)
		return;

	// Until the body moves the whole body plays the clip; then the legs and the upper body hand over to locomotion.
	if (!recovery.released && current.recovery.released && current.recovery.serial == recovery.moveSerial)
	{
		recovery.released = true;
		recovery.releaseElapsed = 0.0f;
	}
	if (recovery.released)
		recovery.releaseElapsed += deltaTime;

	float lower = recovery.released ? FadeOut(recovery.releaseElapsed, recovery.lowerBodyBlend) : 1.0f;
	float upper = recovery.released && recovery.upperBodyBlend >= 0.0f ? FadeOut(recovery.releaseElapsed, recovery.upperBodyBlend) : 1.0f;

	if (recovery.fading)
	{
		// The clip ended (it ends in the idle stance): whatever still plays hands over.
		recovery.fadeElapsed += deltaTime;
		const float remaining = FadeOut(recovery.fadeElapsed, recovery.blendOut);
		lower = std::min(lower, remaining);
		upper = std::min(upper, remaining);
	}

	recovery.lowerWeight = lower;
	recovery.upperWeight = upper;
	if (lower <= 0.0f && upper <= 0.0f)
		DropRecovery();
}

void CharacterAnimator::UpdatePrimary(const AnimatorInput::Layer& input)
{
	if (input.serial != primary.serial)
	{
		if (input.serial != 0)
		{
			BeginTransition(input.blendIn);   // from the last output, recovery included
			DropRecovery();                   // the new move owns the body (and may reuse the clip source)
			StartLayerClip(primaryLayer, input, primary.clip, true);
			primary.blendOut = input.blendOut;
			driveTransition = input.blendIn;
		}
		else
		{
			BeginTransition(primary.blendOut);
			primary.clip = nullptr;
			driveTransition = primary.blendOut;
		}
		primary.serial = input.serial;
	}
	else if (primary.clip)
	{
		Sample(primary.clip, input.time);
		const MathHelper::Transform delta = primary.clip->PullRootMotion();
		if (input.rootMotion)
			rootMotion += delta.Position;
	}
}

// Bones without a bone list is the upper body (as for the primary); a Full overlay replaces the whole body.
void CharacterAnimator::ShapeFromInput(LayerShape& shape, const AnimatorInput::Layer& input)
{
	const bool bones = input.mask == BodyMask::Bones && input.maskBones != nullptr;
	shape.mask = bones ? BodyMask::Bones : (input.mask == BodyMask::Full ? BodyMask::Full : BodyMask::UpperBody);
	shape.meshSpace = input.meshSpace;
	shape.legWeight = input.legWeight;
	shape.airLegs = input.airLegs;
	if (bones)
		shape.maskBones.assign(input.maskBones->begin(), input.maskBones->end());
	else
		shape.maskBones.clear();

	const MoveAdditiveData* additive = input.additive;
	if (additive && additive->IsEnabled())
	{
		shape.additiveBones.assign(additive->boneHashes.begin(), additive->boneHashes.end());
		shape.additiveSpace = additive->space;
	}
	else
	{
		shape.additiveBones.clear();
	}
}

void CharacterAnimator::UpdateOverlay(const AnimatorInput::Layer& input, float deltaTime)
{
	if (input.serial != overlay.serial)
	{
		if (input.serial != 0)
		{
			// Another overlay clip takes over a visible one (a shot restarting its recoil clip, a draw after firing):
			// blend from where the previous one was instead of snapping.
			PoseFade& fade = overlayHandOver.fade;
			fade.active = overlay.clip != nullptr && overlay.weight > 0.001f && input.blendIn > 0.001f;
			if (fade.active)
			{
				CopyPose(fade.pose, overlay.clip->GetAnimationPoseRef());
				std::swap(overlayHandOver.shape, overlay.shape);
				if (!overlayHandOver.shape.additiveBones.empty())
					CopyPose(overlayHandOver.difference, overlay.difference);
				overlayHandOver.weight = overlay.weight;
				fade.duration = input.blendIn;
				fade.elapsed = 0.0f;
			}

			ShapeFromInput(overlay.shape, input);
			StartLayerClip(overlayLayer, input, overlay.clip, false);
			if (overlay.clip && !overlay.shape.additiveBones.empty())
			{
				// The additive layer's reference: the clip at its reference time (then back to the move clock).
				Sample(overlay.clip, input.additive->referenceTime);
				CopyPose(overlay.reference, overlay.clip->GetAnimationPoseRef());
				Sample(overlay.clip, input.time);
			}
			overlay.fadeRate = 1.0f / std::max(input.blendIn, 0.01f);
			overlay.blendOut = input.blendOut;
		}
		else
		{
			overlay.fadeRate = 1.0f / std::max(overlay.blendOut, 0.01f);
		}
		overlay.serial = input.serial;
	}
	else if (overlay.clip && input.serial != 0)
	{
		Sample(overlay.clip, input.time);
	}

	overlayHandOver.fade.Advance(deltaTime);

	const float target = (overlay.clip && input.serial != 0) ? 1.0f : 0.0f;
	overlay.weight = Approach(overlay.weight, target, overlay.fadeRate, deltaTime);
	if (overlay.weight <= 0.0f && input.serial == 0)
		overlay.clip = nullptr;
}

void CharacterAnimator::UpdateLocomotionWeights(float deltaTime)
{
	const LocomotionData& locomotion = data->locomotion;

	airWeight = Approach(airWeight, current.grounded ? 0.0f : 1.0f, 8.0f, deltaTime);
	glideWeight = Approach(glideWeight, current.gliding ? 1.0f : 0.0f, 6.0f, deltaTime);

	const glm::vec2 velocity = current.localVelocity;
	const float speed = glm::length(velocity);

	runForward.weight = runBackward.weight = runRight.weight = runLeft.weight = runReverse.weight = 0.0f;

	if (proceduralStrafe)
	{
		const float run = speed > 0.1f ? std::min(speed / locomotion.runClipSpeed, 1.0f) : 0.0f;
		UpdateStrafe(deltaTime, run);
		idle.weight = 1.0f - run;
	}
	else
	{
		float x = 0.0f, y = 0.0f;
		if (speed > 0.1f)
		{
			x = velocity.x / locomotion.runClipSpeed;
			y = velocity.y / locomotion.runClipSpeed;
		}

		float forward = std::max(y, 0.0f);
		float backward = std::max(-y, 0.0f);
		float right = std::max(x, 0.0f);
		float left = std::max(-x, 0.0f);

		// A direction without its own clip (some exist) plays the forward run: the legs keep moving instead of sliding.
		for (auto [clip, weight] : { std::pair{ &runBackward, &backward }, std::pair{ &runRight, &right }, std::pair{ &runLeft, &left } })
		{
			if (!clip->animation)
			{
				forward += *weight;
				*weight = 0.0f;
			}
		}
		if (!runForward.animation)
			forward = 0.0f;

		const float sum = forward + backward + right + left;
		if (sum > 1.0f)
		{
			const float inv = 1.0f / sum;
			forward *= inv; backward *= inv; right *= inv; left *= inv;
		}

		idle.weight = std::max(0.0f, 1.0f - (forward + backward + right + left));
		runForward.weight = forward;
		runBackward.weight = backward;
		runRight.weight = right;
		runLeft.weight = left;
	}

	// Shared gait phase; playback rate follows speed so feet don't slide.
	const float rate = speed > 0.1f ? std::clamp(speed / locomotion.runClipSpeed, 0.6f, 1.6f) : 1.0f;
	const float cycle = runForward.duration > 0.0f ? runForward.duration : 1.0f;
	locomotionPhase = std::fmod(locomotionPhase + deltaTime * rate / cycle, 1.0f);
	idleTime += deltaTime;

	if (idle.animation)
		Sample(idle.animation, LoopTime(idleTime, idle.duration));

	for (Clip* clip : { &runForward, &runBackward, &runLeft, &runRight })
		if (clip->animation && clip->weight > 0.001f)
			Sample(clip->animation, locomotionPhase * clip->duration);
	if (runReverse.animation && runReverse.weight > 0.001f)
		Sample(runReverse.animation, (1.0f - locomotionPhase) * runReverse.duration);

	if (fall.animation && airWeight > 0.001f)
		Sample(fall.animation, LoopTime(idleTime, fall.duration));
	if (glide.animation && glideWeight > 0.001f)
		Sample(glide.animation, LoopTime(idleTime, glide.duration));
}

// While strafing (the facing is not driven by the movement) the legs follow the move direction (smoothed, at most
// maxTwist off the facing); moving backward they run the reversed clip turned away from it. Forward / backward
// cross-fade with hysteresis around 90 degrees. Otherwise the body turns toward where it moves and the legs stay
// with it.
void CharacterAnimator::UpdateStrafe(float deltaTime, float run)
{
	const StrafeData& strafe = data->locomotion.strafe;
	const float speed = glm::length(current.velocity);

	if (speed > StrafeMinSpeed)
	{
		const float angle = current.strafing ? YawBetween(FlatDirection(current.forward), FlatDirection(current.velocity)) : 0.0f;
		if (run < 0.05f)
			strafeMoveAngle = angle;   // starting to move: no turn from an old direction
		else
		{
			const float step = glm::radians(strafe.turnRate) * deltaTime;
			strafeMoveAngle = WrapAngle(strafeMoveAngle + std::clamp(WrapAngle(angle - strafeMoveAngle), -step, step));
		}

		const float absolute = std::abs(strafeMoveAngle);
		if (!current.strafing)
			strafeBackward = false;
		else if (!strafeBackward && absolute > glm::radians(90.0f + StrafeHysteresis))
			strafeBackward = true;
		else if (strafeBackward && absolute < glm::radians(90.0f - StrafeHysteresis))
			strafeBackward = false;
	}

	strafeBackwardBlend = strafe.switchTime > 0.0f
		? Approach(strafeBackwardBlend, strafeBackward ? 1.0f : 0.0f, 1.0f / strafe.switchTime, deltaTime)
		: (strafeBackward ? 1.0f : 0.0f);

	const float maxTwist = glm::radians(strafe.maxTwist);
	strafeForwardTwist = std::clamp(strafeMoveAngle, -maxTwist, maxTwist);
	strafeBackwardTwist = std::clamp(WrapAngle(strafeMoveAngle - glm::pi<float>()), -maxTwist, maxTwist);

	const float backward = glm::smoothstep(0.0f, 1.0f, strafeBackwardBlend);
	runForward.weight = run * (1.0f - backward);
	runReverse.weight = run * backward;
}

void CharacterAnimator::UpdateLean(float deltaTime)
{
	const LeanData& lean = data->locomotion.lean;
	float targetForward = 0.0f;
	float targetSide = 0.0f;

	if (current.grounded && !lean.bone.empty())
	{
		const glm::vec3 forward = FlatDirection(current.forward);
		const glm::vec3 side = glm::normalize(glm::cross(forward, Up));
		targetForward = glm::radians(std::clamp(glm::dot(current.acceleration, forward) * lean.forwardPerAcceleration, -lean.maxForward, lean.maxForward));
		targetSide = glm::radians(std::clamp(glm::dot(current.acceleration, side) * lean.sidePerAcceleration, -lean.maxSide, lean.maxSide));
	}

	const float response = 1.0f - std::exp(-lean.sharpness * deltaTime);
	leanForward += (targetForward - leanForward) * response;
	leanSide += (targetSide - leanSide) * response;
}

void CharacterAnimator::UpdateStanceWeights(float deltaTime)
{
	for (size_t key = 0; key < stances.size(); ++key)
	{
		Stance& stance = stances[key];
		const bool shown = key < 32 && ((current.stances >> key) & 1u) != 0;
		const float target = shown ? 1.0f : 0.0f;
		if (shown)
			stance.layered = ((current.layeredStances >> key) & 1u) != 0;   // kept while it fades out

		// The first frame starts in the current stance instead of fading in from the locomotion idle.
		if (!stancesStarted || stance.blend <= 0.0f)
			stance.fade = target;
		else
			stance.fade = Approach(stance.fade, target, 1.0f / stance.blend, deltaTime);
		stance.weight = glm::smoothstep(0.0f, 1.0f, stance.fade);

		if (stance.weight <= 0.001f)
			continue;

		// Same clock as the locomotion idle: standing still, the hold layer matches the stance idle. A sweep's hold is
		// its middle frame.
		if (stance.idle.animation)
			Sample(stance.idle.animation, stance.idleTime >= 0.0f ? stance.idleTime : LoopTime(idleTime, stance.idle.duration));
		if (stance.hold.animation && stance.hold.animation != stance.idle.animation)
			Sample(stance.hold.animation, stance.holdTime >= 0.0f ? stance.holdTime : LoopTime(idleTime, stance.hold.duration));
	}
	stancesStarted = true;
}

// The sweep of every shown aiming weapon at the aim pitch (the direction from the chest to the aim point, model space).
void CharacterAnimator::UpdateSweeps()
{
	if (aimWeight <= 0.0f)
		return;

	const glm::vec3 aim = current.aimDirection;
	const float flat = glm::length(glm::vec2(aim.x, aim.z));
	const float pitch = glm::degrees(std::atan2(aim.y, flat));
	for (Stance& stance : stances)
		if (stance.sweepData && stance.weight > 0.001f)
			Sample(stance.sweep.animation, stance.sweepData->TimeAt(pitch, stance.sweep.duration));
}

// ---- Composition ------------------------------------------------------------------------------------------

AnimationPose CharacterAnimator::ProcessResultPose()
{
	return ComposePose();
}

const AnimationPose& CharacterAnimator::ComposePose()
{
	if (!loaded || !idle.animation)
	{
		output.boneTransforms.clear();
		return output;
	}

	roj::BoneNode* rootNode = idle.animation->GetRootNode();
	AnimationPose& pose = output;

	ComposeLocomotion(pose, rootNode);
	if (rootNode)
		LayerHolds(pose, rootNode, false);
	ComposeMove(pose, rootNode);
	if (rootNode)
	{
		ComposeOverlay(pose, rootNode);
		ApplyGrip(pose, rootNode);
		ApplyAim(pose, rootNode);
	}

	if (transition.active)
		LerpFrom(pose, transition.pose, transition.Progress());

	// Root motion is applied to the capsule, not the mesh.
	auto root = pose.boneTransforms.find(RootBone);
	if (root != pose.boneTransforms.end())
	{
		root->second[3].x = 0.0f;
		root->second[3].z = 0.0f;
	}

	CopyPose(lastOutput, pose);
	hasLastOutput = true;

	if (rootNode)
		ApplyHitRecoil(pose, rootNode);
	return pose;
}

// ---- Building blocks

bool CharacterAnimator::IsFullBodyPrimary(const roj::BoneNode* rootNode) const
{
	return primary.clip && (current.primary.mask == BodyMask::Full || current.primary.mask == BodyMask::NoLegs || !rootNode);
}

// A movement move (jump, glide, dash) - or one playing out in recovery - shows its clip: it carries no weapon.
bool CharacterAnimator::IsMovementClipShown() const
{
	return primary.clip ? current.primary.movement : (recovery.clip != nullptr && current.recovery.movement);
}

// A clip's pose ready to blend over `base`: as is when its model has every body bone, else completed from base
// (into `completion`, valid until the next completion).
const AnimationPose& CharacterAnimator::Blendable(Animation* animation, const AnimationPose& base)
{
	if (IsComplete(animation))
		return animation->GetAnimationPoseRef();
	CompleteInto(completion, animation->GetAnimationPoseRef(), base);
	return completion;
}

// pose = Lerp(from, pose, progress): computed apart (the result depends on which pose comes first), swapped in. The
// root comes from pose (Lerp takes it from its second pose).
void CharacterAnimator::LerpFrom(AnimationPose& pose, const AnimationPose& from, float progress)
{
	CopyPose(blendScratch, from);
	AnimationPose::LerpInPlace(blendScratch, pose, progress);
	pose.boneTransforms.swap(blendScratch.boneTransforms);
}

// `layer` on `bone`'s subtree by weight. meshSpace: the share (0..1) of the start bone's rotation blended in mesh space
// (0 local: it follows its parent, 1 mesh: it keeps the layer's model-space orientation). < 0 (Default): mesh at the
// upper-body bone (the torso keeps the layer's facing whatever the legs do), local at a limb (it follows the torso).
// Incomplete layers are completed from the target first. The root keeps its value.
void CharacterAnimator::LayerBranch(AnimationPose& target, roj::BoneNode* rootNode, const hashed_string& bone, const AnimationPose& layer,
	bool layerComplete, float weight, float meshSpace)
{
	if (meshSpace < 0.0f)
		meshSpace = bone == data->locomotion.upperBodyBoneHash ? 1.0f : 0.0f;
	if (!layerComplete)
		CompleteInto(completion, layer, target);

	RootGuard root(target);
	AnimationPose::LayeredLerpInPlace(bone, rootNode, target, layerComplete ? layer : completion, std::min(meshSpace, 1.0f), weight);
}

// A layer on the upper body, or on its mask bones (maskBones: BodyMask::Bones, else null).
void CharacterAnimator::LayerMask(AnimationPose& target, roj::BoneNode* rootNode, const std::vector<hashed_string>* maskBones,
	const AnimationPose& layer, bool layerComplete, float weight, float meshSpace)
{
	if (maskBones == nullptr)
	{
		LayerBranch(target, rootNode, data->locomotion.upperBodyBoneHash, layer, layerComplete, weight, meshSpace);
		return;
	}
	for (const hashed_string& bone : *maskBones)
		LayerBranch(target, rootNode, bone, layer, layerComplete, weight, meshSpace);
}

// The legs (locomotion.legBones subtrees, local to the hips) blend toward `legs` by weight. The pose keeps its root:
// it is the clip's that owns the body (an attack's, the reload's spin).
void CharacterAnimator::HandLegsTo(AnimationPose& pose, roj::BoneNode* rootNode, const AnimationPose& legs, float weight) const
{
	RootGuard root(pose);
	for (const hashed_string& leg : data->locomotion.legBoneHashes)
		AnimationPose::LayeredLerpInPlace(leg, rootNode, pose, legs, 0.0f, weight);
}

// ---- Stages

// Idle (the stance idles replace the locomotion idle: their weights sum to 1 once a weapon is held), the run blended
// over it (procedural strafe: each run turned toward its legs' direction first), the lean, the air poses.
void CharacterAnimator::ComposeLocomotion(AnimationPose& pose, roj::BoneNode* rootNode)
{
	CopyPose(pose, idle.animation->GetAnimationPoseRef());
	{
		float total = 0.0f;
		for (const Stance& stance : stances)
			if (stance.idle.animation && !stance.layered)
				total += stance.weight;

		float accumulated = std::max(0.0f, 1.0f - total);
		for (const Stance& stance : stances)
		{
			if (!stance.idle.animation || stance.layered || stance.weight <= 0.001f)
				continue;
			accumulated += stance.weight;
			AnimationPose::LerpInPlace(pose, Blendable(stance.idle.animation, pose), stance.weight / accumulated);
		}
	}

	float accumulated = idle.weight;
	auto blendIn = [&](const Clip& clip, float twist)
	{
		if (!clip.animation || clip.weight <= 0.001f)
			return;
		const float total = accumulated + clip.weight;
		if (std::abs(twist) > 1e-4f && rootNode)
		{
			AnimationPose& turned = blendScratch;
			CopyPose(turned, Blendable(clip.animation, pose));
			ApplyLegTwist(turned, rootNode, twist);
			AnimationPose::LerpInPlace(pose, turned, clip.weight / total);
		}
		else
		{
			AnimationPose::LerpInPlace(pose, Blendable(clip.animation, pose), clip.weight / total);
		}
		accumulated = total;
	};

	blendIn(runForward, proceduralStrafe ? strafeForwardTwist : 0.0f);
	blendIn(runReverse, strafeBackwardTwist);
	blendIn(runBackward, 0.0f);
	blendIn(runRight, 0.0f);
	blendIn(runLeft, 0.0f);

	if (rootNode)
		ApplyLean(pose, rootNode);

	if (fall.animation && airWeight > 0.001f)
		AnimationPose::LerpInPlace(pose, Blendable(fall.animation, pose), airWeight);

	Animation* glidePose = glide.animation ? glide.animation : fall.animation;
	if (glidePose && glideWeight > 0.001f)
		AnimationPose::LerpInPlace(pose, Blendable(glidePose, pose), glideWeight);
}

// The holds of the stances shown. Over the locomotion (overMove false): each on its holdBones in its holdSpace, the
// stances in the hands normalized like the idles (a cross-fade between two weapons does not dip into the base),
// layered ones on top with their own weight. Over a movement move's clip (overMove true): the hands holding the
// weapon (moveHoldBones, local: the arms keep the move), each with its own weight.
void CharacterAnimator::LayerHolds(AnimationPose& pose, roj::BoneNode* rootNode, bool overMove)
{
	float total = 0.0f;
	for (const Stance& stance : stances)
		if (stance.hold.animation && !stance.layered)
			total += stance.weight;

	float accumulated = std::max(0.0f, 1.0f - total);
	for (const bool layered : { false, true })
	{
		for (const Stance& stance : stances)
		{
			if (!stance.hold.animation || stance.layered != layered || stance.weight <= 0.001f)
				continue;
			float weight = stance.weight;
			if (!layered && !overMove)
			{
				accumulated += stance.weight;
				weight = stance.weight / accumulated;
			}

			const AnimationPose& hold = stance.hold.animation->GetAnimationPoseRef();
			const bool complete = IsComplete(stance.hold.animation);
			if (overMove)
			{
				for (const hashed_string& bone : stance.moveHoldBones)
					LayerBranch(pose, rootNode, bone, hold, complete, weight, -1.0f);
				continue;
			}
			for (const hashed_string& bone : stance.holdBones)
				LayerBranch(pose, rootNode, bone, hold, complete, weight, MeshSpaceShare(stance.holdSpace));
		}
	}
}

// The primary move and the recovery of an attack. A Full / NoLegs clip replaces the pose (move clips pose the arms
// themselves); NoLegs - and Full in the air, faded by the air weight with take-off and landing, unless its clip poses
// its own air legs - hands the legs to the locomotion / air pose by legWeight, local to the clip's hips. The recovery
// plays on the whole body until the body moves, then the legs and the upper body hand over. UpperBody / Bones clips go
// on their subtrees. Over a movement move's clip the firearms' hands take their hold again.
void CharacterAnimator::ComposeMove(AnimationPose& pose, roj::BoneNode* rootNode)
{
	const bool fullBody = IsFullBodyPrimary(rootNode);
	if (fullBody)
	{
		float legBlend = 0.0f;
		if (current.primary.mask == BodyMask::NoLegs)
			legBlend = current.primary.legWeight;
		else if (current.primary.mask == BodyMask::Full && current.primary.airLegs)
			legBlend = current.primary.legWeight * airWeight;
		const bool keepLegs = rootNode && legBlend > 0.001f;
		AnimationPose& locomotionPose = layerScratch;
		if (keepLegs)
			CopyPose(locomotionPose, pose);

		if (IsComplete(primary.clip))
			CopyPose(pose, primary.clip->GetAnimationPoseRef());
		else
			CompleteInto(pose, primary.clip->GetAnimationPoseRef(), pose);

		if (keepLegs)
			HandLegsTo(pose, rootNode, locomotionPose, legBlend);
	}

	if (!rootNode)
		return;

	if (recovery.clip)
	{
		const AnimationPose& recoveryPose = Blendable(recovery.clip, pose);

		// Locomotion keeps the root while the legs hand over (LerpFrom takes it from the pose).
		if (recovery.lowerWeight > 0.001f)
			LerpFrom(pose, recoveryPose, 1.0f - recovery.lowerWeight);

		if (recovery.upperWeight > 0.001f && recovery.lowerWeight < 0.999f)
			LayerBranch(pose, rootNode, data->locomotion.upperBodyBoneHash, recoveryPose, true, recovery.upperWeight, current.recovery.meshSpace);
	}

	if (primary.clip && !fullBody)
	{
		const bool bones = current.primary.mask == BodyMask::Bones && current.primary.maskBones != nullptr;
		LayerMask(pose, rootNode, bones ? current.primary.maskBones : nullptr, primary.clip->GetAnimationPoseRef(),
			IsComplete(primary.clip), 1.0f, current.primary.meshSpace);
	}

	if (primary.clip || recovery.clip)
		LayerAimHolds(pose, rootNode);
	if (IsMovementClipShown())
		LayerHolds(pose, rootNode, true);
}

// While a weapon aims, its whole hold goes over the clip of the move (a jump) or of an attack playing out in recovery -
// faded in with the aim, unless that clip is the weapon's own: the sweep and the yaw (ApplyAim) aim the hold pose, not
// whatever the clip does with the arms. A firearm without a fire clip (the rifle) shows its pose this way; the legs
// keep the clip (the recovery blends them out).
void CharacterAnimator::LayerAimHolds(AnimationPose& pose, roj::BoneNode* rootNode)
{
	const float fade = glm::smoothstep(0.0f, 1.0f, aimWeight);
	if (fade <= 0.001f)
		return;

	const Animation* shown = primary.clip ? primary.clip : recovery.clip;
	auto owner = clipWeapons.find(shown);
	const int shownWeapon = owner != clipWeapons.end() ? owner->second : -1;
	for (size_t key = 0; key < stances.size(); ++key)
	{
		const Stance& stance = stances[key];
		const float weight = stance.weight * fade;
		if (!stance.sweepData || weight <= 0.001f || static_cast<int>(key) == shownWeapon)
			continue;
		const AnimationPose& hold = stance.hold.animation->GetAnimationPoseRef();
		const bool complete = IsComplete(stance.hold.animation);
		for (const hashed_string& bone : stance.holdBones)
			LayerBranch(pose, rootNode, bone, hold, complete, weight, MeshSpaceShare(stance.holdSpace));
	}
}

// One overlay clip over `target` by weight: its additive layer (the clip's difference from its reference frame), then
// its mask. A Full overlay replaces the whole body - its root too (the reload's spin) - and hands its legs to the pose
// underneath by legWeight as the body runs (and, with airLegs, as it flies): standing it keeps the clip's footwork.
void CharacterAnimator::LayerOverlay(AnimationPose& target, roj::BoneNode* rootNode, const LayerShape& shape, const AnimationPose& clip,
	bool complete, const AnimationPose& difference, float weight)
{
	for (const hashed_string& bone : shape.additiveBones)
		AddBranch(target, rootNode, bone, difference, shape.additiveSpace, weight);

	if (shape.mask != BodyMask::Full)
	{
		LayerMask(target, rootNode, shape.mask == BodyMask::Bones ? &shape.maskBones : nullptr, clip, complete, weight, shape.meshSpace);
		return;
	}

	AnimationPose& full = fullScratch;
	if (complete)
		CopyPose(full, clip);
	else
		CompleteInto(full, clip, target);

	const float moving = std::max(1.0f - idle.weight, 0.0f);
	const float legBlend = shape.legWeight * std::max(moving, shape.airLegs ? airWeight : 0.0f);
	if (legBlend > 0.001f)
		HandLegsTo(full, rootNode, target, legBlend);

	LerpWithRoot(target, full, weight);
}

// The overlay keeps the shape it started with until it has faded out. An overlay replacing a visible one cross-fades
// over the new clip's blendIn: the outgoing one (last pose, difference, weight, shape) and the incoming one are each
// layered over the same pose and the results blend - bones both cover go from one clip to the other, bones only one
// covers fade from / to the pose underneath (nothing snaps when the masks differ).
void CharacterAnimator::ComposeOverlay(AnimationPose& pose, roj::BoneNode* rootNode)
{
	const bool incoming = overlay.clip && overlay.weight > 0.001f;
	if (incoming && !overlay.shape.additiveBones.empty())
		AnimationPose::MakeAdditive(overlay.difference, rootNode, overlay.reference, overlay.clip->GetAnimationPoseRef(),
			ToAdditiveSpace(overlay.shape.additiveSpace));

	if (overlayHandOver.fade.active)
	{
		AnimationPose& outgoing = layerScratch;
		CopyPose(outgoing, pose);
		if (overlayHandOver.weight > 0.001f)
			LayerOverlay(outgoing, rootNode, overlayHandOver.shape, overlayHandOver.fade.pose, false, overlayHandOver.difference,
				overlayHandOver.weight);
		if (incoming)
			LayerOverlay(pose, rootNode, overlay.shape, overlay.clip->GetAnimationPoseRef(), IsComplete(overlay.clip), overlay.difference,
				overlay.weight);
		LerpFrom(pose, outgoing, overlayHandOver.fade.Progress());
	}
	else if (incoming)
	{
		LayerOverlay(pose, rootNode, overlay.shape, overlay.clip->GetAnimationPoseRef(), IsComplete(overlay.clip), overlay.difference,
			overlay.weight);
	}
}

// The hands holding the weapon take the grip of the clip posing it, in local space (the arm keeps whatever the
// layers made of it): this weapon's overlay clip, else its move / recovery clip, else its hold clip. Weighted by
// the stance (two weapons cross-fading each keep their own grip) and by how much those clips show. The stances in
// the hands share the grip (normalized like their holds); layered ones grip their own hands on top.
void CharacterAnimator::ApplyGrip(AnimationPose& pose, roj::BoneNode* rootNode)
{
	auto ownedBy = [this](const Animation* animation, int key)
	{
		auto it = clipWeapons.find(animation);
		return animation != nullptr && it != clipWeapons.end() && it->second == key;
	};

	float total = 0.0f;
	for (const Stance& stance : stances)
		if (!stance.handBones.empty() && stance.hold.animation && !stance.layered)
			total += stance.weight;
	float accumulated = std::max(0.0f, 1.0f - total);

	const Animation* moveClip = primary.clip ? primary.clip : recovery.clip;
	const float moveWeight = primary.clip ? 1.0f : recovery.upperWeight;

	RootGuard root(pose);
	for (const bool layered : { false, true })
	{
		for (size_t key = 0; key < stances.size(); ++key)
		{
			const Stance& stance = stances[key];
			if (stance.handBones.empty() || !stance.hold.animation || stance.layered != layered || stance.weight <= 0.001f)
				continue;
			float share = stance.weight;
			if (!layered)
			{
				accumulated += stance.weight;
				share = stance.weight / accumulated;
			}

			struct Source { const Animation* clip; float weight; };
			const Source sources[] = {
				{ stance.hold.animation, share },
				{ ownedBy(moveClip, static_cast<int>(key)) ? moveClip : nullptr, share * moveWeight },
				{ ownedBy(overlay.clip, static_cast<int>(key)) ? overlay.clip : nullptr, share * overlay.weight },
			};

			for (const Source& source : sources)
			{
				if (source.clip == nullptr || source.weight <= 0.001f)
					continue;
				Animation* clip = const_cast<Animation*>(source.clip);
				const AnimationPose& layer = Blendable(clip, pose);
				for (const hashed_string& hand : stance.handBones)
					AnimationPose::LayeredLerpInPlace(hand, rootNode, pose, layer, 0.0f, source.weight);
			}
		}
	}
}

// Aiming weapons point at the aim point. Their hold pose is the sweep's middle frame, aimed level along the facing.
// Pitch: the sweep at the aim pitch, as its difference from that frame, turns the sweep bones the way the clip does -
// spine, arms, coat - over whatever the weapon's clips (recoil, reload) and the grip made of the pose. Yaw: the body
// turns toward the aim at aimTurnRate; until it gets there the yaw bones turn the rest (at most maxYaw, about up, after
// the pitch), so the weapon is on the aim at once. Weighted by the stance and the aim fade.
void CharacterAnimator::ApplyAim(AnimationPose& pose, roj::BoneNode* rootNode)
{
	const float fade = glm::smoothstep(0.0f, 1.0f, aimWeight);
	if (fade <= 0.001f)
		return;

	const glm::vec3 aimFlat(current.aimDirection.x, 0.0f, current.aimDirection.z);
	const float yaw = glm::dot(aimFlat, aimFlat) > 1e-6f ? YawBetween(FlatDirection(current.forward), glm::normalize(aimFlat)) : 0.0f;

	for (Stance& stance : stances)
	{
		const float weight = stance.weight * fade;
		if (!stance.sweepData || weight <= 0.001f)
			continue;
		const WeaponSweepData& sweep = *stance.sweepData;

		AnimationPose::MakeAdditive(stance.sweepDifference, rootNode, stance.hold.animation->GetAnimationPoseRef(),
			stance.sweep.animation->GetAnimationPoseRef(), ToAdditiveSpace(sweep.space));
		for (const hashed_string& bone : sweep.boneHashes)
			AddBranch(pose, rootNode, bone, stance.sweepDifference, sweep.space, weight);

		if (sweep.yawBoneHashes.empty())
			continue;
		const float maxYaw = glm::radians(sweep.maxYaw);
		const float share = std::clamp(yaw, -maxYaw, maxYaw) * weight / static_cast<float>(sweep.yawBoneHashes.size());
		if (std::abs(share) < 1e-5f)
			continue;
		const glm::mat3 step = glm::mat3_cast(glm::angleAxis(share, Up));
		for (const hashed_string& bone : sweep.yawBoneHashes)   // parent first: the shares add up
			RotateInModelSpace(pose, rootNode, bone, step);
	}
}

// Legs turned by `angle` (about up, model space), the counter bones turning the torso back to the facing.
void CharacterAnimator::ApplyLegTwist(AnimationPose& pose, roj::BoneNode* rootNode, float angle) const
{
	const StrafeData& strafe = data->locomotion.strafe;
	RotateInModelSpace(pose, rootNode, strafe.legsBoneHash, glm::mat3_cast(glm::angleAxis(angle, Up)));

	if (strafe.counterBoneHashes.empty())
		return;
	const float share = -angle / static_cast<float>(strafe.counterBoneHashes.size());
	const glm::mat3 counter = glm::mat3_cast(glm::angleAxis(share, Up));
	for (const hashed_string& bone : strafe.counterBoneHashes)   // parent first: each undoes its share
		RotateInModelSpace(pose, rootNode, bone, counter);
}

// Tilts the lean bone toward the acceleration: forward (about the side axis) and into turns (about the facing).
void CharacterAnimator::ApplyLean(AnimationPose& pose, roj::BoneNode* rootNode) const
{
	const LeanData& lean = data->locomotion.lean;
	if (lean.bone.empty() || std::abs(leanForward) + std::abs(leanSide) < 1e-4f)
		return;

	const glm::vec3 forward = FlatDirection(current.forward);
	const glm::vec3 side = glm::normalize(glm::cross(forward, Up));
	// angleAxis(+a, side) tips the top backward and angleAxis(+a, forward) tips it toward `side`.
	const glm::quat rotation = glm::angleAxis(leanSide, forward) * glm::angleAxis(-leanForward, side);
	RotateInModelSpace(pose, rootNode, lean.boneHash, glm::mat3_cast(rotation));
}

// The recoil angle spread over the chain by weight, each bone about its own origin (parent first: they add up).
void CharacterAnimator::ApplyHitRecoil(AnimationPose& pose, roj::BoneNode* rootNode) const
{
	const float angle = glm::length(current.recoil);
	if (angle < 1e-4f)
		return;

	const glm::vec3 axis = current.recoil / angle;
	for (const HitRecoilBoneData& bone : data->hitRecoil.bones)
		RotateInModelSpace(pose, rootNode, bone.boneHash, glm::mat3_cast(glm::angleAxis(angle * bone.weight, axis)));
}

// ---- Outputs ----------------------------------------------------------------------------------------------

glm::vec3 CharacterAnimator::ConsumeRootMotion()
{
	const glm::vec3 result(rootMotion.x, 0.0f, rootMotion.z);
	rootMotion = glm::vec3(0.0f);
	return result;
}

CharacterAnimator::WeaponDrive CharacterAnimator::GetWeaponDrive()
{
	WeaponDrive drive;
	drive.moveTransition = driveTransition;

	auto weaponOf = [this](const Animation* animation)
	{
		auto it = clipWeapons.find(animation);
		return it == clipWeapons.end() ? -1 : it->second;
	};

	if (primary.clip)
	{
		drive.move = &primary.clip->GetAnimationPoseRef();
		drive.moveWeight = 1.0f;
		drive.moveSerial = primary.serial;
		drive.moveWeapon = weaponOf(primary.clip);
	}
	else if (recovery.clip)
	{
		drive.move = &recovery.clip->GetAnimationPoseRef();
		drive.moveWeight = recovery.upperWeight;
		drive.moveSerial = recovery.moveSerial;
		drive.moveWeapon = weaponOf(recovery.clip);
	}

	if (overlay.clip && overlay.weight > 0.001f)
	{
		drive.overlay = &overlay.clip->GetAnimationPoseRef();
		drive.overlayWeight = overlay.weight;
		drive.overlayWeapon = weaponOf(overlay.clip);
	}
	return drive;
}
