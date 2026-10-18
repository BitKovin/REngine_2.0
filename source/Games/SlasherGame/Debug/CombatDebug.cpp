#include "CombatDebug.h"

#ifndef DISTRIBUTION

#include "MoveSetEditor.h"
#include "SelfTests.h"

#include <AI/AIController.h>
#include <Characters/CombatCharacter.h>
#include <Control/PlayerController.h>
#include <Data/CombatData.h>

#include <Camera.h>
#include <Console/Console.h>
#include <Console/ConsoleRegister.h>
#include <Logger.hpp>
#include <imgui/imgui.h>

#include <algorithm>
#include <memory>
#include <string>

namespace
{
	struct PanelState
	{
		EntityHandle selected;
		MoveSetEditor editor;                   // edits a copy of the selected body's moveset
		std::string status;
	};

	PanelState& State()
	{
		static PanelState state;
		return state;
	}

	CombatCharacter* Resolve(EntityHandle& handle)
	{
		return dynamic_cast<CombatCharacter*>(ResolveAlive(handle));
	}

	// Runtime spawn of a combat body type (debug commands: LoadAssets runs in Start, the first body of a type may hitch).
	CombatCharacter* SpawnCombatant(const std::string& type, const glm::vec3& position, float yaw)
	{
		Entity* entity = LevelObjectFactory::instance().create(type);
		auto* body = dynamic_cast<CombatCharacter*>(entity);
		if (body == nullptr)
		{
			delete entity;
			Console::Get().AddLog("'%s' is not a combat body type", type.c_str());
			return nullptr;
		}
		body->Position = position;
		body->Rotation.y = yaw;
		Level::Current->AddEntity(body);
		body->Start();
		return body;
	}

	void DrawCombatantList(PanelState& state, PlayerController& player)
	{
		if (!ImGui::CollapsingHeader("Combatants"))
			return;

		auto view = EcsWorld::Registry().view<CombatStateComponent>();
		for (entt::entity id : view)
		{
			auto* body = dynamic_cast<CombatCharacter*>(EcsWorld::GetEntity(id));
			if (body == nullptr || body->Destroyed)
				continue;

			ImGui::PushID(body);
			const bool isSelected = EntityHandle::FromEntity(body) == state.selected;
			const std::string label = body->Id + "  " + (body->GetData() ? body->GetData()->name : std::string("?"))
				+ (body->IsDead() ? "  [dead]" : "") + (body == player.GetPossessedBody() ? "  [player]" : "");
			if (ImGui::Selectable(label.c_str(), isSelected))
			{
				state.selected = EntityHandle::FromEntity(body);
				state.editor.Reset();
			}
			ImGui::PopID();
		}
	}
}

void DrawCombatDebugPanel(PlayerController& player)
{
	PanelState& state = State();

	if (!ImGui::Begin("Combat"))
	{
		ImGui::End();
		return;
	}

	CombatCharacter* controlled = player.GetPossessedBody();

	if (ImGui::Button("Reload combat data"))
		state.status = CombatData::ReloadAll();
	ImGui::SameLine();
	if (controlled)
	{
		bool god = controlled->GetCombatState()->godMode;
		if (ImGui::Checkbox("God mode", &god))
			controlled->GetCombatState()->godMode = god;
	}
	ImGui::SameLine();
	if (controlled && controlled != player.GetHero())
	{
		if (ImGui::Button("Release"))
			player.Release(player.GetConfig() ? player.GetConfig()->possession.releaseMode : PossessionReleaseMode::Ally);
	}
	else if (ImGui::Button("Possess aimed"))
	{
		std::string reason;
		if (!player.Possess(player.FindPossessionCandidate(), &reason))
			state.status = "possess: " + reason;
	}
	ImGui::SameLine();
	if (ImGui::Button("Self tests"))
		RunCombatSelfTests(state.status);

	if (!state.status.empty())
		ImGui::TextWrapped("%s", state.status.c_str());

	DrawCombatantList(state, player);

	CombatCharacter* selected = Resolve(state.selected);
	if (selected == nullptr)
		selected = player.GetDebugFocus();

	if (selected)
	{
		ImGui::Separator();
		selected->DrawDebugUI();
		if (const MoveSetData* moveSet = selected->GetMoves().GetMoveSet())
			state.editor.Draw(*moveSet);
	}

	ImGui::End();
}

// ---- Console commands ---------------------------------------------------------------------------------

CONSOLE_FUNC("combat_reload", "Re-read every combat data file (movesets, characters, weapons, effects, player)")
{
	(void)args;
	Console::Get().AddLog("%s", CombatData::ReloadAll().c_str());
}

CONSOLE_FUNC("combat_selftest", "Run the combat self tests")
{
	(void)args;
	std::string summary;
	RunCombatSelfTests(summary);
	Console::Get().AddLog("%s", summary.c_str());
}

