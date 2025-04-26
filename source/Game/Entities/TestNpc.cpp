#include "TestNpc.hpp"
#include "Player/Player.hpp"

#include <Particle/GlobalParticleSystem.hpp>

#include <Navigation/Navigation.hpp>

REGISTER_ENTITY(TestNpc, "testnpc")

void TestNpc::ProcessAnimationEvent(AnimationEvent& event)
{
	
	Logger::Log(event.eventName);

	if (event.eventName == "attack_start")
	{
		attackingDamage = true;
	}

	if (event.eventName == "attack_end")
	{
		mesh->PlayAnimation("run", true, 0.2f);
		attacking = false;
		attackingDamage = false;
	}

}

void TestNpc::Attack()
{

	inAttackDelay.AddDelay(2.5f);
	mesh->PlayAnimation("attack");
	attacking = true;

}

void TestNpc::Death()
{

	if (dead) return;

	mesh->ClearHitboxes();
	mesh->PlayAnimation("death");
	Physics::SetLinearVelocity(LeadBody, vec3(0));

	DeathSoundPlayer->Play();

	//Physics::SetBodyType(LeadBody, BodyType::None);
	//Physics::SetCollisionMask(LeadBody, BodyType::World);

	Physics::DestroyBody(LeadBody);
	LeadBody = nullptr;

	dead = true;

}

void TestNpc::OnPointDamage(float Damage, vec3 Point, vec3 Direction, string bone, Entity* DamageCauser, Entity* Weapon)
{
	Entity::OnPointDamage(Damage, Point, Direction, bone, DamageCauser, Weapon);

	GlobalParticleSystem::SpawnParticleAt("hit_flesh", Point, MathHelper::FindLookAtRotation(vec3(0), Direction), vec3(10));

}

void TestNpc::OnDamage(float Damage, Entity* DamageCauser, Entity* Weapon)
{

	Health -= Damage;

	if (Health <= 0)
	{
		Death();
	}


}

void TestNpc::UpdateAttackDamage()
{

	if (attackingDamage == false) return;

	auto hit = Physics::SphereTrace(Position, MathHelper::GetForwardVector(mesh->Rotation)*0.75f + Position, 0.2f, BodyType::World | BodyType::CharacterCapsule, { LeadBody });

	if (hit.hasHit)
	{
		if (hit.entity->HasTag("player"))
		{
			hit.entity->OnPointDamage(20, hit.shapePosition, MathHelper::FastNormalize(hit.shapePosition - Position), "", this, this);
			attackingDamage = false;
			Logger::Log(hit.entity->ClassName);
		}
	}

}

