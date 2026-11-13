#include "GoreComponent.h"
#include "GoreCutSolver.h"
#include "GoreProfile.h"
#include "GoreSettings.h"
#include "GoreSkeletalMesh.h"
#include "GoreSystem.h"

#include <Ecs/Ecs.h>
#include <Ecs/ISystem.h>
#include <Entity.h>
#include <Logger.hpp>
#include <MathHelper.hpp>
#include <Particle/GlobalParticleSystem.hpp>
#include <Time.hpp>

#include <algorithm>
#include <cmath>

namespace Gore
{
    namespace
    {
        int g_budget = 0;            // particle spawns left this frame
        constexpr float kPi2 = 6.28318530718f;
        constexpr double kDripSeconds = 10.0;
    }

    void FxNewFrame()
    {
        g_budget = GoreSettings::Get().maxParticlesPerFrame;
    }

    // Stump effects: a pulsing arterial spurt for a few seconds, then drips ("gore_spurt", GoreParticles.cpp).
    void UpdateStumpFx(Entity* self, GoreComponent& gore)
    {
        if (gore.cutCount == 0) return;

        GoreSkeletalMesh* mesh = FindGoreMesh(self);
        if (mesh == nullptr || mesh->GetSkinnedModel() == nullptr) return;

        const Profile& profile = GetProfile(mesh->filePath, *mesh->GetSkinnedModel());
        if (!profile.valid) return;

        const std::vector<glm::mat4>& palette = mesh->GetSkinPalette();
        if (palette.empty()) return;

        const double now = Time::GameTime;
        const glm::mat4 World = mesh->GetWorldMatrix();

        for (int i = 0; i < gore.cutCount; ++i)
        {
            Cut& cut = gore.cuts[i];
            if (cut.phase != CutPhase::Severed || cut.charred) continue;
            if (now < cut.fxNext) continue;

            const double age = now - cut.severedAt;
            if (age < 0.0 || age > kDripSeconds + 4.0) continue;

            float spurtSeconds = 4.0f, pulseHz = 1.6f;
            if (cut.boneSkel < profile.severableBySkel.size() && profile.severableBySkel[cut.boneSkel] >= 0)
            {
                const SeverableBone& sev = profile.severable[profile.severableBySkel[cut.boneSkel]];
                spurtSeconds = sev.spurtSeconds;
                pulseHz = sev.pulseHz;
            }

            int count = 0;
            bool drip = false;
            double next = 0.05;
            glm::vec3 dirBind = glm::vec3(cut.plane);
            if (gore.isGib) dirBind = -dirBind;          // the gib's cap faces the body

            if (age < 0.18)
            {
                count = 4;                                // the first gush
                next = 0.06;
            }
            else if (age < spurtSeconds)
            {
                const float pulse = std::pow(std::max(0.0f, std::sin(kPi2 * pulseHz * static_cast<float>(age))), 4.0f);
                const float fade = 1.0f - static_cast<float>(age / spurtSeconds) * 0.7f;
                count = pulse > 0.25f ? 1 + static_cast<int>(pulse * 2.0f * fade) : 0;
                next = 0.05;
            }
            else if (age < kDripSeconds)
            {
                count = 1;                                // slow drips
                drip = true;
                dirBind = glm::vec3(0, 0, 0);
                next = 0.35;
            }
            else
            {
                continue;
            }

            cut.fxNext = now + next;
            if (count <= 0 || g_budget <= 0) continue;
            count = std::min(count, g_budget);
            g_budget -= count;

            const glm::mat4 M = BoneBindToWorld(World, palette, cut.boneSkin);
            const glm::vec3 point = glm::vec3(M * glm::vec4(cut.center, 1.0f));
            glm::vec3 dir = glm::vec3(0, -1, 0);
            if (glm::length(dirBind) > 0.1f)
            {
                dir = glm::normalize(glm::transpose(glm::inverse(glm::mat3(M))) * dirBind);
                dir.y += 0.25f;                           // arterial pressure lifts the jet a little
                dir = glm::normalize(dir);
            }

            GlobalParticleSystem::SpawnParticleAt("gore_spurt", point, MathHelper::FindLookAtRotation(glm::vec3(0), dir),
                                                  glm::vec3(static_cast<float>(count), drip ? 0.15f : 1.0f, 0.0f));
        }
    }
}

TICK_ENTITY(GoreStumpFxSystem, PostLateUpdate, Gore::GoreComponent, self, gore)
{
    Gore::UpdateStumpFx(self, gore);
}
REGISTER_TICK(GoreStumpFxSystem)
