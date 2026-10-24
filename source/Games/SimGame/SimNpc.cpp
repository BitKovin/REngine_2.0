// Time.hpp first: engine headers below use Time without including it.
#include <Time.hpp>

#include "SimNpc.h"

#include <AssetRegistry.h>
#include <EngineMain.h>
#include <Level.hpp>
#include <LevelObjectFactory.h>
#include <MathHelper.hpp>
#include <RegisterLevelObject.h>

#include <algorithm>

namespace
{
	constexpr float Radius = 0.45f;
	constexpr float Height = 1.8f;
	constexpr float Mass = 90.0f;
	constexpr float Acceleration = 14.0f;     // m/s^2 on the ground
	constexpr float TurnSpeed = 6.0f;         // how fast the body turns towards where it walks (Interp rate)
	constexpr float ArriveDistance = 0.3f;    // metres from the goal at which a Move goal counts as reached
	constexpr float BlockedAfter = 1.5f;      // seconds of wanting to move without moving before the path is asked for again
	constexpr float BlockedSpeedFraction = 0.2f;
}

// ---- Driver -----------------------------------------------------------------------------------------------------------

SimNpc* SimNpc::SpawnAt(const glm::vec3& feet, const glm::vec3& heading, int model, float speedScale, uint32_t variant, bool warming)
{
	if (!Level::Current)
		return nullptr;
	SimNpc* npc = dynamic_cast<SimNpc*>(LevelObjectFactory::instance().create("sim_agent_body"));
	if (!npc)
		return nullptr;
	npc->model = model;
	npc->variant = variant;
	npc->warming = warming;
	npc->speedScale = speedScale;
	npc->Position = feet + glm::vec3(0.0f, Height * 0.5f, 0.0f);
	if (glm::dot(heading, heading) > 1e-6f)
		npc->facing = glm::normalize(glm::vec3(heading.x, 0.0f, heading.z));
	Level::Current->AddEntity(npc);
	npc->Start();
	npc->LoadAssetsIfNeeded();
	CharacterBody* body = CharacterBody::Spawn(npc->MakeState(glm::vec3(0.0f)));
	if (body)
		npc->bodyHandle = EntityHandle::FromEntity(body);
	return npc;
}

SimNpc::SimNpc()
{
	ClassName = "sim_agent_body";
	SaveGame = false;
	Tags = { "npc", "citizen" };
	pathFollow.reachedTarget = true;
	pathFollow.allowPartialPath = true;
	pathFollow.acceptanceRadius = ArriveDistance;
}

SimNpc::~SimNpc()
{
	delete controller;
}

void SimNpc::Start()
{
	Entity::Start();
	controller = new CharacterController();
	controller->movementQuality = CharacterControllerMovementQuality::NpcGeneric;
	controller->Init(this, Position, Radius, Height, Mass);
}

void SimNpc::Destroy()
{
	if (CharacterBody* body = Body())
		body->Destroy();
	Entity::Destroy();
}

CharacterBody* SimNpc::Body() const
{
	return dynamic_cast<CharacterBody*>(const_cast<EntityHandle&>(bodyHandle).Resolve());
}

bool SimNpc::WasRendered() const
{
	const CharacterBody* body = Body();
	return body && body->Drawn();
}

// Everything the look needs, from this driver's movement and presence.
CharacterState SimNpc::MakeState(const glm::vec3& velocity) const
{
	CharacterState state;
	state.feet = FeetPosition();
	state.bodyYaw = MathHelper::FindLookAtRotation(glm::vec3(), facing).y;
	state.look = glm::vec2(0.0f, state.bodyYaw);
	state.velocity = velocity;
	state.model = model;
	state.variant = variant;
	state.alpha = alpha;
	state.visibility = warming ? CharacterState::Visibility::Warming : CharacterState::Visibility::Shown;
	return state;
}

glm::vec3 SimNpc::FeetPosition() const
{
	return Position - glm::vec3(0.0f, Height * 0.5f, 0.0f);
}

void SimNpc::SetGoal(const glm::vec3& feet, const glm::vec3& next, bool hasNext, sim::movement::Gait newGait, bool final)
{
	goal = feet;
	nextGoal = next;
	hasNextGoal = hasNext && !final;
	gait = newGait;
	finalGoal = final;
	hasGoal = true;
	state = State::Moving;
	blockedFor = 0.0f;
	pathFollow.WaitToFinish();
	pathFollow.CalculatedPath = false;
	pathFollow.reachedTarget = false;
	pathFollow.isPerformingDelay.AddDelay(-1);
}

void SimNpc::Stop()
{
	hasGoal = false;
	state = State::Idle;
	blockedFor = 0.0f;
	pathFollow.WaitToFinish();
	pathFollow.CalculatedPath = true;
	pathFollow.reachedTarget = true;
}

void SimNpc::FadeIn()
{
	alpha = 0.0f;
	fadingIn = true;
	fadingOut = false;
}

void SimNpc::FadeOut()
{
	fadingIn = false;
	fadingOut = true;
}

