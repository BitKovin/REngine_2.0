#include "WeaponFirearm.h"
#include "../../../Gore/StealthGoreAdapter.h"
#include <algorithm>
#include "Animators/WeaponAnimator.h"
#include <RandomHelper.h>
#include "Projectiles/Bullet.h"
#include <SoundSystem/FmodEventInstance.h>
#include "WeaponFireFlash.h"
#include <AiPerception/AiPerceptionSystem.h>
#include <AiPerception/Observer.h>
#include "../../Npc/NpcBase.h"

WeaponFirearm::WeaponFirearm(const FirearmParams& initialParams)
	: params(initialParams)
{
	LateUpdateWhenPaused = params.lateUpdateWhenPaused;

	thirdPersonModelPath = params.modelPathTp;

}

void WeaponFirearm::Start() {
	fireSoundPlayer = SoundPlayer::Create(params.fireSoundEvent);
	fireSoundPlayer->Volume = params.fireVolume;
	fireSoundPlayer->Is2D = params.fireSoundIs2D;

	attackDelay.AddDelay(params.switchDelayTime - 0.1f);
	SwitchDelay.AddDelay(params.switchDelayTime);

	Update();
	AsyncUpdate();
	LateUpdate();
}

void WeaponFirearm::SetData(WeaponSlotData data)
{
	// magazineAmmo < 0 is the "never persisted a magazine count for this
	// instance" sentinel (see WeaponSlotData) - a genuinely fresh pickup.
	// Otherwise this is a previously-owned instance being re-equipped (switched
	// back to, or restored from a save) and its persisted count is restored.
	int pool = owner != nullptr ? owner->GetAmmo(params.ammoType) : 0;

	if (reloadConsumesAmmo)
	{
		// Clip model: the magazine owns its rounds, so it must NOT be clamped to
		// the pool on re-equip (the pool can be 0 while the magazine is full -
		// clamping would silently delete the loaded rounds every weapon switch).
		if (data.magazineAmmo < 0)
		{
			// Fresh weapon is loaded FROM the pool, not conjured on top of it.
			// (If pickups should start loaded for free, drop the ConsumeAmmo.)
			int load = std::min(params.magazineSize, pool);
			if (owner != nullptr && load > 0)
				owner->ConsumeAmmo(params.ammoType, load);
			data.magazineAmmo = load;
		}
		else
			data.magazineAmmo = std::min(data.magazineAmmo, params.magazineSize);
	}
	else
	{
		// Visual-reload model: the magazine is just a view of the pool, so it can
		// never exceed it (other weapons may have used the pool in the meantime).
		if (data.magazineAmmo < 0)
			data.magazineAmmo = std::min(params.magazineSize, pool);
		else
			data.magazineAmmo = std::min({ data.magazineAmmo, params.magazineSize, pool });
	}

	data.magazineAmmo = std::max(0, data.magazineAmmo);
	reloading = false;
	reloadBuffer = 0.0f;

	Weapon::SetData(data);
}

