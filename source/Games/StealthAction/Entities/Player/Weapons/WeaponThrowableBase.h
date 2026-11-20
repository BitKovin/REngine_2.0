#pragma once

#include "WeaponBase.h"

#include <SkeletalMesh.hpp>

// Base of the weapons that are thrown after being lit with a lighter (dynamite, molotov).
//
// The weapon item is its own ammunition: every pickup is one owned instance, a throw uses the
// held instance up, and the next copy of the same weapon is drawn (see Player::ConsumeHeldWeapon).
//
// One attack press runs the whole sequence, with the animations of the viewmodel glb:
//
//   draw -> idle (loop)
//   attack pressed  -> throw_start   the lighter is opened and struck, the wick / rag is lit (lightTime),
//                                    the arm is cocked below the screen
//   held            -> throw_charge  (loop) strength grows over chargeTime
//   released        -> throw_end     the throwable leaves the hand at releaseTime
//   end of throw_end -> the instance is consumed
//
// Releasing the button early does not cancel anything: the sequence finishes with a weak throw.
// There are no animation events, the timings below come from the animation frames (60 fps).
//
// A fuse (fuseTime >= 0) starts when the wick is lit and keeps burning in the hand. If it runs
// out before the throw, the object goes off in the player's hand.
class WeaponThrowableBase : public Weapon
{
public:

	struct Params
	{
		std::string viewmodelPath;               // glb with the Manny skeleton, the props and the animations
		std::string projectileClass;             // technical name of the thrown entity
		vec3 weaponOffset = vec3(0.0f);

		// Seconds into the animation, from its frames at 60 fps.
		float lightTime = 25.0f / 60.0f;         // throw_start: the wick / rag catches fire (frame 26)
		float releaseTime = 13.0f / 60.0f;       // throw_end: the hand lets go (frame 14)

		float chargeTime = 1.0f;                 // held this long = full strength
		float minThrowSpeed = 6.0f;              // m/s
		float maxThrowSpeed = 20.0f;
		float throwUpBias = 0.12f;               // the throw is aimed a little above the crosshair
		float inheritVelocity = 0.6f;            // share of the player's own velocity the object keeps

		float fuseTime = -1.0f;                  // seconds from lighting, < 0 = no fuse

		// Where the object leaves the hand, from the camera: right, up, forward (m).
		vec3 releaseOffset = vec3(0.22f, -0.18f, 0.45f);

		std::string throwSound = "event:/Weapons/knife/knife_attack";   // placeholder whoosh
	};

	Params params;

	WeaponThrowableBase();

	void LoadAssets() override;
	void Start() override;
	void Update() override;
	void AsyncUpdate() override;
	void LateUpdate() override;

	// No switching while a throw is under way (a lit fuse cannot be put back in the pocket).
	bool CanChangeSlot() override;

	// Remaining stock of this weapon class, for the HUD.
	int GetStockCount() const;

	bool IsArmed() const { return state == ThrowState::Lighting || state == ThrowState::Charging || state == ThrowState::Throwing; }

protected:

	SkeletalMesh* viewmodel = nullptr;
	SkeletalMesh* arms = nullptr;

private:

	enum class ThrowState : uint8_t
	{
		Drawing,
		Idle,
		Lighting,
		Charging,
		Throwing,
		Spent
	};

	ThrowState state = ThrowState::Drawing;

	bool lit = false;
	bool released = false;
	bool releaseQueued = false;   // the button went up while the lighting animation was still running
	float chargeTimer = 0.0f;
	float fuseRemaining = -1.0f;

	void BeginLighting();
	void BeginCharging();
	void BeginThrow(float strength);
	void Ignite();
	void Release(float strength);
	void FinishThrow();
	void SpawnProjectile(const vec3& position, const vec3& velocity, float fuse);
	void UpdateFuse(float dt);

	float throwStrength = 0.0f;
};
