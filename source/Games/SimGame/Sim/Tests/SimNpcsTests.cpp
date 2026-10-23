// Authored NPC tests (SLICE_M2D_NPCS.md): the routine behaviour on the chain world, the content file parser, and the shipped
// GameData/sim/npcs.json against the shipped GameData/sim/world.nav.

#include "SimNavFixtures.h"

#ifndef DISTRIBUTION

#include <Sim/Bodies/SimBodies.h>
#include <Sim/Bridge/SimClient.h>
#include <Sim/Bridge/SimNpcFile.h>
#include <Sim/Bridge/SimWorldNavFile.h>
#include <Sim/Movement/SimDemoPopulation.h>
#include <Sim/Npcs/SimNpcs.h>

#include <glm/geometric.hpp>

#include <cmath>
#include <memory>

namespace sim::test
{
	namespace
	{
		using namespace sim::movement;
		using namespace sim::npcs;
		using sim::nav::WorldNav;
		using sim::nav::WorldNavData;

		struct Rig
		{
			ChainWorld& world;
			std::unique_ptr<MovementModule> movement;
			std::unique_ptr<bodies::BodiesModule> bodies;
			std::unique_ptr<NpcModule> npcs;
			std::unique_ptr<DemoModule> demo;
			std::shared_ptr<const TypeRegistry> types;
			std::unique_ptr<SimClient> client;

			explicit Rig(ChainWorld& world, uint64_t seed = 1) : world(world)
			{
				movement = std::make_unique<MovementModule>(*world.nav);
				bodies = std::make_unique<bodies::BodiesModule>(*movement, *world.nav);
				npcs = std::make_unique<NpcModule>(*movement, *world.nav);
				demo = std::make_unique<DemoModule>(*movement, world.points, true); // the crowd rules are on: they must leave NPCs alone
				RegistryBuilder builder;
				builder.Add(*movement);
				builder.Add(*bodies);
				builder.Add(*npcs);
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
			}

			World& GetWorld() { return client->ServerForSetup().GetWorld(); }

			Entity Place(const NpcDefinition& definition)
			{
				Entity placed = NullEntity;
				client->ServerForSetup().RunSetup("npc", [&](ApplyContext& ctx) { placed = npcs->Place(ctx, definition); });
				return placed;
			}

			void Run(double seconds)
			{
				for (int i = 0; i < static_cast<int>(seconds * 10.0 + 0.5); ++i)
					client->Update(0.1);
			}

			bool Clean() { return GetWorld().Stats().invariantFailures == 0; }
		};

		StopDefinition Stop(const ChainWorld& world, size_t level, bool east, float waitMin, float waitMax)
		{
			return { world.chain.levels[level].name, east ? world.chain.east[level] : world.chain.west[level], waitMin, waitMax };
		}

		// A spot half way along the walk from the west end to the east end (on the mesh by construction, unlike the straight line's middle).
		StopDefinition Between(const ChainWorld& world, size_t level, float waitMin, float waitMax)
		{
			const int index = static_cast<int>(level);
			const sim::nav::NavPath path = world.nav->FindPath(world.nav->Project(world.nav->ToNav(index, world.chain.west[level])), world.nav->Project(world.nav->ToNav(index, world.chain.east[level])));
			float walked = 0.0f;
			glm::vec3 spot = path.points.empty() ? glm::vec3(0.0f) : path.points.front();
			for (size_t i = 1; i < path.points.size(); ++i)
			{
				const float leg = glm::distance(path.points[i - 1], path.points[i]);
				if (walked + leg >= path.length * 0.5f)
				{
					spot = path.points[i - 1] + (path.points[i] - path.points[i - 1]) * ((path.length * 0.5f - walked) / std::max(leg, 1e-3f));
					break;
				}
				walked += leg;
			}
			return { world.chain.levels[level].name, world.nav->ToLocal(index, spot), waitMin, waitMax };
		}

		// What an NPC did, seen from outside: where it stood each time it arrived somewhere, and how long it stayed.
		struct Visit
		{
			size_t stop = 0;       // nearest stop of the definition
			double stayed = 0.0;   // seconds between arriving and setting off again
			bool left = false;
		};

