// Kernel tests: archives, random, lazy values, scheduler, event log, registration, waves and causes, cascade cap,
// determinism, save round trip, replay, invariants, text tools, host (pump and thread), throughput.
//
// The "test" module below is a toy world: agents get hungry, hunger hurts, damage kills, death raises an alarm.
// It exercises every kernel path without any game content.

#include "SimTestFramework.h"

#ifndef DISTRIBUTION

#include <Sim/Core/SimHost.h>

#include <algorithm>
#include <thread>

namespace sim::test
{
	namespace
	{
		// ---- Toy module ------------------------------------------------------------------------------------------

		struct TestAgent
		{
			static constexpr const char* TypeName = "TestAgent";
			int32_t health = 100;
			bool dead = false;
			EventId deathCause = NoEvent;
			LazyValue hunger;
			TimerHandle hungerTimer;
			int32_t wanderSteps = 0;
			template<class Ar> void Visit(Ar& ar)
			{
				ar("health", health);
				ar("dead", dead);
				ar("deathCause", deathCause);
				ar("hunger", hunger);
				ar("hungerTimer", hungerTimer);
				ar("wanderSteps", wanderSteps);
			}
		};

		struct TestMarker // empty component
		{
			static constexpr const char* TypeName = "TestMarker";
			template<class Ar> void Visit(Ar&) {}
		};

		struct DamageRequested
		{
			static constexpr const char* TypeName = "DamageRequested";
			Entity target = NullEntity;
			int32_t amount = 0;
			template<class Ar> void Visit(Ar& ar) { ar("target", target); ar("amount", amount); }
		};

		struct AgentDied
		{
			static constexpr const char* TypeName = "AgentDied";
			Entity agent = NullEntity;
			template<class Ar> void Visit(Ar& ar) { ar("agent", agent); }
		};

		struct HungerCritical
		{
			static constexpr const char* TypeName = "HungerCritical";
			Entity agent = NullEntity;
			template<class Ar> void Visit(Ar& ar) { ar("agent", agent); }
		};

		struct AlarmRaised
		{
			static constexpr const char* TypeName = "AlarmRaised";
			Entity agent = NullEntity;
			template<class Ar> void Visit(Ar& ar) { ar("agent", agent); }
		};

		struct ChainLink
		{
			static constexpr const char* TypeName = "ChainLink";
			int32_t depth = 0;
			int32_t maxDepth = 0;
			template<class Ar> void Visit(Ar& ar) { ar("depth", depth); ar("maxDepth", maxDepth); }
		};

		struct Ping
		{
			static constexpr const char* TypeName = "Ping";
			int32_t remaining = 0;
			template<class Ar> void Visit(Ar& ar) { ar("remaining", remaining); }
		};

		struct ApplyDamage
		{
			static constexpr const char* TypeName = "ApplyDamage";
			Entity target = NullEntity;
			int32_t amount = 0;
			template<class Ar> void Visit(Ar& ar) { ar("target", target); ar("amount", amount); }
		};

		struct ResetHunger
		{
			static constexpr const char* TypeName = "ResetHunger";
			Entity agent = NullEntity;
			template<class Ar> void Visit(Ar& ar) { ar("agent", agent); }
		};

		constexpr float HungerThreshold = 0.8f;

		struct TestModule : Module
		{
			TunableId hungerRate = 0;
			TunableId allowNegative = 0;
			RandomStreamId wander = 0;

			const char* Name() const override { return "test"; }

