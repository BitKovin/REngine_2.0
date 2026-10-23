// Abstract movement tests (SLICE_M2_MOVEMENT.md section 8): the walking maths alone, then the module on a baked chain of
// four real levels with six portals.

#include "SimNavFixtures.h"

#ifndef DISTRIBUTION

#include <Sim/Core/SimServer.h>
#include <Sim/Movement/SimDemoPopulation.h>
#include <Sim/Movement/SimMovement.h>
#include <Sim/Nav/WorldNavBake.h>

#include <entt/container/dense_map.hpp>
#include <glm/geometric.hpp>

#include <algorithm>
#include <cmath>
#include <memory>

namespace sim::test
{
	namespace
	{
		using namespace sim::movement;
		using sim::nav::NavPoint;
		using sim::nav::WorldNav;
		using sim::nav::WorldNavData;

		// ---- Walking maths alone ------------------------------------------------------------------------------------

		struct HandPath
		{
			std::vector<glm::vec3> corners;
			std::vector<uint16_t> levels;
			PathView View() const { return { &corners, &levels }; }
		};

		// 30 m east, 40 m north (50 m diagonal leg), a portal jump (never walked), 20 m east in the next level.
		HandPath MakeHandPath()
		{
			HandPath path;
			path.corners = { { 0, 0, 0 }, { 30, 0, 0 }, { 30, 0, 40 }, { 500, 0, 500 }, { 520, 0, 500 } };
			path.levels = { 0, 0, 0, 1, 1 };
			return path;
		}

		void TestWalkMaths(TestContext& t)
		{
			t.Section("movement: walking maths");
			const HandPath path = MakeHandPath();
			const PathView view = path.View();
			const double length = WalkLength(view);
			t.Check(std::fabs(length - (30 + 40 + 20)) < 1e-9, "the portal jump is not part of the walking length (%.3f m)", length);

			// A speed whose event times fall between 0.1 s steps, so "before" and "after" an event are never a rounding coin toss.
			constexpr double Speed = 1.3, Delay = 1.0;
			const double expected = length / Speed + Delay;

			Progress whole;
			const AdvanceResult once = Advance(view, whole, 1000.0, Speed, Delay);
			t.Check(once.arrived && std::fabs(once.timeUsed - expected) < 1e-9 && once.crossings.size() == 1 && once.crossings[0] == 2,
				"one big step: arrives after exactly length / speed + portal delay (%.4f s, expected %.4f), one crossing at corner 2", once.timeUsed, expected);
			t.Check(std::fabs(once.walked - length) < 1e-9, "walked distance equals the walking length");

			Progress small;
			double used = 0.0;
			int crossings = 0, steps = 0;
			glm::vec3 worst(0.0f);
			float biggestGap = 0.0f;
			Progress reference;
			while (true)
			{
				const AdvanceResult r = Advance(view, small, 0.1, Speed, Delay);
				used += r.timeUsed;
				crossings += static_cast<int>(r.crossings.size());
				++steps;
				// The same instant reached in one go from the start must give the same place.
				Progress direct;
				Advance(view, direct, steps * 0.1, Speed, Delay);
				biggestGap = std::max(biggestGap, glm::distance(PositionOf(view, small), PositionOf(view, direct)));
				if (r.arrived)
					break;
			}
			t.Check(biggestGap < 1e-4f, "many small steps end where one big step ends, at every instant (worst gap %.2e m over %d steps)", biggestGap, steps);
			t.Check(std::fabs(used - expected) < 1e-6 && crossings == 1, "small steps use the same total time (%.6f vs %.6f s)", used, expected);

			// Positions along the way.
			Progress progress;
			Advance(view, progress, 30.0 / Speed, Speed, Delay);
			t.Check(glm::distance(PositionOf(view, progress), glm::vec3(30, 0, 0)) < 1e-4f, "at the first corner after 30 m");
			Progress atPortal;
			Advance(view, atPortal, 70.0 / Speed + 0.4, Speed, Delay);
			t.Check(atPortal.crossing && glm::distance(PositionOf(view, atPortal), glm::vec3(30, 0, 40)) < 1e-4f && LevelOf(view, atPortal) == 0,
				"half way through the portal delay the walker still stands at the portal start, in the old level");
			Advance(view, atPortal, 0.7, Speed, Delay);
			t.Check(!atPortal.crossing && LevelOf(view, atPortal) == 1 && glm::distance(PositionOf(view, atPortal), glm::vec3(500.1f, 0, 500)) < 0.1f,
				"after the delay it is at the landing (plus the 0.1 s walked since), in the new level");

			// A speed change in the middle: the stretch so far is walked at the old speed.
			Progress changed;
			Advance(view, changed, 20.0, Speed, Delay);
			const double walkedSoFar = 20.0 * Speed;
			const AdvanceResult rest = Advance(view, changed, 1000.0, 4.0, Delay);
			t.Check(rest.arrived && std::fabs(20.0 + rest.timeUsed - (walkedSoFar / Speed + (length - walkedSoFar) / 4.0 + Delay)) < 1e-9, "a speed change mid-path keeps the arrival time exact");

			// Degenerate paths.
			HandPath one;
			one.corners = { { 1, 2, 3 } };
			one.levels = { 0 };
			Progress none;
			t.Check(Advance(one.View(), none, 0.0, Speed, Delay).arrived && glm::distance(PositionOf(one.View(), none), glm::vec3(1, 2, 3)) < 1e-6f, "a one-corner path has arrived at once");
			HandPath standing;
			standing.corners = { { 1, 0, 1 }, { 1, 0, 1 } };
			standing.levels = { 0, 0 };
			Progress still;
			t.Check(Advance(standing.View(), still, 0.0, Speed, Delay).arrived, "a zero-length path arrives with no time spent");
			Progress frozen;
			t.Check(!Advance(view, frozen, 10.0, 0.0, Delay).arrived && frozen.leg == 0 && frozen.legDistance == 0.0, "zero speed never advances and never loops");
		}

