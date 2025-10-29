#include "NpcBase.h"

#include <SoundSystem/FmodEventInstance.h>

#include <Particle/GlobalParticleSystem.hpp>

#include <Input.h>

#include <imgui/imgui.h>

#include "../Player/Player.hpp"

#include "Ai/nav_point.h"

#include <RandomHelper.h>

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

	LeadBody = Physics::CreateCharacterBody(this, Position, 0.5, 2, 100);


	Physics::SetGravityFactor(LeadBody, 4);

	desiredDirection = MathHelper::XZ(MathHelper::GetForwardVector(Rotation));
	movingDirection = desiredDirection;

	pathFollow.Async = false;

	VoiceSoundPlayer = SoundPlayer::Create();
	VoiceSoundPlayer->SetSound(FmodEventInstance::Create("event:/Character/Dialogue"));
	SetupSoundPlayer(VoiceSoundPlayer);


	editor = BehaviorTreeEditor(&behaviorTree);
	editor.Init();

	behaviorTree.LoadFromFile("GameData/behaviourTrees/general.bt");
	behaviorTree.Owner = this;
	behaviorTree.Start();

	observationTarget = AiPerceptionSystem::CreateTarget(Position, Id, {});
	observationTarget->noticeMaxDistanceMultiplier = 0.3;

	observer = AiPerceptionSystem::CreateObserver(Position + vec3(0, 0.7, 0), movingDirection, 90);
	observer->owner = Id;
	observer->ownerPtr = this;

}


