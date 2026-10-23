// Bodies tests (SLICE_M2C_BODIES.md section 7): the sim's side of materialization against a scripted stand-in for the game.
// FakeGame answers proposals, walks its bodies in straight lines at a chosen fraction of the agent's speed and reports where
// they stand, the way the bridge does with real SimNpc entities.

#include "SimNavFixtures.h"

#ifndef DISTRIBUTION

#include <Sim/Bodies/SimBodies.h>
#include <Sim/Bridge/SimClient.h>
#include <Sim/Movement/SimDemoPopulation.h>

#include <entt/container/dense_map.hpp>
#include <glm/geometric.hpp>

#include <cmath>
#include <memory>

namespace sim::test
{
	namespace
	{
		using namespace sim::movement;
		using namespace sim::bodies;

		struct FakeBody
		{
			Entity entity = NullEntity;
			glm::vec3 local{ 0.0f };
			glm::vec3 goal{ 0.0f };
			bool hasGoal = false;
			glm::vec3 next{ 0.0f };
			bool hasNext = false;
			Gait gait = Gait::Walk;
			float speedScale = 1.0f;
			int sinceReport = 0;
			bool despawning = false;
		};

		struct Rig
		{
			ChainWorld& world;
			std::unique_ptr<MovementModule> movement;
			std::unique_ptr<BodiesModule> bodies;
			std::unique_ptr<DemoModule> demo;
			std::shared_ptr<const TypeRegistry> types;
			std::unique_ptr<SimClient> client;

			// The game.
			bool accept = true;              // false: refuse every proposal
			bool answer = true;              // false: never answer a proposal
			bool report = true;              // false: bodies never report
			bool release = true;             // false: never release on a despawn proposal
			float speed = 1.0f;              // fraction of the agent's speed the bodies manage
			entt::dense_map<uint32_t, FakeBody> bodiesOfGame;
			size_t proposals = 0, despawns = 0, goals = 0;
			size_t mostEmbodied = 0;

			Rig(ChainWorld& world, bool commute, uint64_t seed = 1) : world(world)
			{
				movement = std::make_unique<MovementModule>(*world.nav);
				bodies = std::make_unique<BodiesModule>(*movement, *world.nav);
				demo = std::make_unique<DemoModule>(*movement, world.points, commute);
				RegistryBuilder builder;
				builder.Add(*movement);
				builder.Add(*bodies);
				builder.Add(*demo);
				std::vector<std::string> errors;
				types = builder.Build(errors);
				SimClient::Setup setup;
				setup.types = types;
				setup.worldNav = world.nav.get();
				setup.movement = movement.get();
				setup.seed = seed;
				client = std::make_unique<SimClient>(setup);
				client->Start(Host::Mode::Pump);
				GetWorld().checkInvariants = true;
				GetWorld().SetTunable(movement->speedScaleSpread, 0.0f);
				GetWorld().SetTunable(bodies->spawnRadius, 400.0f);
				GetWorld().SetTunable(bodies->despawnRadius, 450.0f);
			}

			World& GetWorld() { return client->ServerForSetup().GetWorld(); }

			Entity Place(const glm::vec3& nav)
			{
				Entity placed = NullEntity;
				client->ServerForSetup().RunSetup("place", [&](ApplyContext& ctx) { placed = movement->Place(ctx, nav); });
				return placed;
			}

			void Walk(Entity walker, const glm::vec3& destination)
			{
				client->Send(TripWanted{ walker, DestinationIndex(destination) });
			}

			int32_t DestinationIndex(const glm::vec3& nav) const
			{
				for (size_t i = 0; i < world.points.size(); ++i)
					if (glm::distance(world.points[i], nav) < 1e-3f)
						return static_cast<int32_t>(i);
				return 0;
			}

			// One sim tick of world time, then the game's turn.
			void Step()
			{
				client->Update(0.1);
				GameTurn();
				mostEmbodied = std::max(mostEmbodied, BodiesModule::EmbodiedCount(GetWorld()));
			}

