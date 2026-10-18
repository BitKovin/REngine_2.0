#include "CharacterController.h"

#include <array>
#include <cassert>

// ---------------------------------------------------------------------------
// Static registry
// ---------------------------------------------------------------------------

std::vector<CharacterController*> CharacterController::s_allControllers;

void CharacterController::TagBodies()
{
	if (body)
		Physics::GetBodyData(body)->characterController = this;
	if (sensorBody)
		Physics::GetBodyData(sensorBody)->characterController = this;
}

CharacterController* CharacterController::ControllerOf(const BodyID& id)
{
	Body* found = Physics::GetBodyFromId(id);
	BodyData* data = found ? Physics::GetBodyData(found) : nullptr;
	return data ? data->characterController : nullptr;
}

// ---------------------------------------------------------------------------

CharacterController::CharacterController()
{
	currentCameraHeight = cameraHeightStanding;
	targetCameraHeight = cameraHeightStanding;
}

CharacterController::~CharacterController()
{
	s_allControllers.erase(
		std::remove(s_allControllers.begin(), s_allControllers.end(), this),
		s_allControllers.end());

	if (body)
		Physics::DestroyBody(body);
	if (sensorBody)
		Physics::DestroyBody(sensorBody);
}


void CharacterController::Init(Entity* owner, vec3 position, float radius, float height, float mass)
{
	this->owner = owner;
	this->standingHeight = height;

	Destroy();

	body = Physics::CreateCharacterBody(owner, position, radius, height - stepHeight, mass);
	body->GetMotionProperties()->SetLinearDamping(0);
	Physics::GetBodyData(body)->dynamicCollisionGroupOrMask = true;
	Physics::GetBodyData(body)->sensorCollisionMode = SensorCollisionMode::NotCollideWithSensors;

	// Sensor-only body: follows the character but only interacts with sensor volumes
	sensorBody = Physics::CreateCharacterCylinderBody(owner, position, radius, height, mass);
	sensorBody->GetMotionProperties()->SetLinearDamping(0);
	Physics::GetBodyData(sensorBody)->dynamicCollisionGroupOrMask = true;
	Physics::GetBodyData(sensorBody)->sensorCollisionMode = SensorCollisionMode::CollideOnlyWithSensors;
	Physics::SetGravityFactor(sensorBody, 0);
	TagBodies();

	this->height = height;
	this->radius = radius;

	Physics::SetGravityFactor(body, 0);
	groundTraceIgnore.assign(1, owner);

	// Register in the global controller list (guard against double-add on re-init)
	if (std::find(s_allControllers.begin(), s_allControllers.end(), this) == s_allControllers.end())
		s_allControllers.push_back(this);
}

void CharacterController::Destroy()
{
	// Clean up ignore pairs before the bodies are destroyed — BodyIDs become
	// invalid after DestroyBody so this must happen first.
	if (body)
	{
		for (const BodyID& id : activeIgnorePairs)
			Physics::RemoveIgnorePair(body->GetID(), id);
	}
	activeIgnorePairs.clear();

	if (body)
	{
		Physics::DestroyBody(body);
		body = nullptr;
	}
	if (sensorBody)
	{

		Physics::DestroyBody(sensorBody);
		sensorBody = nullptr;
	}
}

