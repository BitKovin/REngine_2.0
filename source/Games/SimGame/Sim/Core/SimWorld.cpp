#include "SimWorld.h"

#include <Logger.hpp>

#include <algorithm>
#include <cstdio>
#include <cstring>

namespace sim
{
	namespace
	{
		constexpr uint32_t SnapshotMagic = 0x574D4953; // "SIMW"
		constexpr uint32_t SnapshotVersion = 1;

		struct ApplyTunable
		{
			static constexpr const char* TypeName = "ApplyTunable";
			TunableId tunable = 0;
			float value = 0.0f;
			template<class Ar> void Visit(Ar& ar) { ar("tunable", tunable); ar("value", value); }
		};

		std::string NameEntity(const void* context, Entity entity)
		{
			return static_cast<const World*>(context)->DescribeEntity(entity);
		}

		void WritePayloadBlob(BinaryWriter& ar, const PayloadOps& ops, const void* payload)
		{
			BinaryWriter blob;
			ops.write(blob, payload);
			uint32_t size = static_cast<uint32_t>(blob.bytes.size());
			ar.Primitive(nullptr, size);
			ar.Raw(blob.bytes.data(), blob.bytes.size());
		}

		// Reads a payload blob into `out` (zeroed first). Returns false if the blob does not match the type.
		bool ReadPayloadBlob(BinaryReader& ar, const PayloadOps* ops, std::array<uint8_t, MaxPayloadSize>& out)
		{
			uint32_t size = 0;
			ar.Primitive(nullptr, size);
			if (ar.Failed() || size > ar.Remaining())
				return false;
			out.fill(0);
			const size_t start = ar.Offset();
			bool ok = false;
			if (ops)
			{
				std::vector<uint8_t> bytes(size);
				ar.Raw(bytes.data(), size);
				BinaryReader payload(bytes.data(), bytes.size());
				ops->read(payload, out.data());
				ok = !payload.Failed() && payload.AtEnd();
			}
			else
				ar.Skip(size);
			return ok && ar.Offset() == start + size;
		}
	}

	void RegisterKernelTypes(Registration& registration)
	{
		registration.Component<Label>();
		registration.Event<SetupStarted>(EventSignificant);
		registration.Event<CommandRejected>();
		registration.Event<SetTunableRequested>(EventInput);
		registration.Command<ApplyTunable>([](ApplyContext& context, const ApplyTunable& command)
		{
			const auto& tunables = context.world.Types().Tunables();
			if (command.tunable >= tunables.size())
			{
				context.Reject("unknown tunable");
				return;
			}
			context.world.SetTunable(command.tunable, command.value);
		});
		registration.Rule<SetTunableRequested>("set_tunable", [](RuleContext& context, const SetTunableRequested& event)
		{
			context.Command(ApplyTunable{ event.tunable, event.value });
		});
	}

	bool IsStaggerDue(Entity entity, Tick now, Tick dt, Tick period)
	{
		if (period <= 1 || dt >= period)
			return true;
		const Tick phase = static_cast<Tick>(EntityIndex(entity) % static_cast<uint64_t>(period));
		const Tick firstTick = now - dt + 1;
		const Tick offset = ((phase - firstTick) % period + period) % period;
		return firstTick + offset <= now;
	}

	// ---- RuleContext / ApplyContext ---------------------------------------------------------------------------------

	Tick RuleContext::Now() const { return world.Now(); }

	Rng RuleContext::Random(uint64_t salt) const
	{
		return Rng(HashCombine(world.Seed(), HashCombine(event.id, rule)), salt);
	}

	void RuleContext::Push(CommandEntry::Kind kind, TypeId type, const void* payload, size_t size, Tick at)
	{
		if (type == InvalidType)
			return;
		CommandEntry entry;
		entry.kind = kind;
		entry.type = type;
		entry.rule = rule;
		entry.cause = event.id;
		entry.at = at;
		std::memcpy(entry.payload.data(), payload, size);
		commands.push_back(entry);
	}

	Tick ApplyContext::Now() const { return world.now; }

	bool ApplyContext::Cancel(TimerHandle& handle) { return world.scheduler.Cancel(handle); }

	bool ApplyContext::IsPending(const TimerHandle& handle) const { return world.scheduler.IsPending(handle); }

	Rng& ApplyContext::Stream(RandomStreamId stream) { return world.streams[stream]; }

