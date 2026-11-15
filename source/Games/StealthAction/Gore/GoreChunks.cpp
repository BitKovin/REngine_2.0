#include "GoreChunks.h"

#include "GoreDrawCommands.h"
#include "GoreFlesh.h"
#include "GoreSettings.h"

#include <AssetRegistry.h>
#include <BSP/Quake3Bsp.h>
#include <Entity.h>
#include <EntityHandle.h>
#include <IDrawMesh.h>
#include <Level.hpp>
#include <Particle/GlobalParticleSystem.hpp>
#include <Physics.h>
#include <Time.hpp>
#include <World/WorldOrientationManager.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <algorithm>
#include <array>
#include <cstdlib>
#include <vector>

namespace Gore
{
    namespace
    {
        constexpr int kMaxChunks = 64;             // size of the pool; GoreSettings::maxChunks may use less
        constexpr double kFlightSeconds = 8.0;     // a chunk that never lands is removed after this
        constexpr double kRestSeconds = 12.0;      // how long a landed chunk stays
        constexpr float kShrinkSeconds = 0.5f;     // it shrinks to nothing over the last half second

        float Rand01() { return static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX); }   // looks only, never gameplay
        glm::vec3 RandVec() { return glm::vec3(Rand01() - 0.5f, Rand01() - 0.5f, Rand01() - 0.5f) * 2.0f; }

        struct Chunk
        {
            bool active = false;
            bool resting = false;
            bool decalDone = false;
            uint8_t variant = 0;
            uint8_t bounces = 0;
            glm::vec3 position = glm::vec3(0);
            glm::vec3 velocity = glm::vec3(0);
            glm::vec3 spinAxis = glm::vec3(0, 1, 0);
            float spinSpeed = 0.0f;                 // radians per second
            glm::quat rotation = glm::quat(1, 0, 0, 0);
            float size = 0.03f;                     // metres
            double bornAt = 0.0;
            double dieAt = 0.0;
        };

        struct Request
        {
            glm::vec3 point, direction;
            int count;
            float speed, size;
        };

        // The shared meshes. CPU side is built once (game thread); GPU side on first draw (main thread).
        struct ChunkMeshes
        {
            std::vector<VertexData> vertices;
            std::vector<uint32_t> indices;
            std::vector<FleshPiece> variants;
            bgfx::VertexBufferHandle vbh = BGFX_INVALID_HANDLE;
            bgfx::IndexBufferHandle ibh = BGFX_INVALID_HANDLE;
            bool gpuReady = false;

            void EnsureGpu()
            {
                if (gpuReady || vertices.empty()) return;
                gpuReady = true;
                vbh = bgfx::createVertexBuffer(bgfx::copy(vertices.data(), static_cast<uint32_t>(sizeof(VertexData) * vertices.size())),
                                               VertexData::Declaration());
                ibh = bgfx::createIndexBuffer(bgfx::copy(indices.data(), static_cast<uint32_t>(sizeof(uint32_t) * indices.size())),
                                              BGFX_BUFFER_INDEX32);
            }
        };

        ChunkMeshes g_meshes;
        const std::vector<mat4> g_identityPalette(1, mat4(1.0f));   // chunk vertices follow "bone 0"
        std::vector<Request> g_requests;
        EntityHandle g_system;
        int g_decalBudget = 0;                                       // blood marks left per frame

        // Draws the chunks: one command each. Reads the chunk array in PreFinalize only.
        class ChunkDrawable : public IDrawMesh
        {
        public:
            explicit ChunkDrawable(const std::array<Chunk, kMaxChunks>* source) : chunks(source)
            {
                for (auto& c : commands) c = std::make_shared<GoreDrawCommand>();
            }

            GpuData gpu;   // textures and gore level; filled by the system in LoadAssets

            void PreFinalize() override
            {
                pendingCount = 0;
                const double now = Time::GameTime;
                for (const Chunk& c : *chunks)
                {
                    if (!c.active) continue;
                    Item& item = pending[pendingCount++];

                    const float fade = glm::clamp(static_cast<float>(c.dieAt - now) / kShrinkSeconds, 0.0f, 1.0f);
                    item.position = c.position;
                    item.world = glm::translate(mat4(1.0f), c.position) * glm::mat4_cast(c.rotation)
                               * glm::scale(mat4(1.0f), glm::vec3(c.size * fade));
                    item.variant = c.variant;

                    // Light at the chunk, like StaticMesh::PreFinalize does for its entity.
                    item.ambient = glm::vec3(0.3f);
                    item.direct = glm::vec3(0.6f);
                    item.lightDirection = glm::vec3(0, -1, 0);
                    if (Level::Current != nullptr)
                    {
                        const LightVolPointData light = Level::Current->BspData.GetLightvolColorPoint(c.position * MAP_SCALE, false);
                        item.ambient = glm::max(light.ambientColor, glm::vec3(0.01f));
                        item.direct = glm::max(light.directColor, glm::vec3(0.03f));
                        item.lightDirection = light.direction;
                    }
                }
            }