			void Register(Registration& reg) override
			{
				reg.Component<TestAgent>();
				reg.Component<TestMarker>();
				reg.Event<DamageRequested>(EventInput);
				reg.Event<AgentDied>(EventOutput | EventSignificant);
				reg.Event<HungerCritical>();
				reg.Event<AlarmRaised>(EventWake | EventOutput);
				reg.Event<ChainLink>(EventInput);
				reg.Event<Ping>(EventInput);

				hungerRate = reg.Tunable("hunger_rate", 0.01f, 0.0f, 1.0f, "hunger gained per second");
				allowNegative = reg.Tunable("allow_negative_health", 0.0f, 0.0f, 1.0f, "1 lets health go below zero (invariant test)");
				wander = reg.RandomStream("wander");

				const TunableId negative = allowNegative;
				reg.Command<ApplyDamage>([negative](ApplyContext& ctx, const ApplyDamage& command)
				{
					TestAgent* agent = ctx.world.TryGet<TestAgent>(command.target);
					if (!agent)
					{
						ctx.Reject("no such agent");
						return;
					}
					if (agent->dead)
					{
						ctx.Reject("already dead");
						return;
					}
					agent->health -= command.amount;
					if (agent->health < 0 && ctx.world.Tunable(negative) < 0.5f)
						agent->health = 0;
					if (agent->health <= 0)
					{
						agent->dead = true;
						agent->deathCause = ctx.cause;
						ctx.Cancel(agent->hungerTimer);
						ctx.Emit(AgentDied{ command.target });
					}
				});

				reg.Command<ResetHunger>([](ApplyContext& ctx, const ResetHunger& command)
				{
					TestAgent* agent = ctx.world.TryGet<TestAgent>(command.agent);
					if (!agent || agent->dead)
					{
						ctx.Reject("no living agent");
						return;
					}
					agent->hunger.Set(ctx.Now(), 0.0f);
					ScheduleHunger(ctx, command.agent, *agent);
				});

				reg.Rule<DamageRequested>("damage", [](RuleContext& ctx, const DamageRequested& event) { ctx.Command(ApplyDamage{ event.target, event.amount }); });
				reg.Rule<HungerCritical>("hunger_hurts", [](RuleContext& ctx, const HungerCritical& event)
				{
					ctx.Command(ApplyDamage{ event.agent, 30 });
					ctx.Command(ResetHunger{ event.agent });
				});
				reg.Rule<AgentDied>("raise_alarm", [](RuleContext& ctx, const AgentDied& event) { ctx.Emit(AlarmRaised{ event.agent }); });
				reg.Rule<ChainLink>("chain", [](RuleContext& ctx, const ChainLink& event)
				{
					if (event.depth < event.maxDepth)
						ctx.Emit(ChainLink{ event.depth + 1, event.maxDepth });
				});
				reg.Rule<Ping>("ping", [](RuleContext& ctx, const Ping& event)
				{
					if (event.remaining > 0)
						ctx.Schedule(ctx.Now() + 5, Ping{ event.remaining - 1 });
				});

				const RandomStreamId stream = wander;
				reg.System("wander", [stream](ApplyContext& ctx, Tick dt)
				{
					Rng& rng = ctx.Stream(stream);
					for (auto [entity, agent] : ctx.world.Registry().view<TestAgent>().each())
						if (!agent.dead && IsStaggerDue(entity, ctx.Now(), dt, 3))
							agent.wanderSteps += static_cast<int32_t>(rng.Below(3));
				});

				reg.Invariant("health_non_negative", [](const World& world, std::string& error)
				{
					for (auto [entity, agent] : world.Registry().view<const TestAgent>().each())
						if (agent.health < 0)
						{
							error = world.DescribeEntity(entity) + " has health " + std::to_string(agent.health);
							return false;
						}
					return true;
				});
			}

			static void ScheduleHunger(ApplyContext& ctx, Entity entity, TestAgent& agent)
			{
				ctx.Cancel(agent.hungerTimer);
				const Tick crossing = agent.hunger.CrossingTick(ctx.Now(), HungerThreshold);
				if (crossing != NeverTick)
					agent.hungerTimer = ctx.Schedule(crossing, HungerCritical{ entity });
			}
		};

		TestModule testModule;

		std::shared_ptr<const TypeRegistry> BuildTypes()
		{
			static std::shared_ptr<const TypeRegistry> types;
			if (!types)
			{
				RegistryBuilder builder;
				builder.Add(testModule);
				std::vector<std::string> errors;
				types = builder.Build(errors);
			}
			return types;
		}

		// Creates `count` agents with staggered hunger rates. Returns them in creation order.
		std::vector<Entity> SetupAgents(Server& server, int count, float rateScale = 1.0f)
		{
			std::vector<Entity> agents;
			server.RunSetup("test agents", [&](ApplyContext& ctx)
			{
				for (int i = 0; i < count; ++i)
				{
					const Entity entity = ctx.Create();
					TestAgent& agent = ctx.world.Registry().emplace<TestAgent>(entity);
					agent.hunger.min = 0.0f;
					agent.hunger.max = 1.0f;
					agent.hunger.Set(ctx.Now(), 0.0f);
					agent.hunger.ratePerSecond = ctx.world.Tunable(testModule.hungerRate) * rateScale * (1.0f + 0.1f * (i % 10));
					ctx.world.Registry().emplace<Label>(entity, "agent" + std::to_string(i));
					if (i % 2 == 0)
						ctx.world.Registry().emplace<TestMarker>(entity);
					TestModule::ScheduleHunger(ctx, entity, agent);
					agents.push_back(entity);
				}
			});
			return agents;
		}

		// The same scripted inputs for every run that must match.
		void ScriptedInputs(Server& server, const std::vector<Entity>& agents)
		{
			const Tick now = server.GetWorld().Now();
			if (now % 97 == 13 && !agents.empty())
				server.QueueInput(DamageRequested{ agents[static_cast<size_t>(now) % agents.size()], 25 });
			if (now % 250 == 7)
				server.QueueInput(Ping{ 4 });
		}

		size_t CountEvents(const World& world, TypeId type)
		{
			size_t count = 0;
			for (size_t i = 0; i < world.Log().Size(); ++i)
				count += world.Log().At(i).type == type;
			return count;
		}

		// ---- Tests -----------------------------------------------------------------------------------------------

		enum class Mood : uint8_t { Calm, Afraid };

		struct Inner
		{
			double depth = 0.0;
			template<class Ar> void Visit(Ar& ar) { ar("depth", depth); }
		};

		struct ArchiveSample
		{
			int32_t number = 0;
			float ratio = 0.0f;
			std::string text;
			std::vector<int16_t> list;
			glm::vec3 position{ 0.0f };
			Entity who = NullEntity;
			Mood mood = Mood::Calm;
			bool flag = false;
			std::array<uint8_t, 3> bytes{};
			FixedString<8> shortName;
			std::vector<Inner> inner;
			template<class Ar> void Visit(Ar& ar)
			{
				ar("number", number);
				ar("ratio", ratio);
				ar("text", text);
				ar("list", list);
				ar("position", position);
				ar("who", who);
				ar("mood", mood);
				ar("flag", flag);
				ar("bytes", bytes);
				ar("shortName", shortName);
				ar("inner", inner);
			}
		};

