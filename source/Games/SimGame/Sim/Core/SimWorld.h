#pragma once

// The simulation state and the tick pipeline. See Docs/SIM_CONTRACT.md for the rules this enforces.
//
// One step (World::Step):
//   1. cascade overflow from the previous step, then client inputs, at the step's first tick; waves
//   2. scheduled events due within the step, in (tick, sequence) order, each at its own tick; waves per tick
//   3. systems at the step's last tick, in registration order; waves
//   4. invariants (when enabled)
// A wave runs every rule of every pending event against the unchanged world, collecting commands, then applies the
// commands in order. Events those commands emit form the next wave. Waves deeper than the cascade cap are deferred to
// the next step and counted.

#include "LazyValue.h"
#include "SimArchive.h"
#include "SimEventLog.h"
#include "SimRandom.h"
#include "SimScheduler.h"
#include "SimTypeRegistry.h"
#include "SimTypes.h"

#include <entt/entity/registry.hpp>

#include <memory>
#include <string>
#include <vector>

namespace sim
{
	// Kernel component: a readable name for tools. Not a game concept; modules set it to whatever helps debugging.
	struct Label
	{
		static constexpr const char* TypeName = "Label";
		std::string text;
		template<class Ar> void Visit(Ar& ar) { ar("text", text); }
	};

	// Kernel events.
	struct SetupStarted
	{
		static constexpr const char* TypeName = "SetupStarted";
		FixedString<40> reason;
		template<class Ar> void Visit(Ar& ar) { ar("reason", reason); }
	};

	struct CommandRejected
	{
		static constexpr const char* TypeName = "CommandRejected";
		TypeId command = InvalidType;
		FixedString<40> reason;
		template<class Ar> void Visit(Ar& ar) { ar("command", command); ar("reason", reason); }
	};

	// Input: changes a tunable at a tick boundary, so live tuning is logged and replayable.
	struct SetTunableRequested
	{
		static constexpr const char* TypeName = "SetTunableRequested";
		TunableId tunable = 0;
		float value = 0.0f;
		template<class Ar> void Visit(Ar& ar) { ar("tunable", tunable); ar("value", value); }
	};

	struct CommandEntry
	{
		enum class Kind : uint8_t { Emit, Schedule, Custom };

		Kind kind = Kind::Custom;
		TypeId type = InvalidType;
		RuleId rule = NoRule;
		EventId cause = NoEvent;
		Tick at = 0;
		alignas(8) std::array<uint8_t, MaxPayloadSize> payload{};
	};

	// An input waiting for the next step (Server and Host build these).
	struct InputEvent
	{
		TypeId type = InvalidType;
		alignas(8) std::array<uint8_t, MaxPayloadSize> payload{};
	};

	class World;

	// What a rule gets: a read-only world, the event, and a command buffer (rule R7).
	class RuleContext
	{
	public:
		RuleContext(const World& world, const EventRecord& event, RuleId rule, std::vector<CommandEntry>& commands)
			: world(world), event(event), rule(rule), commands(commands) {}

		const World& world;
		const EventRecord& event;
		const RuleId rule;

		Tick Now() const;

		// Deterministic randomness for this (event, rule) pair. Every call returns the same sequence; pass a salt
		// for independent draws.
		Rng Random(uint64_t salt = 0) const;

		template<class E> void Emit(const E& payload) { Push(CommandEntry::Kind::Emit, EventType<E>(), &payload, sizeof(E), 0); }
		// Fire-and-forget. Something that must be cancellable is scheduled by a command (ApplyContext::Schedule).
		template<class E> void Schedule(Tick at, const E& payload) { Push(CommandEntry::Kind::Schedule, EventType<E>(), &payload, sizeof(E), at); }
		template<class C> void Command(const C& payload) { Push(CommandEntry::Kind::Custom, CommandType<C>(), &payload, sizeof(C), 0); }

	private:
		std::vector<CommandEntry>& commands;

		template<class E> TypeId EventType() const;
		template<class C> TypeId CommandType() const;
		void Push(CommandEntry::Kind kind, TypeId type, const void* payload, size_t size, Tick at);
	};