			void Run(double seconds)
			{
				for (int i = 0; i < static_cast<int>(seconds * 10.0 + 0.5); ++i)
					Step();
			}

			void GameTurn()
			{
				const TypeRegistry& registry = *types;
				for (Message& message : client->TakeOutputs())
				{
					if (const auto* proposal = message.As<BodySpawnProposed>(registry))
					{
						++proposals;
						if (!answer)
							continue;
						if (!accept)
						{
							client->Send(BodyRefused{ proposal->entity });
							continue;
						}
						FakeBody body;
						body.entity = proposal->entity;
						body.local = proposal->local;
						body.speedScale = proposal->speedScale;
						bodiesOfGame[entt::to_integral(body.entity)] = body;
						client->Send(BodyAccepted{ proposal->entity, body.local });
					}
					else if (const auto* despawn = message.As<BodyDespawnProposed>(registry))
					{
						++despawns;
						auto found = bodiesOfGame.find(entt::to_integral(despawn->entity));
						if (found != bodiesOfGame.end() && release)
						{
							client->Send(BodyReleased{ despawn->entity, found->second.local });
							bodiesOfGame.erase(found);
						}
						else if (found == bodiesOfGame.end())
							client->Send(BodyReleased{ despawn->entity, glm::vec3(0.0f) });
					}
					else if (const auto* goal = message.As<BodyGoal>(registry))
					{
						++goals;
						auto found = bodiesOfGame.find(entt::to_integral(goal->entity));
						if (found == bodiesOfGame.end())
							continue;
						found->second.hasGoal = goal->kind == BodyGoal::Kind::Move;
						found->second.goal = goal->local;
						found->second.next = goal->next;
						found->second.hasNext = goal->hasNext;
						found->second.gait = goal->gait;
						found->second.speedScale = goal->speedScale;
					}
				}
				for (auto&& entry : bodiesOfGame)
				{
					FakeBody& body = entry.second;
					if (body.hasGoal)
					{
						const float step = Speed(MoverClass::Person, body.gait, GroundArea) * body.speedScale * speed * 0.1f;
						const glm::vec3 delta = body.goal - body.local;
						const float distance = glm::length(delta);
						body.local = distance <= step ? body.goal : body.local + delta / distance * step;
						if (distance <= step && body.hasNext)
						{
							body.goal = body.next; // on to the next corner without waiting for the sim
							body.hasNext = false;
						}
					}
					if (report && ++body.sinceReport >= 2)
					{
						body.sinceReport = 0;
						client->Send(BodyReport{ body.entity, body.local });
					}
				}
			}

			const Embodied* BodyOf(Entity entity) { return GetWorld().TryGet<Embodied>(entity); }
			const Walker* WalkerOf(Entity entity) { return GetWorld().TryGet<Walker>(entity); }

			size_t Count(TypeId type)
			{
				size_t count = 0;
				for (size_t i = 0; i < GetWorld().Log().Size(); ++i)
					count += GetWorld().Log().At(i).type == type;
				return count;
			}

			template<class E> std::vector<std::pair<Tick, E>> Events()
			{
				std::vector<std::pair<Tick, E>> found;
				for (size_t i = 0; i < GetWorld().Log().Size(); ++i)
					if (const E* event = GetWorld().Log().At(i).As<E>(*types))
						found.emplace_back(GetWorld().Log().At(i).tick, *event);
				return found;
			}

			bool Clean() { return GetWorld().Stats().invariantFailures == 0; }
		};

