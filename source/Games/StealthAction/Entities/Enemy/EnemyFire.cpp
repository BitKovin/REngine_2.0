#include "EnemyFire.h"

void EnemyFire::LateUpdate()
{

	Entity* targetRef = target.Resolve();

	Position = targetRef->GetBoundingBox().Center();

	NetworkedEntity* netTarget = dynamic_cast<NetworkedEntity*>(targetRef);

	if (netTarget)
	{
		SetOwner(netTarget->networkOwner);
	}

	if(fireParticles)
	{
		fireParticles->Position = Position;
	}

}

void EnemyFire::Update()
{

	Entity* targetRef = target.Resolve();

	LifeTime -= Time::DeltaTime;

	if (targetRef == nullptr)
	{
		Destroy();
		return;
	}

	if (LifeTime <= 0 && isOwned)
	{
		Destroy();
		return;
	}

	if (LifeTime <= 1)
	{
		if(fireParticles)
			fireParticles->StopAll();
	}

	NetworkedEntity* netTarget = dynamic_cast<NetworkedEntity*>(targetRef);

	if (netTarget)
	{
		SetOwner(netTarget->networkOwner);

		if (netTarget->isOwned)
		{
			netTarget->OnDamage(Damage * Time::DeltaTime, this);
		}

	}
	else
	{
		targetRef->OnDamage(Damage * Time::DeltaTime, this);
	}



}


REGISTER_ENTITY(EnemyFire, "enemyFire")