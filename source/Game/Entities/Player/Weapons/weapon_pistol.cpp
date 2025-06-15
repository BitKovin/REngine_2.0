#include "WeaponBase.h"
#include "../Player.hpp"

#include <Animation.h>

#include "Projectiles/Bullet.h"

class weapon_pistol : public Weapon
{
public:

	SkeletalMesh* viewmodel;
	SkeletalMesh* arms;

	vec3 weaponOffset = vec3(0.0, 0.00, -0.0);

	Delay attackDelay;


	weapon_pistol()
	{
		viewmodel = new SkeletalMesh(this);
		arms = new SkeletalMesh(this);

		LateUpdateWhenPaused = true;
	}

	SoundPlayer* fireSoundPlayer = nullptr;

	void Start()
	{
		fireSoundPlayer = SoundPlayer::Create("GameData/sounds/weapons/shotgun/shotgun_fire2.wav");
		fireSoundPlayer->Volume = 0.5f;
		fireSoundPlayer->Is2D = true;

		attackDelay.AddDelay(0.3);
		SwitchDelay.AddDelay(0.35);

	}

	void LoadAssets()
	{
		viewmodel->LoadFromFile("GameData/models/player/weapons/pistol/pistol.glb");
		viewmodel->TexturesLocation = "GameData/models/player/weapons/pistol/";
		viewmodel->PlayAnimation("draw");
		viewmodel->PreloadAssets();

		viewmodel->Transparent = true;

		viewmodel->IsViewmodel = true;

		Drawables.push_back(viewmodel);

		arms->LoadFromFile("GameData/arms.glb");
		arms->IsViewmodel = true;
		Drawables.push_back(arms);

		PreloadEntityType("bullet");

	}

	void Update()
	{
		if (Input::GetAction("attack")->Holding())
		{
			if (attackDelay.Wait() == false)
				PerformAttack();
		}
	}

	void PerformAttack()
	{

		fireSoundPlayer->Play();

		SwitchDelay.AddDelay(0.2f);

		viewmodel->PlayAnimation("fire",false,0.1f);
		Camera::AddCameraShake(CameraShake(
			0.13f,                            // interpIn
			0.5f,                            // duration
			vec3(0.0f, 0.0f, -0.1f),         // positionAmplitude
			vec3(0.0f, 0.0f, 3.4f),          // positionFrequency
			vec3(-4, 0.15f, 0.0f),        // rotationAmplitude
			vec3(-2.0f, 18.8f, 0.0f),        // rotationFrequency
			0.5f,                            // falloff
			CameraShake::ShakeType::SingleWave // shakeType
		));


		mat4 boneMat = viewmodel->GetBoneMatrixWorld("muzzle");

		vec3 startLoc = MathHelper::DecomposeMatrix(boneMat).Position;

		startLoc = mix(startLoc, Camera::position, 0.6f) - Camera::Forward()*0.1f;



		Bullet* bullet = new Bullet();
		Level::Current->AddEntity(bullet);

		vec4 offset = vec4(0);

		vec3 endLoc = Position + MathHelper::GetForwardVector(Camera::rotation) * 80.0f + vec3(offset);

		bullet->Speed = 200.f;
		bullet->Position = startLoc + vec3(offset) * 0.002f;
		bullet->Rotation = MathHelper::FindLookAtRotation(startLoc, endLoc);
		bullet->Start();
		bullet->LoadAssetsIfNeeded();
		bullet->Damage = 21;


		attackDelay.AddDelay(0.3f);

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
		viewmodel->Position = Position + (mat3)Camera::GetRotationMatrix() * weaponOffset;
		viewmodel->Rotation = Rotation;

		arms->Position = viewmodel->Position;
		arms->Rotation = viewmodel->Rotation;
	}


	WeaponSlotData GetDefaultData() override
	{

		WeaponSlotData data;

		data.className = "weapon_pistol";

		return data;
	}


private:

};

REGISTER_ENTITY(weapon_pistol, "weapon_pistol")