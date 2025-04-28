#include "WeaponBase.h"
#include "../Player.hpp"

#include "Projectiles/Bullet.h"

class weapon_shotgun : public Weapon
{
public:
	
	SkeletalMesh* viewmodel;
	SkeletalMesh* arms;

	vec3 weaponOffset = vec3(0.023, 0.013, -0.13);

	weapon_shotgun()
	{
		viewmodel = new SkeletalMesh();
		arms = new SkeletalMesh();
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
		if (Input::GetAction("attack")->Pressed())
		{
			PerformAttack();
		}
	}

	void PerformAttack()
	{
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

		Bullet* bullet = new Bullet();
		Level::Current->AddEntity(bullet);

		mat4 boneMat = viewmodel->GetBoneMatrixWorld("muzzle");

		bullet->Position = MathHelper::DecomposeMatrix(boneMat).Position;
		bullet->Rotation = Rotation;
		bullet->Start();
		bullet->LoadAssetsIfNeeded();


	}

	void AsyncUpdate()
	{
		viewmodel->Update();
		arms->PasteAnimationPose(viewmodel->GetAnimationPose());
	}

	void LateUpdate()
	{
		viewmodel->Position = Position + (mat3)Camera::GetRotationMatrix() * weaponOffset;
		viewmodel->Rotation = Rotation;

		arms->Position = viewmodel->Position;
		arms->Rotation = viewmodel->Rotation;
	}


private:

};

REGISTER_ENTITY(weapon_shotgun, "weapon_shotgun")