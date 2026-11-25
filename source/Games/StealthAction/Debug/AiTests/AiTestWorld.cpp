#include "AiTest.h"

#ifndef DISTRIBUTION

#include <Level.hpp>
#include <LevelObjectFactory.h>
#include <MapData.h>
#include <Physics.h>
#include <Input.h>
#include <Navigation/NavGrid.h>
#include <Navigation/Navigation.hpp>

#include "../../Entities/Npc/NpcBase.h"
#include "../../Entities/Npc/Brain/NpcSounds.h"
#include "../../Entities/Player/Player.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace AiTest
{
	namespace
	{
		// Characters stand with their centre this far above the floor (the guard and the player both).
		constexpr float CenterHeight = 1.4f;
	}

	// ── Puppet ───────────────────────────────────────────────────────────────────────────────────────────────────

	namespace
	{
		constexpr float ArriveDistance = 0.6f;     // horizontal metres from the destination that count as arrived
		constexpr float WaypointDistance = 0.7f;   // ... from a corner of the path before turning to the next
		constexpr float StuckSpeed = 0.3f;         // m/s below which the puppet is not getting anywhere
		constexpr float StuckSeconds = 2.0f;
		constexpr float ViolenceSeconds = 6.0f;    // a shot keeps him a violent criminal this long

		const char* WeaponClass(Loadout loadout)
		{
			switch (loadout)
			{
			case Loadout::Gun: return "weapon_mpsd";
			case Loadout::Blade: return "weapon_sword";
			default: return nullptr;
			}
		}
	}

	bool Puppet::Bind()
	{
		player = Player::Instance;
		state = State::Idle;
		parked = false;
		posture = Posture::Still;
		path.clear();
		problem.clear();

		if (player == nullptr) return false;

		player->cameraRotation.x = 0.0f;
		SetLoadout(Loadout::Gun);
		return true;
	}

	glm::vec3 Puppet::Position() const
	{
		return player ? player->Position : glm::vec3(0.0f);
	}

	std::string Puppet::Id() const
	{
		return player ? player->Id : std::string();
	}

	// The weapon appears with the player's next update: selecting it is what the weapon keys do.
	void Puppet::SetLoadout(Loadout loadout)
	{
		if (player == nullptr) return;

		const char* className = WeaponClass(loadout);
		if (className == nullptr)
		{
			player->currentEntry = WeaponGridEntry();
			player->DestroyWeapon();
			return;
		}

		auto find = [&]() -> const WeaponGridEntry*
		{
			for (const WeaponGridEntry& entry : player->weaponGrid)
			{
				if (entry.className == className && entry.pair == false) return &entry;
			}
			return nullptr;
		};

		if (find() == nullptr)
			player->AddWeaponByName(className);

		if (const WeaponGridEntry* entry = find())
			player->currentEntry = *entry;
	}

	void Puppet::Release()
	{
		Input::GetAction("forward")->SimulateRelease();
		Input::GetAction("dash")->SimulateRelease();
	}

	void Puppet::TeleportTo(const glm::vec3& position)
	{
		if (player == nullptr) return;
		player->Teleport(position);
		player->controller.SetVelocity(glm::vec3(0.0f));
		Release();
		state = State::Idle;
		parked = false;
	}

	void Puppet::Park(const glm::vec3& position)
	{
		if (player == nullptr) return;
		parkPosition = position;
		parked = true;
		state = State::Idle;
		posture = Posture::Still;
		Release();
		player->Teleport(position);
		player->controller.SetVelocity(glm::vec3(0.0f));
	}

	void Puppet::MoveTo(const glm::vec3& position, Posture newPosture)
	{
		if (player == nullptr) return;

		destination = position;
		posture = newPosture;
		parked = false;
		stillSeconds = 0.0f;
		waypoint = 0;
		problem.clear();

		bool reached = false;
		path = NavigationSystem::FindSimplePath(player->Position, destination, 0.0f, &reached, false);

		if (path.empty() || reached == false)
		{
			char text[160];
			snprintf(text, sizeof(text), "no navigation path from %.1f %.1f %.1f to %.1f %.1f %.1f", player->Position.x, player->Position.y,
				player->Position.z, destination.x, destination.y, destination.z);
			problem = text;
			state = State::Failed;
			Release();
			return;
		}

		state = State::Moving;
	}

	void Puppet::Hold(Posture newPosture)
	{
		posture = newPosture;
		if (newPosture != Posture::Still) parked = false;
		state = State::Idle;
		Release();
	}

	std::string Puppet::TakeProblem()
	{
		std::string text;
		text.swap(problem);
		return text;
	}

	void Puppet::Shoot()
	{
		if (player == nullptr) return;

		player->violanceCrimeActiveDelay.AddDelay(ViolenceSeconds);
		Emit(NpcSoundType::Gunshot);
	}

	void Puppet::Emit(NpcSoundType type, float loudness)
	{
		if (player == nullptr) return;
		NpcSounds::Emit(type, player->Position, player->Id, loudness, false);
	}

	// The crouch the player's key toggles, and the movement keys of a walking or running player.
	void Puppet::ApplyPosture(bool forward)
	{
		const bool crouch = posture == Posture::CrouchWalk || posture == Posture::CrouchStill;
		if (player->controller.isCrouched != crouch)
		{
			if (crouch) player->controller.Crouch();
			else player->controller.UnCrouch();
		}

		if (forward) Input::GetAction("forward")->SimulateHolding();
		else Input::GetAction("forward")->SimulateRelease();

		if (forward && posture == Posture::Run) Input::GetAction("dash")->SimulateHolding();
		else Input::GetAction("dash")->SimulateRelease();
	}

	void Puppet::Update(float dt)
	{
		if (player == nullptr) return;

		// The puppet cannot die: the tests are about what the NPCs do, not about the player's health.
		player->Health = 100.0f;

		if (parked)
		{
			player->Teleport(parkPosition);
			return;
		}

		if (state != State::Moving)
		{
			ApplyPosture(false);
			return;
		}

		const glm::vec3 here = player->Position;

		auto horizontal = [](const glm::vec3& from, const glm::vec3& to) { return glm::length(glm::vec2(to.x - from.x, to.z - from.z)); };

		if (horizontal(here, destination) <= ArriveDistance)
		{
			state = State::Arrived;
			ApplyPosture(false);
			return;
		}

		// Round the corners of the path: the next one once this one is close.
		while (waypoint + 1 < path.size() && horizontal(here, path[waypoint]) <= WaypointDistance)
			++waypoint;

		const glm::vec3 toward = path[waypoint] - here;
		if (horizontal(here, path[waypoint]) > 0.05f)
			player->cameraRotation.y = World::YawDegrees(glm::vec3(toward.x, 0.0f, toward.z));

		ApplyPosture(true);

		// Walking into something shows as not getting anywhere.
		const float speed = glm::length(MathHelper::XZ(player->controller.GetVelocity()));
		stillSeconds = speed < StuckSpeed ? stillSeconds + dt : 0.0f;

		if (stillSeconds >= StuckSeconds)
		{
			char text[160];
			snprintf(text, sizeof(text), "stuck for %.0f s at %.1f %.1f %.1f, %.1f m short of %.1f %.1f %.1f", StuckSeconds, here.x, here.y, here.z,
				horizontal(here, destination), destination.x, destination.y, destination.z);
			problem = text;
			state = State::Failed;
			ApplyPosture(false);
		}
	}

	// ── World ────────────────────────────────────────────────────────────────────────────────────────────────────

	float World::YawDegrees(const glm::vec3& direction)
	{
		return glm::degrees(std::atan2(direction.x, direction.z));
	}

	std::vector<NpcBase*> World::Npcs() const
	{
		std::vector<NpcBase*> result;
		if (Level::Current == nullptr) return result;

		for (LevelObject* object : Level::Current->LevelObjects)
		{
			if (NpcBase* npc = dynamic_cast<NpcBase*>(object))
				result.push_back(npc);
		}
		return result;
	}

	NpcBase* World::FindNpc(const std::string& name) const
	{
		for (NpcBase* npc : Npcs())
		{
			if (npc->Name == name) return npc;
		}
		return nullptr;
	}

	int World::RemoveNpcs()
	{
		const std::vector<NpcBase*> npcs = Npcs();

		for (NpcBase* npc : npcs)
			npc->Destroy();

		if (Level::Current)
		{
			Level::Current->RemovePendingEntities();
			Level::Current->MemoryCleanPendingEntities();
		}

		return static_cast<int>(npcs.size());
	}

	Entity* World::SpawnEntity(const std::string& className, const std::string& name, const glm::vec3& position, float yawDegrees,
		const std::map<std::string, std::string>& properties)
	{
		if (Level::Current == nullptr) return nullptr;

		Entity* entity = LevelObjectFactory::instance().create(className);
		if (entity == nullptr) return nullptr;

		// The keyvalues a map would give it.
		EntityData data;
		data.Classname = className;
		data.Properties = properties;
		entity->FromData(data);

		entity->Name = name;

		// The same order the level uses for the player: added first (it gets its id), then placed, loaded and started.
		Level::Current->AddEntity(entity);
		entity->Position = position;
		entity->Rotation = glm::vec3(0.0f, yawDegrees, 0.0f);
		entity->LoadAssetsIfNeeded();
		entity->Start();

		return entity;
	}

	NpcBase* World::SpawnNpc(const std::string& className, const std::string& name, const glm::vec3& position, float yawDegrees,
		const std::map<std::string, std::string>& properties)
	{
		return dynamic_cast<NpcBase*>(SpawnEntity(className, name, position, yawDegrees, properties));
	}

	bool World::SnapToFloor(const glm::vec3& position, glm::vec3& out) const
	{
		if (NavGrid::IsReady() == false) return false;

		const int cell = NavGrid::FindCell(position, 3.0f);
		if (cell < 0) return false;

		out = NavGrid::GetCell(cell).floor;
		return true;
	}

	bool World::HasLineOfSight(const glm::vec3& a, const glm::vec3& b) const
	{
		return Physics::LineTrace(a, b, BodyType::World).hasHit == false;
	}

	bool World::FindClearSpot(const glm::vec3& origin, float distance, float yawDegrees, float angleTolerance,
		glm::vec3& out, float tolerance) const
	{
		if (NavGrid::IsReady() == false) return false;

		std::vector<int> cells;
		NavGrid::CellsInRadius(origin, distance + tolerance, cells, 3.0f);

		float bestScore = 1.0e9f;
		bool found = false;

		const glm::vec3 eye = origin + glm::vec3(0.0f, 0.6f, 0.0f);

		for (int index : cells)
		{
			const glm::vec3 floor = NavGrid::GetCell(index).floor;
			glm::vec3 offset = floor - origin;
			offset.y = 0.0f;

			const float horizontal = glm::length(offset);
			if (horizontal < 0.5f || std::abs(horizontal - distance) > tolerance) continue;

			const float angle = glm::degrees(std::atan2(offset.x, offset.z));
			float difference = std::fmod(std::abs(angle - yawDegrees), 360.0f);
			if (difference > 180.0f) difference = 360.0f - difference;
			if (difference > angleTolerance) continue;

			if (HasLineOfSight(eye, floor + glm::vec3(0.0f, CenterHeight, 0.0f)) == false) continue;

			const float score = std::abs(horizontal - distance) + difference * 0.05f;
			if (score < bestScore)
			{
				bestScore = score;
				out = floor + glm::vec3(0.0f, CenterHeight, 0.0f);
				found = true;
			}
		}

		return found;
	}

	bool World::FindSpotHiddenFromAll(const std::vector<glm::vec3>& eyes, const glm::vec3& around, float minDistance, float maxDistance, glm::vec3& out) const
	{
		if (NavGrid::IsReady() == false || eyes.empty()) return false;

		std::vector<int> cells;
		NavGrid::CellsInRadius(around, maxDistance, cells, 3.0f);

		float bestDistance = 1.0e9f;
		bool found = false;

		for (int index : cells)
		{
			const glm::vec3 floor = NavGrid::GetCell(index).floor;
			const float distance = glm::distance(glm::vec2(floor.x, floor.z), glm::vec2(around.x, around.z));
			if (distance > maxDistance) continue;

			const glm::vec3 centre = floor + glm::vec3(0.0f, CenterHeight, 0.0f);
			const glm::vec3 low = floor + glm::vec3(0.0f, 0.5f, 0.0f);

			bool hidden = true;
			for (const glm::vec3& eye : eyes)
			{
				if (glm::distance(glm::vec2(floor.x, floor.z), glm::vec2(eye.x, eye.z)) < minDistance
					|| HasLineOfSight(eye, centre) || HasLineOfSight(eye, low))
				{
					hidden = false;
					break;
				}
			}

			if (hidden && distance < bestDistance)
			{
				bestDistance = distance;
				out = centre;
				found = true;
			}
		}

		return found;
	}

	bool World::FindEdgeSpot(const glm::vec3& eye, float minDistance, float maxDistance, float step, glm::vec3& seen, glm::vec3& hidden) const
	{
		if (NavGrid::IsReady() == false) return false;

		std::vector<int> cells;
		NavGrid::CellsInRadius(eye, maxDistance + step, cells, 3.0f);

		float bestStep = 1.0e9f;
		bool found = false;

		for (int index : cells)
		{
			const glm::vec3 floor = NavGrid::GetCell(index).floor;
			const float distance = glm::distance(glm::vec2(floor.x, floor.z), glm::vec2(eye.x, eye.z));
			if (distance < minDistance || distance > maxDistance) continue;

			// In plain sight standing and crouched.
			if (HasLineOfSight(eye, floor + glm::vec3(0.0f, CenterHeight, 0.0f)) == false || HasLineOfSight(eye, floor + glm::vec3(0.0f, 0.5f, 0.0f)) == false) continue;

			// The nearest cell that is out of sight, a step away.
			for (int other : cells)
			{
				const glm::vec3 candidate = NavGrid::GetCell(other).floor;
				const float stepDistance = glm::distance(glm::vec2(floor.x, floor.z), glm::vec2(candidate.x, candidate.z));
				if (stepDistance > step || stepDistance >= bestStep) continue;

				if (HasLineOfSight(eye, candidate + glm::vec3(0.0f, CenterHeight, 0.0f)) || HasLineOfSight(eye, candidate + glm::vec3(0.0f, 0.5f, 0.0f))) continue;

				bestStep = stepDistance;
				seen = floor + glm::vec3(0.0f, CenterHeight, 0.0f);
				hidden = candidate + glm::vec3(0.0f, CenterHeight, 0.0f);
				found = true;
			}
		}

		return found;
	}

	bool World::FindHiddenSpot(const glm::vec3& from, float minDistance, float maxDistance, glm::vec3& out) const
	{
		if (NavGrid::IsReady() == false) return false;

		std::vector<int> cells;
		NavGrid::CellsInRadius(from, maxDistance, cells, 3.0f);

		float bestDistance = 1.0e9f;
		bool found = false;

		for (int index : cells)
		{
			const glm::vec3 floor = NavGrid::GetCell(index).floor;
			const float distance = glm::distance(glm::vec2(floor.x, floor.z), glm::vec2(from.x, from.z));
			if (distance < minDistance || distance > maxDistance) continue;

			const glm::vec3 centre = floor + glm::vec3(0.0f, CenterHeight, 0.0f);

			// Hidden from the standing eye and from the lowered head, and not just round a thin edge.
			if (HasLineOfSight(from, centre)) continue;
			if (HasLineOfSight(from, floor + glm::vec3(0.0f, 0.5f, 0.0f))) continue;

			if (distance < bestDistance)
			{
				bestDistance = distance;
				out = centre;
				found = true;
			}
		}

		return found;
	}
}

#endif
