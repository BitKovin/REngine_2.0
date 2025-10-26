#include "NpcBase.h"

#include <SoundSystem/FmodEventInstance.h>

#include <Particle/GlobalParticleSystem.hpp>

#include <Input.h>

#include <imgui/imgui.h>

#include "../Player/Player.hpp"

#include "Ai/nav_point.h"

#include <AiPerception/ObservationTarget.h>

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

	pathFollow.Async = false;

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

	editor = BehaviorTreeEditor(&behaviorTree);
	editor.Init();
	
	behaviorTree.LoadFromFile("GameData/behaviourTrees/general.bt");
	behaviorTree.Owner = this;
	behaviorTree.Start();

	observer = AiPerceptionSystem::CreateObserver(Position + vec3(0, 0.7, 0), movingDirection, 90);
	observer->owner = Id;
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

	AiPerceptionSystem::RemoveObserver(observer);
	observer = nullptr;

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

	UpdateObserver();
	UpdateTargetFollow();
	UpdateBT();

	auto animEvents = mesh->PullAnimationEvents();

	for (auto& event : animEvents)
	{
		ProcessAnimationEvent(event);
	}


	if (dead) 
	{
		UpdateAnimations();
		if (mesh->WasRended)
		{
			mesh->UpdateHitboxes();
		}
		else if (tickIntervalDelay.Wait() == false)
		{
			mesh->UpdateHitboxes();
		}
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


	bool lockAtTarget = target_sees && target_attack && target_attackInRange;

	vec3 desiredDirection = vec3(0);
	
	if (pathFollow.reachedTarget == false)
	{
		pathFollow.UpdateStartAndTarget(Position, desiredTargetLocation);
		pathFollow.TryPerform();
	}

	if (pathFollow.reachedTarget == false)
	{

		vec3 dir = MathHelper::XZ(pathFollow.CalculatedTargetLocation - Position);

		if (length(dir) > 0.001)
		{
			vec3 newMove = normalize(dir) * speed;

			vec3 curMove = MathHelper::XZ(FromPhysics(LeadBody->GetLinearVelocity()));

			desiredDirection = MathHelper::Interp(curMove, newMove, Time::DeltaTimeF, 10.0f);

			if (lockAtTarget == false)
			{
				movingDirection += desiredDirection * Time::DeltaTimeF * 3.0f;
			}

			desiredLookVector = movingDirection;
		}

	}
	else
	{
		if (lockAtTarget == false)
		{
			movingDirection = MathHelper::Interp(movingDirection, desiredLookVector, Time::DeltaTimeF, 2);
		}

	}

	if (lockAtTarget)
	{
		auto targetRef = Level::Current->FindEntityWithId(target_id);

		desiredLookVector = targetRef->Position - Position;

		movingDirection = MathHelper::Interp(movingDirection, desiredLookVector, Time::DeltaTimeF, 3.5f);

	}

	

	if (length(movingDirection)>1.0f)
	{
		movingDirection = normalize(movingDirection);
	}

	float gravity = LeadBody->GetLinearVelocity().GetY();



	vec3 move = desiredDirection;
	move.y = gravity;

	Physics::SetLinearVelocity(LeadBody, move);

	UpdateAnimations();
	UpdateReturnFromRagdoll();

	if (mesh->WasRended)
	{
		mesh->UpdateHitboxes();
	}
	else if (tickIntervalDelay.Wait() == false)
	{
		mesh->UpdateHitboxes();
	}


	mesh->Position = Position - vec3(0, 1, 0);
	mesh->Rotation = vec3(0, MathHelper::FindLookAtRotation(vec3(), movingDirection).y, 0);

	if (tickIntervalDelay.Wait() == false)
	{
		tickIntervalDelay.AddDelay(0.15f + distance(Position, Camera::position) / 200.0f);
	}
	

}

