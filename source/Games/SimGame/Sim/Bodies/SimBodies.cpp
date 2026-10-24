#include "SimBodies.h"

#include <Logger.hpp>

#include <glm/geometric.hpp>

#include <algorithm>
#include <cmath>

namespace sim::bodies
{
	namespace
	{
		struct PlayerInfo
		{
			bool present = false;
			uint16_t level = 0;
			glm::vec3 nav{ 0.0f };
		};

		PlayerInfo FindPlayer(const entt::registry& registry)
		{
			PlayerInfo player;
			for (const auto [entity, anchor] : registry.view<const movement::PlayerAnchor>().each())
			{
				player.present = true;
				player.level = anchor.level;
				player.nav = anchor.nav;
			}
			return player;
		}

		float HorizontalDistance(const glm::vec3& a, const glm::vec3& b)
		{
			const float dx = a.x - b.x, dz = a.z - b.z;
			return std::sqrt(dx * dx + dz * dz);
		}

		Tick Ticks(const World& world, TunableId id) { return std::max<Tick>(1, static_cast<Tick>(std::lround(world.Tunable(id)))); }
	}

	BodiesModule::BodiesModule(const movement::MovementModule& movement, const nav::WorldNav& worldNav) : movement(movement), worldNav(worldNav) {}

	size_t BodiesModule::EmbodiedCount(const World& world)
	{
		const auto* storage = world.Registry().storage<Embodied>();
		return storage ? storage->size() : 0;
	}

	void BodiesModule::Register(Registration& reg)
	{
		reg.Component<Appearance>();
		reg.Component<BodyRetry>();

		reg.Event<BodySpawnProposed>(EventOutput);
		reg.Event<BodyDespawnProposed>(EventOutput);
		reg.Event<BodyGoal>(EventOutput);
		reg.Event<BodyAccepted>(EventInput);
		reg.Event<BodyRefused>(EventInput);
		reg.Event<BodyReleased>(EventInput);
		reg.Event<BodyReport>(EventInput);
		reg.Event<BodiesCleared>(EventInput);

		// The test levels are about 64 m across and have no fog, so what is inside the level must exist as a body: the radii are wider
		// than a level, and bodies come and go with the player's level and with portals. A big open level wants these near 80 and 100.
		spawnRadius = reg.Tunable("spawn_radius", 65.0f, 1.0f, 1000.0f, "metres from the player (same level) within which agents get a body (the game shows it only within its visible radius, see SimAgents)");
		despawnRadius = reg.Tunable("despawn_radius", 80.0f, 1.0f, 1000.0f, "metres from the player beyond which a body is taken back (above spawn_radius)");
		maxBodies = reg.Tunable("max_bodies", 32.0f, 0.0f, 512.0f, "bodies at the same time (proposed and active)");
		proposePeriod = reg.Tunable("propose_period", 5.0f, 1.0f, 100.0f, "ticks between spawn and despawn proposals");
		proposalTimeout = reg.Tunable("proposal_timeout", 30.0f, 1.0f, 1000.0f, "ticks the game has to answer a spawn proposal");
		reportTimeout = reg.Tunable("report_timeout", 30.0f, 1.0f, 1000.0f, "ticks without a position report after which a body is given up");
		retryCooldown = reg.Tunable("retry_cooldown", 50.0f, 1.0f, 1000.0f, "ticks before an agent whose body was refused or lost is proposed again");
		releaseTimeout = reg.Tunable("release_timeout", 60.0f, 1.0f, 1000.0f, "ticks the game has to release a body after a despawn proposal");
		look = reg.RandomStream("look");

		reg.Command<ApplyBodyInput>([this](ApplyContext& ctx, const ApplyBodyInput& input) { ApplyInput(ctx, input); });

		using Kind = ApplyBodyInput::Kind;
		reg.Rule<BodyAccepted>("accepted", [](RuleContext& ctx, const BodyAccepted& event) { ctx.Command(ApplyBodyInput{ event.entity, event.local, Kind::Accepted }); });
		reg.Rule<BodyRefused>("refused", [](RuleContext& ctx, const BodyRefused& event) { ctx.Command(ApplyBodyInput{ event.entity, glm::vec3(0.0f), Kind::Refused }); });
		reg.Rule<BodyReleased>("released", [](RuleContext& ctx, const BodyReleased& event) { ctx.Command(ApplyBodyInput{ event.entity, event.local, Kind::Released }); });
		reg.Rule<BodyReport>("report", [](RuleContext& ctx, const BodyReport& event) { ctx.Command(ApplyBodyInput{ event.entity, event.local, Kind::Report }); });
		reg.Rule<BodiesCleared>("cleared", [](RuleContext& ctx, const BodiesCleared&) { ctx.Command(ApplyBodyInput{ NullEntity, glm::vec3(0.0f), Kind::Cleared }); });

		reg.System("update", [this](ApplyContext& ctx, Tick dt) { Update(ctx, dt); });
	}