	Entity ApplyContext::Create() { return world.registry.create(); }

	void ApplyContext::Destroy(Entity entity)
	{
		if (world.IsAlive(entity))
			world.registry.destroy(entity);
	}

	void ApplyContext::Reject(const char* reason)
	{
		++world.stats.commandsRejected;
		Logger::Warning("sim: %s rejected (rule %s, cause #%llu): %s", what, world.types->RuleName(rule).c_str(), static_cast<unsigned long long>(cause), reason);
		CommandRejected rejected;
		rejected.command = command;
		rejected.reason = reason;
		world.Emit(world.types->EventId<CommandRejected>(), &rejected, sizeof(rejected), cause, rule);
	}

	// ---- World -----------------------------------------------------------------------------------------------------

	World::World(std::shared_ptr<const TypeRegistry> types, uint64_t seed) : types(std::move(types)), seed(seed)
	{
		for (const RandomStreamInfo& info : this->types->RandomStreams())
			streams.emplace_back(HashCombine(seed, HashString(info.name)), HashString(info.name));
		for (const TunableInfo& info : this->types->Tunables())
			tunables.push_back(info.defaultValue);
	}

	void World::SetTunable(TunableId id, float value)
	{
		const TunableInfo& info = types->Tunables()[id];
		tunables[id] = std::clamp(value, info.min, info.max);
	}

	void World::ReportUnregistered(const char* kind, const char* typeName) const
	{
		Logger::Error("sim: %s %s is not registered by any module", kind, typeName);
	}

	EventId World::Emit(TypeId type, const void* payload, size_t size, EventId cause, RuleId rule)
	{
		EventRecord record;
		record.type = type;
		record.cause = cause;
		record.rule = rule;
		std::memcpy(record.payload.data(), payload, size);
		return EmitRecord(record);
	}

	EventId World::EmitRecord(EventRecord record)
	{
		record.id = ++lastEventId;
		record.tick = now;
		record.wave = currentWave;
		log.Append(record);
		queue.push_back(record);

		const uint32_t flags = types->Event(record.type).flags;
		if (flags & EventOutput)
			outputs.push_back(record);
		if (flags & EventWake)
			wakeRequested = true;
		return record.id;
	}

	void World::Step(Tick ticks, const std::vector<InputEvent>& inputs)
	{
		if (ticks < 1)
			ticks = 1;
		invariantFailures.clear();
		const Tick target = now + ticks;
		++now;

		// 1. Cascade overflow keeps its ids and causes; then inputs.
		if (!deferred.empty())
		{
			queue.insert(queue.begin(), deferred.begin(), deferred.end());
			deferred.clear();
		}
		for (const InputEvent& input : inputs)
		{
			const EventTypeInfo& info = types->Event(input.type);
			if (!(info.flags & EventInput))
			{
				Logger::Error("sim: event %s is not an input and was dropped", info.name.c_str());
				continue;
			}
			Emit(input.type, input.payload.data(), info.ops.size, NoEvent, InputRule);
		}
		ProcessWaves();

		// 2. Scheduled events, all events of one tick together.
		ScheduledEvent entry;
		while (scheduler.PopDue(target, entry))
		{
			if (entry.at > now)
				now = entry.at;
			do
			{
				EventRecord record;
				record.type = entry.type;
				record.cause = entry.cause;
				record.rule = entry.rule;
				record.payload = entry.payload;
				EmitRecord(record);
			} while (scheduler.PopDue(now, entry));
			ProcessWaves();
		}

		// 3. Systems.
		now = target;
		for (const SystemInfo& system : types->Systems())
		{
			ApplyContext context(*this, NoEvent, SystemRule, system.name.c_str());
			system.run(context, ticks);
		}
		ProcessWaves();

		// 4. Invariants.
		if (checkInvariants)
			RunInvariants();
	}

	void World::ProcessWaves()
	{
		uint16_t wave = 0;
		std::vector<EventRecord> current;
		while (!queue.empty())
		{
			if (wave >= cascadeDepthCap)
			{
				++stats.cascadeOverflows;
				Logger::Warning("sim: cascade deeper than %d waves at tick %lld, %zu events deferred to the next step", cascadeDepthCap, static_cast<long long>(now), queue.size());
				deferred.insert(deferred.end(), queue.begin(), queue.end());
				queue.clear();
				break;
			}

			current.clear();
			current.swap(queue);
			commands.clear();
			for (const EventRecord& record : current)
			{
				for (const RuleId ruleId : types->Event(record.type).rules)
				{
					RuleContext context(*this, record, ruleId, commands);
					types->Rule(ruleId).run(context, record.payload.data());
					++stats.rulesRun;
				}
			}

			currentWave = static_cast<uint16_t>(wave + 1);
			for (const CommandEntry& command : commands)
				Apply(command);
			currentWave = 0;
			++wave;
		}
		commands.clear();
	}