// The alpha runs as the character fades; CharacterBody draws it blended while it is below 1 and back to the alpha-tested draw at 1.
void SimNpc::UpdateFade(float deltaTime)
{
	if (fadingIn)
	{
		alpha = std::min(1.0f, alpha + deltaTime / FadeSeconds);
		fadingIn = alpha < 1.0f;
	}
	else if (fadingOut)
		alpha = std::max(0.0f, alpha - deltaTime / FadeSeconds);
}

void SimNpc::Update()
{
	if (!controller)
		return;
	const float deltaTime = Time::DeltaTimeF;
	age += deltaTime;
	UpdateFade(deltaTime);
	controller->Update(deltaTime);
	Position = controller->GetSmoothPosition();
	Navigate(deltaTime);

	const glm::vec3 velocity = controller->GetVelocity();
	const float horizontalSpeed = glm::length(glm::vec3(velocity.x, 0.0f, velocity.z));
	if (horizontalSpeed > 0.2f)
	{
		const glm::vec3 direction = glm::vec3(velocity.x, 0.0f, velocity.z) / horizontalSpeed;
		facing = MathHelper::Interp(facing, direction, deltaTime, TurnSpeed);
		if (glm::length(facing) > 1e-4f)
			facing = glm::normalize(facing);
	}

	if (CharacterBody* body = Body())
		body->Apply(MakeState(velocity));
}

// The body's own movement: path to the goal on the level mesh, follow its corners, slow to a stop at the end. The state
// machine is what grows later (doors, other bodies, reactions); the sim keeps dictating only the goal.
void SimNpc::Navigate(float deltaTime)
{
	using namespace sim::movement;
	const glm::vec3 current = controller->GetVelocity();
	glm::vec3 wish(0.0f);

	if (hasGoal)
	{
		const glm::vec3 feet = FeetPosition();
		const float toGoal = glm::length(glm::vec3(goal.x - feet.x, 0.0f, goal.z - feet.z));
		if (toGoal <= ArriveDistance && hasNextGoal)
		{
			// A corner on the way: walk on to the one after it. The sim names it as the goal a moment later.
			goal = nextGoal;
			hasNextGoal = false;
			pathFollow.WaitToFinish();
			pathFollow.CalculatedPath = false;
			pathFollow.reachedTarget = false;
			pathFollow.isPerformingDelay.AddDelay(-1);
		}
		else if (toGoal <= ArriveDistance)
		{
			// Reached the end, or the sim has not named the next corner yet: stand here until it does.
			state = State::Idle;
			pathFollow.WaitToFinish();
			pathFollow.CalculatedPath = true;
			pathFollow.reachedTarget = true;
		}
		else
		{
			state = state == State::Blocked ? State::Blocked : State::Moving;
			if (!pathFollow.reachedTarget || !pathFollow.CalculatedPath)
			{
				pathFollow.UpdateStartAndTarget(Position, goal + glm::vec3(0.0f, Height * 0.5f, 0.0f));
				pathFollow.TryPerform();
			}
			if (pathFollow.CalculatedPath && pathFollow.FoundTarget)
			{
				const glm::vec3 aim = pathFollow.CalculatedTargetLocation;
				const glm::vec3 direction(aim.x - Position.x, 0.0f, aim.z - Position.z);
				if (glm::length(direction) > 0.02f)
					wish = glm::normalize(direction) * (Speed(MoverClass::Person, gait, GroundArea) * speedScale);
			}
			else if (pathFollow.CalculatedPath)
			{
				// No path (partial or none): walk straight at the goal and let the controller and the nav query sort it out.
				const glm::vec3 direction(goal.x - feet.x, 0.0f, goal.z - feet.z);
				wish = glm::normalize(direction) * (Speed(MoverClass::Person, gait, GroundArea) * speedScale);
			}
		}
	}

	// Blocked: wants to go somewhere, is not getting there.
	const glm::vec3 horizontal(current.x, 0.0f, current.z);
	if (glm::length(wish) > 0.01f && glm::length(horizontal) < BlockedSpeedFraction * glm::length(wish))
		blockedFor += deltaTime;
	else
	{
		blockedFor = 0.0f;
		if (state == State::Blocked)
			state = State::Moving;
	}
	if (blockedFor > BlockedAfter && hasGoal)
	{
		state = State::Blocked;
		blockedFor = 0.0f;
		pathFollow.WaitToFinish();
		pathFollow.CalculatedPath = false;
		pathFollow.reachedTarget = false;
		pathFollow.isPerformingDelay.AddDelay(-1);
	}

	// Accelerate towards the wish.
	glm::vec3 next = horizontal;
	const glm::vec3 delta = wish - horizontal;
	const float deltaLength = glm::length(delta);
	const float step = (deltaLength > 0.0f && glm::length(wish) < 0.01f ? Acceleration * 1.5f : Acceleration) * deltaTime;
	next += deltaLength > step ? delta / deltaLength * step : delta;
	controller->SetVelocity(glm::vec3(next.x, current.y, next.z));
}

REGISTER_ENTITY(SimNpc, "sim_agent_body")
