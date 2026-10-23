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

// ---- Animator ---------------------------------------------------------------------------------------------------------

void SimNpcAnimator::LoadAssets()
{
	locomotion = AddAnimation(modelPath, "idle");
}

void SimNpcAnimator::Update()
{
	if (!loaded)
		return;
	const char* wanted = movementSpeed > 3.0f ? "run" : movementSpeed > 1.0f ? "walk" : "idle";
	if (locomotion->currentAnimationData->animationName != wanted)
		locomotion->PlayAnimation(wanted, true, 0.2f);
	Animator::Update();
}

// ---- Body -------------------------------------------------------------------------------------------------------------

const char* SimNpc::ModelPath(int model)
{
	return model == 1 ? "GameData/models/npc/guard.glb" : "GameData/models/npc/base.glb";
}

SimNpc* SimNpc::SpawnAt(const glm::vec3& feet, const glm::vec3& heading, int model, float speedScale, uint32_t variant)
{
	if (!Level::Current)
		return nullptr;
	SimNpc* npc = dynamic_cast<SimNpc*>(LevelObjectFactory::instance().create("sim_agent_body"));
	if (!npc)
		return nullptr;
	npc->model = model;
	npc->variant = variant;
	npc->speedScale = speedScale;
	npc->Position = feet + glm::vec3(0.0f, Height * 0.5f, 0.0f);
	if (glm::dot(heading, heading) > 1e-6f)
		npc->facing = glm::normalize(glm::vec3(heading.x, 0.0f, heading.z));
	Level::Current->AddEntity(npc);
	npc->Start();
	npc->LoadAssetsIfNeeded();
	return npc;
}

SimNpc::SimNpc()
{
	ClassName = "sim_agent_body";
	SaveGame = false;
	Tags = { "npc", "citizen" };
	mesh = new SkeletalMesh(this);
	Drawables.push_back(mesh);
	mesh->UpdatePoseOnlyWhenRendered = true;
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
	mesh->Position = Position - glm::vec3(0.0f, Height * 0.5f, 0.0f);
	mesh->Rotation = glm::vec3(0.0f, MathHelper::FindLookAtRotation(glm::vec3(), facing).y, 0.0f);
}

void SimNpc::LoadAssets()
{
	mesh->LoadFromFile(ModelPath(model));
	mesh->DepthPrePath = false;
	mesh->Masked = true;
	animator = std::make_unique<SimNpcAnimator>(this, ModelPath(model));
	animator->LoadAssetsIfNeeded();
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

// base.glb has no textures at all and renders black. A mesh draws with ONE colour texture and one tint (the engine copies both from the mesh to
// every part each frame), so the citizen model is a plain white-textured figure tinted per person: readable and varied, but a mannequin. guard.glb
// brings its own textures and is left alone.
void SimNpc::ApplyPaint()
{
	if (paintApplied || model == 1)
		return;
	static const glm::vec4 tints[] = { { 0.62f, 0.50f, 0.40f, 1 }, { 0.40f, 0.48f, 0.62f, 1 }, { 0.58f, 0.58f, 0.50f, 1 }, { 0.46f, 0.58f, 0.44f, 1 }, { 0.68f, 0.42f, 0.40f, 1 }, { 0.54f, 0.52f, 0.60f, 1 } };
	mesh->ColorTexture = AssetRegistry::GetTextureFromFile("GameData/textures/generic/white.png");
	mesh->Color = tints[((variant * 2654435761u) >> 8) % 6];
	paintApplied = true;
}

void SimNpc::FadeIn()
{
	alpha = 0.0f;
	fadingIn = true;
	fadingOut = false;
	UpdateFade(0.0f); // invisible from the first frame
}

void SimNpc::FadeOut()
{
	fadingIn = false;
	fadingOut = true;
}

// While the alpha is below 1 the mesh is drawn blended (a tint alpha under 1 makes it a Transparent surface, which never writes depth, so the
// limbs show through each other for the second a fade lasts); at 1 it goes back to the alpha-tested Masked draw.
void SimNpc::UpdateFade(float deltaTime)
{
	if (fadingIn)
	{
		alpha = std::min(1.0f, alpha + deltaTime / FadeSeconds);
		fadingIn = alpha < 1.0f;
	}
	else if (fadingOut)
		alpha = std::max(0.0f, alpha - deltaTime / FadeSeconds);
	mesh->Color.a = alpha;
	mesh->Masked = alpha >= 1.0f;
}

void SimNpc::Update()
{
	if (!controller)
		return;
	ApplyPaint();
	UpdateFade(Time::DeltaTimeF);
	const float deltaTime = Time::DeltaTimeF;
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

	mesh->Position = Position - glm::vec3(0.0f, Height * 0.5f, 0.0f);
	mesh->Rotation = glm::vec3(0.0f, MathHelper::FindLookAtRotation(glm::vec3(), facing).y, 0.0f);
	UpdateAnimation(horizontalSpeed);
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

void SimNpc::UpdateAnimation(float horizontalSpeed)
{
	if (EngineMain::MainInstance->SimulatingGameTicks)
		mesh->WasRended = false;
	if (!animator || !mesh->WasRended)
		return;
	animator->movementSpeed = horizontalSpeed;
	animator->Update();
	mesh->PasteAnimationPose(animator->GetResultPose());
}

REGISTER_ENTITY(SimNpc, "sim_agent_body")