            void FinalizeFrameData() override
            {
                g_meshes.EnsureGpu();
                finalCount = 0;
                if (!bgfx::isValid(g_meshes.vbh) || !bgfx::isValid(g_meshes.ibh) || g_meshes.variants.empty()) return;

                // Textures load in the background: until all are ready the shader would sample black.
                const GpuData& tex = gpu;
                for (const Texture* t : { tex.flesh, tex.bone, tex.guts, tex.blood })
                    if (t == nullptr || !t->valid) return;

                for (int i = 0; i < pendingCount; ++i)
                {
                    const Item& item = pending[i];
                    const FleshPiece& piece = g_meshes.variants[std::min<size_t>(item.variant, g_meshes.variants.size() - 1)];

                    GoreDrawCommand& cmd = *commands[finalCount++];
                    cmd.pass = GoreDrawCommand::Pass::Anatomy;   // same shaders as bones and guts
                    cmd.goreActive = true;
                    cmd.gore = &gpu;
                    cmd.layer = static_cast<float>(kAnatomyLayer);
                    cmd.insetFraction = 0.0f;

                    cmd.VertexBuffer = g_meshes.vbh;
                    cmd.IndexBuffer = g_meshes.ibh;
                    DrawIndexRange range;
                    range.first = piece.first;
                    range.count = piece.count;
                    range.param = glm::vec4(0.0f);               // mask 0: no wounds on a chunk
                    cmd.IndexRanges.assign(1, range);
                    cmd.RangeParamUniform = "u_goreRangeMask";   // must be set, or the mask of the previous draw stays

                    cmd.BoneMatrices = &g_identityPalette;
                    cmd.WorldMatrix = item.world;
                    cmd.WorldBounds = BoundingBox(item.position - glm::vec3(0.3f), item.position + glm::vec3(0.3f));
                    cmd.LightAmbientColor = item.ambient;
                    cmd.LightDirectColor = item.direct;
                    cmd.LightDirection = item.lightDirection;

                    cmd.Cull = StaticMeshDrawCommand::CullFace::None;
                    cmd.TwoSided = true;
                    cmd.DepthPrePass = true;
                    cmd.Material.SurfaceTypeOverride = SurfaceType::Opaque;
                    cmd.Material.CustomId = 0;
                    cmd.IsViewmodel = false;
                    cmd.OnlyShadows = false;
                    cmd.ReceiveDetailShadows = false;
                }
            }

            void CollectDrawCommands(std::vector<IDrawCommand*>& out) override
            {
                for (int i = 0; i < finalCount; ++i) out.push_back(commands[i].get());
            }

            bool IsCameraVisible() override { return pendingCount > 0; }   // the chunks are spread out: no single frustum box

        private:
            struct Item
            {
                mat4 world = mat4(1.0f);
                glm::vec3 position = glm::vec3(0);
                glm::vec3 ambient = glm::vec3(0), direct = glm::vec3(0), lightDirection = glm::vec3(0, -1, 0);
                uint8_t variant = 0;
            };

            const std::array<Chunk, kMaxChunks>* chunks;
            std::array<Item, kMaxChunks> pending;
            int pendingCount = 0, finalCount = 0;
            std::array<std::shared_ptr<GoreDrawCommand>, kMaxChunks> commands;
        };

        class GoreChunkSystem : public Entity
        {
        public:
            GoreChunkSystem()
            {
                ClassName = "gore_chunks";
                SaveGame = false;
                drawable = new ChunkDrawable(&chunks);
                Drawables.push_back(drawable);   // the entity owns and deletes it (Entity::DestroyDrawables)
            }

            void Spawn(const Request& r)
            {
                const double now = Time::GameTime;
                const glm::vec3 up = WorldOrientationManager::GetUpVector();
                const glm::vec3 dir = glm::length(r.direction) > 1.0e-4f ? glm::normalize(r.direction) : up;
                const int limit = glm::clamp(GoreSettings::Get().maxChunks, 1, kMaxChunks);

                for (int k = 0; k < r.count; ++k)
                {
                    // A free slot, else the oldest chunk.
                    Chunk* slot = nullptr;
                    for (int i = 0; i < limit; ++i)
                    {
                        if (!chunks[i].active) { slot = &chunks[i]; break; }
                        if (slot == nullptr || chunks[i].bornAt < slot->bornAt) slot = &chunks[i];
                    }

                    Chunk c;
                    c.active = true;
                    c.variant = Rand01() < 0.2f ? 3 : static_cast<uint8_t>(std::min(static_cast<int>(Rand01() * 3.0f), 2));
                    c.position = r.point + RandVec() * 0.04f;
                    c.velocity = (dir + RandVec() * 0.55f) * (r.speed * glm::mix(0.5f, 1.2f, Rand01())) + up * glm::mix(0.5f, 2.5f, Rand01());
                    const glm::vec3 axis = RandVec() + glm::vec3(0.01f, 0.02f, 0.03f);
                    c.spinAxis = glm::normalize(axis);
                    c.spinSpeed = glm::mix(4.0f, 18.0f, Rand01());
                    c.rotation = glm::angleAxis(Rand01() * 6.28f, c.spinAxis);
                    c.size = r.size * glm::mix(0.6f, 1.5f, Rand01() * Rand01());
                    c.bornAt = now;
                    c.dieAt = now + kFlightSeconds;
                    *slot = c;
                }
            }

