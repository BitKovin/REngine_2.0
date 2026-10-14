#include "PlayerController.h"

#include <AI/AIController.h>
#include <Characters/CombatCharacter.h>
#include <Data/CombatData.h>
#include <Debug/CombatDebug.h>
#include <UI/CombatHud.h>

#include <AssetRegistry.h>
#include <Camera.h>
#include <EngineMain.h>
#include <Input.h>
#include <Logger.hpp>
#include <Physics.h>
#include <Time.hpp>

#include <cmath>

namespace
{
	PlayerController* activeController = nullptr;

	void MapButton(CharacterInput& input, InputButton button, const char* actionName, double now)
	{
		InputAction* action = Input::GetAction(actionName);
		if (action == nullptr)
			return;

		if (action->Pressed() && !action->Holding())
			input.Tap(button, now);   // pressed and released within one frame
		else
			input.SetButton(button, action->Holding(), now);
	}
}

PlayerController::PlayerController()
{
	ClassName = "player_controller";
}

PlayerController::~PlayerController()
{
	if (activeController == this)
		activeController = nullptr;
}

PlayerController* PlayerController::GetActive()
{
	return (activeController != nullptr && !activeController->Destroyed) ? activeController : nullptr;
}

const PlayerFeedbackData& PlayerController::GetFeedback() const
{
	static const PlayerFeedbackData defaults;
	return config ? config->feedback : defaults;
}

void PlayerController::LoadAssets()
{
	CombatData::LoadScope scope;
	config = CombatData::LoadPlayerConfig();

	// HUD textures, so nothing is read from disk when the HUD first draws.
	AssetRegistry::GetTextureFromFile("GameData/textures/generic/white.png");
	AssetRegistry::GetTextureFromFile("GameData/textures/ui/white.png");
}

void PlayerController::Start()
{
	LoadAssetsIfNeeded();
	Entity::Start();

	activeController = this;
	if (config)
		camera.Configure(config->camera);
	else
		Logger::Error("[PlayerController] %s failed to load - using default camera settings", CombatData::PlayerConfigPath);

	hud = std::make_shared<CombatHud>();
	EngineMain::Viewport.AddChild(hud);
}

void PlayerController::Destroy()
{
	if (Destroyed)
		return;

	if (hud)
	{
		EngineMain::Viewport.RemoveChild(hud);
		hud.reset();
	}

	if (CombatCharacter* body = GetPossessedBody())
		body->SetController(body->GetAIController());

	if (activeController == this)
		activeController = nullptr;

	Entity::Destroy();
}

// ---- Bodies -----------------------------------------------------------------------------------------------

CombatCharacter* PlayerController::GetHero()
{
	return dynamic_cast<CombatCharacter*>(ResolveAlive(hero));
}

CombatCharacter* PlayerController::GetPossessedBody()
{
	return dynamic_cast<CombatCharacter*>(ResolveAlive(possessed));
}

void PlayerController::SetHero(CombatCharacter* body)
{
	hero = body ? EntityHandle::FromEntity(body) : EntityHandle();
	respawnDelay = Delay();
	playerDied = false;
	restartRequested = false;
	if (body)
	{
		BindBody(body);
		camera.Reset(*body);
	}
}

void PlayerController::BindBody(CombatCharacter* body)
{
	body->SetController(this);   // calls OnPossess
}

void PlayerController::OnPossess(CombatCharacter& body)
{
	possessed = EntityHandle::FromEntity(&body);
}

void PlayerController::OnUnpossess(CombatCharacter& body)
{
	if (possessed == EntityHandle::FromEntity(&body))
		possessed.Clear();
}

