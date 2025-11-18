#include "WeaponBase.h"
#include "../Player.hpp"

#include <Animation.h>

#include "Projectiles/Bullet.h"
#include <SoundSystem/FmodEventInstance.h>
#include "WeaponFireFlash.h"

class weapon_sniper : public Weapon
{
public:

	SkeletalMesh* viewmodel;
	SkeletalMesh* arms;

	vec3 weaponOffset = vec3(0.0, 0.00, -0.0);

	vec3 weaponAimOffset = vec3(0.062707, 0.033706, -0.0);

	float aimProgress = 0.0f;

	Delay attackDelay;

	bool Silencer = false;

	weapon_sniper()
	{

		LateUpdateWhenPaused = true;

		bobScale = 0;

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

		viewmodel->LoadFromFile("GameData/models/player/weapons/sniper/sniper.glb");
		viewmodel->TexturesLocation = "GameData/models/player/weapons/sniper/sniper.glb/";

		//viewmodel->LoadFromFile("GameData/models/player/weapons/revolver/revolver.glb");
		//viewmodel->TexturesLocation = "GameData/models/player/weapons/revolver/revolver.glb/";
		viewmodel->PlayAnimation("draw");
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

		if (Input::GetAction("attack2")->Holding())
		{
			aimProgress += Time::DeltaTimeF * 3.0f;
		}
		else
		{
			aimProgress -= Time::DeltaTimeF * 3.0f;
		}

		if (aimProgress > 0.01f) SwitchDelay.AddDelay(0.1f);

		aimProgress = glm::clamp(aimProgress, 0.0f, 1.0f);
		bobScale = 1.0f - aimProgress;

		weaponOffset = mix(vec3(0.0, 0.00, 0.0), weaponAimOffset, aimProgress);
		Camera::FOV = mix(75.0f, 10.0f, aimProgress);

		if (Input::GetAction("attack")->Holding())
		{
			if (attackDelay.Wait() == false)
				PerformAttack();
		}

	}

	void PerformAttack()
	{

		fireSoundPlayer->Pitch = Silencer ? 2.8f : 1.0f;
		fireSoundPlayer->Play();

		SwitchDelay.AddDelay(0.5f);

		viewmodel->PlayAnimation("fire", false, 0.08f);
		Camera::AddCameraShake(CameraShake(
			0.13f,                             // interpIn
			0.5f,                              // duration
			vec3(0.0f, 0.0f, -0.1f),           // positionAmplitude
			vec3(0.0f, 0.0f, 3.4f),            // positionFrequency
			vec3(-4, 0.15f, 0.0f),             // rotationAmplitude
			vec3(-2.0f, 18.8f, 0.0f),          // rotationFrequency
			0.5f,                              // falloff
			CameraShake::ShakeType::SingleWave // shakeType
		));


		mat4 boneMat = viewmodel->GetBoneMatrixWorld("muzzle");

		vec3 startLoc = MathHelper::DecomposeMatrix(boneMat).Position;

		startLoc = mix(startLoc, Camera::position, 0.6f) - Camera::Forward() * 0.1f;



		Bullet* bullet = new Bullet();
		Level::Current->AddEntity(bullet);

		vec4 offset = vec4(0);

		vec3 endLoc = Position + MathHelper::GetForwardVector(Camera::rotation) * 80.0f + vec3(offset);

		bullet->Speed = 200.f;
		bullet->Position = startLoc + vec3(offset) * 0.002f;
		bullet->Rotation = MathHelper::FindLookAtRotation(startLoc, endLoc);
		bullet->Start();
		bullet->LoadAssetsIfNeeded();
		bullet->Damage = 70;

		WeaponFireFlash::CreateAt(bullet->Position);

		attackDelay.AddDelay(1.3f);

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

	void UpdateDebugUI() override
	{

		ImGui::Begin("Sniper Weapon Debug");

		ImGui::Checkbox("Silencer", &Silencer);	
		ImGui::DragFloat3("Weapon Offset", &weaponOffset.x, 0.01f);

		ImGui::End();

	}
	
	void LateUpdate()
	{

		weaponOffset.z = mix(0.0f, 0.02f, aimProgress);

		viewmodel->Position = Position + (mat3)Camera::GetRotationMatrix() * weaponOffset;
		viewmodel->Rotation = Rotation;


		viewmodel->Scale.z = mix(1.0f, 0.02f, aimProgress);


		arms->Position = viewmodel->Position;
		arms->Rotation = viewmodel->Rotation;
		arms->Scale = viewmodel->Scale;

		viewmodel->MeshHideList.clear();

		if (Silencer)
		{
			viewmodel->MeshHideList.insert("muzzle");
		}
		else
		{
			viewmodel->MeshHideList.insert("silencer");
		}

	}


	WeaponSlotData GetDefaultData() override
	{

		WeaponSlotData data;

		data.className = "weapon_sniper";
		data.slot = 3;

		return data;
	}


private:

};

REGISTER_ENTITY(weapon_sniper, "weapon_sniper")