	// What a command apply function, a system or a setup function gets: mutable access, stamped with a cause.
	class ApplyContext
	{
	public:
		ApplyContext(World& world, EventId cause, RuleId rule, const char* what, TypeId command = InvalidType)
			: world(world), cause(cause), rule(rule), what(what), command(command) {}

		World& world;
		const EventId cause;
		const RuleId rule;
		const char* const what; // command, system or setup name (messages)
		const TypeId command;   // the command being applied, or InvalidType

		Tick Now() const;

		template<class E> EventId Emit(const E& payload);
		template<class E> TimerHandle Schedule(Tick at, const E& payload);
		template<class E> void ScheduleUntracked(Tick at, const E& payload);
		bool Cancel(TimerHandle& handle);
		bool IsPending(const TimerHandle& handle) const;

		Rng& Stream(RandomStreamId stream);

		Entity Create();
		void Destroy(Entity entity);

		// The command could not be applied (stale handle, failed precondition). Logged as a CommandRejected event.
		void Reject(const char* reason);
	};

	struct WorldStats
	{
		uint64_t rulesRun = 0;
		uint64_t commandsApplied = 0;
		uint64_t commandsRejected = 0;
		uint64_t cascadeOverflows = 0;
		uint64_t invariantFailures = 0;
	};

	// True if `entity`'s staggered slot (one tick in every `period`) falls within the step (now - dt, now].
	bool IsStaggerDue(Entity entity, Tick now, Tick dt, Tick period);

	class World
	{
	public:
		World(std::shared_ptr<const TypeRegistry> types, uint64_t seed);
		World(const World&) = delete;
		World& operator=(const World&) = delete;

		const TypeRegistry& Types() const { return *types; }
		const std::shared_ptr<const TypeRegistry>& TypesPtr() const { return types; }
		uint64_t Seed() const { return seed; }
		Tick Now() const { return now; }
		EventId LastEventId() const { return lastEventId; }

		// Mutable registry access belongs to command apply functions, systems and setup. Rules get a const World.
		entt::registry& Registry() { return registry; }
		const entt::registry& Registry() const { return registry; }

		bool IsAlive(Entity entity) const { return entity != NullEntity && registry.valid(entity); }

		// nullptr for a stale or null handle, or when the component is missing. Never asserts.
		template<class T> const T* TryGet(Entity entity) const { return IsAlive(entity) ? registry.try_get<T>(entity) : nullptr; }
		template<class T> T* TryGet(Entity entity) { return IsAlive(entity) ? registry.try_get<T>(entity) : nullptr; }

		const EventLog& Log() const { return log; }
		const Scheduler& GetScheduler() const { return scheduler; }
		const WorldStats& Stats() const { return stats; }

		float Tunable(TunableId id) const { return tunables[id]; }
		void SetTunable(TunableId id, float value);

		// Steps the world by `ticks` (1 normally; more during fast-forward). Inputs are applied at the first tick.
		void Step(Tick ticks, const std::vector<InputEvent>& inputs);

		// Applies `fn` as a logged root (world population, scenario placement, logged exceptions to rule R4).
		template<class Fn> EventId RunSetup(const char* reason, Fn fn);

		// Events flagged EventOutput since the last call.
		std::vector<EventRecord> TakeOutputs();
		// True if an EventWake event was emitted since the last call.
		bool TakeWake();

		// Invariants run after every step when enabled. Failures of the last step:
		bool checkInvariants = false;
		const std::vector<std::string>& LastInvariantFailures() const { return invariantFailures; }

		int cascadeDepthCap = 8;

		// ---- Snapshots. State = everything except the event log and stats.
		std::vector<uint8_t> Save(bool includeLog = true) const;
		// Builds a world from a snapshot made with the same module list (types matched by name). nullptr on error.
		static std::unique_ptr<World> Load(std::shared_ptr<const TypeRegistry> types, const uint8_t* data, size_t size, std::string& error);
		uint64_t StateHash() const;

