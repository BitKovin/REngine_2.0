#include "WeaponThrowableBase.h"

#include "../Player.hpp"
#include "Projectiles/ThrownProjectileBase.h"

#include <Entities/SoundPlayer.h>
#include <SoundSystem/FmodEventInstance.h>
#include <Logger.hpp>
#include <World/WorldOrientationManager.h>

#include <algorithm>

namespace
{
	const char* kAnimDraw = "draw";
	const char* kAnimIdle = "idle";
	const char* kAnimStart = "throw_start";
	const char* kAnimCharge = "throw_charge";
	const char* kAnimEnd = "throw_end";

	constexpr float kIdleBlend = 0.15f;
	constexpr float kStartBlend = 0.12f;
	// throw_charge starts exactly on the pose throw_start ends on, so it needs no blend.
	constexpr float kChargeBlend = 0.0f;
	// The right hand of throw_end does not start where throw_charge ends; a short blend hides the jump.
	constexpr float kEndBlend = 0.12f;
}

WeaponThrowableBase::WeaponThrowableBase()
{
	LateUpdateWhenPaused = true;

	// Both hands are busy: the lighter is in the left one.
	SupportsOffhandWeapon = false;
	weaponHandlingType = 1;
}

void WeaponThrowableBase::LoadAssets()
{
	Weapon::LoadAssets();

	SoundManager::LoadBankFromPath("GameData/sounds/banks/Desktop/Weapons.bank");

	viewmodel = new SkeletalMesh(owner);
	arms = new SkeletalMesh(owner);

	viewmodel->GravityAlignedRotation = true;
	// The flames, the spark and the rag are single cards, and from the camera's side some of them face away: without this they are culled.
	viewmodel->TwoSided = true;
	arms->GravityAlignedRotation = true;

	viewmodel->LoadFromFile(params.viewmodelPath);
	viewmodel->TexturesLocation = params.viewmodelPath + "/";
	viewmodel->PlayAnimation(kAnimDraw, false, 0.0f);
	viewmodel->PreloadAssets();
	viewmodel->IsViewmodel = true;
	Drawables.push_back(viewmodel);

	arms->LoadFromFile(ArmsModelPath);
	arms->IsViewmodel = true;
	Drawables.push_back(arms);

	PreloadEntityType(params.projectileClass);
}

void WeaponThrowableBase::Start()
{
	SwitchDelay.AddDelay(0.35f);
	state = ThrowState::Drawing;
}

bool WeaponThrowableBase::CanChangeSlot()
{
	return !IsArmed() && SwitchDelay.Wait() == false;
}

int WeaponThrowableBase::GetStockCount() const
{
	return owner != nullptr ? owner->CountOwnedWeapons(Data.className) : 0;
}

// -----------------------------------------------------------------------------------------------
// Update
// -----------------------------------------------------------------------------------------------

void WeaponThrowableBase::Update()
{
	if (viewmodel == nullptr)
		return;

	const float dt = Time::DeltaTimeF;
	const bool holding = Input::GetAction("attack")->Holding();

	UpdateFuse(dt);

	switch (state)
	{
	case ThrowState::Drawing:
		if (!viewmodel->IsAnimationPlaying())
		{
			state = ThrowState::Idle;
			viewmodel->PlayAnimation(kAnimIdle, true, kIdleBlend);
		}
		break;

	case ThrowState::Idle:
		// The press is buffered, so one made while the weapon was still being drawn counts.
		if (Input::GetAction("attack")->PressedBuffered() && CanAttack())
			BeginLighting();
		break;

	case ThrowState::Lighting:
		if (!lit && viewmodel->GetAnimationTime() >= params.lightTime)
			Ignite();

		if (!holding)
			releaseQueued = true;

		if (!viewmodel->IsAnimationPlaying())
		{
			if (releaseQueued)
				BeginThrow(0.0f);
			else
				BeginCharging();
		}
		break;

	case ThrowState::Charging:
		chargeTimer += dt;
		if (!holding)
			BeginThrow(std::clamp(chargeTimer / std::max(params.chargeTime, 0.01f), 0.0f, 1.0f));
		break;

	case ThrowState::Throwing:
		if (!released && viewmodel->GetAnimationTime() >= params.releaseTime)
			Release(throwStrength);

		if (!viewmodel->IsAnimationPlaying())
		{
			if (!released)
				Release(throwStrength);
			FinishThrow();
		}
		break;

	case ThrowState::Spent:
		break;
	}
}

void WeaponThrowableBase::AsyncUpdate()
{
	if (viewmodel == nullptr)
		return;

	viewmodel->Update();
	arms->PasteAnimationPose(viewmodel->GetAnimationPose());
}

