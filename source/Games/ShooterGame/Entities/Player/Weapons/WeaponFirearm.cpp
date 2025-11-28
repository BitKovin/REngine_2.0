#include "WeaponFirearm.h"
#include <algorithm> // for std::clamp

WeaponFirearm::WeaponFirearm(const FirearmParams& initialParams)
    : params(initialParams)
{
    LateUpdateWhenPaused = params.lateUpdateWhenPaused;
}

void WeaponFirearm::Start() {
    fireSoundPlayer = SoundPlayer::Create(params.fireSoundEvent);
    fireSoundPlayer->Volume = params.fireVolume;
    fireSoundPlayer->Is2D = params.fireSoundIs2D;

    attackDelay.AddDelay(params.switchDelayTime - 0.1f);
    SwitchDelay.AddDelay(params.switchDelayTime);
}

void WeaponFirearm::LoadAssets() 
{
    SoundManager::LoadBankFromPath("GameData/sounds/banks/Desktop/Weapons.bank");
    SoundManager::LoadBankFromPath("GameData/sounds/banks/Desktop/SFX.bank");

    viewmodel = new SkeletalMesh(this);
    arms = new SkeletalMesh(this);

    viewmodel->LoadFromFile(params.modelPath);
    if (!params.texturesLocation.empty()) {
        viewmodel->TexturesLocation = params.texturesLocation;
    }
    viewmodel->PlayAnimation(params.drawAnimation);
    viewmodel->PreloadAssets();

    viewmodel->Transparent = true;
    viewmodel->IsViewmodel = true;
    Drawables.push_back(viewmodel);

    arms->LoadFromFile(ArmsModelPath);
    arms->IsViewmodel = true;
    Drawables.push_back(arms);


    Drawables.push_back(thirdPersonModel = new SkeletalMesh(this));

    thirdPersonModel->LoadFromFile(params.modelPathTp);
    thirdPersonModel->TexturesLocation = params.texturesLocationTp;
    thirdPersonModel->PlayAnimation("idle", true, 0.0f);

    PreloadEntityType("bullet");
}

void WeaponFirearm::Update() 
{
    Weapon::Update();


    weaponAim -= Time::DeltaTimeF;
    if (oldWeaponAim > 1.0f && weaponAim <= 1.0f)
    {
        thirdPersonModel->PlayAnimation("idle", true, 0.5f);
    }
    else if (oldWeaponAim <= 1.0f && weaponAim > 1.0f)
    {
        thirdPersonModel->PlayAnimation("aim", true, 0.1f);
    }
    oldWeaponAim = weaponAim;
    thirdPersonModel->Update();



    if (params.hasRecoilModelOffset) {
        if (attackDelay.Wait()) {
            recoilModelOffset = MathHelper::Interp(recoilModelOffset, params.recoilModelTarget, Time::DeltaTimeF, params.recoilModelInterpIn);
        }
        else {
            recoilModelOffset = MathHelper::Interp(recoilModelOffset, 0.0f, Time::DeltaTimeF, params.recoilModelInterpOut);
        }
    }

    if (params.hasActiveSpread) {
        if (attackDelay.Wait() == false) {
            activeSpread -= Time::DeltaTimeF * params.spreadDecreaseSpeed;
        }
        activeSpread = std::clamp(activeSpread, 0.0f, params.maxActiveSpread);
        Spread = params.baseSpread + activeSpread;
        Spread += length(Player::Instance->controller.GetVelocity()) / params.velocitySpreadDivisor;
    }
    else {
        Spread = params.baseSpread;
    }

    if (Input::GetAction("attack")->Holding() && CanAttack()) {
        if (attackDelay.Wait() == false) {
            PerformAttack();
        }
    }
}

