#include "NpcBody.h"

#include "../NpcBase.h"

#include <algorithm>
#include <cmath>

namespace
{
	// The signed angle in degrees from yaw `from` to yaw `to`, in -180..180.
	float AngleDifference(float from, float to)
	{
		float difference = std::fmod(to - from, 360.0f);
		if (difference > 180.0f) difference -= 360.0f;
		if (difference < -180.0f) difference += 360.0f;
		return difference;
	}

	float Approach(float value, float target, float maxStep)
	{
		return value + std::clamp(target - value, -maxStep, maxStep);
	}

	float YawOf(const vec3& flat)
	{
		return glm::degrees(std::atan2(flat.x, flat.z));
	}

	vec3 FlatDirection(float yawDegrees)
	{
		const float radians = glm::radians(yawDegrees);
		return vec3(std::sin(radians), 0.0f, std::cos(radians));
	}
}

void NpcBody::Start()
{
	const vec3 forward = MathHelper::XZ(MathHelper::GetForwardVector(npc.mesh->Rotation));
	if (glm::length(forward) > 0.01f)
		facingDirection = glm::normalize(forward);
}

void NpcBody::Glance(const vec3& point, double now)
{
	glancePoint = point;
	glanceUntil = now + profile->glanceTime;
}

vec3 NpcBody::EyePosition() const
{
	return npc.Position + vec3(0.0f, profile->eyeHeight, 0.0f);
}

vec3 NpcBody::GazeDirection() const
{
	return MathHelper::GetForwardVector(vec3(gazePitch, npc.mesh->Rotation.y + gazeYaw, 0.0f));
}

void NpcBody::UpdateObserver(Observer& observer, float visionRangeScale) const
{
	observer.position = EyePosition();
	observer.forward = GazeDirection();
	observer.vision.rangeScale = visionRangeScale;
}

bool NpcBody::FacingLock(vec3& lookVector) const
{
	if (facingLocked == false) return false;

	lookVector = lockedLook;
	return true;
}

float NpcBody::PaceFor(const BodyIntent& intent) const
{
	if (intent.speed > 0.0f) return intent.speed;

	switch (intent.gait)
	{
	case NpcGait::Stand: return 0.0f;
	case NpcGait::Run: return profile->runSpeed;
	default: return profile->walkSpeed;
	}
}

void NpcBody::Apply(const BodyIntent& intent, float dt, double now)
{
	weapon = profile->armed ? intent.weapon : NpcWeaponState::Holstered;

	// The controller changes its shape on the main thread (NpcBase::ApplyCombatStance), from this order.
	npc.combatMoveOrder.stance = intent.crouch ? NpcStance::Crouched : NpcStance::Standing;

	ApplyGaze(intent, dt, now);
	ApplyFacing(intent, dt);
	ApplyMovement(intent);
}

void NpcBody::ApplyMovement(const BodyIntent& intent)
{
	if (intent.move == false)
	{
		if (moving)
			npc.StopMovement();

		moving = false;
		reachedNow = false;
		settleFrames = 0;
		return;
	}

	const bool newDestination = moving == false || glm::distance(intent.destination, destination) > 0.4f;

	destination = intent.destination;
	npc.MoveTo(destination, intent.acceptRadius);
	npc.speed = PaceFor(intent);

	// The path of the frame a destination is set is stale: start over for two frames, as the behaviour tree did.
	if (newDestination)
		settleFrames = 2;

	if (settleFrames > 0)
	{
		npc.PrepareToStartMovement();
		--settleFrames;
		reachedNow = false;
	}
	else
	{
		reachedNow = npc.pathFollow.reachedTarget && npc.pathFollow.CalculatedPath;
	}

	moving = true;
}

void NpcBody::ApplyFacing(const BodyIntent& intent, float dt)
{
	const vec3 current = MathHelper::XZ(MathHelper::GetForwardVector(npc.mesh->Rotation));

	// What the body should face: the intent's point, or, standing, a gaze the head cannot reach.
	bool hasFacingPoint = false;
	vec3 facingPoint = vec3();

	if (intent.facing == BodyFacing::Point)
	{
		hasFacingPoint = true;
		facingPoint = intent.facingPoint;
	}
	else if (gazeBeyondHead && moving == false && hasGazeTarget)
	{
		hasFacingPoint = true;
		facingPoint = gazeTarget;
	}

	const vec3 wanted = MathHelper::XZ(facingPoint - npc.Position);

	if (hasFacingPoint == false || glm::length(wanted) < 0.05f)
	{
		facingLocked = false;
		if (glm::length(current) > 0.01f)
			facingDirection = glm::normalize(current);
		return;
	}

	const float rate = moving ? profile->body.turnRateWalk : profile->body.turnRateStand;
	const float yaw = YawOf(facingDirection);
	const float wantedYaw = YawOf(wanted);

	facingDirection = FlatDirection(yaw + std::clamp(AngleDifference(yaw, wantedYaw), -rate * dt, rate * dt));

	lockedLook = facingDirection;
	facingLocked = true;

	// Where the body settles when the lock is off (at a task point the task decides the facing).
	npc.desiredLookVector = glm::normalize(wanted);
}

void NpcBody::ApplyGaze(const BodyIntent& intent, float dt, double now)
{
	hasGazeTarget = false;
	gazeBeyondHead = false;

	if (intent.glanceAllowed && now < glanceUntil)
	{
		hasGazeTarget = true;
		gazeTarget = glancePoint;
	}
	else if (intent.hasGaze)
	{
		hasGazeTarget = true;
		gazeTarget = intent.gazePoint;
	}

	float wantedYaw = 0.0f;
	float wantedPitch = 0.0f;

	const vec3 toTarget = gazeTarget - EyePosition();

	if (hasGazeTarget && glm::length(toTarget) > 0.05f)
	{
		const vec3 local = MathHelper::RotateVector(glm::normalize(toTarget), npc.mesh->Rotation * -1.0f);
		const vec3 euler = MathHelper::FindLookAtRotation(vec3(0.0f), local);

		wantedYaw = AngleDifference(0.0f, euler.y);
		wantedPitch = euler.x;

		if (std::isnan(wantedYaw) || std::isnan(wantedPitch))
		{
			wantedYaw = 0.0f;
			wantedPitch = 0.0f;
		}
	}

	const NpcBodyTuning& tuning = profile->body;

	gazeBeyondHead = std::abs(wantedYaw) > tuning.headYawLimit;

	wantedYaw = std::clamp(wantedYaw, -tuning.headYawLimit, tuning.headYawLimit);
	wantedPitch = std::clamp(wantedPitch, -tuning.headPitchLimit, tuning.headPitchLimit);

	gazeYaw = Approach(gazeYaw, wantedYaw, tuning.headSpeed * dt);
	gazePitch = Approach(gazePitch, wantedPitch, tuning.headSpeed * dt);
}