		// ---- The rig: a baked chain of four levels, the module, a toy trip module --------------------------------------

		using Fixture = ChainWorld;

		// Test-only inputs that change a running walk. The trips themselves come from the shipped demo module.
		struct GaitWanted
		{
			static constexpr const char* TypeName = "GaitWanted";
			Entity walker = NullEntity;
			Gait gait = Gait::Run;
			template<class Ar> void Visit(Ar& ar) { ar("walker", walker); ar("gait", gait); }
		};

		struct StopWanted
		{
			static constexpr const char* TypeName = "StopWanted";
			Entity walker = NullEntity;
			template<class Ar> void Visit(Ar& ar) { ar("walker", walker); }
		};

		struct InputModule : Module
		{
			const char* Name() const override { return "test_inputs"; }

			void Register(Registration& reg) override
			{
				reg.Event<GaitWanted>(EventInput);
				reg.Event<StopWanted>(EventInput);
				reg.Rule<GaitWanted>("gait", [](RuleContext& ctx, const GaitWanted& event) { ctx.Command(SetGait{ event.walker, event.gait }); });
				reg.Rule<StopWanted>("stop", [](RuleContext& ctx, const StopWanted& event) { ctx.Command(StopWalk{ event.walker }); });
			}
		};

		struct Rig
		{
			std::unique_ptr<MovementModule> movement;
			std::unique_ptr<DemoModule> demo;
			InputModule inputs;
			std::shared_ptr<const TypeRegistry> types;
			std::unique_ptr<Server> server;
			std::vector<Entity> walkers;

			Rig(Fixture& fixture, bool commute, uint64_t seed)
			{
				movement = std::make_unique<MovementModule>(*fixture.nav);
				demo = std::make_unique<DemoModule>(*movement, fixture.points, commute);
				RegistryBuilder builder;
				builder.Add(*movement);
				builder.Add(*demo);
				builder.Add(inputs);
				std::vector<std::string> errors;
				types = builder.Build(errors);
				server = std::make_unique<Server>(types, seed);
			}

			World& GetWorld() { return server->GetWorld(); }

			void PlaceAt(const std::vector<glm::vec3>& positions)
			{
				server->RunSetup("walkers", [&](ApplyContext& ctx)
				{
					for (const glm::vec3& position : positions)
						walkers.push_back(movement->Place(ctx, position));
				});
			}

			void Player(uint16_t level, const glm::vec3& local) { server->QueueInput(PlayerMoved{ level, local }); }

			size_t Count(TypeId type) const
			{
				size_t count = 0;
				const World& world = server->GetWorld();
				for (size_t i = 0; i < world.Log().Size(); ++i)
					count += world.Log().At(i).type == type;
				return count;
			}
		};

		template<class E> std::vector<std::pair<Tick, E>> Collect(const Rig& rig)
		{
			std::vector<std::pair<Tick, E>> found;
			const World& world = rig.server->GetWorld();
			for (size_t i = 0; i < world.Log().Size(); ++i)
				if (const E* event = world.Log().At(i).As<E>(*rig.types))
					found.emplace_back(world.Log().At(i).tick, *event);
			return found;
		}

