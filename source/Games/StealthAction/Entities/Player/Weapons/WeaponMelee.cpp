#include "WeaponMelee.h"

WeaponMelee::WeaponMelee()
{
	LateUpdateWhenPaused = true;

	DrawTime = 0.2f;
	HideTime = 0.4f;

	// Melee has no "hold to ready" input like firearms - it's presented the
	// moment it's equipped, and only lowers via the auto-hide timer / hide
	// button (see Weapon::autoHideTimer).
	autoHideTimer = AutoHideWaitTime;

	HiddenPosePosition = vec3(0.00f, 0.0f, 0.00f);
	HiddenPoseRotation = vec3(20.0f, 0.0f, 0.0f);
	HiddenPoseRotationPoint = vec3(0.0f, 0.f, 0.f);
}

void WeaponMelee::Start()
{
	fireSoundPlayer = SoundPlayer::Create(params.attackSoundEvent);
	fireSoundPlayer2 = SoundPlayer::Create(params.attackSoundEvent);
	hitSoundPlayer = SoundPlayer::Create(params.hitSoundEvent);

	SwitchDelay.AddDelay(0.35f);

	// Self-init immediately, same as WeaponFirearm::Start() - entities
	// spawned mid-Update() don't get their own Update() call until next
	// frame otherwise, which would both show a wrong pose for one frame and
	// (for the "attack" press that caused this weapon to be equipped in the
	// first place) miss the swing entirely.
	Update();
	AsyncUpdate();
	LateUpdate();
}

void WeaponMelee::LoadAssets()
{
	Weapon::LoadAssets();

	SoundManager::LoadBankFromPath("GameData/sounds/banks/Desktop/Weapons.bank");

	auto loadBlade = [&](SkeletalMesh*& vm, SkeletalMesh*& a, bool mirror)
	{
		vm = new SkeletalMesh(owner);
		a = new SkeletalMesh(owner);

		vm->GravityAlignedRotation = true;
		vm->LoadFromFile(params.modelPath);
		if (!params.texturesLocation.empty())
			vm->TexturesLocation = params.texturesLocation;

		vm->PlayAnimation(params.drawAnim, false, 0.0f);
		// SkipDrawAnimation (default true): jump straight to the last frame
		// of the draw clip instead of playing it - see Weapon::DrawProgress
		// for the procedural raise/lower motion that replaces it.
		if (SkipDrawAnimation)
			vm->SetAnimationTime(vm->GetAnimationDuration());

		vm->IsViewmodel = true;
		Drawables.push_back(vm);

		a->GravityAlignedRotation = true;
		a->LoadFromFile(ArmsModelPath);
		a->IsViewmodel = true;
		Drawables.push_back(a);

		if (mirror)
		{
			vm->Scale = vec3(-1, 1, 1);
			a->Scale = vec3(-1, 1, 1);
			vm->TwoSided = true;
			a->TwoSided = true;
		}
	};

	loadBlade(viewmodel_r, arms_r, false);

	if (DualWield)
		loadBlade(viewmodel_l, arms_l, false); // twinsword's two blades are separate assets, not mirrored

	if (!params.modelPathTp.empty())
		thirdPersonModelPath = params.modelPathTp;

	SwitchDelay.AddDelay(0.4f);
}

void WeaponMelee::PlayBoth(const std::string& anim, bool loop, float blend)
{
	viewmodel_r->PlayAnimation(anim, loop, blend);
	if (DualWield)
		viewmodel_l->PlayAnimation(anim, loop, blend);
}

float WeaponMelee::GetDamageMultiplier() const
{
	return (viewmodel_r->GetAnimationName() == params.counterAnim) ? 3.0f : 1.0f;
}

void WeaponMelee::WarnAboutAttack()
{
	auto hit = Physics::SphereTrace(
		Camera::position,
		Camera::position + MathHelper::GetForwardVector(Camera::rotation) * 1.5f,
		0.5f,
		BodyType::GroupHitTest,
		{ Player::Instance->LeadBody },
		{ Player::Instance }
	);

	if (hit.hasHit && hit.entity != nullptr)
	{
		if (auto* enemy = dynamic_cast<NpcGuardMelee*>(hit.entity))
			enemy->WarnAboutAttack(owner);
	}
}

void WeaponMelee::StartAttack()
{
	if (counterAvailable)
		pendingCounterAttack = true;

	if (attackDelay.Wait() || isBlocking) return;

	bool isCounter = counterAvailable;

	if (isCounter)
	{
		pendingCounterAttack = false;
		counterAvailable = false;
		PlayBoth(params.counterAnim, false, 0.1f);
	}
	else
	{
		PlayBoth(params.attackAnim, false, 0.1f);
	}

	attackDelay.AddDelay(params.attackDelayTime);
	pendingAttackStartDelay.AddDelay(params.attackWindowStart);
	pendingAttackEndDelay.AddDelay(params.attackWindowEnd);
	reAttackDelay.AddDelay(params.reAttackDelayTime);

	Camera::AddCameraShake(CameraShake(
		1.0f, 1.0f, vec3(0), vec3(0), vec3(-5, -5, 0), vec3(7, 7, 0),
		1.0f, CameraShake::ShakeType::SingleWave
	));

	soundToggle ? fireSoundPlayer->Play() : fireSoundPlayer2->Play();
	soundToggle = !soundToggle;

	pendingAttack = true;

	NotifyUsed(); // (re)start the 3s auto-hide countdown from this swing

	WarnAboutAttack();
}