		void TestProposals(TestContext& t, ChainWorld& world)
		{
			t.Section("bodies: proposals");
			Rig rig(world, false, 3);
			rig.GetWorld().SetTunable(rig.bodies->maxBodies, 6.0f);
			// 12 agents at the east end of level 0 (where the player stands) walking west, 12 at the west end walking east.
			rig.client->ServerForSetup().RunSetup("walkers", [&](ApplyContext& ctx)
			{
				for (int i = 0; i < 24; ++i)
				{
					const bool nearPlayer = i < 12;
					const Entity walker = rig.movement->Place(ctx, world.points[nearPlayer ? 0 : 1]);
					ctx.Emit(TripWanted{ walker, nearPlayer ? 1 : 0 });
				}
			});
			rig.client->ReportPlayer(0, world.chain.east[0]);
			rig.Run(0.5);
			t.Check(rig.proposals == 6 && BodiesModule::EmbodiedCount(rig.GetWorld()) == 6, "with a budget of 6 the sim proposes exactly 6 bodies (%zu proposals)", rig.proposals);

			// Only walkers of the player's level, nearest first.
			int wrongLevel = 0;
			float farthest = 0.0f, nearestLeft = 1e30f;
			const glm::vec3 playerNav = world.points[0];
			for (const auto [entity, body] : rig.GetWorld().Registry().view<const Embodied>().each())
			{
				wrongLevel += rig.movement->LevelOf(rig.GetWorld(), entity) != 0 ? 1 : 0;
				farthest = std::max(farthest, glm::distance(rig.GetWorld().TryGet<Position>(entity)->nav, playerNav));
			}
			for (const auto [entity, walker, position] : rig.GetWorld().Registry().view<const Walker, const Position>().each())
				if (!rig.BodyOf(entity) && rig.movement->LevelOf(rig.GetWorld(), entity) == 0)
					nearestLeft = std::min(nearestLeft, glm::distance(position.nav, playerNav));
			t.Check(wrongLevel == 0, "all embodied agents are in the player's level");
			t.Check(farthest <= nearestLeft + 1.0f, "the nearest agents get the bodies (farthest embodied %.1f m, nearest without %.1f m)", farthest, nearestLeft);

			rig.Run(30.0);
			t.Check(rig.mostEmbodied <= 6, "the budget holds for 30 s (at most %zu bodies)", rig.mostEmbodied);
			t.Check(rig.Clean(), "no detector violation with bodies driving agents");

			// Hysteresis: radius 3 m spawns nobody; widening it spawns again.
			Rig tight(world, false, 4);
			tight.GetWorld().SetTunable(tight.bodies->spawnRadius, 3.0f);
			tight.GetWorld().SetTunable(tight.bodies->despawnRadius, 5.0f);
			tight.Place(world.points[1]);
			tight.client->ReportPlayer(0, world.chain.east[0]);
			tight.Run(1.0);
			t.Check(tight.proposals == 0, "an agent outside the spawn radius gets no body");
			tight.GetWorld().SetTunable(tight.bodies->spawnRadius, 200.0f);
			tight.GetWorld().SetTunable(tight.bodies->despawnRadius, 250.0f);
			tight.Run(1.0);
			t.Check(tight.proposals == 1, "inside the spawn radius it does");
			tight.GetWorld().SetTunable(tight.bodies->spawnRadius, 10.0f);
			tight.GetWorld().SetTunable(tight.bodies->despawnRadius, 20.0f);
			tight.Run(1.0);
			t.Check(tight.despawns == 1 && BodiesModule::EmbodiedCount(tight.GetWorld()) == 0, "outside the despawn radius the body is taken back (%zu despawns)", tight.despawns);
		}