		// Steps `ticks` single ticks, failing on the first invariant failure. Returns false on a failure.
		bool Run(TestContext& t, Rig& rig, int ticks, const char* what)
		{
			rig.GetWorld().checkInvariants = true;
			for (int i = 0; i < ticks; ++i)
			{
				rig.server->Step();
				if (!rig.GetWorld().LastInvariantFailures().empty())
					return t.Check(false, "%s: invariant failed at tick %lld: %s", what, static_cast<long long>(rig.GetWorld().Now()), rig.GetWorld().LastInvariantFailures()[0].c_str());
			}
			return true;
		}

		// Every trip arrives when length / speed (+ portal delays) says, within a tick and a half.
		void CheckTripTimes(TestContext& t, Rig& rig, const char* what, int& trips, int& crossingTrips)
		{
			struct Start
			{
				Tick tick;
				float length;
				int portals;
			};
			entt::dense_map<uint32_t, Start> started;
			int bad = 0;
			trips = 0;
			crossingTrips = 0;
			const World& world = rig.GetWorld();
			const float delay = world.Tunable(rig.movement->portalDelay);
			for (size_t i = 0; i < world.Log().Size(); ++i)
			{
				const EventRecord& record = world.Log().At(i);
				if (const PathAssigned* assigned = record.As<PathAssigned>(*rig.types))
					started.insert_or_assign(EntityIndex(assigned->walker), Start{ record.tick, assigned->length, assigned->portals });
				else if (const Arrived* arrived = record.As<Arrived>(*rig.types))
				{
					const auto found = started.find(EntityIndex(arrived->walker));
					const Walker* walker = world.TryGet<Walker>(arrived->walker);
					if (found == started.end() || !walker)
					{
						++bad;
						continue;
					}
					const double seconds = found->second.length / (Speed(walker->moverClass, Gait::Walk, GroundArea) * walker->speedScale) + found->second.portals * delay;
					const double expected = found->second.tick + seconds * TicksPerSecond;
					if (std::fabs(arrived->at - expected) > 1.5)
						++bad;
					++trips;
					crossingTrips += found->second.portals > 0 ? 1 : 0;
				}
			}
			t.Check(bad == 0 && trips > 0, "%s: all %d arrivals happen at length / speed + portal delays within 1.5 ticks (%d off)", what, trips, bad);
		}

		// ---- Tests --------------------------------------------------------------------------------------------------

