#include "MolotovFire.h"

#include <Entities/Enemy/EnemyFire.h>
#include <Network/NetworkedEntity.h>
#include <Physics.h>
#include <Helpers/JsonHelper.hpp>
#include <Helpers/JsonMacros.hpp>
#include <World/WorldOrientationManager.h>

#include <algorithm>
#include <vector>

namespace
{
	constexpr float kGravity = 12.0f;
	constexpr float kBlobRadius = 0.12f;       // collision sphere of the falling liquid
	constexpr float kFloorDot = 0.7f;          // a surface this flat is a floor
	constexpr float kWallSlideKeep = 0.3f;     // share of the speed kept along a wall
	constexpr float kTickSeconds = 0.25f;
	constexpr float kBurnSeconds = 4.5f;       // a little under EnemyFire's lifetime, so a fire is renewed only when it is about to end
}

MolotovFire::MolotovFire()
{
	SaveGame = true;
	ClassName = "molotovFire";
}

bool MolotovFire::TryIgnite(Entity* target, Entity* causer)
{
	if (target == nullptr)
		return false;

	if (burningUntil.size() > 64)
	{
		for (auto it = burningUntil.begin(); it != burningUntil.end();)
			it = it->second < Time::GameTime ? burningUntil.erase(it) : std::next(it);
	}

	double& until = burningUntil[target->Id];
	if (until > Time::GameTime)
		return false;

	until = Time::GameTime + kBurnSeconds;

	EnemyFire* enemyFire = new EnemyFire(target);
	enemyFire->Start();
	Level::Current->AddEntity(enemyFire);
	return true;
}

void MolotovFire::LoadAssets()
{
	PreloadEntityType("fire_medium");
	PreloadEntityType("enemyFire");
}

void MolotovFire::Start()
{
	Entity::Start();

	flames = (ParticleSystem*)Spawn("fire_medium");
	flames->Position = Position;
	flames->Start();
}

void MolotovFire::Update()
{
	if (finished)
		return;

	const float dt = Time::DeltaTimeF;
	age += dt;

	if (age >= lifeTime)
	{
		Extinguish();
		return;
	}

	if (!settled)
		Fall(dt);

	if (finished)
		return;

	if (flames != nullptr)
		flames->Position = Position;

	tickTimer += dt;
	if (tickTimer >= kTickSeconds)
	{
		tickTimer -= kTickSeconds;
		Burn();
	}
}

void MolotovFire::Fall(float dt)
{
	const vec3 up = WorldOrientationManager::GetUpVector();

	velocity -= up * kGravity * dt;
	const vec3 target = Position + velocity * dt;

	const Physics::HitResult hit = Physics::SphereTrace(Position, target, kBlobRadius, BodyType::WorldOpaque);

	if (!hit.hasHit)
	{
		Position = target;
		return;
	}

	if (glm::dot(hit.normal, up) > kFloorDot)
	{
		// Reached a floor: stay here and burn.
		Position = hit.shapePosition;
		velocity = vec3(0);
		settled = true;
		return;
	}

	// A wall or a ceiling: the liquid sticks to it and runs down along it.
	const vec3 along = velocity - hit.normal * glm::dot(velocity, hit.normal);
	velocity = along * kWallSlideKeep;
	Position = hit.shapePosition + hit.normal * 0.02f;
}

void MolotovFire::Burn()
{
	const vec3 up = WorldOrientationManager::GetUpVector();

	// The floor can disappear (a door opens, a prop is moved): the fire falls again.
	if (settled)
	{
		const Physics::HitResult ground = Physics::SphereTrace(Position + up * 0.05f, Position - up * 0.1f, kBlobRadius, BodyType::WorldOpaque);
		if (!ground.hasHit)
		{
			settled = false;
			return;
		}
	}

	std::vector<Entity*> burned;

	const std::vector<Physics::HitResult> hits = Physics::MultiSphereOverlap(Position + up * 0.3f, damageRadius, BodyType::GroupHitTest, {}, {});

	for (const Physics::HitResult& hit : hits)
	{
		Entity* entity = hit.entity;
		if (!hit.hasHit || entity == nullptr || entity == this)
			continue;

		if (std::find(burned.begin(), burned.end(), entity) != burned.end())
			continue;
		burned.push_back(entity);

		const NetworkedEntity* networked = dynamic_cast<NetworkedEntity*>(entity);
		if (networked != nullptr && !networked->isOwned)
			continue;

		if (entity->HasTag("player"))
			entity->OnDamage(playerDamagePerSecond * kTickSeconds, damageCauser, this);
		else if (entity->HasTag("canBurn"))
			TryIgnite(entity, damageCauser);
	}
}

void MolotovFire::Extinguish()
{
	finished = true;
	Destroy();
}

void MolotovFire::Destroy()
{
	finished = true;

	if (flames != nullptr)
	{
		flames->StopAll();
		flames->DestroyWithDelay(1.5f);
		flames = nullptr;
	}

	Entity::Destroy();
}

void MolotovFire::Serialize(json& target)
{
	SERIALIZE_FIELD(target, velocity);
	SERIALIZE_FIELD(target, age);
	SERIALIZE_FIELD(target, settled);
	SERIALIZE_FIELD(target, lifeTime);
}

void MolotovFire::Deserialize(json& source)
{
	DESERIALIZE_FIELD(source, velocity);
	DESERIALIZE_FIELD(source, age);
	DESERIALIZE_FIELD(source, settled);
	DESERIALIZE_FIELD(source, lifeTime);
}

REGISTER_ENTITY(MolotovFire, "molotovFire")
