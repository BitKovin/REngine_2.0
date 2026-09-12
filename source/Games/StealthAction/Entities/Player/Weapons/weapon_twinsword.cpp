#pragma once
#include "WeaponMelee.h"

// Dual-blade sword with an alternating L/R combo and an "ultimate mode"
// speed-up. Everything generic (attack/block/parry/counter timers, draw/hide,
// viewmodel loading) comes from WeaponMelee - this class only overrides the
// bits that are genuinely twinsword-specific.
class weapon_twinsword : public WeaponMelee
{
public:

	particle_system_meleeTrail* trail_r = nullptr;
	particle_system_meleeTrail* trail_l = nullptr;

	// Attack state
	int attackSide = 0; // 0 = right next, 1 = left next

	// -----------------------------------------------------------------------
	// Ultimate mode
	// -----------------------------------------------------------------------

	bool ultimateActive = false;
	const float UltimateSpeedMultiplier = 1.65f;

	bool IsUltimateMode() const { return ultimateActive; }

	// Single source of truth - multiply anim speed, divide delays by this value
	float GetAttackSpeedScale() const
	{
		return IsUltimateMode() ? UltimateSpeedMultiplier : 1.0f;
	}

	// -----------------------------------------------------------------------

	weapon_twinsword()
	{
		DualWield = true;

		params.modelPath = "GameData/models/player/weapons/twinsword/twinsword.glb";
		params.texturesLocation = "GameData/models/player/weapons/twinsword/twinsword.glb/";
		params.drawAnim = "draw";
		params.damage = 25.0f;
		params.attackSoundEvent = "event:/Weapons/knife/knife_attack";
		params.hitSoundEvent = "";
		params.attackRange = 1.3f;
		params.attackRadius = 0.4f;

		thirdPersonModelPath = "GameData/models/player/weapons/twinsword/twinsword_tp.glb";
		weaponHandlingType = 2;

		startTrailDelay.AddDelay(100000000.0f); // inert until StartAttack arms it
	}

	// -----------------------------------------------------------------------
	// Trail
	// -----------------------------------------------------------------------

	void StopTrails()
	{
		auto stopOne = [](particle_system_meleeTrail*& t)
			{
				if (t == nullptr) return;
				t->DestroyWithDelay(2.0f);
				t = nullptr;
			};
		stopOne(trail_r);
		stopOne(trail_l);
	}

	// spawnRight / spawnLeft let the caller choose which swords get a trail.
	// Normal attacks pass one true flag; counter-attacks pass both.
	void StartNewTrails(bool spawnRight, bool spawnLeft)
	{
		MathHelper::Transform weaponTransform = MathHelper::Transform();
		weaponTransform.Position = Position;
		weaponTransform.Rotation = Rotation;
		mat4 weaponMatrix = weaponTransform.ToMatrixEuler();
		mat4 inverseWeaponMatrix = glm::inverse(weaponMatrix);

		if (spawnRight)
		{
			trail_r = new particle_system_meleeTrail();
			vec3 trailStart = viewmodel_r->GetBoneMatrixWorld("trailR_start")[3];
			vec3 trailEnd = viewmodel_r->GetBoneMatrixWorld("trailR_end")[3];

			trailStart = vec3(inverseWeaponMatrix * vec4(trailStart, 1.0f));
			trailEnd = vec3(inverseWeaponMatrix * vec4(trailEnd, 1.0f));

			trail_r->SetTrailTransform(trailStart, trailEnd);
			trail_r->Start();
			Level::Current->AddEntity(trail_r);
			trail_r->LoadAssetsIfNeeded();
		}

		if (spawnLeft)
		{
			trail_l = new particle_system_meleeTrail();
			vec3 trailStart = viewmodel_l->GetBoneMatrixWorld("trailL_start")[3];
			vec3 trailEnd = viewmodel_l->GetBoneMatrixWorld("trailL_end")[3];

			trailStart = vec3(inverseWeaponMatrix * vec4(trailStart, 1.0f));
			trailEnd = vec3(inverseWeaponMatrix * vec4(trailEnd, 1.0f));

			trail_l->SetTrailTransform(trailStart, trailEnd);
			trail_l->Start();
			Level::Current->AddEntity(trail_l);
			trail_l->LoadAssetsIfNeeded();
		}
	}