bool PlayerController::Possess(CombatCharacter* target, std::string* reason)
{
	auto fail = [reason](const char* text) { if (reason) *reason = text; return false; };

	if (!config || !config->possession.enabled) return fail("possession is disabled");
	if (target == nullptr) return fail("no target");
	if (target->IsDead()) return fail("target is dead");
	if (!target->GetData() || !target->GetData()->possession.allowed) return fail("target cannot be possessed");
	if (target->GetCombatState()->inFinisher) return fail("target is in a finisher");

	CombatCharacter* heroBody = GetHero();
	if (target == heroBody) return fail("already the hero");

	CombatCharacter* current = GetPossessedBody();
	if (current == target) return fail("already possessed");
	if (current != nullptr && current != heroBody)
		Release(config->possession.releaseMode);

	// The hero keeps fighting on its own while the player is elsewhere.
	if (heroBody && !heroBody->IsDead() && heroBody->GetAIController())
	{
		heroBody->GetAIController()->SetBrain(heroBody->GetData()->possession.idleBrain);
		heroBody->SetController(heroBody->GetAIController());
	}

	target->SetTeamId(Team::Player);
	BindBody(target);
	return true;
}

void PlayerController::Release(PossessionReleaseMode mode)
{
	CombatCharacter* body = GetPossessedBody();
	CombatCharacter* heroBody = GetHero();
	if (body == nullptr || body == heroBody)
		return;

	if (AIController* ai = body->GetAIController())
	{
		if (mode == PossessionReleaseMode::Ally && !body->IsDead())
		{
			body->SetTeamId(Team::Player);
			ai->SetBrain(body->GetData()->possession.idleBrain);
		}
		else
		{
			body->SetTeamId(body->GetTeam()->originalTeam);
			ai->SetBrain(body->GetData()->ai.brain);
		}
		body->SetController(ai);
	}
	else
	{
		body->SetController(nullptr);
	}

	if (heroBody && !heroBody->IsDead())
	{
		if (AIController* heroAi = heroBody->GetAIController())
			heroAi->SetBrain(heroBody->GetData()->ai.brain);
		BindBody(heroBody);
	}
}

CombatCharacter* PlayerController::FindPossessionCandidate()
{
	if (!config || !config->possession.enabled)
		return nullptr;

	CombatCharacter* heroBody = GetHero();
	CombatCharacter* current = GetPossessedBody();
	const glm::vec3 origin = Camera::position;
	const glm::vec3 forward = camera.GetForward();
	const float cosLimit = std::cos(glm::radians(config->possession.aimAngle));

	CombatCharacter* best = nullptr;
	float bestCos = cosLimit;

	auto view = EcsWorld::Registry().view<CombatStateComponent>();
	for (entt::entity id : view)
	{
		auto* candidate = dynamic_cast<CombatCharacter*>(EcsWorld::GetEntity(id));
		if (candidate == nullptr || candidate == heroBody || candidate == current || candidate->Destroyed)
			continue;
		if (candidate->IsDead() || !candidate->GetData() || !candidate->GetData()->possession.allowed)
			continue;

		const glm::vec3 point = candidate->GetTargetPoint();
		const glm::vec3 offset = point - origin;
		const float distance = glm::length(offset);
		if (distance > config->possession.range || distance < 1e-3f)
			continue;

		const float cosAngle = glm::dot(offset / distance, forward);
		if (cosAngle < bestCos)
			continue;

		if (Physics::LineTrace(origin, point, BodyType::World, {}, { candidate }).hasHit)
			continue;

		bestCos = cosAngle;
		best = candidate;
	}

	return best;
}

CombatCharacter* PlayerController::GetDebugFocus()
{
	CombatCharacter* body = GetPossessedBody();
	if (body)
		if (auto* target = dynamic_cast<CombatCharacter*>(body->GetSoftTarget()))
			return target;
	return body;
}

// ---- Frame ------------------------------------------------------------------------------------------------

