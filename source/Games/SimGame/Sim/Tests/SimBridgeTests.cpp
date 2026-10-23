// Bridge tests (SLICE_M2B_BRIDGE.md section 7): SimClient in both Host modes, the snapshot, the save barrier and the rebake case,
// on the real chain world. No engine statics are touched.

#include "SimNavFixtures.h"

#ifndef DISTRIBUTION

#include <Sim/Bridge/SimClient.h>
#include <Sim/Movement/SimDemoPopulation.h>

#include <Helpers/Base64.h>
#include <Helpers/ByteCompressor.h>

#include <glm/geometric.hpp>

#include <chrono>
#include <memory>
#include <thread>

namespace sim::test
{
	namespace
	{
		using namespace sim::movement;
		using sim::nav::WorldNavData;

		void SleepMs(int milliseconds) { std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds)); }

		// A client over one chain world: movement module, demo module, registry, Host.
		struct ClientRig
		{
			ChainWorld& world;
			std::unique_ptr<MovementModule> movement;
			std::unique_ptr<DemoModule> demo;
			std::shared_ptr<const TypeRegistry> types;
			std::unique_ptr<SimClient> client;

			ClientRig(ChainWorld& world, uint64_t seed = 1) : world(world)
			{
				movement = std::make_unique<MovementModule>(*world.nav);
				demo = std::make_unique<DemoModule>(*movement, world.points, true);
				RegistryBuilder builder;
				builder.Add(*movement);
				builder.Add(*demo);
				std::vector<std::string> errors;
				types = builder.Build(errors);
				SimClient::Setup setup;
				setup.types = types;
				setup.worldNav = world.nav.get();
				setup.movement = movement.get();
				setup.seed = seed;
				client = std::make_unique<SimClient>(setup);
			}

			// Pump mode only: direct access to the world.
			World& GetWorld() { return client->ServerForSetup().GetWorld(); }

			void Populate(int count)
			{
				client->ServerForSetup().RunSetup("walkers", [&](ApplyContext& ctx)
				{
					for (int i = 0; i < count; ++i)
					{
						const Entity walker = movement->Place(ctx, world.points[static_cast<size_t>(i) % world.points.size()]);
						ctx.Emit(TripWanted{ walker, -1 });
					}
				});
			}

			// One second of world time per call at speed 1.
			void Run(double seconds)
			{
				for (double done = 0.0; done < seconds - 1e-9; done += 0.2)
					client->Update(0.2);
			}

			size_t WalkerCount() { return GetWorld().Registry().storage<Walker>().size(); }
			size_t Count(TypeId type)
			{
				size_t count = 0;
				for (size_t i = 0; i < GetWorld().Log().Size(); ++i)
					count += GetWorld().Log().At(i).type == type;
				return count;
			}
		};

		void TestBase64(TestContext& t)
		{
			t.Section("bridge: base64");
			t.Check(Base64::Encode({ 'M', 'a', 'n' }) == "TWFu" && Base64::Encode({ 'M', 'a' }) == "TWE=" && Base64::Encode({ 'M' }) == "TQ==" && Base64::Encode({}).empty(),
				"known encodings, including padding and the empty input");
			Rng rng(77);
			bool same = true;
			for (int length = 0; length < 70; ++length)
			{
				std::vector<uint8_t> bytes(static_cast<size_t>(length));
				for (uint8_t& b : bytes)
					b = static_cast<uint8_t>(rng.Below(256));
				same &= Base64::Decode(Base64::Encode(bytes)) == bytes;
			}
			t.Check(same, "binary data of every length from 0 to 69 survives a round trip");
		}

		void TestSaveFile(TestContext& t, ChainWorld& world, ChainWorld& moved)
		{
			t.Section("bridge: world file v2 and the sim save file");
			WorldNavData reloaded;
			std::string error;
			t.Check(!world.data.spawns.empty() && WorldNavData::Deserialize(world.data.Serialize(), reloaded, error) && reloaded.spawns.size() == world.data.spawns.size() &&
				reloaded.spawns[0].name == world.data.spawns[0].name && reloaded.Fingerprint() == world.data.Fingerprint(), "spawn points are in the world file and survive a round trip (%zu spawns): %s",
				world.data.spawns.size(), error.c_str());
			t.Check(world.data.Fingerprint() == world.data.Fingerprint() && world.data.Fingerprint() != moved.data.Fingerprint(),
				"the fingerprint is stable and changes when the layout does");

			SimSave save;
			save.header.tick = 1234;
			save.header.schema = 99;
			save.header.fingerprint = 7;
			save.header.levels = { "a.bsp", "b.bsp" };
			save.header.placements = { 1, 2, 3 };
			save.world = { 9, 8, 7, 6 };
			SimSave read;
			t.Check(ReadSimSave(WriteSimSave(save), read, error) && read.header.tick == 1234 && read.header.levels == save.header.levels && read.header.placements == save.header.placements &&
				read.world == save.world, "a sim save round trips: %s", error.c_str());
			std::vector<uint8_t> bytes = WriteSimSave(save);
			bytes.resize(bytes.size() / 2);
			t.Check(!ReadSimSave(bytes, read, error), "a truncated save is refused (%s)", error.c_str());
			t.Check(!ReadSimSave({ 1, 2, 3, 4 }, read, error), "garbage is refused (%s)", error.c_str());
			std::vector<uint8_t> raw = ByteCompressor::DecompressData(WriteSimSave(save));
			raw[4] = 9; // format version
			t.Check(!ReadSimSave(ByteCompressor::CompressData(raw), read, error) && error.find("format") != std::string::npos, "another format version is refused (%s)", error.c_str());
		}

		void TestThread(TestContext& t, ChainWorld& world)
		{
			t.Section("bridge: thread mode");
			for (int i = 0; i < 20; ++i)
			{
				ClientRig rig(world);
				rig.client->Start(Host::Mode::Thread);
				rig.client->Stop();
				if (rig.client->IsRunning())
				{
					t.Check(false, "start and stop %d left the host running", i);
					return;
				}
			}
			t.Check(true, "20 starts and stops in a row");

			ClientRig rig(world);
			rig.Populate(10);
			rig.client->Start(Host::Mode::Thread);
			rig.client->SetSpeed(20.0);
			SleepMs(400);
			rig.client->Update();
			const Tick ticks = rig.client->CurrentTick();
			t.Check(ticks >= 20 && ticks <= 400, "at speed 20 the sim ran %lld ticks in 0.4 s", static_cast<long long>(ticks));
			t.Check(rig.client->SnapshotsReceived() > 0 && rig.client->Agents().size() == 10, "snapshots arrive on the game thread (%llu, %zu agents)",
				static_cast<unsigned long long>(rig.client->SnapshotsReceived()), rig.client->Agents().size());

			rig.client->SetSpeed(1.0);
			rig.client->Pause();
			SleepMs(200);
			const Tick frozen = rig.client->CurrentTick();
			SleepMs(300);
			t.Check(rig.client->IsPaused() && rig.client->CurrentTick() == frozen, "pause freezes the tick (%lld)", static_cast<long long>(frozen));
			std::string reply;
			const bool answered = rig.client->TextBlocking("hash", reply);
			t.Check(answered && !reply.empty(), "a text request is answered while paused: %s", reply.c_str());
			std::vector<uint8_t> blob;
			std::string error;
			t.Check(rig.client->SaveBlocking(blob, error) && !blob.empty(), "a save is answered while paused: %s", error.c_str());
			rig.client->Resume();
			SleepMs(500);
			t.Check(rig.client->CurrentTick() > frozen, "resume continues");

			const auto before = std::chrono::steady_clock::now();
			rig.client->Stop();
			const double stopSeconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - before).count();
			t.Check(!rig.client->IsRunning() && stopSeconds < 1.0, "stop returns in %.3f s", stopSeconds);

			// A request nobody answers times out and says so.
			ClientRig stopped(world);
			stopped.client->Start(Host::Mode::Thread);
			stopped.client->Stop();
			t.Check(!stopped.client->SaveBlocking(blob, error, 0.05) && error.find("did not answer") != std::string::npos, "a save to a stopped sim times out: %s", error.c_str());
		}

		void TestPump(TestContext& t, ChainWorld& world)
		{
			t.Section("bridge: pump mode, player and snapshot");
			auto run = [&](std::vector<uint64_t>& hashes)
			{
				ClientRig rig(world, 5);
				rig.Populate(24);
				rig.client->Start(Host::Mode::Pump);
				rig.client->ReportPlayer(0, world.chain.east[0]);
				for (int i = 0; i < 300; ++i)
				{
					rig.client->Update(0.1);
					hashes.push_back(rig.GetWorld().StateHash());
				}
			};
			std::vector<uint64_t> a, b;
			run(a);
			run(b);
			t.Check(a == b && !a.empty(), "the same scenario twice gives the same state hash at every tick");

			ClientRig rig(world, 6);
			rig.Populate(24);
			rig.client->Start(Host::Mode::Pump);
			rig.GetWorld().SetTunable(rig.movement->tier0Radius, 5.0f);
			rig.client->ReportPlayer(0, world.chain.east[0]);
			rig.Run(2.0);
			const auto* anchor = [&]() -> const PlayerAnchor*
			{
				for (const auto [entity, value] : rig.GetWorld().Registry().view<const PlayerAnchor>().each())
					return &value;
				return nullptr;
			}();
			t.Check(anchor && anchor->level == 0 && glm::distance(anchor->nav, world.points[0]) < 0.01f, "a player report reaches the player anchor");

			auto tierCounts = [&](int counts[3])
			{
				counts[0] = counts[1] = counts[2] = 0;
				for (const AgentView& agent : rig.client->Agents())
					++counts[agent.tier];
			};
			int fromSnapshot[3], fromWorld[3] = { 0, 0, 0 };
			tierCounts(fromSnapshot);
			for (const auto [entity, walker] : rig.GetWorld().Registry().view<const Walker>().each())
				++fromWorld[static_cast<int>(rig.movement->TierOf(rig.GetWorld(), entity))];
			t.Check(rig.client->Agents().size() == 24 && rig.client->SnapshotTick() == rig.GetWorld().Now() && fromSnapshot[0] == fromWorld[0] && fromSnapshot[1] == fromWorld[1] &&
				fromSnapshot[2] == fromWorld[2], "the snapshot has every walker and tier counts that match the sim (%d/%d/%d)", fromSnapshot[0], fromSnapshot[1], fromSnapshot[2]);

			const uint64_t snapshots = rig.client->SnapshotsReceived();
			rig.Run(1.0);
			t.Check(rig.client->SnapshotsReceived() > snapshots && rig.client->SnapshotTick() == rig.GetWorld().Now(), "only the newest snapshot is kept (tick %lld)",
				static_cast<long long>(rig.client->SnapshotTick()));

			rig.client->ReportPlayer(3, world.chain.west[3]);
			rig.Run(0.4);
			int moved[3];
			tierCounts(moved);
			t.Check(moved[2] > 0 && moved[0] + moved[1] > 0 && rig.client->Agents().size() == 24 && (moved[0] != fromSnapshot[0] || moved[1] != fromSnapshot[1]), "a player in another level moves the tiers (%d/%d/%d)",
				moved[0], moved[1], moved[2]);

			const size_t rejected = rig.GetWorld().Stats().commandsRejected;
			rig.client->ReportPlayer(99, glm::vec3(0.0f));
			rig.Run(0.4);
			t.Check(rig.GetWorld().Stats().commandsRejected == rejected + 1, "a report for an unknown level is rejected");

			std::string reply;
			t.Check(rig.client->TextBlocking("state", reply) && reply.find("Walker") != std::string::npos, "text commands work through the client");
		}

		void TestSaveLoad(TestContext& t, ChainWorld& world)
		{
			t.Section("bridge: save barrier");
			ClientRig a(world, 11);
			a.Populate(24);
			a.client->Start(Host::Mode::Pump);
			a.client->ReportPlayer(0, world.chain.east[0]);
			a.GetWorld().checkInvariants = true;
			a.Run(40.0);
			t.Check(a.Count(a.types->EventId<Arrived>()) > 0, "agents have been walking (%zu arrivals)", a.Count(a.types->EventId<Arrived>()));

			std::vector<uint8_t> blob;
			std::string error;
			const uint64_t hashAtSave = a.GetWorld().StateHash();
			t.Check(a.client->SaveBlocking(blob, error), "save succeeds: %s", error.c_str());

			ClientRig b(world, 999);
			b.client->Start(Host::Mode::Pump);
			t.Check(b.client->LoadBlocking(blob, error), "load succeeds: %s", error.c_str());
			t.Check(b.GetWorld().StateHash() == hashAtSave, "the loaded sim has the saved state hash");
			t.Check(b.Count(b.types->EventId<Replaced>()) == 0, "an unchanged layout re-places nobody");
			b.GetWorld().checkInvariants = true;
			bool same = true;
			for (int i = 0; i < 400 && same; ++i)
			{
				a.client->Update(0.1);
				b.client->Update(0.1);
				same = a.GetWorld().StateHash() == b.GetWorld().StateHash();
			}
			t.Check(same, "saved and loaded, the world continues identically to the one that kept running for 400 ticks");
			t.Check(a.GetWorld().Stats().invariantFailures == 0 && b.GetWorld().Stats().invariantFailures == 0, "no detector violation on either side");

			// Refusals leave the sim as it was.
			const uint64_t before = b.GetWorld().StateHash();
			std::vector<uint8_t> truncated(blob.begin(), blob.begin() + static_cast<std::ptrdiff_t>(blob.size() / 2));
			t.Check(!b.client->LoadBlocking(truncated, error) && b.GetWorld().StateHash() == before, "a truncated save is refused and the sim is untouched (%s)", error.c_str());
			SimSave save;
			ReadSimSave(blob, save, error);
			SimSave wrongSchema = save;
			wrongSchema.header.schema ^= 1;
			t.Check(!b.client->LoadBlocking(WriteSimSave(wrongSchema), error) && error.find("schema") != std::string::npos && b.GetWorld().StateHash() == before, "different component types are refused (%s)", error.c_str());
			SimSave corruptWorld = save;
			corruptWorld.world.resize(corruptWorld.world.size() / 2);
			t.Check(!b.client->LoadBlocking(WriteSimSave(corruptWorld), error) && b.GetWorld().StateHash() == before, "a corrupt world blob is refused by the kernel and the sim is untouched (%s)", error.c_str());

			// Paused in pump mode, the barrier still answers.
			b.client->Pause();
			b.client->Update(0.0);
			t.Check(b.client->SaveBlocking(blob, error), "a save requested while paused is answered");
		}

		void TestRebake(TestContext& t, ChainWorld& world, ChainWorld& moved)
		{
			t.Section("bridge: loading a save made against another layout");
			ClientRig a(world, 21);
			// Two walkers on every point of the chain.
			a.client->ServerForSetup().RunSetup("walkers", [&](ApplyContext& ctx)
			{
				for (int i = 0; i < 16; ++i)
					a.movement->Place(ctx, world.points[static_cast<size_t>(i) / 2]);
			});
			a.client->Start(Host::Mode::Pump);
			a.client->Update(0.1);
			std::vector<uint8_t> blob;
			std::string error;
			a.client->SaveBlocking(blob, error);

			// Where each saved walker stood, by level name and local position.
			struct Spot
			{
				std::string level;
				glm::vec3 local;
			};
			std::vector<Spot> spots;
			for (const Placement& placement : a.movement->PlacementsOf(a.GetWorld()))
				spots.push_back({ world.nav->Data().levels[placement.level].name, placement.local });

			ClientRig b(moved, 22);
			b.client->Start(Host::Mode::Pump);
			b.GetWorld().checkInvariants = true;
			t.Check(world.nav->Data().Fingerprint() != moved.nav->Data().Fingerprint(), "the second layout really differs");
			t.Check(b.client->LoadBlocking(blob, error), "a save made against another layout loads: %s", error.c_str());
			t.Check(b.WalkerCount() == 16 && b.Count(b.types->EventId<Replaced>()) == 16, "all 16 agents are re-placed (%zu walkers, %zu Replaced)", b.WalkerCount(), b.Count(b.types->EventId<Replaced>()));

			int misplaced = 0;
			size_t index = 0;
			for (const auto [entity, walker, position] : b.GetWorld().Registry().view<const Walker, const Position>().each())
			{
				(void)index;
				const int level = b.movement->LevelOf(b.GetWorld(), entity);
				const Placement* placement = nullptr;
				for (const Placement& candidate : b.movement->PlacementsOf(b.GetWorld()))
					if (candidate.entity == entity)
						placement = &candidate;
				bool found = false;
				for (const Spot& spot : spots)
					found |= level >= 0 && moved.nav->Data().levels[level].name == spot.level && placement && glm::distance(placement->local, spot.local) < 2.5f;
				misplaced += found ? 0 : 1;
			}
			t.Check(misplaced == 0, "every agent is in its old level, within 2.5 m of its old local position (%d are not)", misplaced);

			int waiting = 0;
			for (const auto [entity, walker] : b.GetWorld().Registry().view<const Walker>().each())
				waiting += walker.state == Walker::State::WaitingForPath && walker.corners.empty() ? 1 : 0;
			t.Check(waiting == 16, "their old paths are gone and the demo rule has asked for new walks (%d waiting)", waiting);
			b.Run(60.0);
			t.Check(b.Count(b.types->EventId<Arrived>()) > 0 && b.GetWorld().Stats().invariantFailures == 0, "and then they walk again (%zu arrivals), no detector violation", b.Count(b.types->EventId<Arrived>()));

			// A level that is gone: its agents are removed, loudly.
			ChainWorld shorter;
			if (!shorter.Build(t, { "lvl1", "test", "example" }))
				return;
			ClientRig c(shorter, 23);
			c.client->Start(Host::Mode::Pump);
			t.Check(c.client->LoadBlocking(blob, error), "a save with a level the new world lacks still loads: %s", error.c_str());
			t.Check(c.WalkerCount() == 12, "the four agents of the missing level are removed (%zu left)", c.WalkerCount());
		}

		void TestSnapshotCost(TestContext& t, ChainWorld& world)
		{
			t.Section("bridge: snapshot cost");
			ClientRig rig(world, 31);
			rig.Populate(400);
			rig.client->Start(Host::Mode::Pump);
			rig.Run(2.0);
			std::vector<uint8_t> blob;
			Stopwatch timer;
			constexpr int Repeats = 200;
			for (int i = 0; i < Repeats; ++i)
				rig.movement->WriteSnapshot(rig.GetWorld(), blob);
			t.Note("snapshot of 400 walkers: %zu bytes, %.1f us to build", blob.size(), timer.Seconds() / Repeats * 1e6);
			std::vector<uint8_t> save;
			std::string error;
			Stopwatch saveTimer;
			rig.client->SaveBlocking(save, error);
			t.Note("save of 400 walkers: %zu bytes compressed, %.1f ms (log included)", save.size(), saveTimer.Seconds() * 1e3);
		}
	}

	void RunBridgeTests(TestContext& context)
	{
		TestBase64(context);
		ChainWorld world, moved;
		if (!world.Build(context) || !moved.Build(context, { "test", "lvl1", "example", "test_interior" }))
			return;
		TestSaveFile(context, world, moved);
		TestPump(context, world);
		TestSaveLoad(context, world);
		TestRebake(context, world, moved);
		TestSnapshotCost(context, world);
		TestThread(context, world);
	}
}

#endif
