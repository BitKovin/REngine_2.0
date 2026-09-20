#include "WeaponTool.h"

WeaponTool::WeaponTool()
{
	LateUpdateWhenPaused = true;

	DrawTime = 0.2f;
	HideTime = 0.4f;

	// No "start presented" override here - see WeaponMelee::WeaponMelee()
	// for the reasoning. Tool starts at rest (DrawProgress = 0, autoHideTimer
	// = 0, both base Weapon defaults) on equip, and only rises to ready when
	// it's actually used (StartUse() calling NotifyUsed(), see Update()).
}

void WeaponTool::Start()
{
	SwitchDelay.AddDelay(0.35f);

	// Self-init immediately, same as WeaponFirearm::Start() - entities
	// spawned mid-Update() don't get their own Update() call until next
	// frame otherwise, which would both show a wrong pose for one frame and
	// (for the "useTool" press that caused this weapon to be equipped in
	// the first place) miss its use entirely.
	Update();
	AsyncUpdate();
	LateUpdate();
}

void WeaponTool::LoadAssets()
{
	Weapon::LoadAssets();

	SoundManager::LoadBankFromPath("GameData/sounds/banks/Desktop/Weapons.bank");
	SoundManager::LoadBankFromPath("GameData/sounds/banks/Desktop/SFX.bank");

	viewmodel = new SkeletalMesh(owner);
	arms = new SkeletalMesh(owner);

	viewmodel->GravityAlignedRotation = true;
	arms->GravityAlignedRotation = true;

	viewmodel->LoadFromFile(modelPath);
	if (!texturesLocation.empty())
		viewmodel->TexturesLocation = texturesLocation;

	viewmodel->PlayAnimation(drawAnim, false);
	// SkipDrawAnimation (default true): jump straight to the last frame of
	// the draw clip instead of playing it - see Weapon::DrawProgress for the
	// procedural raise/lower motion that replaces it.
	if (SkipDrawAnimation)
		viewmodel->SetAnimationTime(viewmodel->GetAnimationDuration());

	viewmodel->PreloadAssets();
	viewmodel->IsViewmodel = true;
	Drawables.push_back(viewmodel);

	arms->LoadFromFile(ArmsModelPath);
	arms->IsViewmodel = true;
	Drawables.push_back(arms);
}

void WeaponTool::SetViewmodelScaleFactor(float factor)
{
	viewmodel->ViewmodelScaleFactor = factor;
	arms->ViewmodelScaleFactor = factor;
}

void WeaponTool::Update()
{
	// Single dedicated key: press once to use the tool. Player handles
	// switching back to whatever was equipped before once CanChangeSlot()
	// allows it - the tool just needs to eventually let that return true.
	if (Input::GetAction("useTool")->PressedBuffered() && CanAttack() && !attackDelay.Wait())
	{
		StartUse();
		NotifyUsed(); // (re)start the 3s auto-hide countdown from this use
	}

	UpdateAutoHideTimer();
	UpdateDrawProgress(autoHideTimer > 0.0f);
}

void WeaponTool::AsyncUpdate()
{
	viewmodel->Update();
	auto pose = viewmodel->GetAnimationPose();
	arms->PasteAnimationPose(pose);
}

void WeaponTool::LateUpdate()
{
	viewmodel->Position = Position + (mat3)Camera::GetRotationMatrix() * weaponOffset;
	viewmodel->Rotation = Rotation;

	arms->Position = viewmodel->Position;
	arms->Rotation = viewmodel->Rotation;
}

void WeaponTool::Serialize(json& target)
{
	Weapon::Serialize(target);

	SERIALIZE_FIELD(target, attackDelay);

	auto viewmodelData = viewmodel->GetAnimationState();
	SERIALIZE_FIELD(target, viewmodelData);
}

void WeaponTool::Deserialize(json& source)
{
	DESERIALIZE_FIELD(source, attackDelay);

	AnimationState viewmodelData;
	DESERIALIZE_FIELD(source, viewmodelData);
	viewmodel->SetAnimationState(viewmodelData);
}
