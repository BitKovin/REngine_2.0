#include "Player.hpp"

#include <Particle/GlobalParticleSystem.hpp>

#include <EngineMain.h>

REGISTER_ENTITY(Player, "info_player_start")

Player* Player::Instance = nullptr;

string serializedPlayer = "";

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

    ImGui::Begin("debug");
    
    ImGui::Checkbox("fly", &freeFly);

    ImGui::Checkbox("draw physics", &Physics::DebugDraw);

    if (ImGui::Button("spawn test dog"))
    {
        Entity* entity = Spawn("testnpc");
        entity->Position = Camera::position + Camera::Forward() * 2.0f;
        entity->Start();
    }

    if (ImGui::Button("test serialization"))
    {

        json jPlayer;
        Serialize(jPlayer);

        serializedPlayer = jPlayer.dump(4);

        Logger::Log(serializedPlayer);

    }

    if (ImGui::Button("test DEserialization"))
    {

        json jPlayer = json::parse(serializedPlayer);
        Deserialize(jPlayer);

        Logger::Log(jPlayer.dump(4));

    }

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
        BodyType::GroupHitTest, { LeadBody });

    if (hit.hasHit)
    {
        hit.entity->OnPointDamage(10, hit.position, MathHelper::FastNormalize(hit.position - Camera::position), "", this, this);


        

        //soundPlayer->Sound->EnableReverb = true;
        //soundPlayer->Sound->ReverbDecayTime = 5;

        soundPlayer->MaxDistance = 100;

        //soundPlayer->Sound->EnableReverb = true;
        soundPlayer->Position = hit.position;
        //soundPlayer->Play();



    }

}

void Player::Update()
{
    OnGround = CheckGroundAt(Position);

    if (Input::LockCursor)
    {

        cameraRotation.y += Input::MouseDelta.x;
        cameraRotation.x -= Input::MouseDelta.y;

        if (OnGround)
            if (Input::GetAction("jump")->Holding())
            {
                Jump();

            }

        if (Input::GetAction("attack")->Pressed())
        {

            PerformAttack();

        }

    }



    vec2 input = Input::GetLeftStickPosition();

    if (Input::GetAction("forward")->Holding())
        input += vec2(0, 1);

    if (Input::GetAction("backward")->Holding())
        input += vec2(0, -1);

    if (Input::GetAction("left")->Holding())
        input += vec2(-1, 0);

    if (Input::GetAction("right")->Holding())
        input += vec2(1, 0);

    vec3 right = MathHelper::GetRightVector(Camera::rotation);

    vec3 forward = MathHelper::GetForwardVector(vec3(0, Camera::rotation.y, 0));

    if (freeFly)
        forward = Camera::Forward();

    if (length(input) > 1)
        input = normalize(input);

    vec3 movement = input.x * right + input.y * forward;

    Physics::Activate(LeadBody);

    velocity = FromPhysics(LeadBody->GetLinearVelocity());

    if (OnGround)
    {
        velocity = UpdateGroundVelocity(movement, velocity);
    }
    else
    {
        velocity = UpdateAirVelocity(movement, velocity);
    }


    velocity.y = LeadBody->GetLinearVelocity().GetY();

    if (freeFly)
        velocity = movement * 20.0f;

    LeadBody->SetLinearVelocity(ToPhysics(velocity));



    Camera::position = Position + vec3(0, 0.7, 0);
    Camera::rotation = cameraRotation;

    Camera::ApplyCameraShake(Time::DeltaTimeF);

    viewmodel->Update();

    arms->PasteAnimationPose(viewmodel->GetAnimationPose());



    if (Input::GetAction("qSave")->Pressed())
    {
        LevelSaveSystem::SaveLevelToFile("quicksave");
    }

    if (Input::GetAction("qLoad")->Pressed())
    {
        LevelSaveSystem::LoadLevelFromFile("quicksave");
    }
}

void Player::LateUpdate()
{

    viewmodel->Position = Camera::position + (mat3)Camera::GetRotationMatrix() * weaponOffset;
    viewmodel->Rotation = cameraRotation;

    arms->Position = viewmodel->Position;
    arms->Rotation = viewmodel->Rotation;

    Hud.Update();

}

void Player::Serialize(json& target)
{

    Entity::Serialize(target);

    SERIALIZE_FIELD(target, cameraRotation);
    SERIALIZE_FIELD(target, velocity);

}

void Player::OnDamage(float Damage, Entity* DamageCauser, Entity* Weapon)
{
    Entity::OnDamage(Damage, DamageCauser, Weapon);
}

void Player::OnPointDamage(float Damage, vec3 Point, vec3 Direction, string bone, Entity* DamageCauser, Entity* Weapon)
{
    Entity::OnPointDamage(Damage, Point, Direction, bone, DamageCauser, Weapon);

    GlobalParticleSystem::SpawnParticleAt("hit_flesh", Point, MathHelper::FindLookAtRotation(vec3(0), Direction), vec3(10));

}

void Player::Deserialize(json& source)
{

    Entity::Deserialize(source);

    DESERIALIZE_FIELD(source, cameraRotation);
    DESERIALIZE_FIELD(source, velocity);

    Physics::SetBodyPosition(LeadBody, Position);
    Physics::SetLinearVelocity(LeadBody, velocity);

}