		std::vector<Visit> Watch(Rig& rig, Entity npc, const NpcDefinition& definition, double seconds)
		{
			std::vector<Visit> visits;
			Walker::State previous = Walker::State::Idle;
			Tick arrivedAt = 0;
			bool standing = true; // placed at stop 0
			Tick standingSince = rig.GetWorld().Now();
			(void)previous;
			size_t stop = 0;
			for (int i = 0; i < static_cast<int>(seconds * 10.0); ++i)
			{
				rig.client->Update(0.1);
				const Walker* walker = rig.GetWorld().TryGet<Walker>(npc);
				if (!walker)
					break;
				const bool nowStanding = walker->state == Walker::State::Idle;
				if (nowStanding && !standing)
				{
					standing = true;
					standingSince = rig.GetWorld().Now();
					// Which stop is it at?
					const glm::vec3 where = rig.GetWorld().TryGet<Position>(npc)->nav;
					float best = 1e30f;
					for (size_t s = 0; s < definition.stops.size(); ++s)
					{
						const int level = rig.world.nav->Data().FindLevel(definition.stops[s].level);
						const float distance = glm::distance(where, rig.world.nav->ToNav(level, definition.stops[s].local));
						if (distance < best)
						{
							best = distance;
							stop = s;
						}
					}
					visits.push_back({ stop, 0.0, false });
					arrivedAt = standingSince;
				}
				else if (!nowStanding && standing)
				{
					standing = false;
					if (!visits.empty())
					{
						visits.back().stayed = TicksToSeconds(rig.GetWorld().Now() - arrivedAt);
						visits.back().left = true;
					}
				}
			}
			return visits;
		}

		void TestValidation(TestContext& t, ChainWorld& world)
		{
			t.Section("npcs: validation names the problem");
			Rig rig(world);
			NpcDefinition good;
			good.id = "good";
			good.name = "Good";
			good.stops = { Stop(world, 0, false, 1, 2), Stop(world, 0, true, 1, 2), Between(world, 0, 1, 2) };
			t.Check(rig.npcs->Validate(good).empty(), "a fine definition passes (%s)", rig.npcs->Validate(good).empty() ? "" : rig.npcs->Validate(good)[0].c_str());

			auto problem = [&](const char* what, auto edit, const char* expect)
			{
				NpcDefinition bad = good;
				edit(bad);
				const auto problems = rig.npcs->Validate(bad);
				std::string all;
				for (const auto& p : problems)
					all += p + " | ";
				t.Check(!problems.empty() && all.find(expect) != std::string::npos, "%s is reported (%s)", what, all.c_str());
			};
			problem("an empty id", [](NpcDefinition& d) { d.id.clear(); }, "id");
			problem("a long id", [](NpcDefinition& d) { d.id = std::string(30, 'x'); }, "id");
			problem("no stops", [](NpcDefinition& d) { d.stops.clear(); }, "no stops");
			problem("a model that does not exist", [](NpcDefinition& d) { d.model = 7; }, "model 7");
			problem("a silly speed", [](NpcDefinition& d) { d.speed = 40.0f; }, "speed");
			problem("a level that is not in the world", [](NpcDefinition& d) { d.stops[1].level = "nowhere.bsp"; }, "'nowhere.bsp' is not in the world");
			problem("a stop in the sky", [](NpcDefinition& d) { d.stops[2].local.y += 80.0f; }, "stop 2");
			problem("a stop on the mesh edge of nothing", [](NpcDefinition& d) { d.stops[0].local.x += 500.0f; }, "not on the nav mesh");
			problem("a wait that runs backwards", [](NpcDefinition& d) { d.stops[1].waitMin = 9.0f; d.stops[1].waitMax = 2.0f; }, "wait");
		}

