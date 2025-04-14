#include "TestNpc.hpp"

#include "Player.hpp"

#include "../Navigation/Navigation.hpp"

void TestNpc::AsyncUpdate()
{
	mesh->UpdatePose = mesh->WasRended;

	mesh->Update();

	mesh->Position = Position - vec3(0,1,0);
	

	Player* target = Player::Instance;

	if (target)
	{
		auto path = NavigationSystem::FindSimplePath(Position, target->Position);

		if (path.size()) 
		{

			desiredDirection = MathHelper::XZ(MathHelper::GetForwardVector(MathHelper::FindLookAtRotation(Position, path[0])));

		}

	}

	movingDirection = mix(movingDirection, desiredDirection, Time::DeltaTime*5);

	vec3 realMoveDirection = MathHelper::FastNormalize(movingDirection);

	// Get the current horizontal velocity (preserving the vertical component from physics)
	vec3 currentVelocity = FromPhysics(LeadBody->GetLinearVelocity());
	vec3 currentHorizontalVel(currentVelocity.x, 0.0f, currentVelocity.z);

	// Determine the desired horizontal velocity (5.0f is the intended speed)
	vec3 desiredHorizontalVel = realMoveDirection * 5.0f;

	// Calculate the change in velocity you need to achieve over the current frame
	// Using Time::DeltaTime (dt) to convert velocity difference to the required acceleration
	float dt = Time::DeltaTime;
	vec3 neededAcceleration = (desiredHorizontalVel - currentHorizontalVel) / dt;

	// Retrieve the body mass to calculate the needed force (F = m * a)
	float mass = 50;
	vec3 forceToApply = neededAcceleration * mass;

	// Only apply horizontal forces to avoid interfering with the vertical (gravity, jump, etc.)
	vec3 horizontalForce(forceToApply.x, 0.0f, forceToApply.z);

	// Apply the calculated force to the body
	LeadBody->AddForce(ToPhysics(horizontalForce));

	mesh->Rotation = vec3(0,MathHelper::FindLookAtRotation(vec3(), realMoveDirection).y, 0);

}
