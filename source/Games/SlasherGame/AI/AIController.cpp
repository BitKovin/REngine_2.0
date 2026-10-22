#include "AIController.h"

#include <Characters/CombatCharacter.h>
#include <Combat/CombatComponents.h>
#include <Combat/CombatFrame.h>
#include <Control/PlayerController.h>

#include <AiPerception/AiPerceptionSystem.h>
#include <AiPerception/Observer.h>
#include <EngineMain.h>
#include <Level.hpp>
#include <Logger.hpp>
#include <Physics.h>
#include <RandomHelper.h>
#include <Time.hpp>
#include <imgui/imgui.h>
#include <tracy/tracy/Tracy.hpp>

#include <algorithm>
#include <array>
#include <cmath>

// ---- Registry ---------------------------------------------------------------------------------------------

std::vector<AIBrainType>& AIBrainRegistry::Types()
{
	static std::vector<AIBrainType> types;
	return types;
}

int AIBrainRegistry::Register(AIBrainType type)
{
	auto& types = Types();
	types.push_back(std::move(type));
	return static_cast<int>(types.size()) - 1;
}

std::unique_ptr<AIBrain> AIBrainRegistry::Create(const std::string& name)
{
	for (const AIBrainType& type : Types())
		if (type.Name == name)
			return type.Create();
	return nullptr;
}

std::string AIBrainRegistry::KnownNames()
{
	std::string result;
	for (const AIBrainType& type : Types())
	{
		if (!result.empty()) result += ", ";
		result += type.Name;
	}
	return result;
}

// ---- AIController -----------------------------------------------------------------------------------------

namespace
{
	constexpr float TokenLease = 4.0f;
	constexpr unsigned long SightInterval = 4;   // frames between sight updates of a body (spread over the bodies)
	constexpr int MaxSightCandidates = 6;        // nearest hostiles traced per sight update (+ the current target)
}

AIController::AIController(CombatCharacter& owner)
	: body(owner)
{
	path.allowPartialPath = true;
	if (const CharacterData* data = body.GetData())
		attackCooldowns.assign(data->ai.attacks.size(), Delay());
}

AIController::~AIController()
{
	RemoveObserver();
}

void AIController::SetBrain(const std::string& name)
{
	if (name == brainName && brain)
		return;

	brain = AIBrainRegistry::Create(name);
	brainName = name;
	brainNeedsActivation = true;

	if (!brain)
		Logger::Error("[AIController] %s: unknown brain '%s' (known: %s)", body.GetDataPath().c_str(), name.c_str(), AIBrainRegistry::KnownNames().c_str());

	if (const CharacterData* data = body.GetData())
		attackCooldowns.assign(data->ai.attacks.size(), Delay());
}

void AIController::OnPossess(CombatCharacter& possessed)
{
	(void)possessed;
	brainNeedsActivation = true;
	EnsureObserver();
}

void AIController::OnUnpossess(CombatCharacter& possessed)
{
	(void)possessed;
	ReleaseToken();
	RemoveObserver();
	heldButtons.clear();
	moveGoalReached = true;
}

void AIController::OnBodyDied()
{
	ReleaseToken();
	RemoveObserver();
	heldButtons.clear();
}

void AIController::OnBodyDestroyed()
{
	OnBodyDied();
}

void AIController::EnsureObserver()
{
	if (observer || !body.GetData())
		return;

	const AiData& ai = body.GetData()->ai;
	observer = AiPerceptionSystem::CreateObserver(EyePosition(), body.GetForward(), ai.fov);
	observer->maxDistance = 0.0f;   // hearing only: sight is UpdateSight's
	observer->ownerPtr = &body;
	observer->owner = hashed_string(body.Id);
}

void AIController::RemoveObserver()
{
	if (!observer)
		return;
	AiPerceptionSystem::RemoveObserver(observer);
	observer.reset();
}

glm::vec3 AIController::EyePosition() const
{
	return body.Position + glm::vec3(0.0f, body.GetCapsuleHeight() * 0.4f, 0.0f);
}

bool AIController::IsHostile(Entity* entity) const
{
	if (entity == nullptr || entity == &body || entity->Destroyed)
		return false;

	auto state = entity->GetComponent<CombatStateComponent>();
	if (state && state->dead)
		return false;

	auto team = entity->GetComponent<TeamComponent>();
	return team && GetAttitude(body.GetTeamId(), team->team) == Attitude::Hostile;
}