void PlayerController::UpdateDeathAndRelease()
{
	CombatCharacter* heroBody = GetHero();
	CombatCharacter* body = GetPossessedBody();

	// A possessed body that dies hands control back to the hero.
	if (body && body != heroBody && body->IsDead() && heroBody && !heroBody->IsDead())
		Release(PossessionReleaseMode::Revert);

	body = GetPossessedBody();
	const bool controlledDead = body == nullptr || body->IsDead();
	const bool heroGone = heroBody == nullptr || heroBody->IsDead();

	if (controlledDead && heroGone)
	{
		if (!playerDied)
		{
			playerDied = true;
			respawnDelay.AddDelay(config ? config->respawnDelay : 3.0f);
		}

		if (!restartRequested && !respawnDelay.Wait() && Level::Current)
		{
			restartRequested = true;
			Level::LoadLevelFromFile(Level::Current->filePath);
		}
	}
}

void PlayerController::Update()
{
	UpdateDeathAndRelease();

	if (InputAction* possess = Input::GetAction("possess"); possess && possess->Pressed() && config)
	{
		if (GetPossessedBody() != GetHero())
			Release(config->possession.releaseMode);
		else
			Possess(FindPossessionCandidate());
	}

	if (Input::LockCursor)
		camera.AddLookInput(Input::MouseDelta);
}

void PlayerController::LateUpdate()
{
	CombatCharacter* body = GetPossessedBody();
	if (body)
		camera.Update(*body, Time::DeltaTimeFNoTimeScale);

	if (!hud)
		return;

	CombatHud::State state;
	state.body = body;
	state.possessingOther = body != nullptr && body != GetHero();

	if (body && !body->IsDead())
	{
		// The target of the running attack, else the one the next attack would pick, else what the camera looks at.
		Entity* target = body->GetMoves().IsPrimaryActive() ? body->GetSoftTarget() : nullptr;
		if (target == nullptr)
			target = body->PredictAttackTarget();
		if (target == nullptr)
			target = body->FindSoftTarget(camera.GetPlanarForward(), 15.0f, 25.0f);
		state.target = target;

		const MoveSetData* moveSet = body->GetMoves().GetMoveSet();
		state.finisherAvailable = moveSet && moveSet->GetActionMove(ActionSlot::Finisher) >= 0 && body->FindFinisherTarget() != nullptr;
		state.possessionCandidate = !state.possessingOther && FindPossessionCandidate() != nullptr;
	}

	if (playerDied)
		state.message = "YOU DIED";

	hud->Refresh(state);
}

void PlayerController::UpdateDebugUI()
{
#ifndef DISTRIBUTION
	DrawCombatDebugPanel(*this);
#endif
}

// ---- Input ------------------------------------------------------------------------------------------------

void PlayerController::UpdateInput(CombatCharacter& body, CharacterInput& input, float deltaTime)
{
	(void)body;
	(void)deltaTime;
	const double now = Time::GameTime;

	glm::vec2 axis = Input::GetLeftStickPosition();
	if (InputAction* a = Input::GetAction("forward"); a && a->Holding()) axis.y += 1.0f;
	if (InputAction* a = Input::GetAction("backward"); a && a->Holding()) axis.y -= 1.0f;
	if (InputAction* a = Input::GetAction("left"); a && a->Holding()) axis.x -= 1.0f;
	if (InputAction* a = Input::GetAction("right"); a && a->Holding()) axis.x += 1.0f;
	if (glm::length(axis) > 1.0f)
		axis = glm::normalize(axis);

	input.move = camera.GetPlanarRight() * axis.x + camera.GetPlanarForward() * axis.y;
	input.aimOrigin = Camera::position;
	input.aimDirection = camera.GetForward();
	input.focus.Clear();

	MapButton(input, InputButton::Attack, "attack", now);
	MapButton(input, InputButton::Secondary, "attack2", now);
	MapButton(input, InputButton::Dash, "dash", now);
	MapButton(input, InputButton::Jump, "jump", now);
	MapButton(input, InputButton::Finisher, "finisher", now);
	MapButton(input, InputButton::Interact, "interact", now);
}

REGISTER_ENTITY(PlayerController, "player_controller")