// Called every frame while the hit window is open. pendingAttack is only
// cleared on a successful entity hit, so a wide swing keeps re-tracing.
void WeaponMelee::PerformAttack()
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
		float damage = params.damage * GetDamageMultiplier();

		if (Player::Instance->powerUpManager.IsPowerUpActive(PowerUpManager::PowerUpType::TripleDamage))
			damage *= 3.0f;

		hit.entity->OnPointDamage(damage, hit.position, MathHelper::GetForwardVector(Camera::rotation), hit.hitboxName, owner, this);
		Physics::AddImpulseAtLocation(hit.hitbody, MathHelper::GetForwardVector(Camera::rotation) * damage * 10.0f, hit.position);

		pendingAttack = false; // stop re-tracing once an entity is hit
	}

	if (hitSoundPlayer != nullptr)
	{
		hitSoundPlayer->Position = hit.position;
		hitSoundPlayer->Play();
	}
}

void WeaponMelee::StartBlock()
{
	if (attackDelay.Wait()) return;

	isBlocking = true;
	pendingAttack = false; // cancel any in-flight swing

	PlayBoth(params.blockStartAnim, false, 0.1f);
	blockStartDelay.AddDelay(params.blockStartDelayTime);

	// Parry window - anti-spam: disabled right after releasing a block (see EndBlock)
	if (!parrySpamWindow.Wait())
		parryWindow.AddDelay(0.5f);
}

void WeaponMelee::EndBlock()
{
	parrySpamWindow.AddDelay(params.parrySpamWindowTime);

	isBlocking = false;

	PlayBoth(params.blockEndAnim, false, 0.1f);

	attackDelay.AddDelay(0.3f);
}

// Called by the engine when an enemy attack lands during the parry window.
void WeaponMelee::OnParried()
{
	SoundPlayer::PlayOneshot(params.parrySoundEvent, 1.0f, 1.0f, false);

	// Open the counter-attack opportunity - pressing attack again right away
	// lands a bonus-damage hit, see GetDamageMultiplier()/StartAttack().
	counterAvailable = true;
	counterWindow.AddDelay(params.counterWindowTime);
}

void WeaponMelee::Update()
{
	if (!counterWindow.Wait())
	{
		pendingCounterAttack = false;
		counterAvailable = false;
	}

	// Primary attack / counter
	if (Input::GetAction("attack")->PressedBuffered(0.2f) || pendingCounterAttack)
		StartAttack();

	// Block: hold to block, release to lower guard.
	if (Input::GetAction("block")->PressedBuffered(0.1f) && !isBlocking)
		StartBlock();

	if (!Input::GetAction("block")->Holding() && isBlocking)
		EndBlock();

	// Resolve hit each frame while inside the valid window:
	//   - pendingAttackStartDelay elapsed  -> blade has reached the hit zone
	//   - pendingAttackEndDelay still alive -> blade hasn't passed through yet
	if (pendingAttack && !pendingAttackStartDelay.Wait() && pendingAttackEndDelay.Wait())
		PerformAttack();

	// Presented whenever it's the current weapon - melee doesn't have a
	// "hold to ready" input, so the auto-hide countdown (started at full in
	// the ctor, refreshed by NotifyUsed() below, collapsible by the hide
	// button via RequestHide()) is the only thing that lowers it.
	UpdateAutoHideTimer();
	UpdateDrawProgress(WantsPresented());

	// Default parry/block windows - a simple Delay-based window. Weapons
	// with a richer, animation-time-driven window (see weapon_twinsword)
	// override Update() and recompute these after calling this base version.
	Parrying = parryWindow.Wait();
	Blocking = (isBlocking && !blockStartDelay.Wait()) || parrySpamWindow.Wait();
}

void WeaponMelee::AsyncUpdate()
{
	viewmodel_r->Update();
	{
		auto pose = viewmodel_r->GetAnimationPose();
		auto bone = pose.GetBoneTransform("clavicle_l");
		bone.Rotation += vec3(120, 0, 0);
		bone.Scale *= vec3(0.0f);
		pose.SetBoneTransformEuler("clavicle_l", bone);
		viewmodel_r->PasteAnimationPose(pose);
		arms_r->PasteAnimationPose(pose);
	}

	if (DualWield)
	{
		viewmodel_l->Update();
		auto pose = viewmodel_l->GetAnimationPose();
		auto bone = pose.GetBoneTransform("clavicle_r");
		bone.Rotation += vec3(120, 0, 0);
		bone.Scale *= vec3(0.0f);
		pose.SetBoneTransformEuler("clavicle_r", bone);
		viewmodel_l->PasteAnimationPose(pose);
		arms_l->PasteAnimationPose(pose);
	}
}

void WeaponMelee::LateUpdate()
{
	const vec3 pos = Position + (mat3)Camera::GetRotationMatrix() * weaponOffset;
	const vec3 rot = Rotation;

	viewmodel_r->Position = pos;
	viewmodel_r->Rotation = rot;
	arms_r->Position = pos;
	arms_r->Rotation = rot;

	if (DualWield)
	{
		viewmodel_l->Position = pos;
		viewmodel_l->Rotation = rot;
		arms_l->Position = pos;
		arms_l->Rotation = rot;
	}

	UpdateTrail();
}
