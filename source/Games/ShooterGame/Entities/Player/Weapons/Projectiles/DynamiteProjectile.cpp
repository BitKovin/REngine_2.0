#include "ThrownProjectileBase.h"

#include <Entities/Explosion.h>
#include <Entities/SoundPlayer.h>
#include <Particle/ParticleSystem.hpp>
#include <World/WorldOrientationManager.h>

// A lit stick of dynamite in flight. It bounces, rolls and lies down like any thrown object and
// blows up when its fuse runs out, wherever it is: in the air, on the floor or next to an enemy.
class DynamiteProjectile : public ThrownProjectileBase
{
public:

	static inline const std::string kModel = "GameData/models/player/weapons/dynamite/dynamite_thrown.glb";
	static inline const std::string kHitSound = "event:/Weapons/dynamite/hit";
	static constexpr float kHitSoundMinSpeed = 1.0f;   // slower touches make no sound

	// Tip of the fuse in the model's own space (thrown_info.json, spark_point).
	vec3 fuseTip = vec3(0.0f, 0.153f, 0.0f);

	// It goes off the moment it touches anything with this tag, instead of waiting for the fuse.
	std::string detonateOnTag = "enemy";

	float explosionRadius = 4.0f;
	float minDamage = 60.0f;
	float maxDamage = 200.0f;
	float minPlayerDamage = 30.0f;
	float maxPlayerDamage = 70.0f;

	DynamiteProjectile()
	{
		ClassName = "dynamiteProjectile";

		modelPath = kModel;
		modelScale = 2.25f;
		radius = 0.07f * modelScale;
		restitution = 0.3f;
		friction = 0.4f;
		rollRadius = 0.04f * modelScale;
		restMeshDrop = 0.02f * modelScale;
	}

	void LoadAssets() override
	{
		ThrownProjectileBase::LoadAssets();
		PreloadEntityType("explosion");
		PreloadEntityType("fire_tiny");
	}

	void Start() override
	{
		ThrownProjectileBase::Start();

		sparks = (ParticleSystem*)Spawn("fire_tiny");
		sparks->Position = LocalToWorld(fuseTip);
		sparks->Start();
	}

	void Destroy() override
	{
		if (sparks != nullptr)
		{
			sparks->StopAll();
			sparks->DestroyWithDelay(1.0f);
			sparks = nullptr;
		}

		ThrownProjectileBase::Destroy();
	}

protected:

	bool OnImpact(const Physics::HitResult& hit, float speed) override
	{
		if (hit.entity == nullptr || !hit.entity->HasTag(detonateOnTag))
		{
			// It bounces off: the thud.
			if (speed > kHitSoundMinSpeed)
				SoundPlayer::PlayOneshot(kHitSound, 2.0f, 1.0f, false, Position);
			return false;
		}

		OnFuseExpired();
		return true;
	}

	void UpdateEffects() override
	{
		if (sparks != nullptr)
			sparks->Position = LocalToWorld(fuseTip);
	}

	void OnFuseExpired() override
	{
		Explosion* explosion = new Explosion();
		explosion->Position = Position + WorldOrientationManager::GetUpVector() * 0.25f;
		explosion->Radius = explosionRadius;
		explosion->minDamage = minDamage;
		explosion->MaxDamage = maxDamage;
		explosion->minPlayerDamage = minPlayerDamage;
		explosion->MaxPlayerDamage = maxPlayerDamage;
		explosion->Lifetime = 2.9f;
		explosion->LoadAssetsIfNeeded();
		explosion->damageCauser = DamageCauser();

		Level::Current->AddEntity(explosion);
		explosion->Start();

		Finish();
	}

private:

	ParticleSystem* sparks = nullptr;
};

REGISTER_ENTITY(DynamiteProjectile, "dynamiteProjectile")