void NpcBase::UpdateBT()
{

	behaviorTree.GetBlackboard().SetValue("target", Player::Instance->Id);

	auto navPointRef = dynamic_cast<nav_point*>(Level::Current->FindEntityWithName(CurrentTargetNavPoint));

	if (navPointRef != nullptr)
	{
		behaviorTree.GetBlackboard().SetValue("has_navpoint", true);
		behaviorTree.GetBlackboard().SetValue("navpoint_name", CurrentTargetNavPoint);
		behaviorTree.GetBlackboard().SetValue("navpoint_wait_time", navPointRef->WaitTimeAfterReach);
		behaviorTree.GetBlackboard().SetValue("navpoint_has_wait_time", navPointRef->WaitTimeAfterReach > 0);
		behaviorTree.GetBlackboard().SetValue("navpoint_acceptance_radius", navPointRef->acceptanceRadius);
	}
	else
	{
		behaviorTree.GetBlackboard().SetValue("has_navpoint", false);
		behaviorTree.GetBlackboard().SetValue("navpoint_name", "");
		behaviorTree.GetBlackboard().SetValue("navpoint_wait_time", 0.0f);
		behaviorTree.GetBlackboard().SetValue("navpoint_has_wait_time", false);
		behaviorTree.GetBlackboard().SetValue("navpoint_acceptance_radius", 0.0f);
	}

	behaviorTree.GetBlackboard().SetValue("target_follow", target_follow);
	behaviorTree.GetBlackboard().SetValue("target_id", target_id);
	behaviorTree.GetBlackboard().SetValue("target_sees", target_sees);
	behaviorTree.GetBlackboard().SetValue("target_lastSeenPosition", target_lastSeenPosition);
	behaviorTree.GetBlackboard().SetValue("target_underArrest", target_underArrest);
	behaviorTree.GetBlackboard().SetValue("target_attack", target_attack);
	behaviorTree.GetBlackboard().SetValue("target_attackInRange", target_attackInRange);

	behaviorTree.GetBlackboard().SetValue("investigation", currentInvestigation != InvestigationReason::None);
	behaviorTree.GetBlackboard().SetValue("investigation_target", investigation_target);


	if (btEditorEnabled)
	{
		editor.Update(Time::DeltaTimeF);
	}
	else
	{
		behaviorTree.Update(Time::DeltaTimeF);
	}


}

void NpcBase::UpdateObserver()
{

	if (!observer) return;

	auto headTrans = MathHelper::DecomposeMatrix(mesh->GetBoneMatrixWorld("head"));

	observer->forward = MathHelper::TransformVector(vec3(0, -1, 0), headTrans.RotationQuaternion);

	observer->position = headTrans.Position - observer->forward * 0.2f;

	//Logger::Log(Id);

	target_sees = false;

	for (std::weak_ptr<ObservationTarget> weakTarget : observer->visibleTargets)
	{

		auto target = weakTarget.lock();
		if (target->HasTag("violentCrime"))
		{
			target_id = target->ownerId;
			target_follow = true;
			target_underArrest = true;
			target_attack = true;
			target_underArrestExpire = -1;
		}
		else if (target->ownerId == target_id && target_underArrest)
		{
			target_follow = true;
		}
		else if (target->HasTag("illegal_weapon"))
		{
			target_id = target->ownerId;
			target_follow = true;
			target_underArrest = true;
		}
		else if(target->ownerId == target_id && target_underArrest == false)
		{
			StopTargetFollow();
		}

		if (target->ownerId == target_id)
		{
			target_stopUpdateLastSeenPositionDelay.AddDelay(1.0f);
			
			target_sees = true;

		}


	}


}

void NpcBase::UpdateTargetFollow()
{


	if (observer)
	{
		observer->fovDeg = target_follow ? 300 : 100;
	}

	speed = (target_follow || currentInvestigation <= InvestigationReason::Body) ? 5 : 2;


	if (target_follow == false) return;

	auto targetRef = Level::Current->FindEntityWithId(target_id);

	target_attackInRange = distance(targetRef->Position, Position) < 13;

	if (target_underArrest && target_follow && target_sees)
	{
		if (distance2(targetRef->Position, Position) < 15)
		{
			target_underArrestExpire -= Time::DeltaTimeF;
		}

		if (target_underArrestExpire < 0)
		{
			target_attack = true;
		}
	}


	if (target_stopUpdateLastSeenPositionDelay.Wait())
	{
		auto targetRef = Level::Current->FindEntityWithId(target_id);

		target_lastSeenPosition = targetRef->Position;

	}
	
}

