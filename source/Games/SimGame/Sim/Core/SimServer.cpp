#include "SimServer.h"

#include <Logger.hpp>

#include <cstdio>
#include <cstring>
#include <sstream>

namespace sim
{
	namespace
	{
		constexpr uint32_t RecordingMagic = 0x43524953; // "SIRC"
		constexpr uint32_t RecordingVersion = 1;

		const char* TextHelp =
			"sim commands:\n"
			"  state              every entity, scheduler head and stats\n"
			"  dump <#index>      one entity's components\n"
			"  why <event id>     cause chain of an event\n"
			"  timeline <#index>  logged events that reference an entity\n"
			"  log [count]        newest events (default 30)\n"
			"  scheduler          pending scheduled events\n"
			"  hash               state hash of the current tick\n"
			"  types              registered modules, records, rules, systems, tunables\n"
			"  tunables           current tunable values\n"
			"  set <name> <value> change a tunable at the next tick (logged input)\n";

		bool ParseEntityIndex(const std::string& text, uint32_t& index)
		{
			const char* start = text.c_str();
			if (*start == '#')
				++start;
			char* end = nullptr;
			const unsigned long value = std::strtoul(start, &end, 10);
			if (end == start)
				return false;
			index = static_cast<uint32_t>(value);
			return true;
		}
	}

	// ---- Recording -------------------------------------------------------------------------------------------------

	std::vector<uint8_t> Recording::Serialize(const TypeRegistry& types) const
	{
		BinaryWriter ar;
		uint32_t magic = RecordingMagic, version = RecordingVersion;
		ar.Primitive(nullptr, magic);
		ar.Primitive(nullptr, version);
		ar.Write(start);
		uint32_t inputCount = static_cast<uint32_t>(inputs.size());
		ar.Primitive(nullptr, inputCount);
		for (const InputRecord& input : inputs)
		{
			Tick tick = input.tick;
			std::string name = types.Event(input.type).name;
			ar.Primitive(nullptr, tick);
			ar.String(nullptr, name);
			BinaryWriter payload;
			types.Event(input.type).ops.write(payload, input.payload.data());
			ar.Write(payload.bytes);
		}
		ar.Write(steps);
		ar.Write(checkpoints);
		Tick interval = checkpointInterval, end = endTick;
		ar.Primitive(nullptr, interval);
		ar.Primitive(nullptr, end);
		return std::move(ar.bytes);
	}

	bool Recording::Deserialize(const TypeRegistry& types, const std::vector<uint8_t>& bytes, Recording& out, std::string& error)
	{
		BinaryReader ar(bytes.data(), bytes.size());
		uint32_t magic = 0, version = 0;
		ar.Primitive(nullptr, magic);
		ar.Primitive(nullptr, version);
		if (magic != RecordingMagic || version != RecordingVersion)
		{
			error = "not a sim recording, or an unsupported version";
			return false;
		}
		out = Recording{};
		ar.Read(out.start);
		uint32_t inputCount = 0;
		ar.Primitive(nullptr, inputCount);
		for (uint32_t i = 0; i < inputCount && !ar.Failed(); ++i)
		{
			InputRecord input;
			std::string name;
			std::vector<uint8_t> payload;
			ar.Primitive(nullptr, input.tick);
			ar.String(nullptr, name);
			ar.Read(payload);
			input.type = types.FindEvent(name);
			if (input.type == InvalidType)
			{
				error = "recording uses unknown input event " + name;
				return false;
			}
			BinaryReader payloadReader(payload.data(), payload.size());
			types.Event(input.type).ops.read(payloadReader, input.payload.data());
			if (payloadReader.Failed() || !payloadReader.AtEnd())
			{
				error = "input " + name + " does not match its current fields";
				return false;
			}
			out.inputs.push_back(input);
		}
		ar.Read(out.steps);
		ar.Read(out.checkpoints);
		ar.Primitive(nullptr, out.checkpointInterval);
		ar.Primitive(nullptr, out.endTick);
		if (ar.Failed())
		{
			error = "recording is truncated";
			return false;
		}
		return true;
	}

