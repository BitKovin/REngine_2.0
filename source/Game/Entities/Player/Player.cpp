#include "Player.hpp"

#include <Particle/GlobalParticleSystem.hpp>

#include <EngineMain.h>

REGISTER_ENTITY(Player, "info_player_start")

Player* Player::Instance = nullptr;

string serializedPlayer = "";

void Player::UpdateWalkMovement(vec2 input)
{

    vec3 right = MathHelper::GetRightVector(Camera::rotation);

    vec3 forward = MathHelper::GetForwardVector(vec3(0, Camera::rotation.y, 0));

    if (freeFly)
        forward = Camera::Forward();

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

    if (OnGround) 
    {
        if (Input::GetAction("jump")->Holding())
        {
            Jump();

        }
    }
}

void Player::UpdateBikeMovement(vec2 input)
{
    // Bike movement parameters
    const float maxSpeed = 15.0f;
    const float acceleration = 15.0f;
    const float lateralFriction = 1.5f;
    const float deltaTime = Time::DeltaTimeF; // Implement time handling

    vec3 moveRot = vec3(0, Camera::rotation.y - (input.x * 45.0 * 0), 0);

    // Get bike's forward direction (based on camera yaw)
    vec3 forward = MathHelper::GetForwardVector(moveRot);
    vec3 right = MathHelper::GetRightVector(moveRot);

    

    // Always move forward (override input)
    input = vec2(0, 1);
    vec3 movementDirection = input.x * right + input.y * forward;

    Physics::Activate(LeadBody);
    velocity = FromPhysics(LeadBody->GetLinearVelocity());

    // Separate vertical and horizontal components
    float verticalVelocity = velocity.y;
    vec3 horizontalVelocity = vec3(velocity.x, 0.0f, velocity.z);

    // Calculate forward speed and lateral velocity
    float currentForwardSpeed = glm::dot(horizontalVelocity, forward);
    vec3 lateralVelocity = horizontalVelocity - (forward * currentForwardSpeed);

    // Apply forward acceleration with speed cap
    currentForwardSpeed = glm::min(currentForwardSpeed + acceleration * deltaTime, maxSpeed);

    // Apply lateral friction (drift effect)
    lateralVelocity *= glm::max(1.0f - lateralFriction * deltaTime, 0.0f);

    // Combine new velocity components
    vec3 newHorizontalVelocity = (forward * currentForwardSpeed) + lateralVelocity;
    vec3 newVelocity = vec3(newHorizontalVelocity.x, verticalVelocity, newHorizontalVelocity.z);

    // Update physics body velocity
    LeadBody->SetLinearVelocity(ToPhysics(newVelocity));

    bikeMesh->Rotation.z = -dot(velocity, right)*2.5f;
    bikeMesh->Rotation.y -= bikeMesh->Rotation.z * 0.2f;


    AnimationPose pose = bikeMesh->GetAnimationPose();

    MathHelper::Transform frontRot = pose.GetBoneTransform("pelvis");
	MathHelper::Transform wheelRot = pose.GetBoneTransform("wheel_front");

	frontRot.Rotation -= vec3(0, bikeMesh->Rotation.z * 1.0, 0);
	wheelRot.Rotation += vec3(Time::GameTime * 1000.0f, 0, 0);

	pose.SetBoneTransformEuler("pelvis", frontRot);
	pose.SetBoneTransformEuler("wheel_front", wheelRot);

    bikeMesh->PasteAnimationPose(pose);
    bikeArmsMesh->PasteAnimationPose(bikeMesh->GetAnimationPose());
    bikeArmsMesh->Rotation = bikeMesh->Rotation;


    if (OnGround)
    {
        if (Input::GetAction("jump")->Holding())
        {
            Jump();

        }
    }

}

void Player::CreateWeapon(const string& className)
{

    Weapon* weap = (Weapon*)Spawn(className);

    weap->Start();
    weap->LoadAssetsIfNeeded();

    currentWeapon = weap;

}

void Player::DestroyWeapon()
{
    if (currentWeapon)
    {
        currentWeapon->Destroy();
        currentWeapon = nullptr;
    }
}

void Player::UpdateWeapon()
{
    if (currentWeapon == nullptr) return;

    currentWeapon->Position = Camera::position;
    currentWeapon->Rotation = cameraRotation;

}

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


}

void Player::Update()
{
    OnGround = CheckGroundAt(Position);

    if (Input::LockCursor)
    {

        cameraRotation.y += Input::MouseDelta.x;
        cameraRotation.x -= Input::MouseDelta.y;

        cameraRotation.x = glm::clamp(cameraRotation.x, -89.0f,89.0f);

        if (on_bike)
        {
            cameraRotation.x = glm::clamp(cameraRotation.x, -50.0f, 59.0f);
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

    if (length(input) > 1)
        input = normalize(input);

    if (on_bike == false)
    {
        UpdateWalkMovement(input);
    }

    bikeMesh->Position = Position - vec3(0, 0.9f - 0.8f, 0);
    bikeMesh->Rotation = vec3(0, cameraRotation.y, 0);
    bikeMesh->Update();
    if (on_bike)
    {
        UpdateBikeMovement(input);
    }
    bikeArmsMesh->Rotation = bikeMesh->Rotation;
    bikeArmsMesh->Position = bikeMesh->Position;
    bikeArmsMesh->PasteAnimationPose(bikeMesh->GetAnimationPose());



    Camera::position = Position + vec3(0, 0.7, 0);
    Camera::rotation = cameraRotation;

    Camera::ApplyCameraShake(Time::DeltaTimeF);


    UpdateWeapon();

    if (Input::GetAction("bike")->Holding() && OnGround)
    {
        StartBike();
    }
    else
    {
        StopBike();
    }

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

void Player::StartBike()
{
    if (on_bike) return;

    bikeMesh->PlayAnimation("draw", true, 0.7f);
    on_bike = true;
}

void Player::StopBike()
{
    if (on_bike == false) return;

    bikeMesh->PlayAnimation("hide", true, 0.7f);
    on_bike = false;
}

void Player::ToggleBike()
{
    if (on_bike)
    {
        StopBike();
    }
    else
    {
        StartBike();
    }
}

void Player::LoadAssets()
{
    bikeMesh->LoadFromFile("GameData/Models/Player/Bike/bike.glb");
    bikeMesh->TexturesLocation = "GameData/Models/Player/Bike/Textures/";
    bikeMesh->PreloadAssets();
    bikeMesh->PlayAnimation("hide",true);

    bikeArmsMesh->LoadFromFile("GameData/arms.glb");
    bikeArmsMesh->PreloadAssets();

}
