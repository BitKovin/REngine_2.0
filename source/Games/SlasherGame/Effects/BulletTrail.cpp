#include <Particle/ParticleSystem.hpp>
#include <Particle/RibbonEmitter.h>

// Tracer of a projectile (COMBAT_DESIGN.md §9.4): a camera-facing ribbon from where the shot was a moment ago to
// where it is, a warm streak added onto the scene (additive: it reads as light against the dark levels). The texture
// is a band across the ribbon fading toward the old end. The look is tuned here; the system's Scale (copied to the
// emitter every tick) multiplies it per spawn: x the width, y the lifetime.
class BulletTrailEmitter : public RibbonEmitter
{
public:

	float Width = 0.05f;                                  // m at the shot
	float EndWidth = 0.02f;                               // m at the old end
	float LifeTime = 0.12f;                               // s; the ribbon covers this much of the shot's path
	vec4 TrailColor = vec4(1.0f, 0.62f, 0.32f, 0.5f);
	
	BulletTrailEmitter() : RibbonEmitter()
	{
		texture = "GameData/textures/particles/trail.png";
		SpawnRate = 100;
		Emitting = true;
		InitialSpawnCount = 1;
		SimpleRibbon = true;
		ParticleLighting = false;
		PixelShader = "fs_unlit";
		BlendMode = BgfxStateManager::Blend::Additive;
	}

	Particle UpdateParticle(Particle particle, float deltaTime) override
	{
		const float remaining = (particle.deathTime - particle.lifeTime) / particle.deathTime;
		particle.Transparency = remaining;
		particle.Size = mix(EndWidth, Width, remaining) * Scale.x;
		return particle;
	}

	Particle GetNewParticle() override
	{
		Particle particle = ParticleEmitter::GetNewParticle();
		
		particle.Size = Width * Scale.x;
		particle.Color = TrailColor;
		particle.Transparency = 1.0f;
		particle.deathTime = LifeTime * Scale.y;

		return particle;
	}

private:

};

class BulletTrail : public ParticleSystem
{
public:

	BulletTrail()
	{
		emitters.push_back(new BulletTrailEmitter());
	}

private:

};

REGISTER_ENTITY(BulletTrail, "bullet_trail")