	glm::vec3 BodiesModule::LocalOf(const World& world, Entity entity, const glm::vec3& nav) const
	{
		const int level = movement.LevelOf(world, entity);
		return level >= 0 ? worldNav.ToLocal(level, nav) : nav;
	}

	void BodiesModule::Update(ApplyContext& ctx, Tick dt) const
	{
		Maintain(ctx);
		const Tick now = ctx.Now();
		const Tick period = Ticks(ctx.world, proposePeriod);
		if (now / period != (now - dt) / period)
			Propose(ctx);
	}

	// Every tick: time-outs, and goals for bodies that are the mover.
	void BodiesModule::Maintain(ApplyContext& ctx) const
	{
		entt::registry& registry = ctx.world.Registry();
		const Tick now = ctx.Now();
		std::vector<Entity> embodied;
		for (const Entity entity : registry.view<Embodied>())
			embodied.push_back(entity);

		for (const Entity entity : embodied)
		{
			Embodied& body = registry.get<Embodied>(entity);
			movement::Walker* walker = ctx.world.TryGet<movement::Walker>(entity);
			movement::Position* position = ctx.world.TryGet<movement::Position>(entity);
			if (!walker || !position)
			{
				registry.remove<Embodied>(entity);
				continue;
			}

			if (body.state == Embodied::State::Proposed)
			{
				if (now - body.since > ctx.world.Tunable(proposalTimeout))
				{
					registry.remove<Embodied>(entity);
					registry.emplace_or_replace<BodyRetry>(entity, BodyRetry{ now + Ticks(ctx.world, retryCooldown) });
				}
				continue;
			}

			if (body.state == Embodied::State::Releasing)
			{
				if (now - body.since > ctx.world.Tunable(releaseTimeout))
				{
					Logger::Warning("sim: the game did not release the body of agent #%u in time; it is taken back", EntityIndex(entity));
					registry.remove<Embodied>(entity);
				}
				continue;
			}

			if (now - body.lastReport > ctx.world.Tunable(reportTimeout))
			{
				Logger::Warning("sim: the body of agent #%u stopped reporting; the agent walks on by itself", EntityIndex(entity));
				registry.remove<Embodied>(entity);
				registry.emplace_or_replace<BodyRetry>(entity, BodyRetry{ now + Ticks(ctx.world, retryCooldown) });
				continue;
			}

			// Goals: the end corner of the leg the walker is on, whenever the leg, the path or the gait changes.
			if (walker->state == movement::Walker::State::Walking && !walker->corners.empty())
			{
				const uint32_t leg = walker->progress.leg;
				const size_t count = walker->corners.size();
				if (walker->progress.crossing || movement::IsPortalLeg(walker->View(), leg) || leg + 1 >= count)
					continue; // across a portal the sim walks the agent itself
				if (body.goalSerial != walker->pathSerial || body.goalLeg != leg || body.goalGait != walker->gait || body.stopSent)
				{
					BodyGoal goal;
					goal.entity = entity;
					goal.local = LocalOf(ctx.world, entity, walker->corners[leg + 1]);
					goal.kind = BodyGoal::Kind::Move;
					goal.gait = walker->gait;
					goal.final = leg + 2 == count;
					goal.hasNext = !goal.final && !movement::IsPortalLeg(walker->View(), leg + 1);
					if (goal.hasNext)
						goal.next = LocalOf(ctx.world, entity, walker->corners[leg + 2]);
					goal.speedScale = walker->speedScale;
					ctx.Emit(goal);
					body.goalSerial = walker->pathSerial;
					body.goalLeg = leg;
					body.goalGait = walker->gait;
					body.stopSent = false;
				}
			}
			else if (!body.stopSent)
			{
				BodyGoal goal;
				goal.entity = entity;
				goal.local = LocalOf(ctx.world, entity, position->nav);
				goal.kind = BodyGoal::Kind::Stop;
				goal.speedScale = walker->speedScale;
				ctx.Emit(goal);
				body.stopSent = true;
				body.goalLeg = Embodied::NoGoal;
			}
		}
	}

