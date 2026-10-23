#include "SimNpcs.h"

#include <Sim/Bodies/SimBodies.h>

#include <Logger.hpp>

#include <glm/geometric.hpp>

#include <algorithm>
#include <cstdarg>
#include <cstdio>

namespace sim::npcs
{
	namespace
	{
		const glm::vec3 StopReach{ 1.5f, 3.0f, 1.5f }; // how far from the mesh a stop may be authored

		std::string Fmt(const char* format, ...)
		{
			char buffer[512];
			va_list args;
			va_start(args, format);
			std::vsnprintf(buffer, sizeof(buffer), format, args);
			va_end(args);
			return buffer;
		}
	}

	NpcModule::NpcModule(const movement::MovementModule& movement, const nav::WorldNav& worldNav) : movement(movement), worldNav(worldNav) {}

	void NpcModule::Register(Registration& reg)
	{
		reg.Component<Routine>();
		reg.Event<RoutineStep>();

		retryDelay = reg.Tunable("retry_delay", 3.0f, 0.1f, 600.0f, "seconds an NPC waits before trying its next stop after a walk failed or it was re-placed");
		startDelayMax = reg.Tunable("start_delay_max", 5.0f, 0.0f, 600.0f, "NPCs start their routines after a random delay up to this many seconds, so they do not move in step");
		starts = reg.RandomStream("starts");

		reg.Command<RoutineAdvance>([this](ApplyContext& ctx, const RoutineAdvance& command) { Advance(ctx, command.npc); });
		reg.Rule<RoutineStep>("step", [](RuleContext& ctx, const RoutineStep& event) { ctx.Command(RoutineAdvance{ event.npc }); });

		// Arrived at a stop: stay as long as the stop says, then go on.
		reg.Rule<movement::Arrived>("wait_at_stop", [](RuleContext& ctx, const movement::Arrived& event)
		{
			const Routine* routine = ctx.world.TryGet<Routine>(event.walker);
			if (!routine || routine->current >= routine->stops.size())
				return;
			const RoutineStop& stop = routine->stops[routine->current];
			Rng rng = ctx.Random();
			const double seconds = stop.waitMax > stop.waitMin ? rng.Range(stop.waitMin, stop.waitMax) : stop.waitMin;
			ctx.Schedule(ctx.Now() + std::max<Tick>(1, SecondsToTicks(seconds)), RoutineStep{ event.walker });
		});

		// No way there, or put back on the mesh by a load: try again a little later.
		const TunableId retry = retryDelay;
		reg.Rule<movement::PathFailed>("retry_after_failure", [retry](RuleContext& ctx, const movement::PathFailed& event)
		{
			if (ctx.world.TryGet<Routine>(event.walker))
				ctx.Schedule(ctx.Now() + std::max<Tick>(1, SecondsToTicks(ctx.world.Tunable(retry))), RoutineStep{ event.walker });
		});
		reg.Rule<movement::Replaced>("retry_after_replace", [retry](RuleContext& ctx, const movement::Replaced& event)
		{
			if (ctx.world.TryGet<Routine>(event.walker))
				ctx.Schedule(ctx.Now() + std::max<Tick>(1, SecondsToTicks(ctx.world.Tunable(retry))), RoutineStep{ event.walker });
		});
	}

	nav::NavPoint NpcModule::StopPoint(const RoutineStop& stop) const
	{
		const int level = worldNav.Data().FindLevel(stop.level.str());
		if (level < 0)
			return {};
		return worldNav.Project(worldNav.ToNav(level, stop.local), StopReach);
	}