void WeaponThrowableBase::LateUpdate()
{
	if (viewmodel == nullptr)
		return;

	viewmodel->Position = Position + (mat3)Camera::GetRotationMatrix() * params.weaponOffset;
	viewmodel->Rotation = Rotation;

	arms->Position = viewmodel->Position;
	arms->Rotation = viewmodel->Rotation;

	// A spent weapon has nothing in its hands and waits for the player to draw the next one.
	const bool visible = owner != nullptr && !owner->InThirdPerson() && state != ThrowState::Spent;
	viewmodel->Visible = visible;
	arms->Visible = visible;
}

// -----------------------------------------------------------------------------------------------
// Sequence
// -----------------------------------------------------------------------------------------------

void WeaponThrowableBase::BeginLighting()
{
	state = ThrowState::Lighting;
	lit = false;
	released = false;
	releaseQueued = false;
	fuseRemaining = -1.0f;

	viewmodel->PlayAnimation(kAnimStart, false, kStartBlend);
}

void WeaponThrowableBase::BeginCharging()
{
	state = ThrowState::Charging;
	chargeTimer = 0.0f;

	viewmodel->PlayAnimation(kAnimCharge, true, kChargeBlend);
}

void WeaponThrowableBase::BeginThrow(float strength)
{
	state = ThrowState::Throwing;
	throwStrength = strength;
	released = false;

	viewmodel->PlayAnimation(kAnimEnd, false, kEndBlend);
}

void WeaponThrowableBase::Ignite()
{
	lit = true;
	fuseRemaining = params.fuseTime;
}

void WeaponThrowableBase::UpdateFuse(float dt)
{
	// The fuse burns while the dynamite is in the hand, from lighting until it is let go.
	if (!lit || released || fuseRemaining < 0.0f || state == ThrowState::Spent)
		return;

	fuseRemaining -= dt;
	if (fuseRemaining > 0.0f)
		return;

	// Held too long: it goes off in the player's hand.
	released = true;

	const vec3 forward = MathHelper::GetForwardVector(Camera::rotation);
	const vec3 up = MathHelper::GetUpVector(Camera::rotation);
	SpawnProjectile(Camera::position + forward * 0.3f - up * 0.2f, vec3(0.0f), 0.0f);

	FinishThrow();
}

void WeaponThrowableBase::Release(float strength)
{
	released = true;

	const vec3 forward = MathHelper::GetForwardVector(Camera::rotation);
	const vec3 right = MathHelper::GetRightVector(Camera::rotation);
	const vec3 up = MathHelper::GetUpVector(Camera::rotation);

	// Starts at the hand, but never inside a wall the player is standing against.
	vec3 start = Camera::position + right * params.releaseOffset.x + up * params.releaseOffset.y + forward * params.releaseOffset.z;
	const auto wall = Physics::SphereTrace(Camera::position, start, 0.1f, BodyType::GroupHitTest, { Player::Instance->LeadBody }, { Player::Instance });
	if (wall.hasHit)
		start = wall.shapePosition;

	// Aim at whatever is under the crosshair, so the throw goes where the player looks even
	// though the hand is lower and to the side. Close targets would flip the direction: use the view direction then.
	vec3 aimPoint = Camera::position + forward * 60.0f;
	const auto aim = Physics::LineTrace(Camera::position, aimPoint, BodyType::GroupHitTest, { Player::Instance->LeadBody }, { Player::Instance });
	if (aim.hasHit && glm::length(aim.position - Camera::position) > 2.0f)
		aimPoint = aim.position;

	vec3 direction = glm::normalize(aimPoint - start);
	direction = glm::normalize(direction + WorldOrientationManager::GetUpVector() * params.throwUpBias);

	const float speed = glm::mix(params.minThrowSpeed, params.maxThrowSpeed, strength);
	const vec3 velocity = direction * speed + owner->controller.GetVelocity() * params.inheritVelocity;

	SpawnProjectile(start, velocity, fuseRemaining);

	SoundPlayer::PlayOneshot(params.throwSound, 1.5f, 1.0f, true);
}

void WeaponThrowableBase::SpawnProjectile(const vec3& position, const vec3& velocity, float fuse)
{
	ThrownProjectileBase* projectile = dynamic_cast<ThrownProjectileBase*>(Spawn(params.projectileClass));
	if (projectile == nullptr)
	{
		Logger::Log("WeaponThrowableBase: '" + params.projectileClass + "' is not a ThrownProjectileBase");
		return;
	}

	projectile->Throw(position, velocity, fuse, owner);
	projectile->LoadAssetsIfNeeded();
	projectile->Start();
}

void WeaponThrowableBase::FinishThrow()
{
	state = ThrowState::Spent;

	// One instance of the weapon is the ammunition. The player picks the next copy of it
	// (or the weapon used before) and this one is swapped out as soon as the switch is allowed.
	if (owner != nullptr)
		owner->ConsumeHeldWeapon();
}
