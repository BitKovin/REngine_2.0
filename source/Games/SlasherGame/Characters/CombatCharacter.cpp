#include "CombatCharacter.h"

#include <AI/AIController.h>
#include <Combat/CombatEvents.h>
#include <Combat/CombatFrame.h>
#include <Combat/CombatRules.h>
#include <Control/Controller.h>
#include <Control/PlayerController.h>
#include <Debug/MoveTimeline.h>
#include <Data/CombatData.h>

#include <Camera.h>
#include <Entities/SoundPlayer.h>
#include <Logger.hpp>
#include <Particle/GlobalParticleSystem.hpp>
#include <Physics.h>
#include <Time.hpp>
#include <imgui/imgui.h>
#include <tracy/tracy/Tracy.hpp>

#include <algorithm>
#include <cmath>

namespace
{
	constexpr float MinSettleTurnRate = 90.0f;   // deg/s: eased turns still finish
	constexpr float MinJudgedRise = 0.15f;       // m/s (physics): slower rises are zeroed by the controller on the ground
	constexpr float MaxLeanAcceleration = 40.0f; // m/s^2 reported to the animator (dash starts would spike it)

	glm::vec3 MoveTowards(const glm::vec3& current, const glm::vec3& target, float maxDelta)
	{
		const glm::vec3 delta = target - current;
		const float distance = glm::length(delta);
		if (distance <= maxDelta || distance < 1e-6f)
			return target;
		return current + delta / distance * maxDelta;
	}

	CharacterControllerMovementQuality ToEngineQuality(MovementQuality quality)
	{
		switch (quality)
		{
		case MovementQuality::Player:        return CharacterControllerMovementQuality::Player;
		case MovementQuality::NpcLowQuality: return CharacterControllerMovementQuality::NpcLowQuality;
		default:                             return CharacterControllerMovementQuality::NpcGeneric;
		}
	}
}

CombatCharacter::CombatCharacter(std::string characterDataPath)
	: dataPath(std::move(characterDataPath)), animator(this)
{
	ClassName = "combat_character";

	mesh = new SkeletalMesh(this);
	Drawables.push_back(mesh);

	moves.Initialize(this);
	weapons.Initialize(this);
}

CombatCharacter::~CombatCharacter() = default;

void CombatCharacter::FromData(EntityData entityData)
{
	Entity::FromData(entityData);

	const std::string mapDefinition = entityData.GetPropertyString("def", "");
	if (!mapDefinition.empty())
		dataPath = mapDefinition;

	Rotation.y = entityData.GetPropertyFloat("angle") + 90.0f;
}

void CombatCharacter::LoadAssets()
{
	CombatData::LoadScope scope;

	if (dataPath.empty())
	{
		Logger::Error("[CombatCharacter] '%s' has no character data path", Id.c_str());
		return;
	}

	data = CombatData::LoadCharacter(dataPath);
	statusEffects = CombatData::LoadStatusEffects();
	if (!data)
		return;

	const CharacterMeshData& meshData = data->mesh;
	if (!meshData.textures.empty())
		mesh->TexturesLocation = meshData.textures;
	mesh->LoadFromFile(meshData.model);
	mesh->TwoSided = true;
	if (!meshData.meta.empty())
		mesh->LoadMetaFromFile(meshData.meta);
	mesh->Scale = glm::vec3(meshData.scale);
	mesh->PreloadAssets();

	animator.Configure(data, mesh->model ? &mesh->model->skeleton : nullptr);
	animator.LoadAssetsIfNeeded();

	weapons.LoadAssets(*data);
}

void CombatCharacter::Start()
{
	if (started)
		return;

	LoadAssetsIfNeeded();   // spawned at runtime (Spawner): data is already cached by the type preload
	Entity::Start();
	started = true;

	if (!data)
	{
		Logger::Error("[CombatCharacter] '%s' failed to load '%s' - removing it", Id.c_str(), dataPath.c_str());
		Destroy();
		return;
	}

	yaw = MathHelper::NormalizeAngle(Rotation.y);

	const MovementData& m = data->movement;
	movement.Init(this, Position, m.radius, m.height, m.mass);
	movement.gravity = 0.0f;   // gravity is integrated in local time by ApplyMovement (hit-stop, glide)
	movement.movementQuality = ToEngineQuality(m.quality);

	ConfigureComponents();
	CreateHitboxes();
	moves.SetMoveSet(data->moveSet);

	aiController = std::make_unique<AIController>(*this);
	aiController->SetBrain(data->ai.brain);
	if (controller == nullptr)
		SetController(aiController.get());

	adoptedDataGeneration = CombatData::GetGeneration();
}

void CombatCharacter::ConfigureComponents()
{
	MaxHealth = data->health;
	Health = data->health;

	auto team = GetTeam();
	team->team = data->team;
	team->originalTeam = data->team;

	*GetCombatState() = CombatStateComponent();
	GetPoise()->Configure(data->poise);
	GetWeakPoints()->Configure(data);

	auto effects = GetStatusEffects();
	effects->table = statusEffects;
	effects->Clear();

	GetAttackTokens()->Configure(data->attackTokens);
}

void CombatCharacter::CreateHitboxes()
{
	if (data->mesh.hitboxes)
		mesh->CreateHitboxes(this);

	if (!mesh->hitboxBodies.empty())
		return;

	// No hitbox data for this model: one capsule hitbox that follows the body keeps it hittable.
	const MovementData& m = data->movement;
	fallbackHitbox = Physics::CreateCharacterBody(this, Position, m.radius * 0.9f, m.height * 0.9f, 1.0f, BodyType::HitBox, BodyType::None);
	fallbackHitbox->SetMotionType(JPH::EMotionType::Kinematic);
	Physics::SetObjectLayer(fallbackHitbox, Layers::TRACE_ONLY);   // like model hitboxes: found by sweeps, never simulated
	Bodies.push_back(fallbackHitbox);
}

// ---- Control ----------------------------------------------------------------------------------------------

void CombatCharacter::SetController(Controller* newController)
{
	if (controller == newController)
		return;

	if (controller)
		controller->OnUnpossess(*this);

	input.ReleaseAll(Time::GameTime);
	controller = newController;

	if (controller)
		controller->OnPossess(*this);
}