void NpcBase::UpdateAnimations()
{

	if (dead) return;

	if (mesh->WasRended)
	{
		animator.UpdatePose = mesh->WasRended;

		if (LeadBody != nullptr)
		{
			animator.movementSpeed = LeadBody->GetLinearVelocity().Length();
		}

		animator.Update();

		auto pose = animator.GetResultPose();

		mesh->PasteAnimationPose(pose);
	}

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

	SERIALIZE_FIELD(target, Rotation);
	SERIALIZE_FIELD(target, desiredDirection);
	SERIALIZE_FIELD(target, movingDirection);
	SERIALIZE_FIELD(target, speed);
	SERIALIZE_FIELD(target, dead);
	SERIALIZE_FIELD(target, animationStateSaveData);
	SERIALIZE_FIELD(target, getFromRagdollAnimationSaveState);
	SERIALIZE_FIELD(target, ragdollPose);

	SERIALIZE_FIELD(target, pathFollow.acceptanceRadius);
	SERIALIZE_FIELD(target, pathFollow.CalculatedTargetLocation);
	SERIALIZE_FIELD(target, pathFollow.FoundTarget);
	SERIALIZE_FIELD(target, pathFollow.reachedTarget);

	
	SERIALIZE_FIELD(target, CurrentTargetNavPoint);

	SERIALIZE_FIELD(target, target_follow);
	SERIALIZE_FIELD(target, target_id);
	SERIALIZE_FIELD(target, target_lastSeenPosition);
	SERIALIZE_FIELD(target, target_stopUpdateLastSeenPositionDelay);
	SERIALIZE_FIELD(target, target_sees);
	SERIALIZE_FIELD(target, target_underArrest);
	SERIALIZE_FIELD(target, target_attack);
	SERIALIZE_FIELD(target, target_underArrestExpire);
	SERIALIZE_FIELD(target, target_attackInRange);

	SERIALIZE_FIELD(target, currentInvestigation);
	SERIALIZE_FIELD(target, investigation_target);

	btSaveState = behaviorTree.SaveState().dump(0);
	SERIALIZE_FIELD(target, btSaveState);
}

void NpcBase::Deserialize(json& source)
{

	Entity::Deserialize(source);

	DESERIALIZE_FIELD(source, Rotation);
	DESERIALIZE_FIELD(source, desiredDirection);
	DESERIALIZE_FIELD(source, movingDirection);
	DESERIALIZE_FIELD(source, speed);
	DESERIALIZE_FIELD(source, dead);
	DESERIALIZE_FIELD(source, animationStateSaveData);
	DESERIALIZE_FIELD(source, getFromRagdollAnimationSaveState);
	DESERIALIZE_FIELD(source, ragdollPose);

	DESERIALIZE_FIELD(source, pathFollow.acceptanceRadius);
	DESERIALIZE_FIELD(source, pathFollow.CalculatedTargetLocation);
	DESERIALIZE_FIELD(source, pathFollow.FoundTarget);
	DESERIALIZE_FIELD(source, pathFollow.reachedTarget);

	DESERIALIZE_FIELD(source, CurrentTargetNavPoint);

	DESERIALIZE_FIELD(source, target_follow);
	DESERIALIZE_FIELD(source, target_id);
	DESERIALIZE_FIELD(source, target_lastSeenPosition);
	DESERIALIZE_FIELD(source, target_stopUpdateLastSeenPositionDelay);
	DESERIALIZE_FIELD(source, target_sees);
	DESERIALIZE_FIELD(source, target_underArrest);
	DESERIALIZE_FIELD(source, target_attack);
	DESERIALIZE_FIELD(source, target_underArrestExpire);
	DESERIALIZE_FIELD(source, target_attackInRange);

	DESERIALIZE_FIELD(source, currentInvestigation);
	DESERIALIZE_FIELD(source, investigation_target);

	DESERIALIZE_FIELD(source, btSaveState);
	if (btSaveState.empty() == false)
	{
		behaviorTree.LoadState(json::parse(btSaveState));
	}

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

void NpcBase::MoveTo(const vec3& target, float acceptanceRadius)
{

	desiredTargetLocation = target;
	pathFollow.acceptanceRadius = acceptanceRadius;
	pathFollow.reachedTarget = false;
	pathFollow.FoundTarget = true;

}

void NpcBase::StopTargetFollow()
{
	target_follow = false;
	target_sees = false;
	target_lastSeenPosition = vec3();
}

void NpcBase::UpdateDebugUI()
{

	if (btEditorEnabled)
	{
		editor.Draw();
	}

	ImGui::Begin(Id.c_str());

	ImGui::DragFloat("activeragdoll strength", &mesh->RagdollPoseFollowStrength,0.1f);
	ImGui::Checkbox("update ragdoll pose", &mesh->UpdateRagdollPose);

	ImGui::Checkbox("bt editor", &btEditorEnabled);

	ImGui::End();

}

void NpcBase::TryStartInvestigation(InvestigationReason reason, vec3 target, string causer)
{

	if (target_follow && reason == InvestigationReason::WeaponFire && causer == target_id)
	{
		target_stopUpdateLastSeenPositionDelay.AddDelay(0.1f);
	}

	if (target_follow && target_underArrest == false && reason > InvestigationReason::Body)
	{
		return;
	}

	if (target_follow && target_underArrest)
	{
		return;
	}



	if (reason >= currentInvestigation)
	{
		return;
	}

	currentInvestigation = reason;
	investigation_target = target;

}

void NpcBase::FinishInvestigation()
{
	currentInvestigation = InvestigationReason::None;
	investigation_target = vec3();
}

REGISTER_ENTITY(NpcBase, "npc_base")