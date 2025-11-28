#include "WeaponFirearm.h"

class weapon_pistol : public WeaponFirearm {
public:
    bool Silencer = true;

	SkeletalMesh* thirdPersonModel = nullptr;

	float weaponAim = 0.0f;
	float oldWeaponAim = 0.0f;

	AnimationPose lastAppliedPose;

    weapon_pistol() : WeaponFirearm() 
    {
        params.modelPath = "GameData/models/player/weapons/pistol/pistol.glb";
        params.texturesLocation = "GameData/models/player/weapons/pistol/pistol.glb/";
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

    void Update() override 
    {
        params.pitchModifier = Silencer ? 2.8f : 1.0f;
        WeaponFirearm::Update();

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
    }

    void AsyncUpdate() override 
    {
        WeaponFirearm::AsyncUpdate();

	}

    void LateUpdate() override 
    {
        WeaponFirearm::LateUpdate();
        viewmodel->MeshHideList.clear();
        if (Silencer) {
            viewmodel->MeshHideList.insert("muzzle");
        }
        else {
            viewmodel->MeshHideList.insert("silencer");
        }

        thirdPersonModel->Visible = !viewmodel->Visible;
        thirdPersonModel->Position = owner->bodyMesh->Position;
		thirdPersonModel->Rotation = owner->bodyMesh->Rotation;

    }

    void LoadAssets() override 
    {
        WeaponFirearm::LoadAssets();

		Drawables.push_back(thirdPersonModel = new SkeletalMesh(this));

		thirdPersonModel->LoadFromFile("GameData/models/player/weapons/pistol/pistol_tp.glb");
		thirdPersonModel->TexturesLocation = "GameData/models/player/weapons/pistol/pistol_tp.glb/";
		thirdPersonModel->PlayAnimation("idle", true, 0.0f);


	}

    WeaponSlotData GetDefaultData() override 
    {
        WeaponSlotData data = WeaponFirearm::GetDefaultData();
        data.className = "weapon_pistol";
        return data;
    }

    void FireSingleBullet(const vec3& startLoc, const vec4& gridOffset = vec4(0)) override 
    {
        weaponAim = 2;
        WeaponFirearm::FireSingleBullet(startLoc, gridOffset);
	}

    virtual AnimationPose ApplyWeaponAnimation(AnimationPose thirdPersonPose)
    {

        auto weaponPose = thirdPersonModel->GetAnimationPose();

        if (oldWeaponAim > 1)
        {
            weaponPose.boneTransforms["pelvis"] = MathHelper::GetRotationMatrix(vec3(Camera::rotation.x, 0, 0)) * weaponPose.boneTransforms["pelvis"];
        }
        

        auto appliedWeaponPose = AnimationPose::LayeredLerp(
            "spine_02",
            thirdPersonModel->GetRootNode(),
            thirdPersonPose,
            weaponPose,
            std::clamp(weaponAim, 0.0f, 1.0f),
			0.3f
		);

        appliedWeaponPose = AnimationPose::LayeredLerp(
            "spine_03",
            thirdPersonModel->GetRootNode(),
            appliedWeaponPose,
            weaponPose,
            std::clamp(weaponAim, 0.0f, 1.0f),
            0.3f
        );

        appliedWeaponPose = AnimationPose::LayeredLerp(
            "upperarm_l",
            thirdPersonModel->GetRootNode(),
            appliedWeaponPose,
            weaponPose,
            std::clamp(weaponAim, 0.0f, 1.0f),
			1.0f
        );

        appliedWeaponPose = AnimationPose::LayeredLerp(
            "upperarm_r",
            thirdPersonModel->GetRootNode(),
            appliedWeaponPose,
			weaponPose,
			std::clamp(weaponAim, 0.0f, 1.0f),
            1.0f
        );

		lastAppliedPose = appliedWeaponPose;

		thirdPersonModel->PasteAnimationPose(appliedWeaponPose);



        return appliedWeaponPose;
    }

};

REGISTER_ENTITY(weapon_pistol, "weapon_pistol")