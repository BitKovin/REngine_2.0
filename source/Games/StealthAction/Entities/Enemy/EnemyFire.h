#pragma once

#include <NetworkedEntity.h>
#include <EntityHandle.h>

#include <Particle/ParticleSystem.hpp>

class EnemyFire : public NetworkedEntity
{
public:

	EnemyFire()
	{
		ClassName = "enemyFire";
		SaveGame = true;

	}

	EnemyFire(Entity* targetRef)
	{
		SaveGame = true;
		ClassName = "enemyFire";
		target = EntityHandle::FromEntity(targetRef);

	}

	EntityHandle target;
	
	void Start() override
	{
		NetworkedEntity::Start();

		if (fireParticles == nullptr)
		{
			fireParticles = (ParticleSystem*)Spawn("fire_burning");
			fireParticles->Start();
		}
	}

	void LateUpdate() override;
	void Update() override;

	void NetSerialize(NetPacket& packet) override
	{
		target.Write(packet);
		packet.WriteFloat(Damage);
	}

	void NetDeserialize(NetPacket& packet) override
	{
		target = EntityHandle::Read(packet);
		Damage = packet.ReadFloat();
	}

	void Serialize(json& targetJson) override
	{
		bool   handleNetworked = target.isNetworked;
		uint64_t handleNetId = target.networkId;
		string   handleEntId = target.entityId;
		SERIALIZE_FIELD(targetJson, handleNetworked);
		SERIALIZE_FIELD(targetJson, handleNetId);
		SERIALIZE_FIELD(targetJson, handleEntId);
		SERIALIZE_FIELD(targetJson, LifeTime);
	}
	
	void Deserialize(json& source) override
	{
		bool   handleNetworked = false;
		uint64_t handleNetId = 0;
		string   handleEntId = {};
		DESERIALIZE_FIELD(source, handleNetworked);
		DESERIALIZE_FIELD(source, handleNetId);
		DESERIALIZE_FIELD(source, handleEntId);
		target.isNetworked = handleNetworked;
		target.networkId = handleNetId;
		target.entityId = handleEntId;

		DESERIALIZE_FIELD(source, LifeTime);
	}


	void Destroy() override
	{
		if (fireParticles != nullptr)
		{
			fireParticles->DestroyWithDelay(1);
			fireParticles = nullptr;
		}
		NetworkedEntity::Destroy();
	}

	float Damage = 10;

	float LifeTime = 5.0f;

private:

	ParticleSystem* fireParticles = nullptr;

};