		void TestTiers(TestContext& t, Fixture& fixture)
		{
			t.Section("movement: tiers");
			// Points: 0 east0, 1 west0, 2 east1, 3 west1 (level, side).
			Rig rig(fixture, false, 3);
			rig.GetWorld().SetTunable(rig.movement->tier0Radius, 5.0f);
			rig.PlaceAt({ fixture.points[0], fixture.points[1], fixture.points[3] });
			const Entity nearPlayer = rig.walkers[0], farInLevel = rig.walkers[1], otherLevel = rig.walkers[2];
			const World& world = rig.GetWorld();
			const MovementModule& movement = *rig.movement;

			t.Check(movement.TierOf(world, nearPlayer) == Tier::T2, "without a player everyone is tier 2");
			rig.Player(0, fixture.chain.east[0]);
			rig.server->Step();
			t.Check(movement.TierOf(world, nearPlayer) == Tier::T0, "within the radius of the player: tier 0");
			t.Check(glm::distance(fixture.points[0], fixture.points[1]) > 5.0f && movement.TierOf(world, farInLevel) == Tier::T1, "beyond the radius, same level: tier 1");
			t.Check(movement.TierOf(world, otherLevel) == Tier::T2, "another level: tier 2");
			t.Check(movement.LevelOf(world, nearPlayer) == 0 && movement.LevelOf(world, otherLevel) == 1, "levels are derived from the mesh");

			rig.Player(1, fixture.chain.west[1]);
			rig.server->Step();
			t.Check(movement.TierOf(world, nearPlayer) == Tier::T2 && movement.TierOf(world, farInLevel) == Tier::T2 && movement.TierOf(world, otherLevel) == Tier::T0,
				"the tiers follow the player into another level");

			rig.Player(99, glm::vec3(0.0f));
			rig.server->Step();
			t.Check(rig.GetWorld().Stats().commandsRejected == 1, "a player report for an unknown level is rejected");

			// How often each tier integrates. The player stands at the east end of level 0; one walker crosses level 0
			// from the west (tier 1), one walks in level 1 (tier 2).
			Rig counting(fixture, false, 4);
			counting.GetWorld().SetTunable(counting.movement->tier0Radius, 1.0f);
			counting.PlaceAt({ fixture.points[1], fixture.points[3] });
			counting.Player(0, fixture.chain.east[0]);
			counting.server->QueueInput(TripWanted{ counting.walkers[0], 2 });
			counting.server->QueueInput(TripWanted{ counting.walkers[1], 2 });
			counting.server->Step();
			const Entity tier1 = counting.walkers[0], tier2 = counting.walkers[1];
			const bool ready = t.Check(counting.GetWorld().TryGet<Walker>(tier1)->state == Walker::State::Walking && counting.GetWorld().TryGet<Walker>(tier2)->state == Walker::State::Walking &&
				counting.movement->TierOf(counting.GetWorld(), tier1) == Tier::T1 && counting.movement->TierOf(counting.GetWorld(), tier2) == Tier::T2 &&
				glm::distance(fixture.points[1], fixture.points[0]) > 25.0f, "both walkers are under way: tier 1 and tier 2");
			if (ready)
			{
				int integrations1 = 0, integrations2 = 0;
				for (int i = 0; i < 100; ++i)
				{
					const Tick before1 = counting.GetWorld().TryGet<Walker>(tier1)->lastIntegrated, before2 = counting.GetWorld().TryGet<Walker>(tier2)->lastIntegrated;
					counting.server->Step();
					integrations1 += counting.GetWorld().TryGet<Walker>(tier1)->lastIntegrated != before1 ? 1 : 0;
					integrations2 += counting.GetWorld().TryGet<Walker>(tier2)->lastIntegrated != before2 ? 1 : 0;
				}
				t.Check(integrations1 == 50, "a tier 1 walker integrates every second tick (%d of 100)", integrations1);
				t.Check(integrations2 == 10, "a tier 2 walker integrates every tenth tick (%d of 100)", integrations2);
			}

			// A walker that crosses into the player's level changes tier on the way.
			Rig crossing(fixture, false, 5);
			crossing.GetWorld().SetTunable(crossing.movement->tier0Radius, 50.0f);
			crossing.PlaceAt({ fixture.points[0] });
			crossing.Player(1, fixture.chain.west[1]);
			crossing.server->QueueInput(TripWanted{ crossing.walkers[0], 3 });
			crossing.server->Step();
			const Tier before = crossing.movement->TierOf(crossing.GetWorld(), crossing.walkers[0]);
			Run(t, crossing, 400, "portal tier");
			const Tier after = crossing.movement->TierOf(crossing.GetWorld(), crossing.walkers[0]);
			t.Check(before == Tier::T2 && after != Tier::T2 && crossing.Count(crossing.types->EventId<PortalCrossed>()) == 1,
				"a walker crossing a portal into the player's level leaves tier 2 (tier %d before, %d after)", static_cast<int>(before), static_cast<int>(after));
		}