void WeaponFirearm::LoadAssets()
{

	Weapon::LoadAssets();

	SoundManager::LoadBankFromPath("GameData/sounds/banks/Desktop/Weapons.bank");
	SoundManager::LoadBankFromPath("GameData/sounds/banks/Desktop/SFX.bank");

	if (thirdPersonAnimator == nullptr)
		thirdPersonAnimator = make_unique<WeaponAnimator>();
	thirdPersonAnimator->LoadAssetsIfNeeded();

	PreloadEntityType(params.bulletClass);

	// RIGHT HAND
	viewmodel = new SkeletalMesh(owner);
	arms = new SkeletalMesh(owner);
	viewmodel->LoadFromFile(params.modelPath);
	if (!params.texturesLocation.empty())
		viewmodel->TexturesLocation = params.texturesLocation;
	viewmodel->PlayAnimation(params.drawAnimation);
	//viewmodel->PreloadAssets();
	viewmodel->Transparent = true;
	viewmodel->IsViewmodel = true;
	Drawables.push_back(viewmodel);

	arms->LoadFromFile(ArmsModelPath);
	arms->IsViewmodel = true;
	Drawables.push_back(arms);

	// LEFT HAND (always loaded)
	viewmodelLeft = new SkeletalMesh(this);
	armsLeft = new SkeletalMesh(this);

	if (params.modelPath.empty() == false)
		viewmodelLeft->LoadFromFile(params.modelPath);

	if (!params.texturesLocation.empty())
		viewmodelLeft->TexturesLocation = params.texturesLocation;
	viewmodelLeft->PlayAnimation(params.drawAnimation);
	viewmodelLeft->PreloadAssets();
	viewmodelLeft->Transparent = true;
	viewmodelLeft->IsViewmodel = true;
	Drawables.push_back(viewmodelLeft);

	armsLeft->LoadFromFile(ArmsModelPath);
	armsLeft->IsViewmodel = true;
	Drawables.push_back(armsLeft);

	viewmodelLeft->Scale = vec3(-1, 1, 1);
	armsLeft->Scale = vec3(-1, 1, 1);
	viewmodelLeft->TwoSided = true;
	armsLeft->TwoSided = true;

	// Third person model
	Drawables.push_back(thirdPersonModel = new SkeletalMesh(owner));

	if (params.modelPathTp.empty() == false)
		thirdPersonModel->LoadFromFile(params.modelPathTp);

	thirdPersonModel->TexturesLocation = params.texturesLocationTp;

	PreloadEntityType("bullet");

	// Initialize left models as hidden if akimbo off
	viewmodelLeft->Visible = false;
	armsLeft->Visible = false;
	akimboPrev = false;

	viewmodel->GravityAlignedRotation = true;
	arms->GravityAlignedRotation = true;
	viewmodelLeft->GravityAlignedRotation = true;
	armsLeft->GravityAlignedRotation = true;
	thirdPersonModel->GravityAlignedRotation = true;

	viewmodel->MeshCustomShaderParams["rim_pow"] = vec4(6);
	viewmodel->MeshCustomShaderParams["specular_pow"] = vec4(8);
	viewmodel->MeshCustomShaderParams["specular_scale"] = vec4(0.2);

}

void WeaponFirearm::SetAkimbo(bool enabled)
{
	akimbo = enabled;

	if (akimbo && !akimboPrev)
	{
		viewmodelLeft->Visible = true;
		armsLeft->Visible = true;
		viewmodelLeft->PlayAnimation(params.drawAnimation, false, 0);
	}
	if (!akimbo && akimboPrev)
	{
		viewmodelLeft->Visible = false;
		armsLeft->Visible = false;
	}

	akimboPrev = akimbo;
}

void WeaponFirearm::StopTrail(ParticleSystem* trail)
{

	if (trail == nullptr) return;

	for (auto em : trail->emitters)
	{
		em->emitterTime = 1.2f * WEAPONSMOKE_DURATION_FACTOR;
	}

	trail->StopAll();
	trail->DestroyWithDelay(3.0f * WEAPONSMOKE_DURATION_FACTOR);
}

