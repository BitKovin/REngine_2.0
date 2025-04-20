#include "TestNpc.hpp"
#include "Player/Player.hpp"

#include <Navigation/Navigation.hpp>

REGISTER_ENTITY(TestNpc, "testnpc")

void TestNpc::AsyncUpdate()
{
	mesh->UpdatePose = mesh->WasRended;

	mesh->Update();

	mesh->Position = Position - vec3(0,1,0);
	

	Entity* target = Player::Instance;

	if (target)
	{
		pathFollow.UpdateStartAndTarget(Position, target->Position);
		pathFollow.TryPerform();

	}
	if (pathFollow.FoundTarget)
	{
		desiredDirection = MathHelper::FastNormalize(MathHelper::XZ(pathFollow.CalculatedTargetLocation - Position));
	}
	

	movingDirection = mix(movingDirection, desiredDirection, Time::DeltaTime*3);

	movingDirection = MathHelper::FastNormalize(movingDirection);

	// Get the current horizontal velocity (preserving the vertical component from physics)
	vec3 currentVelocity = FromPhysics(LeadBody->GetLinearVelocity());
	vec3 currentHorizontalVel(currentVelocity.x, 0.0f, currentVelocity.z);

	// Determine the desired horizontal velocity (5.0f is the intended speed)
	vec3 desiredHorizontalVel = movingDirection * 5.0f;

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

	Physics::Activate(LeadBody);

	mesh->Rotation = vec3(0,MathHelper::FindLookAtRotation(vec3(), movingDirection).y, 0);

}

void TestNpc::Serialize(json& target)
{

	Entity::Serialize(target);

	Rotation = MathHelper::ToYawPitchRoll(FromPhysics(LeadBody->GetRotation()));

	SERIALIZE_FIELD(target, Rotation)
	SERIALIZE_FIELD(target, desiredDirection)
	SERIALIZE_FIELD(target, movingDirection)

}

void TestNpc::Deserialize(json& source)
{

	Entity::Deserialize(source);

	DESERIALIZE_FIELD(source, Rotation)
	DESERIALIZE_FIELD(source, desiredDirection)
	DESERIALIZE_FIELD(source, movingDirection)

	Physics::SetBodyPosition(LeadBody, Position);

}

void TestNpc::LoadAssets()
{
	mesh->LoadFromFile("GameData/dog.glb");
	mesh->PlayAnimation("run");
	mesh->SetLooped(true);
	mesh->ColorTexture = AssetRegistry::GetTextureFromFile("GameData/cat.png");
}


