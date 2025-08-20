#include "NpcBase.h"

#include <SoundSystem/FmodEventInstance.h>

#include <Particle/GlobalParticleSystem.hpp>

#include <Input.h>

#include <imgui/imgui.h>

NpcBase::NpcBase()
{

	mesh = new SkeletalMesh(this);
	Drawables.push_back(mesh);

	ClassName = "npc_base";
	SaveGame = true;

	Health = 50;

	mesh->UpdatePoseOnlyWhenRendered = true;

	animator = NpcAnimatorBase(this);

	Tags.push_back("enemy");

}

NpcBase::~NpcBase()
{
}

void NpcBase::ProcessAnimationEvent(AnimationEvent& event)
{


}

void NpcBase::Start()
{

	mesh->Position = Position - vec3(0, 1, 0);
	mesh->Rotation = Rotation;

	//Drawables.push_back(mesh);

	LeadBody = Physics::CreateCharacterBody(this, Position, 0.5, 2, 50);


	Physics::SetGravityFactor(LeadBody, 4);

	desiredDirection = MathHelper::XZ(MathHelper::GetForwardVector(Rotation));
	movingDirection = desiredDirection;

	pathFollow.CalculatePathOnThread();

	DeathSoundPlayer = SoundPlayer::Create();
	HurtSoundPlayer = SoundPlayer::Create();
	StunSoundPlayer = SoundPlayer::Create();
	AttackSoundPlayer = SoundPlayer::Create();
	AttackHitSoundPlayer = SoundPlayer::Create();

	SetupSoundPlayer(DeathSoundPlayer);
	DeathSoundPlayer->MaxDistance *= 1.5f;
	DeathSoundPlayer->Volume *= 2.0f;
	SetupSoundPlayer(HurtSoundPlayer);
	HurtSoundPlayer->Volume *= 1.5f;
	SetupSoundPlayer(StunSoundPlayer);
	SetupSoundPlayer(AttackSoundPlayer);
	AttackSoundPlayer->Volume *= 0.7f;
	SetupSoundPlayer(AttackHitSoundPlayer);
	AttackHitSoundPlayer->Volume *= 1.2;
}


void NpcBase::Death()
{

	if (dead) return;

	//mesh->ClearHitboxes();
	mesh->StartRagdoll();

	animator.PainProgress = 1;

	mesh->SetAnimationPaused(true);
	Physics::SetLinearVelocity(LeadBody, vec3(0));

	DeathSoundPlayer->Play();

	//Physics::SetBodyType(LeadBody, BodyType::None);
	//Physics::SetCollisionMask(LeadBody, BodyType::World);

	Physics::DestroyBody(LeadBody);
	LeadBody = nullptr;

	dead = true;

	//Tags.clear();

	return;
	if (DeathSoundPlayer)
	{
		DeathSoundPlayer->DestroyWithDelay(3);
		HurtSoundPlayer->DestroyWithDelay(3);
		StunSoundPlayer->DestroyWithDelay(3);
		AttackSoundPlayer->DestroyWithDelay(3);
		AttackHitSoundPlayer->DestroyWithDelay(3);
		DeathSoundPlayer = nullptr;
		HurtSoundPlayer = nullptr;
		StunSoundPlayer = nullptr;
		AttackSoundPlayer = nullptr;
	}

}

void NpcBase::OnPointDamage(float Damage, vec3 Point, vec3 Direction, string bone, Entity* DamageCauser, Entity* Weapon)
{
	Damage *= mesh->GetHitboxDamageMultiplier(bone);
	Entity::OnPointDamage(Damage, Point, Direction, bone, DamageCauser, Weapon);

	if (mesh->GetHitboxDamageMultiplier(bone) > 1.1f && dead)
	{
		Time::AddTimeScaleEffect(0.3f, 0.15f, true, "hit_slow");
	}

	GlobalParticleSystem::SpawnParticleAt("hit_flesh", Point, MathHelper::FindLookAtRotation(vec3(0), Direction), vec3(Damage / 20.0f));

}

void NpcBase::OnDamage(float Damage, Entity* DamageCauser, Entity* Weapon)
{

	Health -= Damage;

	if (Health <= 0)
	{
		Death();
	}


	if (LeadBody)
	{
		LeadBody->SetLinearVelocity(LeadBody->GetLinearVelocity() / 2.0f);
		speed /= 2.0f;
		HurtSoundPlayer->Play();
	}


}