void WeaponFirearm::Update()
{
	Weapon::Update();

	// When the magazine is just a capped view into the shared pool (rather than
	// reload having actually moved ammo out of it), it can go stale: another
	// weapon of the same ammo type may have fired and dropped the pool below
	// what this magazine currently shows. Keep it honest every frame. Not done
	// when reloadConsumesAmmo is true, since in that mode the magazine's ammo
	// has genuinely been removed from the pool and is this weapon's own.
	if (!reloadConsumesAmmo && owner != nullptr)
		Data.magazineAmmo = std::min(Data.magazineAmmo, owner->GetAmmo(params.ammoType));

	if (akimbo)
	{
		akimboDistanceProgress += Time::DeltaTimeF * 4.0f;
	}
	else
	{
		akimboDistanceProgress -= Time::DeltaTimeF * 4.0f;
	}

	akimboDistanceProgress = std::clamp(akimboDistanceProgress, 0.0f, 1.0f);

	// Detect akimbo change
	if (akimbo != akimboPrev)
		SetAkimbo(akimbo);

	weaponAim -= Time::DeltaTimeF * 1.5f;
	if (!CanAttack() && weaponAim > 1)
		weaponAim = 1.0f;

	oldWeaponAim = weaponAim;

	if (params.hasRecoilModelOffset) {
		if (attackDelay.Wait())
			recoilModelOffset = MathHelper::Interp(recoilModelOffset, params.recoilModelTarget, Time::DeltaTimeF, params.recoilModelInterpIn);
		else
			recoilModelOffset = MathHelper::Interp(recoilModelOffset, 0.0f, Time::DeltaTimeF, params.recoilModelInterpOut);
	}

	if (params.hasActiveSpread) {
		if (!attackDelay.Wait())
			activeSpread -= Time::DeltaTimeF * params.spreadDecreaseSpeed;
		activeSpread = std::clamp(activeSpread, 0.0f, params.maxActiveSpread);
		Spread = params.baseSpread + activeSpread + length(Player::Instance->controller.GetVelocity()) / params.velocitySpreadDivisor;
	}
	else
		Spread = params.baseSpread;

	bool firstPerson = owner != nullptr && owner->InThirdPerson() == false;
	if (!firstPerson)
		Spread *= 0.5f;

	// Snapshot before anything below can start a reload (the key, or the
	// dry-fire auto-reload inside PerformAttack) - see the completion check.
	const bool wasReloadingAtFrameStart = reloading;

	if (Input::GetAction("attack")->Holding() && CanAttack() && !attackDelay.Wait())
		PerformAttack();

	// Buffer the key press: Pressed() is a single-frame event, and CanReload()
	// can be false on exactly that frame (e.g. CanAttack() is still blocked by
	// the delay a shot just added), which used to swallow the press entirely.
	if (Input::GetAction("reload")->Pressed())
		reloadBuffer = 0.3f;

	if (reloadBuffer > 0.0f)
	{
		reloadBuffer -= Time::DeltaTimeF;

		if (GetRoundsToLoad() <= 0)
			reloadBuffer = 0.0f; // nothing to reload, don't keep a stale request
		else if (CanReload())
		{
			StartReload();
		}
	}

	// Skipped on the frame the reload starts: the animation duration/time read
	// below can still belong to the previous clip (fire/draw) on that frame,
	// which made the reload "complete" instantly or against the wrong clip.
	if (reloading && wasReloadingAtFrameStart)
	{
		// Detect completion by animation time remaining rather than a fixed
		// duration, so reload speed always matches whatever clip is playing.
		float remaining = viewmodel->GetAnimationDuration() - viewmodel->GetAnimationTime();

		if (remaining <= 0.2f)
		{
			// Evaluated NOW, not when the reload began - the pool may have changed
			// during the animation (pickup, another weapon firing).
			int load = GetRoundsToLoad();

			if (load > 0)
			{
				// Clip model: rounds physically leave the pool.
				// Visual model: nothing leaves the pool, the view just refills.
				if (reloadConsumesAmmo)
					owner->ConsumeAmmo(params.ammoType, load);

				Data.magazineAmmo += load;
			}

			reloading = false;
		}
	}
}