void CharacterController::Update(float deltaTime)
{
	if (body == nullptr) return;

	UpdateSmoothPosition(deltaTime);

	Entity* standingOnEntityPtr = standingOnEntity.Resolve(false);

	Body* lastStandingOnBodyPtr = Physics::GetBodyFromId(lastStandingOnBody);

	vec3 currentVelocity = GetVelocity();
	bool isIdle = glm::length(MathHelper::XZ(currentVelocity)) < 0.001f && currentVelocity.y <= 0;
	bool skipTracesAndMovement = (isIdle && onGround && standingOnEntityPtr != nullptr && standingOnEntityPtr->Static == true) && allowSleep;

	if (skipTracesAndMovement)
	{
		SetVelocity(vec3(0.0f));
	}
	else
	{
		// -------------------------------
		// 1) If previously attached, preemptively apply platform delta
		// -------------------------------
		if (lastStandingOnBodyPtr)
		{
			vec3 basePos = FromPhysics(lastStandingOnBodyPtr->GetPosition());
			glm::quat baseRot = FromPhysics(lastStandingOnBodyPtr->GetRotation());

			vec3 attachWorldPos = baseRot * baseLocalAttachPoint + basePos;
			vec3 platformDelta = attachWorldPos - prevAttachWorldPos;

			// clamp huge deltas to prevent teleport jumps
			const float maxDelta = 10.0f;
			if (glm::length(platformDelta) > maxDelta)
			{
				prevAttachWorldPos = attachWorldPos;
			}
			else
			{
				if (glm::length(platformDelta) > 1e-6f)
				{
					vec3 currentPos = FromPhysics(body->GetPosition());
					vec3 targetPos = currentPos + platformDelta;

					// sweep character to follow platform while resolving collisions
					Physics::SweepBody(body, targetPos, { owner });
				}

				prevAttachWorldPos = attachWorldPos;
			}

			prevBaseRotation = baseRot;
			prevBasePosition = basePos;
		}

		// -------------------------------
		// 2) Ground check after potential platform move
		// -------------------------------
		float verticalPosition;
		bool standsOnGround;
		vec3 walkableNormal = vec3();
		vec3 notWalkableNormal = vec3();
		UpdateGroundCheck(standsOnGround, verticalPosition, onGround, walkableNormal, notWalkableNormal);

		currentGroundNormal = walkableNormal;

		//DebugDraw::Line(GetPosition() - vec3(0, 1, 0) * height / 2.0f, GetPosition() - vec3(0, 1, 0) * height / 2.0f + walkableNormal);

		if (onGround == false)
		{
			standingOnBody = BodyID();
			standingOnEntity = EntityHandle();
		}

		Body* currentBase = Physics::GetBodyFromId(standingOnBody);

		// -------------------------------
		// 3) Platform attach/detach detection
		// -------------------------------
		bool wasAttached = (lastStandingOnBodyPtr != nullptr);
		bool isAttached = (currentBase != nullptr);

		if (wasAttached && !isAttached)
		{
			// Detached: apply last platform velocity to character
			// (use fresh computation for accuracy on rotation)
			vec3 basePos = FromPhysics(lastStandingOnBodyPtr->GetPosition());
			glm::quat baseRot = FromPhysics(lastStandingOnBodyPtr->GetRotation());
			vec3 linearVel = FromPhysics(lastStandingOnBodyPtr->GetLinearVelocity());
			vec3 angularVel = FromPhysics(lastStandingOnBodyPtr->GetAngularVelocity());
			vec3 worldOffset = baseRot * baseLocalAttachPoint;
			vec3 platformVelAtAttach = linearVel + glm::cross(angularVel, worldOffset);

			vec3 currentVel = GetVelocity();
			currentVel += platformVelAtAttach;
			SetVelocity(currentVel);
			lastPlatformVelocity = vec3(0.0f);
		}
		else if (wasAttached && isAttached && (currentBase != lastStandingOnBodyPtr))
		{
			// Switched platforms: detach from old, attach to new
			// Detach from old
			vec3 oldBasePos = FromPhysics(lastStandingOnBodyPtr->GetPosition());
			glm::quat oldBaseRot = FromPhysics(lastStandingOnBodyPtr->GetRotation());
			vec3 oldLinearVel = FromPhysics(lastStandingOnBodyPtr->GetLinearVelocity());
			vec3 oldAngularVel = FromPhysics(lastStandingOnBodyPtr->GetAngularVelocity());
			vec3 oldWorldOffset = oldBaseRot * baseLocalAttachPoint;
			vec3 oldPlatformVel = oldLinearVel + glm::cross(oldAngularVel, oldWorldOffset);

			vec3 currentVel = GetVelocity();
			currentVel += oldPlatformVel;
			SetVelocity(currentVel);
			lastPlatformVelocity = vec3(0.0f);  // Temporary reset

			// Attach to new (fall through to attach logic below)
			wasAttached = false;  // Force attach block to run
		}

		if (!wasAttached && isAttached)
		{
			// Attaching to a new platform: store local attach point
			vec3 basePos = FromPhysics(currentBase->GetPosition());
			glm::quat baseRot = FromPhysics(currentBase->GetRotation());
			vec3 charPos = FromPhysics(body->GetPosition());

			baseLocalAttachPoint = glm::inverse(baseRot) * (charPos - basePos);
			prevAttachWorldPos = charPos;
			prevBaseRotation = baseRot;
			prevBasePosition = basePos;

			// Adjust character's velocity to be relative to the platform
			vec3 linearVel = FromPhysics(currentBase->GetLinearVelocity());
			vec3 angularVel = FromPhysics(currentBase->GetAngularVelocity());
			vec3 worldOffset = baseRot * baseLocalAttachPoint;
			vec3 platformVelAtAttach = linearVel + glm::cross(angularVel, worldOffset);

			vec3 currentVel = GetVelocity();
			currentVel -= platformVelAtAttach;
			SetVelocity(currentVel);

			lastPlatformVelocity = platformVelAtAttach;
		}

		if (currentBase)
		{
			lastStandingOnBody = currentBase->GetID();
		}else
		{
			lastStandingOnBody = BodyID();
		}


		// -------------------------------
		// 4) Original character movement / gravity / slope logic
		// -------------------------------
		vec3 velocity = GetVelocity();

		if (velocity.y > 0.1f)
		{
			onGround = false;
		}

		// Manual gravity — skipped when the ladder (or any future system) takes
		// sole ownership of vertical velocity via suppressGravity.
		if (!suppressGravity)
		{
			if (onGround && velocity.y < 0)
			{
				velocity.y -= gravity * deltaTime * (1.0f - walkableNormal.y);
			}
			else
			{
				velocity.y -= gravity * deltaTime;
			}
		}

		// The walkable ground under the feet lies off to one side of the center (an edge), not all around it.
		const bool supportToOneSide = glm::length(groundSupportOffset) > radius * 0.25f;
		bool heldAboveGround = false;

		if (onGround && velocity.y <= 0)
		{
			vec3 currentPosition = FromPhysics(body->GetPosition());
			float newVerticalPosition = verticalPosition + stepHeight / 2.0f + height / 2.0f;

			float snapDelta = newVerticalPosition - currentPosition.y;

			// Stepping up onto an edge without room above it for the whole character (a sill in a window recess lower
			// than the character, a step under a low ceiling): standing there would leave the legs inside the ground,
			// so it is not ground, and the character slides out from under the overhang. With ground all around there
			// is nowhere to slide to; standing at the height that fits is the lesser evil there.
			constexpr float MinCheckedStep = 0.05f;
			if (snapDelta > MinCheckedStep && supportToOneSide && !HasRoomAbove(snapDelta))
			{
				onGround = false;
				standingOnBody = BodyID();
				standingOnEntity = EntityHandle();
			}
			else if (std::abs(snapDelta) > 0.005f)
			{
				Physics::SweepBody(body, vec3(currentPosition.x, newVerticalPosition, currentPosition.z), { owner });

				float moved = currentPosition.y - FromPhysics(body->GetPosition()).y;
				if (std::abs(moved) > 0.001f)
				{
					heightSmoothOffset += moved;
				}

				// Going down to the ground but stopped on the way: something holds the body above where it stands.
				heldAboveGround = snapDelta < -MinCheckedStep && -snapDelta - moved > MinCheckedStep;
			}
		}

		// An edge under the rim of the rounded bottom of the body (the lip of a window sill, the corner of a post), while
		// falling or while held above the ground it should stand on. The traces do not hold the body there, the physics
		// does: the rounded bottom rests on the edge, and moving into it the solver lifts the body over it (the contact
		// normal points up and out), which adds rising speed nothing asked for. So the body slides off the edge here,
		// and never moves into it. Ground all around (a gap the body bridges) is left alone: there is nowhere to slip
		// to. Only once the body stops rising (the velocity it was given, before this step's gravity): on the way up,
		// sliding along an edge is movement the player can use.
		vec3  offEdge = vec3(0.0f);
		float edgeClearance = 0.0f;
		if ((onGround == false || heldAboveGround) && currentVelocity.y <= 0 && !suppressGravity)
		{
			offEdge = AwayFromEdgeUnderRim(edgeClearance);
			if (glm::length(offEdge) > 0.0f)
			{
				onGround = false;
				standingOnBody = BodyID();
				standingOnEntity = EntityHandle();
			}
		}

		if (onGround)
		{
			velocity.y = 0;
		}

		vec3 applyVelocity = velocity;

		// Only a steep surface that was actually hit slides the character along it. Without one notWalkableNormal is
		// straight up, and clipping against it took all of the fall speed: the character hung in the air, falling,
		// next to ground it could not stand on.
		const bool touchesSteep = standsOnGround && glm::length(notWalkableNormal) > 1e-4f
			&& GroundAngleDeg(notWalkableNormal) > groundMaxAngle;

		if (onGround == false && touchesSteep && (velocity.y <= 0))
		{
			vec3 slopeNormal = normalize(notWalkableNormal);

			// 1. Calculate how much of our velocity is pointing INTO the slope
			float velocityIntoSlope = dot(velocity, slopeNormal);

			// 2. Only alter velocity if we are actually moving into the unwalkable surface
			if (velocityIntoSlope < 0)
			{
				// Clip the velocity. This removes the inward force and leaves 
				// ONLY the velocity running parallel down the slope.
				applyVelocity = velocity - slopeNormal * velocityIntoSlope;

				// Optional: Apply a slight over-bounce to prevent floating point penetration
				// applyVelocity = velocity - slopeNormal * (velocityIntoSlope * 1.001f);
			}
			else
			{
				// If moving away from the slope (e.g. jumping off), keep the trajectory
				applyVelocity = velocity;
			}

			UpdateSmoothPosition(deltaTime * 2);
		}
		else if (onGround == false && touchesSteep)
		{
			vec3 slopeNormal = normalize(notWalkableNormal);
			float velocityTowardSlope = dot(velocity, slopeNormal);
			if (velocityTowardSlope < 0)
			{
				vec3 slopeTangent = velocity - slopeNormal * velocityTowardSlope;
				float originalSpeed = length(velocity);

				applyVelocity = slopeTangent;
				applyVelocity.y = velocity.y;

				float newSpeed = length(applyVelocity);
				if (newSpeed > originalSpeed)
					applyVelocity = applyVelocity * (originalSpeed / newSpeed);

				UpdateSmoothPosition(deltaTime * 2);
			}
		}

		// Slide off the edge: no speed into it, and at least enough away from it to clear it in EdgeSlipTime (gently
		// when it only grazes the rim, faster the deeper it reaches under the body).
		if (glm::length(offEdge) > 0.0f)
		{
			constexpr float EdgeSlipTime = 0.15f;
			const float towardEdge = glm::dot(applyVelocity, -offEdge);
			if (towardEdge > 0.0f)
				applyVelocity += offEdge * towardEdge;

			const float slipSpeed = std::clamp(edgeClearance / EdgeSlipTime, 0.5f, 3.0f);
			const float awayFromEdge = glm::dot(applyVelocity, offEdge);
			if (awayFromEdge < slipSpeed)
				applyVelocity += offEdge * (slipSpeed - awayFromEdge);
		}

		if (glm::isnan(applyVelocity).x || glm::isnan(applyVelocity).y || glm::isnan(applyVelocity).z)
		{
			assert(false);
			applyVelocity = vec3(0.01f);
		}

		// -------------------------------
		// 5) Set final velocity
		// -------------------------------
		SetVelocity(applyVelocity);

		// -------------------------------
		// 6) If still attached, update local attach point and platform velocity
		// -------------------------------
		if (lastStandingOnBodyPtr)
		{
			vec3 basePos = FromPhysics(lastStandingOnBodyPtr->GetPosition());
			glm::quat baseRot = FromPhysics(lastStandingOnBodyPtr->GetRotation());
			vec3 charPos = FromPhysics(body->GetPosition());

			baseLocalAttachPoint = glm::inverse(baseRot) * (charPos - basePos);
			prevAttachWorldPos = charPos;
			prevBaseRotation = baseRot;
			prevBasePosition = basePos;

			vec3 linearVel = FromPhysics(lastStandingOnBodyPtr->GetLinearVelocity());
			vec3 angularVel = FromPhysics(lastStandingOnBodyPtr->GetAngularVelocity());
			vec3 worldOffset = baseRot * baseLocalAttachPoint;
			lastPlatformVelocity = linearVel + glm::cross(angularVel, worldOffset);
		}
	}

	// -------------------------------
	// 7) Character-on-character stacking: ignore pairs + horizontal push
	// -------------------------------
	UpdateCharacterStacking(deltaTime);

	if (sensorBody && body)
	{
		Physics::SetBodyPosition(sensorBody, FromPhysics(body->GetPosition()) - vec3(0, stepHeight * 0.5f, 0));
		Physics::SetLinearVelocity(sensorBody, FromPhysics(body->GetLinearVelocity()));
	}


	// Update camera height target based on state
	targetCameraHeight = (isCrouched) ? cameraHeightCrouching : cameraHeightStanding;

	// Smooth camera height
	float smoothFactor = 1.0f - std::exp(-stepSmoothingSpeed * deltaTime);
	currentCameraHeight = glm::mix(currentCameraHeight, targetCameraHeight, smoothFactor);
}


