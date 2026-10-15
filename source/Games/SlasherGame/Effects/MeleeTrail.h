#pragma once

#include <Particle/GlobalParticleSystem.hpp>
#include <Particle/TrailEmitter.h>

// ---------------------------------------------------------------------------
// particle_meleeTrail
//
// The weapon's swing ribbon (COMBAT_DESIGN.md §9.4). TrailEmitter spans every sample by its Size along the forward
// of its rotation, so each sample is the blade itself: centred on it, turned along it (base -> tip), as long as it.
// The newest sample follows the blade every frame, so the ribbon starts exactly at the weapon; older ones fade over
// their life. Colour and shape come from the texture: u runs from the oldest sample (0) to the blade (1), v from the
// tip edge (0) to the base (1).
// ---------------------------------------------------------------------------
class particle_meleeTrail : public TrailEmitter
{
public:
    float Width = 0.0f;   // blade span (m), set by the system

    particle_meleeTrail()
    {
        texture             = "GameData/textures/particles/melee_trail.png";
        InitialSpawnCount   = 2;
        SpawnRate           = 120;
        Emitting            = true;
        IsViewmodel         = false;
        ParticleLighting    = false;
        PixelShader         = "fs_unlit";
        BlendMode           = BgfxStateManager::Blend::Alpha;
    }

    void Update(float deltaTime) override
    {
        TrailEmitter::Update(deltaTime);

        std::lock_guard<std::recursive_mutex> lock(particlesMutex);
        if (Emitting && !Particles.empty())
        {
            Particle& head = Particles.back();
            head.position = Position;
            head.globalRotation = Rotation;
            head.Size = Width;
        }
    }

    Particle UpdateParticle(Particle particle, float deltaTime) override
    {
        particle = TrailEmitter::UpdateParticle(particle, deltaTime);
        const float age = glm::clamp(particle.lifeTime / particle.deathTime, 0.0f, 1.0f);
        particle.Transparency = 1.0f - glm::smoothstep(0.0f, 1.0f, age);
        return particle;
    }

    Particle GetNewParticle() override
    {
        Particle particle = TrailEmitter::GetNewParticle();
        particle.Size         = Width;
        particle.Transparency = 1.0f;
        particle.deathTime    = 0.18f;
        particle.Color        = glm::vec4(1.0f, 1.0f, 1.0f, 0.6f);   // overall opacity: present, not glaring
        return particle;
    }
};

// ---------------------------------------------------------------------------
// particle_system_meleeTrail
//
// GlobalParticleSystem wrapper. SetBlade places it on the blade span every frame (WeaponSet, main thread); the
// emitter picks it up in the async tick.
// ---------------------------------------------------------------------------
class particle_system_meleeTrail : public ParticleSystem
{
public:
    particle_system_meleeTrail()
    {
        particle_MeleeTrail = new particle_meleeTrail();
        emitters.push_back(particle_MeleeTrail);
    }

    void SetBlade(const vec3& base, const vec3& tip)
    {
        SetTrailTransform(base, tip);
        Width = glm::distance(base, tip);
    }

    void Start() override
    {
        particle_MeleeTrail->Width = Width;
        ParticleSystem::Start();
    }

    void AsyncUpdate() override
    {
        particle_MeleeTrail->Width = Width;
        ParticleSystem::AsyncUpdate();
    }

private:
    particle_meleeTrail* particle_MeleeTrail = nullptr;
    float Width = 0.0f;
};