            void Update() override
            {
                const float dt = Time::DeltaTimeF;
                if (dt <= 0.0f) return;
                const double now = Time::GameTime;
                const glm::vec3 up = WorldOrientationManager::GetUpVector();

                for (Chunk& c : chunks)
                {
                    if (!c.active) continue;
                    if (now >= c.dieAt) { c.active = false; continue; }
                    if (c.resting) continue;

                    c.velocity -= up * (9.8f * dt);
                    const glm::vec3 next = c.position + c.velocity * dt;
                    const Physics::HitResult hit = Physics::LineTrace(c.position, next, BodyType::World);

                    if (!hit.hasHit)
                    {
                        c.position = next;
                        c.rotation = glm::normalize(glm::angleAxis(c.spinSpeed * dt, c.spinAxis) * c.rotation);
                        continue;
                    }

                    if (!c.decalDone && g_decalBudget > 0)   // a blood mark where it first touches the world
                    {
                        --g_decalBudget;
                        c.decalDone = true;
                        GlobalParticleSystem::SpawnParticleAt("decal_blood", hit.position, hit.normal, glm::vec3(1));
                    }

                    const float into = -glm::dot(c.velocity, hit.normal);
                    if (into > 1.5f && c.bounces < 2)        // a wet bounce: most of the speed is lost
                    {
                        const glm::vec3 reflected = glm::reflect(c.velocity, hit.normal);
                        const glm::vec3 normalPart = hit.normal * glm::dot(reflected, hit.normal);
                        c.velocity = normalPart * 0.3f + (reflected - normalPart) * 0.6f;
                        c.position = hit.position + hit.normal * (c.size * 0.6f);
                        c.spinSpeed *= 0.5f;
                        ++c.bounces;
                    }
                    else                                     // it stays where it hit (meat sticks to walls too)
                    {
                        c.position = hit.position + hit.normal * (c.size * 0.45f);
                        c.velocity = glm::vec3(0);
                        c.resting = true;
                        c.dieAt = now + kRestSeconds;
                    }
                }
            }

        protected:
            void LoadAssets() override
            {
                if (g_meshes.vertices.empty())
                    BuildChunkMeshes(g_meshes.vertices, g_meshes.indices, g_meshes.variants);

                // The same textures the humanoid profile uses (GameData/gore/humanoid.gore).
                GpuData& g = drawable->gpu;
                g.flesh = AssetRegistry::GetTextureFromFile("GameData/textures/gore/flesh_d.png");
                g.edgeNoise = AssetRegistry::GetTextureFromFile("GameData/textures/gore/edge_noise.png");
                g.blood = AssetRegistry::GetTextureFromFile("GameData/textures/gore/blood_soak.png");
                g.decal = AssetRegistry::GetTextureFromFile("GameData/textures/gore/blood_decals.png");
                g.bone = AssetRegistry::GetTextureFromFile("GameData/textures/gore/bone_d.png");
                g.guts = AssetRegistry::GetTextureFromFile("GameData/textures/lq_flesh/flesh_gut1.jpg");
                g.counts = glm::vec4(0.0f);                  // no wounds, cuts or splats on a chunk
                g.params = glm::vec4(0.0f, 0.0f, 5.0f, static_cast<float>(GoreSettings::Get().level));
            }

        private:
            std::array<Chunk, kMaxChunks> chunks;
            ChunkDrawable* drawable = nullptr;
        };
    }

    void SpawnChunks(const glm::vec3& point, const glm::vec3& direction, int count, float speed, float size)
    {
        if (count <= 0 || g_requests.size() >= 64) return;
        g_requests.push_back({ point, direction, count, speed, size });
    }

    namespace ChunkPool
    {
        void Tick()
        {
            g_decalBudget = 4;
            if (g_requests.empty()) return;

            // Chunks are a "full gore" effect only.
            if (Level::Current == nullptr || GoreSettings::Get().level != GoreSettings::Level::Full)
            {
                g_requests.clear();
                return;
            }

            GoreChunkSystem* system = dynamic_cast<GoreChunkSystem*>(g_system.Resolve());
            if (system == nullptr)   // first use in this level
            {
                system = new GoreChunkSystem();
                Level::Current->AddEntity(system);
                system->LoadAssetsIfNeeded();
                g_system = EntityHandle::FromEntity(system);
            }

            int budget = GoreSettings::Get().maxChunkSpawnsPerFrame;
            for (Request& r : g_requests)
            {
                r.count = std::min(r.count, budget);
                budget -= r.count;
                if (r.count > 0) system->Spawn(r);
            }
            g_requests.clear();
        }
    }
}
