#include "ParticleEmitter.h"

#include "../MathHelper.hpp"
#include "../ShaderManager.h"
#include "../FrustrumCull.hpp"
#include "../BoundingSphere.hpp"
#include "../Renderer/Renderer.h"
#include "../BSP/Quake3Bsp.h"

#include <BgfxStateManager.h>
#include <Renderer/Abstractions/ViewIdManager.h>
#include <Profiling/ResourceStatistics.hpp>

// Static member definitions
bgfx::VertexBufferHandle ParticleEmitter::s_billboardVbh   = BGFX_INVALID_HANDLE;
bgfx::IndexBufferHandle  ParticleEmitter::s_billboardIbh   = BGFX_INVALID_HANDLE;
bgfx::VertexLayout       ParticleEmitter::s_billboardLayout = {};

// Helpers
mat4 GetWorldMatrix(const Particle& particle)
{
    return translate(particle.position) * MathHelper::GetRotationMatrix(particle.globalRotation) * scale(vec3(particle.Size));
}

// Spawn / Update
void ParticleEmitter::SpawnParticles(int num)
{
    std::lock_guard<std::recursive_mutex> lock(particlesMutex);
    for (int i = 0; i < num; i++) {
        Particle particle = GetNewParticle();
        particle.deathTime *= i + 1;
        AddParticle(particle);
    }
}

void ParticleEmitter::Update(float deltaTime)
{
    if (destroyed)
        return;

    std::lock_guard<std::recursive_mutex> lock(particlesMutex);
    emitterTime += deltaTime;
    elapsedTime += deltaTime;
    if (elapsedTime > Duration)
        Emitting = false;

    const float spawnInterval = (SpawnRate > 0.0f) ? (1.0f / SpawnRate) : 0.0f;

    if (SpawnRate > 0.0f && Emitting && spawnInterval > 0.0f)
    {
        while (elapsedTime >= spawnInterval)
        {
            Particles.push_back(GetNewParticle());
            elapsedTime -= spawnInterval;
        }
    }

    for (auto& p : Particles)
        p.lifeTime += deltaTime;

    Particles.erase(
        std::remove_if(Particles.begin(), Particles.end(),
            [](const Particle& p) { return p.lifeTime >= p.deathTime; }),
        Particles.end());

    if (MaxParticles > 0 && Particles.size() > MaxParticles)
    {
        const size_t excess = Particles.size() - MaxParticles;
        Particles.erase(
            Particles.begin(),
            Particles.begin() + static_cast<std::ptrdiff_t>(excess));
    }

    for (auto& p : Particles)
        p = UpdateParticle(p, deltaTime);

    if (!Emitting && Particles.empty())
        destroyed = true;
}

// InitBilboardVaoIfNeeded
void ParticleEmitter::InitBilboardVaoIfNeeded()
{
    if (bgfx::isValid(s_billboardVbh)) return;

    // Exact match to shader $input — this is what the official bgfx example does
    s_billboardLayout.begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
        .end();

    struct BillboardVertex {
        float x, y, z;
        float u, v;
    };

    BillboardVertex vertices[4] = {
        {-0.5f, -0.5f, 0.0f, 0.0f, 1.0f},
        { 0.5f, -0.5f, 0.0f, 1.0f, 1.0f},
        { 0.5f,  0.5f, 0.0f, 1.0f, 0.0f},
        {-0.5f,  0.5f, 0.0f, 0.0f, 0.0f}
    };

    uint16_t indices[6] = { 0, 1, 2, 2, 3, 0 };

    s_billboardVbh = bgfx::createVertexBuffer(bgfx::copy(vertices, sizeof(vertices)), s_billboardLayout);
    s_billboardIbh = bgfx::createIndexBuffer(bgfx::copy(indices, sizeof(indices)));

    if (bgfx::isValid(s_billboardVbh))
    {
        ResourceStatistics::Instance().registerResource(
            ResourceType::VertexBuffer, s_billboardVbh.idx, sizeof(vertices), "Particle Billboard VB");
    }
    if (bgfx::isValid(s_billboardIbh))
    {
        ResourceStatistics::Instance().registerResource(
            ResourceType::IndexBuffer, s_billboardIbh.idx, sizeof(indices), "Particle Billboard IB");
    }
}

void ParticleEmitter::DestroyBillboardVao()
{

	if (bgfx::isValid(s_billboardVbh))
    {
        ResourceStatistics::Instance().unregisterResource(ResourceType::VertexBuffer, s_billboardVbh.idx);
        bgfx::destroy(s_billboardVbh);
        s_billboardVbh = BGFX_INVALID_HANDLE;
    }

	if (bgfx::isValid(s_billboardIbh))
    {
        ResourceStatistics::Instance().unregisterResource(ResourceType::IndexBuffer, s_billboardIbh.idx);
        bgfx::destroy(s_billboardIbh);
        s_billboardIbh = BGFX_INVALID_HANDLE;
    }

}

// SnapshotParticles - commits Particles into finalizedParticles and captures the camera values the instance build reads.
void ParticleEmitter::SnapshotParticles()
{
    {
        std::lock_guard<std::recursive_mutex> lock(particlesMutex);
        finalizedParticles = Particles;
    }

    const vec3 cameraRotation = Camera::finalizedRotation;
    earlyCameraPosition = Camera::finalizedPosition;
    earlyCameraForward = MathHelper::GetForwardVector(cameraRotation);
    earlyCameraRight = MathHelper::GetRightVector(cameraRotation);
    earlyCameraUp = MathHelper::GetUpVector(cameraRotation);

    earlyCameraCluster = Level::Current->BspData.FindClusterAtPosition(earlyCameraPosition);

    earlyInstances.resize(finalizedParticles.size());
}

