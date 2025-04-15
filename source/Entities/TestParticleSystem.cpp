#include "TestParticleSystem.hpp"

TestParticleSystem* TestParticleSystem::Instance = nullptr;

void TestParticleSystem::SpawnParticleAt(vec3 position, vec3 rotation, vec3 scale)
{
	if (Instance == nullptr)
	{
		TestParticleSystem* testSystem = new TestParticleSystem();

		testSystem->Position = position;
		testSystem->Rotation = rotation;
		testSystem->Scale = scale;
		testSystem->Start();
		Level::Current->AddEntity(testSystem);

		Instance = testSystem;

	}

	Instance->SpawnParticleAtInst(position, rotation, scale);

}

void TestParticleSystem::SpawnParticleAtInst(vec3 position, vec3 rotation, vec3 scale)
{
	Position = position;
	Rotation = rotation;
	Scale = scale;

	for (auto emitter : emitters)
	{

		emitter->Position = position;
		emitter->Rotation = rotation;
		emitter->Scale = scale;

		emitter->AddParticle(emitter->GetNewParticle());
	}

}