//returns center of character controller
vec3 CharacterController::GetPosition()
{
	if (body)
	{
		return FromPhysics(body->GetPosition()) - vec3(0, stepHeight * 0.5f, 0);
	}

	return vec3();
}

vec3 CharacterController::GetSmoothPosition()
{
	return GetPosition() + vec3(0, heightSmoothOffset, 0);
}

//sets center of character controller
void CharacterController::SetPosition(vec3 position)
{
	if (body)
		Physics::SetBodyPosition(body, position + vec3(0, stepHeight * 0.5f, 0));
	if (sensorBody)
		Physics::SetBodyPosition(sensorBody, position + vec3(0, stepHeight * 0.5f, 0));
}

void CharacterController::SetSmoothPosition(vec3 position)
{
	if (body)
	{
		Physics::SetBodyPosition(body, position + vec3(0, stepHeight, 0) - vec3(0, heightSmoothOffset, 0));
	}
}

vec3 CharacterController::GetSmoothOffset()
{
	return vec3(0, heightSmoothOffset, 0);
}

void CharacterController::UpdateSmoothPosition(float deltaTime)
{

	heightSmoothOffset *= std::exp(-stepSmoothingSpeed * deltaTime);

}

vec3 CharacterController::GetVelocity()
{
	if (!body) return vec3();
	return FromPhysics(body->GetLinearVelocity());
}