void TestNpc::AsyncUpdate()
{

	DeathSoundPlayer->Position = Position;
	HurtSoundPlayer->Position = Position;
	StunSoundPlayer->Position = Position;
	AttackSoundPlayer->Position = Position;

	

	//mesh->UpdatePose = mesh->WasRended;

	mesh->Update();
	

	auto animEvents = mesh->PullAnimationEvents();

	for (auto& event : animEvents)
	{
		ProcessAnimationEvent(event);
	}

	mesh->Position = Position - vec3(0, 1, 0);

	//auto testTrans = MathHelper::DecomposeMatrix(mesh->GetBoneMatrixWorld("Bone.013"));

	//DebugDraw::Line(testTrans.Position - vec3(0, 1, 0), testTrans.Position + vec3(0, 2, 0), 0.005f, 0.2);

	auto rootMotion = mesh->PullRootMotion();

	if (LeadBody != nullptr)
	{
		Physics::MoveBody(LeadBody, rootMotion.Position);

		if (rootMotion.Position != vec3())
		{
			Physics::SetLinearVelocity(LeadBody, vec3(0, LeadBody->GetLinearVelocity().GetY(), 0));
		}

	}
	else
	{
		Position += rootMotion.Position;
	}
	
	if (rootMotion.Rotation != vec3())
	{
		mesh->Rotation += rootMotion.Rotation;
		movingDirection = MathHelper::GetForwardVector(mesh->Rotation);
	}


	mesh->UpdateHitboxes();

	if (dead) return;

	UpdateAttackDamage();

	DeathSoundPlayer->Velocity = FromPhysics(LeadBody->GetLinearVelocity());
	HurtSoundPlayer->Velocity = FromPhysics(LeadBody->GetLinearVelocity());
	StunSoundPlayer->Velocity = FromPhysics(LeadBody->GetLinearVelocity());
	AttackSoundPlayer->Velocity = FromPhysics(LeadBody->GetLinearVelocity());

	Entity* target = Player::Instance;

	if (attacking)
	{

		if (target)
		{
			pathFollow.UpdateStartAndTarget(Position, target->Position);
			pathFollow.TryPerform();

		}

		return;
	}

	vec3 lookAtDir = MathHelper::FastNormalize(target->Position - Position);

	if (distance(target->Position, Position) < 5 
		&& dot(MathHelper::GetForwardVector(mesh->Rotation), lookAtDir) > 0.7)
	{
		
		Attack();
	}

	if (target)
	{
		pathFollow.UpdateStartAndTarget(Position, target->Position);
		pathFollow.TryPerform();

	}
	if (pathFollow.FoundTarget)
	{
		desiredDirection = MathHelper::FastNormalize(MathHelper::XZ(pathFollow.CalculatedTargetLocation - Position));
	}
	

	movingDirection = mix(movingDirection, desiredDirection, Time::DeltaTime*5);

	movingDirection = MathHelper::FastNormalize(movingDirection);

	// Get the current horizontal velocity (preserving the vertical component from physics)
	vec3 currentVelocity = FromPhysics(LeadBody->GetLinearVelocity());
	vec3 currentHorizontalVel(currentVelocity.x, 0.0f, currentVelocity.z);

	// Determine the desired horizontal velocity (5.0f is the intended speed)
	vec3 desiredHorizontalVel = movingDirection * 5.0f;

	// Calculate the change in velocity you need to achieve over the current frame
	// Using Time::DeltaTime (dt) to convert velocity difference to the required acceleration
	float dt = Time::DeltaTime;
	vec3 neededAcceleration = (desiredHorizontalVel - currentHorizontalVel) / dt;

	// Retrieve the body mass to calculate the needed force (F = m * a)
	float mass = 50;
	vec3 forceToApply = neededAcceleration * mass;

	// Only apply horizontal forces to avoid interfering with the vertical (gravity, jump, etc.)
	vec3 horizontalForce(forceToApply.x, 0.0f, forceToApply.z);

	// Apply the calculated force to the body
	LeadBody->AddForce(ToPhysics(horizontalForce));

	Physics::Activate(LeadBody);

	mesh->Rotation = vec3(0,MathHelper::FindLookAtRotation(vec3(), movingDirection).y, 0);

}

void TestNpc::Serialize(json& target)
{

	Entity::Serialize(target);

	animationStateSaveData = mesh->GetAnimationState();

	Rotation = mesh->Rotation;

	SERIALIZE_FIELD(target, Rotation)
	SERIALIZE_FIELD(target, desiredDirection)
	SERIALIZE_FIELD(target, movingDirection)
	SERIALIZE_FIELD(target, dead)
	SERIALIZE_FIELD(target, animationStateSaveData)
	SERIALIZE_FIELD(target, attacking)
	SERIALIZE_FIELD(target, stuned)
	SERIALIZE_FIELD(target, attackingDamage)
}

void TestNpc::Deserialize(json& source)
{

	Entity::Deserialize(source);

	DESERIALIZE_FIELD(source, Rotation)
	DESERIALIZE_FIELD(source, desiredDirection)
	DESERIALIZE_FIELD(source, movingDirection)
	DESERIALIZE_FIELD(source, dead)
	DESERIALIZE_FIELD(source, animationStateSaveData)
	DESERIALIZE_FIELD(source, attacking)
	DESERIALIZE_FIELD(source, stuned)
	DESERIALIZE_FIELD(source, attackingDamage)


	Physics::SetBodyPosition(LeadBody, Position);


	if (dead)
	{
		Physics::DestroyBody(LeadBody);
		LeadBody = nullptr;
		mesh->ClearHitboxes();
	}


	mesh->Rotation = Rotation;

	mesh->SetAnimationState(animationStateSaveData);
	mesh->Update(0);
	mesh->PullRootMotion();

}

void TestNpc::LoadAssets()
{
	mesh->LoadFromFile("GameData/dog.glb");
	mesh->CreateHitboxes(this);
	mesh->PlayAnimation("run",true);
	mesh->SetLooped(true);
	mesh->ColorTexture = AssetRegistry::GetTextureFromFile("GameData/cat.png");

	DeathSoundPlayer->Sound = SoundManager::GetSoundFromPath("GameData/Sounds/Dog/Death.wav");
	HurtSoundPlayer->Sound = SoundManager::GetSoundFromPath("GameData/Sounds/Dog/Death.wav");
	StunSoundPlayer->Sound = SoundManager::GetSoundFromPath("GameData/Sounds/Dog/Death.wav");
	AttackSoundPlayer->Sound = SoundManager::GetSoundFromPath("GameData/Sounds/Dog/Death.wav");


}


