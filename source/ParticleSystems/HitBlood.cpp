#include "../Particle/GlobalParticleSystem.hpp"



class BloodDecal : public ParticleEmitter
{
public:
	BloodDecal()
	{
		Duration = 10000000;
		InitialSpawnCount = 0;
		SpawnRate = 0;	
        texture = "GameData/Textures/particles/smoke.png";
	}
	
    // Override UpdateParticle.
     Particle UpdateParticle(Particle particle, float deltaTime) override {
        // Call the base update first.
        particle = ParticleEmitter::UpdateParticle(particle, deltaTime);

        // Increase the scale based on lifetime.
        float incScale = particle.lifeTime / 9.0f;
        // Use glm::mix to linearly interpolate between 1 and 0.1.
        float lerpVal = glm::mix(1.0f, 0.1f, incScale * incScale);
        float maxVal = (lerpVal > 0.0f) ? lerpVal : 0.0f;
        particle.Size += deltaTime * maxVal * 0.3f;

        const float despawnTime = 4.0f;
        if ((particle.deathTime - particle.lifeTime - 0.1f) < despawnTime)
            particle.Transparency -= deltaTime / despawnTime;

        return particle;
    }

    // Override GetNewParticle.
     Particle GetNewParticle() override {
        Particle particle = ParticleEmitter::GetNewParticle();

        particle.UseWorldRotation = true;

        // Compute the normal from the emitter's Rotation.
        glm::vec3 normal = Rotation;
        // Set globalRotation based on a "look-at" rotation.
        particle.globalRotation = MathHelper::FindLookAtRotation(glm::vec3(0.0f), normal);
        // Add a random angle (in degrees) on Z.
        float randomAngle = static_cast<float>(rand()) / RAND_MAX * 360.0f;
        particle.globalRotation.z += randomAngle;

        // Slightly adjust particle position.
        particle.position += normal * 0.005f;

        particle.Size = 1.4f;
        //particle.MaxDrawDistance = 60.0f;
        particle.BouncePower = 0.1f;

        // Generate a random deathTime in the range [0, 90] approximately.
        float randomVal = static_cast<float>(rand()) / RAND_MAX;
        particle.deathTime = 30.0f * randomVal * 3.0f;

        // Set the color to a semi-transparent red.
        particle.Color = glm::vec4(0.7f, 0.0f, 0.0f, 0.7f);

        return particle;
    }

private:

};

class BloodDecalSystem : public GlobalParticleSystem
{
public:
    
    BloodDecal* decal = nullptr;

    BloodDecalSystem()
    {

        decal = new BloodDecal();

        emitters.push_back(decal);
    }

    void SpawnParticleAtInst(vec3 position, vec3 rotation, vec3 scale)
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


private:

};


REGISTER_LEVEL_OBJECT(BloodDecalSystem, "decal_blood")