#include "SimMovement.h"

#include <Logger.hpp>

#include <entt/container/dense_set.hpp>
#include <glm/geometric.hpp>

#include <algorithm>
#include <climits>
#include <cmath>

namespace sim::movement
{
	namespace
	{
		constexpr float SpeedTolerance = 0.01f; // metres of slack in the speed check (float rounding)
		const glm::vec3 OnMeshExtents{ 0.3f, 1.0f, 0.3f };

		void ClearPath(Walker& walker)
		{
			walker.corners.clear();
			walker.cornerLevels.clear();
			walker.progress = Progress{};
			walker.walkLength = 0.0f;
		}
	}

	MovementModule::MovementModule(nav::WorldNav& navigation) : worldNav(navigation), pathService(navigation) {}

	void MovementModule::Register(Registration& reg)
	{
		reg.Component<Position>();
		reg.Component<Walker>();
		reg.Component<PlayerAnchor>();

		reg.Event<PlayerMoved>(EventInput);
		reg.Event<PathAssigned>();
		reg.Event<PathFailed>(EventSignificant);
		reg.Event<PortalCrossed>();
		reg.Event<Arrived>(EventSignificant);
		reg.Event<PositionException>(EventSignificant);
		reg.Event<Replaced>(EventSignificant);

		tier0Radius = reg.Tunable("tier0_radius", 50.0f, 1.0f, 1000.0f, "metres from the player (same level) stepped at tier 0");
		tier0Step = reg.Tunable("tier0_step", 1.0f, 1.0f, 100.0f, "ticks between integrations at tier 0");
		tier1Step = reg.Tunable("tier1_step", 2.0f, 1.0f, 100.0f, "ticks between integrations at tier 1");
		tier2Step = reg.Tunable("tier2_step", 10.0f, 1.0f, 600.0f, "ticks between integrations at tier 2");
		portalDelay = reg.Tunable("portal_delay", 1.0f, 0.0f, 60.0f, "seconds a walker spends at a level portal");
		pathsPerTick = reg.Tunable("paths_per_tick", 16.0f, 1.0f, 1000.0f, "path requests solved per step, oldest first");
		speedScaleSpread = reg.Tunable("speed_scale_spread", 0.1f, 0.0f, 0.5f, "per-agent speed variation at placement (+/- fraction)");
		placement = reg.RandomStream("placement");

		reg.Command<RequestWalk>([this](ApplyContext& ctx, const RequestWalk& command)
		{
			Walker* walker = ctx.world.TryGet<Walker>(command.walker);
			if (!walker || !ctx.world.TryGet<Position>(command.walker))
			{
				ctx.Reject("no such walker");
				return;
			}
			SyncWalker(ctx, command.walker);
			ClearPath(*walker);
			walker->state = Walker::State::WaitingForPath;
			walker->destination = command.destination;
			walker->gait = command.gait;
			walker->requestedAt = ctx.Now();
			walker->lastIntegrated = ctx.Now();
		});

		reg.Command<StopWalk>([this](ApplyContext& ctx, const StopWalk& command)
		{
			Walker* walker = ctx.world.TryGet<Walker>(command.walker);
			if (!walker || !ctx.world.TryGet<Position>(command.walker))
			{
				ctx.Reject("no such walker");
				return;
			}
			SyncWalker(ctx, command.walker);
			ClearPath(*walker);
			walker->state = Walker::State::Idle;
		});

		reg.Command<SetGait>([this](ApplyContext& ctx, const SetGait& command)
		{
			Walker* walker = ctx.world.TryGet<Walker>(command.walker);
			if (!walker)
			{
				ctx.Reject("no such walker");
				return;
			}
			SyncWalker(ctx, command.walker); // the stretch so far is walked at the old speed
			walker->gait = command.gait;
		});

		reg.Command<SetPlayerLocus>([this](ApplyContext& ctx, const SetPlayerLocus& command)
		{
			if (command.level >= worldNav.Data().levels.size())
			{
				ctx.Reject("unknown level");
				return;
			}
			entt::registry& registry = ctx.world.Registry();
			Entity player = NullEntity;
			for (const Entity entity : registry.view<PlayerAnchor>())
				player = entity;
			if (player == NullEntity)
				player = ctx.Create();
			registry.emplace_or_replace<PlayerAnchor>(player, PlayerAnchor{ command.level, worldNav.ToNav(command.level, command.local) });
		});

		reg.Rule<PlayerMoved>("player_moved", [](RuleContext& ctx, const PlayerMoved& event) { ctx.Command(SetPlayerLocus{ event.level, event.local }); });

		reg.System("paths", [this](ApplyContext& ctx, Tick) { SolvePaths(ctx); });
		reg.System("integrate", [this](ApplyContext& ctx, Tick dt) { Integrate(ctx, dt); });

		reg.Invariant("no_teleport", [this](const World& world, std::string& error) { return CheckNoTeleport(world, error); });
	}

