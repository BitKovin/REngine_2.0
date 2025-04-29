#include "Bullet.h"

#include <Physics.h>

REGISTER_ENTITY(Bullet, "bullet")

Bullet::Bullet()
{
	mesh = new StaticMesh();
	Drawables.push_back(mesh);
}

Bullet::~Bullet()
{
}

void Bullet::Update()
{

	mesh->Position = Position;
	mesh->Rotation = Rotation;

	float travelDistance = Speed * Time::DeltaTimeF;

	traveledDistance += travelDistance;

	Position += MathHelper::GetForwardVector(Rotation) * travelDistance;

	trail->Position = Position;

	auto hit = Physics::LineTrace(oldPos, Position, BodyType::GroupHitTest);

	if (hit.hasHit)
	{
		hit.entity->OnPointDamage(Damage, hit.position, MathHelper::FastNormalize(Position - oldPos), hit.hitboxName, this, this);

		Destroy();
		trail->StopAll();
		trail = nullptr;

	}



	oldPos = Position;

}
