#include "Door.h"

void Door::Start()
{
	Entity::Start();

	LeadBody = Physics::CreateBoxBody(this,Position, Scale * vec3(doorSize.x, doorSize.y, 0.2f), 10, false, BodyType::MainBody, BodyType::GroupCollisionTest | HitBox);

	Physics::SetMotionType(LeadBody, JPH::EMotionType::Kinematic);

}

void Door::Update()
{

	doorMesh->Rotation.y = Rotation.y + DoorRotation;
	doorMesh->Position = MathHelper::RotateAroundPoint(Position, Position - MathHelper::GetRightVector(Rotation) * doorSize.x/2.0f, vec3(0, DoorRotation,0));

	Physics::MoveKinematic(LeadBody,doorMesh->Position + vec3(0, doorSize.y/2,0), doorMesh->Rotation);

}

void Door::LoadAssets()
{

	doorMesh = new StaticMesh(this);
	doorMesh->LoadFromFile("GameData/models/enviroment/door.glb");
	doorMesh->TexturesLocation = "GameData/models/enviroment/door.glb/";
	doorMesh->MeshHideList.insert("frame");
	Drawables.push_back(doorMesh);

	doorFrameMesh = new StaticMesh(this);
	doorFrameMesh->LoadFromFile("GameData/models/enviroment/door.glb");
	doorFrameMesh->TexturesLocation = "GameData/models/enviroment/door.glb/";
	doorFrameMesh->MeshHideList.insert("door");
	Drawables.push_back(doorFrameMesh);

}

REGISTER_ENTITY(Door, "env_door")