void WeaponFirearm::PerformAttack()
{

	if (reloading)
	{
		return;
	}

	// Out of rounds in the magazine. Under BOTH ammo models an empty magazine
	// means "reload first". Start one automatically if it can actually do
	// something (and silently do nothing if the pool is empty too).
	if (Data.magazineAmmo <= 0)
	{
		if (CanReload())
			StartReload();
		return;
	}

	// Akimbo: the RIGHT (primary) gun owns the magazine and decides when the
	// weapon needs a reload - an empty magazine blocks BOTH hands (checked above)
	// and the reload animation plays on both. The LEFT gun has no magazine of its
	// own: it draws straight from the player's ammo pool and never reloads.
	// The turn is handed to the other hand before the left-hand ammo gate below,
	// so a blocked left shot doesn't leave the weapon retrying the same hand.
	bool fireLeft = akimbo && alternateFire && fireLeftNext;
	fireLeftNext = !fireLeftNext;

	if (fireLeft && owner->GetAmmo(params.ammoType) <= 0)
		return;

	if (params.hasActiveSpread)
		activeSpread += params.spreadIncreasePerShot;

	if (params.notifyNpcs)
		NotifyNpcs();

	if (params.activateViolenceCrime)
		Player::Instance->violanceCrimeActiveDelay.AddDelay(params.violenceCrimeDelay);

	if (params.useOneshotSound)
		SoundPlayer::PlayOneshot(params.fireSoundEvent, params.pitchModifier, params.fireVolume);
	else
	{
		fireSoundPlayer->Pitch = params.pitchModifier;
		fireSoundPlayer->Play();
	}

	bool firstperson = owner != nullptr && owner->ThirdPersonView == false;

	float shakeMultiplier = 1.0f;

	if (firstperson)
	{

	}
	else
	{
		shakeMultiplier = 0.3f;
	}

	if (params.hasRandomRecoilStrength) {
		float horizontalRecoilStrength = RandomHelper::RandomFloat() * 2 - 1;
		float verticalRecoilStrength = RandomHelper::RandomFloat() * 0.5f + 0.5f;
		CameraShake modifiedShake = params.recoilShake;
		modifiedShake.rotationAmplitude *= vec3(verticalRecoilStrength, horizontalRecoilStrength, 1);
		modifiedShake.rotationAmplitude *= shakeMultiplier;
		Camera::AddCameraShake(modifiedShake);
	}
	else {

		auto shake = params.recoilShake;

		shake.rotationAmplitude *= shakeMultiplier;

		Camera::AddCameraShake(shake);
	}

	SwitchDelay.AddDelay(params.switchDelayOnAttack * (akimbo ? 0.5f : 1));
	attackDelay.AddDelay(params.attackDelayTime * (akimbo ? 0.5f : 1.0f));

	if (akimbo)
	{
		if (fireLeft)
			viewmodelLeft->PlayAnimation(params.fireAnimation, false, params.fireAnimInterpInTime);
		else
			viewmodel->PlayAnimation(params.fireAnimation, false, params.fireAnimInterpInTime);
	}
	else
		viewmodel->PlayAnimation(params.fireAnimation, false, params.fireAnimInterpInTime);

	// muzzle selection
	mat4 boneMat = (akimbo && fireLeft ? viewmodelLeft : viewmodel)->GetBoneMatrixWorld(params.boneMuzzle);
	vec3 startLoc = MathHelper::DecomposeMatrix(boneMat).Position;
	startLoc = mix(startLoc, Camera::position, params.muzzleMix) - MathHelper::GetForwardVector(Camera::finalizedRotation) * params.muzzleForwardOffset;

	WeaponFireFlash::CreateAt(startLoc);

	int c = 0;

	// spread shooting
	if (params.spreadType == "grid")
	{
		for (float y = -params.gridSpreadSize; y <= params.gridSpreadSize; y += params.gridStep) {
			for (float x = -params.gridSpreadSize; x <= params.gridSpreadSize; x += params.gridStep) {
				if (length(vec2(x, y)) > params.gridMaxLength) continue;
				c++;
				FireSingleBullet(startLoc, vec4(x, y, 0, 1));
			}
		}
	}
	else {
		for (int i = 0; i < params.bulletsPerShot; ++i)
			FireSingleBullet(startLoc, vec4(0));
	}

	if (fireLeft)
	{
		// Left gun: always straight from the pool, in both ammo models. It never
		// touches Data.magazineAmmo. (In visual-reload mode the magazine is a
		// capped view of the pool, so Update() shrinks it if this drops the pool
		// below what it shows - no manual adjustment needed here.)
		owner->ConsumeAmmo(GetAmmoType(), 1);
	}
	else
	{
		Data.magazineAmmo = std::max(0, Data.magazineAmmo - 1);

		// Visual-reload model: the pool is the real ammo, so the shot comes out
		// of it directly. Clip model: the round already left the pool when the
		// magazine was loaded - charging it again would double-bill the player.
		if (!reloadConsumesAmmo)
			owner->ConsumeAmmo(GetAmmoType(), 1);
	}

	if (fireLeft)
	{

		if (smokeTrailL == nullptr || smokeTrailL->emitters.empty() || smokeTrailL->emitters[0]->emitterTime > 1.0f * WEAPONSMOKE_DURATION_FACTOR)
		{
			smokeTrailL = static_cast<ParticleSystem*>(Spawn("weapon_smoke"));
			SnapTrailPositions();
			smokeTrailL->Start();

		}

		stopEmittingSmokeDelayL.AddDelay(1.0f * WEAPONSMOKE_DURATION_FACTOR);
		smokeTrailL->emitters[0]->emitterTime = 0;

	}
	else
	{

		if (smokeTrail == nullptr || smokeTrail->emitters.empty() || smokeTrail->emitters[0]->emitterTime > 1.0f * WEAPONSMOKE_DURATION_FACTOR)
		{
			smokeTrail = static_cast<ParticleSystem*>(Spawn("weapon_smoke"));
			SnapTrailPositions();
			smokeTrail->Start();

		}

		stopEmittingSmokeDelay.AddDelay(1.0f * WEAPONSMOKE_DURATION_FACTOR);
		smokeTrail->emitters[0]->emitterTime = 0;

	}


}

