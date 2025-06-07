#include <Entity.h>

#include <Input.h>

#include <MathHelper.hpp>

#include <Camera.h>

#include <Physics.h>

#include <DebugDraw.hpp>

#include <algorithm>   // for std::clamp
#include <cmath>       // for std::max

#include <Navigation/Navigation.hpp>

#include <SkeletalMesh.hpp>

#include <imgui/imgui.h>

#include <Particle/ParticleSystem.hpp>

#include <SaveSystem/LevelSaveSystem.h>

#include <Entities/SoundPlayer.h>
#include <SoundSystem/SoundManager.hpp>

#include "../../UI/Player/PlayerHud.hpp"

#include "Weapons/WeaponBase.h"

class Player : public Entity
{

private:

	float maxSpeed = 7;
	float maxSpeedAir = 2;
	float acceleration = 90;
	float airAcceleration = 30;

	vec3 velocity = vec3(0);

	vec3 cameraRotation = vec3(0);

	vec3 weaponOffset = vec3(0.023, 0.013, -0.13);

	Delay jumpDelay;

	bool freeFly = false;

	PlayerHud Hud;

	SoundPlayer* soundPlayer;

	Weapon* currentWeapon = nullptr;

	std::vector<WeaponSlotData> weaponSlots;

	SkeletalMesh* bikeMesh = nullptr;
	SkeletalMesh* bikeArmsMesh = nullptr;

	bool on_bike = false;

	float cameraHeightOffset = 0;

	Delay stepDelay = Delay();

	float bike_progress = 0;

	vec3 stepForceWalkDirection = vec3();

	Delay afterStepDelay;

	Delay coyoteTime;

	glm::vec3 Friction(glm::vec3 vel, float factor = 60.0f) {
		vel = MathHelper::XZ(vel);
		float length = glm::length(vel);

		// Avoid division by zero: if length is positive, normalize; otherwise return zero vector.
		glm::vec3 direction = (length > 0.0f) ? glm::normalize(vel) : glm::vec3(0.0f);

		length -= factor * Time::DeltaTimeF;
		length = std::max(0.0f, length);

		return direction * length;
	}

	glm::vec3 UpdateGroundVelocity(glm::vec3 withDir, glm::vec3 vel) {
		vel = MathHelper::XZ(vel);
		vel = Friction(vel);

		// Project current velocity onto the direction
		float currentSpeed = glm::dot(vel, withDir);

		// Clamp the additional speed so that it does not exceed what can be accelerated in the frame.
		float addSpeed = glm::clamp(maxSpeed - currentSpeed, 0.0f, acceleration * Time::DeltaTimeF);

		if (false) {
			if (currentSpeed + addSpeed > maxSpeed)
				addSpeed = maxSpeed - currentSpeed;
		}

		return vel + addSpeed * withDir;
	}

	glm::vec3 UpdateAirVelocity(glm::vec3 wishdir, glm::vec3 vel) {
		vel = MathHelper::XZ(vel);

		float currentSpeed = glm::dot(vel, wishdir);
		float wishspeed = maxSpeedAir;
		float addSpeed = wishspeed - currentSpeed;

		if (addSpeed <= 0.0f) {
			return vel;
		}

		float accelspeed = airAcceleration * Time::DeltaTimeF * wishspeed;

		if (accelspeed > addSpeed) {
			accelspeed = addSpeed;
		}

		return vel + accelspeed * wishdir;
	}

	void Jump()
	{
		LeadBody->SetLinearVelocity(JPH::Vec3(velocity.x, 9.5, velocity.z));
		jumpDelay.AddDelay(0.3);
	}

	bool CheckGroundAt(vec3 location)
	{

		if (jumpDelay.Wait())
			return false;

		if (afterStepDelay.Wait()) return true;

		auto result = Physics::LineTrace(location, location - vec3(0, 0.92, 0), BodyType::GroupCollisionTest, { LeadBody });

		return result.hasHit;

	}

	vec3 testStart;



public:
	Player()
	{

		bikeMesh = new SkeletalMesh(this);
		Drawables.push_back(bikeMesh);
		bikeArmsMesh = new SkeletalMesh(this);
		Drawables.push_back(bikeArmsMesh);

		ClassName = "info_player_start";

		SaveGame = true;

		Tags = { "player" };

		Health = 100;

		LateUpdateWhenPaused = true;

	}
	~Player() { Logger::Log("player destructor"); }

	float Speed = 5;

	static Player* Instance;

	void FromData(EntityData data)
	{
		Entity::FromData(data);
		cameraRotation.y = data.GetPropertyFloat("angle") + 90;
	}

	void Start()
	{

		Instance = this;

		LeadBody = Physics::CreateCharacterBody(this, Position, 0.4, 1.8, 90);
		Physics::SetGravityFactor(LeadBody, 3);
		LeadBody->SetFriction(0.1f);


		ParticleSystem::PreloadSystemAssets("decal_blood");
		ParticleSystem::PreloadSystemAssets("hit_flesh");

		soundPlayer = new SoundPlayer();
		Level::Current->AddEntity(soundPlayer);
		soundPlayer->Sound = SoundManager::GetSoundFromPath("GameData/sounds/mew.wav");

		Hud.Init(this);

		CreateWeapon("weapon_pistol");

	}

	void UpdateWalkMovement(vec2 input);
	void UpdateBikeMovement(vec2 input);

	void CreateWeapon(const string& className);
	void DestroyWeapon();

	void Destroy()
	{
		Entity::Destroy();

		Instance = nullptr;

	}

	dtObstacleRef playerObstacle = 0;

	void UpdateWeapon();

	void UpdateDebugUI();

	bool OnGround();

	void PerformAttack();

	void TryStep(vec3 dir);

	void Update();
	void LateUpdate();

	void Serialize(json& target);
	void OnDamage(float Damage, Entity* DamageCauser = nullptr, Entity* Weapon = nullptr);
	void OnPointDamage(float Damage, vec3 Point, vec3 Direction, string bone, Entity* DamageCauser, Entity* Weapon);

	void Deserialize(json& source);

	void StartBike();
	void StopBike();
	void ToggleBike();

protected:

	void LoadAssets();

};