		void TestPathRequests(TestContext& t, Fixture& fixture)
		{
			t.Section("movement: path requests");
			{
				Rig rig(fixture, false, 6);
				rig.GetWorld().SetTunable(rig.movement->pathsPerTick, 3.0f);
				rig.PlaceAt(std::vector<glm::vec3>(8, fixture.points[0]));
				auto walking = [&]
				{
					int count = 0;
					for (const Entity e : rig.walkers)
						count += rig.GetWorld().TryGet<Walker>(e)->state == Walker::State::Walking ? 1 : 0;
					return count;
				};
				// Four ask in the first tick, two more in the second.
				for (const int i : { 4, 5, 6, 7 })
					rig.server->QueueInput(TripWanted{ rig.walkers[i], 1 });
				rig.server->Step();
				t.Check(walking() == 3, "three requests per step: the cap holds (%d walking)", walking());
				for (const int i : { 0, 1 })
					rig.server->QueueInput(TripWanted{ rig.walkers[i], 1 });
				rig.server->Step();
				t.Check(walking() == 6, "the next three are the oldest waiting one and the two new ones (%d walking)", walking());
				const auto assigned = Collect<PathAssigned>(rig);
				const std::vector<int> order = { 4, 5, 6, 7, 0, 1 };
				bool inOrder = assigned.size() == order.size();
				for (size_t i = 0; inOrder && i < order.size(); ++i)
					inOrder = assigned[i].second.walker == rig.walkers[static_cast<size_t>(order[i])];
				t.Check(inOrder, "results come in request order, oldest first, ties in storage order");
				t.Check(assigned.size() == 6 && assigned[0].second.length > 1.0f && assigned[0].second.corners >= 2, "PathAssigned reports the walking length and corners");
			}
			{
				Rig rig(fixture, false, 8);
				rig.PlaceAt({ fixture.points[0] });
				rig.server->RunSetup("destroy", [&](ApplyContext& ctx) { ctx.Destroy(rig.walkers[0]); });
				rig.server->QueueInput(TripWanted{ rig.walkers[0], 1 });
				rig.server->Step();
				t.Check(rig.GetWorld().Stats().commandsRejected == 1, "a walk for a destroyed walker is rejected, not applied");
			}
			{
				// A destination in the sky is off the mesh.
				std::vector<glm::vec3> list = fixture.points;
				list.push_back(fixture.points[0] + glm::vec3(0.0f, 500.0f, 0.0f));
				MovementModule movement(*fixture.nav);
				DemoModule demo(movement, list, false);
				RegistryBuilder builder;
				builder.Add(movement);
				builder.Add(demo);
				std::vector<std::string> errors;
				const auto types = builder.Build(errors);
				Server server(types, 9);
				Entity walker = NullEntity;
				server.RunSetup("place", [&](ApplyContext& ctx) { walker = movement.Place(ctx, fixture.points[0]); });
				server.QueueInput(TripWanted{ walker, 8 });
				server.Step();
				std::string reason;
				for (size_t i = 0; i < server.GetWorld().Log().Size(); ++i)
					if (const PathFailed* failed = server.GetWorld().Log().At(i).As<PathFailed>(*types))
						reason = failed->reason.str();
				const Walker* w = server.GetWorld().TryGet<Walker>(walker);
				t.Check(w && w->state == Walker::State::Idle && reason == "destination_off_mesh", "a destination off the mesh fails cleanly (%s)", reason.c_str());
			}
		}

		void TestUnreachable(TestContext& t, Fixture& fixture)
		{
			t.Section("movement: unreachable destination");
			// An island: a ground polygon of level 0 that no route from the main region reaches.
			OriginalMesh original;
			if (!original.Build(fixture.chain.levels[0].nav))
				return;
			const auto mainRegion = MainRegionPolys(original);
			glm::vec3 island(0.0f);
			bool found = false;
			const dtNavMesh* mesh = original.mesh;
			for (int i = 0; i < mesh->getMaxTiles() && !found; ++i)
			{
				const dtMeshTile* tile = mesh->getTile(i);
				if (!tile || !tile->header)
					continue;
				for (int p = 0; p < tile->header->polyCount && !found; ++p)
				{
					const dtPoly& poly = tile->polys[p];
					if (poly.getType() != DT_POLYTYPE_GROUND)
						continue;
					glm::vec3 centre(0.0f);
					for (int v = 0; v < poly.vertCount; ++v)
						centre += glm::vec3(tile->verts[poly.verts[v] * 3], tile->verts[poly.verts[v] * 3 + 1], tile->verts[poly.verts[v] * 3 + 2]);
					centre /= static_cast<float>(poly.vertCount);
					const NavPoint start = fixture.nav->Project(fixture.points[0]);
					const NavPoint end = fixture.nav->Project(fixture.nav->ToNav(0, centre), { 0.05f, 0.1f, 0.05f });
					if (end.IsValid() && start.IsValid() && !fixture.nav->FindPath(start, end).complete)
					{
						island = fixture.nav->ToNav(0, centre);
						found = true;
					}
				}
			}
			if (!found)
			{
				t.Note("no unreachable polygon in the first level of the chain: unreachable-destination check skipped");
				return;
			}
			std::vector<glm::vec3> list = fixture.points;
			list.push_back(island);
			MovementModule movement(*fixture.nav);
			DemoModule demo(movement, list, false);
			RegistryBuilder builder;
			builder.Add(movement);
			builder.Add(demo);
			std::vector<std::string> errors;
			auto types = builder.Build(errors);
			Server server(types, 10);
			Entity walker = NullEntity;
			server.RunSetup("place", [&](ApplyContext& ctx) { walker = movement.Place(ctx, fixture.points[0]); });
			server.QueueInput(TripWanted{ walker, 8 });
			server.Step();
			std::string reason;
			for (size_t i = 0; i < server.GetWorld().Log().Size(); ++i)
				if (const PathFailed* failed = server.GetWorld().Log().At(i).As<PathFailed>(*types))
					reason = failed->reason.str();
			const Walker* w = server.GetWorld().TryGet<Walker>(walker);
			t.Check(w && w->state == Walker::State::Idle && reason == "unreachable", "a destination no route reaches fails, nobody walks to the closest point (%s)", reason.c_str());
		}