	void World::Apply(const CommandEntry& command)
	{
		++stats.commandsApplied;
		switch (command.kind)
		{
		case CommandEntry::Kind::Emit:
			Emit(command.type, command.payload.data(), types->Event(command.type).ops.size, command.cause, command.rule);
			break;
		case CommandEntry::Kind::Schedule:
		{
			const size_t size = types->Event(command.type).ops.size;
			scheduler.Schedule(command.at < now ? now : command.at, command.type, command.payload.data(), size, command.cause, command.rule, false);
			break;
		}
		case CommandEntry::Kind::Custom:
		{
			const CommandTypeInfo& info = types->Command(command.type);
			ApplyContext context(*this, command.cause, command.rule, info.name.c_str(), command.type);
			info.apply(context, command.payload.data());
			break;
		}
		}
	}

	void World::RunInvariants()
	{
		for (const InvariantInfo& invariant : types->Invariants())
		{
			std::string error;
			if (invariant.check(*this, error))
				continue;
			++stats.invariantFailures;
			std::string message = "t=" + std::to_string(now) + " " + invariant.name + ": " + error;
			Logger::Error("sim: invariant failed %s", message.c_str());
			invariantFailures.push_back(std::move(message));
		}
	}

	std::vector<EventRecord> World::TakeOutputs()
	{
		std::vector<EventRecord> result;
		result.swap(outputs);
		return result;
	}

	bool World::TakeWake()
	{
		const bool wake = wakeRequested;
		wakeRequested = false;
		return wake;
	}

	// ---- Snapshots -------------------------------------------------------------------------------------------------

	void World::WriteRecord(BinaryWriter& ar, const EventRecord& record) const
	{
		EventId id = record.id, cause = record.cause;
		Tick tick = record.tick;
		TypeId type = record.type;
		RuleId rule = record.rule;
		uint16_t wave = record.wave;
		ar.Primitive(nullptr, id);
		ar.Primitive(nullptr, tick);
		ar.Primitive(nullptr, cause);
		ar.Primitive(nullptr, type);
		ar.Primitive(nullptr, rule);
		ar.Primitive(nullptr, wave);
		WritePayloadBlob(ar, types->Event(record.type).ops, record.payload.data());
	}

	void World::WriteScheduled(BinaryWriter& ar, const ScheduledEvent& entry) const
	{
		ScheduledEvent copy = entry;
		ar.Primitive(nullptr, copy.at);
		ar.Primitive(nullptr, copy.sequence);
		ar.Primitive(nullptr, copy.slot);
		ar.Primitive(nullptr, copy.version);
		ar.Primitive(nullptr, copy.cause);
		ar.Primitive(nullptr, copy.rule);
		ar.Primitive(nullptr, copy.type);
		WritePayloadBlob(ar, types->Event(entry.type).ops, entry.payload.data());
	}