bool CombatCharacter::IsPlayerControlled() const
{
	return controller != nullptr && controller->IsPlayerController();
}

// ---- State ------------------------------------------------------------------------------------------------

bool CombatCharacter::IsDead() const
{
	auto state = const_cast<CombatCharacter*>(this)->GetCombatState();
	return state && state->dead;
}

bool CombatCharacter::IsBroken() const
{
	auto poise = const_cast<CombatCharacter*>(this)->GetPoise();
	return poise && poise->IsBroken();
}

Team CombatCharacter::GetTeamId() const
{
	auto team = const_cast<CombatCharacter*>(this)->GetTeam();
	return team ? team->team : Team::Neutral;
}

void CombatCharacter::SetTeamId(Team team)
{
	GetTeam()->team = team;
}

float CombatCharacter::GetTimeDilation() const
{
	auto state = const_cast<CombatCharacter*>(this)->GetCombatState();
	return state ? state->GetTimeDilation() : 1.0f;
}

float CombatCharacter::GetLocalDeltaTime() const
{
	return Time::DeltaTimeF * GetTimeDilation();
}

float CombatCharacter::GetAttackSpeedMultiplier() const
{
	auto effects = const_cast<CombatCharacter*>(this)->GetStatusEffects();
	return effects ? effects->modifiers.attackSpeed : 1.0f;
}

float CombatCharacter::GetMoveSpeedMultiplier() const
{
	auto effects = const_cast<CombatCharacter*>(this)->GetStatusEffects();
	return effects ? effects->modifiers.moveSpeed : 1.0f;
}

const WeaponData* CombatCharacter::GetAimingWeapon() const
{
	if (IsDead())
		return nullptr;

	const MoveSlot& primary = moves.GetPrimary();
	if (primary.IsActive() && primary.move->overlay != OverlayPolicy::Allow)
		return nullptr;

	const WeaponData* weapon = weapons.GetWeapon(weapons.GetActiveSlot());
	return weapon != nullptr && !weapon->stance.aimBones.empty() ? weapon : nullptr;
}

// ---- Combat frame -----------------------------------------------------------------------------------------

void CombatCharacter::BeginCombatFrame()
{
	Position = movement.GetSmoothPosition();

	const bool wasGrounded = grounded;
	grounded = movement.onGround;
	if (grounded && !wasGrounded)
	{
		resources = MovementResources();
		airCombatRemaining = 0.0f;
	}

	requests = FrameRequests();
	input.BeginFrame();
}

void CombatCharacter::UpdateCombat()
{
	const float realDeltaTime = Time::DeltaTimeF;
	const float deltaTime = GetLocalDeltaTime();

	if (controller != nullptr && !IsDead())
	{
		ZoneScopedN("Body: controller");
		controller->UpdateInput(*this, input, realDeltaTime);
	}

	{
		ZoneScopedN("Body: moves");
		moves.Tick(deltaTime);
		UpdateRecoveryRelease();
	}
	{
		ZoneScopedN("Body: weapons");
		weapons.Tick(deltaTime);
		UpdateWeaponReturn();
	}
	{
		ZoneScopedN("Body: facing + aim");
		UpdateFacing(deltaTime);
		UpdateAim();
	}
}

void CombatCharacter::FlushCombatFrame()
{
	for (const HitRequestEvent& hit : queuedHits)
		EcsScheduler::Emit(hit);
	queuedHits.clear();

	for (const CueData* cue : queuedCues)
		PlayCueNow(*cue);
	queuedCues.clear();

	weapons.FlushShots();

	if (!queuedActions.empty())
	{
		// An action may start moves that queue nothing (we are on the main thread now), but keep the loop safe.
		std::vector<std::function<void()>> actions;
		actions.swap(queuedActions);
		for (const std::function<void()>& action : actions)
			action();
	}

	if (IsDead() && data->corpseLifetime > 0.0f && !IsPlayerControlled() && !corpseDelay.Wait())
	{
		Destroy();
		return;
	}

	if (!moves.IsPrimaryActive())
		AdoptReloadedData();
}

void CombatCharacter::RunOnMainThread(std::function<void()> fn)
{
	if (CombatFrame::IsParallel())
		queuedActions.push_back(std::move(fn));
	else
		fn();
}

void CombatCharacter::QueueHit(const HitRequestEvent& hit)
{
	if (CombatFrame::IsParallel())
		queuedHits.push_back(hit);
	else
		EcsScheduler::Emit(hit);
}

// ---- Pose frame (CombatBodyPoseSystem) --------------------------------------------------------------------

// Pose level of detail (COMBAT_DESIGN.md §2.4). The controlled body and attacking bodies evaluate every frame (their
// blades sweep along the pose); the others by distance from the camera, and at the far rate while the camera does not
// see them. Far and unseen bodies follow the ground with the controller's line traces instead of its shape cast.
void CombatCharacter::UpdatePoseDetail()
{
	const MoveSlot& primary = moves.GetPrimary();
	const bool attacking = primary.IsActive() && primary.move->HasTag(MoveTag::Attack);
	if (IsPlayerControlled() || attacking)
	{
		animationInterval = 1;
	}
	else
	{
		const AnimationLodData& lod = data->animationLod;
		const float distance = glm::distance(Position, Camera::position);
		animationInterval = distance < lod.halfRateDistance ? 1 : (distance < lod.quarterRateDistance ? 2 : 4);
		if (!mesh->IsCameraVisible())   // last rendered frame's culling
			animationInterval = 4;
	}

	movement.movementQuality = animationInterval == 4 ? CharacterControllerMovementQuality::NpcLowQuality : ToEngineQuality(data->movement.quality);
}