Entity* AIController::GetTarget()
{
	Entity* entity = ResolveAlive(target);
	if (entity && !IsHostile(entity))
	{
		target.Clear();   // died, or changed sides (possession / recruitment)
		return nullptr;
	}
	return entity;
}

float AIController::GetTargetDistance()
{
	Entity* entity = GetTarget();
	return entity ? glm::length(MathHelper::XZ(entity->Position - body.Position)) : 1e9f;
}

void AIController::SetTarget(Entity* newTarget)
{
	if (newTarget == nullptr)
	{
		target.Clear();
		return;
	}

	const EntityHandle handle = EntityHandle::FromEntity(newTarget);
	if (handle == target)
		return;

	ReleaseToken();
	target = handle;
	lastKnownTargetPosition = newTarget->Position;
	if (const CharacterData* data = body.GetData())
	{
		loseTargetDelay.AddDelay(data->ai.loseTargetTime);
		reactionDelay.AddDelay(data->ai.reactionTime);
	}
	hasInvestigatePoint = false;
}

CombatCharacter* AIController::GetLeader() const
{
	PlayerController* player = PlayerController::GetActive();
	CombatCharacter* leader = player ? player->GetPossessedBody() : nullptr;
	if (leader == nullptr || leader == &body || leader->IsDead() || leader->GetTeamId() != body.GetTeamId())
		return nullptr;
	return leader;
}

Entity* AIController::FindVisibleHostileNear(const glm::vec3& around, float maxDistance)
{
	Entity* best = nullptr;
	float bestDistance = maxDistance;

	for (EntityHandle& visible : visibleHostiles)
	{
		Entity* entity = ResolveAlive(visible);
		if (!IsHostile(entity))
			continue;

		const float distance = glm::length(entity->Position - around);
		if (distance < bestDistance)
		{
			bestDistance = distance;
			best = entity;
		}
	}
	return best;
}

void AIController::NotifyDamaged(Entity* attacker)
{
	// Being hit reveals the attacker even outside the sight cone.
	if (attacker == nullptr || !IsHostile(attacker))
		return;

	if (GetTarget() == nullptr || !targetVisible)
	{
		SetTarget(attacker);
		reactionDelay = Delay();   // no surprise delay when hit
	}
}

void AIController::UpdatePerception(double now)
{
	// The observer is created when this controller takes the body (OnPossess, main thread): the perception system's
	// lists must not change while bodies update in parallel.
	if (!observer)
		return;

	observer->position = EyePosition();
	observer->forward = body.GetForward();
	UpdateSight();

	Entity* current = GetTarget();
	targetVisible = false;
	if (current)
	{
		const EntityHandle handle = EntityHandle::FromEntity(current);
		targetVisible = std::find(visibleHostiles.begin(), visibleHostiles.end(), handle) != visibleHostiles.end();
	}

	const AiData& ai = body.GetData()->ai;
	Entity* best = FindVisibleHostileNear(body.Position, ai.sightRadius);

	if (current == nullptr && best != nullptr)
		SetTarget(best);
	else if (current != nullptr && !targetVisible && best != nullptr && best != current)
		SetTarget(best);

	current = GetTarget();
	if (current)
	{
		if (current == best)
			targetVisible = true;

		if (targetVisible)
		{
			loseTargetDelay.AddDelay(ai.loseTargetTime);
			lastKnownTargetPosition = current->Position;
		}
		else if (!loseTargetDelay.Wait())
		{
			investigatePoint = lastKnownTargetPosition;
			hasInvestigatePoint = true;
			target.Clear();
			ReleaseToken();
		}
	}
	else
	{
		for (const HeardSound& sound : observer->heardSounds)
		{
			investigatePoint = sound.position;
			hasInvestigatePoint = true;
		}
	}
}