int WeaponFirearm::GetRoundsToLoad()
{
	if (owner == nullptr)
		return 0;

	int pool = owner->GetAmmo(params.ammoType);
	int space = params.magazineSize - Data.magazineAmmo;

	if (space <= 0 || pool <= 0)
		return 0;

	// Clip model: the pool is pure reserve, so take as much as fits, limited by
	// what's there. (The old code compared the magazine against
	// min(magazineSize, pool) as if the pool were the magazine's total, so with
	// e.g. 7/8 loaded and 3 in reserve it computed a target of 3, decided the
	// magazine was already "full", and refused to reload.)
	if (reloadConsumesAmmo)
		return std::min(space, pool);

	// Visual model: the magazine is a view of the pool, capped by it.
	return std::max(0, std::min(params.magazineSize, pool) - Data.magazineAmmo);
}

bool WeaponFirearm::CanReload()
{
	if (reloading)
		return false;

	if (owner == nullptr)
		return false;

	if (!CanAttack())
		return false;

	// Nothing a reload could add under the active ammo model.
	if (GetRoundsToLoad() <= 0)
		return false;

	// Can't reload while the offhand weapon is mid-action (e.g. the cane swinging).
	if (owner->currentOffhandWeapon != nullptr && owner->currentOffhandWeapon->UsesLeftHand())
		return false;

	return true;
}

void WeaponFirearm::StartReload()
{
	reloading = true;
	reloadBuffer = 0.0f;

	viewmodel->PlayAnimation(params.reloadAnimation, false, params.fireAnimInterpInTime);

	if (viewmodelLeft != nullptr)
		viewmodelLeft->PlayAnimation(params.reloadAnimation, false, params.fireAnimInterpInTime);
}

void WeaponFirearm::FireSingleBullet(const vec3& startLoc, const vec4& gridOffset)
{
	Bullet* bullet = static_cast<Bullet*>(LevelObjectFactory::instance().create(params.bulletClass));

	bullet->debuffOnHit = params.debuffOnHit;
	bullet->debuffStacks = params.debuffStacksOnHit;

	vec3 offset;
	if (params.spreadType == "grid")
		offset = MathHelper::GetRotationMatrix(Camera::finalizedRotation) * gridOffset;
	else
		offset = RandomHelper::RandomPosition(1) * Spread;

	vec3 endLoc;
	bool firstperson = owner != nullptr && owner->InThirdPerson() == false;

	if (firstperson)
		endLoc = Position + MathHelper::GetForwardVector(Camera::finalizedRotation) * params.range + offset;
	else {
		endLoc = Camera::position + MathHelper::GetForwardVector(Camera::finalizedRotation) * (params.range + 3);
		auto hit = Physics::LineTrace(Camera::finalizedPosition, endLoc, BodyType::GroupHitTest, {}, { owner });
		if (hit.hasHit) endLoc = hit.shapePosition;
		endLoc += offset * hit.fraction;
	}

	bullet->damageCauser = owner;
	bullet->Speed = params.bulletSpeed;
	bullet->Position = startLoc + offset * 0.002f;
	bullet->Rotation = MathHelper::FindLookAtRotation(startLoc, endLoc);
	Level::Current->AddEntity(bullet);
	StealthGore::TagProjectile(bullet, this);
	bullet->Start();
	bullet->LoadAssetsIfNeeded();
	bullet->Damage = params.bulletDamage;

	if (Player::Instance->powerUpManager.IsPowerUpActive(PowerUpManager::PowerUpType::TripleDamage))
	{
		bullet->Damage *= 3;
	}

}