void NpcBase::StartReturnFromRagdoll()
{

	if (mesh->InRagdoll == false) return;

	if (LeadBody == nullptr)
	{
		LeadBody = Physics::CreateCharacterBody(this, Position, 0.5, 2, 50);
	}


	vec3 pelvisPos = MathHelper::DecomposeMatrix(mesh->GetBoneMatrixWorld("pelvis")).Position;
	vec3 pelvisRot = MathHelper::DecomposeMatrix(mesh->GetBoneMatrixWorld("pelvis")).Rotation;
	vec3 spinePos = MathHelper::DecomposeMatrix(mesh->GetBoneMatrixWorld("spine_03")).Position;

	ragdollPose = mesh->GetAnimationPose();

	bool onFront = MathHelper::GetUpVector(MathHelper::DecomposeMatrix(mesh->GetBoneMatrixWorld("pelvis")).Rotation).y > 0;

	getFromRagdollAnimation->PlayAnimation(onFront? "front" : "back", false, 0);

	auto pelvisTransformWorld = MathHelper::DecomposeMatrix(mesh->GetBoneMatrixWorld("pelvis"));

	Position = pelvisTransformWorld.Position + vec3(0,1,0);
	Physics::SetBodyPosition(LeadBody, Position);

	float oldRot = mesh->Rotation.y;

	if (onFront)
	{
		mesh->Rotation = vec3(0, MathHelper::FindLookAtRotation(pelvisPos, spinePos).y, 0);
	}
	else
	{
		mesh->Rotation = vec3(0, MathHelper::FindLookAtRotation(spinePos, pelvisPos).y, 0);
	}

	

	auto pelvisTransform = MathHelper::DecomposeMatrix(ragdollPose.boneTransforms["pelvis"]);

	pelvisTransform.Position = vec3(0,0.5,0);
	pelvisTransform.RotationQuaternion = MathHelper::GetRotationQuaternion(vec3(0, oldRot - mesh->Rotation.y ,0)) * pelvisTransform.RotationQuaternion;

	ragdollPose.boneTransforms["pelvis"] = pelvisTransform.ToMatrix();

	mesh->StopRagdoll();



	movingDirection = MathHelper::GetForwardVector(mesh->Rotation);
	desiredDirection = movingDirection;


}

void NpcBase::UpdateReturnFromRagdoll()
{

	if (getFromRagdollAnimation->IsAnimationPlaying() == false) return;

	auto meshPose = mesh->GetAnimationPose();

	getFromRagdollAnimation->Update(1.0f);

	float blendInTime = 0.5;
	float blendOutTime = 1;

	float lerpProgressFromStart = 1.0f - ((blendInTime - getFromRagdollAnimation->GetAnimationTime()) / blendInTime);

	lerpProgressFromStart = saturate(lerpProgressFromStart);

	auto animationPose = getFromRagdollAnimation->GetAnimationPose();

	auto newPose = AnimationPose::Lerp(ragdollPose, animationPose, lerpProgressFromStart);

	float lerpProgressFromEnd = ((blendOutTime - (getFromRagdollAnimation->GetAnimationDuration() - getFromRagdollAnimation->GetAnimationTime())) / blendOutTime);

	newPose = AnimationPose::Lerp(newPose, meshPose, lerpProgressFromEnd);

	mesh->PasteAnimationPose(newPose);

}


void NpcBase::AsyncUpdate()
{


	auto animEvents = mesh->PullAnimationEvents();

	for (auto& event : animEvents)
	{
		ProcessAnimationEvent(event);
	}

	if (Input::GetAction("attack2")->Pressed() && false)
	{

		Health = 100;
		dead = false;

		StartReturnFromRagdoll();

	}

	if (dead) 
	{
		UpdateAnimations();
		mesh->UpdateHitboxes();
		return;
	}

	DeathSoundPlayer->Position = Position;
	HurtSoundPlayer->Position = Position;
	StunSoundPlayer->Position = Position;
	AttackSoundPlayer->Position = Position;
	AttackHitSoundPlayer->Position = Position;

	DeathSoundPlayer->Velocity = FromPhysics(LeadBody->GetLinearVelocity());
	HurtSoundPlayer->Velocity = FromPhysics(LeadBody->GetLinearVelocity());
	StunSoundPlayer->Velocity = FromPhysics(LeadBody->GetLinearVelocity());
	AttackSoundPlayer->Velocity = FromPhysics(LeadBody->GetLinearVelocity());
	AttackHitSoundPlayer->Velocity = FromPhysics(LeadBody->GetLinearVelocity());



	pathFollow.UpdateStartAndTarget(Position, Position + MathHelper::GetForwardVector(mesh->Rotation) + MathHelper::GetRightVector(mesh->Rotation));
	pathFollow.TryPerform();

	UpdateAnimations();
	UpdateReturnFromRagdoll();

	mesh->UpdateHitboxes();

	mesh->Position = Position - vec3(0, 1, 0);
	mesh->Rotation = vec3(0, MathHelper::FindLookAtRotation(vec3(), movingDirection).y, 0);

}