void CharacterController::SetVelocity(vec3 vel)
{
	if (!body) return;
	Physics::SetLinearVelocity(body, vel);
}

float CharacterController::GetCameraHeight()
{
	return currentCameraHeight;
}

// ── Crouch ─────────────────────────────────────────────────────────────────
void CharacterController::Crouch()
{
	if (isCrouched) return;

	vec3 oldVel = GetVelocity();

	vec3 currentBodyPos = FromPhysics(body->GetPosition());
	float oldHeight = height;
	float newHeight = crouchHeight;
	float delta = oldHeight - newHeight;
	vec3 deltaPos(0.0f);

	if (onGround)
	{
		deltaPos.y = -delta / 2.0f;
	}
	else
	{
		float oldCam = cameraHeightStanding;
		float newCam = cameraHeightCrouching;
		deltaPos.y = (newHeight - oldHeight) * 0.5f + (oldCam - newCam);
		currentCameraHeight = cameraHeightCrouching;
	}

	vec3 newBodyPos = currentBodyPos + deltaPos;

	Destroy(); // destroys both body and sensorBody, cleans up ignore pairs

	stepHeight = 0.25f;

	body = Physics::CreateCharacterCylinderBody(owner, newBodyPos, radius, newHeight - stepHeight, 30);
	body->GetMotionProperties()->SetLinearDamping(0);
	Physics::GetBodyData(body)->dynamicCollisionGroupOrMask = true;
	Physics::GetBodyData(body)->sensorCollisionMode = SensorCollisionMode::NotCollideWithSensors;
	Physics::SetGravityFactor(body, 0);

	sensorBody = Physics::CreateCharacterCylinderBody(owner, newBodyPos, radius, newHeight - stepHeight, 30);
	sensorBody->GetMotionProperties()->SetLinearDamping(0);
	Physics::GetBodyData(sensorBody)->dynamicCollisionGroupOrMask = true;
	Physics::GetBodyData(sensorBody)->sensorCollisionMode = SensorCollisionMode::CollideOnlyWithSensors;
	Physics::SetGravityFactor(sensorBody, 0);
	TagBodies();

	height = newHeight;
	isCrouched = true;

	SetVelocity(oldVel);
}

// ── UnCrouch ───────────────────────────────────────────────────────────────
void CharacterController::UnCrouch()
{
	if (!isCrouched) return;
	if (!CanStandUp()) return;

	vec3 oldVel = GetVelocity();

	vec3 currentBodyPos = FromPhysics(body->GetPosition());
	float oldHeight = height;
	float newHeight = standingHeight;
	float delta = newHeight - oldHeight;
	vec3 deltaPos(0.0f);

	if (onGround)
	{
		deltaPos.y = delta / 2.0f;
	}
	else
	{
		float oldCam = cameraHeightCrouching;
		float newCam = cameraHeightStanding;
		deltaPos.y = (newHeight - oldHeight) * 0.5f + (oldCam - newCam);
		currentCameraHeight = cameraHeightStanding;
	}

	vec3 newBodyPos = currentBodyPos + deltaPos;

	Destroy(); // cleans up ignore pairs before destroying bodies

	stepHeight = 0.4f;

	body = Physics::CreateCharacterCylinderBody(owner, newBodyPos, radius, newHeight - stepHeight, 30);
	body->GetMotionProperties()->SetLinearDamping(0);
	Physics::GetBodyData(body)->dynamicCollisionGroupOrMask = true;
	Physics::GetBodyData(body)->sensorCollisionMode = SensorCollisionMode::NotCollideWithSensors;
	Physics::SetGravityFactor(body, 0);

	sensorBody = Physics::CreateCharacterCylinderBody(owner, newBodyPos, radius, newHeight - stepHeight, 30);
	sensorBody->GetMotionProperties()->SetLinearDamping(0);
	Physics::GetBodyData(sensorBody)->dynamicCollisionGroupOrMask = true;
	Physics::GetBodyData(sensorBody)->sensorCollisionMode = SensorCollisionMode::CollideOnlyWithSensors;
	Physics::SetGravityFactor(sensorBody, 0);
	TagBodies();

	height = newHeight;
	isCrouched = false;

	SetVelocity(oldVel);
}

bool CharacterController::CanStandUp()
{
	if (!isCrouched) return true;

	vec3 currentBodyPos = FromPhysics(body->GetPosition());
	float oldHeight = height;
	float newHeight = standingHeight;
	float delta = newHeight - oldHeight;
	vec3 deltaPos(0.0f);

	if (onGround)
	{
		deltaPos.y = delta / 2.0f;
	}
	else
	{
		deltaPos.y = -delta / 2.0f;
	}

	vec3 newBodyPos = currentBodyPos + deltaPos;
	float newPhysHalf = (newHeight - stepHeight * 2.0f) / 2.0f;
	vec3 bottom = newBodyPos;
	bottom -= vec3(0, 0.15f, 0);
	vec3 top = newBodyPos + vec3(0.0f, newPhysHalf, 0.0f);

	Physics::HitResult result = Physics::CylinderTrace(bottom, top, radius - 0.05f, 0.05f, BodyType::GroupCollisionTest, { body }, { owner }, true);


	return !result.hasHit;
}

bool CharacterController::HasRoomAbove(float rise)
{
	// A sphere in the top cap of the body, a little thinner than it: a wall the body leans on is beside it, not above.
	const float bodyHalfHeight = (height - stepHeight) * 0.5f;
	const vec3  capCenter = FromPhysics(body->GetPosition()) + vec3(0, bodyHalfHeight - radius, 0);

	Physics::HitResult result = Physics::SphereTrace(capCenter, capCenter + vec3(0, rise, 0), radius - 0.05f, BodyType::GroupCollisionTest, { body }, { owner }, true);

	return !result.hasHit;
}