	// ---- Server ----------------------------------------------------------------------------------------------------

	Server::Server(std::shared_ptr<const TypeRegistry> types, uint64_t seed) : types(types), world(std::make_unique<World>(types, seed))
	{
	}

	bool Server::QueueInput(TypeId type, const void* payload, size_t size)
	{
		if (type >= types->Events().size() || !(types->Event(type).flags & EventInput) || size != types->Event(type).ops.size)
		{
			Logger::Error("sim: rejected input of type %u (not a registered input event)", type);
			return false;
		}
		InputEvent input;
		input.type = type;
		std::memcpy(input.payload.data(), payload, size);
		pending.push_back(input);
		return true;
	}

	void Server::Step(Tick ticks)
	{
		if (ticks < 1)
			ticks = 1;
		const Tick before = world->Now();
		if (recording)
		{
			for (const InputEvent& input : pending)
			{
				InputRecord record;
				record.tick = before + 1;
				record.type = input.type;
				record.payload = input.payload;
				current.inputs.push_back(record);
			}
			if (ticks != 1)
				current.steps.push_back({ before, ticks });
		}

		world->Step(ticks, pending);
		pending.clear();

		if (recording)
		{
			const Tick interval = current.checkpointInterval;
			if (world->Now() / interval != before / interval)
				current.checkpoints.push_back({ world->Now(), world->StateHash() });
			current.endTick = world->Now();
		}
	}

	bool Server::Load(const uint8_t* data, size_t size, std::string& error)
	{
		std::unique_ptr<World> loaded = World::Load(types, data, size, error);
		if (!loaded)
			return false;
		loaded->checkInvariants = world->checkInvariants;
		loaded->cascadeDepthCap = world->cascadeDepthCap;
		world = std::move(loaded);
		pending.clear();
		recording = false;
		return true;
	}

	void Server::StartRecording(Tick checkpointInterval)
	{
		current = Recording{};
		current.start = world->Save(true);
		current.checkpointInterval = checkpointInterval < 1 ? 1 : checkpointInterval;
		current.endTick = world->Now();
		current.checkpoints.push_back({ world->Now(), world->StateHash() });
		recording = true;
	}

	Recording Server::StopRecording()
	{
		if (recording && (current.checkpoints.empty() || current.checkpoints.back().tick != world->Now()))
			current.checkpoints.push_back({ world->Now(), world->StateHash() });
		recording = false;
		Recording result = std::move(current);
		current = Recording{};
		return result;
	}