void CombatCharacter::UpdatePose()
{
	if (!IsCombatReady())
		return;

	const float deltaTime = GetLocalDeltaTime();

	// Far bodies evaluate their pose at a reduced rate; the clips are sampled at the move clock, so a late
	// evaluation shows exactly where the move is. Root motion becomes a velocity over the evaluated span.
	UpdatePoseDetail();
	animationPendingTime += deltaTime;
	if (++animationFramesSinceEvaluate >= animationInterval || animator.GetPose().boneTransforms.empty())
	{
		ZoneScopedN("Body: animation");
		AnimatorInput animatorInput;
		BuildAnimatorInput(animatorInput);
		{
			ZoneScopedN("Body: animator evaluate");
			animator.Evaluate(animatorInput, animationPendingTime);
		}
		const AnimationPose& pose = animator.ComposePose();   // the animator's storage, no copy

		const glm::vec3 rootMotion = animator.ConsumeRootMotion();
		rootMotionVelocity = animationPendingTime > 1e-6f ? rootMotion / animationPendingTime : glm::vec3(0.0f);
		animationPendingTime = 0.0f;
		animationFramesSinceEvaluate = 0;

		if (!pose.boneTransforms.empty())
		{
			ZoneScopedN("Body: paste pose");
			mesh->PasteAnimationPose(pose);
		}
	}

	{
		ZoneScopedN("Body: movement");
		ApplyMovement(deltaTime);
	}
	hitRecoil.Step(deltaTime, data->hitRecoil.frequency, data->hitRecoil.damping);

	glm::vec3 shake(0.0f);
	if (auto state = GetCombatState())
		shake = state->GetShakeOffset();
	mesh->Position = GetFeetPosition() + shake;
	mesh->Rotation = glm::vec3(0.0f, yaw + data->mesh.yawOffset, 0.0f);

	{
		ZoneScopedN("Body: hitboxes");
		if (fallbackHitbox)
			Physics::SetBodyPosition(fallbackHitbox, Position);
		else
			mesh->UpdateHitboxes();
	}

	ZoneScopedN("Body: weapon pose");
	UpdateWeaponReach();
	weapons.AsyncUpdate(*mesh, animator.GetPose(), animator.GetWeaponDrive(), deltaTime);
}

// Weapon length from the reach of the moves playing (MoveData::reach): the primary - or the attack playing out in
// recovery - and the overlay; the longest reach per weapon wins.
void CombatCharacter::UpdateWeaponReach()
{
	std::array<float, static_cast<size_t>(WeaponSlot::Count)> reach{};
	auto add = [&](const MoveData* move, float time)
	{
		if (move != nullptr && move->reach.IsActive())
		{
			float& length = reach[static_cast<size_t>(move->reach.weapon)];
			length = std::max(length, move->reach.At(time));
		}
	};

	const MoveSlot& primary = moves.GetPrimary();
	const RecoverySlot& recovery = moves.GetRecovery();
	if (primary.IsActive())
		add(primary.move, primary.time);
	else if (recovery.IsActive())
		add(recovery.move, recovery.time);
	const MoveSlot& overlay = moves.GetOverlay();
	if (overlay.IsActive())
		add(overlay.move, overlay.time);

	for (size_t slot = 0; slot < reach.size(); ++slot)
		weapons.SetExtension(static_cast<WeaponSlot>(slot), reach[slot]);
}

void CombatCharacter::LateUpdate()
{
	if (!IsCombatReady())
		return;

	weapons.LateUpdate();
}

void CombatCharacter::Destroy()
{
	if (Destroyed)
		return;

	if (started)
	{
		moves.StopAll(MoveEndReason::Interrupted);
		weapons.Destroy();

		if (controller)
			controller->OnUnpossess(*this);
		controller = nullptr;

		if (aiController)
			aiController->OnBodyDestroyed();

		movement.Destroy();
	}

	queuedHits.clear();
	queuedCues.clear();
	queuedActions.clear();

	Entity::Destroy();
}

void CombatCharacter::Teleport(vec3 position)
{
	Position = position;
	if (started)
		movement.SetPosition(position);
	velocity = glm::vec3(0.0f);
}

// A weapon other than Main goes back once its holsterDelay ran out (COMBAT_DESIGN.md §9.3): the Draw action plays
// when nothing on the overlay channel runs (firing keeps it in the hands), or the swap is instant without one.
void CombatCharacter::UpdateWeaponReturn()
{
	const WeaponSlot active = weapons.GetActiveSlot();
	if (active == WeaponSlot::Main || !weapons.HasWeapon(WeaponSlot::Main) || !weapons.IsHolsterDue(active) || IsDead())
		return;
	if (moves.GetOverlay().IsActive())
		return;

	const MoveSetData* moveSet = moves.GetMoveSet();
	const int draw = moveSet ? moveSet->GetActionMove(ActionSlot::Draw) : -1;
	if (draw >= 0)
		moves.TryStartMove(draw);   // refused while a primary move blocks overlays: retried next frame
	else if (!moves.IsPrimaryActive())
		weapons.SetActiveSlot(WeaponSlot::Main);
}

// A recovering attack plays out on the whole body until the body moves (COMBAT_DESIGN.md §5.5).
void CombatCharacter::UpdateRecoveryRelease()
{
	if (!moves.IsRecovering() || moves.GetRecovery().released)
		return;

	const bool moving = glm::length(MathHelper::XZ(input.move)) > 0.1f;
	if (moving || !grounded || IsDead())
		moves.ReleaseRecovery();
}

// The weapon in the hands aims at the controller's aim point (the crosshair for the player, the target for AI)
// when its stance has aim bones and no move overrides it; the animator turns those bones toward it (§9.1).
void CombatCharacter::UpdateAim()
{
	hasAimPoint = GetAimingWeapon() != nullptr;
	if (hasAimPoint)
		aimPoint = GetAimPoint();
}