void WeaponFirearm::NotifyNpcs()
{

	AiPerceptionSystem::EmitSoundAt(owner->Position, params.npcNotifyRadius, (int)InvestigationReason::WeaponFire, Player::Instance->Id);

	auto observers = AiPerceptionSystem::GetObserversInRadius(owner->Position, params.npcNotifyRadius);
	for (auto observer : observers) {
		auto ownerNpc = dynamic_cast<NpcBase*>(Level::Current->FindEntityWithId(observer->owner));
		if (ownerNpc)
		{
			ownerNpc->TryStartInvestigation(InvestigationReason::Noise, owner->Position, Player::Instance->Id);
		}
	}
}

void WeaponFirearm::AsyncUpdate()
{

	// max (not ||): HideWeapon is now a continuous 0..1 blend for two-handed
	// weapons handing off to an offhand weapon, and this needs to preserve the
	// fractional value rather than collapsing it to a bool. Keeps the existing
	// "stay hidden one extra frame" anti-flicker behaviour for everything else,
	// since it's just max(current, previous) instead of OR(current, previous).
	float hide = std::max(HideWeapon, lastFrameHide);

	lastFrameHide = HideWeapon;

	if (akimbo)
	{
		hide = 1;
	}

	// RIGHT GUN
	viewmodel->Update();
	auto pose = viewmodel->GetAnimationPose();
	auto leftHandPose = pose.GetBoneTransform("clavicle_l");
	leftHandPose.Rotation += vec3(30, 0, 0) * hide;
	pose.SetBoneTransformEuler("clavicle_l", leftHandPose);
	arms->PasteAnimationPose(pose);

	// LEFT GUN
	if (akimbo)
	{
		viewmodelLeft->Update();
		auto poseL = viewmodelLeft->GetAnimationPose();
		auto leftHandPose = poseL.GetBoneTransform("clavicle_l");
		leftHandPose.Rotation += vec3(35, 0, 0) * hide;
		poseL.SetBoneTransformEuler("clavicle_l", leftHandPose);
		armsLeft->PasteAnimationPose(poseL);
	}


}



void WeaponFirearm::LateUpdate()
{

	vec3 offset = params.weaponOffset;

	offset.x += akimboDistanceProgress * -0.020f;

	viewmodel->Position = Position + (mat3)Camera::GetRotationMatrix() * offset;
	viewmodel->Rotation = Rotation;
	if (params.hasRecoilModelOffset) viewmodel->Rotation.x += recoilModelOffset;
	viewmodel->Visible = owner != nullptr && owner->InThirdPerson() == false;
	arms->Visible = viewmodel->Visible;
	arms->Position = viewmodel->Position;
	arms->Rotation = viewmodel->Rotation;

	thirdPersonModel->Visible = !viewmodel->Visible;
	thirdPersonModel->Position = owner->bodyMesh->Position;
	thirdPersonModel->Rotation = owner->bodyMesh->Rotation;

	if (akimbo)
	{

		viewmodelLeft->Visible = viewmodel->Visible;
		armsLeft->Visible = arms->Visible;
		viewmodelLeft->Position = Position + (mat3)Camera::GetRotationMatrix() * (offset * vec3(-1, 1, 1));
		viewmodelLeft->Rotation = Rotation;
		if (params.hasRecoilModelOffset) viewmodelLeft->Rotation.x += recoilModelOffset;
		armsLeft->Position = viewmodelLeft->Position;
		armsLeft->Rotation = viewmodelLeft->Rotation;
	}

	if (smokeTrail)
	{
		mat4 boneMat = (viewmodel)->GetBoneMatrixWorld("weapon_fire_point");
		vec3 startLoc = MathHelper::DecomposeMatrix(boneMat).Position;

		smokeTrail->Position = startLoc;
		smokeTrail->emitters[0]->SnapLastParticleToEmitterPosition();

	}

	if (smokeTrailL)
	{
		mat4 boneMat = (viewmodelLeft)->GetBoneMatrixWorld("weapon_fire_point");
		vec3 startLoc = MathHelper::DecomposeMatrix(boneMat).Position;

		smokeTrailL->Position = startLoc;
		smokeTrailL->emitters[0]->SnapLastParticleToEmitterPosition();

	}

	Visible == viewmodel->AllAssetsLoaded() && viewmodelLeft->AllAssetsLoaded() && arms->AllAssetsLoaded() && armsLeft->AllAssetsLoaded();

}

