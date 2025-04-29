#include "Bullet.h"

#include <Physics.h>

REGISTER_ENTITY(Bullet, "bullet")

Bullet::Bullet()
{

}

Bullet::~Bullet()
{
}

void Bullet::Update()
{

	float travelDistance = Speed * Time::DeltaTimeF;

	traveledDistance += travelDistance;

	Position += MathHelper::GetForwardVector(Rotation) * travelDistance;

	trail->Position = Position;

	auto hit = Physics::LineTrace(oldPos, Position, BodyType::GroupHitTest);

	if (hit.hasHit)
	{
		hit.entity->OnPointDamage(Damage, hit.position, MathHelper::FastNormalize(Position - oldPos), hit.hitboxName, this, this);

		Destroy();
		trail->Position = hit.position;
		trail->StopAll(1);
		trail = nullptr;

	}

	if (traveledDistance > MaxDistance)
	{
		Destroy();
		trail->Position = hit.position;
		trail->StopAll(1);
		trail = nullptr;
	}

	oldPos = Position;

}
