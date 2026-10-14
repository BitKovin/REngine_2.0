#include "Projectile.h"

#include <Characters/CombatCharacter.h>
#include <Combat/CombatComponents.h>
#include <Combat/CombatEvents.h>

#include <Entities/SoundPlayer.h>
#include <Logger.hpp>
#include <Particle/GlobalParticleSystem.hpp>
#include <Particle/ParticleSystem.hpp>
#include <Physics.h>
#include <Time.hpp>

#include <algorithm>
#include <vector>

namespace
{
	const BodyType ProjectileHitMask = static_cast<BodyType>(BodyType::World | BodyType::HitBox | BodyType::MainBody);
}

Projectile* Projectile::Spawn(std::shared_ptr<const WeaponData> weapon, CombatCharacter& shooter, const glm::vec3& origin, const glm::vec3& direction)
{
	if (!weapon)
		return nullptr;

	auto* projectile = new Projectile();
	projectile->weapon = std::move(weapon);
	projectile->shooter = EntityHandle::FromEntity(&shooter);
	projectile->shooterTeam = shooter.GetTeamId();
	projectile->Position = origin;
	projectile->velocity = glm::normalize(direction) * projectile->Data().speed;
	projectile->ClassName = "projectile";

	Level::Current->AddEntity(projectile);
	projectile->Start();
	return projectile;
}

void Projectile::Start()
{
	Entity::Start();

	const std::string& tracerName = Data().tracer;
	if (tracerName.empty())
		return;

	// The tracer type was preloaded by the owning WeaponSet in LoadAssets.
	Entity* entity = LevelObjectFactory::instance().create(tracerName);
	tracer = dynamic_cast<ParticleSystem*>(entity);
	if (tracer == nullptr)
	{
		delete entity;
		Logger::Warning("[Projectile] tracer '%s' is not a particle system", tracerName.c_str());
		return;
	}

	tracer->Position = Position;
	tracer->Start();
	Level::Current->AddEntity(tracer);
	tracer->LoadAssetsIfNeeded();
}

void Projectile::Sweep(float deltaTime)
{
	const ProjectileData& data = Data();

	velocity.y -= data.gravity * deltaTime;
	const glm::vec3 previous = Position;
	const glm::vec3 next = Position + velocity * deltaTime;
	nextPosition = next;
	impact = PendingImpact();

	Entity* shooterEntity = ResolveAlive(shooter);
	std::vector<Entity*> ignore;
	if (shooterEntity) ignore.push_back(shooterEntity);

	std::vector<Physics::HitResult> hits = Physics::MultiSphereTrace(previous, next, data.radius, ProjectileHitMask, {}, ignore);
	std::sort(hits.begin(), hits.end(), [](const Physics::HitResult& a, const Physics::HitResult& b) { return a.fraction < b.fraction; });

	for (const Physics::HitResult& hit : hits)
	{
		if (!hit.hasHit)
			continue;

		Entity* entity = hit.entity;
		if (entity != nullptr)
		{
			if (entity->Destroyed)
				continue;

			if (auto state = entity->GetComponent<CombatStateComponent>(); state && state->dead)
				continue;   // corpses don't stop bullets

			if (auto team = entity->GetComponent<TeamComponent>(); team && GetAttitude(shooterTeam, team->team) == Attitude::Friendly)
				continue;   // pass through allies
		}

		impact.hit = true;
		impact.entity = entity;
		impact.point = hit.position;
		impact.normal = hit.normal;
		impact.hitbox = hit.hitboxName;
		return;
	}
}

void Projectile::Resolve()
{
	if (impact.hit)
	{
		// The entity was alive during the sweep; an earlier impact this frame may have removed it since.
		Entity* entity = impact.entity != nullptr && impact.entity->Destroyed ? nullptr : impact.entity;
		if (impact.entity == nullptr || entity != nullptr)
		{
			Impact(entity, impact.point, impact.normal, impact.hitbox);
			return;
		}
	}

	travelled += glm::length(nextPosition - Position);
	Position = nextPosition;
	if (tracer)
		tracer->Position = Position;

	if (travelled >= Data().maxDistance)
		Destroy();
}

void Projectile::Impact(Entity* entity, const glm::vec3& point, const glm::vec3& normal, const std::string& hitbox)
{
	const ProjectileData& data = Data();
	const glm::vec3 direction = glm::normalize(velocity);

	if (entity != nullptr)
	{
		HitRequestEvent request;
		request.attacker = shooter;
		request.victim = EntityHandle::FromEntity(entity);
		request.source = DamageSource::Firearm;
		request.damage = data.damage;
		request.poiseDamage = data.poiseDamage;
		request.weakPointDamage = data.weakPointDamage < 0.0f ? data.damage : data.weakPointDamage;
		request.reaction = data.reaction;
		request.knockback = data.knockback;
		request.hitStop = data.hitStop;
		request.hitStopScale = data.hitStopScale;
		request.point = point;
		request.normal = normal;
		request.direction = direction;
		request.spray = -direction;
		request.hitbox = hitbox;
		request.hitSound = data.impactSound;
		request.hitParticle = data.impactParticle;
		EcsScheduler::Emit(request);
	}
	else
	{
		// World impact: feedback only.
		if (!data.impactSound.empty())
			SoundPlayer::PlayOneshot(data.impactSound, 2.0f, 1.0f, false, point);
		if (!data.impactParticle.empty())
			GlobalParticleSystem::SpawnParticleAt(data.impactParticle, point, MathHelper::FindLookAtRotation(point, point + normal), glm::vec3(1.0f));
	}

	Position = point;
	if (tracer)
		tracer->Position = point;
	Destroy();
}

void Projectile::Destroy()
{
	if (Destroyed)
		return;

	if (tracer)
	{
		tracer->StopAll();
		tracer->DestroyWithDelay(0.5f);
		tracer = nullptr;
	}

	Entity::Destroy();
}

// Every projectile, every frame (COMBAT_DESIGN.md §7): sweeps in parallel, then impacts / movement / removal on the
// main thread in a stable order. Runs after the bodies so shots fired this frame fly at once.
DECLARE_SYSTEM(ProjectileUpdateSystem, PostUpdate)
{
	static std::vector<Projectile*> projectiles;   // main thread only; keeps its capacity between frames
	projectiles.clear();

	ctx.ForEach<ProjectileComponent>([](entt::entity, ProjectileComponent& component)
	{
		if (component.projectile != nullptr && !component.projectile->Destroyed)
			projectiles.push_back(component.projectile);
	});

	std::sort(projectiles.begin(), projectiles.end(), [](Projectile* a, Projectile* b)
	{
		return entt::to_integral(a->Handle()) < entt::to_integral(b->Handle());
	});

	const float deltaTime = ctx.DeltaTime;
	ctx.ParallelFor(projectiles.size(), [deltaTime](size_t i) { projectiles[i]->Sweep(deltaTime); });

	for (Projectile* projectile : projectiles)
		if (!projectile->Destroyed)
			projectile->Resolve();
}
REGISTER_SYSTEM(ProjectileUpdateSystem, .After({ "CombatBodyUpdateSystem" }).ParallelSafe(true))