		void TestRoutines(TestContext& t, ChainWorld& world)
		{
			t.Section("npcs: routines");
			{
				// A loop of three stops, with the crowd rules switched on beside it.
				Rig rig(world, 2);
				NpcDefinition loop;
				loop.id = "looper";
				loop.name = "Looper";
				loop.stops = { Stop(world, 0, false, 4, 6), Stop(world, 0, true, 4, 6), Between(world, 0, 4, 6) };
				const Entity npc = rig.Place(loop);
				t.Check(npc != NullEntity && rig.world.nav->Data().FindLevel(loop.stops[0].level) == 0, "an NPC is placed at its first stop");
				const auto visits = Watch(rig, npc, loop, 600.0);
				bool order = visits.size() >= 6;
				for (size_t i = 0; i < visits.size(); ++i)
					order &= visits[i].stop == (i + 1) % 3;
				t.Check(order, "it goes round its loop in order (%zu stops reached in 10 minutes)", visits.size());
				bool dwell = true;
				for (const Visit& visit : visits)
					if (visit.left)
						dwell &= visit.stayed >= 3.7 && visit.stayed <= 6.6;
				t.Check(dwell, "and stays between 4 and 6 seconds at each stop");
				t.Check(rig.Clean(), "no detector violation");
			}
			{
				Rig rig(world, 3);
				NpcDefinition pingpong;
				pingpong.id = "pacer";
				pingpong.name = "Pacer";
				pingpong.mode = Routine::Mode::PingPong;
				pingpong.stops = { Stop(world, 0, false, 1, 1), Between(world, 0, 1, 1), Stop(world, 0, true, 1, 1) };
				const Entity npc = rig.Place(pingpong);
				const auto visits = Watch(rig, npc, pingpong, 600.0);
				const size_t expected[] = { 1, 2, 1, 0 };
				bool order = visits.size() >= 8;
				for (size_t i = 0; i < visits.size(); ++i)
					order &= visits[i].stop == expected[i % 4];
				t.Check(order, "a ping-pong routine goes 1, 2, 1, 0, 1, 2 ... (%zu stops)", visits.size());
			}
			{
				// A stop in another level: the walk crosses the portals.
				Rig rig(world, 4);
				NpcDefinition courier;
				courier.id = "courier";
				courier.name = "Courier";
				courier.stops = { Stop(world, 0, false, 2, 3), Stop(world, 1, true, 2, 3), Stop(world, 2, false, 2, 3) };
				const Entity npc = rig.Place(courier);
				const auto visits = Watch(rig, npc, courier, 1500.0);
				size_t crossings = 0;
				for (size_t i = 0; i < rig.GetWorld().Log().Size(); ++i)
					crossings += rig.GetWorld().Log().At(i).type == rig.types->EventId<PortalCrossed>() ? 1 : 0;
				bool order = visits.size() >= 6;
				for (size_t i = 0; i < visits.size(); ++i)
					order &= visits[i].stop == (i + 1) % 3;
				t.Check(order && crossings >= 6, "a route over three levels is walked in order, over the portals (%zu stops, %zu crossings)", visits.size(), crossings);
				t.Check(rig.Clean(), "no detector violation across levels");
			}
			{
				// One stop: it stands there.
				Rig rig(world, 5);
				NpcDefinition statue;
				statue.id = "statue";
				statue.name = "Statue";
				statue.stops = { Stop(world, 0, true, 0, 0) };
				const Entity npc = rig.Place(statue);
				const glm::vec3 start = rig.GetWorld().TryGet<Position>(npc)->nav;
				rig.Run(60.0);
				t.Check(glm::distance(start, rig.GetWorld().TryGet<Position>(npc)->nav) < 0.01f, "an NPC with one stop stays where it is");
			}
		}

		void TestSaveAndRebake(TestContext& t, ChainWorld& world, ChainWorld& moved)
		{
			t.Section("npcs: saves and rebakes");
			NpcDefinition walker;
			walker.id = "walker";
			walker.name = "Walker";
			walker.stops = { Stop(world, 0, false, 3, 5), Stop(world, 0, true, 3, 5) };
			walker.model = 1;

			Rig a(world, 6);
			a.Place(walker);
			a.Run(45.0);
			std::vector<uint8_t> blob;
			std::string error;
			a.client->SaveBlocking(blob, error);

			Rig same(world, 7);
			const bool loaded = same.client->LoadBlocking(blob, error);
			bool identical = loaded;
			for (int i = 0; i < 1500 && identical; ++i)
			{
				a.client->Update(0.1);
				same.client->Update(0.1);
				identical = a.GetWorld().StateHash() == same.GetWorld().StateHash();
			}
			t.Check(loaded && identical, "saved and loaded, an NPC carries on exactly as the one that never stopped (%s)", error.c_str());

			// The snapshot names it.
			const auto& agents = a.client->Agents();
			t.Check(agents.size() == 1 && std::string(agents[0].id.c_str()) == "walker", "the snapshot carries the NPC's id");

			// A rebake moved the levels: the NPC is re-placed by level and goes on with its routine.
			Rig b(moved, 8);
			const bool ok = b.client->LoadBlocking(blob, error);
			t.Check(ok, "a save made before a rebake loads: %s", error.c_str());
			if (!ok)
				return;
			t.Check(b.GetWorld().Registry().storage<Identity>().size() == 1 && b.GetWorld().Registry().storage<Routine>().size() == 1, "the NPC and its routine survive");
			b.GetWorld().checkInvariants = true;
			b.Run(120.0);
			size_t arrivals = 0;
			for (size_t i = 0; i < b.GetWorld().Log().Size(); ++i)
				arrivals += b.GetWorld().Log().At(i).type == b.types->EventId<Arrived>() ? 1 : 0;
			t.Check(arrivals >= 2 && b.Clean(), "and keeps walking its stops on the new layout (%zu arrivals)", arrivals);
		}

