#pragma once

#include <Entity.h>
#include <Particle/ParticleSystem.hpp>

#include <map>
#include <string>

// A blob of burning liquid from a broken molotov. It has physics of its own: it is thrown out
// of the impact, falls, and when it hits a wall it slides down it until it reaches the floor,
// where it burns for a few seconds. Whoever stands in it burns: the player takes damage over
// time, anything tagged "canBurn" is set alight (EnemyFire).
class MolotovFire : public Entity
{
public:

	Entity* damageCauser = nullptr;
	vec3 velocity = vec3(0);

	float lifeTime = 7.0f;               // burns this long once it exists
	float damageRadius = 1.7f;
	float playerDamagePerSecond = 12.0f;

	MolotovFire();

	// Sets a burnable target alight, unless it is already burning from an earlier call.
	// Returns true when a new fire was started.
	static bool TryIgnite(Entity* target, Entity* causer);

	void LoadAssets() override;
	void Start() override;
	void Update() override;
	void Destroy() override;

	void Serialize(json& target) override;
	void Deserialize(json& source) override;

private:

	ParticleSystem* flames = nullptr;
	bool settled = false;                // lying on a floor
	bool finished = false;
	float age = 0.0f;
	float tickTimer = 0.0f;

	// Entity id -> game time until which it is considered burning.
	static inline std::map<std::string, double> burningUntil;

	void Fall(float dt);
	void Burn();
	void Extinguish();
};
