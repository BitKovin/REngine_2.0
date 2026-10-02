#include "WeaponFirearm.h"
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
	// instance" sentinel (see WeaponSlotData) - a genuinely fresh pickup, so
	// start full. Otherwise this is a previously-owned instance being
	// re-equipped (switched back to, or restored from a save) - restore
	// exactly what it had. Either way, clamp to the current shared pool: it
	// may have dropped since this instance was last equipped, since other
	// weapons of the same ammo type draw from the same pool.
	int pool = owner != nullptr ? owner->GetAmmo(params.ammoType) : 0;

	if (data.magazineAmmo < 0)
		data.magazineAmmo = std::min(params.magazineSize, pool);
	else
		data.magazineAmmo = std::min({ data.magazineAmmo, params.magazineSize, pool });

	reloading = false;

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

	if(params.modelPathTp.empty()== false)
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

	if (Input::GetAction("attack")->Holding() && CanAttack() && !attackDelay.Wait())
		PerformAttack();

	if (Input::GetAction("reload")->Pressed() && CanReload())
		StartReload();

	if (reloading)
	{
		// Detect completion by animation time remaining rather than a fixed
		// duration, so reload speed always matches whatever clip is playing.
		float remaining = viewmodel->GetAnimationDuration() - viewmodel->GetAnimationTime();

		if (remaining <= 0.2f)
		{
			if (owner != nullptr)
			{
				int target = std::min(params.magazineSize, owner->GetAmmo(params.ammoType));

				if (reloadConsumesAmmo)
				{
					// Actually move the rounds from the pool into the magazine.
					int amountToLoad = std::max(0, target - Data.magazineAmmo);
					owner->ConsumeAmmo(params.ammoType, amountToLoad);
					Data.magazineAmmo += amountToLoad;
				}
				else
				{
					// Just cap the shared view - nothing leaves the pool.
					Data.magazineAmmo = target;
				}
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

	// Computed - and the hands alternated - before the ammo gate below, so a
	// shot that gets blocked for lack of ammo still hands the turn to the
	// other hand next time, instead of getting stuck retrying the same empty
	// hand forever.
	bool fireLeft = akimbo && alternateFire && fireLeftNext;
	fireLeftNext = !fireLeftNext;

	// Whether it's the left or right hand's turn, the weapon overall still has
	// to actually be loaded to fire at all - otherwise, once the magazine hits
	// zero, "spare" below collapses to the entire remaining pool and the left
	// hand alone could keep the weapon firing forever, with the right hand
	// silently failing every other shot and reload never becoming necessary.
	if (Data.magazineAmmo <= 0)
	{
		return;
	}

	// Akimbo's left-hand gun doesn't have its own tracked magazine - it draws
	// straight from the shared ammo pool and never needs reloading. Only the
	// primary gun (right hand when akimbo, either hand otherwise) consumes
	// from Data.magazineAmmo, but both hands are still gated on it being
	// non-empty (see above) - the left hand is gated further still, below.
	if (fireLeft)
	{
		// "Spare" ammo: whatever's in the pool beyond what the primary magazine
		// already shows as loaded, so the left hand can't eat into rounds the
		// right hand is counting on. When reloadConsumesAmmo is true the pool is
		// already pure reserve (a reload physically moves rounds out of it into
		// the magazine), so none of that subtraction is needed; when false the
		// magazine is just a capped view into the same pool, so whatever it's
		// currently showing as loaded isn't actually free for the left hand too.
		int spareAmmo = owner->GetAmmo(params.ammoType) - (reloadConsumesAmmo ? 0 : Data.magazineAmmo);

		if (spareAmmo <= 0)
			return;
	}

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
		// Left hand never has its own magazine - it always draws straight from
		// the shared pool, regardless of reloadConsumesAmmo.
		owner->ConsumeAmmo(GetAmmoType(), 1);
	}
	else
	{
		Data.magazineAmmo = std::max(0, Data.magazineAmmo - 1);

		// Only take it out of the pool too if the magazine isn't already
		// physically separate from it - i.e. a reload didn't already move
		// these rounds out of the pool when it was loaded. Consuming from
		// both here would double-charge the player for the same rounds.
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

bool WeaponFirearm::CanReload()
{
	if (reloading)
		return false;

	if (owner == nullptr)
		return false;

	if (!CanAttack())
		return false;

	// Already as full as the magazine (or the shared pool) allows - nothing to gain.
	if (Data.magazineAmmo >= std::min(params.magazineSize, owner->GetAmmo(params.ammoType)))
		return false;

	if (owner->GetAmmo(params.ammoType) <= 0)
		return false;

	// Can't reload while the offhand weapon is mid-action (e.g. the cane swinging).
	if (owner->currentOffhandWeapon != nullptr && owner->currentOffhandWeapon->UsesLeftHand())
		return false;

	return true;
}

void WeaponFirearm::StartReload()
{
	reloading = true;

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

	offset.x += akimboDistanceProgress * -0.012f;

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
		viewmodelLeft->Position = Position + (mat3)Camera::GetRotationMatrix() * (offset * vec3(-1,1,1));
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
	thirdPersonAnimator->weaponAim = std::clamp(weaponAim,0.0f, 1.0f);
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
