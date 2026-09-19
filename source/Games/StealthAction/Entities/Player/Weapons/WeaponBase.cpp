#include "WeaponBase.h"

#include "../Player.hpp"

#include <algorithm>

bool Weapon::CanAttack()
{
	return owner->RunProgress < 0.01f && owner->dead == false && DrawProgress >= 0.99f;
}

void Weapon::UpdateDrawProgress(bool wantDrawn)
{
	float target = wantDrawn ? 1.0f : 0.0f;
	float time = wantDrawn ? DrawTime : HideTime;
	float speed = 1.0f / std::max(time, 0.001f);

	if (DrawProgress < target)
		DrawProgress = std::min(target, DrawProgress + speed * Time::DeltaTimeF);
	else
		DrawProgress = std::max(target, DrawProgress - speed * Time::DeltaTimeF);
}

void Weapon::UpdateDebugUI()
{

	ImGui::Begin("weapon parameters");

	ImGui::DragFloat3("HiddenPosePosition", &HiddenPosePosition.x, 0.02);
	ImGui::DragFloat3("HiddenPoseRotation", &HiddenPoseRotation.x, 0.25);
	ImGui::DragFloat3("HiddenPoseRotationPoint", &HiddenPoseRotationPoint.x, 0.02);
	ImGui::End();
}

void Weapon::LoadAssets()
{

	SkeletalMesh* skm = new SkeletalMesh(this);

	skm->LoadFromFile(thirdPersonModelPath);
	skm->PreloadAssets();
	delete skm;


}
