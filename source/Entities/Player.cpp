#include "Player.hpp"

#include "../Particle/GlobalParticleSystem.hpp"

REGISTER_LEVEL_OBJECT(Player, "info_player_start")

Player* Player::Instance = nullptr;

void Player::PerformAttack()
{
    viewmodel->PlayAnimation("attack");
    Camera::AddCameraShake(CameraShake(
        0.13f,                            // interpIn
        1.2f,                            // duration
        vec3(0.0f, 0.0f, -0.2f),         // positionAmplitude
        vec3(0.0f, 0.0f, 6.4f),          // positionFrequency
        vec3(-8, 0.15f, 0.0f),        // rotationAmplitude
        vec3(-5.0f, 28.8f, 0.0f),        // rotationFrequency
        1.2f,                            // falloff
        CameraShake::ShakeType::SingleWave // shakeType
    ));

    auto hit = Physics::LineTrace(Camera::position, Camera::position + Camera::Forward() * 1000.0f, 
        BodyType::GroupHitTest | BodyType::CharacterCapsule, { LeadBody });

    if (hit.hasHit)
    {
        hit.entity->OnPointDamage(10, hit.position, MathHelper::FastNormalize(hit.position - Camera::position), "", this, this);


        GlobalParticleSystem::SpawnParticleAt("decal_blood", hit.position, hit.normal, vec3(1));


    }

}