		// The body, not the clock, decides when the agent arrives.
		void TestBodyDrives(TestContext& t, ChainWorld& world)
		{
			t.Section("bodies: the body is the mover");
			auto arrivalAt = [&](float fraction, bool bodies, double& lengthOut, Tick& startOut) -> Tick
			{
				Rig rig(world, false, 5);
				rig.speed = fraction;
				if (!bodies)
					rig.GetWorld().SetTunable(rig.bodies->maxBodies, 0.0f);
				const Entity walker = rig.Place(world.points[1]);
				rig.client->ReportPlayer(0, world.chain.east[0]);
				rig.Walk(walker, world.points[0]);
				rig.Run(200.0);
				const auto assigned = rig.Events<PathAssigned>();
				const auto arrived = rig.Events<Arrived>();
				if (assigned.empty() || arrived.empty() || !rig.Clean())
					return -1;
				lengthOut = assigned[0].second.length;
				startOut = assigned[0].first;
				if (bodies)
				{
					const Embodied* body = rig.BodyOf(walker);
					(void)body;
				}
				return arrived[0].second.at;
			};
			double length = 0.0;
			Tick start = 0;
			double lengthGhost = 0.0;
			Tick startGhost = 0;
			const Tick ghost = arrivalAt(1.0f, false, lengthGhost, startGhost);
			const Tick full = arrivalAt(1.0f, true, length, start);
			const Tick slow = arrivalAt(0.5f, true, length, start);
			t.Check(ghost > 0 && full > 0 && slow > 0, "the walks arrive (ghost tick %lld, body at full speed %lld, body at half speed %lld)", static_cast<long long>(ghost), static_cast<long long>(full), static_cast<long long>(slow));
			if (ghost <= 0 || full <= 0 || slow <= 0)
				return;
			const double speed = Speed(MoverClass::Person, Gait::Walk, GroundArea);
			const double expectedSlow = start + (length - 0.75) / (speed * 0.5) * TicksPerSecond;
			t.Check(std::llabs(full - ghost) <= 15, "a body walking at the agent's speed arrives with the ghost within 1.5 s (%lld vs %lld ticks)", static_cast<long long>(full), static_cast<long long>(ghost));
			t.Check(slow > ghost + 100 && std::fabs(static_cast<double>(slow) - expectedSlow) <= 25.0, "a body at half speed makes the agent arrive when the body does (tick %lld, expected about %.0f, ghost %lld)",
				static_cast<long long>(slow), expectedSlow, static_cast<long long>(ghost));
		}

