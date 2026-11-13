#include <Particle/GlobalParticleSystem.hpp>
#include <MathHelper.hpp>
#include <Physics.h>
#include <World/WorldOrientationManager.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>

// Blood of a severed stump: many small, fast droplets that arc under gravity and splat on the world.
// "gore_spurt" is spawned by GoreFx with scale = (droplet batches, speed multiplier, 0) and a rotation that looks along the jet.
namespace
{
    float Rand01() { return static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX); }

    class GoreDroplets : public ParticleEmitter
    {
    public:
        float speed = 1.0f;       // set by the system before each batch
        float spread = 0.12f;

        GoreDroplets()
        {
            texture = "GameData/textures/particles/blood.png";
            InitialSpawnCount = 0;
            SpawnRate = 0;
            Emitting = true;
            DepthSorting = false;
            Duration = 1000000;
        }

        Particle UpdateParticle(Particle particle, float deltaTime) override
        {
            const vec3 up = WorldOrientationManager::GetUpVector();
            particle.velocity -= up * 9.8f * (deltaTime * 0.5f);
            particle = ParticleEmitter::UpdateParticle(particle, deltaTime);
            particle.velocity -= up * 9.8f * (deltaTime * 0.5f);

            if (glm::length(particle.position - particle.position2) > 0.001f)
            {
                const Physics::HitResult hit = Physics::LineTrace(particle.position2, particle.position, BodyType::World);
                if (hit.hasHit)
                {
                    if (Rand01() < 0.10f)
                        GlobalParticleSystem::SpawnParticleAt("decal_blood", hit.position, hit.normal, vec3(1));
                    particle.deathTime = 0.0f;
                    return particle;
                }
                particle.position2 = particle.position;
            }

            particle.Transparency = std::min(1.0f, std::max(0.0f, (particle.deathTime - particle.lifeTime) * 2.5f));
            return particle;
        }

        Particle GetNewParticle() override
        {
            Particle particle = ParticleEmitter::GetNewParticle();

            const glm::vec3 forward = MathHelper::GetForwardVector(Rotation);   // the inverse of FindLookAtRotation
            glm::vec3 side(Rand01() - 0.5f, Rand01() - 0.5f, Rand01() - 0.5f);
            side *= 2.0f * spread;

            const float s = speed * glm::mix(2.2f, 5.5f, Rand01());
            particle.velocity = forward * s + side * s;
            particle.position2 = Position;
            particle.Size = glm::mix(0.025f, 0.085f, Rand01() * Rand01());
            particle.deathTime = glm::mix(0.9f, 2.0f, Rand01());
            particle.rotation = Rand01() * 360.0f;
            particle.Transparency = 1.0f;
            particle.Color = glm::vec4(0.42f, 0.015f, 0.015f, 1.0f);
            return particle;
        }
    };

    class GoreSpurtSystem : public GlobalParticleSystem
    {
    public:
        GoreDroplets* droplets;

        GoreSpurtSystem()
        {
            droplets = new GoreDroplets();
            emitters.push_back(droplets);
        }

    private:
        void SpawnParticleAtInst(vec3 position, vec3 rotation, vec3 scale) override
        {
            Position = position;
            Rotation = rotation;
            Scale = scale;

            droplets->Position = position;
            droplets->Rotation = rotation;
            droplets->speed = std::max(scale.y, 0.01f);
            droplets->spread = scale.y < 0.5f ? 0.5f : 0.12f;      // drips fall almost straight, jets are tight
            droplets->SpawnParticles(static_cast<int>(std::ceil(scale.x)) * 6);
        }
    };
}

REGISTER_ENTITY(GoreSpurtSystem, "gore_spurt")
