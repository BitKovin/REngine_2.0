#include "CharacterController.h"

CharacterController::CharacterController()
{

}

CharacterController::~CharacterController()
{
	if (body)
	{
		Physics::DestroyBody(body);
	}
}


void CharacterController::Init(Entity* owner, vec3 position,float radius, float height)
{

	Destroy();

	body = Physics::CreateCharacterCylinderBody(owner, position, radius, height - stepHeight, 30);
	body->GetMotionProperties()->SetLinearDamping(0);

	Physics::GetBodyData(body)->dynamicCollisionGroupOrMask = true;


	this->height = height;
	this->radius = radius;

	Physics::SetGravityFactor(body, 0);

}

void CharacterController::Destroy()
{

	if (body == nullptr) return;

	Physics::DestroyBody(body);
	body = nullptr;
}

void CharacterController::Update(float deltaTime)
{
	UpdateSmoothPosition(deltaTime);

	// -------------------------------
	// 1) If previously attached, preemptively apply platform delta
	// -------------------------------
	if (lastStandingOnBody)
	{
		vec3 basePos = FromPhysics(lastStandingOnBody->GetPosition());
		glm::quat baseRot = FromPhysics(lastStandingOnBody->GetRotation());

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
				Physics::SweepBody(body, targetPos);
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
	vec3 notWalkableNormal = vec3();
	UpdateGroundCheck(standsOnGround, verticalPosition, onGround, notWalkableNormal);

	const Body* currentBase = standingOnBody;

	// -------------------------------
	// 3) Platform attach/detach detection
	// -------------------------------
	bool wasAttached = (lastStandingOnBody != nullptr);
	bool isAttached = (currentBase != nullptr);

	if (wasAttached && !isAttached)
	{
		// Detached: apply last platform velocity to character
		// (use fresh computation for accuracy on rotation)
		vec3 basePos = FromPhysics(lastStandingOnBody->GetPosition());
		glm::quat baseRot = FromPhysics(lastStandingOnBody->GetRotation());
		vec3 linearVel = FromPhysics(lastStandingOnBody->GetLinearVelocity());
		vec3 angularVel = FromPhysics(lastStandingOnBody->GetAngularVelocity());
		vec3 worldOffset = baseRot * baseLocalAttachPoint;
		vec3 platformVelAtAttach = linearVel + glm::cross(angularVel, worldOffset);

		vec3 currentVel = GetVelocity();
		currentVel += platformVelAtAttach;
		SetVelocity(currentVel);
		lastPlatformVelocity = vec3(0.0f);
	}
	else if (wasAttached && isAttached && (currentBase != lastStandingOnBody))
	{
		// Switched platforms: detach from old, attach to new
		// Detach from old
		vec3 oldBasePos = FromPhysics(lastStandingOnBody->GetPosition());
		glm::quat oldBaseRot = FromPhysics(lastStandingOnBody->GetRotation());
		vec3 oldLinearVel = FromPhysics(lastStandingOnBody->GetLinearVelocity());
		vec3 oldAngularVel = FromPhysics(lastStandingOnBody->GetAngularVelocity());
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

	lastStandingOnBody = currentBase;

	// -------------------------------
	// 4) Original character movement / gravity / slope logic
	// -------------------------------
	vec3 velocity = GetVelocity();

	if (velocity.y > 0)
	{
		onGround = false;
	}

	if (standsOnGround && velocity.y < 0)
	{
		velocity.y -= gravity * deltaTime * (1.0f - notWalkableNormal.y);
	}
	else
	{
		velocity.y -= gravity * deltaTime;
	}

	if (standsOnGround && velocity.y <= 0)
	{
		vec3 currentPosition = FromPhysics(body->GetPosition());
		float newVerticalPosition = verticalPosition + stepHeight + height / 2;
		Physics::SweepBody(body, vec3(currentPosition.x, newVerticalPosition, currentPosition.z));
		heightSmoothOffset += (currentPosition.y - FromPhysics(body->GetPosition()).y);
	}

	if (onGround)
	{
		velocity.y = 0;
	}

	vec3 applyVelocity = velocity;

	if (onGround == false && standsOnGround && (velocity.y <= 0))
	{
		// Project velocity onto slope plane
		vec3 slopeNormal = normalize(notWalkableNormal);
		vec3 slopeTangent = velocity - slopeNormal * dot(velocity, slopeNormal);
		applyVelocity = slopeTangent;
		applyVelocity.y = velocity.y;
		UpdateSmoothPosition(deltaTime * 2);
	}
	else if (onGround == false && standsOnGround)
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

	// -------------------------------
	// 5) Set final velocity
	// -------------------------------
	SetVelocity(applyVelocity);

	// -------------------------------
	// 6) If still attached, update local attach point and platform velocity
	// -------------------------------
	if (lastStandingOnBody)
	{
		vec3 basePos = FromPhysics(lastStandingOnBody->GetPosition());
		glm::quat baseRot = FromPhysics(lastStandingOnBody->GetRotation());
		vec3 charPos = FromPhysics(body->GetPosition());

		baseLocalAttachPoint = glm::inverse(baseRot) * (charPos - basePos);
		prevAttachWorldPos = charPos;
		prevBaseRotation = baseRot;
		prevBasePosition = basePos;

		vec3 linearVel = FromPhysics(lastStandingOnBody->GetLinearVelocity());
		vec3 angularVel = FromPhysics(lastStandingOnBody->GetAngularVelocity());
		vec3 worldOffset = baseRot * baseLocalAttachPoint;
		lastPlatformVelocity = linearVel + glm::cross(angularVel, worldOffset);
	}
}


//returns center of character controller
vec3 CharacterController::GetPosition()
{
	if (body)
	{
		return FromPhysics(body->GetPosition()) - vec3(0,stepHeight,0);
	}

	return vec3();
}

vec3 CharacterController::GetSmoothPosition()
{
	return GetPosition() + vec3(0, heightSmoothOffset,0);
}

//sets center of character controller
void CharacterController::SetPosition(vec3 position)
{
	if (body)
	{
		Physics::SetBodyPosition(body, position + vec3(0, stepHeight, 0));
	}
}

void CharacterController::SetSmoothPosition(vec3 position)
{
	if (body)
	{
		Physics::SetBodyPosition(body, position + vec3(0, stepHeight, 0) - vec3(0, heightSmoothOffset, 0));
	}
}

void CharacterController::UpdateSmoothPosition(float deltaTime)
{

	heightSmoothOffset *= std::exp(-stepSmoothingSpeed * deltaTime);
	return;

	if (heightSmoothOffset != 0)
	{

		if (heightSmoothOffset > 0)
		{
			heightSmoothOffset -= deltaTime * stepSmoothingSpeed;
			
			heightSmoothOffset = glm::clamp(heightSmoothOffset, 0.0f, 0.4f);
		}

		if (heightSmoothOffset < 0)
		{
			heightSmoothOffset += deltaTime * stepSmoothingSpeed;

			heightSmoothOffset = glm::clamp(heightSmoothOffset, -1.0f, 0.0f);
		}

	}

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

void CharacterController::UpdateGroundCheck(bool& hitsGround, float& calculatedGroundHeight, bool& canStand, vec3& avgNormal)
{
	hitsGround = false;
	calculatedGroundHeight = 0;
	avgNormal = vec3(0,0,0);
	canStand = false;
	standingOnBody = nullptr;

	if (GetVelocity().y > 0)
	{

		//return;

	}

	int numOfIterations = 16;

	float accumulatedHeight = 0;
	int numOfHits = 0;

	float outheight = 0;
	bool outCanStand = false;

	vec3 heightOffset = vec3(0, stepHeight, 0);

	vec3 outNormal = vec3();

	int nNotWalk = 0;

	float startRadius = 0.1;
	
	float stepRadius = 0.3;

	const Body* hitBody = nullptr;

	if (ThreadPool::Supported() == false)
	{
		startRadius = 0.4;

		numOfIterations = 8;

	}

	

	for (float r = 0.1; r <= 1; r += 0.3)
	{

		for (int i = 0; i < numOfIterations; i++)
		{
			float angle = (2.0f * M_PI / numOfIterations) * i; // Full circle in radians

			vec3 offset = vec3(cos(angle), 0.0f, sin(angle)) * (radius * r - 0.11f);


			if (CheckGroundAt(FromPhysics(body->GetPosition()) + offset - heightOffset,0.1f, outheight, outCanStand, outNormal, &hitBody))
			{

				if (outCanStand)
				{
					canStand = true;
				}

				avgNormal += outNormal;
				nNotWalk++;

				float heightComp = GetPosition().y - height / 2.0f - 0.001f;

				if (outheight > heightComp)
				{
					hitsGround = true;
				}

				accumulatedHeight += outheight;
				numOfHits++;
			}

		}

	}
	

	if (CheckGroundAt(FromPhysics(body->GetPosition()) - heightOffset, radius - 0.01f, outheight, outCanStand, outNormal, &hitBody))
	{

		if (outCanStand)
		{
			canStand = true;

			standingOnBody = hitBody;

		}
		
		avgNormal += outNormal;
		nNotWalk++;

		float heightComp = GetPosition().y - height / 2.0f - 0.001f;

		if (outheight > heightComp)
		{
			hitsGround = true;
		}

		if (numOfHits > 0 && false)
		{
			accumulatedHeight += outheight * numOfHits;
			numOfHits *= 2;
		}
		else
		{
			accumulatedHeight += outheight * 3;
			numOfHits += 3;
		}

		
	}

	if (nNotWalk)
	{
		avgNormal /= nNotWalk;
	}

	hitsGround = hitsGround && (numOfHits>0);

	canStand = hitsGround && (GroundAngleDeg(avgNormal) <= groundMaxAngle);

	calculatedGroundHeight = accumulatedHeight / numOfHits;

}

bool CharacterController::CheckGroundAt(vec3 location,float checkRadius, float& outheight, bool& canStand, vec3& normal, const Body** hitBody)
{


	Physics::HitResult result;
	
	if (checkRadius > 0)
	{
		result = Physics::CylinderTrace(location, location - vec3(0, height / 2 + stepHeight - 0.05f, 0), checkRadius, 0.05f, BodyType::GroupCollisionTest, { body });

		result.position = result.shapePosition - vec3(0, 0.02f,0);

	}
	else
	{
		result = Physics::LineTrace(location, location - vec3(0, height / 2 + stepHeight, 0), BodyType::GroupCollisionTest, { body });
	}

	*hitBody = result.hitbody;

	if (result.normal.y < 0.1)
		return false;

	outheight = result.position.y;

	canStand = result.hasHit && (GroundAngleDeg(result.normal) <= groundMaxAngle);

	//DebugDraw::Line(result.position, result.position + result.normal, 0.01f);


	normal = result.normal;

	return result.hasHit;
}
