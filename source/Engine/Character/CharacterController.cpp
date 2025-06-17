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

	float verticalPosition;
	bool standsOnGround;
	vec3 notWalkableNormal = vec3();
	UpdateGroundCheck(standsOnGround, verticalPosition, onGround, notWalkableNormal);


	vec3 velocity = GetVelocity();
	
	if (standsOnGround && velocity.y<0)
	{
		velocity.y -= gravity * deltaTime * (1.0f - notWalkableNormal.y);
	}
	else
	{
		velocity.y -= gravity * deltaTime;
	}

	if (standsOnGround)
	{
		vec3 currentPosition = FromPhysics(body->GetPosition());
		float newVerticalPosition = verticalPosition + stepHeight + height / 2;
		Physics::SweepBody(body, vec3(currentPosition.x, newVerticalPosition, currentPosition.z));
		heightSmoothOffset += (currentPosition.y - body->GetPosition().GetY());
	}

	if (onGround)
	{
		velocity.y = 0;
	}

	vec3 applyVelocity = velocity;

	if (onGround == false && standsOnGround)
	{
		// Project velocity onto the surface plane to maintain momentum along the slope
		vec3 slopeNormal = normalize(notWalkableNormal);
		vec3 slopeTangent = velocity - slopeNormal * dot(velocity, slopeNormal);

		// Add downward slide effect while preserving horizontal momentum
		applyVelocity = slopeTangent;
		applyVelocity.y = velocity.y; // Maintain gravity effect

		UpdateSmoothPosition(deltaTime * 2); // Smoothes camera offset faster
	}

	SetVelocity(vec3(applyVelocity.x, applyVelocity.y, applyVelocity.z));
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
	return FromPhysics(body->GetLinearVelocity());
}

void CharacterController::SetVelocity(vec3 vel)
{
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

	for (float r = 0.1; r <= 1; r += 0.3)
	{

		for (int i = 0; i < numOfIterations; i++)
		{
			float angle = (2.0f * M_PI / numOfIterations) * i; // Full circle in radians

			vec3 offset = vec3(cos(angle), 0.0f, sin(angle)) * (radius * r - 0.11f);


			if (CheckGroundAt(FromPhysics(body->GetPosition()) + offset - heightOffset,0.1f, outheight, outCanStand, outNormal))
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

	if (CheckGroundAt(FromPhysics(body->GetPosition()) - heightOffset, radius - 0.01f, outheight, outCanStand, outNormal))
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

		if (numOfHits > 0)
		{
			accumulatedHeight += outheight * numOfHits;
			numOfHits *= 2;
		}
		else
		{
			accumulatedHeight += outheight;
			numOfHits ++;
		}

		
	}

	if (nNotWalk)
	{
		avgNormal /= nNotWalk;
	}

	hitsGround = hitsGround && (numOfHits>0);

	printf("%f \n", GroundAngleDeg(avgNormal));

	canStand = hitsGround && (GroundAngleDeg(avgNormal) <= groundMaxAngle);

	calculatedGroundHeight = accumulatedHeight / numOfHits;

}

bool CharacterController::CheckGroundAt(vec3 location,float checkRadius, float& outheight, bool& canStand, vec3& normal)
{

	Physics::HitResult result;
	
	if (radius > 0)
	{
		result = Physics::CylinderTrace(location, location - vec3(0, height / 2 + stepHeight - 0.05f, 0), checkRadius, 0.05f, BodyType::GroupCollisionTest, { body });

		result.position = result.shapePosition - vec3(0, 0.02f,0);

	}
	else
	{
		result = Physics::LineTrace(location, location - vec3(0, height / 2 + stepHeight, 0), BodyType::GroupCollisionTest, { body });
	}

	if (result.normal.y < 0.1)
		return false;

	outheight = result.position.y;

	canStand = result.hasHit && (GroundAngleDeg(result.normal) <= groundMaxAngle);

	//DebugDraw::Line(result.position, result.position + result.normal, 0.01f);



	normal = result.normal;

	return result.hasHit;
}