		void TestGoalsAndReplan(TestContext& t, ChainWorld& world)
		{
			t.Section("bodies: goals, re-planning, giving up");
			{
				Rig rig(world, false, 6);
				const Entity walker = rig.Place(world.points[1]);
				rig.client->ReportPlayer(0, world.chain.east[0]);
				rig.Walk(walker, world.points[0]);
				rig.Run(3.0);
				const Walker* w = rig.WalkerOf(walker);
				t.Check(rig.goals >= 1 && w && w->state == Walker::State::Walking && rig.BodyOf(walker) && rig.BodyOf(walker)->state == Embodied::State::Active, "the sim names the first corner as the body's goal (%zu goals)", rig.goals);
				rig.Run(200.0);
				t.Check(rig.Count(rig.types->EventId<Arrived>()) == 1 && rig.WalkerOf(walker)->state == Walker::State::Idle, "the walk ends with an arrival");
				t.Check(!rig.bodiesOfGame.empty() && !rig.bodiesOfGame.begin()->second.hasGoal, "an arrived agent's body is told to stand still");
			}
			{
				// The body wanders off the route: the sim plans again from where it stands.
				Rig rig(world, false, 7);
				const Entity walker = rig.Place(world.points[1]);
				rig.client->ReportPlayer(0, world.chain.east[0]);
				rig.Walk(walker, world.points[0]);
				rig.Run(5.0);
				const size_t plans = rig.Count(rig.types->EventId<PathAssigned>());
				for (auto&& entry : rig.bodiesOfGame)
				{
					glm::vec3 along = entry.second.goal - entry.second.local;
					along.y = 0.0f;
					const glm::vec3 aside = glm::normalize(glm::vec3(-along.z, 0.0f, along.x) + glm::vec3(1e-6f));
					entry.second.local += aside * 6.0f; // 6 m to the side of its route
				}
				rig.Run(1.0);
				t.Check(rig.Count(rig.types->EventId<PathAssigned>()) > plans || rig.Count(rig.types->EventId<PathFailed>()) > 0, "a body 6 m off its route makes the sim plan again");
				rig.Run(200.0);
				t.Check(rig.Clean(), "and the detector stays quiet");
			}
			{
				// Nobody answers: the proposal lapses and the agent walks on by itself.
				Rig rig(world, false, 8);
				rig.answer = false;
				const Entity walker = rig.Place(world.points[1]);
				rig.client->ReportPlayer(0, world.chain.east[0]);
				rig.Walk(walker, world.points[0]);
				rig.Run(1.0);
				t.Check(rig.BodyOf(walker) && rig.BodyOf(walker)->state == Embodied::State::Proposed, "a proposal waits for an answer");
				rig.Run(4.0);
				t.Check(!rig.BodyOf(walker) && rig.proposals == 1, "after the timeout the proposal is dropped (%zu proposals)", rig.proposals);
				rig.Run(200.0);
				t.Check(rig.Count(rig.types->EventId<Arrived>()) == 1, "and the agent still arrives, walked by the sim");
			}
			{
				// A refusal puts the agent on a cool-down.
				Rig rig(world, false, 9);
				rig.accept = false;
				rig.Place(world.points[1]);
				rig.client->ReportPlayer(0, world.chain.east[0]);
				rig.Run(4.0);
				const size_t early = rig.proposals;
				rig.Run(12.0);
				t.Check(early == 1 && rig.proposals >= 2 && rig.proposals <= 4, "a refused agent is proposed again only after the cool-down (%zu, then %zu proposals)", early, rig.proposals);
			}
			{
				// A body that stops reporting is given up.
				Rig rig(world, false, 10);
				const Entity walker = rig.Place(world.points[1]);
				rig.client->ReportPlayer(0, world.chain.east[0]);
				rig.Walk(walker, world.points[0]);
				rig.Run(3.0);
				rig.report = false;
				rig.Run(5.0);
				t.Check(rig.WalkerOf(walker) && rig.WalkerOf(walker)->state == Walker::State::Walking, "the agent is still walking");
				const Embodied* body = rig.BodyOf(walker);
				t.Check(!body || body->state != Embodied::State::Active, "a body silent for 3 s is given up");
				rig.Run(200.0);
				t.Check(rig.Count(rig.types->EventId<Arrived>()) == 1 && rig.Clean(), "the agent walks on and arrives");
			}
		}

