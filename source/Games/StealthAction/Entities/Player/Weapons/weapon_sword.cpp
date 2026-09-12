#pragma once
#include "WeaponMelee.h"

// Simple single-blade sword: a 3-hit combo (attack_start -> attack -> attack2
// -> attack_finish) is the only thing that doesn't fit WeaponMelee's default
// single-swing behaviour, so that's all this class overrides. Block/parry/
// counter-attack and the draw/hide handling all come from the base class.
class weapon_sword : public WeaponMelee
{
public:

	particle_system_meleeTrail* trail = nullptr;

	// Combo state
	int attack = -1;
	bool hadHit = false;

	weapon_sword()
	{
		params.modelPath = "GameData/models/player/weapons/sword/sword.glb";
		params.texturesLocation = "GameData/models/player/weapons/sword/sword.glb/";
		params.drawAnim = "draw";
		params.damage = 30.0f;
		params.attackSoundEvent = "event:/Weapons/knife/knife_attack";
		params.hitSoundEvent = "event:/Weapons/knife/knife_hit";
		params.attackRange = 1.3f;
		params.attackRadius = 0.3f;

		startTrailDelay.AddDelay(100000000.0f); // start inert - only armed after StartAttack
	}

	// -----------------------------------------------------------------------
	// Trail
	// -----------------------------------------------------------------------

	void StopTrail()
	{
		if (trail == nullptr) return;
		trail->DestroyWithDelay(2.0f);
		trail = nullptr;
	}

	void StartNewTrail()
	{
		trail = new particle_system_meleeTrail();
		vec3 trailStart = viewmodel_r->GetBoneMatrixWorld("trail_start")[3];
		vec3 trailEnd = viewmodel_r->GetBoneMatrixWorld("trail_end")[3];

		trail->SetTrailTransform(trailStart, trailEnd);
		trail->Start();
		Level::Current->AddEntity(trail);
		trail->LoadAssetsIfNeeded();
	}

	void UpdateTrail() override
	{
		// Spawn the trail once the start delay has elapsed
		if (!startTrailDelay.Wait())
		{
			StartNewTrail();
			startTrailDelay.AddDelay(100000000.0f); // re-arm as inert
		}

		// Tell the trail to stop emitting once the hit window is over
		if (!pendingAttackEndDelay.Wait() && trail != nullptr)
			trail->StopAll();

		if (trail == nullptr) return;

		// Feed the two blade endpoints from the animated skeleton
		vec3 trailStart = viewmodel_r->GetBoneMatrixWorld("trail_start")[3];
		vec3 trailEnd = viewmodel_r->GetBoneMatrixWorld("trail_end")[3];

		trail->SetTrailTransform(trailStart, trailEnd);
	}

	// -----------------------------------------------------------------------
	// Attack combo
	// -----------------------------------------------------------------------

	void StartAttack() override
	{
		if (attackDelay.Wait() || isBlocking) return;

		// Advance or reset the combo counter
		if (reAttackDelay.Wait())
			attack++;
		else
			if (attack >= 0) attack = 0;

		if (attack > 2) attack = 0;

		// Finisher only available if the previous swing actually landed
		if (attack == 2 && !hadHit) attack = 0;

		// Play the correct animation for each combo step
		if (attack == -1)
		{
			viewmodel_r->PlayAnimation("attack_start", false, 0.0f);
			attackDelay.AddDelay(0.4f);
			pendingAttackStartDelay.AddDelay(0.0f);
			Camera::AddCameraShake(CameraShake(1.0f, 1.0f, vec3(0), vec3(0), vec3(-5, -5, 0), vec3(7, 7, 0), 1.0f, CameraShake::ShakeType::SingleWave));
		}
		else if (attack == 0)
		{
			viewmodel_r->PlayAnimation("attack", false, 0.1f);
			attackDelay.AddDelay(0.4f);
			pendingAttackStartDelay.AddDelay(0.15f);
			Camera::AddCameraShake(CameraShake(1.0f, 1.0f, vec3(0), vec3(0), vec3(-5, -5, 0), vec3(7, 7, 0), 1.0f, CameraShake::ShakeType::SingleWave));
		}
		else if (attack == 1)
		{
			viewmodel_r->PlayAnimation("attack2", false, 0.1f);
			attackDelay.AddDelay(0.4f);
			pendingAttackStartDelay.AddDelay(0.15f);
			Camera::AddCameraShake(CameraShake(1.0f, 1.0f, vec3(0), vec3(0), vec3(-5, 5, 0), vec3(7, 7, 0), 1.0f, CameraShake::ShakeType::SingleWave));
		}
		else if (attack == 2)
		{
			viewmodel_r->PlayAnimation("attack_finish", false, 0.1f);
			attackDelay.AddDelay(0.5f);
			pendingAttackStartDelay.AddDelay(0.15f);
		}

		// Hit-window closes after this delay - attack must land before then
		pendingAttackEndDelay.AddDelay(0.35f);

		// Kill the previous trail immediately and start a new one after a
		// short delay so it begins at the moment the blade actually moves
		StopTrail();
		startTrailDelay.AddDelay(0.19f);

		soundToggle ? fireSoundPlayer->Play() : fireSoundPlayer2->Play();
		soundToggle = !soundToggle;

		pendingAttack = true;
		reAttackDelay.AddDelay(0.55f);

		NotifyUsed(); // (re)start the 3s auto-hide countdown from this swing
	}

	// Called every frame while the hit window is open.
	void PerformAttack() override
	{
		hadHit = false;

		auto hit = Physics::SphereTrace(
			Camera::position,
			Camera::position + MathHelper::GetForwardVector(Camera::rotation) * params.attackRange,
			params.attackRadius,
			BodyType::GroupHitTest,
			{ Player::Instance->LeadBody },
			{ Player::Instance }
		);

		if (!hit.hasHit) return;

		if (hit.entity != nullptr)
		{
			hit.entity->OnPointDamage(params.damage, hit.position, MathHelper::GetForwardVector(Camera::rotation), hit.hitboxName, Player::Instance, this);
			hadHit = true;
			pendingAttack = false; // stop re-tracing once an entity is hit
		}

		hitSoundPlayer->Position = hit.position;
		hitSoundPlayer->Play();
	}

	// -----------------------------------------------------------------------
	// Misc
	// -----------------------------------------------------------------------

	WeaponSlotData GetDefaultData() override
	{
		WeaponSlotData data;
		data.className = "weapon_sword";
		return data;
	}
};

REGISTER_ENTITY(weapon_sword, "weapon_sword")