	std::string Server::RunTextCommand(const std::string& command)
	{
		std::istringstream stream(command);
		std::string verb, argument, value;
		stream >> verb >> argument >> value;
		World& w = *world;

		if (verb.empty() || verb == "help")
			return TextHelp;
		if (verb == "state")
			return w.DumpState();
		if (verb == "scheduler")
			return w.DumpScheduler(200);
		if (verb == "types")
			return types->Describe();
		if (verb == "hash")
		{
			char buffer[96];
			std::snprintf(buffer, sizeof(buffer), "%s hash %016llx\n", World::FormatTick(w.Now()).c_str(), static_cast<unsigned long long>(w.StateHash()));
			return buffer;
		}
		if (verb == "dump" || verb == "timeline")
		{
			uint32_t index = 0;
			if (!ParseEntityIndex(argument, index))
				return "usage: " + verb + " <#index>\n";
			const Entity entity = w.FindByIndex(index);
			if (entity == NullEntity)
				return "no alive entity with index " + std::to_string(index) + "\n";
			return verb == "dump" ? w.DumpEntity(entity) : w.Timeline(entity);
		}
		if (verb == "why")
		{
			const char* start = argument.c_str();
			if (*start == '#')
				++start;
			return w.WhyTrace(std::strtoull(start, nullptr, 10));
		}
		if (verb == "log")
		{
			size_t count = argument.empty() ? 30 : std::strtoul(argument.c_str(), nullptr, 10);
			const EventLog& log = w.Log();
			std::string out;
			const size_t first = log.Size() > count ? log.Size() - count : 0;
			for (size_t i = first; i < log.Size(); ++i)
				out += w.DescribeEvent(log.At(i)) + "  <- #" + std::to_string(log.At(i).cause) + "\n";
			return out;
		}
		if (verb == "tunables")
		{
			std::string out;
			char line[256];
			for (size_t i = 0; i < types->Tunables().size(); ++i)
			{
				const TunableInfo& info = types->Tunables()[i];
				std::snprintf(line, sizeof(line), "  %-36s %g (default %g)\n", info.name.c_str(), w.Tunable(static_cast<TunableId>(i)), info.defaultValue);
				out += line;
			}
			return out;
		}
		if (verb == "set")
		{
			const TunableId id = types->FindTunable(argument);
			if (id == static_cast<TunableId>(-1) || value.empty())
				return "usage: set <tunable> <value> (see 'tunables')\n";
			SetTunableRequested request;
			request.tunable = id;
			request.value = std::strtof(value.c_str(), nullptr);
			QueueInput(request);
			return "queued " + argument + " = " + value + "\n";
		}
		return "unknown sim command '" + verb + "'\n" + TextHelp;
	}

	// ---- Replay ----------------------------------------------------------------------------------------------------

	std::string ReplayRecording(std::shared_ptr<const TypeRegistry> types, const Recording& recording, bool& identical)
	{
		identical = false;
		Server server(types, 0);
		std::string error;
		if (!server.Load(recording.start.data(), recording.start.size(), error))
			return "replay: cannot load the start snapshot: " + error + "\n";

		std::string report;
		size_t nextInput = 0, nextStep = 0, nextCheckpoint = 0, mismatches = 0, compared = 0;
		auto compareCheckpoints = [&]()
		{
			const Tick now = server.GetWorld().Now();
			while (nextCheckpoint < recording.checkpoints.size() && recording.checkpoints[nextCheckpoint].tick <= now)
			{
				const Checkpoint& expected = recording.checkpoints[nextCheckpoint++];
				if (expected.tick != now)
					continue; // the replay stepped past it; cannot compare
				++compared;
				const uint64_t actual = server.GetWorld().StateHash();
				if (actual != expected.hash && mismatches++ < 10)
				{
					char line[160];
					std::snprintf(line, sizeof(line), "  MISMATCH at %s: recorded %016llx, replay %016llx\n", World::FormatTick(now).c_str(),
						static_cast<unsigned long long>(expected.hash), static_cast<unsigned long long>(actual));
					report += line;
				}
			}
		};

		compareCheckpoints();
		while (server.GetWorld().Now() < recording.endTick)
		{
			const Tick before = server.GetWorld().Now();
			while (nextInput < recording.inputs.size() && recording.inputs[nextInput].tick <= before + 1)
			{
				const InputRecord& input = recording.inputs[nextInput++];
				server.QueueInput(input.type, input.payload.data(), types->Event(input.type).ops.size);
			}
			Tick ticks = 1;
			if (nextStep < recording.steps.size() && recording.steps[nextStep].start == before)
				ticks = recording.steps[nextStep++].ticks;
			server.Step(ticks);
			compareCheckpoints();
		}

		identical = mismatches == 0 && compared == recording.checkpoints.size();
		char summary[200];
		std::snprintf(summary, sizeof(summary), "replay to %s: %zu inputs, %zu/%zu checkpoints compared, %zu mismatches -> %s\n", World::FormatTick(recording.endTick).c_str(),
			recording.inputs.size(), compared, recording.checkpoints.size(), mismatches, identical ? "IDENTICAL" : "DIVERGED");
		return summary + report;
	}
}