	void UpdateTrail() override
	{
		// Spawn trails once the start delay has elapsed.
		if (!startTrailDelay.Wait())
		{
			StartNewTrails(pendingTrailRight, pendingTrailLeft);
			startTrailDelay.AddDelay(100000000.0f); // re-arm as inert
		}

		// Tell active trails to stop emitting once the hit window is over
		if (!pendingAttackEndDelay.Wait())
		{
			if (trail_r != nullptr) trail_r->StopAll();
			if (trail_l != nullptr) trail_l->StopAll();
		}

		MathHelper::Transform weaponTransform = MathHelper::Transform();
		weaponTransform.Position = Position;
		weaponTransform.Rotation = Rotation;
		mat4 weaponMatrix = weaponTransform.ToMatrixEuler();
		mat4 inverseWeaponMatrix = glm::inverse(weaponMatrix);

		if (trail_r != nullptr)
		{
			vec3 trailStart = viewmodel_r->GetBoneMatrixWorld("trailR_start")[3];
			vec3 trailEnd = viewmodel_r->GetBoneMatrixWorld("trailR_end")[3];

			trailStart = vec3(inverseWeaponMatrix * vec4(trailStart, 1.0f));
			trailEnd = vec3(inverseWeaponMatrix * vec4(trailEnd, 1.0f));

			trail_r->SetTrailTransform(trailStart, trailEnd);
			trail_r->RelativeTransform = weaponMatrix;
		}

		if (trail_l != nullptr)
		{
			vec3 trailStart = viewmodel_l->GetBoneMatrixWorld("trailL_start")[3];
			vec3 trailEnd = viewmodel_l->GetBoneMatrixWorld("trailL_end")[3];

			trailStart = vec3(inverseWeaponMatrix * vec4(trailStart, 1.0f));
			trailEnd = vec3(inverseWeaponMatrix * vec4(trailEnd, 1.0f));

			trail_l->SetTrailTransform(trailStart, trailEnd);
			trail_l->RelativeTransform = weaponMatrix;
		}
	}

	// -----------------------------------------------------------------------
	// Attack
	// -----------------------------------------------------------------------

	void StartAttack() override
	{
		if (counterAvailable)
			pendingCounterAttack = true;

		if (attackDelay.Wait() || isBlocking) return;

		const float s = GetAttackSpeedScale();

		if (counterAvailable)
		{
			pendingCounterAttack = false;
			counterAvailable = false;

			Time::AddTimeScaleEffect(0.3, 0.1, true, "weapon", 0.3f, 0.1f);

			PlayBoth(params.counterAnim, false, 0.1f);
			viewmodel_l->SetAnimationTime(0.1f);
			viewmodel_r->SetAnimationTime(0.1f);

			pendingTrailRight = true;
			pendingTrailLeft = true;

			attackDelay.AddDelay(0.5f / s);
			pendingAttackStartDelay.AddDelay(0.15f / s);
			pendingAttackEndDelay.AddDelay(0.45f / s);

			Camera::AddCameraShake(CameraShake(
				1.5f, 1.5f, vec3(0), vec3(0),
				vec3(-7, 0, 0), vec3(10, 10, 0),
				1.0f, CameraShake::ShakeType::SingleWave
			));
		}
		else
		{
			// Normal attack: only the active sword swings - one trail
			const bool isRight = (attackSide == 0);

			PlayBoth(isRight ? "attack2_r" : "attack2_l", false, 0.1f);

			pendingTrailRight = isRight;
			pendingTrailLeft = !isRight;

			attackDelay.AddDelay(0.5f / s);
			pendingAttackStartDelay.AddDelay(0.15f / s);
			pendingAttackEndDelay.AddDelay(0.5f / s);

			Camera::AddCameraShake(CameraShake(
				1.0f, 1.0f, vec3(0), vec3(0),
				isRight ? vec3(-5, 5, 0) : vec3(-5, -5, 0),
				vec3(7, 7, 0),
				1.0f, CameraShake::ShakeType::SingleWave
			));

			attackSide = isRight ? 1 : 0; // alternate for next hit
		}

		// Kill the previous trails and queue new ones
		StopTrails();
		startTrailDelay.AddDelay(0.15f / s);

		soundToggle ? fireSoundPlayer->Play() : fireSoundPlayer2->Play();
		soundToggle = !soundToggle;

		pendingAttack = true;
		reAttackDelay.AddDelay(0.55f / s);

		NotifyUsed(); // (re)start the 3s auto-hide countdown from this swing

		WarnAboutAttack();
	}

	// Called every frame while the hit window is open.
	void PerformAttack() override
	{
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
			float damageToDeal = params.damage * GetDamageMultiplier();

			float ultimateFinalDamageMultiplier = 2.0f / GetAttackSpeedScale();

			if (IsUltimateMode())
				damageToDeal *= ultimateFinalDamageMultiplier;

			if (Player::Instance->powerUpManager.IsPowerUpActive(PowerUpManager::PowerUpType::TripleDamage))
				damageToDeal *= 3;

			bool isEnemy = dynamic_cast<IEnemy*>(hit.entity) && hit.entity->Health > 0;

			if (isEnemy)
				Player::Instance->Heal(damageToDeal * 0.10f);

			hit.entity->OnPointDamage(
				damageToDeal,
				hit.position,
				MathHelper::GetForwardVector(Camera::rotation),
				"spine_03",
				owner,
				this
			);

			Physics::AddImpulseAtLocation(hit.hitbody, MathHelper::GetForwardVector(Camera::rotation) * (params.damage + 2) * 14.0f, hit.position);

			pendingAttack = false; // stop re-tracing once an entity is hit
		}

