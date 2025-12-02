#include "WeaponBase.h"
#include "../Player.hpp"

#include <SkeletalMesh.hpp>

class weapon_swords : public Weapon
{

private:

	SkeletalMesh* thirdPersonMesh = nullptr;

	bool comboContinueWait = false;
	bool comboContinue = false;

public:

	weapon_swords()
	{

	}
	~weapon_swords()
	{

	}

	void Update()
	{

		if (Input::GetAction("attack")->Pressed())
		{

			if (thirdPersonMesh->GetAnimationName() != "idle")
			{

			}
			else
			{
				thirdPersonMesh->PlayAnimation("combo_1_1");
			}


		}

		if (thirdPersonMesh->IsAnimationPlaying() == false)
		{
			thirdPersonMesh->PlayAnimation("idle", true);
		}

		thirdPersonMesh->Update();

		auto events = thirdPersonMesh->PullAnimationEvents();

		for (auto& event : events)
		{

			if (event.eventName == "nextAttack_start")
			{
				comboContinue = false;
				comboContinueWait = true;
			}

			if (event.eventName == "nextAttack_end")
			{

				bool comboContinue = Input::GetAction("attack")->PressedBuffered(0.5f);

				if (comboContinue == false)
				{
					thirdPersonMesh->PlayAnimation("idle", true, 0.6);
				}

				if (event.userData1.empty() == false)
				{
					thirdPersonMesh->PlayAnimation(event.userData1,false,0.2);
				}

				comboContinue = false;
				comboContinueWait = false;

			}

		}

	}

	void LateUpdate()
	{
		thirdPersonMesh->Position = owner->bodyMesh->Position;
		thirdPersonMesh->Rotation = owner->bodyMesh->Rotation;
	}

	AnimationPose ApplyWeaponAnimation(AnimationPose thirdPersonPose) override
	{

		auto weaponPose = thirdPersonMesh->GetAnimationPose();

		auto leftFootT = MathHelper::DecomposeMatrix(AnimationPose::GetModelSpaceTransform("foot_l", thirdPersonMesh->GetRootNode(), thirdPersonPose));
		auto rightFootT = MathHelper::DecomposeMatrix(AnimationPose::GetModelSpaceTransform("foot_r", thirdPersonMesh->GetRootNode(), thirdPersonPose));

		auto resultPose = AnimationPose::LayeredLerp("pelvis", thirdPersonMesh->GetRootNode(),
			thirdPersonPose, weaponPose, 1, 1.0);
		resultPose = AnimationPose::LayeredLerp("spine_01", thirdPersonMesh->GetRootNode(),
			resultPose, weaponPose, 1, 1.0);

		resultPose = AnimationPose::ApplyFABRIK("thigh_l", "foot_l", thirdPersonMesh->GetRootNode(),
			resultPose, leftFootT.Position, leftFootT.RotationQuaternion);

		resultPose = AnimationPose::ApplyFABRIK("thigh_r", "foot_r", thirdPersonMesh->GetRootNode(),
			resultPose, rightFootT.Position, rightFootT.RotationQuaternion);

		return resultPose;

	}

	WeaponSlotData GetDefaultData() override
	{
		WeaponSlotData data;
		data.className = "weapon_swords"; // Override in subclasses
		data.slot = 0; // Override in subclasses
		return data;
	}

protected:

	void LoadAssets() override
	{
		
		Drawables.push_back(thirdPersonMesh = new SkeletalMesh(owner));

		thirdPersonMesh->LoadFromFile("GameData/models/player/weapons/swords/swords_tp.glb");

		thirdPersonMesh->PlayAnimation("idle");

	}

};

REGISTER_ENTITY(weapon_swords,"weapon_swords")