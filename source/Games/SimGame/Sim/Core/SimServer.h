#pragma once

// The sim as a server (addendum section 3): owns the World, queues client inputs, applies them at the start of the
// next step, records what it applied (input log) and answers text commands. Single-threaded; Host decides which
// thread calls it.

#include "SimWorld.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace sim
{
	struct InputRecord
	{
		Tick tick = 0; // the step's first tick, when the input was applied
		TypeId type = InvalidType;
		alignas(8) std::array<uint8_t, MaxPayloadSize> payload{};
	};

	// A step that was not exactly one tick (fast-forward). Replays must step identically.
	struct StepRecord
	{
		Tick start = 0; // world tick before the step
		Tick ticks = 1;
		template<class Ar> void Visit(Ar& ar) { ar("start", start); ar("ticks", ticks); }
	};

	struct Checkpoint
	{
		Tick tick = 0;
		uint64_t hash = 0;
		template<class Ar> void Visit(Ar& ar) { ar("tick", tick); ar("hash", hash); }
	};

	// Everything needed to reproduce a run: starting snapshot, inputs, step sizes, and state hashes to compare.
	struct Recording
	{
		std::vector<uint8_t> start;
		std::vector<InputRecord> inputs;
		std::vector<StepRecord> steps;
		std::vector<Checkpoint> checkpoints;
		Tick checkpointInterval = 10;
		Tick endTick = 0;

		std::vector<uint8_t> Serialize(const TypeRegistry& types) const;
		static bool Deserialize(const TypeRegistry& types, const std::vector<uint8_t>& bytes, Recording& out, std::string& error);
	};

	class Server
	{
	public:
		Server(std::shared_ptr<const TypeRegistry> types, uint64_t seed);

		World& GetWorld() { return *world; }
		const World& GetWorld() const { return *world; }
		const TypeRegistry& Types() const { return *types; }

		template<class Fn> EventId RunSetup(const char* reason, Fn fn) { return world->RunSetup(reason, fn); }

		// Queues a client input (an event registered with EventInput) for the next step.
		template<class E> bool QueueInput(const E& payload)
		{
			const TypeId type = types->EventId<E>();
			if (type == InvalidType)
			{
				world->ReportUnregistered("event", E::TypeName);
				return false;
			}
			return QueueInput(type, &payload, sizeof(E));
		}
		bool QueueInput(TypeId type, const void* payload, size_t size);

		void Step(Tick ticks = 1);

		std::vector<EventRecord> TakeOutputs() { return world->TakeOutputs(); }

		// Replaces the world. Pending inputs are dropped; recording stops.
		bool Load(const uint8_t* data, size_t size, std::string& error);

		// Recording: snapshot now, then log every input and step until StopRecording.
		void StartRecording(Tick checkpointInterval = 10);
		Recording StopRecording();
		bool IsRecording() const { return recording; }

		// Text commands for consoles and tools; see the help text in the .cpp.
		std::string RunTextCommand(const std::string& command);

	private:
		std::shared_ptr<const TypeRegistry> types;
		std::unique_ptr<World> world;
		std::vector<InputEvent> pending;

		bool recording = false;
		Recording current;
	};

	// Loads the recording's start, re-applies its inputs and step sizes, and compares state hashes at every checkpoint.
	// Returns a text report; `identical` is true when every checkpoint matched.
	std::string ReplayRecording(std::shared_ptr<const TypeRegistry> types, const Recording& recording, bool& identical);
}