vec3 CharacterController::AwayFromEdgeUnderRim(float& clearance)
{
	// What touches the bottom cap of the body, told apart by where it touches it rather than by the contact normal (on
	// a mesh edge that can point straight up):
	//   - level with the cap center or above it: the side of the body, a wall;
	//   - under the cap, within perchRadius of the axis: ground under the center, the body may rest on it;
	//   - under the cap, further out: an edge hooking the rim. Those are collected.
	// The probe reaches a little beyond the cap: an edge the body is about to come down on counts before it touches,
	// so the body never lands on it to be lifted over it. (Flat ground it reaches lies within perchRadius: resting on
	// it, the probe touches a disc of sqrt((r + LookAhead)^2 - r^2) around the axis.)
	constexpr float Skin = 0.02f;
	constexpr float LookAhead = 0.05f;
	const float bodyHalfHeight = (height - stepHeight) * 0.5f;
	const vec3  bodyPosition = FromPhysics(body->GetPosition());
	const vec3  capCenter = bodyPosition - vec3(0, bodyHalfHeight - radius, 0);
	const float perchRadius = radius * ledgeSupportFraction;
	const BodyType groundMask = static_cast<BodyType>(BodyType::GroupCharacter & ~BodyType::CharacterCapsule);

	clearance = 0.0f;
	edgeContacts.clear();
	for (const Physics::HitResult& hit : Physics::MultiSphereOverlap(capCenter, radius + LookAhead, groundMask, {}, groundTraceIgnore, true))
	{
		if (hit.hitbody == nullptr || hit.hitbody->IsSensor())
			continue;

		const vec3 fromCap = hit.position - capCenter;
		if (fromCap.y > -radius * 0.2f)
			continue;
		const vec3 out = MathHelper::XZ(fromCap);
		if (glm::length(out) <= perchRadius)
			continue;
		edgeContacts.push_back(out);
	}
	if (edgeContacts.empty())
		return vec3(0.0f);

	// Away from the edges; edges on opposite sides (a gap the body bridges) cancel out.
	vec3 away = vec3(0.0f);
	for (const vec3& out : edgeContacts)
		away -= glm::normalize(out);
	if (glm::length(away) < 0.5f * static_cast<float>(edgeContacts.size()))
		return vec3(0.0f);
	away = glm::normalize(away);

	// Far enough that every edge is outside the cap.
	for (const vec3& out : edgeContacts)
		clearance = std::max(clearance, radius + Skin + glm::dot(out, away));
	clearance = std::min(clearance, radius);
	return away;
}


float CharacterController::GroundAngleRad(const glm::vec3& normal)
{
	// ensure the normal is normalized
	glm::vec3 n = glm::normalize(normal);

	// dot with world up (0,1,0)
	float cosTheta = std::clamp(n.y, -1.0f, 1.0f);

	// angle between the two vectors
	return std::acos(cosTheta);
}

float CharacterController::GroundAngleDeg(const glm::vec3& normal)
{
	return glm::degrees(GroundAngleRad(normal));
}

// ── SetLadderMode ──────────────────────────────────────────────────────────
//
// Called by Player::EnterLadder / ExitLadder.
//
// What it does:
//   1. Toggles suppressGravity so CharacterController::Update() stops applying
//      manual gravity while the player is on a ladder.
//   2. Rebuilds the physics body with stepHeight = 0 on entry (restored on
//      exit).  Normally the capsule bottom sits 0.4 m above the player's feet
//      (the step-height gap).  On a ladder that raised bottom catches on the
//      top corner of the platform when descending.  Zeroing stepHeight makes
//      the capsule sit flush with the feet so it slides past cleanly.
//   3. Preserves logical position: GetPosition() returns the same value before
//      and after the call because bodyPos is derived from GetPosition() with
//      the NEW stepHeight baked in.
//
void CharacterController::SetLadderMode(bool enabled)
{
	if (isOnLadder == enabled) return;

	isOnLadder = enabled;
	suppressGravity = enabled;

	// Save/restore the pre-ladder velocity so the body rebuild is transparent.
	vec3 oldVel = GetVelocity();
	// Logical center — accounts for the current stepHeight offset so the
	// character doesn't jump when the body is replaced.
	vec3 logicalPos = GetPosition();

	if (enabled)
	{
		savedStepHeight = stepHeight;
		stepHeight = 0.0f;
	}
	else
	{
		stepHeight = savedStepHeight;
	}

	// Body center in physics space = logical center + stepHeight/2 up.
	// With stepHeight == 0 (ladder) body center IS the logical center.
	vec3 bodyPos = logicalPos + vec3(0.0f, stepHeight * 0.5f, 0.0f);

	Destroy(); // cleans up ignore pairs, nulls body & sensorBody

	// Main body — capsule, same construction as Init().
	body = Physics::CreateCharacterBody(owner, bodyPos, radius, standingHeight - stepHeight, 30);
	body->GetMotionProperties()->SetLinearDamping(0);
	Physics::GetBodyData(body)->dynamicCollisionGroupOrMask = true;
	Physics::GetBodyData(body)->sensorCollisionMode = SensorCollisionMode::NotCollideWithSensors;
	Physics::SetGravityFactor(body, 0);

	// Sensor body — cylinder, full standing height, centred on logical position.
	// Update() will keep re-snapping it to body->GetPosition() - stepHeight*0.5
	// every frame so the initial placement just needs to be in the right ballpark.
	sensorBody = Physics::CreateCharacterCylinderBody(owner, logicalPos, radius, standingHeight, 30);
	sensorBody->GetMotionProperties()->SetLinearDamping(0);
	Physics::GetBodyData(sensorBody)->dynamicCollisionGroupOrMask = true;
	Physics::GetBodyData(sensorBody)->sensorCollisionMode = SensorCollisionMode::CollideOnlyWithSensors;
	Physics::SetGravityFactor(sensorBody, 0);
	TagBodies();

	height = standingHeight;

	SetVelocity(oldVel);
}