// BuildInstances - pure CPU math + BSP lightvol reads for earlyInstances[begin, end). Jobs get disjoint ranges.
void ParticleEmitter::BuildInstances(size_t begin, size_t end)
{
    // Depth sorting always culls and only lights when ParticleLighting is on; the unsorted path always lights and culls when ParticleCulling is on.
    const bool cull = DepthSorting || ParticleCulling;
    const bool light = DepthSorting ? ParticleLighting : true;

    for (size_t i = begin; i < end; i++)
    {
        const Particle& particle = finalizedParticles[i];
        EarlyInstance& out = earlyInstances[i];
        out.visible = false;

        if (cull)
        {
            if (!Camera::frustum.IsSphereVisible(particle.position, particle.Size))
                continue;

            int targetC = Level::Current->BspData.FindClusterAtPosition(particle.position);
            if (!Level::Current->BspData.IsClusterVisible(earlyCameraCluster, targetC))
                continue;
        }

        mat4x4 world;
        if (particle.UseWorldRotation)
            world = GetWorldMatrix(particle);
        else
            world = MathHelper::CreateBillboardMatrix(
                particle.position, earlyCameraPosition,
                earlyCameraForward, earlyCameraRight, earlyCameraUp,
                vec3(particle.Size), particle.rotation);

        InstanceData& data = out.data;
        data.model[0] = world[0];  // column 0
        data.model[1] = world[1];
        data.model[2] = world[2];
        data.model[3] = world[3];
        data.Color = particle.Color * (light ? vec4(GetLightForParticle(particle), 1.0f) : vec4(1.0f));
        data.Color.a *= particle.Transparency;

        data.UvPositionAndScale = particle.UvPositionAndScale;

        out.depth = DepthSorting ? glm::dot(earlyCameraForward, particle.position - earlyCameraPosition) : 0.0f;
        out.visible = true;
    }
}

// PreFinalizeEarlyJobs - serial. Snapshots the particles, then splits the instance build into at most MaxEarlyJobs slices.
void ParticleEmitter::PreFinalizeEarlyJobs(std::vector<std::function<void()>>& outJobs)
{
    SnapshotParticles();
    earlySnapshotTaken = true;
    earlyJobsHandedOut = false;

    const size_t count = finalizedParticles.size();
    const size_t jobCount = std::min(MaxEarlyJobs, count / MinParticlesPerEarlyJob);
    if (jobCount < 2)
        return;   // too small to be worth a job each - PreFinalize builds it inline

    earlyJobsHandedOut = true;

    const size_t perJob = (count + jobCount - 1) / jobCount;
    for (size_t begin = 0; begin < count; begin += perJob)
    {
        const size_t end = std::min(count, begin + perJob);
        outJobs.push_back([this, begin, end]() { BuildInstances(begin, end); });
    }
}

// PreFinalize - parallel across meshes. The per-particle math is already done if PreFinalizeEarlyJobs handed out jobs;
// otherwise (small emitter, or called directly without the early stage) it is done here.
void ParticleEmitter::PreFinalize()
{
    if (!earlySnapshotTaken)
        SnapshotParticles();

    if (!earlyJobsHandedOut)
        BuildInstances(0, finalizedParticles.size());

    earlySnapshotTaken = false;
    earlyJobsHandedOut = false;

    preFinalizedInstances.clear();

    if (DepthSorting)
    {
        sortScratch.clear();
        sortScratch.reserve(earlyInstances.size());
        for (size_t i = 0; i < earlyInstances.size(); i++)
        {
            if (earlyInstances[i].visible)
                sortScratch.emplace_back(earlyInstances[i].depth, static_cast<uint32_t>(i));
        }

        if (!sortScratch.empty())
        {
            std::sort(sortScratch.begin(), sortScratch.end(),
                [](const auto& a, const auto& b) { return a.first > b.first; });

            preFinalizedInstances.reserve(sortScratch.size());
            for (const auto& s : sortScratch)
                preFinalizedInstances.push_back(earlyInstances[s.second].data);
        }
    }
    else
    {
        preFinalizedInstances.reserve(earlyInstances.size());
        for (const auto& e : earlyInstances)
        {
            if (e.visible)
                preFinalizedInstances.push_back(e.data);
        }
    }
}

// FinalizeFrameData - main thread: resolve the texture and commit onto drawCommand.
void ParticleEmitter::FinalizeFrameData()
{
    if (savedTextureName != texture)
    {
        savedTexture = AssetRegistry::GetTextureFromFile(texture);
        savedTextureName = texture;
    }

    drawCommand.Instances = std::move(preFinalizedInstances);
    drawCommand.ResolvedTexture = savedTexture;
    drawCommand.IsDecal = isDecal;
    drawCommand.PixelShader = PixelShader;
    drawCommand.BlendMode = BlendMode;

    SyncCommandFlags(drawCommand);
}

// GetLightForParticle
vec3 ParticleEmitter::GetLightForParticle(const Particle& particle)
{
    if (particle.UseWorldRotation == false)
    {
        auto light = Level::Current->BspData.GetLightvolColorPoint(MathHelper::TransformVector(particle.position, RelativeTransform) * MAP_SCALE);
        return (light.ambientColor + light.directColor) * 1.0f;
    }

    vec3 normal = MathHelper::GetForwardVector(particle.globalRotation);

    auto light = Level::Current->BspData.GetLightvolColorPoint((MathHelper::TransformVector(particle.position, RelativeTransform) + normal) * MAP_SCALE) * 1.0f;

    float dirFactor = 1.0f;

    return light.ambientColor + light.directColor * dirFactor;
}
