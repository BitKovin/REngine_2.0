// Time.hpp first: engine headers below use Time without including it.
#include <Time.hpp>

#include "SimPawn.h"

#include <Sim/Movement/SimSpeedTable.h>

#include <Camera.h>
#include <Helpers/JsonMacros.hpp>
#include <Input.h>
#include <Level.hpp>
#include <MathHelper.hpp>
#include <RegisterLevelObject.h>

namespace
{
	constexpr float Radius = 0.4f;
	constexpr float JumpSpeed = 9.5f;
	constexpr float GroundAcceleration = 40.0f;
	constexpr float AirAcceleration = 6.0f;
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

SimPawn* SimPawn::SpawnAt(const glm::vec3& feet, float yaw)
{
	if (!SpawnEnabled || !Level::Current || pawnLevelChange == Level::CurrentLevelChangeId)
		return nullptr;
	pawnLevelChange = Level::CurrentLevelChangeId;

	Entity* entity = LevelObjectFactory::instance().create("sim_pawn");
	SimPawn* pawn = dynamic_cast<SimPawn*>(entity);
	if (!pawn)
		return nullptr;
	pawn->Position = feet + vec3(0.0f, 0.9f, 0.0f);
	pawn->cameraRotation = vec3(0.0f, yaw, 0.0f);
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

	// Eye camera.
	const float feetY = Position.y - controller.height * 0.5f;
	Camera::position = vec3(Position.x, feetY + controller.GetCameraHeight(), Position.z);
	Camera::rotation = cameraRotation;
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