		hitSoundPlayer->Position = hit.position;
		hitSoundPlayer->Play();
	}

	// GetDamageMultiplier() (base class) checks for params.counterAnim, which
	// matches "attack_counter" here too, so the inherited 3x-on-counter still
	// applies without an override.

	// -----------------------------------------------------------------------
	// Block / parry
	// -----------------------------------------------------------------------

	void StartBlock() override
	{
		if (attackDelay.Wait()) return;
		if (counterWindow.Wait()) return;

		isBlocking = true;
		pendingAttack = false; // cancel any in-flight swing

		PlayBoth("block_start", false, 0.1f);

		blockStartDelay.AddDelay(0.2f);

		// Parry window is animation-time driven (see Update).
		// Spam check is handled via parrySpamWindow set in EndBlock.
	}

	void EndBlock() override
	{
		if (viewmodel_r->GetAnimationName() == "block_start" && viewmodel_r->GetAnimationTime() < 0.2f) return;

		// Arm the spam window so the next block press can't instantly parry
		parrySpamWindow.AddDelay(0.3f);

		isBlocking = false;

		if (viewmodel_r->GetAnimationName() == "block_start")
		{
			PlayBoth("block_stop", false, 0.1f);

			attackDelay.AddDelay(0.3f);
		}
	}

	// Called by the engine when an enemy attack lands during the parry window
	void OnParried() override
	{
		SoundPlayer::PlayOneshot(params.parrySoundEvent, 1.0f, 1.0f, false);
		Time::AddTimeScaleEffect(0.3, 0.1, true, "parry", 0.24f, 0.1f);
		EndBlock();

		isBlocking = false;

		forceBlockInput.AddDelay(0.5f);

		PlayBoth("parry", false, 0.05f);

		attackDelay.AddDelay(0.2f);

		// Open the counter-attack opportunity
		counterAvailable = true;
		counterWindow.AddDelay(1.1f);
	}

	// -----------------------------------------------------------------------
	// Update
	// -----------------------------------------------------------------------

	void Update() override
	{
		// Handles attack/block dispatch, hit-window resolution, counter
		// expiry, and the draw/hide progress ramp. Parrying/Blocking below
		// are then recomputed with twinsword's animation-time-driven window
		// instead of the base class's simpler Delay-based one.
		WeaponMelee::Update();

		bool blockAnimPlaying =
			isBlocking &&
			viewmodel_r->GetAnimationTime() < viewmodel_r->GetAnimationDuration();

		if (blockAnimPlaying)
			blockingWindow.AddDelay(0.1f);

		Parrying = (blockAnimPlaying && !parrySpamWindow.Wait()) || blockingWindow.Wait();
		Blocking = (isBlocking && !blockStartDelay.Wait()) || parrySpamWindow.Wait();
	}

	void AsyncUpdate() override
	{
		const float speed = GetAttackSpeedScale();
		viewmodel_r->Update(speed);
		viewmodel_l->Update(speed);

		// arms_r: driven by viewmodel_r, right arm shown - hide left clavicle
		{
			auto pose = viewmodel_r->GetAnimationPose();
			auto bone = pose.GetBoneTransform("clavicle_l");
			bone.Rotation += vec3(120, 0, 0);
			bone.Scale *= vec3(0.0f);
			pose.SetBoneTransformEuler("clavicle_l", bone);
			viewmodel_r->PasteAnimationPose(pose);
			arms_r->PasteAnimationPose(pose);
		}

		// arms_l: driven by viewmodel_l, left arm shown - hide right clavicle
		{
			auto pose = viewmodel_l->GetAnimationPose();
			auto bone = pose.GetBoneTransform("clavicle_r");
			bone.Rotation += vec3(120, 0, 0);
			bone.Scale *= vec3(0.0f);
			pose.SetBoneTransformEuler("clavicle_r", bone);
			viewmodel_l->PasteAnimationPose(pose);
			arms_l->PasteAnimationPose(pose);
		}
	}

	// -----------------------------------------------------------------------
	// Misc
	// -----------------------------------------------------------------------

	WeaponSlotData GetDefaultData() override
	{
		WeaponSlotData data;
		data.className = "weapon_twinsword";
		return data;
	}
};

REGISTER_ENTITY(weapon_twinsword, "weapon_twinsword")