	std::vector<std::string> NpcModule::Validate(const NpcDefinition& definition) const
	{
		std::vector<std::string> problems;
		const std::string who = "npc '" + definition.id + "'";
		if (definition.id.empty() || definition.id.size() > 23)
			problems.push_back(who + ": the id must be 1 to 23 characters");
		if (definition.stops.empty())
			problems.push_back(who + ": has no stops");
		if (definition.model < 0 || definition.model >= bodies::BodiesModule::ModelCount)
			problems.push_back(Fmt("%s: model %d does not exist (0 and %d are valid)", who.c_str(), definition.model, bodies::BodiesModule::ModelCount - 1));
		if (!(definition.speed >= 0.2f && definition.speed <= 3.0f))
			problems.push_back(Fmt("%s: speed %.2f is outside 0.2 to 3", who.c_str(), definition.speed));

		nav::NavPoint first;
		for (size_t i = 0; i < definition.stops.size(); ++i)
		{
			const StopDefinition& stop = definition.stops[i];
			const int level = worldNav.Data().FindLevel(stop.level);
			if (level < 0)
			{
				problems.push_back(Fmt("%s stop %zu: level '%s' is not in the world", who.c_str(), i, stop.level.c_str()));
				continue;
			}
			if (stop.level.size() > 39)
				problems.push_back(Fmt("%s stop %zu: level name is too long", who.c_str(), i));
			if (stop.waitMin < 0.0f || stop.waitMax < stop.waitMin)
				problems.push_back(Fmt("%s stop %zu: wait %.1f to %.1f s is not a valid range", who.c_str(), i, stop.waitMin, stop.waitMax));
			const nav::NavPoint point = worldNav.Project(worldNav.ToNav(level, stop.local), StopReach);
			if (!point.IsValid())
			{
				problems.push_back(Fmt("%s stop %zu: (%.1f %.1f %.1f) in %s is not on the nav mesh (within %.1f m sideways, %.1f m up or down)", who.c_str(), i, stop.local.x, stop.local.y,
					stop.local.z, stop.level.c_str(), StopReach.x, StopReach.y));
				continue;
			}
			if (!first.IsValid())
				first = point;
			else
			{
				const nav::NavPath path = worldNav.FindPath(first, point);
				if (!path.complete)
					problems.push_back(Fmt("%s stop %zu: (%.1f %.1f %.1f) in %s cannot be reached on foot from stop 0", who.c_str(), i, stop.local.x, stop.local.y, stop.local.z, stop.level.c_str()));
			}
		}
		return problems;
	}

	Entity NpcModule::Place(ApplyContext& ctx, const NpcDefinition& definition) const
	{
		Routine routine;
		routine.mode = definition.mode;
		routine.gait = definition.gait;
		for (const StopDefinition& stop : definition.stops)
			routine.stops.push_back({ stop.level, stop.local, stop.waitMin, stop.waitMax });

		const nav::NavPoint start = routine.stops.empty() ? nav::NavPoint{} : StopPoint(routine.stops[0]);
		if (!start.IsValid())
		{
			Logger::Error("sim: npc '%s' cannot be placed: its first stop is not on the mesh", definition.id.c_str());
			return NullEntity;
		}

		const Entity npc = movement.Place(ctx, start.position);
		ctx.world.TryGet<movement::Walker>(npc)->speedScale = definition.speed;
		ctx.world.Registry().emplace<Identity>(npc, Identity{ definition.id.c_str(), definition.name });
		ctx.world.Registry().emplace<bodies::Appearance>(npc, bodies::Appearance{ static_cast<uint8_t>(definition.model) });
		ctx.world.Registry().emplace<Routine>(npc, std::move(routine));

		const double delay = ctx.Stream(starts).Unit() * ctx.world.Tunable(startDelayMax);
		ctx.Schedule(ctx.Now() + std::max<Tick>(1, SecondsToTicks(delay)), RoutineStep{ npc });
		return npc;
	}

	void NpcModule::Advance(ApplyContext& ctx, Entity npc) const
	{
		Routine* routine = ctx.world.TryGet<Routine>(npc);
		if (!routine)
		{
			ctx.Reject("no such npc");
			return;
		}
		const int count = static_cast<int>(routine->stops.size());
		if (count < 2)
			return; // a stationary NPC

		// The next stop that still exists on the mesh (a rebake can remove one).
		int index = static_cast<int>(routine->current);
		for (int tries = 0; tries < count; ++tries)
		{
			if (routine->mode == Routine::Mode::Loop)
				index = (index + 1) % count;
			else
			{
				if (index + routine->direction < 0 || index + routine->direction >= count)
					routine->direction = -routine->direction;
				index += routine->direction;
			}
			const nav::NavPoint point = StopPoint(routine->stops[index]);
			if (!point.IsValid())
				continue;
			routine->current = static_cast<uint32_t>(index);
			movement.StartWalk(ctx, npc, point.position, routine->gait);
			return;
		}
		Logger::Warning("sim: npc #%u has no stop left on the mesh", EntityIndex(npc));
	}
}