		void TestReplay(TestContext& t, ChainWorld& world)
		{
			t.Section("npcs: replay");
			Rig rig(world, 9);
			for (int i = 0; i < 4; ++i)
			{
				NpcDefinition definition;
				definition.id = "npc" + std::to_string(i);
				definition.name = definition.id;
				definition.mode = i % 2 ? Routine::Mode::PingPong : Routine::Mode::Loop;
				definition.stops = { Stop(world, static_cast<size_t>(i % 3), false, 1, 4), Stop(world, static_cast<size_t>((i + 1) % 3), true, 1, 4), Stop(world, 0, true, 2, 3) };
				rig.Place(definition);
			}
			rig.client->ServerForSetup().StartRecording(100);
			rig.Run(300.0);
			Recording recording = rig.client->ServerForSetup().StopRecording();
			bool identical = false;
			const std::string report = ReplayRecording(rig.types, recording, identical);
			t.Check(identical && rig.Clean(), "four NPCs over five minutes replay to identical checkpoints:\n%s", report.c_str());
		}

		// ---- The content file -----------------------------------------------------------------------------------------

		void TestParser(TestContext& t, ChainWorld& world)
		{
			t.Section("npcs: the content file");
			const std::string level0 = world.chain.levels[0].name;
			const std::string spawn = "from_" + world.chain.levels[1].name; // BuildChain names the arrival spawns after the level they come from
			auto parse = [&](const std::string& text, std::vector<NpcDefinition>& npcs, std::vector<std::string>& errors) { SimNpcFile::Parse(text, *std::make_unique<const sim::nav::WorldNavData>(world.data), npcs, errors); };

			std::vector<NpcDefinition> npcs;
			std::vector<std::string> errors;
			parse(R"({ "npcs": [
				{ "id": "a", "name": "Alice", "model": "guard", "speed": 0.8, "gait": "run", "routine": "pingpong", "level": ")" + level0 + R"(",
				  "stops": [ { "at": [1, 2, 3], "wait": [4, 5] }, { "spawn": ")" + spawn + R"(", "level": ")" + world.chain.levels[0].name + R"(", "wait": 7 } ] } ] })", npcs, errors);
			t.Check(npcs.size() == 1 && errors.empty(), "a good file parses (%zu npcs, %zu errors: %s)", npcs.size(), errors.size(), errors.empty() ? "" : errors[0].c_str());
			if (npcs.size() == 1)
			{
				const NpcDefinition& a = npcs[0];
				t.Check(a.id == "a" && a.name == "Alice" && a.model == 1 && std::fabs(a.speed - 0.8f) < 1e-6f && a.gait == Gait::Run && a.mode == Routine::Mode::PingPong && a.stops.size() == 2 &&
					a.stops[0].local == glm::vec3(1, 2, 3) && a.stops[0].waitMin == 4.0f && a.stops[0].waitMax == 5.0f && a.stops[1].waitMin == 7.0f && a.stops[1].waitMax == 7.0f,
					"and every field arrives");
			}