void CharacterController::UpdateGroundCheck(bool& hitsGround, float& calculatedGroundHeight, bool& canStand, vec3& avgNormal, vec3& notWalkableNormal)
{
	hitsGround = false;
	calculatedGroundHeight = 0;
	avgNormal = vec3(0, 1, 0);
	canStand = false;
	standingOnBody = BodyID();
	standingOnEntity = EntityHandle();
	notWalkableNormal = vec3(0, 1, 0);
	groundSupportOffset = vec3(0.0f);

	vec3 heightOffset = vec3(0, stepHeight, 0);
	float outheight = 0;
	bool outCanStand = false;
	vec3 outNormal = vec3();
	const Body* hitBody = nullptr;
	Entity* hitEntity = nullptr;

	// -------------------------------------------------------
	// NPC fast path: single center cast, skip ring samples
	// if surface is flat (normal.y close to 1.0)
	// -------------------------------------------------------
	if (movementQuality == CharacterControllerMovementQuality::NpcGeneric)
	{
		constexpr float flatThreshold = 0.99f; // ~cos(4.4 degrees)

		bool centerHit = CheckGroundAt(
			FromPhysics(body->GetPosition()) - heightOffset,
			radius - 0.01f,
			outheight, outCanStand, outNormal, &hitBody, &hitEntity);

		if (centerHit && (outNormal.y >= flatThreshold || GetVelocity().x != 0 || GetVelocity().z != 0))
		{
			// Perfectly flat ground - no need to sample around ring
			avgNormal = outNormal;
			float heightComp = GetPosition().y - height / 2.0f - 0.001f;
			hitsGround = (outheight > heightComp);
			canStand = hitsGround && outCanStand;
			calculatedGroundHeight = outheight;

			if (outCanStand)
			{
				standingOnBody = hitBody->GetID();
				standingOnEntity = EntityHandle::FromEntity(hitEntity);
			}

			return;
		}

		// Sloped or missed — fall through to reduced NPC sample loop below
	}


	if (movementQuality == CharacterControllerMovementQuality::NpcLowQuality)
	{
		constexpr float flatThreshold = MathHelper::constexpr_cos(30.0f * M_PI / 180.0f); // ~cos(30 degrees)

		bool centerHit = CheckGroundAt(
			FromPhysics(body->GetPosition()) - heightOffset,
			0,
			outheight, outCanStand, outNormal, &hitBody, &hitEntity);

		if (centerHit && outNormal.y >= flatThreshold)
		{
			// Perfectly flat ground - no need to sample around ring
			avgNormal = outNormal;
			float heightComp = GetPosition().y - height / 2.0f - 0.001f;
			hitsGround = (outheight > heightComp);
			canStand = hitsGround && outCanStand;
			calculatedGroundHeight = outheight;

			if (outCanStand)
			{
				standingOnBody = hitBody->GetID();
				standingOnEntity = EntityHandle::FromEntity(hitEntity);
			}

			return;
		}

		// Sloped or missed — fall through to reduced NPC sample loop below
	}

	// -------------------------------------------------------
	// Full / reduced sample loop
	// Walkable and non-walkable hits are accumulated separately.
	// Non-walkable hits (steep surfaces) still detect that ground
	// exists (hitsGround) but do NOT poison avgNormal or height.
	// If no walkable hits were found at all, we fall back to the
	// full set so behaviour on fully-steep ground is unchanged.
	// -------------------------------------------------------
	int numOfIterations = 16;
	float startRadius = 0.1f;
	float rayRadius = 0.1f;
	float radiusStep = 0.3f;

	if (ThreadPool::Supported() == false)
	{
		startRadius = 0.405f;
		numOfIterations = 4;
	}

	if (movementQuality == CharacterControllerMovementQuality::Player)
	{
		numOfIterations = 8;
		startRadius = 0.25f;
		rayRadius = 0.0f;
		radiusStep = 0.25f;
	}

	if (movementQuality == CharacterControllerMovementQuality::NpcGeneric)
	{
		numOfIterations = 4;
		startRadius = 1;
		rayRadius = 0;

		if (length(GetVelocity()) > 0.2f)
			startRadius = 0.5f;

	}

	if (movementQuality == CharacterControllerMovementQuality::NpcLowQuality)
	{
		numOfIterations = 4;
		startRadius = 1;
		rayRadius = 0;

		if (length(GetVelocity()) > 0.2f)
			startRadius = 1.0f;

	}

	// --- walkable bucket ---
	vec3  walkNormalSum = vec3(0);
	float walkHeightSum = 0;
	int   walkHits = 0;
	int   walkNormalCount = 0;

	// --- non-walkable bucket (steep / bad surfaces) ---
	vec3  steepNormalSum = vec3(0);
	float steepHeightSum = 0;
	int   steepHits = 0;
	int   steepNormalCount = 0;

	const float heightComp = GetPosition().y - height / 2.0f - 0.001f;

	// --- where the walkable ground is around the center (can the character stand on it, or only perch on an edge) ---
	const vec3  center = FromPhysics(body->GetPosition());
	const float perchRadius = radius * ledgeSupportFraction;
	bool  supportNearCenter = false;               // walkable ground within perchRadius of the center
	std::array<float, 64> supportAngles;           // directions of the walkable ring samples
	int   supportAngleCount = 0;
	vec3  underfootOffsetSum = vec3(0);            // walkable samples at the feet (not the ground a step below)
	int   underfootCount = 0;
	vec3  centerContactOffset = vec3(0);
	const float underfootHeight = heightComp - 0.05f;

	// Walkable ground further below the feet than a step is a drop, not ground to stand on. A floor half a meter below
	// a window sill used to count as support all around the character and pull the stance down towards it, while the
	// capsule hung on the sill.
	const float lowestGroundHeight = heightComp - stepHeight;
	// Likewise above: higher than a step (and the bottom of the body resting on flat ground, a little over it) is the
	// lip of something the rounded bottom leans on, not ground to step up onto.
	const float highestGroundHeight = heightComp + stepHeight + 0.1f;
	auto IsGroundHeight = [&](float h) { return h >= lowestGroundHeight && h <= highestGroundHeight; };

	// Helper: route one CheckGroundAt result into the right bucket.
	// Weight is 1 for ring samples, 3 for the weighted center cast.
	auto AccumulateHit = [&](bool hit, float weight, const Body* hb, Entity* he)
		{
			if (!hit) return;

			bool walkable = outCanStand; // set by the last CheckGroundAt call

			if (walkable && !IsGroundHeight(outheight))
				return;

			if (outheight > heightComp)
				hitsGround = true;

			if (walkable)
			{
				walkNormalSum += outNormal;
				walkHeightSum += outheight * weight;
				walkHits += static_cast<int>(weight);
				walkNormalCount += 1;
				canStand = true;
				if (hb) standingOnBody = hb->GetID();
				if (he) standingOnEntity = EntityHandle::FromEntity(he);
			}
			else
			{
				steepNormalSum += outNormal;
				steepHeightSum += outheight * weight;
				steepHits += static_cast<int>(weight);
				steepNormalCount += 1;
			}
		};

	// Ring samples
	for (float r = startRadius; r <= 1; r += radiusStep)
	{

		if (movementQuality == CharacterControllerMovementQuality::Player)
		{
			numOfIterations = 4
				+ 4 * (r >= 0.5f)
				+ 8 * (r >= 0.8f);
		}

		for (int i = 0; i < numOfIterations; i++)
		{
			float angle = (2.0f * M_PI / numOfIterations) * i;
			vec3  offset = vec3(cos(angle), 0.0f, sin(angle)) * (radius * r - rayRadius - 0.05f);

			bool hit = CheckGroundAt(
				FromPhysics(body->GetPosition()) + offset - heightOffset,
				rayRadius, outheight, outCanStand, outNormal, &hitBody, &hitEntity);

			AccumulateHit(hit, 1.0f, hitBody, hitEntity);

			if (hit && outCanStand && IsGroundHeight(outheight))
			{
				if (glm::length(offset) <= perchRadius)
					supportNearCenter = true;
				if (supportAngleCount < static_cast<int>(supportAngles.size()))
					supportAngles[supportAngleCount++] = std::atan2(offset.z, offset.x);
				if (outheight > underfootHeight)
				{
					underfootOffsetSum += offset;
					underfootCount++;
				}
			}
		}
	}

	// Weighted center cast (weight 3 — same as before)
	{
		vec3 contact;
		bool hit = CheckGroundAt(
			center - heightOffset,
			radius - 0.01f, outheight, outCanStand, outNormal, &hitBody, &hitEntity, &contact);

		AccumulateHit(hit, 3.0f, hitBody, hitEntity);

		// It touches the highest ground anywhere under the character: an edge it reaches tells where that edge is.
		if (hit && outCanStand && IsGroundHeight(outheight))
		{
			centerContactOffset = MathHelper::XZ(contact - center);
			if (glm::length(centerContactOffset) <= perchRadius)
				supportNearCenter = true;
			if (outheight <= underfootHeight)
				centerContactOffset = vec3(0);
		}
	}

	// The center is over the support when the walkable samples are not all on one side of it: no gap of half a turn
	// between their directions (a gap or a hole the character bridges, rather than an edge it hangs over).
	bool supportAroundCenter = false;
	if (supportAngleCount >= 3)
	{
		std::sort(supportAngles.begin(), supportAngles.begin() + supportAngleCount);
		float largestGap = supportAngles[0] + 2.0f * static_cast<float>(M_PI) - supportAngles[supportAngleCount - 1];
		for (int i = 1; i < supportAngleCount; i++)
			largestGap = std::max(largestGap, supportAngles[i] - supportAngles[i - 1]);
		supportAroundCenter = largestGap < static_cast<float>(M_PI);
	}

	groundSupportOffset = underfootCount > 0 ? underfootOffsetSum / static_cast<float>(underfootCount) : centerContactOffset;

	// -------------------------------------------------------
	// Resolve: prefer walkable bucket; fall back to full set
	// only when there are zero walkable hits (pure steep ground).
	// -------------------------------------------------------
	if (walkHits > 0)
	{
		// Normal surface or mixed (player partially on steep edge):
		// use only walkable samples so steep patches don't tilt the
		// normal and cause the bogus downhill acceleration / slow-down.
		avgNormal = walkNormalSum / static_cast<float>(walkNormalCount);
		calculatedGroundHeight = walkHeightSum / static_cast<float>(walkHits);
	}
	else if (steepHits > 0)
	{
		// Entirely on steep ground — keep old behaviour so gravity /
		// slope-slide still work correctly.
		avgNormal = steepNormalSum / static_cast<float>(steepNormalCount);
		calculatedGroundHeight = steepHeightSum / static_cast<float>(steepHits);
	}

	if (steepNormalCount > 0)
	{
		notWalkableNormal = steepNormalSum / static_cast<float>(steepNormalCount);
	}
	else
	{
		notWalkableNormal = vec3(0, 1, 0);
	}

	const int totalHits = walkHits + steepHits;
	hitsGround = hitsGround && (totalHits > 0);
	canStand = hitsGround && (GroundAngleDeg(avgNormal) <= groundMaxAngle) && canStand;

	// Standing needs the center over the support. Ground reaching only the rim of the character (a sill or trim along
	// a wall, the corner of a post) would leave it perched on the edge: it is touched, not stood on. A thin support
	// near the center can fall between the samples, so look there once before refusing.
	if (canStand && !supportNearCenter && !supportAroundCenter)
	{
		float perchHeight = 0;
		bool  perchWalkable = false;
		vec3  perchNormal;
		const Body* perchBody = nullptr;
		supportNearCenter = CheckGroundAt(center - heightOffset, perchRadius, perchHeight, perchWalkable, perchNormal, &perchBody)
			&& perchWalkable && IsGroundHeight(perchHeight);
	}
	canStand = canStand && (supportNearCenter || supportAroundCenter);
}