		void TestRelease(TestContext& t, ChainWorld& world)
		{
			t.Section("bodies: release, portals, load");
			{
				// The player leaves the level: bodies are taken back.
				Rig rig(world, false, 11);
				const Entity walker = rig.Place(world.points[1]);
				rig.client->ReportPlayer(0, world.chain.east[0]);
				rig.Run(1.0);
				t.Check(rig.BodyOf(walker) && rig.BodyOf(walker)->state == Embodied::State::Active, "an agent near the player has an active body");
				rig.client->ReportPlayer(2, world.chain.east[2]);
				rig.Run(1.0);
				t.Check(!rig.BodyOf(walker) && rig.despawns >= 1 && rig.bodiesOfGame.empty(), "when the player moves to another level the body is released");

				// A game that never releases: the sim takes the body back after the release timeout.
				Rig stubborn(world, false, 12);
				stubborn.release = false;
				const Entity other = stubborn.Place(world.points[1]);
				stubborn.client->ReportPlayer(0, world.chain.east[0]);
				stubborn.Run(1.0);
				stubborn.client->ReportPlayer(2, world.chain.east[2]);
				stubborn.Run(8.0);
				t.Check(!stubborn.BodyOf(other), "a body the game does not release is taken back after the timeout");
			}
			{
				// A route over a portal: the body drives up to the portal, then the sim crosses it.
				Rig rig(world, false, 13);
				const Entity walker = rig.Place(world.points[1]);
				rig.client->ReportPlayer(0, world.chain.east[0]);
				rig.Walk(walker, world.points[3]); // west of level 1, through the portal at the east end of level 0
				rig.Run(300.0);
				t.Check(rig.Count(rig.types->EventId<PortalCrossed>()) == 1 && rig.Count(rig.types->EventId<Arrived>()) == 1, "an agent with a body crosses a portal and arrives (%zu crossings)", rig.Count(rig.types->EventId<PortalCrossed>()));
				t.Check(rig.movement->LevelOf(rig.GetWorld(), walker) == 1 && !rig.BodyOf(walker) && rig.bodiesOfGame.empty(), "it ends in the other level, its body released");
				t.Check(rig.Clean(), "no detector violation across the handoff");
			}
			{
				// A stray report for an agent the sim does not know.
				Rig rig(world, false, 14);
				rig.client->ReportPlayer(0, world.chain.east[0]);
				rig.client->Send(BodyReport{ static_cast<Entity>(5), world.chain.east[0] });
				const size_t before = rig.despawns;
				rig.Run(0.5);
				t.Check(rig.despawns > before, "a body the sim knows nothing about is told to go");
			}
			{
				// Save with bodies out, load: the game has none.
				Rig rig(world, true, 15);
				rig.client->ServerForSetup().RunSetup("walkers", [&](ApplyContext& ctx)
				{
					for (int i = 0; i < 8; ++i)
						ctx.Emit(TripWanted{ rig.movement->Place(ctx, world.points[static_cast<size_t>(i)]), -1 });
				});
				rig.client->ReportPlayer(0, world.chain.east[0]);
				rig.Run(10.0);
				const size_t embodied = BodiesModule::EmbodiedCount(rig.GetWorld());
				std::vector<uint8_t> blob;
				std::string error;
				rig.client->SaveBlocking(blob, error);
				Rig loaded(world, true, 99);
				const bool ok = loaded.client->LoadBlocking(blob, error);
				t.Check(ok && embodied > 0 && BodiesModule::EmbodiedCount(loaded.GetWorld()) == 0, "%zu bodies were out at the save; a loaded sim has none (%s)", embodied, error.c_str());
				loaded.Run(60.0);
				t.Check(loaded.Count(loaded.types->EventId<Arrived>()) > 0 && loaded.Clean(), "its agents walk on from the saved progress");
			}
		}

		void TestBodiesDeterminism(TestContext& t, ChainWorld& world)
		{
			t.Section("bodies: replay");
			Rig rig(world, true, 16);
			rig.client->ServerForSetup().RunSetup("walkers", [&](ApplyContext& ctx)
			{
				for (int i = 0; i < 16; ++i)
					ctx.Emit(TripWanted{ rig.movement->Place(ctx, world.points[static_cast<size_t>(i) % world.points.size()]), -1 });
			});
			rig.GetWorld().SetTunable(rig.bodies->maxBodies, 8.0f);
			rig.client->ServerForSetup().StartRecording(50);
			rig.client->ReportPlayer(0, world.chain.east[0]);
			rig.speed = 0.8f;
			rig.Run(300.0);
			Recording recording = rig.client->ServerForSetup().StopRecording();
			t.Check(rig.Clean() && rig.mostEmbodied > 0, "a 2 minute run with %zu bodies at most and body reports as inputs is clean", rig.mostEmbodied);
			bool identical = false;
			const std::string report = ReplayRecording(rig.types, recording, identical);
			t.Check(identical, "replaying it, body reports included, reproduces every checkpoint:\n%s", report.c_str());
		}
	}

	void RunBodiesTests(TestContext& context)
	{
		ChainWorld world;
		if (!world.Build(context))
			return;
		TestProposals(context, world);
		TestBodyDrives(context, world);
		TestGoalsAndReplan(context, world);
		TestRelease(context, world);
		TestBodiesDeterminism(context, world);
	}
}

#endif
