#include "WeaponBase.h"
#include "../Player.hpp"

#include <Animation.h>

#include "Projectiles/Bullet.h"

class weapon_shotgun : public Weapon
{
public:
	
	SkeletalMesh* viewmodel;
	SkeletalMesh* arms;

	vec3 weaponOffset = vec3(0.023, 0.013, -0.13);

	Delay attackDelay;

	weapon_shotgun()
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
		viewmodel->LoadFromFile("GameData/testViewmodel.glb");

		viewmodel->PlayAnimation("draw");

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
			PerformAttack();
		}
	}

	void PerformAttack()
	{

		if (attackDelay.Wait())
			return;

		SwitchDelay.AddDelay(0.2f);

		attackDelay.AddDelay(1);

		fireSoundPlayer->Play();

		viewmodel->PlayAnimation("attack");
		Camera::AddCameraShake(CameraShake(
			0.13f,                            // interpIn
			1.2f,                            // duration
			vec3(0.0f, 0.0f, -0.2f),         // positionAmplitude
			vec3(0.0f, 0.0f, 6.4f),          // positionFrequency
			vec3(-8, 0.15f, 0.0f),        // rotationAmplitude
			vec3(-5.0f, 28.8f, 0.0f),        // rotationFrequency
			1.2f,                            // falloff
			CameraShake::ShakeType::SingleWave // shakeType
		));



		mat4 boneMat = viewmodel->GetBoneMatrixWorld("muzzle");

		vec3 startLoc = MathHelper::DecomposeMatrix(boneMat).Position;

		startLoc = mix(startLoc, Camera::position, 0.6f);

		int n = 0;

		for (float y = -4; y <= 4; y += 2.0f)
			for (float x = -4; x <= 4; x += 2.0f)
			{

				if (length(vec2(x, y)) > 4.7f)
					continue;

				Bullet* bullet = new Bullet();
				Level::Current->AddEntity(bullet);

				vec4 offset = MathHelper::GetRotationMatrix(Rotation) * vec4(x,y,0,1);

				vec3 endLoc = Position + MathHelper::GetForwardVector(Rotation) * 80.0f + vec3(offset);

				bullet->Position = startLoc + vec3(offset) * 0.002f;
				bullet->Rotation = MathHelper::FindLookAtRotation(startLoc, endLoc);
				bullet->Start();
				bullet->LoadAssetsIfNeeded();
				bullet->Damage = 50.0f / 21.0f;

				n++;

			}

	}

	void AsyncUpdate()
	{
		viewmodel->Update();
		auto pose = viewmodel->GetAnimationPose();

		auto leftHandPose = pose.GetBoneTransform("clavicle_l");

		leftHandPose.Rotation += vec3(50,0,0) * HideWeapon;

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

		data.className = "weapon_shotgun";
		data.slot = 1;

		return data;
	}

private:

};

REGISTER_ENTITY(weapon_shotgun, "weapon_shotgun")