	std::vector<uint8_t> World::Save(bool includeLog) const
	{
		BinaryWriter ar;
		uint32_t magic = SnapshotMagic, version = SnapshotVersion;
		uint64_t savedSeed = seed;
		Tick savedNow = now;
		EventId savedLast = lastEventId;
		ar.Primitive(nullptr, magic);
		ar.Primitive(nullptr, version);
		ar.Primitive(nullptr, savedSeed);
		ar.Primitive(nullptr, savedNow);
		ar.Primitive(nullptr, savedLast);

		// Name tables: ids in the rest of the snapshot are indices into these.
		std::vector<std::string> eventNames, ruleNames;
		for (const EventTypeInfo& info : types->Events())
			eventNames.push_back(info.name);
		for (const RuleInfo& info : types->Rules())
			ruleNames.push_back(info.name);
		ar.Write(eventNames);
		ar.Write(ruleNames);

		// Scheduler.
		uint64_t sequence = scheduler.NextSequence();
		ar.Primitive(nullptr, sequence);
		ar.Write(scheduler.SlotVersions());
		ar.Write(scheduler.FreeSlots());
		const std::vector<ScheduledEvent> live = scheduler.SortedLiveEntries();
		uint32_t liveCount = static_cast<uint32_t>(live.size());
		ar.Primitive(nullptr, liveCount);
		for (const ScheduledEvent& entry : live)
			WriteScheduled(ar, entry);

		// Random streams and tunables, by name.
		uint32_t streamCount = static_cast<uint32_t>(streams.size());
		ar.Primitive(nullptr, streamCount);
		for (size_t i = 0; i < streams.size(); ++i)
		{
			std::string name = types->RandomStreams()[i].name;
			ar.String(nullptr, name);
			ar.Write(streams[i]);
		}
		uint32_t tunableCount = static_cast<uint32_t>(tunables.size());
		ar.Primitive(nullptr, tunableCount);
		for (size_t i = 0; i < tunables.size(); ++i)
		{
			std::string name = types->Tunables()[i].name;
			float value = tunables[i];
			ar.String(nullptr, name);
			ar.Primitive(nullptr, value);
		}

		// Cascade overflow waiting for the next step.
		uint32_t deferredCount = static_cast<uint32_t>(deferred.size());
		ar.Primitive(nullptr, deferredCount);
		for (const EventRecord& record : deferred)
			WriteRecord(ar, record);

		// Entity storage in packed order, including released ids, so handles and future ids match exactly.
		const auto* entities = registry.storage<Entity>();
		uint32_t entityCount = static_cast<uint32_t>(entities->size());
		uint32_t freeList = static_cast<uint32_t>(entities->free_list());
		ar.Primitive(nullptr, entityCount);
		ar.Primitive(nullptr, freeList);
		for (uint32_t i = 0; i < entityCount; ++i)
		{
			Entity entity = entities->data()[i];
			ar.EntityRef(nullptr, entity);
		}

		// Components, each as a length-prefixed chunk so unknown ones can be skipped.
		uint32_t componentCount = static_cast<uint32_t>(types->Components().size());
		ar.Primitive(nullptr, componentCount);
		for (const ComponentTypeInfo& info : types->Components())
		{
			std::string name = info.name;
			uint32_t componentVersion = info.version;
			ar.String(nullptr, name);
			ar.Primitive(nullptr, componentVersion);
			BinaryWriter chunk;
			info.save(registry, chunk);
			uint32_t size = static_cast<uint32_t>(chunk.bytes.size());
			ar.Primitive(nullptr, size);
			ar.Raw(chunk.bytes.data(), chunk.bytes.size());
		}

		uint8_t hasLog = includeLog ? 1 : 0;
		ar.Primitive(nullptr, hasLog);
		if (includeLog)
		{
			uint32_t logCount = static_cast<uint32_t>(log.Size());
			ar.Primitive(nullptr, logCount);
			for (size_t i = 0; i < log.Size(); ++i)
				WriteRecord(ar, log.At(i));
		}
		return std::move(ar.bytes);
	}

	uint64_t World::StateHash() const
	{
		const std::vector<uint8_t> bytes = Save(false);
		return HashBytes(bytes.data(), bytes.size());
	}