void CombatCharacter::BuildAnimatorInput(AnimatorInput& out) const
{
	out.localVelocity = glm::vec2(glm::dot(velocity, GetRight()), glm::dot(velocity, GetForward()));
	out.grounded = grounded;
	out.gliding = requests.gliding;
	out.stance = static_cast<int>(weapons.GetActiveSlot());
	out.aiming = hasAimPoint;
	out.strafing = hasAimPoint || requests.hasFacing;   // facing set by the aim or a move / brain, not by the movement

	// Model space = the mesh's space: undo the mesh yaw (facing + the model's yaw offset).
	const glm::vec3 toModel(0.0f, -(yaw + data->mesh.yawOffset), 0.0f);
	out.forward = MathHelper::RotateVector(GetForward(), toModel);
	out.velocity = MathHelper::RotateVector(MathHelper::XZ(velocity), toModel);
	out.acceleration = MathHelper::RotateVector(acceleration, toModel);
	out.recoil = MathHelper::RotateVector(hitRecoil.value, toModel);
	if (hasAimPoint)
	{
		const glm::vec3 chest = Position + glm::vec3(0.0f, GetCapsuleHeight() * 0.35f, 0.0f);
		const glm::vec3 toAim = aimPoint - chest;
		out.aimDirection = glm::dot(toAim, toAim) > 1e-4f ? MathHelper::RotateVector(glm::normalize(toAim), toModel) : out.forward;
	}
	else
	{
		out.aimDirection = out.forward;
	}

	auto fill = [](AnimatorInput::Layer& layer, const MoveSlot& slot)
	{
		if (!slot.IsActive())
			return;
		const MoveData& move = *slot.move;
		layer.animation = move.animationIndex;
		layer.clip = move.GetClipHash(slot.clip);
		layer.time = slot.time;
		layer.serial = slot.serial;
		layer.blendIn = slot.GetBlendIn();
		layer.blendOut = move.resolvedBlendOut;
		layer.mask = move.bodyMask;
		layer.maskBones = move.bodyMask == BodyMask::Bones ? &move.maskBoneHashes : nullptr;
		layer.legWeight = move.legWeight;
		layer.airLegs = move.airLegs;
		layer.rootMotion = move.rootMotion.enabled;
	};

	fill(out.primary, moves.GetPrimary());
	fill(out.overlay, moves.GetOverlay());

	const RecoverySlot& recovery = moves.GetRecovery();
	if (recovery.IsActive())
	{
		const MoveData& move = *recovery.move;
		out.recovery.animation = move.animationIndex;
		out.recovery.clip = move.GetClipHash(recovery.clip);
		out.recovery.time = recovery.time;
		out.recovery.serial = recovery.serial;
		out.recovery.released = recovery.released;
		out.recovery.rootMotion = move.rootMotion.enabled;
		out.recovery.lowerBodyBlend = move.recovery.lowerBodyBlend;
		out.recovery.upperBodyBlend = move.recovery.upperBodyBlend;
		out.recovery.blendOut = move.resolvedBlendOut;
	}
}

void CombatCharacter::ApplyMovement(float deltaTime)
{
	const MovementData& m = data->movement;
	const float dilation = GetTimeDilation();

	// The physics step moved the capsule with the velocity set last frame; if something blocked upward motion
	// (ceiling), adopt the reduced vertical speed instead of pushing into it.
	velocity.y = ResolveRise(velocity.y, appliedDilation, movement.GetVelocity().y);

	const glm::vec3 previousHorizontal = MathHelper::XZ(velocity);
	glm::vec3 horizontal = previousHorizontal;
	float vertical = velocity.y;

	const MoveSlot& primary = moves.GetPrimary();
	const float locomotionScale = requests.locomotionScale >= 0.0f ? requests.locomotionScale : (primary.IsActive() ? 0.0f : 1.0f);

	// The clip moving the capsule: the primary move's, or a recovering attack's until the body moves (§5.5).
	const MoveData* rootMotionMove = nullptr;
	if (primary.IsActive())
	{
		if (primary.move->rootMotion.enabled && locomotionScale <= 0.0f)
			rootMotionMove = primary.move;
	}
	else if (const RecoverySlot& recovery = moves.GetRecovery(); recovery.IsActive() && !recovery.released && recovery.move->rootMotion.enabled)
	{
		rootMotionMove = recovery.move;
	}

	if (requests.hasVelocity)
	{
		horizontal = MathHelper::XZ(requests.velocity);
		if (requests.velocityIncludesVertical)
			vertical = requests.velocity.y;
	}
	else if (rootMotionMove)
	{
		const float air = grounded ? 1.0f : data->airCombat.rootMotionScale;
		const float scale = rootMotionMove->rootMotion.scale * requests.rootMotionScale * data->mesh.scale * air;
		const glm::vec3 worldVelocity = MathHelper::RotateVector(rootMotionVelocity, glm::vec3(0.0f, yaw + data->mesh.yawOffset, 0.0f)) * scale;
		horizontal = MathHelper::XZ(worldVelocity);
	}
	else
	{
		const float speed = m.runSpeed * GetMoveSpeedMultiplier() * std::max(locomotionScale, 0.0f) * requests.speedScale;
		const glm::vec3 desired = MathHelper::XZ(input.move) * speed;
		float rate = glm::length(desired) > glm::length(horizontal) ? m.acceleration : m.deceleration;
		if (!grounded)
			rate *= m.airControl;
		horizontal = MoveTowards(horizontal, desired, rate * deltaTime);
	}

	// After the velocity of this frame is known (a dash cancelled into a jump still requested its speed).
	if (pendingHorizontalLimit >= 0.0f)
	{
		const float speed = glm::length(horizontal);
		if (speed > pendingHorizontalLimit)
			horizontal *= pendingHorizontalLimit / speed;
		pendingHorizontalLimit = -1.0f;
	}

	// Melee target assist, on top of root motion / locomotion: close the gap to the ideal distance in time for the
	// strike, and never let anything carry the body inside it.
	if (requests.hasMagnet)
	{
		const glm::vec3 offset = MathHelper::XZ(requests.magnetTarget - Position);
		const float distance = glm::length(offset);
		if (distance > 1e-3f)
		{
			const glm::vec3 direction = offset / distance;
			const float gap = distance - requests.magnetIdealDistance;
			float closing = glm::dot(horizontal, direction);

			if (requests.magnetCloseGap && gap > 0.0f)
			{
				const float wanted = std::min(gap / std::max(requests.magnetArrivalTime, 0.05f), requests.magnetMaxSpeed);
				if (wanted > closing)
				{
					horizontal += direction * (wanted - closing);
					closing = wanted;
				}
			}

			const float maxClosing = std::max(gap, 0.0f) / std::max(deltaTime, 1e-4f);
			if (closing > maxClosing)
				horizontal -= direction * (closing - maxClosing);
		}
	}
	horizontal += MathHelper::XZ(requests.additiveVelocity);

	if (hasPendingVerticalSpeed)
	{
		vertical = pendingVerticalSpeed;
		hasPendingVerticalSpeed = false;
	}
	else if (!(requests.hasVelocity && requests.velocityIncludesVertical))
	{
		if (grounded && vertical <= 0.0f)
			vertical = 0.0f;
		else if (airCombatRemaining > 0.0f)
			vertical = StepAirCombatVertical(vertical, m.gravity, deltaTime, data->airCombat);
		else
			vertical -= m.gravity * requests.gravityScale * deltaTime;

		if (requests.maxFallSpeed > 0.0f)
			vertical = std::max(vertical, -requests.maxFallSpeed);
	}
	if (pendingLift > 0.0f)
	{
		vertical = std::max(vertical, pendingLift);
		pendingLift = 0.0f;
	}

	// Counts down on the ground too: a launch enters air combat while the body still stands (landing resets it).
	airCombatRemaining = std::max(0.0f, airCombatRemaining - deltaTime);

	velocity = horizontal + glm::vec3(0.0f, vertical, 0.0f);

	acceleration = deltaTime > 1e-6f ? (horizontal - previousHorizontal) / deltaTime : glm::vec3(0.0f);
	const float accelerationLength = glm::length(acceleration);
	if (accelerationLength > MaxLeanAcceleration)
		acceleration *= MaxLeanAcceleration / accelerationLength;

	movement.SetVelocity(velocity * dilation);   // physics integrates in global time
	appliedDilation = dilation;
	movement.Update(deltaTime);
}

