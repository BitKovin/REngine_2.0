// Time.hpp first: engine headers below use Time without including it.
#include <Time.hpp>

#include "SimPawn.h"

#include <Sim/Movement/SimSpeedTable.h>

#include <Camera.h>
#include <Console/ConsoleRegister.h>
#include <Helpers/JsonMacros.hpp>
#include <Input.h>
#include <Level.hpp>
#include <Logger.hpp>
#include <MathHelper.hpp>
#include <Physics.h>
#include <RegisterLevelObject.h>

namespace
{
	constexpr float Radius = 0.4f;
	constexpr float JumpSpeed = 9.5f;
	constexpr float GroundAcceleration = 40.0f;
	constexpr float AirAcceleration = 6.0f;
	// Third-person camera boom: behind the eye along the view, up a little, pulled in by a sphere trace so it never enters a wall.
	constexpr float ThirdPersonDistance = 2.4f;
	constexpr float ThirdPersonUp = 0.3f;
	constexpr float ThirdPersonRadius = 0.3f;
	// One pawn per level; SimSpawn asks for it from FromData, before the previous level's pawn is necessarily gone.
	int pawnLevelChange = -1;
}

SimPawn::SimPawn()
{
	ClassName = "sim_pawn";
	Name = "sim_pawn";
	SaveGame = true;
	Tags = { "player" };
}

SimPawn::~SimPawn()
{
	if (Instance == this)
		Instance = nullptr;
}

void SimPawn::Destroy()
{
	if (CharacterBody* body = Body())
		body->Destroy();
	Entity::Destroy();
}

CharacterBody* SimPawn::Body() const
{
	return dynamic_cast<CharacterBody*>(const_cast<EntityHandle&>(bodyHandle).Resolve());
}

SimPawn* SimPawn::SpawnAt(const glm::vec3& feet, float yaw, float pitch)
{
	if (!SpawnEnabled || !Level::Current || pawnLevelChange == Level::CurrentLevelChangeId)
		return nullptr;
	pawnLevelChange = Level::CurrentLevelChangeId;

	Entity* entity = LevelObjectFactory::instance().create("sim_pawn");
	SimPawn* pawn = dynamic_cast<SimPawn*>(entity);
	if (!pawn)
		return nullptr;
	pawn->Position = feet + vec3(0.0f, 0.9f, 0.0f);
	pawn->cameraRotation = vec3(glm::clamp(LaunchPitch.value_or(pitch), -89.0f, 89.0f), yaw, 0.0f);
	LaunchPitch.reset();
	Level::Current->AddEntity(pawn);
	return pawn;
}

void SimPawn::Start()
{
	Entity::Start();
	Instance = this;
	controller.Init(this, Position, Radius);
	scriptedLast = Position;
}

void SimPawn::Teleport(vec3 target)
{
	Position = target;
	controller.SetPosition(target);
	controller.heightSmoothOffset = 0;
	scriptedLast = target;
}

glm::vec3 SimPawn::FeetPosition() const
{
	return Position - vec3(0.0f, controller.height * 0.5f, 0.0f);
}

void SimPawn::LookAt(const glm::vec3& point)
{
	const vec3 eye = Position + vec3(0.0f, controller.GetCameraHeight() - controller.height * 0.5f, 0.0f);
	const vec3 rotation = MathHelper::FindLookAtRotation(eye, point);
	cameraRotation = vec3(glm::clamp(rotation.x, -89.0f, 89.0f), rotation.y, 0.0f);
}

void SimPawn::StartScriptedWalk(float metres)
{
	scriptedTarget = metres;
	scriptedWalked = 0.0f;
	scriptedStuck = 0.0f;
	scriptedLast = Position;
}