			auto fails = [&](const char* what, const std::string& text, const char* expect)
			{
				std::vector<NpcDefinition> got;
				std::vector<std::string> problems;
				parse(text, got, problems);
				std::string all;
				for (const auto& p : problems)
					all += p + " | ";
				t.Check(!problems.empty() && all.find(expect) != std::string::npos, "%s is reported: %s", what, all.c_str());
				return got;
			};
			fails("text that is not JSON", "{ npcs: ", "not valid JSON");
			fails("no npcs list", R"({ "people": [] })", "\"npcs\" list");
			fails("an NPC without stops", R"({ "npcs": [ { "id": "x" } ] })", "stops");
			fails("a stop without a position", R"({ "npcs": [ { "id": "x", "level": "l", "stops": [ { "wait": 1 } ] } ] })", "\"at\"");
			fails("a stop without a level", R"({ "npcs": [ { "id": "x", "stops": [ { "at": [0,0,0] } ] } ] })", "no level");
			fails("an unknown spawn point", R"({ "npcs": [ { "id": "x", "level": ")" + level0 + R"(", "stops": [ { "spawn": "nowhere" } ] } ] })", "spawn point 'nowhere'");
			fails("an unknown gait", R"({ "npcs": [ { "id": "x", "gait": "fly", "level": "l", "stops": [ { "at": [0,0,0] } ] } ] })", "gait 'fly'");
			fails("an unknown model", R"({ "npcs": [ { "id": "x", "model": "tank", "level": "l", "stops": [ { "at": [0,0,0] } ] } ] })", "model 'tank'");
			fails("a malformed wait", R"({ "npcs": [ { "id": "x", "level": "l", "stops": [ { "at": [0,0,0], "wait": "long" } ] } ] })", "wait");
			const auto kept = fails("an id used twice", R"({ "npcs": [
				{ "id": "x", "level": "l", "stops": [ { "at": [0,0,0] } ] },
				{ "id": "x", "level": "l", "stops": [ { "at": [1,0,0] } ] } ] })", "used twice");
			t.Check(kept.size() == 1 && kept[0].stops[0].local.x == 0.0f, "the first one is kept");
			const auto rest = fails("one bad NPC among good ones", R"({ "npcs": [
				{ "id": "bad", "gait": "fly", "level": "l", "stops": [ { "at": [0,0,0] } ] },
				{ "id": "good", "level": "l", "stops": [ { "at": [0,0,0] } ] } ] })", "gait 'fly'");
			t.Check(rest.size() == 1 && rest[0].id == "good", "and the good one still loads");
		}

		// The content that ships, against the world that ships. A failure here means someone edited a map or the file without
		// rebaking or fixing the other.
		void TestShippedContent(TestContext& t)
		{
			t.Section("npcs: the shipped content");
			WorldNavData data;
			std::string error;
			if (!SimWorldNavFile::Load(data, error))
			{
				t.Note("%s: the shipped npcs.json was not checked", error.c_str());
				return;
			}
			WorldNav nav;
			if (!t.Check(nav.Load(data, error), "the shipped world.nav loads: %s", error.c_str()))
				return;
			MovementModule movement(nav);
			bodies::BodiesModule bodiesModule(movement, nav);
			NpcModule module(movement, nav);

			std::vector<NpcDefinition> npcs;
			std::vector<std::string> problems;
			SimNpcFile::Load(data, npcs, problems, true);
			size_t invalid = 0;
			for (const NpcDefinition& definition : npcs)
				for (const std::string& problem : module.Validate(definition))
				{
					problems.push_back(problem);
					++invalid;
				}
			std::string all;
			for (const auto& p : problems)
				all += "\n    " + p;
			t.Check(problems.empty() && npcs.size() >= 8, "all %zu NPCs of %s parse and fit the baked world%s", npcs.size(), SimNpcFile::Path, all.c_str());
			size_t models[2] = { 0, 0 };
			bool timed = true;
			for (const NpcDefinition& definition : npcs)
			{
				++models[definition.model & 1];
				for (const StopDefinition& stop : definition.stops)
					timed &= stop.waitMax >= stop.waitMin;
			}
			t.Check(models[0] > 0 && models[1] > 0, "the cast uses both models (%zu base, %zu guard)", models[0], models[1]);
			(void)invalid;
			(void)timed;
		}
	}

	void RunNpcTests(TestContext& context)
	{
		ChainWorld world, moved;
		if (!world.Build(context) || !moved.Build(context, { "test", "lvl1", "example", "test_interior" }))
			return;
		TestValidation(context, world);
		TestRoutines(context, world);
		TestSaveAndRebake(context, world, moved);
		TestReplay(context, world);
		TestParser(context, world);
		TestShippedContent(context);
	}
}

#endif
