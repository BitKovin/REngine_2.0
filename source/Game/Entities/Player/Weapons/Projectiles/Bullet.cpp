#include "Bullet.h"

#include <Physics.h>

#include <Particle/GlobalParticleSystem.hpp>

#include <Entities/PointLight.h>

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

	vec3 forward = MathHelper::GetForwardVector(Rotation);

	Position += MathHelper::GetForwardVector(Rotation) * travelDistance;

	trail->Position = Position;

	auto hit = Physics::LineTrace(oldPos, Position, BodyType::GroupHitTest);

	if (hit.hasHit)
	{
		hit.entity->OnPointDamage(Damage, hit.position, MathHelper::FastNormalize(Position - oldPos), hit.hitboxName, this, this);

		Physics::AddImpulseAtLocation(hit.hitbody, forward * (Damage+2) * 15.0f, hit.position);

		//Logger::Log(hit.surfaceName);

		//GlobalParticleSystem::SpawnParticleAt("hit_flesh", hit.position, MathHelper::FindLookAtRotation(vec3(0), MathHelper::FastNormalize(Position - oldPos)), vec3(2.0f));

		PointLight* pointLight = new PointLight();
		pointLight->Position = hit.position + hit.normal*0.5f;
		Level::Current->AddEntity(pointLight);

		Destroy();
		trail->Position = hit.position;
		trail->StopAll();
		trail = nullptr;
		return;
	}

	if (traveledDistance > MaxDistance)
	{
		Destroy();
		trail->Position = hit.position;
		trail->StopAll();
		trail = nullptr;
	}

	oldPos = Position;

}