void CombatCharacter::UpdateFacing(float deltaTime)
{
	glm::vec3 desired;
	float rate = 0.0f;
	float sharpness = data->movement.turnSharpness;

	const float locomotionScale = requests.locomotionScale >= 0.0f ? requests.locomotionScale : (moves.IsPrimaryActive() ? 0.0f : 1.0f);

	if (requests.hasFacing)
	{
		desired = requests.facing;
		rate = requests.facingRate;
		if (requests.facingSharpness > 0.0f)
			sharpness = requests.facingSharpness;
	}
	else if (const WeaponData* aiming = GetAimingWeapon())
	{
		// An aiming weapon in the hands: the body keeps facing the aim and strafes.
		desired = input.aimDirection;
		rate = aiming->stance.aimTurnRate;
	}
	else if (locomotionScale > 0.0f && glm::length(MathHelper::XZ(input.move)) > 0.1f && !IsDead())
	{
		desired = input.move;
		rate = data->movement.turnRate;
	}
	else
	{
		return;
	}

	desired = MathHelper::XZ(desired);
	if (glm::dot(desired, desired) < 1e-6f)
		return;

	// At most `rate`, easing over the last degrees so the body settles into the direction instead of stopping dead.
	const float targetYaw = MathHelper::FindLookAtRotation(glm::vec3(0.0f), desired).y;
	const float delta = MathHelper::NormalizeAngle(targetYaw - yaw);
	float step = std::abs(delta);
	if (rate > 0.0f)
	{
		const float eased = std::abs(delta) * (1.0f - std::exp(-sharpness * deltaTime));
		step = std::min({ step, rate * deltaTime, std::max(eased, MinSettleTurnRate * deltaTime) });
	}
	yaw = MathHelper::NormalizeAngle(yaw + std::clamp(delta, -step, step));
	Rotation.y = yaw;
}

void CombatCharacter::AdoptReloadedData()
{
#ifndef DISTRIBUTION
	const uint32_t generation = CombatData::GetGeneration();
	if (generation == adoptedDataGeneration)
		return;
	adoptedDataGeneration = generation;

	std::shared_ptr<const CharacterData> latest = CombatData::FindCharacter(dataPath);
	if (!latest || latest == data)
		return;

	// Live tuning adopts moves and numbers; mesh / capsule changes need a respawn.
	data = latest;
	animator.Configure(data, mesh->model ? &mesh->model->skeleton : nullptr);
	if (data->moveSet)
		animator.EnsureMoveSetAnimations(*data->moveSet);
	moves.SetMoveSet(data->moveSet);

	auto poise = GetPoise();
	const float fraction = poise->GetFraction();
	const float brokenRemaining = poise->brokenRemaining;
	poise->Configure(data->poise);
	poise->current = poise->max * fraction;
	poise->brokenRemaining = brokenRemaining;
	if (aiController)
		aiController->SetBrain(data->ai.brain);
#endif
}

// ---- Movement requests ------------------------------------------------------------------------------------

glm::vec3 CombatCharacter::GetForward() const
{
	return MathHelper::GetForwardVector(glm::vec3(0.0f, yaw, 0.0f));
}

glm::vec3 CombatCharacter::GetRight() const
{
	return MathHelper::GetRightVector(glm::vec3(0.0f, yaw, 0.0f));
}

float CombatCharacter::GetCapsuleHeight() const
{
	return data ? data->movement.height : 1.8f;
}

glm::vec3 CombatCharacter::GetFeetPosition() const
{
	return Position - glm::vec3(0.0f, GetCapsuleHeight() * 0.5f, 0.0f);
}

void CombatCharacter::RequestFacing(const glm::vec3& direction, float turnRateDegrees, float sharpness)
{
	requests.hasFacing = true;
	requests.facing = direction;
	requests.facingRate = turnRateDegrees;
	requests.facingSharpness = sharpness;
}

void CombatCharacter::SnapFacing(const glm::vec3& direction)
{
	const glm::vec3 flat = MathHelper::XZ(direction);
	if (glm::dot(flat, flat) < 1e-6f)
		return;
	yaw = MathHelper::NormalizeAngle(MathHelper::FindLookAtRotation(glm::vec3(0.0f), flat).y);
	Rotation.y = yaw;
}

void CombatCharacter::RequestVelocity(const glm::vec3& worldVelocity, bool includeVertical)
{
	requests.hasVelocity = true;
	requests.velocity = worldVelocity;
	requests.velocityIncludesVertical = includeVertical;
}

void CombatCharacter::RequestMagnet(const glm::vec3& target, float idealDistance, float arrivalTime, float maxSpeed, bool closeGap)
{
	requests.hasMagnet = true;
	requests.magnetTarget = target;
	requests.magnetIdealDistance = idealDistance;
	requests.magnetArrivalTime = arrivalTime;
	requests.magnetMaxSpeed = maxSpeed;
	requests.magnetCloseGap = closeGap;
}