// Sight (COMBAT_DESIGN.md §8): only hostile bodies are looked at - this frame's bodies (CombatFrame::GetBodies) inside
// sightRadius and the view cone, the nearest MaxSightCandidates plus the current target, each through the BSP
// visibility and one line-of-sight trace against the opaque world. Every SightInterval frames per body, spread over
// the bodies; the result holds in between. (The engine observer traced every body in the level against every
// other: at 300 bodies 37 ms of CPU per frame.)
void AIController::UpdateSight()
{
	const unsigned long frame = EngineMain::MainInstance->frame;
	if (sightValid && (frame + entt::to_integral(body.Handle())) % SightInterval != 0)
		return;
	sightValid = true;
	visibleHostiles.clear();

	const AiData& ai = body.GetData()->ai;
	const glm::vec3 eye = EyePosition();
	const glm::vec3 forward = body.GetForward();
	const float rangeSquared = ai.sightRadius * ai.sightRadius;
	const float minCos = std::cos(glm::radians(ai.fov * 0.5f));
	const Team team = body.GetTeamId();
	const Entity* current = ResolveAlive(target);

	struct Candidate
	{
		CombatCharacter* body = nullptr;
		float distanceSquared = 0.0f;
	};
	std::array<Candidate, MaxSightCandidates> nearest;
	int count = 0;
	Candidate currentCandidate;

	for (CombatCharacter* other : CombatFrame::GetBodies())
	{
		if (other == &body || other->IsDead() || GetAttitude(team, other->GetTeamId()) != Attitude::Hostile)
			continue;

		const glm::vec3 offset = other->GetTargetPoint() - eye;
		const float distanceSquared = glm::dot(offset, offset);
		if (distanceSquared > rangeSquared || distanceSquared < 1e-6f || glm::dot(offset, forward) < minCos * std::sqrt(distanceSquared))
			continue;

		if (other == current)
		{
			currentCandidate = { other, distanceSquared };
			continue;
		}
		if (count == MaxSightCandidates && distanceSquared >= nearest[count - 1].distanceSquared)
			continue;

		int slot = count < MaxSightCandidates ? count++ : count - 1;
		while (slot > 0 && nearest[slot - 1].distanceSquared > distanceSquared)
		{
			nearest[slot] = nearest[slot - 1];
			--slot;
		}
		nearest[slot] = { other, distanceSquared };
	}

	CQuake3BSP& bsp = Level::Current->BspData;
	const int eyeCluster = bsp.FindClusterAtPosition(eye);
	auto sees = [&](const CombatCharacter& other)
	{
		const glm::vec3 point = other.GetTargetPoint();
		return bsp.IsClusterVisible(eyeCluster, bsp.FindClusterAtPosition(point)) && !Physics::LineTrace(eye, point, BodyType::WorldOpaque).hasHit;
	};

	// Nearest first, the current target in its place by distance.
	bool currentPlaced = currentCandidate.body == nullptr;
	for (int i = 0; i <= count; ++i)
	{
		if (!currentPlaced && (i == count || currentCandidate.distanceSquared <= nearest[i].distanceSquared))
		{
			currentPlaced = true;
			if (sees(*currentCandidate.body))
				visibleHostiles.push_back(EntityHandle::FromEntity(currentCandidate.body));
		}
		if (i < count && sees(*nearest[i].body))
			visibleHostiles.push_back(EntityHandle::FromEntity(nearest[i].body));
	}
}

void AIController::UpdateHeldButtons(double now)
{
	for (size_t i = 0; i < heldButtons.size();)
	{
		if (!heldButtons[i].release.Wait())
		{
			input->SetButton(heldButtons[i].button, false, now);
			heldButtons.erase(heldButtons.begin() + static_cast<std::ptrdiff_t>(i));
			continue;
		}
		input->SetButton(heldButtons[i].button, true, now);
		++i;
	}
}

void AIController::UpdateInput(CombatCharacter& self, CharacterInput& characterInput, float deltaTime)
{
	(void)self;
	if (!body.GetData())
		return;

	const double now = Time::GameTime;
	input = &characterInput;

	{
		ZoneScopedN("AI: perception");
		UpdatePerception(now);
	}
	UpdateHeldButtons(now);

	input->move = glm::vec3(0.0f);
	input->aimOrigin = EyePosition();
	Entity* current = GetTarget();
	input->focus = current ? EntityHandle::FromEntity(current) : EntityHandle();
	if (auto* character = dynamic_cast<CombatCharacter*>(current))
		AimAt(character->GetTargetPoint(), character->Position);
	else if (current)
		LookAt(current->Position);
	else
		input->aimDirection = body.GetForward();

	if (heldToken != AttackToken::None && !IsBodyAttacking() && heldButtons.empty())
		ReleaseToken();

	if (brain)
	{
		AIContext context{ body, *this, *input, deltaTime, now };
		if (brainNeedsActivation)
		{
			brain->OnActivated(context);
			brainNeedsActivation = false;
		}
		ZoneScopedN("AI: think");
		brain->Think(context);
	}

	input = nullptr;
}

// ---- Movement ---------------------------------------------------------------------------------------------