void NpcBase::UpdateAnimations()
{

	animator.movementSpeed = speed;
	animator.Update();
	
	auto pose = animator.GetResultPose();

	mesh->PasteAnimationPose(pose);

}

void NpcBase::LoadAssets()
{

	SoundManager::LoadBankFromPath("GameData/sounds/banks/Desktop/SFX.bank");

	mesh->LoadFromFile("GameData/models/npc/base.glb");
	mesh->CreateHitboxes(this);

	getFromRagdollAnimation = new Animation(this);
	getFromRagdollAnimation->LoadFromFile("GameData/animations/npc/standUp.glb");

	animator.LoadAssetsIfNeeded();

	mesh->ColorTexture = AssetRegistry::GetTextureFromFile("GameData/cat.png");

	SET_SOUND_SAFE(DeathSoundPlayer, FmodEventInstance::Create("event:/NPC/Dog/DogDeath"));
	SET_SOUND_SAFE(HurtSoundPlayer, SoundManager::GetSoundFromPath("GameData/sounds/dog/dog_hit.wav"));
	SET_SOUND_SAFE(StunSoundPlayer, FmodEventInstance::Create("event:/NPC/Dog/DogStun"));
	SET_SOUND_SAFE(AttackSoundPlayer, FmodEventInstance::Create("event:/NPC/Dog/DogAttackStart"));
	SET_SOUND_SAFE(AttackHitSoundPlayer, FmodEventInstance::Create("event:/NPC/Dog/DogAttack"));

}

void NpcBase::Serialize(json& target)
{

	Entity::Serialize(target);

	animationStateSaveData = mesh->GetAnimationState();

	getFromRagdollAnimationSaveState = getFromRagdollAnimation->GetAnimationState();

	Rotation = mesh->Rotation;

	SERIALIZE_FIELD(target, Rotation)
		SERIALIZE_FIELD(target, desiredDirection)
		SERIALIZE_FIELD(target, movingDirection)
		SERIALIZE_FIELD(target, speed)
		SERIALIZE_FIELD(target, dead)
		SERIALIZE_FIELD(target, animationStateSaveData)
		SERIALIZE_FIELD(target, getFromRagdollAnimationSaveState)
		SERIALIZE_FIELD(target, ragdollPose)
}

void NpcBase::Deserialize(json& source)
{

	Entity::Deserialize(source);

	DESERIALIZE_FIELD(source, Rotation)
		DESERIALIZE_FIELD(source, desiredDirection)
		DESERIALIZE_FIELD(source, movingDirection)
		DESERIALIZE_FIELD(source, speed)
		DESERIALIZE_FIELD(source, dead)
		DESERIALIZE_FIELD(source, animationStateSaveData)
		DESERIALIZE_FIELD(source, getFromRagdollAnimationSaveState)
		DESERIALIZE_FIELD(source, ragdollPose)


		Physics::SetBodyPosition(LeadBody, Position);


	if (dead)
	{
		Physics::DestroyBody(LeadBody);
		LeadBody = nullptr;
		/*
		DeathSoundPlayer->Destroy();
		HurtSoundPlayer->Destroy();
		StunSoundPlayer->Destroy();
		AttackSoundPlayer->Destroy();
		DeathSoundPlayer = nullptr;
		HurtSoundPlayer = nullptr;
		StunSoundPlayer = nullptr;
		AttackSoundPlayer = nullptr;
		*/

	}


	mesh->Rotation = Rotation;

	mesh->SetAnimationState(animationStateSaveData);
	getFromRagdollAnimation->SetAnimationState(getFromRagdollAnimationSaveState);

	//mesh->Update(0);
	//mesh->PullRootMotion();

}

void NpcBase::UpdateDebugUI()
{

	ImGui::Begin(Id.c_str());

	ImGui::DragFloat("activeragdoll strength", &mesh->RagdollPoseFollowStrength,0.1f);
	ImGui::Checkbox("update ragdoll pose", &mesh->UpdateRagdollPose);

	ImGui::End();

}

REGISTER_ENTITY(NpcBase, "npc_base")