// Time.hpp first: engine headers below use Time without including it.
#include <Time.hpp>

#include "SimTransition.h"

#include "SimWorldNavFile.h"

#include <SimPawn.h>

#include <Level.hpp>
#include <LevelTraversalSystem.h>
#include <Logger.hpp>
#include <SaveSystem/LevelSaveSystem.h>

namespace SimTransition
{
	Target Resolve(const sim::nav::WorldNavData& world, const std::string& portal, const std::string& targetLevel, const std::string& targetSpawn)
	{
		Target target;
		if (targetLevel.empty() || targetSpawn.empty())
		{
			target.error = "portal " + portal + " does not name a targetLevel and a targetSpawn";
			return target;
		}
		const int level = world.FindLevel(targetLevel);
		if (level < 0)
		{
			target.error = "portal " + portal + " leads to " + targetLevel + ", which is not a level of the world file (is it in world.manifest, and was the world baked after?)";
			return target;
		}
		for (size_t i = 0; i < world.spawns.size(); ++i)
		{
			const sim::nav::WorldSpawn& spawn = world.spawns[i];
			if (spawn.level == level && spawn.name == targetSpawn)
			{
				target.ok = true;
				target.level = level;
				target.spawn = static_cast<int>(i);
				target.landing = spawn.position - world.levels[level].offset;
				return target;
			}
		}
		target.error = "portal " + portal + " leads to spawn " + targetSpawn + ", which level " + targetLevel + " does not have in the world file (sim_spawn targetname, then rebake)";
		return target;
	}

	namespace
	{
		struct PendingRequest
		{
			bool active = false;
			std::string portal, level, spawn;
			float pitch = 0.0f;
		};

		// The transition that was started and has not delivered the pawn yet.
		struct ArrivalState
		{
			bool active = false;
			std::string spawn;
			float pitch = 0.0f;
			int level = -1;            // world level the pawn is going to
			uint16_t changeId = 0;     // Level::CurrentLevelChangeId when the load was asked for
		};

		PendingRequest pending;
		ArrivalState arrival;
		uint32_t performed = 0;

		void Perform(const sim::nav::WorldNavData& world)
		{
			const PendingRequest request = pending;
			pending = PendingRequest();

			SimPawn* pawn = SimPawn::Instance;
			if (!pawn || !Level::Current)
				return;
			const Target target = Resolve(world, request.portal, request.level, request.spawn);
			if (!target.ok)
			{
				Logger::Error("sim transition refused: %s", target.error.c_str());
				return;
			}

			// The level the pawn leaves keeps its own state (doors, moved props) in the engine's level memory, which the game save carries. The pawn is
			// not part of it, and neither are the bodies (SaveGame = false).
			const bool pawnSaved = pawn->SaveGame;
			pawn->SaveGame = false;
			LevelSaveData left = LevelSaveSystem::SaveLevelToData();
			pawn->SaveGame = pawnSaved;
			LevelTraversalSystem::LevelMemory[left.identifier.empty() ? left.name : left.identifier] = left;

			// Entering: the memory of the level, if it has one, as it was left (its own clock included: nothing is caught up).
			const std::string map = SimWorldNavFile::MapPath(world.levels[target.level].name);
			const auto memory = LevelTraversalSystem::LevelMemory.find(map);
			LevelSaveSystem::pendingSave = memory != LevelTraversalSystem::LevelMemory.end() ? memory->second : LevelSaveData();

			LevelTraversalSystem::Traveling = true;
			LevelTraversalSystem::DesiredSpawnPointName = request.spawn;
			LevelTraversalSystem::TimeSkip = 0;

			arrival.active = true;
			arrival.spawn = request.spawn;
			arrival.pitch = request.pitch;
			arrival.level = target.level;
			arrival.changeId = Level::CurrentLevelChangeId;
			++performed;

			Logger::Log("sim transition: %s -> %s at %s", request.portal.c_str(), world.levels[target.level].name.c_str(), request.spawn.c_str());
			Level::LoadLevelFromFile(map);
		}

		// The new level did not create the pawn (its map has no such sim_spawn any more): the world file still knows where the spawn was.
		void Fallback(const sim::nav::WorldNavData& world)
		{
			const sim::nav::WorldSpawn* chosen = nullptr;
			for (const sim::nav::WorldSpawn& spawn : world.spawns)
				if (spawn.level == arrival.level && (spawn.name == arrival.spawn || (!chosen && spawn.playerStart)))
				{
					chosen = &spawn;
					if (spawn.name == arrival.spawn)
						break;
				}
			if (!chosen)
			{
				Logger::Error("sim transition: level %s has no spawn %s and no default start: the pawn could not be placed", world.levels[arrival.level].name.c_str(), arrival.spawn.c_str());
				return;
			}
			Logger::Error("sim transition: the map of %s has no sim_spawn %s (the world file is out of date): the pawn is placed at %s from the world file", world.levels[arrival.level].name.c_str(),
				arrival.spawn.c_str(), chosen->name.c_str());
			SimPawn::SpawnAt(chosen->position - world.levels[arrival.level].offset, 0.0f, arrival.pitch);
		}
	}

	void Request(const std::string& portal, const std::string& targetLevel, const std::string& targetSpawn)
	{
		if (pending.active || arrival.active || !SimPawn::Instance)
			return;
		pending.active = true;
		pending.portal = portal;
		pending.level = targetLevel;
		pending.spawn = targetSpawn;
		pending.pitch = SimPawn::Instance->Pitch();
	}

	void Update(const sim::nav::WorldNavData& world)
	{
		if (Level::HasPendingLevelLoad())
			return;
		if (pending.active)
			Perform(world);
		else if (arrival.active && Level::CurrentLevelChangeId != arrival.changeId)
		{
			// The load is done. TakeArrival ends the arrival when the level had the spawn.
			if (!SimPawn::Instance)
				Fallback(world);
			arrival.active = false;
		}
	}

	bool Arriving()
	{
		return arrival.active && LevelTraversalSystem::Traveling;
	}

	bool TakeArrival(const std::string& spawnName, float& pitch)
	{
		if (!Arriving() || spawnName != arrival.spawn)
			return false;
		pitch = arrival.pitch;
		arrival.active = false;
		return true;
	}

	uint32_t Performed()
	{
		return performed;
	}
}