		void TestSpeedAndStop(TestContext& t, Fixture& fixture)
		{
			t.Section("movement: gait, stop, extrapolation");
			// West to east in level 0, no portals.
			const std::vector<glm::vec3> route = { fixture.points[1], fixture.points[0] };
			{
				Rig rig(fixture, false, 11);
				rig.GetWorld().SetTunable(rig.movement->speedScaleSpread, 0.0f);
				rig.PlaceAt({ route[0] });
				rig.server->QueueInput(TripWanted{ rig.walkers[0], 0 });
				rig.server->Step();
				const Walker* walker = rig.GetWorld().TryGet<Walker>(rig.walkers[0]);
				const Tick started = rig.GetWorld().Now();
				const double length = walker->walkLength;
				t.Check(length > 25.0, "the route is long enough to change gait on (%.1f m)", length);
				Run(t, rig, 100, "gait");
				const Tick changeAt = rig.GetWorld().Now() + 1;
				rig.server->QueueInput(GaitWanted{ rig.walkers[0], Gait::Run });
				Run(t, rig, 3000, "gait");
				const auto arrived = Collect<Arrived>(rig);
				const double walked = Speed(MoverClass::Person, Gait::Walk, GroundArea) * TicksToSeconds(changeAt - started);
				const double expected = started + (changeAt - started) + (length - walked) / Speed(MoverClass::Person, Gait::Run, GroundArea) * TicksPerSecond;
				t.Check(arrived.size() == 1 && std::fabs(arrived[0].second.at - expected) <= 1.5, "switching to a run mid-walk: arrival at %.1f expected, got %lld (%.1f m route)",
					expected, arrived.empty() ? -1LL : static_cast<long long>(arrived[0].second.at), length);
			}
			{
				Rig rig(fixture, false, 12);
				rig.PlaceAt({ route[0] });
				rig.server->QueueInput(TripWanted{ rig.walkers[0], 0 });
				Run(t, rig, 150, "stop");
				const glm::vec3 expectedPlace = rig.movement->PositionAt(rig.GetWorld(), rig.walkers[0], rig.GetWorld().Now() + 1);
				rig.server->QueueInput(StopWanted{ rig.walkers[0] });
				Run(t, rig, 1, "stop");
				const Walker* walker = rig.GetWorld().TryGet<Walker>(rig.walkers[0]);
				const glm::vec3 where = rig.GetWorld().TryGet<Position>(rig.walkers[0])->nav;
				t.Check(walker->state == Walker::State::Idle && walker->corners.empty() && glm::distance(where, expectedPlace) < 1e-3f && glm::distance(where, route[0]) > 1.0f,
					"a stopped walker stays where it was extrapolated to, with no path (moved %.2f m)", glm::distance(where, route[0]));
				Run(t, rig, 2000, "stop");
				t.Check(rig.Count(rig.types->EventId<Arrived>()) == 0, "and never arrives");
			}
			{
				// A tier 2 walker stepped every 10 ticks is extrapolated to where a tick-by-tick walker really is.
				Rig fine(fixture, false, 13), coarse(fixture, false, 13);
				fine.GetWorld().SetTunable(fine.movement->tier2Step, 1.0f);
				for (Rig* rig : { &fine, &coarse })
				{
					rig->GetWorld().SetTunable(rig->movement->speedScaleSpread, 0.0f);
					rig->PlaceAt({ route[0] });
					rig->server->QueueInput(TripWanted{ rig->walkers[0], 0 });
				}
				float worst = 0.0f;
				for (int i = 0; i < 400; ++i)
				{
					fine.server->Step();
					coarse.server->Step();
					const glm::vec3 a = fine.GetWorld().TryGet<Position>(fine.walkers[0])->nav;
					const glm::vec3 b = coarse.movement->PositionAt(coarse.GetWorld(), coarse.walkers[0], coarse.GetWorld().Now());
					worst = std::max(worst, glm::distance(a, b));
				}
				t.Check(worst < 2e-3f, "PositionAt extrapolates a coarse walker to the fine walker's position (worst %.2e m)", worst);
			}
		}

