#include "ThrownProjectileBase.h"
#include "MolotovFire.h"

#include <Entities/SoundPlayer.h>
#include <Particle/ParticleSystem.hpp>
#include <World/WorldOrientationManager.h>

#include <cstdlib>

namespace
{
	float RandRange(float lo, float hi)
	{
		return lo + (hi - lo) * static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
	}

	vec3 RandomDirection()
	{
		vec3 v;
		float lengthSquared;
		do
		{
			v = vec3(RandRange(-1.0f, 1.0f), RandRange(-1.0f, 1.0f), RandRange(-1.0f, 1.0f));
			lengthSquared = glm::dot(v, v);
		} while (lengthSquared < 0.0001f || lengthSquared > 1.0f);
		return glm::normalize(v);
	}
}

// A lit molotov in flight. It breaks on the first thing it touches: the burning liquid
// sprays out of the impact as several MolotovFire blobs, which fall and run down walls.
class MolotovProjectile : public ThrownProjectileBase
{
public:

	static inline const std::string kModel = "GameData/models/player/weapons/molotov/molotov_thrown.glb";
	// Placeholder until the molotov has an event of its own.
	static inline const std::string kShatterSound = "event:/Weapons/knife/knife_hit";

	// Tip of the rag in the model's own space (thrown_info.json, flame_point).
	vec3 flameTip = vec3(0.0f, 0.198f, 0.0f);

	int blobCount = 14;

	MolotovProjectile()
	{
		ClassName = "molotovProjectile";

		modelPath = kModel;
		modelScale = 1.5f;
		radius = 0.06f * modelScale;
		rollRadius = 0.04f * modelScale;
		restMeshDrop = 0.02f * modelScale;
	}

	void LoadAssets() override
	{
		ThrownProjectileBase::LoadAssets();
		PreloadEntityType("molotovFire");
		PreloadEntityType("fire_small");
	}

	void Start() override
	{
		ThrownProjectileBase::Start();

		flame = (ParticleSystem*)Spawn("fire_small");
		flame->Position = LocalToWorld(flameTip);
		flame->Start();
	}

	void Destroy() override
	{
		if (flame != nullptr)
		{
			flame->StopAll();
			flame->DestroyWithDelay(1.0f);
			flame = nullptr;
		}

		ThrownProjectileBase::Destroy();
	}

protected:

	void UpdateEffects() override
	{
		if (flame != nullptr)
			flame->Position = LocalToWorld(flameTip);
	}

	bool OnImpact(const Physics::HitResult& hit, float speed) override
	{
		Entity* causer = DamageCauser();
		const vec3 normal = hit.normal;

		// A direct hit sets the target alight right away.
		if (hit.entity != nullptr && hit.entity->HasTag("canBurn"))
			MolotovFire::TryIgnite(hit.entity, causer);

		// The liquid sprays out of the impact, mostly along the surface and a little away from it.
		for (int i = 0; i < blobCount; ++i)
		{
			MolotovFire* fire = (MolotovFire*)Spawn("molotovFire");
			fire->damageCauser = causer;

			vec3 spread = RandomDirection();
			spread -= normal * glm::dot(spread, normal);
			if (glm::length(spread) > 0.0001f)
				spread = glm::normalize(spread);

			fire->velocity = spread * RandRange(3.0f, 7.5f) + normal * RandRange(1.5f, 3.0f) + velocity * 0.12f;
			fire->Position = Position + normal * 0.2f;
			fire->LoadAssetsIfNeeded();
			fire->Start();
		}

		SoundPlayer::PlayOneshot(kShatterSound, 2.0f, 1.0f, false, Position);

		Finish();
		return true;
	}

private:

	ParticleSystem* flame = nullptr;
};

REGISTER_ENTITY(MolotovProjectile, "molotovProjectile")
