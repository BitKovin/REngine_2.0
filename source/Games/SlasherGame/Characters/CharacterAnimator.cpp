#include "CharacterAnimator.h"

#include <Logger.hpp>

#include <algorithm>
#include <cmath>

namespace
{
	constexpr float AimBlendTime = 0.15f;          // s for the aim offset to fade in / out
	constexpr float StrafeMinSpeed = 0.3f;         // m/s below which the legs keep their direction
	constexpr float StrafeHysteresis = 20.0f;      // deg around 90 between running forward and backward

	const glm::vec3 Up(0.0f, 1.0f, 0.0f);

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

	const hashed_string RootBone("root");

	// A clip of a model that lacks some of the body's bones (a weapon model without the coat) must leave them to the
	// pose underneath: Lerp's fast path would drop them and LayeredLerp would collapse them to identity.
	AnimationPose CompleteFrom(const AnimationPose& layer, const AnimationPose& base)
	{
		AnimationPose out = AnimationPose::Lerp(layer, base, 0.0f);   // the layer plus the bones only the base has
		auto root = layer.boneTransforms.find(RootBone);
		if (root != layer.boneTransforms.end())
			out.boneTransforms[RootBone] = root->second;               // Lerp takes the root from the base
		return out;
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
}

void CharacterAnimator::Configure(std::shared_ptr<const CharacterData> characterData, const roj::FlatSkeleton* skeleton)
{
	data = std::move(characterData);
	bodySkeleton = skeleton;
}

void CharacterAnimator::LoadLocomotionClip(LocomotionClip& clip, const std::string& source, const std::string& name, bool loop)
{
	clip = LocomotionClip();
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

void CharacterAnimator::LoadStances()
{
	for (size_t slot = 0; slot < stances.size(); ++slot)
	{
		Stance& stance = stances[slot];
		stance = Stance();

		const WeaponData* weapon = data->weaponData[slot].get();
		if (!weapon)
			continue;

		const WeaponStanceData& stanceData = weapon->stance;
		stance.blend = stanceData.blend;

		LoadLocomotionClip(stance.idle, stanceData.resolvedAnimation, stanceData.idle, true);
		if (!stanceData.hold.empty() && stanceData.hold == stanceData.idle)
			stance.hold = stance.idle;
		else
			LoadLocomotionClip(stance.hold, stanceData.resolvedAnimation, stanceData.hold, true);

		for (const std::string& bone : stanceData.holdBones)
			stance.holdBones.emplace_back(bone);
		for (const std::string& bone : stanceData.aimBones)
			stance.aimBones.emplace_back(bone);
		for (const std::string& bone : stanceData.handBones)
			stance.handBones.emplace_back(bone);
		stance.sources.push_back(stanceData.resolvedAnimation);
		for (const WeaponMeshData& mesh : weapon->meshes)
			stance.sources.push_back(mesh.model);
		stance.aimMaxPitch = stanceData.aimMaxPitch;
		stance.aimMaxYaw = stanceData.aimMaxYaw;
	}

	stancesStarted = false;
}

void CharacterAnimator::LoadMoveSetSource(const std::string& source)
{
	Animation* primary = AddAnimation(source, "", false);
	Animation* overlay = AddAnimation(source, "", false);

	std::unordered_map<hashed_string, float> durations;
	if (primary && primary->model)
	{
		for (const auto& [name, clip] : primary->model->animations)
			if (clip.ticksPerSec > 0.0)
				durations[name] = static_cast<float>(clip.duration / clip.ticksPerSec);
	}
	else
	{
		Logger::Error("[CharacterAnimator] could not load clip source '%s'", source.c_str());
	}

	loadedSources.push_back(source);
	primaryLayer.push_back(primary);
	overlayLayer.push_back(overlay);
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

// Which weapon a move clip poses: the slot whose models (or stance source) the clip's file is.
void CharacterAnimator::MapClipWeapons()
{
	clipWeapons.clear();
	for (size_t source = 0; source < loadedSources.size(); ++source)
	{
		for (size_t slot = 0; slot < stances.size(); ++slot)
		{
			const std::vector<std::string>& files = stances[slot].sources;
			if (std::find(files.begin(), files.end(), loadedSources[source]) == files.end())
				continue;
			if (primaryLayer[source])
				clipWeapons[primaryLayer[source]] = static_cast<int>(slot);
			if (overlayLayer[source])
				clipWeapons[overlayLayer[source]] = static_cast<int>(slot);
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

	LoadLocomotionClip(idle, source("idle"), locomotion.idle, true);
	LoadLocomotionClip(runForward, source("runForward"), locomotion.runForward, true);
	LoadLocomotionClip(runBackward, source("runBackward"), locomotion.runBackward, true);
	LoadLocomotionClip(runLeft, source("runLeft"), locomotion.runLeft, true);
	LoadLocomotionClip(runRight, source("runRight"), locomotion.runRight, true);
	LoadLocomotionClip(fall, source("fall"), locomotion.fall, true);
	LoadLocomotionClip(glide, source("glide"), locomotion.glide, true);

	// No directional clips at all: strafe procedurally with the forward run (a second instance plays it backward).
	proceduralStrafe = locomotion.strafe.enabled && runForward.animation
		&& !runBackward.animation && !runLeft.animation && !runRight.animation;
	if (proceduralStrafe)
		LoadLocomotionClip(runReverse, source("runForward"), locomotion.runForward, true);
	else
		runReverse = LocomotionClip();

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

	activePrimary = nullptr;
	activeOverlay = nullptr;
	primarySerial = 0;
	overlaySerial = 0;
	DropRecovery();
	recoverySerial = 0;
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

void CharacterAnimator::BeginSnapshot(float duration)
{
	if (!hasLastOutput || duration <= 0.001f)
	{
		snapshotActive = false;
		return;
	}

	snapshot = lastOutput;
	snapshotDuration = duration;
	snapshotElapsed = 0.0f;
	snapshotActive = true;
}

void CharacterAnimator::BeginRecovery(const AnimatorInput::Recovery& recovery)
{
	DropRecovery();

	if (activePrimary && recovery.serial == primarySerial)
	{
		// The attack that just ended keeps playing: same Animation, same clip, no restart and no transition.
		activeRecovery = activePrimary;
		activePrimary = nullptr;
		primarySerial = 0;   // consumed - the primary layer must not blend out
		Sample(activeRecovery, recovery.time);
		const MathHelper::Transform delta = activeRecovery->PullRootMotion();
		if (recovery.rootMotion && !recovery.released)
			rootMotion += delta.Position;
	}
	else
	{
		// Not continuous with what is shown (body spawned mid-move, dev reload): start the clip and blend to it.
		AnimatorInput::Layer layer;
		layer.animation = recovery.animation;
		layer.clip = recovery.clip;
		layer.time = recovery.time;
		BeginSnapshot(recovery.lowerBodyBlend);
		StartLayerClip(primaryLayer, layer, activeRecovery, true);
		driveTransition = recovery.lowerBodyBlend;
	}

	if (!activeRecovery)
		return;

	recoveryMoveSerial = recovery.serial;
	recoveryReleased = false;
	recoveryReleaseElapsed = 0.0f;
	recoveryFadeElapsed = 0.0f;
	recoveryLowerBodyBlend = recovery.lowerBodyBlend;
	recoveryUpperBodyBlend = recovery.upperBodyBlend;
	recoveryBlendOut = recovery.blendOut;
	recoveryLowerWeight = 1.0f;
	recoveryUpperWeight = 1.0f;
	recoveryFading = false;
}

void CharacterAnimator::DropRecovery()
{
	activeRecovery = nullptr;
	recoveryMoveSerial = 0;
	recoveryReleased = false;
	recoveryLowerWeight = 0.0f;
	recoveryUpperWeight = 0.0f;
	recoveryFading = false;
}

void CharacterAnimator::UpdateRecoveryWeights(float deltaTime)
{
	if (!activeRecovery)
		return;

	// Until the body moves the whole body plays the clip; then the legs and the upper body hand over to locomotion.
	if (!recoveryReleased && current.recovery.released && current.recovery.serial == recoveryMoveSerial)
	{
		recoveryReleased = true;
		recoveryReleaseElapsed = 0.0f;
	}
	if (recoveryReleased)
		recoveryReleaseElapsed += deltaTime;

	float lower = recoveryReleased ? FadeOut(recoveryReleaseElapsed, recoveryLowerBodyBlend) : 1.0f;
	float upper = recoveryReleased && recoveryUpperBodyBlend >= 0.0f ? FadeOut(recoveryReleaseElapsed, recoveryUpperBodyBlend) : 1.0f;

	if (recoveryFading)
	{
		// The clip ended (it ends in the idle stance): whatever still plays hands over.
		recoveryFadeElapsed += deltaTime;
		const float remaining = FadeOut(recoveryFadeElapsed, recoveryBlendOut);
		lower = std::min(lower, remaining);
		upper = std::min(upper, remaining);
	}

	recoveryLowerWeight = lower;
	recoveryUpperWeight = upper;
	if (lower <= 0.0f && upper <= 0.0f)
		DropRecovery();
}

void CharacterAnimator::Evaluate(const AnimatorInput& input, float deltaTime)
{
	if (!loaded)
		return;

	current = input;
	rootMotion = glm::vec3(0.0f);

	// ---- Recovery (before the primary: it takes over the clip of the attack that just ended) ----
	if (input.recovery.serial != recoverySerial)
	{
		if (input.recovery.serial != 0)
			BeginRecovery(input.recovery);
		else if (activeRecovery)
			recoveryFading = true;   // the clip ended (or a new move replaced it - dropped below)
		recoverySerial = input.recovery.serial;
	}
	else if (activeRecovery && recoverySerial != 0)
	{
		Sample(activeRecovery, input.recovery.time);
		const MathHelper::Transform delta = activeRecovery->PullRootMotion();
		if (input.recovery.rootMotion && !input.recovery.released && !recoveryReleased)
			rootMotion += delta.Position;   // released: the capsule follows input instead
	}

	// ---- Primary layer ----
	if (input.primary.serial != primarySerial)
	{
		if (input.primary.serial != 0)
		{
			BeginSnapshot(input.primary.blendIn);   // from the last output, recovery included
			DropRecovery();                          // the new move owns the body (and may reuse the clip source)
			StartLayerClip(primaryLayer, input.primary, activePrimary, true);
			pendingBlendOut = input.primary.blendOut;
			driveTransition = input.primary.blendIn;
		}
		else
		{
			BeginSnapshot(pendingBlendOut);
			activePrimary = nullptr;
			driveTransition = pendingBlendOut;
		}
		primarySerial = input.primary.serial;
	}
	else if (activePrimary)
	{
		Sample(activePrimary, input.primary.time);
		const MathHelper::Transform delta = activePrimary->PullRootMotion();
		if (input.primary.rootMotion)
			rootMotion += delta.Position;
	}

	// ---- Overlay layer ----
	if (input.overlay.serial != overlaySerial)
	{
		if (input.overlay.serial != 0)
		{
			// Another overlay clip takes over a visible one (a shot restarting its recoil clip, a draw after firing):
			// blend from where the previous one was instead of snapping.
			overlaySnapshotActive = activeOverlay != nullptr && overlayWeight > 0.001f && input.overlay.blendIn > 0.001f;
			if (overlaySnapshotActive)
			{
				overlaySnapshot = activeOverlay->GetAnimationPose();
				overlaySnapshotDuration = input.overlay.blendIn;
				overlaySnapshotElapsed = 0.0f;
			}
			StartLayerClip(overlayLayer, input.overlay, activeOverlay, false);
			overlayFadeRate = 1.0f / std::max(input.overlay.blendIn, 0.01f);
			overlayBlendOut = input.overlay.blendOut;
		}
		else
		{
			overlayFadeRate = 1.0f / std::max(overlayBlendOut, 0.01f);
		}
		overlaySerial = input.overlay.serial;
	}
	else if (activeOverlay && input.overlay.serial != 0)
	{
		Sample(activeOverlay, input.overlay.time);
	}

	if (overlaySnapshotActive)
	{
		overlaySnapshotElapsed += deltaTime;
		if (overlaySnapshotElapsed >= overlaySnapshotDuration)
			overlaySnapshotActive = false;
	}

	const float overlayTarget = (activeOverlay && input.overlay.serial != 0) ? 1.0f : 0.0f;
	overlayWeight = Approach(overlayWeight, overlayTarget, overlayFadeRate, deltaTime);
	if (overlayWeight <= 0.0f && input.overlay.serial == 0)
		activeOverlay = nullptr;

	aimWeight = Approach(aimWeight, input.aiming ? 1.0f : 0.0f, 1.0f / AimBlendTime, deltaTime);

	UpdateRecoveryWeights(deltaTime);

	// ---- Locomotion and the weapon stance ----
	UpdateLocomotionWeights(deltaTime);
	UpdateLean(deltaTime);
	UpdateStanceWeights(deltaTime);

	if (snapshotActive)
	{
		snapshotElapsed += deltaTime;
		if (snapshotElapsed >= snapshotDuration)
			snapshotActive = false;
	}
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

	for (LocomotionClip* clip : { &runForward, &runBackward, &runLeft, &runRight })
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
	for (size_t slot = 0; slot < stances.size(); ++slot)
	{
		Stance& stance = stances[slot];
		const float target = static_cast<int>(slot) == current.stance ? 1.0f : 0.0f;

		// The first frame starts in the current stance instead of fading in from the locomotion idle.
		if (!stancesStarted || stance.blend <= 0.0f)
			stance.fade = target;
		else
			stance.fade = Approach(stance.fade, target, 1.0f / stance.blend, deltaTime);
		stance.weight = glm::smoothstep(0.0f, 1.0f, stance.fade);

		if (stance.weight <= 0.001f)
			continue;

		// Same clock as the locomotion idle: standing still, the hold layer matches the stance idle.
		if (stance.idle.animation)
			Sample(stance.idle.animation, LoopTime(idleTime, stance.idle.duration));
		if (stance.hold.animation && stance.hold.animation != stance.idle.animation)
			Sample(stance.hold.animation, LoopTime(idleTime, stance.hold.duration));
	}
	stancesStarted = true;
}

AnimationPose CharacterAnimator::ProcessResultPose()
{
	if (!idle.animation)
		return AnimationPose();

	roj::BoneNode* rootNode = idle.animation->GetRootNode();
	AnimationPose completion;   // scratch for clips that need completing

	// A clip's pose ready to blend over `base`: as is when its model has every body bone, else completed from base.
	auto blendable = [&](Animation* animation, const AnimationPose& base) -> const AnimationPose&
	{
		if (IsComplete(animation))
			return animation->GetAnimationPoseRef();
		completion = CompleteFrom(animation->GetAnimationPoseRef(), base);
		return completion;
	};

	// ---- Idle: the stance idles replace the locomotion idle (their weights sum to 1 once a weapon is held) ----
	AnimationPose pose = idle.animation->GetAnimationPose();
	{
		float total = 0.0f;
		for (const Stance& stance : stances)
			if (stance.idle.animation)
				total += stance.weight;

		float idleAccumulated = std::max(0.0f, 1.0f - total);
		for (const Stance& stance : stances)
		{
			if (!stance.idle.animation || stance.weight <= 0.001f)
				continue;
			idleAccumulated += stance.weight;
			pose = AnimationPose::Lerp(pose, blendable(stance.idle.animation, pose), stance.weight / idleAccumulated);
		}
	}

	// ---- Locomotion blend (procedural strafe: each run turned toward its legs' direction before blending) ----
	float accumulated = idle.weight;

	auto blendIn = [&](const LocomotionClip& clip, float twist)
	{
		if (!clip.animation || clip.weight <= 0.001f)
			return;
		const float total = accumulated + clip.weight;
		if (std::abs(twist) > 1e-4f && rootNode)
		{
			AnimationPose turned = blendable(clip.animation, pose);
			ApplyLegTwist(turned, rootNode, twist);
			pose = AnimationPose::Lerp(pose, turned, clip.weight / total);
		}
		else
		{
			pose = AnimationPose::Lerp(pose, blendable(clip.animation, pose), clip.weight / total);
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
		pose = AnimationPose::Lerp(pose, blendable(fall.animation, pose), airWeight);

	Animation* glidePose = glide.animation ? glide.animation : fall.animation;
	if (glidePose && glideWeight > 0.001f)
		pose = AnimationPose::Lerp(pose, blendable(glidePose, pose), glideWeight);

	const hashed_string upperBodyBone(data->locomotion.upperBodyBone);

	// Partial layers: the engine's blends take the root bone from the layer, the legs keep theirs. A layer rooted at
	// the upper-body bone blends that bone's rotation in mesh space, so the torso keeps the layer's facing whatever
	// the legs do; a limb layer follows the torso (local space). Incomplete layers are completed from the pose first.
	auto layerBranch = [&](const hashed_string& bone, const AnimationPose& layer, bool layerComplete, float weight)
	{
		auto root = pose.boneTransforms.find(RootBone);
		const bool hasRoot = root != pose.boneTransforms.end();
		const glm::mat4 baseRoot = hasRoot ? root->second : glm::mat4(1.0f);

		const float meshSpace = bone == upperBodyBone ? 1.0f : 0.0f;
		if (layerComplete)
			pose = AnimationPose::LayeredLerp(bone, rootNode, pose, layer, meshSpace, weight);
		else
			pose = AnimationPose::LayeredLerp(bone, rootNode, pose, CompleteFrom(layer, pose), meshSpace, weight);

		if (hasRoot)
			pose.boneTransforms[RootBone] = baseRoot;
	};

	// A move layer on the upper body, or on the move's mask bones (BodyMask::Bones).
	auto layerMove = [&](const AnimatorInput::Layer& input, const AnimationPose& layer, bool layerComplete, float weight)
	{
		if (input.mask == BodyMask::Bones && input.maskBones != nullptr)
		{
			for (const hashed_string& bone : *input.maskBones)
				layerBranch(bone, layer, layerComplete, weight);
		}
		else
		{
			layerBranch(upperBodyBone, layer, layerComplete, weight);
		}
	};

	// ---- Stance hold: how the weapon in the hands is carried during locomotion ----
	// Weights are normalized like the idles, so a cross-fade between two weapons does not dip into the base.
	if (rootNode)
	{
		float total = 0.0f;
		for (const Stance& stance : stances)
			if (stance.hold.animation)
				total += stance.weight;

		float holdAccumulated = std::max(0.0f, 1.0f - total);
		for (const Stance& stance : stances)
		{
			if (!stance.hold.animation || stance.weight <= 0.001f)
				continue;
			holdAccumulated += stance.weight;

			const AnimationPose& hold = stance.hold.animation->GetAnimationPoseRef();
			const bool complete = IsComplete(stance.hold.animation);
			for (const hashed_string& bone : stance.holdBones)
				layerBranch(bone, hold, complete, stance.weight / holdAccumulated);
		}
	}

	// ---- Full-body primary: replaces locomotion (move clips pose the arms themselves). NoLegs blends the legs toward
	// the locomotion pose (the air pose in the air) by legWeight, local to the clip's hips: the hips follow the attack
	// and a little of its leg motion stays. A Full move does the same in the air (airWeight fades it in and out with
	// take-off and landing), unless its clip poses its own air legs ----
	const bool fullPrimary = activePrimary
		&& (current.primary.mask == BodyMask::Full || current.primary.mask == BodyMask::NoLegs || !rootNode);
	if (fullPrimary)
	{
		float legBlend = 0.0f;
		if (current.primary.mask == BodyMask::NoLegs)
			legBlend = current.primary.legWeight;
		else if (current.primary.mask == BodyMask::Full && current.primary.airLegs)
			legBlend = current.primary.legWeight * airWeight;
		const bool keepLegs = rootNode && legBlend > 0.001f;
		AnimationPose locomotionPose;
		if (keepLegs)
			locomotionPose = pose;

		if (IsComplete(activePrimary))
			pose = activePrimary->GetAnimationPose();
		else
			pose = CompleteFrom(activePrimary->GetAnimationPoseRef(), pose);

		if (keepLegs)
			for (const hashed_string& leg : data->locomotion.legBoneHashes)
				pose = AnimationPose::LayeredLerp(leg, rootNode, pose, locomotionPose, 0.0f, legBlend);
	}

	// ---- Recovery: the whole body until the body moves, then the legs and the upper body hand over ----
	if (activeRecovery && rootNode)
	{
		const AnimationPose& recoveryPose = blendable(activeRecovery, pose);

		// Lerp takes the root from its second pose: locomotion keeps the root while the legs hand over.
		if (recoveryLowerWeight > 0.001f)
			pose = AnimationPose::Lerp(recoveryPose, pose, 1.0f - recoveryLowerWeight);

		if (recoveryUpperWeight > 0.001f && recoveryLowerWeight < 0.999f)
			layerBranch(upperBodyBone, recoveryPose, true, recoveryUpperWeight);
	}

	// ---- Upper-body / masked primary, then the overlay (it runs with primaries that allow it: on top) ----
	if (activePrimary && !fullPrimary)
		layerMove(current.primary, activePrimary->GetAnimationPoseRef(), IsComplete(activePrimary), 1.0f);

	if (activeOverlay && overlayWeight > 0.001f && rootNode)
	{
		if (overlaySnapshotActive)
			layerMove(current.overlay, AnimationPose::Lerp(overlaySnapshot, CompleteFrom(activeOverlay->GetAnimationPoseRef(), overlaySnapshot),
				glm::smoothstep(0.0f, 1.0f, overlaySnapshotElapsed / overlaySnapshotDuration)), false, overlayWeight);
		else
			layerMove(current.overlay, activeOverlay->GetAnimationPoseRef(), IsComplete(activeOverlay), overlayWeight);
	}

	// ---- Aim offset of the weapon in the hands, then the grip of the hands holding it ----
	if (rootNode)
	{
		ApplyAim(pose, rootNode);
		ApplyGrip(pose, rootNode);
	}

	// ---- Transition from the frozen pose ----
	if (snapshotActive && snapshotDuration > 0.0f)
	{
		pose = AnimationPose::Lerp(snapshot, pose, glm::smoothstep(0.0f, 1.0f, snapshotElapsed / snapshotDuration));
	}

	// ---- Root motion is applied to the capsule, not the mesh ----
	auto root = pose.boneTransforms.find(RootBone);
	if (root != pose.boneTransforms.end())
	{
		root->second[3].x = 0.0f;
		root->second[3].z = 0.0f;
	}

	lastOutput = pose;
	hasLastOutput = true;

	if (rootNode)
		ApplyHitRecoil(pose, rootNode);
	return pose;
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

void CharacterAnimator::ApplyAim(AnimationPose& pose, roj::BoneNode* rootNode) const
{
	// The stance's hold pose aims along the body's facing; its aim bones turn it toward the aim point: yaw about up,
	// then pitch, each clamped, spread evenly over the bones (parent first, each about its own origin).
	const float fade = glm::smoothstep(0.0f, 1.0f, aimWeight);
	if (fade <= 0.001f)
		return;

	glm::vec3 reference(current.forward.x, 0.0f, current.forward.z);
	if (glm::dot(reference, reference) < 1e-6f || glm::dot(current.aimDirection, current.aimDirection) < 1e-6f)
		return;
	reference = glm::normalize(reference);

	const glm::vec3 aim = glm::normalize(current.aimDirection);
	const glm::vec3 aimFlat(aim.x, 0.0f, aim.z);
	const float flatLength = glm::length(aimFlat);
	const float yaw = flatLength > 1e-3f ? YawBetween(reference, aimFlat / flatLength) : 0.0f;
	const float pitch = std::atan2(aim.y, flatLength);
	const glm::vec3 pitchAxis = glm::normalize(glm::cross(reference, Up));

	for (const Stance& stance : stances)
	{
		if (stance.aimBones.empty() || stance.weight <= 0.001f)
			continue;

		const float share = stance.weight * fade / static_cast<float>(stance.aimBones.size());
		const float yawStep = glm::clamp(yaw, -glm::radians(stance.aimMaxYaw), glm::radians(stance.aimMaxYaw)) * share;
		const float pitchStep = glm::clamp(pitch, -glm::radians(stance.aimMaxPitch), glm::radians(stance.aimMaxPitch)) * share;
		const glm::mat3 step = glm::mat3_cast(glm::angleAxis(yawStep, Up) * glm::angleAxis(pitchStep, pitchAxis));

		for (const hashed_string& bone : stance.aimBones)
			RotateInModelSpace(pose, rootNode, bone, step);
	}
}

// The hands holding the weapon take the grip of the clip posing it, in local space (the arm keeps whatever the
// layers made of it): this weapon's overlay clip, else its move / recovery clip, else its hold clip. Weighted by
// the stance (two weapons cross-fading each keep their own grip) and by how much those clips show.
void CharacterAnimator::ApplyGrip(AnimationPose& pose, roj::BoneNode* rootNode) const
{
	auto ownedBy = [this](const Animation* animation, int slot)
	{
		auto it = clipWeapons.find(animation);
		return animation != nullptr && it != clipWeapons.end() && it->second == slot;
	};

	float total = 0.0f;
	for (const Stance& stance : stances)
		if (!stance.handBones.empty() && stance.hold.animation)
			total += stance.weight;
	float accumulated = std::max(0.0f, 1.0f - total);

	for (size_t slot = 0; slot < stances.size(); ++slot)
	{
		const Stance& stance = stances[slot];
		if (stance.handBones.empty() || !stance.hold.animation || stance.weight <= 0.001f)
			continue;
		accumulated += stance.weight;
		const float share = stance.weight / accumulated;

		const Animation* moveClip = activePrimary ? activePrimary : activeRecovery;
		const float moveWeight = activePrimary ? 1.0f : recoveryUpperWeight;

		struct Source { const Animation* clip; float weight; };
		const Source sources[] = {
			{ stance.hold.animation, share },
			{ ownedBy(moveClip, static_cast<int>(slot)) ? moveClip : nullptr, share * moveWeight },
			{ ownedBy(activeOverlay, static_cast<int>(slot)) ? activeOverlay : nullptr, share * overlayWeight },
		};

		for (const Source& source : sources)
		{
			if (source.clip == nullptr || source.weight <= 0.001f)
				continue;
			Animation* clip = const_cast<Animation*>(source.clip);
			const AnimationPose& grip = clip->GetAnimationPoseRef();
			const AnimationPose completed = IsComplete(clip) ? AnimationPose() : CompleteFrom(grip, pose);
			const AnimationPose& layer = IsComplete(clip) ? grip : completed;

			for (const hashed_string& hand : stance.handBones)
			{
				auto root = pose.boneTransforms.find(RootBone);
				const bool hasRoot = root != pose.boneTransforms.end();
				const glm::mat4 baseRoot = hasRoot ? root->second : glm::mat4(1.0f);
				pose = AnimationPose::LayeredLerp(hand, rootNode, pose, layer, 0.0f, source.weight);
				if (hasRoot)
					pose.boneTransforms[RootBone] = baseRoot;
			}
		}
	}
}

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

	if (activePrimary)
	{
		drive.move = &activePrimary->GetAnimationPoseRef();
		drive.moveWeight = 1.0f;
		drive.moveSerial = primarySerial;
	}
	else if (activeRecovery)
	{
		drive.move = &activeRecovery->GetAnimationPoseRef();
		drive.moveWeight = recoveryUpperWeight;
		drive.moveSerial = recoveryMoveSerial;
	}

	if (activeOverlay && overlayWeight > 0.001f)
	{
		drive.overlay = &activeOverlay->GetAnimationPoseRef();
		drive.overlayWeight = overlayWeight;
	}
	return drive;
}
