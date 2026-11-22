#pragma once

#include <Entity.h>
#include <StaticMesh.h>
#include <Physics.h>
#include <MathHelper.hpp>
#include <glm/gtc/quaternion.hpp>

// Base of the objects the player throws (dynamite, molotov).
//
// The object is a sphere on a ballistic arc. Every frame the move is swept against the world
// (Physics::SphereTrace), so it cannot pass through walls. On a hit it either bounces
// (restitution / friction) or the subclass decides what happens (OnImpact). Its model tumbles
// the way it moves: end over end in the air, rolling after a bounce, and it lies down on its
// side once it comes to rest.
//
// A fuse is optional: fuseRemaining >= 0 counts down and calls OnFuseExpired().
class ThrownProjectileBase : public Entity
{
public:

	Entity* owner = nullptr;          // thrower, also the damage causer. Player::Instance when left empty
	vec3 velocity = vec3(0);
	float fuseRemaining = -1.0f;      // seconds, < 0 = no fuse

	// Tuning, set in the constructor of the subclass.
	std::string modelPath;            // glb with its textures inside
	float modelScale = 1.0f;          // size of the model; points in the model's own space (LocalToWorld) scale with it
	float radius = 0.06f;             // collision sphere
	float gravity = 9.81f;
	float restitution = 0.35f;        // share of the speed into the surface that comes back
	float friction = 0.35f;           // share of the speed along the surface lost per bounce
	float restSpeed = 0.6f;           // below this speed on a floor the object stops
	float rollRadius = 0.045f;        // radius of the roll after a bounce
	float restMeshDrop = 0.02f;       // the model is lowered by this much while lying (the sphere is thicker than it)
	float maxLifetime = 60.0f;        // safety: never lives forever

	ThrownProjectileBase();

	// Sets the start state. Call before Start().
	void Throw(const vec3& startPosition, const vec3& startVelocity, float fuseSeconds, Entity* thrower);

	void LoadAssets() override;
	void Start() override;
	void Update() override;
	void Destroy() override;

	void Serialize(json& target) override;
	void Deserialize(json& source) override;

protected:

	StaticMesh* mesh = nullptr;
	glm::quat orientation = glm::quat(1, 0, 0, 0);

	// World position of a point given in the model's own space.
	vec3 LocalToWorld(const vec3& local) const { return Position + orientation * (local * modelScale); }

	Entity* DamageCauser() const;

	// The object hit something. speed = how fast it was moving into the surface.
	// Return true when it handled the hit itself (usually by destroying the object);
	// false lets it bounce.
	virtual bool OnImpact(const Physics::HitResult& hit, float speed) { return false; }

	virtual void OnFuseExpired() { Finish(); }

	// Called every frame after the transform is updated (attach particles here).
	virtual void UpdateEffects() {}

	// Destroys the object and stops further processing this frame.
	void Finish();

	bool IsResting() const { return resting; }

private:

	vec3 angularVelocity = vec3(0);   // rad/s, world axes
	float age = 0.0f;
	bool resting = false;
	bool finished = false;

	void UpdateFlight(float dt);
	void UpdateResting(float dt);
	void Bounce(const Physics::HitResult& hit);
	void SpinInAir();
	void SpinRolling(const vec3& surfaceNormal, const vec3& tangentVelocity);
	void Integrate(float dt);
	void UpdateVisuals();
};
