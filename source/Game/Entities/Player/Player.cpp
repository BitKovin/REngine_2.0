#include "Player.hpp"

#include <Particle/GlobalParticleSystem.hpp>

#include <EngineMain.h>

REGISTER_LEVEL_OBJECT(Player, "info_player_start")

Player* Player::Instance = nullptr;



void Player::UpdateDebugUI()
{
    ImGui::Begin("navigation");

    ImGui::Checkbox("draw nav mesh", &NavigationSystem::DebugDrawNavMeshEnabled);

    if (ImGui::Button("PlaceObstacle"))
    {
        NavigationSystem::RemoveObstacle(playerObstacle);
        playerObstacle = NavigationSystem::CreateObstacleBox(Position - vec3(1, 1, 1), Position + vec3(1, 1, 1));
    }

    if (ImGui::Button("place start location"))
    {
        testStart = Position;
        DebugDraw::Line(Position, Position - vec3(0, 1, 0), 2, 0.1);
    }

    if (ImGui::Button("calculate path to player"))
    {
        auto path = NavigationSystem::FindSimplePath(Position, testStart);

        path.insert(path.begin(), Position);

        DebugDraw::Path(path, 15, 0.05);
    }

    ImGui::End();

    ImGui::Begin("weapon");
    ImGui::DragFloat3("offset", &weaponOffset.x, 0.01);
    ImGui::End();

    ImGui::Begin("graphic");
    ImGui::SliderInt("multisample count",&EngineMain::MainInstance->MainRenderer->MultiSampleCount,0,8);
    ImGui::End();

}

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


        GlobalParticleSystem::SpawnParticleAt("hit_flesh", hit.position, MathHelper::FindLookAtRotation(vec3(0), hit.normal), vec3(10));


    }

}
