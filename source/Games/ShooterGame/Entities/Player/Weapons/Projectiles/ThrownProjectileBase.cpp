#include "ThrownProjectileBase.h"

#include "../../Player.hpp"

#include <Helpers/JsonHelper.hpp>
#include <Helpers/JsonMacros.hpp>
#include <World/WorldOrientationManager.h>

#include <algorithm>
#include <cmath>
#include <vector>

namespace
{
	constexpr float kSkin = 0.002f;           // a hair off the surface after a bounce
	constexpr float kFloorDot = 0.6f;         // a surface this flat counts as a floor
	constexpr float kOwnerGrace = 0.3f;       // the thrower is not hit by his own throw for this long
	constexpr float kMaxSpin = 16.0f;         // rad/s
	constexpr float kLayDownRate = 6.0f;      // rad/s, how fast a resting object rolls onto its side
}

ThrownProjectileBase::ThrownProjectileBase()
{
	SaveGame = true;
}

void ThrownProjectileBase::Throw(const vec3& startPosition, const vec3& startVelocity, float fuseSeconds, Entity* thrower)
{
	Position = startPosition;
	velocity = startVelocity;
	fuseRemaining = fuseSeconds;
	owner = thrower;

	// Start upright, facing the way it flies.
	const float yaw = glm::length(MathHelper::XZ(velocity)) > 0.01f ? MathHelper::FindLookAtRotation(vec3(0), velocity).y : 0.0f;
	orientation = MathHelper::GetRotationQuaternion(vec3(0, yaw, 0));

	SpinInAir();
}

void ThrownProjectileBase::LoadAssets()
{
	mesh = new StaticMesh(this);
	mesh->LoadFromFile(modelPath);
	mesh->TexturesLocation = modelPath + "/";
	mesh->Scale = vec3(modelScale);
	Drawables.push_back(mesh);
}

void ThrownProjectileBase::Start()
{
	Entity::Start();

	if (owner == nullptr)
		owner = Player::Instance;

	UpdateVisuals();
}

void ThrownProjectileBase::Destroy()
{
	finished = true;

	if (burnSound != nullptr)
	{
		burnSound->Stop();
		burnSound->Destroy();
		burnSound = nullptr;
	}

	Entity::Destroy();
}

Entity* ThrownProjectileBase::DamageCauser() const
{
	return owner != nullptr ? owner : Player::Instance;
}

void ThrownProjectileBase::Finish()
{
	finished = true;
	Destroy();
}

void ThrownProjectileBase::Update()
{
	if (finished)
		return;

	const float dt = Time::DeltaTimeF;
	age += dt;

	if (age > maxLifetime)
	{
		Finish();
		return;
	}

	if (fuseRemaining >= 0.0f)
	{
		fuseRemaining -= dt;
		if (fuseRemaining <= 0.0f)
		{
			fuseRemaining = -1.0f;
			OnFuseExpired();
			if (finished)
				return;
		}
	}

	if (resting)
		UpdateResting(dt);
	else
		UpdateFlight(dt);

	if (finished)
		return;

	Integrate(dt);
	UpdateVisuals();

	if (burnSound != nullptr)
		burnSound->Position = Position;
}

void ThrownProjectileBase::UpdateFlight(float dt)
{
	const vec3 up = WorldOrientationManager::GetUpVector();

	velocity -= up * gravity * dt;
	const vec3 target = Position + velocity * dt;

	std::vector<Entity*> ignored;
	if (owner != nullptr && age < kOwnerGrace)
		ignored.push_back(owner);

	const Physics::HitResult hit = Physics::SphereTrace(Position, target, radius,
		BodyType::GroupHitTest | BodyType::WorldSkybox, {}, ignored);

	if (!hit.hasHit)
	{
		Position = target;
		return;
	}

	const auto* bodyData = hit.hitbody != nullptr ? Physics::GetBodyData(hit.hitbody) : nullptr;
	if (bodyData != nullptr && bodyData->group == BodyType::WorldSkybox)
	{
		Finish();
		return;
	}

	Position = hit.shapePosition;

	const float speedIntoSurface = std::max(0.0f, -glm::dot(velocity, hit.normal));
	if (OnImpact(hit, speedIntoSurface) || finished)
		return;

	Bounce(hit);
}