void CombatCharacter::RequestLocomotion(float speedScale) { requests.locomotionScale = std::max(requests.locomotionScale, speedScale); }
void CombatCharacter::RequestSpeedScale(float scale) { requests.speedScale = std::min(requests.speedScale, scale); }
void CombatCharacter::RequestGravityScale(float scale) { requests.gravityScale = scale; }
void CombatCharacter::RequestMaxFallSpeed(float speed) { requests.maxFallSpeed = speed; }
void CombatCharacter::RequestRootMotionScale(float scale) { requests.rootMotionScale = scale; }
void CombatCharacter::RequestGliding() { requests.gliding = true; }
void CombatCharacter::RequestAdditiveVelocity(const glm::vec3& worldVelocity) { requests.additiveVelocity += worldVelocity; }
void CombatCharacter::Launch(float upwardSpeed) { SetVerticalSpeed(std::max(0.0f, upwardSpeed)); }

void CombatCharacter::SetVerticalSpeed(float speed)
{
	hasPendingVerticalSpeed = true;
	pendingVerticalSpeed = speed;
}

void CombatCharacter::LimitHorizontalSpeed(float maxSpeed)
{
	pendingHorizontalLimit = pendingHorizontalLimit < 0.0f ? maxSpeed : std::min(pendingHorizontalLimit, maxSpeed);
}

float CombatCharacter::ResolveRise(float velocityY, float appliedDilation, float physicsY)
{
	if (velocityY <= 0.0f || appliedDilation <= 0.01f)
		return velocityY;

	// Compared in the units the speed was applied in: dividing by this frame's dilation instead cut every launch at
	// the end of its hit-stop (the dilation jumps from 0.03 toward 1 while last frame's speed was 0.03 of the real one).
	const float applied = velocityY * appliedDilation;
	if (applied <= MinJudgedRise || physicsY >= applied * 0.5f)
		return velocityY;
	return std::max(physicsY, 0.0f) / appliedDilation;
}

void CombatCharacter::EnterAirCombat(float seconds)
{
	airCombatRemaining = std::max(airCombatRemaining, seconds);
}

void CombatCharacter::LiftTo(float upwardSpeed)
{
	pendingLift = std::max(pendingLift, upwardSpeed);
}

float CombatCharacter::StepAirCombatVertical(float vertical, float gravity, float deltaTime, const AirCombatData& air)
{
	if (vertical > 0.0f)
		return vertical - gravity * air.riseGravityScale * deltaTime;
	if (vertical >= -air.maxFallSpeed)
		return std::max(vertical - gravity * air.fallGravityScale * deltaTime, -air.maxFallSpeed);

	// Faster than the sink (a fall that entered air combat): ease back to it without gravity pushing past it.
	return -air.maxFallSpeed + (vertical + air.maxFallSpeed) * std::exp(-air.fallEase * deltaTime);
}

glm::vec3 CombatCharacter::TakePendingKnockback()
{
	const glm::vec3 result = pendingKnockback;
	pendingKnockback = glm::vec3(0.0f);
	return result;
}

// ---- Targeting --------------------------------------------------------------------------------------------

Entity* CombatCharacter::FindFinisherTarget() const
{
	if (!data)
		return nullptr;
	const glm::vec3 inputDirection = MathHelper::XZ(input.move);
	const glm::vec3 direction = glm::dot(inputDirection, inputDirection) > 0.01f ? inputDirection : GetForward();
	return FindSoftTarget(direction, data->finisher.range, data->finisher.angle, true);
}

// Read-only over the other bodies: safe from the parallel body update (component pools exist, nothing is added
// or removed while bodies update).
Entity* CombatCharacter::FindSoftTarget(const glm::vec3& direction, float range, float halfAngleDegrees, bool requireBroken,
	const Entity* prefer) const
{
	glm::vec3 forward = MathHelper::XZ(direction);
	forward = glm::dot(forward, forward) > 1e-6f ? glm::normalize(forward) : GetForward();
	const float cosLimit = std::cos(glm::radians(halfAngleDegrees));
	const Team myTeam = GetTeamId();

	Entity* best = nullptr;
	float bestScore = 1e30f;

	auto view = EcsWorld::Registry().view<TeamComponent, CombatStateComponent>();
	for (entt::entity id : view)
	{
		Entity* candidate = EcsWorld::GetEntity(id);
		if (candidate == nullptr || candidate == this || candidate->Destroyed)
			continue;

		const TeamComponent& team = view.get<TeamComponent>(id);
		const CombatStateComponent& state = view.get<CombatStateComponent>(id);
		if (state.dead || state.inFinisher || GetAttitude(myTeam, team.team) != Attitude::Hostile)
			continue;

		if (requireBroken)
		{
			auto poise = candidate->GetComponent<PoiseComponent>();
			if (!poise || !poise->IsBroken())
				continue;
		}

		const glm::vec3 offset = MathHelper::XZ(candidate->Position - Position);
		const float distance = glm::length(offset);
		if (distance > range)
			continue;

		const float cosAngle = distance > 1e-3f ? glm::dot(offset / distance, forward) : 1.0f;
		if (cosAngle < cosLimit)
			continue;

		if (candidate == prefer)
			return candidate;

		// Prefer close and centered targets.
		const float score = distance * (2.0f - cosAngle);
		if (score < bestScore)
		{
			bestScore = score;
			best = candidate;
		}
	}

	return best;
}

Entity* CombatCharacter::FindAttackTarget(float range) const
{
	if (!data || range <= 0.0f)
		return nullptr;

	const TargetingData& targeting = data->targeting;
	const glm::vec3 inputDirection = MathHelper::XZ(input.move);
	const float inputLength = glm::length(inputDirection);
	const glm::vec3 reference = inputLength >= targeting.inputThreshold ? inputDirection / inputLength : GetForward();
	const Team myTeam = GetTeamId();

	EntityHandle previousHandle = softTarget;
	const Entity* previous = ResolveAlive(previousHandle);

	struct Candidate
	{
		float score;
		Entity* entity;
	};
	std::vector<Candidate> candidates;

	auto view = EcsWorld::Registry().view<TeamComponent, CombatStateComponent>();
	for (entt::entity id : view)
	{
		Entity* candidate = EcsWorld::GetEntity(id);
		if (candidate == nullptr || candidate == this || candidate->Destroyed)
			continue;

		const CombatStateComponent& state = view.get<CombatStateComponent>(id);
		if (state.dead || state.inFinisher || GetAttitude(myTeam, view.get<TeamComponent>(id).team) != Attitude::Hostile)
			continue;

		const glm::vec3 offset = MathHelper::XZ(candidate->Position - Position);
		const float distance = glm::length(offset);
		if (distance > range)
			continue;

		const float cosAngle = distance > 1e-3f ? std::clamp(glm::dot(offset / distance, reference), -1.0f, 1.0f) : 1.0f;
		const float score = CombatRules::ScoreAttackTarget(glm::degrees(std::acos(cosAngle)), distance, range, candidate == previous, targeting);
		candidates.push_back({ score, candidate });
	}

	// Best first; a body behind a wall is skipped (traces only until one is in sight).
	std::sort(candidates.begin(), candidates.end(), [](const Candidate& a, const Candidate& b) { return a.score < b.score; });
	const glm::vec3 eye = GetTargetPoint();
	for (const Candidate& candidate : candidates)
	{
		auto* body = dynamic_cast<CombatCharacter*>(candidate.entity);
		const glm::vec3 point = body != nullptr ? body->GetTargetPoint() : candidate.entity->Position;
		if (!Physics::LineTrace(eye, point, BodyType::World).hasHit)
			return candidate.entity;
	}
	return nullptr;
}

