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
	body = Physics::CreateCharacterCylinderBody(owner, position, radius, height - stepHeight, 30);

	body->GetMotionProperties()->SetLinearDamping(0);

	this->height = height;
	this->radius = radius;

	Physics::SetGravityFactor(body, 0);

}

void CharacterController::Destroy()
{
	Physics::DestroyBody(body);
	body = nullptr;
}

void CharacterController::Update(float deltaTime)
{

	UpdateSmoothPosition(deltaTime);

	float verticalPosition;
	UpdateGroundCheck(onGround, verticalPosition);

	vec3 velocity = GetVelocity();
	velocity.y -= gravity * deltaTime;


	if (onGround)
	{

		vec3 currentPosition = FromPhysics(body->GetPosition());

		float newVerticalPosition = verticalPosition + stepHeight + height / 2;



		Physics::SweepBody(body, vec3(currentPosition.x, newVerticalPosition, currentPosition.z));

		heightSmoothOffset += (currentPosition.y - body->GetPosition().GetY());

		velocity.y = 0;

	}


	//Physics::SetBodyPosition(body, FromPhysics(body->GetPosition()) + velocity * deltaTime);
	SetVelocity(velocity);



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

void CharacterController::UpdateGroundCheck(bool& hitsGround, float& calculatedGroundHeight)
{
	hitsGround = false;
	calculatedGroundHeight = 0;

	if (GetVelocity().y > 0)
	{

		return;

	}

	int numOfIterations = 16;

	float accumulatedHeight = 0;
	int numOfHits = 0;

	float outheight = 0;

	vec3 heightOffset = vec3(0, stepHeight, 0);

	for (float r = 0.1; r <= 1; r += 0.3)
	{

		for (int i = 0; i < numOfIterations; i++)
		{
			float angle = (2.0f * M_PI / numOfIterations) * i; // Full circle in radians
			vec3 offset = vec3(cos(angle), 0.0f, sin(angle)) * radius * r - 0.11f;



			if (CheckGroundAt(FromPhysics(body->GetPosition()) + offset - heightOffset,0.1f, outheight))
			{

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

	if (CheckGroundAt(FromPhysics(body->GetPosition()) - heightOffset, radius - 0.003f, outheight))
	{

		float heightComp = GetPosition().y - height / 2.0f - 0.001f;

		if (outheight > heightComp)
		{
			hitsGround = true;
		}

		accumulatedHeight += outheight;
		numOfHits++;
	}

	hitsGround = hitsGround && (numOfHits>0);

	calculatedGroundHeight = accumulatedHeight / numOfHits;

}

bool CharacterController::CheckGroundAt(vec3 location,float radius, float& outheight)
{

	Physics::HitResult result;
	
	if (radius > 0)
	{
		result = Physics::CylinderTrace(location, location - vec3(0, height / 2 + stepHeight - 0.1f, 0), radius, 0.1f, BodyType::GroupCollisionTest, { body });
	}
	else
	{
		result = Physics::LineTrace(location, location - vec3(0, height / 2 + stepHeight, 0), BodyType::GroupCollisionTest, { body });
	}


	outheight = result.position.y;

	return result.hasHit && (GroundAngleDeg(result.normal)<=groundMaxAngle);
}
