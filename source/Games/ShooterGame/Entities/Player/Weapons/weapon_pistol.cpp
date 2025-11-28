#include "WeaponFirearm.h"

class weapon_pistol : public WeaponFirearm {
public:
    bool Silencer = true;

    weapon_pistol() : WeaponFirearm() {
        params.modelPath = "GameData/models/player/weapons/pistol/glock.glb";
        params.texturesLocation = "GameData/models/player/weapons/pistol/glock.glb/";
        params.fireSoundEvent = "event:/Weapons/pistol/pistol_fire";
        params.useOneshotSound = false;
        params.pitchModifier = 1.0f; // Modified in Update based on Silencer
        params.baseSpread = 0.1f;
        params.spreadIncreasePerShot = 0.1f;
        params.maxActiveSpread = 1.2f;
        params.velocitySpreadDivisor = 7.0f;
        params.attackDelayTime = 0.3f;
        params.switchDelayTime = 0.35f;
        params.switchDelayOnAttack = 0.2f;
        params.weaponOffset = vec3(0.0, 0.00, -0.0);
        params.bulletSpeed = 200.0f;
        params.bulletDamage = 21.0f;
        params.range = 80.0f;
        params.fireAnimation = "fire";
        params.fireAnimInterpInTime = 0.08f;
        params.notifyNpcs = false;
        params.activateViolenceCrime = false;
        params.hasRecoilModelOffset = false;
        params.hasRandomRecoilStrength = false;
        params.recoilShake = CameraShake(
            0.13f,                             // interpIn
            0.5f,                              // duration
            vec3(0.0f, 0.0f, -0.1f),           // positionAmplitude
            vec3(0.0f, 0.0f, 3.4f),            // positionFrequency
            vec3(-3, 0.15f, 0.0f),             // rotationAmplitude
            vec3(-1.5f, 18.8f, 0.0f),          // rotationFrequency
            0.5f,                              // falloff
            CameraShake::ShakeType::SingleWave // shakeType
        );
    }

    void Update() override {
        params.pitchModifier = Silencer ? 2.8f : 1.0f;
        WeaponFirearm::Update();
    }

    void LateUpdate() override {
        WeaponFirearm::LateUpdate();
        viewmodel->MeshHideList.clear();
        if (Silencer) {
            viewmodel->MeshHideList.insert("muzzle");
        }
        else {
            viewmodel->MeshHideList.insert("silencer");
        }
    }

    WeaponSlotData GetDefaultData() override {
        WeaponSlotData data = WeaponFirearm::GetDefaultData();
        data.className = "weapon_pistol";
        return data;
    }
};

REGISTER_ENTITY(weapon_pistol, "weapon_pistol")