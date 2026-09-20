// weapon_sniper.cpp (modified)
#include "WeaponFirearm.h"
#include "Animators/Animator_Rifle.h"
#include <algorithm>

class weapon_sniper : public WeaponFirearm {
public:
    bool Silencer = false;

    vec3 weaponAimOffset = vec3(0.062707, 0.033706, -0.0f);

    weapon_sniper() : WeaponFirearm() 
    {
		params.ammoType = WeaponAmmoType::CannonBullets;
        params.modelPath = "GameData/models/player/weapons/sniper/sniper.glb";
        params.texturesLocation = "GameData/models/player/weapons/sniper/sniper.glb/";
        params.modelPathTp = "GameData/models/player/weapons/sniper/sniper_tp.glb";
        params.texturesLocationTp = "GameData/models/player/weapons/sniper/sniper_tp.glb/";
        params.fireSoundEvent = "event:/Weapons/pistol/pistol_fire";
        params.useOneshotSound = false;
        params.pitchModifier = 1.0f; // Modified in Update based on Silencer
        params.baseSpread = 0.0f;
        params.hasActiveSpread = false;
        params.attackDelayTime = 1.3f;
        params.switchDelayTime = 0.35f;
        params.switchDelayOnAttack = 0.5f;
        params.weaponOffset = vec3(0.0, 0.00, -0.0);
        params.bulletSpeed = 200.0f;
        params.bulletDamage = 70.0f;
        params.range = 80.0f;
        params.fireAnimation = "fire";
        params.fireAnimInterpInTime = 0.08f;
        params.notifyNpcs = true;
        params.activateViolenceCrime = true;
        params.hasRecoilModelOffset = false;
        params.hasRandomRecoilStrength = false;
        params.recoilShake = CameraShake(
            0.13f,                             // interpIn
            0.5f,                              // duration
            vec3(0.0f, 0.0f, -0.1f),           // positionAmplitude
            vec3(0.0f, 0.0f, 3.4f),            // positionFrequency
            vec3(-4, 0.15f, 0.0f),             // rotationAmplitude
            vec3(-2.0f, 18.8f, 0.0f),          // rotationFrequency
            0.5f,                              // falloff
            CameraShake::ShakeType::SingleWave // shakeType
        );
        bobScale = 0.0f;

        // Sniper's aim (attack2 hold, inherited from WeaponFirearm) is a
        // real scope zoom rather than the modest ADS every other gun gets -
        // slower to come up, and a much tighter FOV at full aimProgress.
        params.aimSpeed = 3.0f;
        params.restFOV = 65.0f;
        params.aimFOV = 10.0f;

        thirdPersonAnimator = make_unique<Animator_Rifle>(this);

        // Readying (LMB fires from rest, see WeaponFirearm::Update) speed -
        // the rifle raises to the hip-ready position at this rate.
        DrawTime = 0.2f;
        HideTime = 0.4f;

        // Rifle-styled hidden/carry pose: lowered, barrel forward-down.
        HiddenPosePosition = vec3(0.080, -0.470, 0.140);
        HiddenPoseRotation = vec3(65.0f, 8.0f, -4.0f);
        HiddenPoseRotationPoint = vec3(-0.03f, -0.14f, 0.55f);

    }

    void Update() override {

        params.pitchModifier = Silencer ? 2.8f : 1.0f;

        // Computes DrawProgress (presented, from NotifyUsed()/firing),
        // aimProgress (scope zoom, from holding attack2, using this
        // weapon's own aimSpeed/restFOV/aimFOV set above), and handles
        // firing/spread/Camera::FOV - all shared with every other firearm
        // now. Everything below just layers the sniper's extra scope
        // flourishes on top of that same inherited aimProgress.
        WeaponFirearm::Update();

        bobScale = 1.0f - aimProgress;

        params.weaponOffset = mix(vec3(0.0, 0.00, 0.0), weaponAimOffset, aimProgress);

        ForceFirstPerson = aimProgress > 0.9;

        // Keep the scope from being yanked away mid-zoom by melee/tool
        // interrupts - same reasoning as before, just riding CanChangeSlot()
        // instead of a dedicated toggle.
        if (aimProgress > 0.01f) SwitchDelay.AddDelay(0.1f);
    }

    void UpdateDebugUI() override {
        ImGui::Begin("Sniper Weapon Debug");
        ImGui::Checkbox("Silencer", &Silencer);
        ImGui::DragFloat3("Weapon Offset", &params.weaponOffset.x, 0.01f);
        ImGui::End();
    }

    void LateUpdate() override {
        params.weaponOffset.z = mix(0.0f, 0.02f, aimProgress);

        WeaponFirearm::LateUpdate();

        viewmodel->Scale.z = mix(1.0f, 0.02f, aimProgress);
        arms->Scale = viewmodel->Scale;

        viewmodel->MeshHideList.clear();
        if (Silencer) {
            viewmodel->MeshHideList.insert("muzzle");
        }
        else {
            viewmodel->MeshHideList.insert("silencer");
        }
    }

    void Serialize(json& target) override
    {
        WeaponFirearm::Serialize(target);
        SERIALIZE_FIELD(target, Silencer);
    }

    void Deserialize(json& source) override
    {
        WeaponFirearm::Deserialize(source);
        DESERIALIZE_FIELD(source, Silencer);
    }

    WeaponSlotData GetDefaultData() override {
        WeaponSlotData data = WeaponFirearm::GetDefaultData();
        data.className = "weapon_sniper";
        data.AmmoType = WeaponAmmoType::CannonBullets; // must match params.ammoType above, or GetAmmo()/ConsumeAmmo() check a pool this weapon never gets stocked into
        return data;
    }
};

REGISTER_ENTITY(weapon_sniper, "weapon_sniper")