void AIController::MoveTo(const glm::vec3& point, float speedScale, float acceptanceRadius)
{
	if (input == nullptr)
		return;

	const glm::vec3 offset = MathHelper::XZ(point - body.Position);
	const float distance = glm::length(offset);
	if (distance <= acceptanceRadius)
	{
		moveGoalReached = true;
		input->move = glm::vec3(0.0f);
		return;
	}
	moveGoalReached = false;

	glm::vec3 direction = offset / std::max(distance, 1e-4f);

	// Close and unobstructed: steer directly. Otherwise follow the navmesh path.
	const bool direct = distance < 4.0f
		&& !Physics::LineTrace(EyePosition(), point + glm::vec3(0.0f, 0.5f, 0.0f), BodyType::World, {}, { &body }).hasHit;

	if (!direct)
	{
		ZoneScopedN("AI: path");
		path.acceptanceRadius = acceptanceRadius;
		path.UpdateStartAndTarget(body.Position, point);
		path.TryPerform();

		if (path.FoundTarget)
		{
			const glm::vec3 next = MathHelper::XZ(path.CalculatedTargetLocation - body.Position);
			if (glm::dot(next, next) > 1e-4f)
				direction = glm::normalize(next);
		}
	}

	input->move = direction * std::clamp(speedScale, 0.0f, 1.0f);
}

void AIController::MoveDirection(const glm::vec3& direction, float speedScale)
{
	if (input == nullptr)
		return;
	glm::vec3 flat = MathHelper::XZ(direction);
	input->move = glm::dot(flat, flat) > 1e-6f ? glm::normalize(flat) * std::clamp(speedScale, 0.0f, 1.0f) : glm::vec3(0.0f);
}

void AIController::Circle(const glm::vec3& center, float preferredDistance, float speedScale)
{
	if (input == nullptr)
		return;

	glm::vec3 offset = MathHelper::XZ(body.Position - center);
	float distance = glm::length(offset);
	if (distance < 1e-3f)
	{
		offset = -body.GetForward();
		distance = 1.0f;
	}

	// Circling direction flips at random intervals so groups don't orbit in lockstep.
	if (!strafeFlipDelay.Wait())
	{
		strafeSide = RandomHelper::RandomFloat() < 0.5f ? -1.0f : 1.0f;
		strafeFlipDelay.AddDelay(2.0 + RandomHelper::RandomFloat() * 2.0);
	}

	const glm::vec3 radial = offset / distance;
	const glm::vec3 tangent = glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), radial);
	const float correction = std::clamp((preferredDistance - distance) * 0.8f, -1.0f, 1.0f);

	MoveDirection(tangent * strafeSide + radial * correction, speedScale);
	LookAt(center);
}

void AIController::Stop()
{
	if (input)
		input->move = glm::vec3(0.0f);
	moveGoalReached = true;
}

void AIController::LookAt(const glm::vec3& point)
{
	AimAt(point + glm::vec3(0.0f, 0.4f, 0.0f), point);
}

void AIController::AimAt(const glm::vec3& aimPoint, const glm::vec3& facePoint)
{
	if (input == nullptr)
		return;

	glm::vec3 direction = aimPoint - EyePosition();
	if (glm::dot(direction, direction) < 1e-6f)
		return;
	direction = glm::normalize(direction);

	// Firearm aim error.
	const float error = body.GetData()->ai.aimError;
	if (error > 0.0f)
	{
		const float yawError = (RandomHelper::RandomFloat() * 2.0f - 1.0f) * error;
		const float pitchError = (RandomHelper::RandomFloat() * 2.0f - 1.0f) * error * 0.5f;
		direction = MathHelper::GetRotationQuaternion(glm::vec3(pitchError, yawError, 0.0f)) * direction;
	}

	input->aimDirection = direction;
	body.RequestFacing(MathHelper::XZ(facePoint - body.Position), body.GetData()->movement.turnRate);
}

// ---- Buttons ----------------------------------------------------------------------------------------------

void AIController::Tap(InputButton button)
{
	if (input)
		input->Tap(button, Time::GameTime);
}

void AIController::HoldFor(InputButton button, float seconds)
{
	if (input == nullptr)
		return;

	input->SetButton(button, true, Time::GameTime);
	for (HeldButton& held : heldButtons)
	{
		if (held.button == button)
		{
			if (held.release.GetRemainTime() < seconds)
				held.release.AddDelay(seconds);
			return;
		}
	}
	heldButtons.push_back({ button, Delay(seconds) });
}

// ---- Attacks ----------------------------------------------------------------------------------------------

