#include <Particle/ParticleSystem.hpp>
#include <Particle/RibbonEmitter.h>

class TrailEmitter : public RibbonEmitter
{
public:

	TrailEmitter() : RibbonEmitter()
	{
		texture = "GameData/textures/particles/trail.png";
		SpawnRate = 100;
		Emitting = true;
		InitialSpawnCount = 1;

	}

	Particle UpdateParticle(Particle particle, float deltaTime) override
	{
		
		particle.Transparency = (particle.deathTime - particle.lifeTime) / particle.deathTime;
		particle.Size = mix(0.0f, 0.03f, (particle.deathTime - particle.lifeTime) / particle.deathTime);
		return particle;
	}

	Particle GetNewParticle() override
	{
		Particle particle = ParticleEmitter::GetNewParticle();
		
		particle.Size = 0.1f;
		particle.Color = vec4(0.5, 0, 0, 0.8);
		particle.Transparency = 0.8;
		particle.deathTime = 0.1f;

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
