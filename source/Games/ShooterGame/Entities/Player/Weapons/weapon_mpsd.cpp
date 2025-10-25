#include "WeaponBase.h"
#include "../Player.hpp"

#include <Animation.h>

#include "Projectiles/Bullet.h"
#include <SoundSystem/FmodEventInstance.h>
#include <RandomHelper.h>
#include "WeaponFireFlash.h"

class weapon_mpsd : public Weapon
{
public:

	SkeletalMesh* viewmodel;
	SkeletalMesh* arms;

	vec3 weaponOffset = vec3(0.0, 0.00, -0.0);

	Delay attackDelay;

	float recoilModelOffset = 0;

	weapon_mpsd()
	{

		LateUpdateWhenPaused = true;
	}

	SoundPlayer* fireSoundPlayer = nullptr;

	void Start()
	{

		fireSoundPlayer = SoundPlayer::Create("event:/Weapons/pistol/pistol_fire");
		fireSoundPlayer->Volume = 0.5f;
		fireSoundPlayer->Is2D = true;


		attackDelay.AddDelay(0.3);
		SwitchDelay.AddDelay(0.35);

	}

	void LoadAssets()
	{

		SoundManager::LoadBankFromPath("GameData/sounds/banks/Desktop/Weapons.bank");
		SoundManager::LoadBankFromPath("GameData/sounds/banks/Desktop/SFX.bank");

		viewmodel = new SkeletalMesh(this);
		arms = new SkeletalMesh(this);

		viewmodel->LoadFromFile("GameData/models/player/weapons/mpsd/mpsd.glb");
		viewmodel->PlayAnimation("reload");
		viewmodel->PreloadAssets();

		viewmodel->Transparent = true;

		viewmodel->IsViewmodel = true;

		Drawables.push_back(viewmodel);

		arms->LoadFromFile(ArmsModelPath);
		arms->IsViewmodel = true;
		Drawables.push_back(arms);

		PreloadEntityType("bullet");

	}

	void Update()
	{
		if (attackDelay.Wait())
		{
			recoilModelOffset = MathHelper::Interp(recoilModelOffset, -2.5f, Time::DeltaTimeF, 2);
		}
		else
		{
			recoilModelOffset = MathHelper::Interp(recoilModelOffset, 0.0f, Time::DeltaTimeF, 7);
		}


		if (Input::GetAction("attack")->Holding())
		{
			if (attackDelay.Wait() == false)
				PerformAttack();
		}
	}

	void PerformAttack()
	{

		//fireSoundPlayer->Play();

		SoundPlayer::PlayOneshot("event:/Weapons/pistol/pistol_fire", 1.2f, 0.4f);

		SwitchDelay.AddDelay(0.09f);

		viewmodel->PlayAnimation("fire", false, 0.08f);

		float horizontalRecoilStength = RandomHelper::RandomFloat() * 2 - 1;
		float verticalRecoilStength = RandomHelper::RandomFloat() * 0.5f + 0.5f;

		Camera::AddCameraShake(CameraShake(
			0.13f,                            // interpIn
			0.5f,                            // duration
			vec3(0.0f, 0.0f, -0.1f),         // positionAmplitude
			vec3(0.0f, 0.0f, 3.4f),          // positionFrequency
			vec3(-4, 1.15f, 0.0f) * vec3(verticalRecoilStength, horizontalRecoilStength, 1),        // rotationAmplitude
			vec3(-2.0f, 12.8f, 0.0f),        // rotationFrequency
			0.5f,                            // falloff
			CameraShake::ShakeType::SingleWave // shakeType
		));


		mat4 boneMat = viewmodel->GetBoneMatrixWorld("muzzle");

		vec3 startLoc = MathHelper::DecomposeMatrix(boneMat).Position;

		startLoc = mix(startLoc, Camera::position, 0.6f) - Camera::Forward() * 0.1f;



		Bullet* bullet = new Bullet();
		Level::Current->AddEntity(bullet);

		vec3 offset = RandomHelper::RandomPosition(0.2f);

		vec3 endLoc = Position + MathHelper::GetForwardVector(Camera::rotation) * 50.0f + vec3(offset);

		bullet->Speed = 200.f;
		bullet->Position = startLoc + vec3(offset) * 0.002f;
		bullet->Rotation = MathHelper::FindLookAtRotation(startLoc, endLoc);
		bullet->Start();
		bullet->LoadAssetsIfNeeded();
		bullet->Damage = 20;

		WeaponFireFlash::CreateAt(bullet->Position);

		attackDelay.AddDelay(0.13f);

	}

	void AsyncUpdate()
	{
		viewmodel->Update();
		auto pose = viewmodel->GetAnimationPose();

		auto leftHandPose = pose.GetBoneTransform("clavicle_l");

		leftHandPose.Rotation += vec3(50, 0, 0) * HideWeapon;

		pose.SetBoneTransformEuler("clavicle_l", leftHandPose);
		arms->PasteAnimationPose(pose);
	}

	void LateUpdate()
	{

		Rotation.x += recoilModelOffset;

		viewmodel->Position = Position + (mat3)Camera::GetRotationMatrix() * weaponOffset;
		viewmodel->Rotation = Rotation;

		arms->Position = viewmodel->Position;
		arms->Rotation = viewmodel->Rotation;
	}


	WeaponSlotData GetDefaultData() override
	{

		WeaponSlotData data;

		data.className = "weapon_mpsd";
		data.slot = 2;

		return data;
	}


private:

};

REGISTER_ENTITY(weapon_mpsd, "weapon_mpsd")