void SimPawn::Update()
{
	controller.Update(Time::DeltaTimeF);
	Position = controller.GetSmoothPosition();

	const bool scripted = scriptedTarget > 0.0f && !ScriptedWalkDone();
	vec2 input(0.0f);

	if (scripted)
	{
		input = vec2(0.0f, 1.0f);
		const vec3 moved = Position - scriptedLast;
		scriptedLast = Position;
		const float step = glm::length(vec2(moved.x, moved.z));
		scriptedWalked += step;
		// Against a wall: turn away.
		scriptedStuck = step < 0.3f * Time::DeltaTimeF * sim::movement::Speed(sim::movement::MoverClass::Person, sim::movement::Gait::Run, sim::movement::GroundArea) ? scriptedStuck + Time::DeltaTimeF : 0.0f;
		if (scriptedStuck > 0.4f)
		{
			cameraRotation.y += 90.0f;
			scriptedStuck = 0.0f;
		}
	}
	else
	{
		if (Input::LockCursor)
		{
			cameraRotation.y += Input::MouseDelta.x;
			cameraRotation.x -= Input::MouseDelta.y;
			cameraRotation.x = glm::clamp(cameraRotation.x, -89.0f, 89.0f);
		}

		input = Input::GetLeftStickPosition();
		if (Input::GetAction("forward")->Holding())
			input += vec2(0, 1);
		if (Input::GetAction("backward")->Holding())
			input += vec2(0, -1);
		if (Input::GetAction("left")->Holding())
			input += vec2(-1, 0);
		if (Input::GetAction("right")->Holding())
			input += vec2(1, 0);
		if (glm::length(input) > 1.0f)
			input = glm::normalize(input);
	}

	const bool crouching = !scripted && Input::GetAction("crouch")->Holding();
	if (crouching)
		controller.Crouch();
	else
		controller.UnCrouch();

	using namespace sim::movement;
	const float speed = Speed(MoverClass::Person, crouching ? Gait::Walk : Gait::Run, GroundArea);
	const vec3 right = MathHelper::GetRightVector(vec3(0.0f, cameraRotation.y, 0.0f));
	const vec3 forward = MathHelper::GetForwardVector(vec3(0.0f, cameraRotation.y, 0.0f));
	const vec3 wish = (input.x * right + input.y * forward) * speed;

	velocity = controller.GetVelocity();
	const float acceleration = (controller.onGround ? GroundAcceleration : AirAcceleration) * Time::DeltaTimeF;
	vec3 horizontal(velocity.x, 0.0f, velocity.z);
	const vec3 delta = wish - horizontal;
	const float deltaLength = glm::length(delta);
	horizontal += deltaLength > acceleration ? delta / deltaLength * acceleration : delta;
	velocity = vec3(horizontal.x, velocity.y, horizontal.z);

	if (controller.onGround && !scripted && Input::GetAction("jump")->Holding())
		velocity.y = JumpSpeed;
	controller.SetVelocity(velocity);

	if (!scripted && Input::LockCursor)
		if (InputAction* toggle = Input::GetAction("view"); toggle && toggle->Pressed())
			View = View == ViewMode::FirstPerson ? ViewMode::ThirdPerson : ViewMode::FirstPerson;

	// Camera: the eye, or a boom behind it.
	const float feetY = Position.y - controller.height * 0.5f;
	Camera::position = vec3(Position.x, feetY + controller.GetCameraHeight(), Position.z);
	Camera::rotation = cameraRotation;
	if (View == ViewMode::ThirdPerson)
		PlaceThirdPersonCamera();

	// The look: created with the first frame (not in Start, which the level may run while it is still adding entities), then driven by the state.
	if (!bodyCreated)
	{
		bodyCreated = true;
		if (CharacterBody* body = CharacterBody::Spawn(MakeState(crouching)))
			bodyHandle = EntityHandle::FromEntity(body);
	}
	if (CharacterBody* body = Body())
		body->Apply(MakeState(crouching));
}

CharacterState SimPawn::MakeState(bool crouching)
{
	CharacterState state;
	state.feet = FeetPosition();
	state.bodyYaw = cameraRotation.y; // the body faces where the camera looks; strafing and backing up play their own clips
	state.look = glm::vec2(cameraRotation.x, cameraRotation.y);
	state.velocity = controller.GetVelocity();
	state.stance = crouching ? CharacterState::Stance::Crouching : CharacterState::Stance::Standing;
	state.onGround = controller.onGround;
	state.model = CharacterModels::PlayerCoat;
	state.visibility = View == ViewMode::FirstPerson ? CharacterState::Visibility::FirstPersonOwner : CharacterState::Visibility::Shown;
	return state;
}

// The camera sits behind the eye along the view and a little up; a sphere trace from the eye stops it at the first wall so the view never goes through one.
void SimPawn::PlaceThirdPersonCamera()
{
	const vec3 eye = Camera::position;
	const vec3 wanted = eye - MathHelper::GetForwardVector(cameraRotation) * ThirdPersonDistance + vec3(0.0f, ThirdPersonUp, 0.0f);
	const auto hit = Physics::SphereTrace(eye, wanted, ThirdPersonRadius, BodyType::GroupCollisionTest, {}, { this });
	Camera::position = hit.hasHit ? hit.shapePosition : wanted;
}

void SimPawn::Serialize(json& target)
{
	Entity::Serialize(target);
	SERIALIZE_FIELD(target, cameraRotation);
	SERIALIZE_FIELD(target, velocity);
}

void SimPawn::Deserialize(json& source)
{
	Entity::Deserialize(source);
	DESERIALIZE_FIELD(source, cameraRotation);
	DESERIALIZE_FIELD(source, velocity);
	controller.SetVelocity(velocity);
	Teleport(Position);
}

REGISTER_ENTITY(SimPawn, "sim_pawn")

#ifndef DISTRIBUTION
CONSOLE_FUNC("sim_view", "Switch the player's view: sim_view [first|third] (no argument toggles)")
{
	if (!args.empty())
		SimPawn::SetViewMode(args[0] == "third" || args[0] == "3" ? SimPawn::ViewMode::ThirdPerson : SimPawn::ViewMode::FirstPerson);
	else
		SimPawn::SetViewMode(SimPawn::GetViewMode() == SimPawn::ViewMode::FirstPerson ? SimPawn::ViewMode::ThirdPerson : SimPawn::ViewMode::FirstPerson);
	Logger::Log("sim_view: %s", SimPawn::GetViewMode() == SimPawn::ViewMode::FirstPerson ? "first person" : "third person");
}
#endif