bool AIController::TryAcquireToken(AttackToken kind)
{
	if (kind == AttackToken::None)
		return true;

	Entity* current = GetTarget();
	if (current == nullptr)
		return false;

	auto tokens = current->GetComponent<AttackTokensComponent>();
	if (!tokens)
		return true;

	if (!tokens->TryAcquire(EntityHandle::FromEntity(&body), kind, Time::GameTime, TokenLease))
		return false;

	tokenOwner = EntityHandle::FromEntity(current);
	heldToken = kind;
	return true;
}

void AIController::ReleaseToken()
{
	if (heldToken == AttackToken::None)
		return;

	if (Entity* owner = ResolveAlive(tokenOwner))
		if (auto tokens = owner->GetComponent<AttackTokensComponent>())
			tokens->Release(EntityHandle::FromEntity(&body));

	heldToken = AttackToken::None;
	tokenOwner.Clear();
}

bool AIController::IsBodyAttacking() const
{
	const MoveSlot& primary = body.GetMoves().GetPrimary();
	return primary.IsActive() && primary.move->HasTag(MoveTag::Attack);
}

bool AIController::HasRangedAttacks() const
{
	const CharacterData* data = body.GetData();
	if (!data) return false;
	return std::any_of(data->ai.attacks.begin(), data->ai.attacks.end(), [](const AiAttackData& a) { return a.token == AttackToken::Ranged; });
}

float AIController::GetMaxAttackRange() const
{
	const CharacterData* data = body.GetData();
	float range = 0.0f;
	if (data)
		for (const AiAttackData& attack : data->ai.attacks)
			range = std::max(range, attack.maxRange);
	return range;
}

bool AIController::TryStartAttack()
{
	const CharacterData* data = body.GetData();
	Entity* current = GetTarget();
	if (data == nullptr || current == nullptr || input == nullptr)
		return false;

	const float distance = GetTargetDistance();
	const auto& attacks = data->ai.attacks;
	if (attackCooldowns.size() != attacks.size())
		attackCooldowns.assign(attacks.size(), Delay());

	// The move of an attack: its own, or what the equipped weapons put in its slot.
	const MoveTables& tables = body.GetMoves().GetTables();
	auto moveOf = [&tables](const AiAttackData& attack)
	{
		if (attack.comboSlot >= 0)
			return tables.GetComboOpener(static_cast<ComboSlot>(attack.comboSlot));
		if (attack.actionSlot >= 0)
			return tables.GetActionMove(static_cast<ActionSlot>(attack.actionSlot));
		return attack.moveIndex;
	};

	// Valid candidates, then weighted picks without replacement until one gets its token.
	std::vector<size_t> candidates;
	float totalWeight = 0.0f;
	for (size_t i = 0; i < attacks.size(); ++i)
	{
		const AiAttackData& attack = attacks[i];
		if (distance < attack.minRange || distance > attack.maxRange) continue;
		if (attackCooldowns[i].Wait()) continue;
		if (attack.requireLineOfSight && !targetVisible) continue;
		if (!body.GetMoves().CanStartMove(moveOf(attack))) continue;
		candidates.push_back(i);
		totalWeight += attack.weight;
	}

	while (!candidates.empty() && totalWeight > 0.0f)
	{
		float roll = RandomHelper::RandomFloat() * totalWeight;
		size_t pick = 0;
		for (; pick + 1 < candidates.size(); ++pick)
		{
			roll -= attacks[candidates[pick]].weight;
			if (roll <= 0.0f) break;
		}

		const size_t index = candidates[pick];
		const AiAttackData& attack = attacks[index];
		candidates.erase(candidates.begin() + static_cast<std::ptrdiff_t>(pick));
		totalWeight -= attack.weight;

		if (!TryAcquireToken(attack.token))
			continue;

		input->focus = EntityHandle::FromEntity(current);
		if (!body.GetMoves().TryStartMove(moveOf(attack)))
			continue;

		if (attack.cooldown > 0.0f)
			attackCooldowns[index].AddDelay(attack.cooldown);
		if (attack.holdTime > 0.0f)
			HoldFor(attack.holdButton, attack.holdTime);
		return true;
	}

	return false;
}

void AIController::DrawDebugUI()
{
#ifndef DISTRIBUTION
	Entity* current = ResolveAlive(target);
	ImGui::Text("AI brain: %s  %s", brainName.c_str(), brain ? brain->DescribeState() : "(none)");
	ImGui::Text("Target: %s  visible %d  dist %.1f  token %d  goal %s", current ? current->Id.c_str() : "-",
		targetVisible ? 1 : 0, current ? GetTargetDistance() : 0.0f, static_cast<int>(heldToken), moveGoalReached ? "reached" : "moving");
#endif
}