	// Every propose_period ticks: take bodies back, then hand out new ones, nearest first.
	void BodiesModule::Propose(ApplyContext& ctx) const
	{
		entt::registry& registry = ctx.world.Registry();
		const Tick now = ctx.Now();
		const PlayerInfo player = FindPlayer(registry);
		const float despawn = ctx.world.Tunable(despawnRadius);
		const float spawn = std::min(ctx.world.Tunable(spawnRadius), despawn);

		std::vector<Entity> embodied;
		for (const Entity entity : registry.view<Embodied>())
			embodied.push_back(entity);
		for (const Entity entity : embodied)
		{
			Embodied& body = registry.get<Embodied>(entity);
			if (body.state == Embodied::State::Releasing)
				continue;
			const movement::Walker* walker = ctx.world.TryGet<movement::Walker>(entity);
			const movement::Position* position = ctx.world.TryGet<movement::Position>(entity);
			if (!walker || !position)
				continue;
			const bool atPortal = walker->state == movement::Walker::State::Walking && !walker->corners.empty() &&
				(walker->progress.crossing || movement::IsPortalLeg(walker->View(), walker->progress.leg));
			BodyDespawnProposed::Reason reason = BodyDespawnProposed::Reason::Unknown;
			if (!player.present)
				reason = BodyDespawnProposed::Reason::NoPlayer;
			else if (atPortal)
				reason = BodyDespawnProposed::Reason::AtPortal;
			else if (movement.LevelOf(ctx.world, entity) != player.level)
				reason = BodyDespawnProposed::Reason::OtherLevel;
			else if (HorizontalDistance(position->nav, player.nav) > despawn)
				reason = BodyDespawnProposed::Reason::OutOfRange;
			if (reason == BodyDespawnProposed::Reason::Unknown)
				continue;
			body.state = Embodied::State::Releasing;
			body.since = now;
			ctx.Emit(BodyDespawnProposed{ entity, reason });
		}

		if (!player.present)
			return;
		size_t count = EmbodiedCount(ctx.world);
		const size_t budget = static_cast<size_t>(ctx.world.Tunable(maxBodies));
		if (count >= budget)
			return;

		struct Candidate
		{
			float distance;
			Entity entity;
		};
		std::vector<Candidate> candidates;
		for (const auto [entity, walker, position] : registry.view<const movement::Walker, const movement::Position>().each())
		{
			const float distance = HorizontalDistance(position.nav, player.nav);
			if (distance > spawn || registry.all_of<Embodied>(entity))
				continue;
			if (const BodyRetry* retry = ctx.world.TryGet<BodyRetry>(entity); retry && retry->until > now)
				continue;
			const bool atPortal = walker.state == movement::Walker::State::Walking && !walker.corners.empty() &&
				(walker.progress.crossing || movement::IsPortalLeg(walker.View(), walker.progress.leg));
			if (atPortal || movement.LevelOf(ctx.world, entity) != player.level)
				continue;
			candidates.push_back({ distance, entity });
		}
		std::sort(candidates.begin(), candidates.end(), [](const Candidate& a, const Candidate& b)
		{
			return a.distance != b.distance ? a.distance < b.distance : EntityIndex(a.entity) < EntityIndex(b.entity);
		});

		for (const Candidate& candidate : candidates)
		{
			if (count >= budget)
				break;
			const Entity entity = candidate.entity;
			const movement::Walker& walker = registry.get<movement::Walker>(entity);
			const movement::Position& position = registry.get<movement::Position>(entity);

			if (!registry.all_of<Appearance>(entity))
				registry.emplace<Appearance>(entity, Appearance{ static_cast<uint8_t>(ctx.Stream(look).Below(ModelCount)) });

			glm::vec3 heading(0.0f, 0.0f, 1.0f);
			if (walker.state == movement::Walker::State::Walking && walker.progress.leg + 1 < walker.corners.size())
			{
				const glm::vec3 direction = walker.corners[walker.progress.leg + 1] - walker.corners[walker.progress.leg];
				const glm::vec3 flat(direction.x, 0.0f, direction.z);
				if (glm::dot(flat, flat) > 1e-6f)
					heading = glm::normalize(flat);
			}

			Embodied body;
			body.state = Embodied::State::Proposed;
			body.since = now;
			body.lastReport = now;
			registry.emplace<Embodied>(entity, body);
			ctx.Emit(BodySpawnProposed{ entity, LocalOf(ctx.world, entity, position.nav), heading, walker.speedScale, registry.get<Appearance>(entity).model });
			++count;
		}
	}