		void TestDetector(TestContext& t, Fixture& fixture)
		{
			t.Section("movement: teleport detector");
			// Walkers standing still are never integrated, so what the test writes into them stays until the invariant looks.
			auto failures = [&](auto tamper, const char* expect)
			{
				Rig rig(fixture, false, 14);
				rig.PlaceAt({ fixture.points[1] });
				rig.server->Step();
				rig.GetWorld().checkInvariants = true;
				tamper(rig);
				rig.server->Step();
				const auto& found = rig.GetWorld().LastInvariantFailures();
				t.Check(!found.empty() && found[0].find(expect) != std::string::npos, "'%s' is caught: %s", expect, found.empty() ? "nothing reported" : found[0].c_str());
			};
			failures([](Rig& rig) { rig.GetWorld().TryGet<Position>(rig.walkers[0])->nav += glm::vec3(5.0f, 0.0f, 0.0f); }, "outside the movement module");
			failures([](Rig& rig)
			{
				Walker* walker = rig.GetWorld().TryGet<Walker>(rig.walkers[0]);
				walker->lastWalked = 50.0f;
				walker->lastTicks = 1;
			}, "walked");
			failures([](Rig& rig)
			{
				// Standing exactly where the module put it, but off the mesh, as if the module itself had gone wrong.
				Walker* walker = rig.GetWorld().TryGet<Walker>(rig.walkers[0]);
				walker->lastTo += glm::vec3(0.0f, 30.0f, 0.0f);
				rig.GetWorld().TryGet<Position>(rig.walkers[0])->nav = walker->lastTo;
				walker->lastTicks = 1;
				walker->lastIntegrated = rig.GetWorld().Now() + 1;
			}, "not on the nav mesh");
		}

		void TestCommute(TestContext& t, Fixture& fixture)
		{
			t.Section("movement: commute across four levels");
			constexpr int Walkers = 40;
			constexpr int Ticks = 18000; // 30 minutes of world time
			constexpr int Checked = 1500;
			auto place = [&](Rig& rig)
			{
				std::vector<glm::vec3> spots;
				for (int i = 0; i < Walkers; ++i)
					spots.push_back(fixture.points[static_cast<size_t>(i) % fixture.points.size()]);
				rig.PlaceAt(spots);
			};
			auto kick = [&](Rig& rig)
			{
				rig.Player(0, fixture.chain.east[0]);
				for (const Entity walker : rig.walkers)
					rig.server->QueueInput(TripWanted{ walker, -1 });
			};

			Rig rig(fixture, true, 1234);
			place(rig);
			rig.server->StartRecording(100);
			kick(rig);
			Stopwatch timer;
			if (!Run(t, rig, Ticks / 2, "commute"))
				return;
			const std::vector<uint8_t> midpoint = rig.GetWorld().Save(true);
			std::vector<uint64_t> hashes;
			for (int i = 0; i < Checked; ++i)
			{
				rig.server->Step();
				hashes.push_back(rig.GetWorld().StateHash());
			}
			if (!Run(t, rig, Ticks / 2 - Checked, "commute"))
				return;
			Recording recording = rig.server->StopRecording();
			const double seconds = timer.Seconds();

			int trips = 0, crossingTrips = 0;
			CheckTripTimes(t, rig, "commute", trips, crossingTrips);
			const size_t crossings = rig.Count(rig.types->EventId<PortalCrossed>());
			t.Check(crossingTrips > 10 && crossings >= static_cast<size_t>(crossingTrips), "walkers keep crossing levels: %d trips, %d across portals, %zu portal crossings", trips, crossingTrips, crossings);
			t.Check(rig.GetWorld().Stats().invariantFailures == 0, "no teleport-detector violation in %d ticks", Ticks);
			size_t placed = 0, portals = 0, other = 0;
			for (const auto& exception : Collect<PositionException>(rig))
			{
				const std::string reason = exception.second.reason.str();
				placed += reason == "placed";
				portals += reason == "portal";
				other += reason != "placed" && reason != "portal";
			}
			t.Check(placed == Walkers && portals == crossings && other == 0, "the only logged position exceptions are placements (%zu) and portal crossings (%zu)", placed, portals);
			t.Check(rig.Count(rig.types->EventId<PathFailed>()) == 0, "every request found a route");
			t.Note("commute: %d trips, %d across portals, %zu portal crossings", trips, crossingTrips, crossings);
			t.Note("commute: %d walkers, %d ticks, %.2f s real time (%.0f us per tick, invariants, hash and recording on)", Walkers, Ticks, seconds, seconds / Ticks * 1e6);

			// Same seed, same inputs: replay reproduces every checkpoint.
			bool identical = false;
			const std::string report = ReplayRecording(rig.types, recording, identical);
			t.Check(identical, "replay reproduces every checkpoint of the commute:\n%s", report.c_str());

			// Saved in the middle of the walk and continued, the world matches the uninterrupted one.
			Rig loaded(fixture, true, 99);
			std::string error;
			const bool ok = loaded.server->Load(midpoint.data(), midpoint.size(), error);
			t.Check(ok, "the midpoint snapshot loads: %s", error.c_str());
			if (ok)
			{
				bool same = true;
				for (int i = 0; i < Checked && same; ++i)
				{
					loaded.server->Step();
					same = loaded.GetWorld().StateHash() == hashes[static_cast<size_t>(i)];
				}
				t.Check(same, "saved mid-walk and continued, the world matches the uninterrupted one for %d ticks", Checked);
			}
		}