		// ---- Text tools (addendum section 13: every tool has a text output).
		std::string DescribeEntity(Entity entity) const;
		std::string DumpEntity(Entity entity) const;
		std::string DescribeEvent(const EventRecord& record) const;
		std::string DescribePayload(TypeId type, const void* payload) const;
		std::string WhyTrace(EventId id, int maxDepth = 64) const;
		std::string Timeline(Entity entity, size_t maxEvents = 200) const;
		std::string DumpScheduler(size_t maxEntries = 100) const;
		std::string DumpState() const;
		static std::string FormatTick(Tick tick);
		// Finds an alive entity by index (text commands type "#12").
		Entity FindByIndex(uint32_t index) const;

		void ReportUnregistered(const char* kind, const char* typeName) const;

	private:
		friend class RuleContext;
		friend class ApplyContext;

		std::shared_ptr<const TypeRegistry> types;
		uint64_t seed = 0;
		entt::registry registry;
		Tick now = 0;
		EventId lastEventId = NoEvent;
		Scheduler scheduler;
		EventLog log;
		std::vector<Rng> streams;
		std::vector<float> tunables;

		std::vector<EventRecord> queue;    // next wave
		std::vector<EventRecord> deferred; // cascade overflow, continues next step
		std::vector<EventRecord> outputs;
		std::vector<CommandEntry> commands;
		uint16_t currentWave = 0;
		bool wakeRequested = false;

		WorldStats stats;
		std::vector<std::string> invariantFailures;

		EventId Emit(TypeId type, const void* payload, size_t size, EventId cause, RuleId rule);
		EventId EmitRecord(EventRecord record);
		void ProcessWaves();
		void Apply(const CommandEntry& command);
		void RunInvariants();

		void WriteRecord(BinaryWriter& ar, const EventRecord& record) const;
		void WriteScheduled(BinaryWriter& ar, const ScheduledEvent& entry) const;
	};

	// ---------------------------------------------------------------------------------------------------------------

	template<class T>
	const T* EventRecord::As(const TypeRegistry& registry) const
	{
		return type == registry.EventId<T>() ? reinterpret_cast<const T*>(payload.data()) : nullptr;
	}

	template<class E>
	TypeId RuleContext::EventType() const
	{
		const TypeId type = world.Types().EventId<E>();
		if (type == InvalidType)
			world.ReportUnregistered("event", E::TypeName);
		return type;
	}

	template<class C>
	TypeId RuleContext::CommandType() const
	{
		const TypeId type = world.Types().CommandId<C>();
		if (type == InvalidType)
			world.ReportUnregistered("command", C::TypeName);
		return type;
	}

	template<class E>
	EventId ApplyContext::Emit(const E& payload)
	{
		const TypeId type = world.Types().EventId<E>();
		if (type == InvalidType)
		{
			world.ReportUnregistered("event", E::TypeName);
			return NoEvent;
		}
		return world.Emit(type, &payload, sizeof(E), cause, rule);
	}

	template<class E>
	TimerHandle ApplyContext::Schedule(Tick at, const E& payload)
	{
		const TypeId type = world.Types().EventId<E>();
		if (type == InvalidType)
		{
			world.ReportUnregistered("event", E::TypeName);
			return {};
		}
		return world.scheduler.Schedule(at < world.now ? world.now : at, type, &payload, sizeof(E), cause, rule, true);
	}

	template<class E>
	void ApplyContext::ScheduleUntracked(Tick at, const E& payload)
	{
		const TypeId type = world.Types().EventId<E>();
		if (type == InvalidType)
		{
			world.ReportUnregistered("event", E::TypeName);
			return;
		}
		world.scheduler.Schedule(at < world.now ? world.now : at, type, &payload, sizeof(E), cause, rule, false);
	}

	template<class Fn>
	EventId World::RunSetup(const char* reason, Fn fn)
	{
		SetupStarted started;
		started.reason = reason;
		const EventId root = Emit(types->EventId<SetupStarted>(), &started, sizeof(started), NoEvent, SetupRule);
		{
			ApplyContext context(*this, root, SetupRule, reason);
			fn(context);
		}
		ProcessWaves();
		return root;
	}
}