	std::unique_ptr<World> World::Load(std::shared_ptr<const TypeRegistry> types, const uint8_t* data, size_t size, std::string& error)
	{
		BinaryReader ar(data, size);
		uint32_t magic = 0, version = 0;
		uint64_t seed = 0;
		ar.Primitive(nullptr, magic);
		ar.Primitive(nullptr, version);
		ar.Primitive(nullptr, seed);
		if (magic != SnapshotMagic)
		{
			error = "not a sim snapshot";
			return nullptr;
		}
		if (version != SnapshotVersion)
		{
			error = "unsupported snapshot version " + std::to_string(version);
			return nullptr;
		}

		auto world = std::make_unique<World>(types, seed);
		ar.Primitive(nullptr, world->now);
		ar.Primitive(nullptr, world->lastEventId);

		std::vector<std::string> eventNames, ruleNames;
		ar.Read(eventNames);
		ar.Read(ruleNames);
		std::vector<TypeId> eventMap;
		for (const std::string& name : eventNames)
		{
			eventMap.push_back(types->FindEvent(name));
			if (eventMap.back() == InvalidType)
				Logger::Warning("sim load: event type %s no longer exists", name.c_str());
		}
		std::vector<RuleId> ruleMap;
		for (const std::string& name : ruleNames)
		{
			RuleId found = NoRule;
			for (size_t i = 0; i < types->Rules().size(); ++i)
				if (types->Rules()[i].name == name)
					found = static_cast<RuleId>(i);
			ruleMap.push_back(found);
		}
		auto mapRule = [&ruleMap](RuleId saved) -> RuleId
		{
			if (saved >= SetupRule)
				return saved; // pseudo rules and NoRule are stored as themselves
			return saved < ruleMap.size() ? ruleMap[saved] : NoRule;
		};
		auto mapEvent = [&eventMap](TypeId saved) -> TypeId { return saved < eventMap.size() ? eventMap[saved] : InvalidType; };
		auto readRecord = [&](EventRecord& record) -> bool
		{
			TypeId savedType = InvalidType;
			RuleId savedRule = NoRule;
			ar.Primitive(nullptr, record.id);
			ar.Primitive(nullptr, record.tick);
			ar.Primitive(nullptr, record.cause);
			ar.Primitive(nullptr, savedType);
			ar.Primitive(nullptr, savedRule);
			ar.Primitive(nullptr, record.wave);
			record.type = mapEvent(savedType);
			record.rule = mapRule(savedRule);
			const PayloadOps* ops = record.type == InvalidType ? nullptr : &types->Event(record.type).ops;
			return ReadPayloadBlob(ar, ops, record.payload);
		};

		// Scheduler.
		uint64_t sequence = 0;
		std::vector<uint32_t> slotVersions, freeSlots;
		ar.Primitive(nullptr, sequence);
		ar.Read(slotVersions);
		ar.Read(freeSlots);
		uint32_t liveCount = 0;
		ar.Primitive(nullptr, liveCount);
		std::vector<ScheduledEvent> live;
		for (uint32_t i = 0; i < liveCount && !ar.Failed(); ++i)
		{
			ScheduledEvent entry;
			TypeId savedType = InvalidType;
			RuleId savedRule = NoRule;
			ar.Primitive(nullptr, entry.at);
			ar.Primitive(nullptr, entry.sequence);
			ar.Primitive(nullptr, entry.slot);
			ar.Primitive(nullptr, entry.version);
			ar.Primitive(nullptr, entry.cause);
			ar.Primitive(nullptr, savedRule);
			ar.Primitive(nullptr, savedType);
			entry.type = mapEvent(savedType);
			entry.rule = mapRule(savedRule);
			const PayloadOps* ops = entry.type == InvalidType ? nullptr : &types->Event(entry.type).ops;
			if (!ReadPayloadBlob(ar, ops, entry.payload))
			{
				Logger::Warning("sim load: dropped a scheduled event of a removed or changed type (saved type %u)", savedType);
				continue;
			}
			if (entry.slot != TimerHandle::NoSlot && entry.slot >= slotVersions.size())
			{
				error = "scheduler entry refers to a missing timer slot";
				return nullptr;
			}
			live.push_back(entry);
		}
		world->scheduler.Restore(std::move(live), std::move(slotVersions), std::move(freeSlots), sequence);

		// Random streams and tunables.
		uint32_t streamCount = 0;
		ar.Primitive(nullptr, streamCount);
		for (uint32_t i = 0; i < streamCount && !ar.Failed(); ++i)
		{
			std::string name;
			Rng rng;
			ar.String(nullptr, name);
			ar.Read(rng);
			const RandomStreamId id = types->FindRandomStream(name);
			if (id != static_cast<RandomStreamId>(-1))
				world->streams[id] = rng;
		}
		uint32_t tunableCount = 0;
		ar.Primitive(nullptr, tunableCount);
		for (uint32_t i = 0; i < tunableCount && !ar.Failed(); ++i)
		{
			std::string name;
			float value = 0.0f;
			ar.String(nullptr, name);
			ar.Primitive(nullptr, value);
			const TunableId id = types->FindTunable(name);
			if (id != static_cast<TunableId>(-1))
				world->SetTunable(id, value);
		}

		uint32_t deferredCount = 0;
		ar.Primitive(nullptr, deferredCount);
		for (uint32_t i = 0; i < deferredCount && !ar.Failed(); ++i)
		{
			EventRecord record;
			if (readRecord(record))
				world->deferred.push_back(record);
		}

		// Entities.
		uint32_t entityCount = 0, freeList = 0;
		ar.Primitive(nullptr, entityCount);
		ar.Primitive(nullptr, freeList);
		if (freeList > entityCount || entityCount > ar.Remaining() / sizeof(uint32_t))
		{
			error = "corrupt entity table";
			return nullptr;
		}
		auto& entities = world->registry.storage<Entity>();
		uint32_t nextIndex = 0;
		for (uint32_t i = 0; i < entityCount; ++i)
		{
			Entity entity = NullEntity;
			ar.EntityRef(nullptr, entity);
			if (entities.generate(entity) != entity)
			{
				error = "entity table has duplicates";
				return nullptr;
			}
			nextIndex = std::max(nextIndex, EntityIndex(entity) + 1);
		}
		if (entityCount > 0)
			entities.start_from(static_cast<Entity>(nextIndex));
		entities.free_list(freeList);

		// Components.
		uint32_t componentCount = 0;
		ar.Primitive(nullptr, componentCount);
		for (uint32_t i = 0; i < componentCount && !ar.Failed(); ++i)
		{
			std::string name;
			uint32_t savedVersion = 0, chunkSize = 0;
			ar.String(nullptr, name);
			ar.Primitive(nullptr, savedVersion);
			ar.Primitive(nullptr, chunkSize);
			if (ar.Failed() || chunkSize > ar.Remaining())
			{
				error = "snapshot is truncated in component " + name;
				return nullptr;
			}
			const ComponentTypeInfo* info = types->FindComponent(name);
			if (!info)
			{
				Logger::Warning("sim load: component %s no longer exists, %u bytes skipped", name.c_str(), chunkSize);
				ar.Skip(chunkSize);
				continue;
			}
			std::vector<uint8_t> chunk(chunkSize);
			ar.Raw(chunk.data(), chunkSize);
			BinaryReader chunkReader(chunk.data(), chunk.size());
			if (!info->load(world->registry, chunkReader, savedVersion) || !chunkReader.AtEnd())
			{
				error = "component " + name + " data does not match its type (missing migration?)";
				return nullptr;
			}
		}

		uint8_t hasLog = 0;
		ar.Primitive(nullptr, hasLog);
		if (hasLog)
		{
			uint32_t logCount = 0;
			ar.Primitive(nullptr, logCount);
			for (uint32_t i = 0; i < logCount && !ar.Failed(); ++i)
			{
				EventRecord record;
				if (readRecord(record) && record.type != InvalidType)
					world->log.Append(record);
			}
		}

		if (ar.Failed() || !ar.AtEnd())
		{
			error = ar.Failed() ? "snapshot is truncated or corrupt" : "snapshot has trailing data";
			return nullptr;
		}
		return world;
	}