		// The same trips stepped in blocks of 1, 10 and 600 ticks arrive at the same ticks.
		void TestFastForward(TestContext& t, Fixture& fixture)
		{
			t.Section("movement: fast-forward steps");
			std::vector<std::vector<std::pair<Tick, Entity>>> arrivals;
			for (const Tick block : { 1, 10, 600 })
			{
				Rig rig(fixture, false, 21);
				rig.GetWorld().SetTunable(rig.movement->tier2Step, 10.0f);
				std::vector<glm::vec3> spots;
				for (int i = 0; i < 12; ++i)
					spots.push_back(fixture.points[static_cast<size_t>(i) % fixture.points.size()]);
				rig.PlaceAt(spots);
				for (size_t i = 0; i < rig.walkers.size(); ++i)
					rig.server->QueueInput(TripWanted{ rig.walkers[i], static_cast<int32_t>((i * 5 + 3) % fixture.points.size()) });
				rig.GetWorld().checkInvariants = true;
				rig.server->Step(); // requests are solved at tick 1 in every run
				for (int done = 1; done < 12000; done += static_cast<int>(block))
					rig.server->Step(block);
				t.Check(rig.GetWorld().Stats().invariantFailures == 0, "blocks of %lld ticks: no invariant failure", static_cast<long long>(block));
				std::vector<std::pair<Tick, Entity>> list;
				for (const auto& arrived : Collect<Arrived>(rig))
					list.emplace_back(arrived.second.at, arrived.second.walker);
				std::sort(list.begin(), list.end(), [](const auto& a, const auto& b) { return a.second != b.second ? EntityIndex(a.second) < EntityIndex(b.second) : a.first < b.first; });
				arrivals.push_back(std::move(list));
			}
			bool same = arrivals[0].size() >= 10 && arrivals[0].size() == arrivals[1].size() && arrivals[0].size() == arrivals[2].size();
			Tick worst = 0;
			for (size_t i = 0; same && i < arrivals[0].size(); ++i)
			{
				same = arrivals[0][i].second == arrivals[1][i].second && arrivals[0][i].second == arrivals[2][i].second;
				worst = std::max({ worst, std::abs(arrivals[0][i].first - arrivals[1][i].first), std::abs(arrivals[0][i].first - arrivals[2][i].first) });
			}
			t.Check(same && worst <= 1, "%zu arrivals at the same ticks whatever the step size (worst difference %lld ticks)", arrivals[0].size(), static_cast<long long>(worst));
		}

		void TestCost(TestContext& t, Fixture& fixture)
		{
			t.Section("movement: cost");
			Rig rig(fixture, true, 77);
			constexpr int Walkers = 400, Ticks = 1200;
			std::vector<glm::vec3> spots;
			for (int i = 0; i < Walkers; ++i)
				spots.push_back(fixture.points[static_cast<size_t>(i) % fixture.points.size()]);
			rig.PlaceAt(spots);
			rig.Player(0, fixture.chain.east[0]);
			for (const Entity walker : rig.walkers)
				rig.server->QueueInput(TripWanted{ walker, -1 });
			Stopwatch timer;
			for (int i = 0; i < Ticks; ++i)
				rig.server->Step();
			const double seconds = timer.Seconds();
			t.Note("%d walkers, %d ticks: %.0f us per tick (%.2f us per walker per tick), %zu arrivals", Walkers, Ticks, seconds / Ticks * 1e6, seconds / Ticks / Walkers * 1e6,
				rig.Count(rig.types->EventId<Arrived>()));
		}
	}

	void RunMovementTests(TestContext& context)
	{
		TestWalkMaths(context);
		Fixture fixture;
		if (!fixture.Build(context))
			return;
		TestTiers(context, fixture);
		TestPathRequests(context, fixture);
		TestUnreachable(context, fixture);
		TestSpeedAndStop(context, fixture);
		TestDetector(context, fixture);
		TestFastForward(context, fixture);
		TestCommute(context, fixture);
		TestCost(context, fixture);
	}
}

#endif
