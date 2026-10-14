#pragma once

#include <Combat/CombatTypes.h>
#include <Data/WeaponData.h>

#include <Ecs/Ecs.h>
#include <Entity.h>
#include <EntityHandle.h>
#include <glm.h>

#include <memory>
#include <string>

class CombatCharacter;
class ParticleSystem;
class Projectile;

// Marks a flying projectile. ProjectileUpdateSystem sweeps every projectile in parallel and resolves impacts on the
// main thread in a stable order (COMBAT_DESIGN.md §7).
struct ProjectileComponent
{
	Projectile* projectile = nullptr;
};
REGISTER_COMPONENT(ProjectileComponent)

// A firearm projectile (COMBAT_DESIGN.md §7). Engine entity spawned at runtime (main thread): each frame it
// sphere-sweeps from its previous to its new position, passes through friendlies and corpses, and emits a
// HitRequestEvent on the first hostile or world hit. The tracer is the weapon's particle system following it.
class Projectile : public Entity
{
	ECS_COMPONENT(ProjectileComponent, ProjectileState, this);

public:
	static Projectile* Spawn(std::shared_ptr<const WeaponData> weapon, CombatCharacter& shooter, const glm::vec3& origin, const glm::vec3& direction);

	void Start() override;
	void Destroy() override;

	// ProjectileUpdateSystem: Sweep on a worker thread (reads the world, writes only this projectile), then Resolve
	// on the main thread (impact, movement, removal).
	void Sweep(float deltaTime);
	void Resolve();

private:
	struct PendingImpact
	{
		bool hit = false;
		Entity* entity = nullptr;
		glm::vec3 point = glm::vec3(0.0f);
		glm::vec3 normal = glm::vec3(0.0f, 1.0f, 0.0f);
		std::string hitbox;
	};

	std::shared_ptr<const WeaponData> weapon;     // keeps the projectile data alive (dev reload safe)
	EntityHandle shooter;
	Team shooterTeam = Team::Neutral;
	glm::vec3 velocity = glm::vec3(0.0f);
	glm::vec3 nextPosition = glm::vec3(0.0f);
	float travelled = 0.0f;
	PendingImpact impact;
	ParticleSystem* tracer = nullptr;

	const ProjectileData& Data() const { return weapon->firearm.projectile; }
	void Impact(Entity* entity, const glm::vec3& point, const glm::vec3& normal, const std::string& hitbox);
};