CONSOLE_FUNC("god", "Toggle god mode on the controlled body")
{
	(void)args;
	PlayerController* player = PlayerController::GetActive();
	CombatCharacter* body = player ? player->GetPossessedBody() : nullptr;
	if (body == nullptr)
		return;
	auto state = body->GetCombatState();
	state->godMode = !state->godMode;
	Console::Get().AddLog("god mode %s", state->godMode ? "on" : "off");
}

CONSOLE_FUNC("possess", "Possess the body under the crosshair")
{
	(void)args;
	PlayerController* player = PlayerController::GetActive();
	if (player == nullptr)
		return;
	std::string reason;
	if (!player->Possess(player->FindPossessionCandidate(), &reason))
		Console::Get().AddLog("possess failed: %s", reason.c_str());
}

CONSOLE_FUNC("release", "Release the possessed body: release [ally|revert]")
{
	PlayerController* player = PlayerController::GetActive();
	if (player == nullptr)
		return;
	const std::string mode = Console::ArgString(args, 0, "ally");
	player->Release(mode == "revert" ? PossessionReleaseMode::Revert : PossessionReleaseMode::Ally);
}

CONSOLE_FUNC("spawn_combatant", "Spawn a body type around the controlled body: spawn_combatant <entity type> [distance m] [angle deg, + = right of the view]")
{
	const std::string type = Console::ArgString(args, 0, "enemy_grunt");
	const float distance = Console::ArgFloat(args, 1, 4.0f);
	const float angle = Console::ArgFloat(args, 2, 0.0f);

	// Around the controlled body, measured from the view direction (+yaw turns left in the engine); the camera without one.
	PlayerController* player = PlayerController::GetActive();
	CombatCharacter* controlled = player ? player->GetPossessedBody() : nullptr;
	const float yaw = Camera::rotation.y - angle;
	const glm::vec3 direction = MathHelper::GetForwardVector(glm::vec3(0.0f, yaw, 0.0f));
	SpawnCombatant(type, (controlled ? controlled->Position : Camera::position) + direction * distance, yaw + 180.0f);
}

CONSOLE_FUNC("perf_crowd", "Profiling load: perf_crowd <count> [spacing m] [types, comma separated] [ally share 0..1] - rings of bodies around the controlled body, facing it; a share of them joins the player's team (Ally brain) so the crowd fights itself")
{
	const int count = std::max(Console::ArgInt(args, 0, 50), 0);
	const float spacing = std::max(Console::ArgFloat(args, 1, 1.8f), 0.8f);
	const float allyShare = std::clamp(Console::ArgFloat(args, 3, 0.0f), 0.0f, 1.0f);
	std::vector<std::string> types;
	const std::string typeList = Console::ArgString(args, 2, "enemy_grunt,enemy_grunt,enemy_gunner,enemy_brute");
	for (size_t start = 0; start <= typeList.size();)
	{
		const size_t end = std::min(typeList.find(',', start), typeList.size());
		if (end > start)
			types.push_back(typeList.substr(start, end - start));
		start = end + 1;
	}
	if (types.empty())
		return;

	PlayerController* player = PlayerController::GetActive();
	CombatCharacter* controlled = player ? player->GetPossessedBody() : nullptr;
	const glm::vec3 center = controlled ? controlled->Position : Camera::position;

	// Rings from 3 m out, `spacing` apart, each filled at `spacing` along its circumference.
	int spawned = 0;
	for (float radius = 3.0f; spawned < count; radius += spacing)
	{
		const int slots = std::max(1, static_cast<int>(glm::two_pi<float>() * radius / spacing));
		for (int i = 0; i < slots && spawned < count; ++i)
		{
			const float yaw = 360.0f * static_cast<float>(i) / static_cast<float>(slots);
			const glm::vec3 position = center + MathHelper::GetForwardVector(glm::vec3(0.0f, yaw, 0.0f)) * radius + glm::vec3(0.0f, 0.2f, 0.0f);
			CombatCharacter* body = SpawnCombatant(types[static_cast<size_t>(spawned) % types.size()], position, yaw + 180.0f);
			if (body == nullptr)
				return;

			// Recruited like a possessed body released as an ally (PlayerController::Release).
			const int allies = static_cast<int>(std::floor((spawned + 1) * allyShare)) - static_cast<int>(std::floor(spawned * allyShare));
			if (allies > 0 && body->GetAIController() != nullptr)
			{
				body->SetTeamId(Team::Player);
				body->GetAIController()->SetBrain(body->GetData()->possession.idleBrain);
			}
			++spawned;
		}
	}
	Console::Get().AddLog("perf_crowd: %d bodies", spawned);
	Logger::Log("perf_crowd: %d bodies", spawned);
}

#endif
