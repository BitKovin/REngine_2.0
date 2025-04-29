#include <Particle/ParticleSystem.hpp>
#include <Particle/RibbonEmitter.h>

class TrailEmitter : public RibbonEmitter
{
public:

	TrailEmitter() : RibbonEmitter()
	{
		texture = "GameData/Textures/Particles/trail.png";
		SpawnRate = 60;
		Emitting = true;
		InitialSpawnCount = 2;

	}

	Particle GetNewParticle() override
	{
		Particle particle = ParticleEmitter::GetNewParticle();
		
		particle.Size = 0.1f;
		particle.Color = vec4(0.5, 0, 0, 0.8);
		particle.Transparency = 0.8;
		particle.deathTime = 1;

		return particle;
	}

private:

};

class BulletTrail : public ParticleSystem
{
public:
	BulletTrail()
	{
		emitters.push_back(new TrailEmitter());
	}


private:

};

REGISTER_ENTITY(BulletTrail, "bullet_trail")