	// ---- Text tools ------------------------------------------------------------------------------------------------

	std::string World::FormatTick(Tick tick)
	{
		const Tick totalSeconds = tick / TicksPerSecond;
		char buffer[64];
		std::snprintf(buffer, sizeof(buffer), "t%lld(%lldh%02lldm%02lld.%llds)", static_cast<long long>(tick), static_cast<long long>(totalSeconds / 3600),
			static_cast<long long>(totalSeconds / 60 % 60), static_cast<long long>(totalSeconds % 60), static_cast<long long>(tick % TicksPerSecond));
		return buffer;
	}

	std::string World::DescribeEntity(Entity entity) const
	{
		std::string name = TextArchive::DefaultEntityName(entity);
		if (entity == NullEntity)
			return name;
		if (!IsAlive(entity))
			return name + "(gone)";
		if (const Label* label = registry.try_get<Label>(entity); label && !label->text.empty())
			return label->text + "(" + name + ")";
		return name;
	}

	std::string World::DumpEntity(Entity entity) const
	{
		std::string out = DescribeEntity(entity) + "\n";
		if (!IsAlive(entity))
			return out;
		for (const ComponentTypeInfo& info : types->Components())
		{
			TextArchive ar;
			ar.namer = &NameEntity;
			ar.namerContext = this;
			if (info.dump(registry, entity, ar))
				out += "  " + ar.text + "\n";
		}
		return out;
	}