void WeaponFirearm::PerformAttack() {
    if (params.hasActiveSpread) {
        activeSpread += params.spreadIncreasePerShot;
    }

    if (params.notifyNpcs) {
        NotifyNpcs();
    }

    if (params.activateViolenceCrime) {
        Player::Instance->violanceCrimeActiveDelay.AddDelay(params.violenceCrimeDelay);
    }

    if (params.useOneshotSound) {
        SoundPlayer::PlayOneshot(params.fireSoundEvent, params.pitchModifier, params.fireVolume);
    }
    else {
        fireSoundPlayer->Pitch = params.pitchModifier;
        fireSoundPlayer->Play();
    }

    SwitchDelay.AddDelay(params.switchDelayOnAttack);

    viewmodel->PlayAnimation(params.fireAnimation, false, params.fireAnimInterpInTime);

    if (params.hasRandomRecoilStrength) {
        float horizontalRecoilStrength = RandomHelper::RandomFloat() * 2 - 1;
        float verticalRecoilStrength = RandomHelper::RandomFloat() * 0.5f + 0.5f;
        CameraShake modifiedShake = params.recoilShake;
        modifiedShake.rotationAmplitude *= vec3(verticalRecoilStrength, horizontalRecoilStrength, 1);
        Camera::AddCameraShake(modifiedShake);
    }
    else {
        Camera::AddCameraShake(params.recoilShake);
    }

    mat4 boneMat = viewmodel->GetBoneMatrixWorld(params.boneMuzzle);
    vec3 startLoc = MathHelper::DecomposeMatrix(boneMat).Position;
    startLoc = mix(startLoc, Camera::position, params.muzzleMix) - Camera::Forward() * params.muzzleForwardOffset;

    if (params.spreadType == "grid") {
        // Shotgun-style grid
        for (float y = -params.gridSpreadSize; y <= params.gridSpreadSize; y += params.gridStep) {
            for (float x = -params.gridSpreadSize; x <= params.gridSpreadSize; x += params.gridStep) {
                if (length(vec2(x, y)) > params.gridMaxLength) continue;
                FireSingleBullet(startLoc, vec4(x, y, 0, 1));
            }
        }
    }
    else {
        // Random spread
        for (int i = 0; i < params.bulletsPerShot; ++i) {
            FireSingleBullet(startLoc, vec4(0));
        }
    }

    attackDelay.AddDelay(params.attackDelayTime);
}

void WeaponFirearm::FireSingleBullet(const vec3& startLoc, const vec4& gridOffset) 
{

    weaponAim = 2;

    Bullet* bullet = new Bullet();
    Level::Current->AddEntity(bullet);

    vec3 offset;
    if (params.spreadType == "grid") {
        offset = MathHelper::GetRotationMatrix(Rotation) * gridOffset;
    }
    else {
        offset = RandomHelper::RandomPosition(1) * Spread;
    }

    vec3 endLoc = Position + MathHelper::GetForwardVector(Rotation) * params.range + offset;
    bullet->Speed = params.bulletSpeed;
    bullet->Position = startLoc + offset * 0.002f;
    bullet->Rotation = MathHelper::FindLookAtRotation(startLoc, endLoc);
    bullet->Start();
    bullet->LoadAssetsIfNeeded();
    bullet->Damage = params.bulletDamage;

    WeaponFireFlash::CreateAt(bullet->Position);
}

void WeaponFirearm::NotifyNpcs() {
    auto observers = AiPerceptionSystem::GetObserversInRadius(Position, params.npcNotifyRadius);
    for (auto observer : observers) {
        auto ownerNpc = dynamic_cast<NpcBase*>(Level::Current->FindEntityWithId(observer->owner));
        ownerNpc->TryStartInvestigation(InvestigationReason::LoudNoise, Position, Player::Instance->Id);
    }
}

void WeaponFirearm::AsyncUpdate() {
    viewmodel->Update();
    auto pose = viewmodel->GetAnimationPose();
    auto leftHandPose = pose.GetBoneTransform("clavicle_l");
    leftHandPose.Rotation += vec3(50, 0, 0) * HideWeapon;
    pose.SetBoneTransformEuler("clavicle_l", leftHandPose);
    arms->PasteAnimationPose(pose);
}

void WeaponFirearm::LateUpdate() 
{


    viewmodel->Position = Position + (mat3)Camera::GetRotationMatrix() * params.weaponOffset;
    viewmodel->Rotation = Rotation;

    if (params.hasRecoilModelOffset) {
        viewmodel->Rotation.x += recoilModelOffset;
    }

    viewmodel->Visible = owner != nullptr && owner->ThirdPersonView == false;
    arms->Visible = viewmodel->Visible;

    arms->Position = viewmodel->Position;
    arms->Rotation = viewmodel->Rotation;

    thirdPersonModel->Visible = !viewmodel->Visible;
    thirdPersonModel->Position = owner->bodyMesh->Position;
    thirdPersonModel->Rotation = owner->bodyMesh->Rotation;
}

WeaponSlotData WeaponFirearm::GetDefaultData() {
    WeaponSlotData data;
    data.className = "firearm"; // Override in subclasses
    data.slot = 0; // Override in subclasses
    return data;
}