void ThrownProjectileBase::Bounce(const Physics::HitResult& hit)
{
	const vec3 up = WorldOrientationManager::GetUpVector();
	const vec3 normal = hit.normal;

	const float into = glm::dot(velocity, normal);                // negative: moving into the surface
	const vec3 normalPart = normal * std::min(into, 0.0f);
	const vec3 tangentPart = velocity - normal * into;

	velocity = tangentPart * (1.0f - friction) - normalPart * restitution;
	Position = hit.shapePosition + normal * kSkin;

	const bool floor = glm::dot(normal, up) > kFloorDot;
	if (floor && glm::length(velocity) < restSpeed)
	{
		resting = true;
		velocity = vec3(0);
		angularVelocity = vec3(0);
		return;
	}

	if (floor)
		SpinRolling(normal, velocity - normal * glm::dot(velocity, normal));
	else
		SpinInAir();
}

void ThrownProjectileBase::UpdateResting(float dt)
{
	const vec3 up = WorldOrientationManager::GetUpVector();

	// Still something underneath? If not (the floor was a moving prop, a door...) it falls again.
	const Physics::HitResult ground = Physics::SphereTrace(Position + up * 0.02f, Position - up * 0.06f, radius, BodyType::GroupHitTest);
	if (!ground.hasHit)
	{
		resting = false;
		SpinInAir();
		return;
	}

	// Roll onto the side: the model's long axis (its Y) becomes horizontal.
	const vec3 axis = orientation * vec3(0, 1, 0);
	vec3 flat = axis - up * glm::dot(axis, up);
	if (glm::length(flat) < 0.01f)
		flat = orientation * vec3(1, 0, 0);
	flat = glm::normalize(flat - up * glm::dot(flat, up));

	const vec3 turnAxis = glm::cross(axis, flat);
	const float turnLength = glm::length(turnAxis);
	if (turnLength > 0.0001f)
	{
		const float angle = std::acos(std::clamp(glm::dot(axis, flat), -1.0f, 1.0f));
		const float step = std::min(angle, kLayDownRate * dt);
		orientation = glm::normalize(glm::angleAxis(step, turnAxis / turnLength) * orientation);
	}
}

void ThrownProjectileBase::SpinInAir()
{
	const vec3 up = WorldOrientationManager::GetUpVector();
	const vec3 horizontal = velocity - up * glm::dot(velocity, up);
	const float horizontalLength = glm::length(horizontal);

	if (horizontalLength < 0.05f)
	{
		angularVelocity = vec3(0);
		return;
	}

	// End over end, top first in the direction of travel.
	const vec3 axis = glm::normalize(glm::cross(up, horizontal / horizontalLength));
	const float rate = std::clamp(glm::length(velocity) * 0.8f, 3.0f, kMaxSpin);
	angularVelocity = axis * rate;
}

void ThrownProjectileBase::SpinRolling(const vec3& surfaceNormal, const vec3& tangentVelocity)
{
	const float speed = glm::length(tangentVelocity);
	if (speed < 0.05f)
	{
		angularVelocity = vec3(0);
		return;
	}

	const vec3 axis = glm::normalize(glm::cross(surfaceNormal, tangentVelocity / speed));
	angularVelocity = axis * std::min(speed / std::max(rollRadius, 0.01f), kMaxSpin);
}

void ThrownProjectileBase::Integrate(float dt)
{
	const float spin = glm::length(angularVelocity);
	if (spin < 0.0001f)
		return;

	orientation = glm::normalize(glm::angleAxis(spin * dt, angularVelocity / spin) * orientation);
}

void ThrownProjectileBase::UpdateVisuals()
{
	const vec3 up = WorldOrientationManager::GetUpVector();

	Rotation = MathHelper::ToYawPitchRoll(orientation);

	if (mesh != nullptr)
	{
		mesh->Position = Position - (resting ? up * restMeshDrop : vec3(0));
		mesh->Rotation = Rotation;
	}

	UpdateEffects();
}

void ThrownProjectileBase::Serialize(json& target)
{
	const vec3 orientationEuler = MathHelper::ToYawPitchRoll(orientation);

	SERIALIZE_FIELD(target, velocity);
	SERIALIZE_FIELD(target, angularVelocity);
	SERIALIZE_FIELD(target, fuseRemaining);
	SERIALIZE_FIELD(target, age);
	SERIALIZE_FIELD(target, resting);
	SERIALIZE_FIELD(target, orientationEuler);
}

void ThrownProjectileBase::Deserialize(json& source)
{
	vec3 orientationEuler = vec3(0);

	DESERIALIZE_FIELD(source, velocity);
	DESERIALIZE_FIELD(source, angularVelocity);
	DESERIALIZE_FIELD(source, fuseRemaining);
	DESERIALIZE_FIELD(source, age);
	DESERIALIZE_FIELD(source, resting);
	DESERIALIZE_FIELD(source, orientationEuler);

	orientation = MathHelper::GetRotationQuaternion(orientationEuler);
}