	std::string World::DescribePayload(TypeId type, const void* payload) const
	{
		if (type == InvalidType)
			return "<unknown event>";
		const EventTypeInfo& info = types->Event(type);
		TextArchive ar;
		ar.namer = &NameEntity;
		ar.namerContext = this;
		info.ops.text(ar, payload);
		return info.name + "{" + ar.text + "}";
	}

	std::string World::DescribeEvent(const EventRecord& record) const
	{
		char header[96];
		std::snprintf(header, sizeof(header), "#%llu %s ", static_cast<unsigned long long>(record.id), FormatTick(record.tick).c_str());
		std::string out = header + DescribePayload(record.type, record.payload.data()) + " by " + types->RuleName(record.rule);
		if (record.wave)
			out += " wave " + std::to_string(record.wave);
		return out;
	}

	std::string World::WhyTrace(EventId id, int maxDepth) const
	{
		std::string out = "why #" + std::to_string(id) + ":\n";
		EventId current = id;
		for (int depth = 0; depth < maxDepth; ++depth)
		{
			if (current == NoEvent)
			{
				out += "  (root)\n";
				return out;
			}
			const EventRecord* record = log.Find(current);
			if (!record)
			{
				out += "  #" + std::to_string(current) + " (no longer in the log)\n";
				return out;
			}
			out += (depth == 0 ? "  " : "  <- ") + DescribeEvent(*record) + "\n";
			current = record->cause;
		}
		out += "  ... (depth limit)\n";
		return out;
	}

	std::string World::Timeline(Entity entity, size_t maxEvents) const
	{
		std::vector<const EventRecord*> hits;
		for (size_t i = 0; i < log.Size(); ++i)
		{
			const EventRecord& record = log.At(i);
			EntityCollector collector;
			types->Event(record.type).ops.collect(collector, record.payload.data());
			if (std::find(collector.entities.begin(), collector.entities.end(), entity) != collector.entities.end())
				hits.push_back(&record);
		}
		std::string out = "timeline " + DescribeEntity(entity) + ": " + std::to_string(hits.size()) + " events in the log\n";
		const size_t first = hits.size() > maxEvents ? hits.size() - maxEvents : 0;
		for (size_t i = first; i < hits.size(); ++i)
			out += "  " + DescribeEvent(*hits[i]) + "  <- #" + std::to_string(hits[i]->cause) + "\n";
		return out;
	}

	std::string World::DumpScheduler(size_t maxEntries) const
	{
		const std::vector<ScheduledEvent> live = scheduler.SortedLiveEntries();
		std::string out = "scheduler: " + std::to_string(live.size()) + " pending (" + std::to_string(scheduler.EntryCount()) + " heap entries)\n";
		for (size_t i = 0; i < live.size() && i < maxEntries; ++i)
			out += "  " + FormatTick(live[i].at) + " " + DescribePayload(live[i].type, live[i].payload.data()) + " <- #" + std::to_string(live[i].cause) +
				(live[i].slot == TimerHandle::NoSlot ? "" : " (cancellable)") + "\n";
		return out;
	}

	std::string World::DumpState() const
	{
		const auto* entities = registry.storage<Entity>();
		char header[256];
		std::snprintf(header, sizeof(header), "world %s seed %llu, %zu alive entities, last event #%llu, log %zu/%zu\n", FormatTick(now).c_str(),
			static_cast<unsigned long long>(seed), static_cast<size_t>(entities->free_list()), static_cast<unsigned long long>(lastEventId), log.Size(), log.Capacity());
		std::string out = header;
		std::snprintf(header, sizeof(header), "stats: rules %llu, commands %llu, rejected %llu, cascade overflows %llu, invariant failures %llu\n",
			static_cast<unsigned long long>(stats.rulesRun), static_cast<unsigned long long>(stats.commandsApplied), static_cast<unsigned long long>(stats.commandsRejected),
			static_cast<unsigned long long>(stats.cascadeOverflows), static_cast<unsigned long long>(stats.invariantFailures));
		out += header;
		out += "components:";
		for (const ComponentTypeInfo& info : types->Components())
			out += " " + info.name + "=" + std::to_string(info.count(registry));
		out += "\n" + DumpScheduler(20);
		for (const auto [entity] : entities->each())
			out += DumpEntity(entity);
		return out;
	}

	Entity World::FindByIndex(uint32_t index) const
	{
		for (const auto [entity] : registry.storage<Entity>()->each())
			if (EntityIndex(entity) == index)
				return entity;
		return NullEntity;
	}
}