bool CharacterController::CheckGroundAt(vec3 location, float checkRadius, float& outheight, bool& canStand, vec3& normal, const Body** hitBody, Entity** hitEntity, vec3* hitPoint)
{
	Physics::HitResult result;

	vec3 start = location + vec3(0, 0.2f, 0);
	vec3 end = location - vec3(0, height / 2.0f + stepHeight, 0);

	// Characters (their capsule / sensor bodies, this one's included) are not ground: the trace goes through them to
	// what is below. Rejecting a hit on another character used to lose the ground under a body in a crowd, and finding
	// out cost a scan over every controller per hit.
	const BodyType groundMask = static_cast<BodyType>(BodyType::GroupCharacter & ~BodyType::CharacterCapsule);
	static const std::vector<Body*> noBodies;
	if (checkRadius > 0)
	{
		result = Physics::CylinderTrace(start, end - vec3(0, 0.05f, 0), checkRadius, 0.05f, groundMask, noBodies, groundTraceIgnore, true);
		if (hitPoint) *hitPoint = result.position;   // where the cylinder touched, before it becomes its bottom
		result.position = result.shapePosition - vec3(0, 0.02f, 0);
	}
	else
	{
		result = Physics::LineTrace(start, end, groundMask, noBodies, groundTraceIgnore, true);
		if (hitPoint) *hitPoint = result.position;
	}

	*hitBody = result.hitbody;
	if (hitEntity) *hitEntity = result.entity;

	if (result.normal.y < 0.1)
		return false;

	outheight = result.position.y;

	canStand = result.hasHit && (GroundAngleDeg(result.normal) <= groundMaxAngle);

	normal = result.normal;

	return result.hasHit;
}

