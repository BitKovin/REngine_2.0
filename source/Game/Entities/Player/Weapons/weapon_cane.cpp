#include "WeaponBase.h"
#include "../Player.hpp"

#include <Animation.h>

#include "Projectiles/CaneProjectile.h"
#include <SoundSystem/FmodEventInstance.h>


class weapon_cane : public Weapon
{
public:

	SkeletalMesh* viewmodel;
	SkeletalMesh* arms;

	vec3 weaponOffset = vec3(0.0, 0.00, -0.0);

	Delay attackDelay;

	vec3 projectileOffset = vec3(0.03f, -0.15f, -0.3f);

	bool thrown = false;

	weapon_cane()
	{

		LateUpdateWhenPaused = true;
	}

	SoundPlayer* fireSoundPlayer = nullptr;

	void Start()
	{

		fireSoundPlayer = SoundPlayer::Create("event:/Weapons/pistol/pistol_fire");
		fireSoundPlayer->Volume = 0.5f;
		fireSoundPlayer->Is2D = true;


		//attackDelay.AddDelay(0.3);
		SwitchDelay.AddDelay(0.35);

	}

	void SetViewmodelScaleFactor(float factor)
	{
		viewmodel->ViewmodelScaleFactor = factor;
		arms->ViewmodelScaleFactor = factor;
	}

	void LoadAssets()
	{

		SoundManager::LoadBankFromPath("GameData/sounds/banks/Desktop/Weapons.bank");
		SoundManager::LoadBankFromPath("GameData/sounds/banks/Desktop/SFX.bank");

		viewmodel = new SkeletalMesh(this);
		arms = new SkeletalMesh(this);

		viewmodel->LoadFromFile("GameData/models/player/weapons/cane/cane.glb");
		//viewmodel->ColorTexture = AssetRegistry::GetTextureFromFile("GameData/textures/cat.png");
		viewmodel->TexturesLocation = "GameData/models/player/weapons/cane/"; // to search in file:   cane.glb/
		viewmodel->PlayAnimation("idle",true);
		viewmodel->PreloadAssets();

		viewmodel->IsViewmodel = true;

		Drawables.push_back(viewmodel);

		arms->LoadFromFile("GameData/arms.glb");
		arms->IsViewmodel = true;
		Drawables.push_back(arms);

		PreloadEntityType("caneProjectile");

	}

	void ReturnCane()
	{
		thrown = false;

		auto projectiles = Level::Current->FindAllEntitiesWithName("caneProjectile");

		for (auto p : projectiles)
		{
			p->Destroy();
		}

		viewmodel->PlayAnimation("idle", true, 0.3f);

	}

	void GrabCane()
	{
		thrown = false;

		auto projectiles = Level::Current->FindAllEntitiesWithName("caneProjectile");

		for (auto p : projectiles)
		{
			p->Destroy();
		}

		SetViewmodelScaleFactor(0.5);

		viewmodel->PlayAnimation("grab", false, 0.0f);
		Time::AddTimeScaleEffect(0.65, 0.2, true, "weapon", 0.2f, 0.2);

	}

	void Update()
	{

		if (Input::GetAction("attack2")->Pressed())
		{
			if (attackDelay.Wait() == false)
			{

				if (thrown)
				{
					GrabCane();
				}
				else
				{
					Attack();
				}

			}
		}

		Entity* caneProjectile = Level::Current->FindEntityWithName("caneProjectile");

		if (caneProjectile == nullptr && thrown)
		{
			ReturnCane();
		}
		else
		{

			if (caneProjectile != nullptr && attackDelay.Wait() == false)
			{
				if (viewmodel->currentAnimationData->animationName != "throw")
				{
					viewmodel->PlayAnimation("throw", false, 0);
					viewmodel->SetAnimationTime(viewmodel->GetAnimationDuration());
					viewmodel->Update();
					viewmodel->PullAnimationEvents();
				}	


			}

		}

		auto events = viewmodel->PullAnimationEvents();

		for (auto event : events)
		{
			if (event.eventName == "throw")
			{
				PerformAttack();
			}

			if (event.eventName == "restore_size")
			{
				SetViewmodelScaleFactor(2);
			}
		}

	}

	void Attack()
	{

		SwitchDelay.AddDelay(0.2f);

		SetViewmodelScaleFactor(2);

		viewmodel->PlayAnimation("throw", false, 0);
		Camera::AddCameraShake(CameraShake(
			0.13f,                            // interpIn
			0.0f,                            // duration
			vec3(0.0f, 0.0f, -0.1f),         // positionAmplitude
			vec3(0.0f, 0.0f, 3.4f),          // positionFrequency
			vec3(-4, 0.15f, 0.0f),        // rotationAmplitude
			vec3(-2.0f, 18.8f, 0.0f),        // rotationFrequency
			0.5f,                            // falloff
			CameraShake::ShakeType::SingleWave // shakeType
		));



		attackDelay.AddDelay(1.0f);

	}

	void UpdateDebugUI()
	{

		//ImGui::Begin("cane options");

		//ImGui::DragFloat3("projectile offset", &projectileOffset.x, 0.01f);

		//ImGui::End();

	}

	void PerformAttack()
	{

		vec3 startLoc = Camera::position +
			MathHelper::TransformVector(projectileOffset,
				Camera::GetRotationMatrix());

		thrown = true;
		auto projectiles = Level::Current->FindAllEntitiesWithName("caneProjectile");

		for (auto p : projectiles)
		{
			p->Destroy();
		}

		CaneProjectile* bullet = new CaneProjectile();
		Level::Current->AddEntity(bullet);

		vec4 offset = vec4(0);

		vec3 endLoc = Position + MathHelper::GetForwardVector(Camera::rotation) * 80.0f + vec3(offset);

		bullet->Speed = 60.f;
		bullet->MaxDistance = 120;
		bullet->Position = startLoc + vec3(offset) * 0.002f;
		bullet->Rotation = MathHelper::FindLookAtRotation(startLoc, endLoc);
		bullet->Start();
		bullet->LoadAssetsIfNeeded();
		bullet->Damage = 21;

		fireSoundPlayer->Play();

	}

	void AsyncUpdate()
	{
		viewmodel->Update();

		auto pose = viewmodel->GetAnimationPose();

		arms->PasteAnimationPose(pose);
	}

	void LateUpdate()
	{
		viewmodel->Position = Position + (mat3)Camera::GetRotationMatrix() * weaponOffset;
		viewmodel->Rotation = Rotation;

		arms->Position = viewmodel->Position;
		arms->Rotation = viewmodel->Rotation;

		viewmodel->Visible = !thrown;
	}


	WeaponSlotData GetDefaultData() override
	{

		WeaponSlotData data;

		data.className = "weapon_cane";

		return data;
	}


private:

};

REGISTER_ENTITY(weapon_cane, "weapon_cane")