Entity* CombatCharacter::PredictAttackTarget() const
{
	const MoveSetData* moveSet = moves.GetMoveSet();
	if (!moveSet)
		return nullptr;
	const MoveData* opener = moveSet->GetMove(moveSet->GetComboOpener(moveSet->GetOpenerSlot(grounded)));
	return opener != nullptr ? FindAttackTarget(opener->tracking.searchRange) : nullptr;
}

void CombatCharacter::SetSoftTarget(Entity* target)
{
	softTarget = target ? EntityHandle::FromEntity(target) : EntityHandle();
}

glm::vec3 CombatCharacter::GetAimPoint() const
{
	const glm::vec3 origin = input.aimOrigin;
	const glm::vec3 direction = glm::dot(input.aimDirection, input.aimDirection) > 1e-6f ? glm::normalize(input.aimDirection) : GetForward();
	constexpr float range = 200.0f;

	const Physics::HitResult hit = Physics::LineTrace(origin, origin + direction * range,
		static_cast<BodyType>(BodyType::World | BodyType::HitBox | BodyType::MainBody), {}, { const_cast<CombatCharacter*>(this) });
	return hit.hasHit ? hit.position : origin + direction * range;
}

glm::vec3 CombatCharacter::GetTargetPoint() const
{
	return Position + glm::vec3(0.0f, GetCapsuleHeight() * 0.25f, 0.0f);
}

glm::vec3 CombatCharacter::BonePositionOrOrigin(const std::string& bone) const
{
	if (!bone.empty() && mesh != nullptr)
		return glm::vec3(mesh->GetBoneMatrixWorld(bone)[3]);
	return Position;
}

// ---- MoveRunner hooks -------------------------------------------------------------------------------------

void CombatCharacter::OnMoveStarted(const MoveSlot& slot, bool primary)
{
	if (!primary)
		return;

	// The first weapon-driven hit window decides which weapon is in the hands for this move.
	for (const HitWindowData& hit : slot.move->hits)
	{
		if (hit.bone.empty())
		{
			weapons.SetActiveSlot(hit.weapon);
			break;
		}
	}
}

void CombatCharacter::OnMoveEnded(const MoveSlot& slot, MoveEndReason reason, bool primary)
{
	(void)slot;
	(void)reason;
	(void)primary;
}

void CombatCharacter::PlayCue(const CueData& cue)
{
	if (CombatFrame::IsParallel())
		queuedCues.push_back(&cue);
	else
		PlayCueNow(cue);
}

void CombatCharacter::PlayCueNow(const CueData& cue)
{
	const glm::vec3 position = BonePositionOrOrigin(cue.bone);

	if (!cue.sound.empty())
		SoundPlayer::PlayOneshot(cue.sound, 3.0f, 1.0f, false, position);

	if (!cue.particle.empty())
		GlobalParticleSystem::SpawnParticleAt(cue.particle, position, glm::vec3(0.0f, yaw, 0.0f), glm::vec3(1.0f));

	if (cue.trail >= 0)
		weapons.SetTrail(cue.trailWeapon, cue.trail == 1);

	if (cue.cameraShake > 0.0f && IsPlayerControlled())
		if (PlayerController* player = PlayerController::GetActive())
			player->GetCamera().AddShake(cue.cameraShake);
}

// ---- Combat callbacks -------------------------------------------------------------------------------------

void CombatCharacter::OnHitResolved(const HitResult& result)
{
	if (auto state = GetCombatState())
	{
		state->lastAttacker = result.request ? result.request->attacker : EntityHandle();
		state->lastHitTime = Time::GameTime;
	}

	if (aiController)
		aiController->NotifyDamaged(result.attacker);

	// The upper body bends away from the blow at once; armor (no reaction) halves it.
	if (result.request)
		AddHitRecoil(result.request->direction, result.request->knockback * (result.reaction == HitReaction::None && !result.killed ? 0.5f : 1.0f));

	if (result.killed)
	{
		OnDeath(result.attacker);
		return;
	}

	// Air combat (§5.8): hit in the air or launched, the body hangs for the hit's stall; hit in the air it is lifted too
	// (a launch keeps its own speed).
	if (result.request && (!grounded || result.reaction == HitReaction::Launch))
	{
		EnterAirCombat(result.request->airStall);
		if (!grounded && result.reaction != HitReaction::Launch)
			LiftTo(result.request->airLift);
	}

	const MoveSetData* moveSet = moves.GetMoveSet();
	if (!moveSet)
		return;

	auto reactionMove = [moveSet](std::initializer_list<ReactionSlot> preference)
	{
		for (ReactionSlot slot : preference)
		{
			const int index = moveSet->GetReactionMove(slot);
			if (index >= 0) return index;
		}
		return -1;
	};

	int moveIndex = -1;
	if (result.poiseBroken)
		moveIndex = reactionMove({ ReactionSlot::PoiseBreak, ReactionSlot::Stagger, ReactionSlot::Flinch });
	else
	{
		switch (result.reaction)
		{
		case HitReaction::Flinch:    moveIndex = reactionMove({ ReactionSlot::Flinch }); break;
		case HitReaction::Stagger:   moveIndex = reactionMove({ ReactionSlot::Stagger, ReactionSlot::Flinch }); break;
		case HitReaction::Knockdown: moveIndex = reactionMove({ ReactionSlot::Knockdown, ReactionSlot::Stagger, ReactionSlot::Flinch }); break;
		case HitReaction::Launch:    moveIndex = reactionMove({ ReactionSlot::Launch, ReactionSlot::Knockdown, ReactionSlot::Stagger }); break;
		default: break;
		}
	}

	if (moveIndex < 0)
	{
		pendingKnockback = glm::vec3(0.0f);
		return;
	}

	// In the air a push carries the body further (no ground friction in the reaction): scaled so the attacker stays in reach.
	pendingKnockback = result.knockback;
	if (!grounded)
	{
		pendingKnockback.x *= data->airCombat.knockbackScale;
		pendingKnockback.z *= data->airCombat.knockbackScale;
	}
	moves.StartForcedMove(moveIndex);
}