// ---------------------------------------------------------------------------
// UpdateCharacterStacking
//
// Called once per Update().  This controller is the "top" character.
//
// 1. Find characters that are currently below us (broadphase box under the feet)
//    (our feet near their top, XZ overlap).
// 2. Add ignore pairs for newly detected bodies so we pass through them
//    vertically while still being able to push horizontally.
// 3. Remove ignore pairs for bodies that are no longer below us.
// 4. Apply a symmetric horizontal separation impulse to both bodies for
//    every active stacking pair so they push apart over time.
// ---------------------------------------------------------------------------
void CharacterController::UpdateCharacterStacking(float deltaTime)
{
	if (!body) return;

	const BodyID myID = body->GetID();
	const vec3   myPos = GetPosition();
	const float  myFeetY = myPos.y - height * 0.5f;

	// ── 1. Detect which characters are currently below us ───────────────────
	// Candidates: moving bodies whose bounds overlap the band under our feet where a stacked character's top can be
	// (a broadphase query, not a scan over every controller).
	constexpr float MaxOtherRadius = 1.0f;
	const float reach = radius + MaxOtherRadius + 0.15f;
	stackingCandidates.clear();
	Physics::CollectBodiesInBox(
		vec3(myPos.x - reach, myFeetY - height * 0.5f, myPos.z - reach),
		vec3(myPos.x + reach, myFeetY + stepHeight + 0.3f, myPos.z + reach),
		Layers::MOVING, stackingCandidates);

	std::vector<BodyID>& detectedThisFrame = stackedBelow;
	detectedThisFrame.clear();

	for (const BodyID& candidate : stackingCandidates)
	{
		CharacterController* other = ControllerOf(candidate);
		if (other == nullptr || other == this || !other->body || other->body->GetID() != candidate) continue;   // capsules only

		const vec3  otherPos = other->GetPosition();
		const float otherTopY = otherPos.y + other->height * 0.5f;
		const float combinedRadii = radius + other->radius;

		// Horizontal distance (XZ only)
		const vec3  diff3 = myPos - otherPos;
		const float xzDist = glm::length(vec2(diff3.x, diff3.z));

		// yDiff ~= 0 when our feet sit exactly on their top.
		// Accept a window above (+height*0.5) so that the pair is detected
		// slightly before contact, and below (-stepHeight - 0.3) to keep
		// the pair alive while we are passing through.
		const float yDiff = myFeetY - otherTopY;
		const bool  verticallyStacked = (yDiff > -(stepHeight + 0.3f)) && (yDiff < height * 0.5f);

		// Small extra margin so the pair is established just before the
		// bodies visually overlap.
		const bool  horizontallyClose = xzDist < combinedRadii + 0.15f;

		if (verticallyStacked && horizontallyClose)
			detectedThisFrame.push_back(other->body->GetID());
	}

	// ── 2. Add ignore pairs for newly detected bodies ────────────────────────
	for (const BodyID& id : detectedThisFrame)
	{
		const bool alreadyIgnored = std::find(activeIgnorePairs.begin(), activeIgnorePairs.end(), id) != activeIgnorePairs.end();
		if (!alreadyIgnored)
		{
			Physics::AddIgnorePair(myID, id);
			activeIgnorePairs.push_back(id);
			removeCollisionCooldown[id].AddDelay(0.3);
		}
	}

	// ── 3. Remove ignore pairs for bodies no longer below us ─────────────────
	for (auto it = activeIgnorePairs.begin(); it != activeIgnorePairs.end(); )
	{
		bool stillDetected = std::find(detectedThisFrame.begin(), detectedThisFrame.end(), *it) != detectedThisFrame.end();
		if (removeCollisionCooldown[*it].Wait())
			stillDetected = true;

		if (!stillDetected)
		{
			Physics::RemoveIgnorePair(myID, *it);
			it = activeIgnorePairs.erase(it);
		}
		else
			++it;
	}

	// ── 4. Horizontal separation push ────────────────────────────────────────
	// Only the top character (this) drives the push so there is no
	// double-counting between two controllers that are stacked.
	for (const BodyID& paired : activeIgnorePairs)
	{
		CharacterController* other = ControllerOf(paired);
		if (other == nullptr || other == this || !other->body) continue;

		vec3 diff = GetPosition() - other->GetPosition();
		diff.y = 0.0f; // horizontal only

		const float dist = glm::length(diff);
		const float combinedRadii = radius + other->radius;
		const float overlap = combinedRadii - dist;

		if (overlap <= 0.0f) continue;

		// Degenerate case: characters at the exact same XZ position.
		// Pick an arbitrary direction so they always separate.
		const vec3 pushDir = (dist > 0.001f)
			? glm::normalize(diff)
			: vec3(1.0f, 0.0f, 0.0f);

		// Frame-rate-independent continuous push, scaled by overlap depth.
		const float magnitude = overlap * 8.0f;
		Physics::AddImpulse(body, pushDir * (magnitude * deltaTime));
		Physics::AddImpulse(other->body, pushDir * (-magnitude * deltaTime));
	}
}