	// ---- Queries ----------------------------------------------------------------------------------------------------

	MovementModule::PlayerInfo MovementModule::FindPlayer(const entt::registry& registry)
	{
		PlayerInfo player;
		for (const auto [entity, anchor] : registry.view<const PlayerAnchor>().each())
		{
			player.present = true;
			player.level = anchor.level;
			player.nav = anchor.nav;
		}
		return player;
	}

	Tier MovementModule::TierFor(const PlayerInfo& player, int level, const glm::vec3& position, float radius)
	{
		if (!player.present || level != player.level)
			return Tier::T2;
		const float dx = position.x - player.nav.x, dz = position.z - player.nav.z;
		return dx * dx + dz * dz <= radius * radius ? Tier::T0 : Tier::T1;
	}

	Tick MovementModule::StepOf(const World& world, Tier tier) const
	{
		const TunableId id = tier == Tier::T0 ? tier0Step : tier == Tier::T1 ? tier1Step : tier2Step;
		return std::max<Tick>(1, static_cast<Tick>(std::lround(world.Tunable(id))));
	}

	float MovementModule::SpeedOf(const Walker& walker) const { return Speed(walker.moverClass, walker.gait, GroundArea) * walker.speedScale; }

	int MovementModule::LevelOf(const World& world, Entity entity) const
	{
		const Walker* walker = world.TryGet<Walker>(entity);
		if (walker && !walker->corners.empty())
			return movement::LevelOf(walker->View(), walker->progress);
		const Position* position = world.TryGet<Position>(entity);
		return position ? worldNav.LevelOfPoly(worldNav.Project(position->nav).poly) : -1;
	}

	Tier MovementModule::TierOf(const World& world, Entity entity) const
	{
		const Position* position = world.TryGet<Position>(entity);
		if (!position)
			return Tier::T2;
		return TierFor(FindPlayer(world.Registry()), LevelOf(world, entity), position->nav, world.Tunable(tier0Radius));
	}

	glm::vec3 MovementModule::PositionAt(const World& world, Entity entity, Tick tick) const
	{
		const Position* position = world.TryGet<Position>(entity);
		if (!position)
			return glm::vec3(0.0f);
		const Walker* walker = world.TryGet<Walker>(entity);
		if (!walker || walker->state != Walker::State::Walking)
			return position->nav;
		Progress progress = walker->progress;
		Advance(walker->View(), progress, TicksToSeconds(std::max<Tick>(0, tick - walker->lastIntegrated)), SpeedOf(*walker), world.Tunable(portalDelay));
		return PositionOf(walker->View(), progress);
	}

	// ---- Snapshot, placements, rebake -------------------------------------------------------------------------------

	void MovementModule::WriteSnapshot(const World& world, std::vector<uint8_t>& blob) const
	{
		const PlayerInfo player = FindPlayer(world.Registry());
		const float radius = world.Tunable(tier0Radius);
		SnapshotView snapshot;
		snapshot.hasPlayer = player.present;
		snapshot.playerLevel = player.level;
		snapshot.playerNav = player.nav;
		std::vector<AgentView>& views = snapshot.agents;
		for (const auto [entity, walker, position] : world.Registry().view<const Walker, const Position>().each())
		{
			AgentView view;
			view.entity = entity;
			view.level = static_cast<uint16_t>(std::max(LevelOf(world, entity), 0));
			view.tier = static_cast<uint8_t>(TierFor(player, view.level, position.nav, radius));
			view.state = static_cast<uint8_t>(walker.state);
			view.nav = position.nav;
			views.push_back(view);
		}
		BinaryWriter writer;
		writer.Write(snapshot);
		blob = std::move(writer.bytes);
	}

