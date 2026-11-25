#pragma once

// Small helpers the scenarios share.

#ifndef DISTRIBUTION

#include "../AiTest.h"

#include "../../../Entities/Npc/Npc.h"
#include "../../../Entities/Npc/Director/NpcDirector.h"

#include <Time.hpp>

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

namespace AiTest::Kit
{
	inline std::vector<Snapshot> Alive(Ctx& ctx)
	{
		std::vector<Snapshot> result;
		for (const Snapshot& snapshot : ctx.probe.Latest())
		{
			if (snapshot.alive) result.push_back(snapshot);
		}
		return result;
	}

	// NPCs that are in a fight with the puppet (following it and set to attack).
	inline int CountAttacking(Ctx& ctx)
	{
		int count = 0;
		for (const Snapshot& snapshot : Alive(ctx))
		{
			if (snapshot.attacking) ++count;
		}
		return count;
	}

	inline int CountSeeing(Ctx& ctx)
	{
		int count = 0;
		for (const Snapshot& snapshot : Alive(ctx))
		{
			if (snapshot.sees) ++count;
		}
		return count;
	}

	inline const Snapshot* NearestNpc(Ctx& ctx, const glm::vec3& point)
	{
		const Snapshot* best = nullptr;
		float bestDistance = 1.0e9f;
		for (const Snapshot& snapshot : ctx.probe.Latest())
		{
			if (snapshot.alive == false) continue;
			const float distance = glm::distance(snapshot.position, point);
			if (distance < bestDistance)
			{
				bestDistance = distance;
				best = &snapshot;
			}
		}
		return best;
	}

	inline glm::vec3 Centroid(Ctx& ctx)
	{
		glm::vec3 sum(0.0f);
		int count = 0;
		for (const Snapshot& snapshot : Alive(ctx))
		{
			sum += snapshot.position;
			++count;
		}
		return count > 0 ? sum / static_cast<float>(count) : glm::vec3(0.0f);
	}

	// Eye points of every living NPC.
	inline std::vector<glm::vec3> Eyes(Ctx& ctx)
	{
		std::vector<glm::vec3> eyes;
		for (const Snapshot& snapshot : Alive(ctx))
			eyes.push_back(snapshot.position + glm::vec3(0.0f, 0.6f, 0.0f));
		return eyes;
	}

	// A spot no living NPC can see, `minDistance` or more from each of them, nearest to `around` (tries smaller distances in turn).
	inline bool HiddenFromEveryone(Ctx& ctx, const glm::vec3& around, float minDistance, glm::vec3& out)
	{
		const std::vector<glm::vec3> eyes = Eyes(ctx);
		for (float distance = minDistance; distance >= 12.0f; distance -= 6.0f)
		{
			if (ctx.world.FindSpotHiddenFromAll(eyes, around, distance, 140.0f, out))
				return true;
		}
		return false;
	}

	inline float NearestDistanceToPlayer(Ctx& ctx)
	{
		float best = 1.0e9f;
		for (const Snapshot& snapshot : Alive(ctx))
			best = std::min(best, snapshot.distanceToPlayer);
		return best;
	}

	// ── The new brain ───────────────────────────────────────────────────────────────────────────────────────────────

	inline const glm::vec3 FarAway(300.0f, 100.0f, 300.0f);

	// The NPC of the new class with this name (the scenario fails when there is none).
	inline Npc& Guard(Ctx& ctx, const char* name)
	{
		Npc* npc = dynamic_cast<Npc*>(ctx.world.FindNpc(name));
		if (npc == nullptr)
			ctx.Abort("there is no npc called %s", name);
		return *npc;
	}

	// A guard of the new class standing on the floor at `floor`, turned to `yaw`; `task` is the task point it starts on
	// (its post). The caller lets it settle.
	inline Npc& SpawnGuard(Ctx& ctx, const char* name, const glm::vec3& floor, float yaw, const std::string& task = "", const char* profile = "guard")
	{
		std::map<std::string, std::string> keys;
		keys["profile"] = profile;
		if (task.empty() == false)
			keys["target"] = task;

		Npc* npc = dynamic_cast<Npc*>(ctx.world.SpawnNpc("npc", name, floor + glm::vec3(0.0f, 1.4f, 0.0f), yaw, keys));
		if (npc == nullptr)
			ctx.Abort("could not spawn the npc %s", name);
		return *npc;
	}

	// A task point (class "nav_point": walk there, wait, then go to `next`; a post when there is no next).
	inline void SpawnTaskPoint(Ctx& ctx, const char* name, const glm::vec3& floor, const std::string& next = "", float waitTime = 0.0f)
	{
		std::map<std::string, std::string> keys;
		keys["waitTime"] = std::to_string(waitTime);
		if (next.empty() == false)
			keys["target"] = next;

		if (ctx.world.SpawnEntity("nav_point", name, floor + glm::vec3(0.0f, 0.1f, 0.0f), 0.0f, keys) == nullptr)
			ctx.Abort("could not spawn the task point %s", name);
	}

