#include "CaneProjectile.h"

#include "../../Player.hpp"

REGISTER_ENTITY(CaneProjectile, "caneProjectile")



void CaneProjectile::Update()
{

	float travelDistance = Speed * Time::DeltaTimeF;

	traveledDistance += travelDistance;

	vec3 forward = MathHelper::GetForwardVector(Rotation);


	trail->Position = Position;
	mesh->Position = Position;
	mesh->Rotation = Rotation;

	if (hited) return;

	Position += MathHelper::GetForwardVector(Rotation) * travelDistance;

	auto hit = Physics::LineTrace(oldPos, Position, BodyType::GroupHitTest);

	if (hit.hasHit)
	{
		hit.entity->OnPointDamage(Damage, hit.position, MathHelper::FastNormalize(Position - oldPos), hit.hitboxName, this, this);

		Physics::AddImpulseAtLocation(hit.hitbody, forward * (Damage + 2) * 15.0f, hit.position);

		trail->Position = hit.position;
		mesh->Position = hit.position;
		Position = hit.position;

		hited = true;

		Speed = 0;

		return;
	}

	if (traveledDistance > MaxDistance)
	{
		trail->Position = hit.position;
		trail->StopAll();
		trail = nullptr;

		Destroy();

	}

	oldPos = Position;

}