	bool MovementModule::ReadSnapshot(const std::vector<uint8_t>& blob, SnapshotView& out)
	{
		BinaryReader reader(blob.data(), blob.size());
		out = SnapshotView{};
		reader.Read(out);
		return !reader.Failed() && reader.AtEnd();
	}

	std::vector<Placement> MovementModule::PlacementsOf(const World& world) const
	{
		std::vector<Placement> placements;
		for (const auto [entity, walker, position] : world.Registry().view<const Walker, const Position>().each())
		{
			const int level = LevelOf(world, entity);
			if (level < 0)
				continue;
			placements.push_back({ entity, static_cast<uint16_t>(level), worldNav.ToLocal(level, position.nav) });
		}
		return placements;
	}

	std::vector<uint8_t> MovementModule::WritePlacements(const std::vector<Placement>& placements)
	{
		BinaryWriter writer;
		writer.Write(placements);
		return std::move(writer.bytes);
	}

	bool MovementModule::ReadPlacements(const std::vector<uint8_t>& bytes, std::vector<Placement>& out)
	{
		BinaryReader reader(bytes.data(), bytes.size());
		out.clear();
		reader.Read(out);
		return !reader.Failed() && reader.AtEnd();
	}

	size_t MovementModule::ReplaceAfterRebake(Server& server, const std::vector<std::string>& oldLevels, const std::vector<Placement>& placements) const
	{
		size_t lost = 0;
		server.RunSetup("rebake migration", [&](ApplyContext& ctx)
		{
			entt::registry& registry = ctx.world.Registry();
			entt::dense_set<uint32_t> placed;
			for (const Placement& placement : placements)
			{
				Walker* walker = ctx.world.TryGet<Walker>(placement.entity);
				Position* position = ctx.world.TryGet<Position>(placement.entity);
				if (!walker || !position)
					continue;
				placed.insert(EntityIndex(placement.entity));
				const int level = placement.level < oldLevels.size() ? worldNav.Data().FindLevel(oldLevels[placement.level]) : -1;
				nav::NavPoint point;
				for (const float reach : { 2.0f, 10.0f, 50.0f })
				{
					if (level < 0 || point.IsValid())
						break;
					point = worldNav.Project(worldNav.ToNav(level, placement.local), { reach, 4.0f, reach });
				}
				if (!point.IsValid())
				{
					Logger::Error("sim: rebake migration: agent #%u had no place in %s (%s) and is removed", EntityIndex(placement.entity),
						placement.level < oldLevels.size() ? oldLevels[placement.level].c_str() : "an unknown level", level < 0 ? "the level is gone" : "no mesh within 50 m");
					ctx.Destroy(placement.entity);
					++lost;
					continue;
				}
				const glm::vec3 before = position->nav;
				ClearPath(*walker);
				walker->state = Walker::State::Idle;
				position->nav = point.position;
				walker->lastTo = point.position;
				walker->lastWalked = 0.0f;
				walker->lastTicks = 0;
				walker->lastIntegrated = ctx.Now();
				ctx.Emit(PositionException{ placement.entity, before, point.position, "rebake" });
				ctx.Emit(Replaced{ placement.entity });
			}

			// A walker the save did not describe cannot be placed either.
			std::vector<Entity> unplaced;
			for (const auto [entity, walker] : registry.view<Walker>().each())
				if (!placed.contains(EntityIndex(entity)))
					unplaced.push_back(entity);
			for (const Entity entity : unplaced)
			{
				Logger::Error("sim: rebake migration: agent #%u is not in the save's placements and is removed", EntityIndex(entity));
				ctx.Destroy(entity);
				++lost;
			}
			std::vector<Entity> anchors;
			for (const Entity entity : registry.view<PlayerAnchor>())
				anchors.push_back(entity);
			for (const Entity entity : anchors)
				ctx.Destroy(entity);
		});
		return lost;
	}