void CombatCharacter::AddHitRecoil(const glm::vec3& direction, float strength)
{
	const HitRecoilData& recoil = data->hitRecoil;
	const glm::vec3 flat = MathHelper::XZ(direction);
	if (recoil.bones.empty() || strength <= 0.0f || glm::dot(flat, flat) < 1e-6f)
		return;

	// About up x direction: tips the top of the body along the blow.
	const glm::vec3 axis = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), glm::normalize(flat)));
	const float angle = glm::radians(std::min(strength * recoil.degreesPerKnockback, recoil.maxAngle));
	hitRecoil.Displace(axis * angle, glm::radians(recoil.maxAngle));
}

void CombatCharacter::OnHitDealt(const HitResult& result)
{
	moves.NotifyHitDealt(result);
}

void CombatCharacter::OnDeath(Entity* killer)
{
	(void)killer;

	auto state = GetCombatState();
	if (!state || state->dead)
		return;

	state->dead = true;
	state->invulnerable = false;
	corpseDelay.AddDelay(data->corpseLifetime);

	input.ReleaseAll(Time::GameTime);
	weapons.StopAllTrails();
	moves.SetLocked(true);

	if (aiController)
		aiController->OnBodyDied();

	// Corpses don't block movement.
	movement.SetCollisionMask(static_cast<BodyType>(BodyType::World | BodyType::WorldSkybox));

	const int deathMove = moves.GetMoveSet() ? moves.GetMoveSet()->GetReactionMove(ReactionSlot::Death) : -1;
	if (deathMove >= 0)
	{
		moves.StartForcedMove(deathMove);
	}
	else
	{
		moves.StopAll(MoveEndReason::Interrupted);
		mesh->StartRagdoll();
	}
}

void CombatCharacter::OnDamage(float damage, Entity* causer, Entity* weapon)
{
	OnPointDamage(damage, GetTargetPoint(), causer ? glm::normalize(Position - causer->Position + glm::vec3(0.0f, 0.001f, 0.0f)) : GetForward(), "", causer, weapon);
}

void CombatCharacter::OnPointDamage(float damage, vec3 point, vec3 direction, string bone, Entity* causer, Entity* weapon)
{
	(void)weapon;

	// Engine-originated damage (explosions, triggers, hazards) enters the same funnel as combat hits.
	HitRequestEvent request;
	request.attacker = causer ? EntityHandle::FromEntity(causer) : EntityHandle();
	request.victim = EntityHandle::FromEntity(this);
	request.source = DamageSource::Environment;
	request.damage = damage;
	request.poiseDamage = damage;
	request.weakPointDamage = damage;
	request.reaction = HitReaction::Flinch;
	request.point = point;
	request.direction = glm::dot(direction, direction) > 1e-6f ? glm::normalize(direction) : GetForward();
	request.spray = request.direction;
	request.hitbox = bone;
	request.flags = HitFlags::FriendlyFire;
	EcsScheduler::Emit(request);   // any thread (the bus is thread-safe); engine damage has no ordering contract
}

// ---- Debug ------------------------------------------------------------------------------------------------

void CombatCharacter::DrawDebugUI()
{
#ifndef DISTRIBUTION
	ImGui::Text("%s  [%s]  data: %s", Id.c_str(), ClassName.c_str(), dataPath.c_str());
	if (!data)
		return;

	ImGui::Text("Health %.1f / %.1f   Team %d   %s%s", Health, MaxHealth, static_cast<int>(GetTeamId()),
		IsDead() ? "DEAD " : "", IsPlayerControlled() ? "PLAYER" : "AI");

	if (auto poise = GetPoise())
		ImGui::Text("Poise %.1f / %.1f  armor %.2f  %s", poise->current, poise->max, poise->armorResist,
			poise->IsBroken() ? "BROKEN (finishable)" : "");

	if (auto state = GetCombatState())
		ImGui::Text("Hit-stop %.3f / %.3f s (time x%.2f)  invulnerable %d", state->hitStopElapsed, state->hitStopDuration,
			state->GetTimeDilation(), state->invulnerable ? 1 : 0);

	ImGui::Text("Grounded %d  velocity (%.1f, %.1f, %.1f)  yaw %.0f  air combat %.2f  anim 1/%d", grounded ? 1 : 0,
		velocity.x, velocity.y, velocity.z, yaw, airCombatRemaining, animationInterval);

	const float attackSpeed = GetAttackSpeedMultiplier();
	auto speedOf = [attackSpeed](const MoveSlot& slot) { return slot.IsActive() ? slot.move->speed * attackSpeed : 1.0f; };
	DrawMoveTimeline("Primary", moves.GetPrimary(), speedOf(moves.GetPrimary()));
	DrawRecoveryStatus(moves.GetRecovery());
	DrawMoveTimeline("Overlay", moves.GetOverlay(), speedOf(moves.GetOverlay()));
	ImGui::Text("Combo input: %s   last decision: %s", moves.GetComboResolver().DescribePending(), moves.GetLastComboDecision());
	ImGui::SetItemTooltip("Combo input: the Attack press the running attack's combo window is holding, if any.\n"
		"Last decision: which follow-up the last combo link chose - light (a tap), heavy (held through the heavy point /\n"
		"late press) or hold (held for holdDelayInMove: the launcher).");

	if (aiController)
		aiController->DrawDebugUI();
#endif
}