WeaponSlotData WeaponFirearm::GetDefaultData() {
	WeaponSlotData data;
	data.className = "firearm";
	data.slot = 0;
	return data;
}

void WeaponFirearm::Destroy()
{

	Weapon::Destroy();
	StopTrail(smokeTrail);
	StopTrail(smokeTrailL);
}

AnimationPose WeaponFirearm::ApplyWeaponAnimation(AnimationPose thirdPersonPose)
{
	thirdPersonAnimator->weaponAim = std::clamp(weaponAim, 0.0f, 1.0f);
	thirdPersonAnimator->inPose = thirdPersonPose;
	thirdPersonAnimator->Update();
	lastAppliedPose = thirdPersonAnimator->GetResultPose();
	thirdPersonModel->PasteAnimationPose(lastAppliedPose);
	return lastAppliedPose;
}

void WeaponFirearm::Serialize(json& target)
{
	SERIALIZE_FIELD(target, attackDelay);
	SERIALIZE_FIELD(target, SwitchDelay);
	SERIALIZE_FIELD(target, activeSpread);
	SERIALIZE_FIELD(target, recoilModelOffset);
	SERIALIZE_FIELD(target, weaponAim);
	SERIALIZE_FIELD(target, fireLeftNext);
	// Not SERIALIZE_FIELD: magazineAmmo now lives on Data (Data.magazineAmmo),
	// not a same-named member, so the macro's implicit field-name lookup can't
	// reach it.
	target["magazineAmmo"] = Data.magazineAmmo;
	SERIALIZE_FIELD(target, reloading);

	auto viewmodelData = viewmodel->GetAnimationState();
	SERIALIZE_FIELD(target, viewmodelData);
	auto viewmodelLeftData = viewmodelLeft->GetAnimationState();
	SERIALIZE_FIELD(target, viewmodelLeftData);

}

void WeaponFirearm::Deserialize(json& source)
{
	DESERIALIZE_FIELD(source, attackDelay);
	DESERIALIZE_FIELD(source, SwitchDelay);
	DESERIALIZE_FIELD(source, activeSpread);
	DESERIALIZE_FIELD(source, recoilModelOffset);
	DESERIALIZE_FIELD(source, weaponAim);
	DESERIALIZE_FIELD(source, fireLeftNext);
	if (source.contains("magazineAmmo"))
		Data.magazineAmmo = source["magazineAmmo"].get<int>();
	DESERIALIZE_FIELD(source, reloading);

	AnimationState viewmodelData;
	DESERIALIZE_FIELD(source, viewmodelData);
	viewmodel->SetAnimationState(viewmodelData);

	AnimationState viewmodelLeftData;
	DESERIALIZE_FIELD(source, viewmodelLeftData);
	viewmodelLeft->SetAnimationState(viewmodelLeftData);

}

bool WeaponFirearm::IsInUltimateAkimbo()
{
	return false;
}

void WeaponFirearm::UpdateDebugUI()
{

	ImGui::Begin((std::string("WeaponFirearm:") + ClassName).c_str());

	ImGui::DragFloat3("offset", &params.weaponOffset.x, 0.002f, -1.0f, 1.0f);

	ImGui::End();
}

void WeaponFirearm::SnapTrailPositions()
{

	if (smokeTrail)
	{
		mat4 boneMat = (viewmodel)->GetBoneMatrixWorld("weapon_fire_point");
		vec3 startLoc = MathHelper::DecomposeMatrix(boneMat).Position;

		smokeTrail->Position = startLoc;
		smokeTrail->emitters[0]->SnapLastParticleToEmitterPosition();

	}

	if (smokeTrailL)
	{
		mat4 boneMat = (viewmodelLeft)->GetBoneMatrixWorld("weapon_fire_point");
		vec3 startLoc = MathHelper::DecomposeMatrix(boneMat).Position;

		smokeTrailL->Position = startLoc;
		smokeTrailL->emitters[0]->SnapLastParticleToEmitterPosition();

	}


}