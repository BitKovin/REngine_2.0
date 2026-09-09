#include <Particle/GlobalParticleSystem.hpp>
#include <World/WorldOrientationManager.h>

class particle_fireFlame : public ParticleEmitter
{
public:
    particle_fireFlame()
    {
        texture = "GameData/textures/particles/fireSheet5x5.png";

        Transparent = true;
        DepthSorting = false;

        // Continuous emission - engine spawns particles on its own via SpawnRate.
        InitialSpawnCount = 0;
        SpawnRate = 13.0f;
        Emitting = true;
        Duration = 1000000; // effectively infinite / looping emitter
        BlendMode = BgfxStateManager::Blend::Additive;
        PixelShader = "fs_unlit";
    }

    Particle UpdateParticle(Particle particle, float deltaTime) override
    {
        particle = ParticleEmitter::UpdateParticle(particle, deltaTime);

        // Rising motion with a little turbulence so the flame doesn't look rigid.
        particle.velocity += WorldOrientationManager::GetUpVector() * 0.2f * deltaTime;
        particle.velocity.x += (RandomFloat() - 0.5f) * 0.5f * deltaTime;
        particle.velocity.z += (RandomFloat() - 0.5f) * 0.5f * deltaTime;

        float lifeRatio = particle.lifeTime / particle.deathTime;
        lifeRatio = std::min(lifeRatio, 1.0f);

        // Shrink from spawn size down toward a small tip as it burns out.
        particle.Size = glm::mix(particle.UserValue1, particle.UserValue1 * 0.6f, lifeRatio);

        // Fade out over the back half of life.
        const float fadeStart = 0.55f;
        if (lifeRatio > fadeStart)
        {
            float fadeT = (lifeRatio - fadeStart) / (1.0f - fadeStart);
            particle.Transparency = glm::mix(1.0f, 0.0f, fadeT);
        }
        else
        {
            particle.Transparency = 1.0f;
        }
        particle.Transparency *= 0.75;
        // Colour cools from bright yellow-white at the base to deep red/orange at the tip.
        glm::vec3 hotColor = glm::vec3(1.0f, 0.9f, 0.5f);
        glm::vec3 coolColor = glm::vec3(0.9f, 0.25f, 0.05f);
        glm::vec3 color = glm::mix(hotColor, coolColor, lifeRatio);
        particle.Color = glm::vec4(color, particle.Transparency);

        // Step through the 5x5 sheet across the particle's lifetime (looping 4x).
        const int frameCount = SheetCols * SheetRows;
        int frame = std::min((int)(lifeRatio * 2 * frameCount), frameCount - 1);

        particle.UvPositionAndScale = GetFrameUV(frame);

        return particle;
    }

    Particle GetNewParticle() override
    {
        Particle particle = ParticleEmitter::GetNewParticle();

        // Small random spawn offset so flames don't all stack on one point.
        particle.position += RandomOffset(0.2f) * vec3(1,0.3,1);

        // Gentle upward drift with a bit of sideways randomness.
        glm::vec3 velocity = WorldOrientationManager::GetUpVector() * glm::mix(0.3f, 1.4f, RandomFloat());
        velocity.x += (RandomFloat() - 0.5f) * 0.2f;
        velocity.z += (RandomFloat() - 0.5f) * 0.2f;
        particle.velocity = velocity;

        particle.deathTime = glm::mix(0.8f, 1.4f, RandomFloat());

        particle.Size = glm::mix(2.2f, 2.7f, RandomFloat());
        particle.UserValue1 = particle.Size; // remember spawn size for the shrink-over-life curve

        particle.Transparency = 1.0f;

        // Start on frame 0 of the sheet.
        particle.UvPositionAndScale = GetFrameUV(0);

        particle.Color = glm::vec4(1.0f, 0.9f, 0.5f, 1.0f);

        return particle;
    }

private:
    static constexpr int SheetCols = 5;
    static constexpr int SheetRows = 5;

    // How much to horizontally stretch each flame frame. 1.0 = sample the
    // frame's full width as-is (no stretch). Values above 1.0 sample a
    // narrower vertical slice out of the middle of the frame and blow it up
    // across the full quad width, making the flame look thinner/elongated
    // horizontally instead of a plain 1:1 crop of the sheet. Because the
    // sampled slice is always centered inside the cell, this never bleeds
    // into neighboring frames, no matter how high the value goes.
    const float fireHorizontalStretch = 1.3f;

    glm::vec4 GetFrameUV(int frame) const
    {
        int col = frame % SheetCols;
        int row = frame / SheetCols;

        float cellW = 1.0f / SheetCols;
        float cellH = 1.0f / SheetRows;

        // Sample a narrower horizontal slice, centered in the cell, and
        // stretch it across the same quad width via UvPositionAndScale.z.
        float uScale = cellW / fireHorizontalStretch;
        float uOffset = col * cellW + (cellW - uScale) * 0.5f;

        return glm::vec4(uOffset, row * cellH + 0.01f, uScale, cellH);
    }

    float RandomFloat() const { return static_cast<float>(rand()) / (float)RAND_MAX; }

    glm::vec3 RandomOffset(float radius) const
    {
        return glm::vec3(
            (RandomFloat() - 0.5f) * 2.0f * radius,
            0.0f,
            (RandomFloat() - 0.5f) * 2.0f * radius
        );
    }
};

class FireSystem : public GlobalParticleSystem
{
public:
    particle_fireFlame* flame;

    FireSystem()
    {
        flame = new particle_fireFlame();
        emitters.push_back(flame);
    }
};

REGISTER_ENTITY(FireSystem, "fire_burning")