		void TestArchives(TestContext& t)
		{
			t.Section("archives");
			ArchiveSample a;
			a.number = -42;
			a.ratio = 0.25f;
			a.text = "hello";
			a.list = { 1, -2, 3 };
			a.position = { 1.5f, -2.0f, 3.0f };
			a.who = static_cast<Entity>(7u);
			a.mood = Mood::Afraid;
			a.flag = true;
			a.bytes = { 9, 8, 7 };
			a.shortName = "toolongname";
			a.inner = { Inner{ 1.0 }, Inner{ 2.5 } };

			BinaryWriter writer;
			writer.Write(a);
			ArchiveSample b;
			BinaryReader reader(writer.bytes.data(), writer.bytes.size());
			reader.Read(b);
			t.Check(!reader.Failed() && reader.AtEnd(), "binary read consumed everything");
			t.Check(b.number == -42 && b.ratio == 0.25f && b.text == "hello" && b.list == a.list && b.position == a.position && b.who == a.who &&
				b.mood == Mood::Afraid && b.flag && b.bytes == a.bytes && b.inner.size() == 2 && b.inner[1].depth == 2.5, "binary round trip keeps every field");
			t.Check(b.shortName.str() == "toolong", "FixedString truncates to N-1 characters (got '%s')", b.shortName.c_str());

			HashArchive ha, hb;
			ha.Add(a);
			hb.Add(b);
			t.Check(ha.hash == hb.hash, "equal records hash equal");
			b.inner[0].depth = 1.0000001;
			HashArchive hc;
			hc.Add(b);
			t.Check(hc.hash != ha.hash, "a nested change changes the hash");

			TextArchive text;
			text.Add(a);
			t.Check(text.text.find("text=\"hello\"") != std::string::npos && text.text.find("position={x=1.5 y=-2 z=3}") != std::string::npos &&
				text.text.find("list=[1 -2 3]") != std::string::npos, "text dump: %s", text.text.c_str());

			EntityCollector collector;
			collector.Add(a);
			t.Check(collector.entities.size() == 1 && collector.entities[0] == a.who, "entity collector finds entity fields");

			for (size_t cut : { size_t{ 0 }, size_t{ 3 }, writer.bytes.size() / 2, writer.bytes.size() - 1 })
			{
				ArchiveSample c;
				BinaryReader truncated(writer.bytes.data(), cut);
				truncated.Read(c);
				t.Check(truncated.Failed(), "truncated data (%zu bytes) is reported, not crashed on", cut);
			}
		}

		void TestRandom(TestContext& t)
		{
			t.Section("random");
			Rng a(1234, 5), b(1234, 5), c(1234, 6);
			bool same = true, differs = false, inRange = true;
			for (int i = 0; i < 1000; ++i)
			{
				const uint32_t x = a.NextU32();
				same &= x == b.NextU32();
				differs |= x != c.NextU32();
				const uint32_t below = a.Below(7);
				const float unit = a.Unit();
				const int32_t range = a.Range(-3, 3);
				inRange &= below < 7 && unit >= 0.0f && unit < 1.0f && range >= -3 && range <= 3;
				b.Below(7), b.Unit(), b.Range(-3, 3);
			}
			t.Check(same, "same seed and stream give the same sequence");
			t.Check(differs, "a different stream gives a different sequence");
			t.Check(inRange, "Below, Unit and Range stay in range");
		}

		void TestLazyValue(TestContext& t)
		{
			t.Section("lazy values");
			LazyValue v;
			v.min = 0.0f;
			v.max = 1.0f;
			v.Set(0, 0.0f);
			v.SetRate(0, 0.5f);
			t.Check(v.CrossingTick(0, 1.0f) == 20, "rising 0.5/s reaches 1.0 at tick 20 (got %lld)", static_cast<long long>(v.CrossingTick(0, 1.0f)));
			t.Check(v.At(20) == 1.0f && v.At(400) == 1.0f && v.At(10) == 0.5f, "value is clamped and linear");
			t.Check(v.CrossingTick(0, 1.5f) == NeverTick, "a threshold above max is never reached");
			v.SetRate(10, -0.25f);
			t.Check(v.At(10) == 0.5f && v.CrossingTick(10, 0.0f) == 30, "falling from 0.5 at 0.25/s reaches 0 at tick 30");
			v.SetRate(10, 0.0f);
			t.Check(v.CrossingTick(10, 0.9f) == NeverTick, "zero rate never crosses");
			LazyValue odd;
			odd.min = 0.0f;
			odd.max = 1.0f;
			odd.Set(0, 0.0f);
			odd.SetRate(0, 0.03f);
			const Tick crossing = odd.CrossingTick(0, 0.8f);
			t.Check(odd.At(crossing) >= 0.8f && odd.At(crossing - 1) < 0.8f, "crossing tick is the first tick at or past the threshold");
		}

