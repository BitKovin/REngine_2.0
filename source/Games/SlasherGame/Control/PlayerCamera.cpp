#include "PlayerCamera.h"

#include <Characters/CombatCharacter.h>
#include <Settings/GameSettings.h>

#include <Camera.h>
#include <Physics.h>

#include <algorithm>
#include <cmath>

void PlayerCamera::Reset(const CombatCharacter& body)
{
	yaw = body.GetYaw();
	pitch = settings.defaultPitch;
	pivot = body.Position + glm::vec3(0.0f, settings.focusHeight, 0.0f);
	impact.Reset();
	initialized = true;
}

void PlayerCamera::AddLookInput(const glm::vec2& delta)
{
	yaw = MathHelper::NormalizeAngle(yaw + delta.x * settings.sensitivity);
	pitch = std::clamp(pitch - delta.y * settings.sensitivity, settings.minPitch, settings.maxPitch);
}

glm::vec3 PlayerCamera::GetForward() const
{
	return MathHelper::GetForwardVector(glm::vec3(pitch, yaw, 0.0f));
}

glm::vec3 PlayerCamera::GetPlanarForward() const
{
	return MathHelper::GetForwardVector(glm::vec3(0.0f, yaw, 0.0f));
}

glm::vec3 PlayerCamera::GetPlanarRight() const
{
	return MathHelper::GetRightVector(glm::vec3(0.0f, yaw, 0.0f));
}

void PlayerCamera::Update(const CombatCharacter& body, float deltaTime)
{
	if (!initialized)
		Reset(body);

	// The focus point on the body, smoothed (frame-rate independent); the boom itself is rigid.
	const glm::vec3 focusTarget = body.Position + glm::vec3(0.0f, settings.focusHeight, 0.0f);
	const float blend = 1.0f - std::exp(-settings.followSharpness * deltaTime);
	pivot = glm::mix(pivot, focusTarget, blend);

	const glm::vec3 rotation(pitch, yaw, 0.0f);
	const glm::vec3 forward = MathHelper::GetForwardVector(rotation);
	const glm::vec3 up = MathHelper::GetUpVector(rotation);
	const glm::vec3 right = MathHelper::GetRightVector(glm::vec3(0.0f, yaw, 0.0f));

	// Framing in camera space: the view center sits frameHeight above the focus point along the camera's own up axis
	// (and shoulderOffset to its right), so the body keeps the same place on screen - below the crosshair - at every
	// pitch, looking down included. Looking up the boom shortens (the camera stays above the floor); the offset
	// shortens with it so the framing angle does not change.
	const float lookingUp = settings.minPitch < 0.0f ? glm::smoothstep(0.0f, 1.0f, pitch / settings.minPitch) : 0.0f;
	const float distance = glm::mix(settings.distance, settings.lookUpDistance, lookingUp);
	const float frameScale = distance / settings.distance;

	const glm::vec3 shoulder = pivot + (up * settings.frameHeight + right * settings.shoulderOffset) * frameScale;

	// Keep the camera out of walls: sweep from the body to the shoulder, then along the boom.
	const std::vector<Entity*> ignore = { const_cast<CombatCharacter*>(&body) };
	const Physics::HitResult shoulderHit = Physics::SphereTrace(pivot, shoulder, settings.collisionRadius, BodyType::World, {}, ignore);
	const glm::vec3 boomStart = shoulderHit.hasHit ? shoulderHit.shapePosition : shoulder;
	const glm::vec3 boomEnd = boomStart - forward * distance;
	const Physics::HitResult boomHit = Physics::SphereTrace(boomStart, boomEnd, settings.collisionRadius, BodyType::World, {}, ignore);
	const glm::vec3 position = boomHit.hasHit ? boomHit.shapePosition : boomEnd;

	// Impact response on top of the framed view, scaled by the player's Camera Shake setting (0 = none): +pitch looks
	// down and +yaw turns left, both move the image toward +y / +x on screen; the offset slides the camera along its
	// own right / up.
	const float intensity = std::clamp(GameSettings::Instance().Gameplay.CameraShake, GameplaySettingsData::MinCameraShake,
		GameplaySettingsData::MaxCameraShake);
	impact.Update(deltaTime);
	const glm::vec2 turn = impact.GetRotation() * intensity;
	const glm::vec2 slide = impact.GetOffset() * intensity;

	Camera::position = position + MathHelper::GetRightVector(rotation) * slide.x + MathHelper::GetUpVector(rotation) * slide.y;
	Camera::rotation = rotation + glm::vec3(turn.x, turn.y, 0.0f);
	Camera::FOV = settings.fov;

	// Shakes queued through the engine (explosions, scripted events) on top, scaled the same way.
	const glm::vec3 steadyPosition = Camera::position;
	const glm::vec3 steadyRotation = Camera::rotation;
	Camera::ApplyCameraShake(deltaTime);
	Camera::position = glm::mix(steadyPosition, Camera::position, intensity);
	Camera::rotation = glm::mix(steadyRotation, Camera::rotation, intensity);
}

void PlayerCamera::AddKick(const glm::vec3& worldDirection, float size)
{
	// The blow as seen on screen; one travelling along the view axis has no screen direction (CameraImpact: down).
	const glm::vec3 rotation(pitch, yaw, 0.0f);
	glm::vec2 screen(glm::dot(worldDirection, MathHelper::GetRightVector(rotation)), glm::dot(worldDirection, MathHelper::GetUpVector(rotation)));
	if (glm::length(screen) < 0.2f)
		screen = glm::vec2(0.0f);
	impact.Kick(screen, size);
}

void PlayerCamera::AddShake(float trauma)
{
	impact.Shake(trauma);
}