	// An empty level: the map's own NPCs are gone and the puppet is far away. Returns the floor where the puppet started.
	inline glm::vec3 CleanRoom(Ctx& ctx)
	{
		const glm::vec3 start = ctx.puppet.Position();

		ctx.world.RemoveNpcs();

		glm::vec3 floor;
		if (ctx.world.SnapToFloor(start, floor) == false)
			ctx.Abort("the start position has no navigation grid cell");

		ctx.puppet.Park(floor + FarAway);
		return floor;
	}

	inline float Flat(const glm::vec3& a, const glm::vec3& b)
	{
		return glm::length(glm::vec2(a.x - b.x, a.z - b.z));
	}

	// The angle in degrees between where an NPC's eyes point and the direction to a point.
	inline float GazeError(const Npc& npc, const glm::vec3& point)
	{
		if (npc.observer == nullptr) return 180.0f;

		const glm::vec3 toPoint = point - npc.observer->position;
		if (glm::length(toPoint) < 0.01f) return 0.0f;

		const float cosine = glm::dot(glm::normalize(npc.observer->forward), glm::normalize(toPoint));
		return glm::degrees(std::acos(std::clamp(cosine, -1.0f, 1.0f)));
	}

	// Whether two JSON values are the same, numbers to within `tolerance`. `where` says where they first differ.
	inline bool JsonNear(const json& a, const json& b, double tolerance, std::string& where, const std::string& path = "")
	{
		if (a.is_number() && b.is_number())
		{
			if (std::abs(a.get<double>() - b.get<double>()) <= tolerance) return true;
			where = path + ": " + a.dump() + " against " + b.dump();
			return false;
		}

		if (a.type() != b.type())
		{
			where = path + ": a " + std::string(a.type_name()) + " against a " + b.type_name();
			return false;
		}

		if (a.is_object())
		{
			for (auto it = a.begin(); it != a.end(); ++it)
			{
				if (b.contains(it.key()) == false)
				{
					where = path + "/" + it.key() + ": missing after";
					return false;
				}
				if (JsonNear(it.value(), b[it.key()], tolerance, where, path + "/" + it.key()) == false) return false;
			}
			for (auto it = b.begin(); it != b.end(); ++it)
			{
				if (a.contains(it.key()) == false)
				{
					where = path + "/" + it.key() + ": missing before";
					return false;
				}
			}
			return true;
		}

		if (a.is_array())
		{
			if (a.size() != b.size())
			{
				where = path + ": " + std::to_string(a.size()) + " items against " + std::to_string(b.size());
				return false;
			}
			for (size_t i = 0; i < a.size(); ++i)
			{
				if (JsonNear(a[i], b[i], tolerance, where, path + "[" + std::to_string(i) + "]") == false) return false;
			}
			return true;
		}

		if (a == b) return true;
		where = path + ": " + a.dump() + " against " + b.dump();
		return false;
	}

	// Puts the puppet in the open in sight of the guard nearest to where it starts, shoots, and steps until at
	// least `minAttackers` guards are fighting it. Returns false (with a failed check) when that did not happen.
	inline bool EngageSquad(Ctx& ctx, int minAttackers, float timeout = 30.0f)
	{
		ctx.Wait(3.0f); // let the guards settle (the first frames place them and turn them to their posts)

		const glm::vec3 start = ctx.puppet.Position();
		const Snapshot* nearest = NearestNpc(ctx, start);
		if (nearest == nullptr)
		{
			ctx.Abort("no NPC in the level to fight");
			return false;
		}

		// An armed player is a crime in sight: he has to stand in front of a guard, in the open, for the fight to start
		// (a shot nobody can tie to him is not enough). The nearest guard's facing decides where.
		const glm::vec3 guard = nearest->position;
		const float facing = nearest->yaw;

		glm::vec3 spot;
		bool placed = false;
		for (float distance : { 10.0f, 8.0f, 14.0f, 6.0f, 18.0f })
		{
			if (ctx.world.FindClearSpot(guard, distance, facing, 25.0f, spot))
			{
				placed = true;
				break;
			}
		}

		if (placed == false)
		{
			ctx.Info("no open ground in sight of the nearest guard: fighting from the start position");
			spot = start;
		}

		ctx.puppet.TeleportTo(spot);
		ctx.puppet.Hold(Posture::Still);
		ctx.Wait(1.5f);

		// A player in a fight keeps shooting: a shot every 3 s (each makes him a violent criminal for a few seconds).
		bool engaged = false;
		const double started = ctx.Now();
		while (ctx.Now() - started < timeout)
		{
			ctx.puppet.Shoot();
			engaged = ctx.WaitUntil([&] { return CountAttacking(ctx) >= minAttackers; }, 3.0f);
			if (engaged) break;
		}
		ctx.Check(engaged, "at least %d guards fight the puppet within %.0f s of the first shot (%d do)", minAttackers, timeout, CountAttacking(ctx));
		return engaged;
	}
}

#endif
