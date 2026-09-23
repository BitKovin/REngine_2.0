#include "WeaponFirearm.h"
#include "Animators/Animator_Rifle.h"

class weapon_mpsd : public WeaponFirearm {
public:

    weapon_mpsd() : WeaponFirearm() {
        params.ammoType = WeaponAmmoType::PistolBullets;
        params.modelPath = "GameData/models/player/weapons/stg44/stg44.glb";
        params.fireSoundEvent = "event:/Weapons/pistol/pistol_fire";
        params.modelPathTp = "GameData/models/player/weapons/stg44/stg44_tp.glb";
        params.texturesLocationTp = "GameData/models/player/weapons/stg44/stg44_tp.glb/";
        params.useOneshotSound = true;
        params.pitchModifier = 1.2f;
        params.fireVolume = 0.4f;

        params.baseSpread = 0.02f;
        params.spreadIncreasePerShot = 0.01f;
        params.maxActiveSpread = 0.7f;
        params.velocitySpreadDivisor = 15.5f;
        params.attackDelayTime = 0.13f;
        params.switchDelayTime = 0.35f;
        params.switchDelayOnAttack = 0.09f;
        params.weaponOffset = vec3(0.0, 0.00, -0.0);
        params.bulletSpeed = 200.0f;
        params.bulletDamage = 15.0f;
        params.range = 50.0f;
        params.fireAnimation = "fire";
        params.drawAnimation = "reload";
        params.fireAnimInterpInTime = 0.08f;
        params.notifyNpcs = true;
        params.activateViolenceCrime = true;
        params.violenceCrimeDelay = 0.3f;
        params.hasRecoilModelOffset = true;
        params.recoilModelTarget = -2;
        params.recoilModelInterpIn = 2.0f;
        params.recoilModelInterpOut = 7.0f;
        params.hasRandomRecoilStrength = true;
        params.recoilShake = CameraShake(
            0.13f,
            0.5f,
            vec3(0.0f, 0.0f, -0.1f),
            vec3(0.0f, 0.0f, 3.4f),
            vec3(-2, 0.6f, 0.0f),
            vec3(-1.0f, 6.f, 0.0f),
            0.5f,
            CameraShake::ShakeType::SingleWave
        );
        Illegal = true;

        thirdPersonAnimator = make_unique<Animator_Rifle>(this);

        weaponHandlingType = 0;

        params.magazineSize = 34; // matches GetDefaultData().startAmmo below

        // This is the "two-handed but can still use an offhand weapon" example:
        // it needs both hands to hold normally (UsesLeftHand), so it only hands
        // its left hand off to an offhand weapon while that weapon is actively
        // mid-action (see Player::UpdateWeapon).
        SupportsOffhandWeapon = true;
    }

    bool UsesLeftHand() override
    {
        return true;
    }

    WeaponSlotData GetDefaultData() override {
        WeaponSlotData data = WeaponFirearm::GetDefaultData();
        data.className = "weapon_mpsd";
        data.slot = 3;
        data.AmmoType = WeaponAmmoType::PistolBullets;
        data.startAmmo = 34;
        return data;
    }
};

REGISTER_ENTITY(weapon_mpsd, "weapon_mpsd")