	// ---- Placement and the position chokepoint ----------------------------------------------------------------------

	Entity MovementModule::Place(ApplyContext& ctx, const glm::vec3& navPosition, MoverClass moverClass) const
	{
		const Entity entity = ctx.Create();
		Walker walker;
		walker.moverClass = moverClass;
		walker.speedScale = 1.0f + ctx.world.Tunable(speedScaleSpread) * (ctx.Stream(placement).Unit() * 2.0f - 1.0f);
		walker.lastTo = navPosition;
		walker.lastIntegrated = ctx.Now();
		ctx.world.Registry().emplace<Position>(entity, Position{ navPosition });
		ctx.world.Registry().emplace<Walker>(entity, std::move(walker));
		ctx.Emit(PositionException{ entity, navPosition, navPosition, "placed" });
		return entity;
	}

	void MovementModule::AdvanceWalker(ApplyContext& ctx, Entity entity, Walker& walker, Position& position) const
	{
		const Tick now = ctx.Now();
		const Tick previous = walker.lastIntegrated;
		const Tick elapsed = std::max<Tick>(0, now - previous);
		const AdvanceResult result = Advance(walker.View(), walker.progress, TicksToSeconds(elapsed), SpeedOf(walker), ctx.world.Tunable(portalDelay));

		for (const uint32_t corner : result.crossings)
		{
			ctx.Emit(PortalCrossed{ entity, walker.cornerLevels[corner], walker.cornerLevels[corner + 1] });
			ctx.Emit(PositionException{ entity, walker.corners[corner], walker.corners[corner + 1], "portal" });
		}

		// The one place a walker's position is written after placement; the detector invariant compares against it.
		const glm::vec3 where = PositionOf(walker.View(), walker.progress);
		position.nav = where;
		walker.lastTo = where;
		walker.lastWalked = static_cast<float>(result.walked);
		walker.lastTicks = elapsed;
		walker.lastIntegrated = now;

		if (result.arrived)
		{
			const Tick at = std::clamp<Tick>(previous + static_cast<Tick>(std::ceil(result.timeUsed * TicksPerSecond - 1e-6)), previous, now);
			walker.state = Walker::State::Idle;
			ClearPath(walker);
			ctx.Emit(Arrived{ entity, at });
		}
	}

	void MovementModule::SyncWalker(ApplyContext& ctx, Entity entity) const
	{
		Walker* walker = ctx.world.TryGet<Walker>(entity);
		Position* position = ctx.world.TryGet<Position>(entity);
		if (walker && position && walker->state == Walker::State::Walking)
			AdvanceWalker(ctx, entity, *walker, *position);
	}

	// ---- Systems ----------------------------------------------------------------------------------------------------