	void BodiesModule::ApplyInput(ApplyContext& ctx, const ApplyBodyInput& input) const
	{
		entt::registry& registry = ctx.world.Registry();
		const Tick now = ctx.Now();
		if (input.kind == ApplyBodyInput::Kind::Cleared)
		{
			registry.clear<Embodied>();
			return;
		}
		Embodied* body = ctx.world.TryGet<Embodied>(input.entity);
		if (!ctx.world.IsAlive(input.entity) || !ctx.world.TryGet<movement::Walker>(input.entity))
		{
			// The game has a body for something that is gone. Only a report or an acceptance means the body still exists.
			if (input.kind == ApplyBodyInput::Kind::Accepted || input.kind == ApplyBodyInput::Kind::Report)
				ctx.Emit(BodyDespawnProposed{ input.entity });
			return;
		}

		switch (input.kind)
		{
		case ApplyBodyInput::Kind::Accepted:
			if (!body || body->state != Embodied::State::Proposed)
			{
				ctx.Emit(BodyDespawnProposed{ input.entity }); // answered too late: the proposal is over
				return;
			}
			body->state = Embodied::State::Active;
			body->since = now;
			body->lastReport = now;
			movement.ApplyBodyPosition(ctx, input.entity, input.local, true);
			break;
		case ApplyBodyInput::Kind::Refused:
			if (body)
			{
				registry.remove<Embodied>(input.entity);
				registry.emplace_or_replace<BodyRetry>(input.entity, BodyRetry{ now + Ticks(ctx.world, retryCooldown) });
			}
			break;
		case ApplyBodyInput::Kind::Released:
			if (body)
			{
				if (body->state != Embodied::State::Proposed)
					movement.ApplyBodyPosition(ctx, input.entity, input.local, false);
				registry.remove<Embodied>(input.entity);
			}
			break;
		case ApplyBodyInput::Kind::Report:
			if (!body)
			{
				ctx.Emit(BodyDespawnProposed{ input.entity });
				return;
			}
			if (body->state == Embodied::State::Proposed)
				return;
			body->lastReport = now;
			movement.ApplyBodyPosition(ctx, input.entity, input.local, false);
			break;
		case ApplyBodyInput::Kind::Cleared:
			break;
		}
	}
}