		void TestScheduler(TestContext& t)
		{
			t.Section("scheduler");
			Scheduler s;
			const int payload = 0;
			s.Schedule(5, 0, &payload, sizeof(payload), 1, 0, false);
			TimerHandle h3a = s.Schedule(3, 0, &payload, sizeof(payload), 2, 0, true);
			s.Schedule(3, 0, &payload, sizeof(payload), 3, 0, false);
			TimerHandle h9 = s.Schedule(9, 0, &payload, sizeof(payload), 4, 0, true);
			t.Check(s.IsPending(h3a) && s.IsPending(h9), "handles are pending after scheduling");

			ScheduledEvent e;
			std::vector<EventId> order;
			while (s.PopDue(6, e))
				order.push_back(e.cause);
			t.Check(order == std::vector<EventId>{ 2, 3, 1 }, "pops by tick, then by scheduling order");
			t.Check(!s.IsPending(h3a), "a fired handle is no longer pending");
			const TimerHandle old = h9;
			TimerHandle copy = h9;
			t.Check(s.Cancel(h9) && !h9.IsSet(), "cancel returns true and resets the handle");
			t.Check(!s.Cancel(copy), "cancelling twice is harmless");
			copy = old;
			TimerHandle reuse = s.Schedule(20, 0, &payload, sizeof(payload), 5, 0, true);
			t.Check(reuse.slot == copy.slot && !s.IsPending(copy) && s.IsPending(reuse), "a reused slot does not revive an old handle");
			t.Check(!s.PopDue(19, e) && s.PopDue(20, e) && e.cause == 5, "cancelled entries never fire");

			// Many cancellations trigger compaction; the surviving order must not change.
			Scheduler big;
			Rng rng(99);
			std::vector<TimerHandle> handles;
			std::vector<std::pair<Tick, EventId>> expected;
			for (int i = 0; i < 2000; ++i)
			{
				const Tick at = rng.Range(1, 500);
				handles.push_back(big.Schedule(at, 0, &payload, sizeof(payload), static_cast<EventId>(i + 1), 0, true));
				expected.emplace_back(at, static_cast<EventId>(i + 1));
			}
			std::vector<bool> cancelled(2000, false);
			for (int i = 0; i < 1500; ++i)
			{
				const uint32_t index = rng.Below(2000);
				if (big.Cancel(handles[index]))
					cancelled[index] = true;
			}
			std::vector<std::pair<Tick, EventId>> survivors;
			for (size_t i = 0; i < expected.size(); ++i)
				if (!cancelled[i])
					survivors.push_back(expected[i]);
			std::stable_sort(survivors.begin(), survivors.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
			std::vector<std::pair<Tick, EventId>> popped;
			while (big.PopDue(1000, e))
				popped.emplace_back(e.at, e.cause);
			t.Check(popped == survivors, "after compaction %zu survivors pop in (tick, sequence) order", survivors.size());
		}

		void TestEventLog(TestContext& t)
		{
			t.Section("event log");
			EventLog log(8);
			for (EventId id = 1; id <= 20; ++id)
			{
				EventRecord r;
				r.id = id;
				log.Append(r);
			}
			t.Check(log.Size() == 8 && log.Oldest().id == 13 && log.Newest().id == 20, "ring keeps the newest records");
			t.Check(log.Find(12) == nullptr && log.Find(13) && log.Find(13)->id == 13 && log.Find(20)->id == 20 && log.Find(21) == nullptr, "lookup by id");
		}

		void TestRegistration(TestContext& t)
		{
			t.Section("registration");
			struct BadModule : Module
			{
				const char* Name() const override { return "bad"; }
				void Register(Registration& reg) override
				{
					reg.Event<AgentDied>();
					reg.Event<AgentDied>();
					reg.Rule<HungerCritical>("orphan", [](RuleContext&, const HungerCritical&) {});
					reg.Tunable("out_of_range", 5.0f, 0.0f, 1.0f, "");
				}
			} bad;
			RegistryBuilder builder;
			builder.Add(bad);
			std::vector<std::string> errors;
			auto types = builder.Build(errors);
			t.Check(!types && errors.size() == 3, "duplicate event, rule on unregistered event and bad tunable are reported (%zu errors)", errors.size());

			auto good = BuildTypes();
			t.Check(good != nullptr, "the test module registers cleanly");
			if (good)
			{
				const std::string described = good->Describe();
				t.Check(described.find("-> test.damage") != std::string::npos && described.find("test.wander") != std::string::npos, "startup dump lists rules and systems");
			}
		}

		void TestStagger(TestContext& t)
		{
			t.Section("stagger");
			bool exact = true;
			for (uint32_t index = 0; index < 12; ++index)
			{
				const Entity e = static_cast<Entity>(index);
				int due = 0;
				for (Tick now = 1; now <= 30; ++now)
					due += IsStaggerDue(e, now, 1, 5);
				exact &= due == 6;
			}
			t.Check(exact, "each entity is due exactly once per period");
			t.Check(IsStaggerDue(static_cast<Entity>(3u), 100, 5, 5) && IsStaggerDue(static_cast<Entity>(4u), 100, 10, 5), "a step as long as the period always covers the slot");
		}

		void TestWorldBasics(TestContext& t)
		{
			t.Section("world: waves, causes, rejection, stale handles");
			auto types = BuildTypes();
			Server server(types, 7);
			const std::vector<Entity> agents = SetupAgents(server, 5);
			World& world = server.GetWorld();

			server.QueueInput(DamageRequested{ agents[1], 150 });
			server.Step();
			const TestAgent* victim = world.TryGet<TestAgent>(agents[1]);
			t.Check(victim && victim->dead && victim->health == 0, "damage over health kills");

			std::vector<EventRecord> outputs = server.TakeOutputs();
			const EventRecord* died = nullptr;
			for (const EventRecord& r : outputs)
				if (const AgentDied* d = r.As<AgentDied>(*types); d && d->agent == agents[1])
					died = &r;
			t.Check(died != nullptr, "AgentDied is an output");
			t.Check(world.TakeWake(), "AlarmRaised (a wake event) set the wake flag");
			if (died)
			{
				const std::string why = world.WhyTrace(died->id);
				t.Check(why.find("DamageRequested") != std::string::npos && why.find("by input") != std::string::npos && why.find("(root)") != std::string::npos,
					"why-trace walks to the input:\n%s", why.c_str());
				t.Check(victim->deathCause == died->cause, "the component records the cause of death");
				const EventRecord* cause = world.Log().Find(died->cause);
				t.Check(cause && cause->type == types->EventId<DamageRequested>() && died->rule == types->Event(types->EventId<DamageRequested>()).rules[0],
					"the death is stamped with the damage event and the damage rule");
			}
			const std::string timeline = world.Timeline(agents[1]);
			t.Check(timeline.find("DamageRequested") != std::string::npos && timeline.find("AgentDied") != std::string::npos && timeline.find("AlarmRaised") != std::string::npos,
				"timeline lists every event about the agent:\n%s", timeline.c_str());

			server.QueueInput(DamageRequested{ agents[1], 10 });
			server.Step();
			t.Check(world.Stats().commandsRejected == 1 && CountEvents(world, types->EventId<CommandRejected>()) == 1, "damage to a dead agent is rejected and logged");

			Entity gone = agents[4];
			server.RunSetup("remove agent", [&](ApplyContext& ctx) { ctx.Destroy(gone); });
			t.Check(!world.IsAlive(gone) && world.TryGet<TestAgent>(gone) == nullptr, "a stale handle reads as missing");
			t.Check(world.DescribeEntity(gone).find("(gone)") != std::string::npos, "a stale handle prints as gone");
			server.QueueInput(DamageRequested{ gone, 10 });
			server.Step();
			t.Check(world.Stats().commandsRejected == 2, "a command on a stale handle is rejected, not applied");
			server.RunSetup("new agent", [&](ApplyContext& ctx) { ctx.Create(); });
			t.Check(!world.IsAlive(gone), "reusing the index does not revive the old handle");
		}

		void TestCascade(TestContext& t)
		{
			t.Section("cascade cap");
			Server server(BuildTypes(), 1);
			World& world = server.GetWorld();
			const TypeId chain = world.Types().EventId<ChainLink>();
			server.QueueInput(ChainLink{ 0, 20 });
			server.Step();
			int maxDepth = -1;
			for (size_t i = 0; i < world.Log().Size(); ++i)
				if (const ChainLink* link = world.Log().At(i).As<ChainLink>(world.Types()))
					maxDepth = std::max(maxDepth, link->depth);
			t.Check(world.Stats().cascadeOverflows == 1, "the chain overflows the cap once in the first step");
			t.Check(maxDepth == 8, "events beyond the cap are logged but not yet processed (max depth %d)", maxDepth);
			server.Step();
			server.Step();
			t.Check(CountEvents(world, chain) == 21, "the deferred chain completes in later steps (%zu links)", CountEvents(world, chain));
			bool contiguous = true;
			for (size_t i = 1; i < world.Log().Size(); ++i)
				contiguous &= world.Log().At(i).id == world.Log().At(i - 1).id + 1;
			t.Check(contiguous, "event ids stay contiguous");
		}

		void TestScheduledEvents(TestContext& t)
		{
			t.Section("scheduled events");
			Server server(BuildTypes(), 1);
			World& world = server.GetWorld();
			server.QueueInput(Ping{ 3 });
			for (int i = 0; i < 30; ++i)
				server.Step();
			std::vector<Tick> ticks;
			for (size_t i = 0; i < world.Log().Size(); ++i)
				if (world.Log().At(i).As<Ping>(world.Types()))
					ticks.push_back(world.Log().At(i).tick);
			t.Check(ticks == std::vector<Tick>{ 1, 6, 11, 16 }, "a rule's scheduled event fires at its tick, chained 3 times");

			// Big steps fire each scheduled event at its own tick.
			Server ff(BuildTypes(), 1);
			ff.QueueInput(Ping{ 3 });
			ff.Step(25);
			ticks.clear();
			for (size_t i = 0; i < ff.GetWorld().Log().Size(); ++i)
				if (ff.GetWorld().Log().At(i).As<Ping>(ff.GetWorld().Types()))
					ticks.push_back(ff.GetWorld().Log().At(i).tick);
			t.Check(ticks == std::vector<Tick>{ 1, 6, 11, 16 } && ff.GetWorld().Now() == 25, "a 25-tick step fires the same events at the same ticks");
		}

		// Runs a scripted world for `ticks`, returning the hash after every tick.
		std::vector<uint64_t> RunScripted(uint64_t seed, int ticks)
		{
			Server server(BuildTypes(), seed);
			const std::vector<Entity> agents = SetupAgents(server, 40, 3.0f);
			std::vector<uint64_t> hashes;
			for (int i = 0; i < ticks; ++i)
			{
				ScriptedInputs(server, agents);
				server.Step();
				hashes.push_back(server.GetWorld().StateHash());
			}
			return hashes;
		}

		void TestDeterminism(TestContext& t)
		{
			t.Section("determinism");
			const auto a = RunScripted(42, 1500);
			const auto b = RunScripted(42, 1500);
			const auto c = RunScripted(43, 1500);
			size_t firstDiff = a.size();
			for (size_t i = 0; i < a.size(); ++i)
				if (a[i] != b[i])
				{
					firstDiff = i;
					break;
				}
			t.Check(firstDiff == a.size(), "same seed and inputs: identical hash every tick (first difference at tick %zu)", firstDiff + 1);
			t.Check(a.back() != c.back(), "a different seed gives a different world");
		}

		void TestSaveRoundTrip(TestContext& t)
		{
			t.Section("save round trip");
			auto types = BuildTypes();
			Server original(types, 5);
			const std::vector<Entity> agents = SetupAgents(original, 40, 3.0f);
			original.QueueInput(DamageRequested{ agents[3], 500 });
			for (int i = 0; i < 300; ++i)
			{
				ScriptedInputs(original, agents);
				original.Step();
			}
			original.QueueInput(ChainLink{ 0, 12 }); // leaves deferred events across the save
			original.Step();

			const std::vector<uint8_t> blob = original.GetWorld().Save(true);
			Server loaded(types, 999);
			std::string error;
			const bool ok = loaded.Load(blob.data(), blob.size(), error);
			t.Check(ok, "load succeeds: %s", error.c_str());
			if (!ok)
				return;
			t.Check(loaded.GetWorld().StateHash() == original.GetWorld().StateHash(), "loaded state hashes equal");
			t.Check(loaded.GetWorld().Save(true) == blob, "save of the loaded world is byte-identical");
			const TestAgent* dead = loaded.GetWorld().TryGet<TestAgent>(agents[3]);
			t.Check(dead && dead->dead, "a killed agent stays dead across save and load");
			if (dead)
				t.Check(loaded.GetWorld().WhyTrace(dead->deathCause).find("DamageRequested") != std::string::npos, "why-trace works on a loaded world (log is saved)");

			bool identical = true;
			for (int i = 0; i < 1500; ++i)
			{
				ScriptedInputs(original, agents);
				ScriptedInputs(loaded, agents);
				original.Step();
				loaded.Step();
				if (original.GetWorld().StateHash() != loaded.GetWorld().StateHash())
				{
					identical = false;
					t.Check(false, "continued runs diverge at %s", World::FormatTick(original.GetWorld().Now()).c_str());
					break;
				}
			}
			t.Check(identical, "saved-and-loaded run matches the uninterrupted run for 1500 ticks");
			const auto [deadA, deadB] = std::pair{ CountEvents(original.GetWorld(), types->EventId<AgentDied>()), CountEvents(loaded.GetWorld(), types->EventId<AgentDied>()) };
			t.Note("deaths in the continued runs: %zu / %zu (events in log %zu)", deadA, deadB, original.GetWorld().Log().Size());

			std::vector<uint8_t> corrupt = blob;
			corrupt.resize(corrupt.size() / 2);
			std::unique_ptr<World> bad = World::Load(types, corrupt.data(), corrupt.size(), error);
			t.Check(bad == nullptr && !error.empty(), "a truncated snapshot is rejected (%s)", error.c_str());
		}

		void TestReplay(TestContext& t)
		{
			t.Section("replay");
			auto types = BuildTypes();
			Server server(types, 11);
			const std::vector<Entity> agents = SetupAgents(server, 30, 3.0f);
			for (int i = 0; i < 50; ++i)
				server.Step();
			server.StartRecording(25);
			for (int i = 0; i < 800; ++i)
			{
				ScriptedInputs(server, agents);
				server.Step(i % 100 == 50 ? 10 : 1); // include fast-forward steps
			}
			Recording recording = server.StopRecording();

			std::string error;
			Recording reloaded;
			const bool ok = Recording::Deserialize(*types, recording.Serialize(*types), reloaded, error);
			t.Check(ok, "recording serializes and loads: %s", error.c_str());
			bool identical = false;
			const std::string report = ReplayRecording(types, reloaded, identical);
			t.Check(identical, "replay reproduces every checkpoint:\n%s", report.c_str());

			if (!reloaded.inputs.empty())
			{
				Recording tampered = reloaded;
				reinterpret_cast<DamageRequested*>(tampered.inputs[0].payload.data())->amount += 1;
				bool tamperedIdentical = true;
				ReplayRecording(types, tampered, tamperedIdentical);
				t.Check(!tamperedIdentical, "a changed input is detected as divergence");
			}
		}

		void TestInvariantsAndTunables(TestContext& t)
		{
			t.Section("invariants and tunables");
			auto types = BuildTypes();
			Server server(types, 3);
			const std::vector<Entity> agents = SetupAgents(server, 3);
			server.GetWorld().checkInvariants = true;
			server.Step();
			t.Check(server.GetWorld().LastInvariantFailures().empty(), "a healthy world passes");

			const std::string reply = server.RunTextCommand("set test.allow_negative_health 1");
			server.QueueInput(DamageRequested{ agents[0], 130 });
			server.Step();
			t.Check(server.GetWorld().Tunable(testModule.allowNegative) == 1.0f, "a tunable changes through a logged input (%s)", reply.c_str());
			t.Check(server.GetWorld().LastInvariantFailures().size() == 1, "negative health is caught by the invariant");
			if (!server.GetWorld().LastInvariantFailures().empty())
				t.Note("invariant message: %s", server.GetWorld().LastInvariantFailures()[0].c_str());
			server.Step();
			t.Check(server.GetWorld().Stats().invariantFailures == 2, "it keeps failing while violated");
		}

		void TestTextCommands(TestContext& t)
		{
			t.Section("text commands");
			Server server(BuildTypes(), 3);
			const std::vector<Entity> agents = SetupAgents(server, 3);
			server.QueueInput(DamageRequested{ agents[2], 200 });
			server.Step();
			const std::string dump = server.RunTextCommand("dump #" + std::to_string(EntityIndex(agents[2])));
			t.Check(dump.find("TestAgent:") != std::string::npos && dump.find("dead=1") != std::string::npos && dump.find("agent2") != std::string::npos, "dump:\n%s", dump.c_str());
			const std::string log = server.RunTextCommand("log 5");
			t.Check(log.find("AgentDied{agent=agent2(") != std::string::npos, "log names entities by label:\n%s", log.c_str());
			t.Check(server.RunTextCommand("scheduler").find("HungerCritical") != std::string::npos, "scheduler lists pending hunger events");
			t.Check(server.RunTextCommand("state").find("TestAgent=3") != std::string::npos, "state counts components");
			t.Check(server.RunTextCommand("hash").find("hash") != std::string::npos && server.RunTextCommand("bogus").find("unknown") != std::string::npos, "hash and unknown commands answer");
		}

		void TestHostPump(TestContext& t)
		{
			t.Section("host: pump mode");
			auto types = BuildTypes();
			Host host(types, 21);
			const std::vector<Entity> agents = SetupAgents(host.ServerUnsafe(), 4);
			host.maxStepsPerService = 1000;
			host.Start(Host::Mode::Pump);

			host.SendEvent(DamageRequested{ agents[0], 999 });
			host.Pump(0.1);
			t.Check(host.CurrentTick() == 1, "0.1 s of real time is one tick (tick %lld)", static_cast<long long>(host.CurrentTick()));
			bool sawDeath = false;
			for (const Message& m : host.Receive())
				if (const AgentDied* d = m.As<AgentDied>(*types))
					sawDeath = d->agent == agents[0] && m.tick == 1;
			t.Check(sawDeath, "the death arrives as an output message");

			host.Pump(0.55);
			t.Check(host.CurrentTick() == 6, "the accumulator carries fractions (tick %lld)", static_cast<long long>(host.CurrentTick()));

			const uint32_t textId = host.SendRequest(Message::Kind::Text, "hash");
			const uint32_t saveId = host.SendRequest(Message::Kind::Save);
			host.Pump(0.0);
			std::vector<uint8_t> blob;
			bool textReply = false;
			for (Message& m : host.Receive())
			{
				textReply |= m.kind == Message::Kind::Reply && m.requestId == textId && m.text.find("hash") != std::string::npos;
				if (m.kind == Message::Kind::Reply && m.requestId == saveId)
					blob = m.blob;
			}
			t.Check(textReply && !blob.empty(), "text and save requests are answered at the tick boundary");

			host.Send({ Message::Kind::Pause });
			host.Pump(5.0);
			t.Check(host.CurrentTick() == 6 && host.IsPaused(), "paused: real time does not advance the sim");
			Message step;
			step.kind = Message::Kind::Step;
			step.value = 3;
			host.Send(step);
			host.Pump(0.0);
			t.Check(host.CurrentTick() == 9, "step runs exactly 3 ticks while paused");
			host.Send({ Message::Kind::Resume });

			Message fast;
			fast.kind = Message::Kind::SetSpeed;
			fast.value = 100.0;
			fast.stepTicks = 10;
			host.Send(fast);
			host.Pump(1.0);
			t.Check(host.CurrentTick() == 9 + 1000, "fast-forward: 100x for 1 s in 10-tick steps (tick %lld)", static_cast<long long>(host.CurrentTick()));

			host.Send(fast);
			host.SendEvent(DamageRequested{ agents[1], 999 }); // the death raises a wake event
			host.Pump(1.0);
			bool woke = false;
			for (const Message& m : host.Receive())
				woke |= m.kind == Message::Kind::Woke;
			t.Check(woke && host.CurrentTick() == 1009 + 10, "a wake event stops fast-forward in the step it happened (tick %lld)", static_cast<long long>(host.CurrentTick()));

			Message load;
			load.kind = Message::Kind::Load;
			load.requestId = 77;
			load.blob = blob;
			host.Send(load);
			host.Pump(0.0);
			bool loaded = false;
			for (const Message& m : host.Receive())
				loaded |= m.kind == Message::Kind::Reply && m.requestId == 77;
			t.Check(loaded && host.CurrentTick() == 6, "load replaces the world at a tick boundary");

			Message bogus;
			bogus.kind = Message::Kind::Event;
			bogus.type = types->EventId<AgentDied>(); // not an input
			host.Send(bogus);
			host.Pump(0.0);
			bool rejected = false;
			for (const Message& m : host.Receive())
				rejected |= m.kind == Message::Kind::Error;
			t.Check(rejected, "a non-input event from the client is refused");
			host.Stop();
		}

		void TestHostThread(TestContext& t)
		{
			t.Section("host: thread mode");
			auto types = BuildTypes();
			Host host(types, 31);
			const std::vector<Entity> agents = SetupAgents(host.ServerUnsafe(), 10);
			host.Start(Host::Mode::Thread);

			auto waitFor = [&](auto predicate, double seconds)
			{
				Stopwatch timer;
				while (timer.Seconds() < seconds)
				{
					if (predicate())
						return true;
					std::this_thread::sleep_for(std::chrono::milliseconds(5));
				}
				return predicate();
			};

			host.SendEvent(DamageRequested{ agents[5], 999 });
			bool sawDeath = false;
			waitFor([&]
			{
				for (const Message& m : host.Receive())
					if (const AgentDied* d = m.As<AgentDied>(*types))
						sawDeath |= d->agent == agents[5];
				return sawDeath;
			}, 2.0);
			t.Check(sawDeath, "an input crosses to the sim thread and the output comes back");
			t.Check(host.CurrentTick() > 0, "the sim thread advances in real time");

			host.Send({ Message::Kind::Pause });
			waitFor([&] { return host.IsPaused(); }, 1.0);
			const Tick pausedAt = host.CurrentTick();
			std::this_thread::sleep_for(std::chrono::milliseconds(300));
			t.Check(host.IsPaused() && host.CurrentTick() == pausedAt, "pause holds the tick");

			Message step;
			step.kind = Message::Kind::Step;
			step.value = 5;
			host.Send(step);
			t.Check(waitFor([&] { return host.CurrentTick() == pausedAt + 5; }, 1.0), "step while paused advances exactly 5 ticks");

			const uint32_t saveId = host.SendRequest(Message::Kind::Save);
			std::vector<uint8_t> blob;
			Tick savedAt = -1;
			waitFor([&]
			{
				for (Message& m : host.Receive())
					if (m.kind == Message::Kind::Reply && m.requestId == saveId)
					{
						blob = std::move(m.blob);
						savedAt = m.tick;
					}
				return !blob.empty();
			}, 2.0);
			t.Check(!blob.empty() && savedAt == pausedAt + 5, "save barrier returns a snapshot of the paused tick");
			host.Send({ Message::Kind::Resume });
			std::this_thread::sleep_for(std::chrono::milliseconds(150));
			host.Stop();
			t.Check(!host.IsRunning(), "stop joins the thread");

			Server check(types, 0);
			std::string error;
			const bool ok = check.Load(blob.data(), blob.size(), error);
			const TestAgent* dead = ok ? check.GetWorld().TryGet<TestAgent>(agents[5]) : nullptr;
			t.Check(ok && dead && dead->dead && check.GetWorld().Now() == savedAt, "the snapshot loads with the dead agent and the right tick");
		}

		void TestThroughput(TestContext& t)
		{
			t.Section("throughput (informational)");
			auto types = BuildTypes();
			Server server(types, 77);
			const std::vector<Entity> agents = SetupAgents(server, 400, 20.0f);
			Stopwatch timer;
			const EventId before = server.GetWorld().LastEventId();
			for (int i = 0; i < 6000; ++i)
				server.Step();
			const double seconds = timer.Seconds();
			const EventId events = server.GetWorld().LastEventId() - before;
			t.Note("400 agents, 6000 ticks (10 world minutes): %llu events, %llu rules, %.3f s -> %.0f ticks/s, %.0f events/s",
				static_cast<unsigned long long>(events), static_cast<unsigned long long>(server.GetWorld().Stats().rulesRun), seconds, 6000.0 / seconds, events / seconds);
			Stopwatch hashTimer;
			const uint64_t hash = server.GetWorld().StateHash();
			const std::vector<uint8_t> blob = server.GetWorld().Save(true);
			t.Note("state hash %016llx in %.2f ms; full save %zu bytes", static_cast<unsigned long long>(hash), hashTimer.Seconds() * 1000.0, blob.size());
			t.Check(events > 0, "the toy world produced events");
		}
	}

	void RunKernelTests(TestContext& context)
	{
		TestArchives(context);
		TestRandom(context);
		TestLazyValue(context);
		TestScheduler(context);
		TestEventLog(context);
		TestRegistration(context);
		TestStagger(context);
		TestWorldBasics(context);
		TestCascade(context);
		TestScheduledEvents(context);
		TestDeterminism(context);
		TestSaveRoundTrip(context);
		TestReplay(context);
		TestInvariantsAndTunables(context);
		TestTextCommands(context);
		TestHostPump(context);
		TestHostThread(context);
		TestThroughput(context);
	}
}

#endif