void NpcBase::Death()
{

	if (dead) return;

	needToInvestigateBody = true;

	//mesh->ClearHitboxes();
	mesh->StartRagdoll();

	animator.PainProgress = 1;

	mesh->SetAnimationPaused(true);
	Physics::SetLinearVelocity(LeadBody, vec3(0));

	VoiceSoundPlayer->Stop();
	VoiceSoundPlayer->Volume = 0;

	//Physics::SetBodyType(LeadBody, BodyType::None);
	//Physics::SetCollisionMask(LeadBody, BodyType::World);

	Physics::DestroyBody(LeadBody);
	LeadBody = nullptr;

	dead = true;

	//Tags.clear();

	AiPerceptionSystem::RemoveObserver(observer);
	observer = nullptr;



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

	getFromRagdollAnimation->PlayAnimation(onFront ? "front" : "back", false, 0);

	auto pelvisTransformWorld = MathHelper::DecomposeMatrix(mesh->GetBoneMatrixWorld("pelvis"));

	Position = pelvisTransformWorld.Position + vec3(0, 1, 0);
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

	pelvisTransform.Position = vec3(0, 0.5, 0);
	pelvisTransform.RotationQuaternion = MathHelper::GetRotationQuaternion(vec3(0, oldRot - mesh->Rotation.y, 0)) * pelvisTransform.RotationQuaternion;

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


void NpcBase::PlayPhrace(std::string name)
{

	if (VoiceSoundPlayer == nullptr) return;

	if (globalPhraceDelay.Wait()) return;

	const string eventPath = vo_base_event_path + name;

	VoiceSoundPlayer->Stop();
	VoiceSoundPlayer->SetSound(FmodEventInstance::Create(eventPath));
	VoiceSoundPlayer->Play();

	globalPhraceDelay.AddDelay(1.0f);

}

void NpcBase::AsyncUpdate()
{

	pathFollow.WaitToFinish();

	if (dead == false)
	{
		UpdateObserver();
		UpdateTargetFollow();
		UpdateBT();
	}

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

	auto headHitbox = mesh->FindHitboxByName("neck_01");

	if (headHitbox)
		VoiceSoundPlayer->Position = FromPhysics(headHitbox->GetPosition());

	VoiceSoundPlayer->Velocity = FromPhysics(LeadBody->GetLinearVelocity());


	bool lockAtTarget = target_attack && target_sees && target_attackInRange && isGuard;

	vec3 desiredDirection = vec3(0);

	if (pathFollow.reachedTarget == false || pathFollow.CalculatedPath == false)
	{
		pathFollow.UpdateStartAndTarget(Position, desiredTargetLocation);
		pathFollow.TryPerform();
	}

	if (pathFollow.reachedTarget == false && pathFollow.CalculatedPath)
	{

		vec3 dir = MathHelper::XZ(pathFollow.CalculatedTargetLocation - Position);

		if (length(dir) > 0.001)
		{
			vec3 newMove = normalize(dir) * speed;

			vec3 curMove = MathHelper::XZ(FromPhysics(LeadBody->GetLinearVelocity()));

			desiredDirection = MathHelper::Interp(curMove, newMove, Time::DeltaTimeF, 5.0f);

			if (lockAtTarget == false)
			{
				movingDirection += desiredDirection * Time::DeltaTimeF * 3.0f;
			}

			desiredLookVector = movingDirection;
		}

	}
	else
	{

		vec3 curMove = MathHelper::XZ(FromPhysics(LeadBody->GetLinearVelocity()));

		desiredDirection = MathHelper::Interp(curMove, vec3(0), Time::DeltaTimeF, 8.0f);

		if (lockAtTarget == false)
		{
			movingDirection = MathHelper::Interp(movingDirection, desiredLookVector, Time::DeltaTimeF, 2);
		}

	}

	if (lockAtTarget)
	{
		auto targetRef = Level::Current->FindEntityWithId(target_id);

		desiredLookVector = targetRef->Position - Position;

		movingDirection = MathHelper::Interp(movingDirection, desiredLookVector, Time::DeltaTimeF, 4.0f);

	}



	if (length(movingDirection) > 1.0f)
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
void NpcBase::LateUpdate()
{
	UpdateObservationTarget();
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

	behaviorTree.GetBlackboard().SetValue("isGuard", isGuard);

	behaviorTree.GetBlackboard().SetValue("target_follow", target_follow);
	behaviorTree.GetBlackboard().SetValue("target_id", target_id);
	behaviorTree.GetBlackboard().SetValue("target_sees", target_sees);
	behaviorTree.GetBlackboard().SetValue("target_lastSeenPosition", target_lastSeenPosition);
	behaviorTree.GetBlackboard().SetValue("target_underArrest", target_underArrest);
	behaviorTree.GetBlackboard().SetValue("target_attack", target_attack);
	behaviorTree.GetBlackboard().SetValue("target_attackInRange", target_attackInRange);

	behaviorTree.GetBlackboard().SetValue("investigation", currentInvestigation != InvestigationReason::None);
	behaviorTree.GetBlackboard().SetValue("investigation_target", investigation_target);

	behaviorTree.GetBlackboard().SetValue("report_to_guard", report_to_guard);
	behaviorTree.GetBlackboard().SetValue("closestGuard", closestGuard);
	behaviorTree.GetBlackboard().SetValue("found_guard", found_guard);

	behaviorTree.GetBlackboard().SetValue("flee_target", flee_target);

	if (investigation_changed)
	{
		behaviorTree.GetBlackboard().SetValue("investigation", false); //stopping investigation for one frame to reset logic
		investigation_changed = false;
	}

	if (btEditorEnabled)
	{
		editor.Update(Time::DeltaTimeF / 1.0f);
	}
	else
	{
		behaviorTree.Update(Time::DeltaTimeF);
	}


}

void NpcBase::UpdateObserver()
{

	if (!observer) return;

	observer->searchForTriggeredNpc = !target_attack;

	if (distance(Camera::finalizedPosition, Position) < 40)
	{

		MathHelper::Transform headTrans;

		headTrans = MathHelper::DecomposeMatrix(mesh->GetBoneMatrixWorld("head"));

		observer->forward = MathHelper::TransformVector(vec3(0, -1, 0), headTrans.RotationQuaternion);

		observer->position = headTrans.Position - observer->forward * 0.2f;
	}
	else
	{
		observer->forward = movingDirection;

		observer->position = Position + vec3(0, 0.65, 0);
	}




	//Logger::Log(Id);

	target_sees = false;

	for (std::shared_ptr<ObservationTarget> target : observer->visibleTargets)
	{


		bool foundCrime = false;



		if (target->HasTag("violentCrime"))
		{

			TryCommitCrime(Crime::WeaponFire, target->ownerId, target->position);

			foundCrime = true;
		}
		else if (target->HasTag("body"))
		{
			for (std::shared_ptr<ObservationTarget> target2 : observer->visibleTargets)
			{


				if (target2->HasTag("player"))
				{

					if (distance(target->position, target2->position) < 5)
					{

						TryCommitCrime(Crime::NearBody, target2->ownerId, target2->position);

						foundCrime = true;
					}

				}

			}

			if (foundCrime == false)
			{
				TryStartInvestigation(InvestigationReason::Body, target->position, target->ownerId);
			}

		}

		if (foundCrime == false && currentInvestigation == InvestigationReason::WeaponFire
			&& distance(target->position, investigation_target) < 4)
		{

			TryCommitCrime(Crime::WeaponFireSound, target->ownerId, target->position);

		}
		else if (target->ownerId == target_id && target_underArrest)
		{

			if (target_follow == false)
			{
				PlayPhrace("target_found");

				report_to_guard = true;

			}

			target_follow = true;

		}
		else if (target->HasTag("illegal_weapon"))
		{

			TryCommitCrime(Crime::WeaponHolding, target->ownerId, target->position);


		}
		else if (target->HasTag("in_trouble"))
		{
			TryStartInvestigation(InvestigationReason::NpcInTrouble, target->position, target->ownerId);
		}
		else if (target->ownerId == target_id && target_underArrest == false)
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

void NpcBase::UpdateObservationTarget()
{

	observationTarget->tags.clear();

	observationTarget->active = false; // needToInvestigateBody || (target_attack && target_follow);
	observationTarget->isTriggeredNpc = false;

	if (dead)
	{
		observationTarget->position = FromPhysics(mesh->FindHitboxByName("pelvis")->GetPosition()) + vec3(0, 0.5, 0);
	}
	else
	{
		observationTarget->position = Position + vec3(0, 0.5, 0);
	}

	if (dead)
	{
		if (needToInvestigateBody)
		{
			observationTarget->tags.insert("body");

			observationTarget->active = true;

		}
	}

	else
	{
		if (isGuard)
		{
			observationTarget->tags = { "guard" };
		}
		else
		{
			observationTarget->tags = { "civilian" };
		}

		if (target_underArrest && target_follow)
		{

			observationTarget->tags.insert("in_trouble");
			observationTarget->active = true;
			observationTarget->isTriggeredNpc = true;
		}
		else
		{
			if (isGuard && (target_attack == false || target_follow == false))
			{
				observationTarget->tags.emplace("guard_safe");
			}
		}

	}




}

void NpcBase::UpdateTargetFollow()
{


	if (observer)
	{

		if (target_follow || currentInvestigation == InvestigationReason::WeaponFire)
		{
			observer->fovDeg = 400;
		}
		else if (currentInvestigation <= InvestigationReason::Body)
		{
			observer->fovDeg = 200;
		}
		else
		{
			observer->fovDeg = 120;
		}

	}

	speed = (target_follow || currentInvestigation <= InvestigationReason::Body) ? 5 : 2;


	if (target_follow || report_to_guard)
	{
		FindClosestGuard();
	}

	if (target_follow == false) return;

	if (target_underArrest && observer)
	{
		auto observers = AiPerceptionSystem::GetObserversInRadius(observer->position, isGuard ? 13 : 7);

		for (auto ob : observers)
		{
			NpcBase* npcRef = dynamic_cast<NpcBase*>(ob->ownerPtr);

			if (npcRef == nullptr)
				npcRef = dynamic_cast<NpcBase*>(Level::Current->FindEntityWithId(ob->owner));

			if (npcRef)
			{
				ShareTargetKnowlageWith(npcRef);
			}

		}

	}

	auto targetRef = Level::Current->FindEntityWithId(target_id);

	target_attackInRange = distance(targetRef->Position, Position) < 13;

	if (target_attack && target_sees)
	{
		currentInvestigation = InvestigationReason::None;
	}

	if (target_underArrest && target_follow && target_sees && target_attack == false)
	{
		if (distance2(targetRef->Position, Position) < 15)
		{
			target_underArrestExpire -= Time::DeltaTimeF;
		}

		if (target_underArrestExpire < 0)
		{
			target_attack = true;
			PlayPhrace("arrest_final");
		}
	}


	if (target_stopUpdateLastSeenPositionDelay.Wait())
	{
		auto targetRef = Level::Current->FindEntityWithId(target_id);

		target_lastSeenPosition = targetRef->Position;
		target_lastSeenTime = Time::GameTime;

	}

}

void NpcBase::UpdateAnimations()
{

	if (dead) return;

	if (isGuard)
	{
		animator.weapon_holds = target_underArrest;
		animator.weapon_ready = target_follow && target_underArrest;
		animator.weapon_aims = target_attack && animator.weapon_ready && target_attackInRange;
	}



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
	SoundManager::LoadBankFromPath("GameData/sounds/banks/Desktop/VO.bank");

	mesh->LoadFromFile("GameData/models/npc/base.glb");
	mesh->CreateHitboxes(this);

	getFromRagdollAnimation = new Animation(this);
	getFromRagdollAnimation->LoadFromFile("GameData/animations/npc/standUp.glb");

	animator.LoadAssetsIfNeeded();

	mesh->ColorTexture = AssetRegistry::GetTextureFromFile("GameData/cat.png");



}

void NpcBase::ShareTargetKnowlageWith(NpcBase* anotherNpc)
{

	bool hasChanges = false;

	if (target_underArrest && anotherNpc->target_underArrest == false)
	{
		hasChanges = true;
	}

	if (target_underArrestExpire < anotherNpc->target_underArrestExpire + 0.2f && anotherNpc->target_underArrest == false)
	{
		hasChanges = true;
	}

	if (target_attack && anotherNpc->target_attack == false)
	{
		hasChanges = true;
	}

	if (target_lastSeenTime > anotherNpc->target_lastSeenTime + 0.5f)
	{
		hasChanges = true;
	}

	if (hasChanges == false) return;

	if (Physics::LineTrace(Position, anotherNpc->Position, BodyType::WorldOpaque).hasHit) return;

	if (target_underArrest)
	{
		anotherNpc->target_underArrest = true;
		anotherNpc->target_id = target_id;
	}

	if (target_underArrestExpire < anotherNpc->target_underArrestExpire)
	{
		anotherNpc->target_underArrestExpire = target_underArrestExpire;
	}

	if (target_attack)
	{
		anotherNpc->target_attack = true;
	}

	if (target_lastSeenTime > anotherNpc->target_lastSeenTime)
	{
		anotherNpc->target_lastSeenPosition = target_lastSeenPosition;
		anotherNpc->target_lastSeenTime = target_lastSeenTime;
	}

}

bool NpcBase::TryCommitCrime(Crime crime, std::string offender, vec3 pos)
{

	if (isGuard == false)
	{
		if (crime < Crime::Group_Arrest)
		{
			target_follow = true;
			report_to_guard = true;
		}
	}


	if (crime >= currentCrime) return true; // Note: Returns false if no change, but since we need to know if committed (changed), adjust to return bool

	bool wasUnderArrest = target_underArrest;
	bool wasAttack = target_attack;


	currentCrime = crime;

	target_id = offender;

	target_stopUpdateLastSeenPositionDelay.AddDelay(0.5f);

	if (crime < Crime::Group_Attack)
	{
		target_underArrest = true;
		target_attack = true;
		target_underArrestExpire = -1.0f;
	}

	if (crime < Crime::Group_Arrest)
	{
		target_underArrest = true;
		target_follow = true;

		if (isGuard == false)
		{
			report_to_guard = true;
		}

	}

	if (crime < Crime::Group_Follow && isGuard)
	{
		target_follow = true;
	}

	if (target_attack && !wasAttack)
	{
		PlayPhrace("arrest_final");
	}
	else if (target_underArrest && !wasUnderArrest)
	{
		if (crime == Crime::WeaponHolding || crime == Crime::WeaponFireSound)
		{
			PlayPhrace("weapon_surrender");
		}
		else if (crime == Crime::NearBody)
		{
			PlayPhrace("arrest");
		}
	}

	return true;
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
	SERIALIZE_FIELD(target, target_lastSeenTime);
	SERIALIZE_FIELD(target, target_stopUpdateLastSeenPositionDelay);
	SERIALIZE_FIELD(target, target_sees);
	SERIALIZE_FIELD(target, target_underArrest);
	SERIALIZE_FIELD(target, target_attack);
	SERIALIZE_FIELD(target, target_underArrestExpire);
	SERIALIZE_FIELD(target, target_attackInRange);

	SERIALIZE_FIELD(target, report_to_guard);
	SERIALIZE_FIELD(target, found_guard);
	SERIALIZE_FIELD(target, closestGuard);

	SERIALIZE_FIELD(target, currentInvestigation);
	SERIALIZE_FIELD(target, investigation_target);
	SERIALIZE_FIELD(target, investigation_targetId);
	SERIALIZE_FIELD(target, investigation_changed);
	SERIALIZE_FIELD(target, needToInvestigateBody);

	SERIALIZE_FIELD(target, currentCrime);

	SERIALIZE_FIELD(target, flee_target);

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
	DESERIALIZE_FIELD(source, target_lastSeenTime);
	DESERIALIZE_FIELD(source, target_stopUpdateLastSeenPositionDelay);
	DESERIALIZE_FIELD(source, target_sees);
	DESERIALIZE_FIELD(source, target_underArrest);
	DESERIALIZE_FIELD(source, target_attack);
	DESERIALIZE_FIELD(source, target_underArrestExpire);
	DESERIALIZE_FIELD(source, target_attackInRange);

	DESERIALIZE_FIELD(source, report_to_guard);
	DESERIALIZE_FIELD(source, found_guard);
	DESERIALIZE_FIELD(source, closestGuard);

	DESERIALIZE_FIELD(source, currentInvestigation);
	DESERIALIZE_FIELD(source, investigation_target);
	DESERIALIZE_FIELD(source, investigation_targetId);
	DESERIALIZE_FIELD(source, investigation_changed);
	DESERIALIZE_FIELD(source, needToInvestigateBody);

	DESERIALIZE_FIELD(source, currentCrime);

	DESERIALIZE_FIELD(source, flee_target);

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

void NpcBase::PrepareToStartMovement()
{

	pathFollow.WaitToFinish();
	pathFollow.CalculatedPath = false;
	pathFollow.reachedTarget = false;
	pathFollow.isPerformingDelay.AddDelay(-1);

}

void NpcBase::StopMovement()
{

	pathFollow.WaitToFinish();
	pathFollow.CalculatedPath = true;
	pathFollow.reachedTarget = true;
	//desiredDirection = vec3(0);

}

void NpcBase::MoveTo(const vec3& target, float acceptanceRadius)
{
	desiredTargetLocation = target;
	pathFollow.acceptanceRadius = acceptanceRadius;

}

void NpcBase::StopTargetFollow()
{

	if (target_underArrest)
	{
		PlayPhrace("target_lost");
	}

	target_follow = false;
	target_sees = false;
	target_lastSeenPosition = vec3();
	pathFollow.reachedTarget = false;
	pathFollow.FoundTarget = true;
	currentCrime = Crime::None;

}

void NpcBase::BodyInvestigated()
{
	needToInvestigateBody = false;
}

void NpcBase::UpdateDebugUI()
{

	if (btEditorEnabled)
	{
		editor.Draw();
	}

	ImGui::Begin(Id.c_str());

	ImGui::DragFloat("activeragdoll strength", &mesh->RagdollPoseFollowStrength, 0.1f);
	ImGui::Checkbox("update ragdoll pose", &mesh->UpdateRagdollPose);

	ImGui::Checkbox("bt editor", &btEditorEnabled);

	ImGui::End();

}

void NpcBase::FindClosestGuard()
{

	if (findGuardCooldown.Wait()) return;

	std::vector<std::shared_ptr<ObservationTarget>> npcsInRadius;
	
	if(report_to_guard)
		npcsInRadius = AiPerceptionSystem::GetTargetsInRadiusWithTagOrdered(Position, 150, "guard_safe");

	if (npcsInRadius.size() > 0)
	{
		closestGuard = npcsInRadius[0]->ownerId;
		found_guard = true;
	}
	else
	{
		closestGuard = "";
		found_guard = false;

		auto fleePath = NavigationSystem::FindFleePathSimple(Position, target_lastSeenPosition, 50, 4, speed);

		if (fleePath.size() > 0)
		{
			flee_target = fleePath[fleePath.size() - 1];
		}
		else
		{
			flee_target = normalize(Position - target_lastSeenPosition) * 10.0f;
		}

	}

	findGuardCooldown.AddDelay(0.3 + RandomHelper::RandomFloat()/3.0f);

}

void NpcBase::TryStartInvestigation(InvestigationReason reason, vec3 target, string causer, bool sharedByNpc)
{

	if (target_follow && reason == InvestigationReason::WeaponFire && causer == target_id)
	{
		target_stopUpdateLastSeenPositionDelay.AddDelay(0.2f);
	}

	if (target_follow && target_underArrest == false && reason > InvestigationReason::Body)
	{
		return;
	}

	if (target_follow && target_underArrest)
	{
		return;
	}

	if (isGuard == false && reason == InvestigationReason::NpcInTrouble) return;

	if (reason >= currentInvestigation)
	{
		return;
	}

	if (reason == InvestigationReason::LoudNoise || reason == InvestigationReason::Noise)
	{

		if(sharedByNpc == false)
			PlayPhrace("heard_sound");
	}

	if (reason == InvestigationReason::WeaponFire)
	{
		PlayPhrace("shots_fired");
	}

	if (reason < InvestigationReason::LoudNoise && isGuard == false)
	{
		report_to_guard = true;
	}

	investigation_changed = true;

	currentInvestigation = reason;
	investigation_target = target;
	investigation_targetId = causer;
}

void NpcBase::FinishInvestigation()
{


	if (isGuard)
	{
		if (currentInvestigation == InvestigationReason::Body)
		{
			if (investigation_targetId.empty() == false)
			{
				NpcBase* npcRef = dynamic_cast<NpcBase*>(Level::Current->FindEntityWithId(investigation_targetId));

				if (npcRef)
				{
					npcRef->BodyInvestigated();
					PlayPhrace("dead_body");
				}

			}
		}
	}
	else
	{

		if (found_guard)
		{
			auto guardRef = dynamic_cast<NpcBase*>(Level::Current->FindEntityWithId(closestGuard));

			if (currentInvestigation != InvestigationReason::None)
			{

				if (guardRef)
				{
					guardRef->TryStartInvestigation(currentInvestigation, investigation_target, Id, true);
					report_to_guard = false;
				}

			}
			else
			{
				if (guardRef->target_lastSeenTime < target_lastSeenTime + 0.5f)
				{
					guardRef->target_lastSeenTime = target_lastSeenTime;
					guardRef->target_lastSeenPosition = target_lastSeenPosition;
					guardRef->TryStartInvestigation(InvestigationReason::TargetSeen, target_lastSeenPosition, Id, true);

				}
			}

		}

	}



	currentInvestigation = InvestigationReason::None;
	investigation_target = vec3();

}

REGISTER_ENTITY(NpcBase, "npc_base")