	void MovementModule::SolvePaths(ApplyContext& ctx)
	{
		entt::registry& registry = ctx.world.Registry();
		struct Waiting
		{
			Tick requestedAt;
			Entity entity;
		};
		std::vector<Waiting> waiting;
		for (const auto [entity, walker] : registry.view<Walker>().each())
			if (walker.state == Walker::State::WaitingForPath)
				waiting.push_back({ walker.requestedAt, entity });
		if (waiting.empty())
			return;
		// Oldest first; walkers asking in the same tick go by entity index.
		std::sort(waiting.begin(), waiting.end(), [](const Waiting& a, const Waiting& b)
		{
			return a.requestedAt != b.requestedAt ? a.requestedAt < b.requestedAt : EntityIndex(a.entity) < EntityIndex(b.entity);
		});
		waiting.resize(std::min(waiting.size(), static_cast<size_t>(std::max(1.0f, ctx.world.Tunable(pathsPerTick)))));

		auto fail = [&](Entity entity, Walker& walker, const char* reason)
		{
			walker.state = Walker::State::Idle;
			ClearPath(walker);
			ctx.Emit(PathFailed{ entity, reason });
		};

		struct Issued
		{
			Entity entity;
			nav::PathService::RequestId request;
		};
		std::vector<Issued> issued;
		for (const Waiting& entry : waiting)
		{
			Walker& walker = registry.get<Walker>(entry.entity);
			const nav::NavPoint start = worldNav.Project(registry.get<Position>(entry.entity).nav);
			const nav::NavPoint end = worldNav.Project(walker.destination);
			if (!start.IsValid())
				fail(entry.entity, walker, "start_off_mesh");
			else if (!end.IsValid())
				fail(entry.entity, walker, "destination_off_mesh");
			else
				issued.push_back({ entry.entity, pathService.Request(start, end) });
		}

		pathService.Update(INT_MAX);
		const std::vector<nav::PathService::Result> results = pathService.TakeResults(); // in request order
		for (size_t i = 0; i < results.size() && i < issued.size(); ++i)
		{
			const Entity entity = issued[i].entity;
			Walker& walker = registry.get<Walker>(entity);
			const nav::PathService::Result& result = results[i];
			if (result.id != issued[i].request || result.failed || result.path.points.empty())
			{
				fail(entity, walker, "no_path");
				continue;
			}
			if (!result.path.complete)
			{
				fail(entity, walker, "unreachable");
				continue;
			}

			walker.corners = result.path.points;
			walker.corners.front() = registry.get<Position>(entity).nav; // no pop: start exactly where the agent is
			walker.cornerLevels.clear();
			for (const int level : result.path.pointLevels)
				walker.cornerLevels.push_back(static_cast<uint16_t>(std::max(level, 0)));
			walker.progress = Progress{};
			walker.walkLength = static_cast<float>(WalkLength(walker.View()));
			walker.state = Walker::State::Walking;
			walker.lastIntegrated = ctx.Now();
			ctx.Emit(PathAssigned{ entity, walker.walkLength, static_cast<uint16_t>(walker.corners.size()), static_cast<uint16_t>(result.path.portalCrossings) });
		}
	}

	void MovementModule::Integrate(ApplyContext& ctx, Tick dt) const
	{
		entt::registry& registry = ctx.world.Registry();
		const PlayerInfo player = FindPlayer(registry);
		const float radius = ctx.world.Tunable(tier0Radius);
		const Tick steps[3] = { StepOf(ctx.world, Tier::T0), StepOf(ctx.world, Tier::T1), StepOf(ctx.world, Tier::T2) };
		const Tick now = ctx.Now();

		for (const auto [entity, walker, position] : registry.view<Walker, Position>().each())
		{
			if (walker.state != Walker::State::Walking)
				continue;
			const Tier tier = TierFor(player, movement::LevelOf(walker.View(), walker.progress), position.nav, radius);
			if (IsStaggerDue(entity, now, dt, steps[static_cast<int>(tier)]))
				AdvanceWalker(ctx, entity, walker, position);
		}
	}

	// ---- Detector ---------------------------------------------------------------------------------------------------

	bool MovementModule::CheckNoTeleport(const World& world, std::string& error) const
	{
		for (const auto [entity, walker, position] : world.Registry().view<const Walker, const Position>().each())
		{
			if (position.nav != walker.lastTo)
			{
				error = world.DescribeEntity(entity) + ": position was written outside the movement module";
				return false;
			}
			const float allowed = MaxSpeed(walker.moverClass) * walker.speedScale * static_cast<float>(TicksToSeconds(walker.lastTicks)) + SpeedTolerance;
			if (walker.lastWalked > allowed)
			{
				error = world.DescribeEntity(entity) + ": walked " + std::to_string(walker.lastWalked) + " m in " + std::to_string(walker.lastTicks) +
					" ticks, at most " + std::to_string(allowed) + " m is possible";
				return false;
			}
			if (walker.lastTicks > 0 && walker.lastIntegrated == world.Now() && !worldNav.Project(position.nav, OnMeshExtents).IsValid())
			{
				error = world.DescribeEntity(entity) + ": position is not on the nav mesh";
				return false;
			}
		}
		return true;
	}
}
