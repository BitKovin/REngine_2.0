#pragma once

// Everything a module declares, collected once at startup in an explicit order (design section 16.2), then frozen.
// A frozen TypeRegistry is immutable and may be read from any thread.
//
// Record types name themselves with `static constexpr const char* TypeName = "...";` and declare their fields with a
// Visit member (SimArchive.h). Events and commands must also be trivially copyable and at most MaxPayloadSize bytes,
// because they are stored inline in the log, the scheduler and the command buffer. Components can hold anything
// Visit supports (strings, vectors).

#include "SimArchive.h"
#include "SimTypes.h"

#include <entt/container/dense_map.hpp>
#include <entt/core/type_info.hpp>
#include <entt/entity/registry.hpp>

#include <cstring>
#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

namespace sim
{
	class World;
	class RuleContext;
	class ApplyContext;

	enum EventFlags : uint32_t
	{
		EventNone = 0,
		EventInput = 1u << 0,       // may arrive from the client (Host::SendEvent); applied at the start of a tick
		EventOutput = 1u << 1,      // copied to the client after the tick that emitted it
		EventWake = 1u << 2,        // stops fast-forward (addendum section 6.4, open item O3)
		EventSignificant = 1u << 3, // kept by history compaction (design section 4.9)
	};

	using TunableId = uint16_t;
	using RandomStreamId = uint16_t;

	struct PayloadOps
	{
		size_t size = 0;
		void (*write)(BinaryWriter&, const void*) = nullptr;
		void (*read)(BinaryReader&, void*) = nullptr;
		void (*hash)(HashArchive&, const void*) = nullptr;
		void (*text)(TextArchive&, const void*) = nullptr;
		void (*collect)(EntityCollector&, const void*) = nullptr;

		template<class T>
		static PayloadOps Make()
		{
			static_assert(std::is_trivially_copyable_v<T>, "event and command payloads must be trivially copyable");
			static_assert(sizeof(T) <= MaxPayloadSize, "event and command payloads must fit MaxPayloadSize");
			static_assert(alignof(T) <= 8, "event and command payloads must not need more than 8-byte alignment");
			PayloadOps ops;
			ops.size = sizeof(T);
			ops.write = [](BinaryWriter& ar, const void* p) { ar.Write(*static_cast<const T*>(p)); };
			ops.read = [](BinaryReader& ar, void* p)
			{
				T value{};
				ar.Read(value);
				std::memcpy(p, &value, sizeof(T));
			};
			ops.hash = [](HashArchive& ar, const void* p) { ar.Add(*static_cast<const T*>(p)); };
			ops.text = [](TextArchive& ar, const void* p) { ar.Add(*static_cast<const T*>(p)); };
			ops.collect = [](EntityCollector& ar, const void* p) { ar.Add(*static_cast<const T*>(p)); };
			return ops;
		}
	};

	struct EventTypeInfo
	{
		std::string name;
		std::string module;
		TypeId id = InvalidType;
		uint32_t flags = EventNone;
		PayloadOps ops;
		std::vector<RuleId> rules; // in registration order
	};

	struct CommandTypeInfo
	{
		std::string name;
		std::string module;
		TypeId id = InvalidType;
		PayloadOps ops;
		std::function<void(ApplyContext&, const void*)> apply;
	};

	struct RuleInfo
	{
		std::string name; // "module.rule"
		std::string module;
		TypeId eventType = InvalidType;
		std::function<void(RuleContext&, const void*)> run;
	};

	struct ComponentTypeInfo
	{
		std::string name;
		std::string module;
		uint32_t version = 1;
		void (*save)(const entt::registry&, BinaryWriter&) = nullptr;
		// Called with the version the data was saved with. Returns false on corrupt data.
		bool (*load)(entt::registry&, BinaryReader&, uint32_t savedVersion) = nullptr;
		bool (*dump)(const entt::registry&, Entity, TextArchive&) = nullptr;
		size_t (*count)(const entt::registry&) = nullptr;
	};

	struct SystemInfo
	{
		std::string name;
		std::string module;
		std::function<void(ApplyContext&, Tick dt)> run;
	};

	struct InvariantInfo
	{
		std::string name;
		std::string module;
		// Returns false and fills `error` when violated.
		std::function<bool(const World&, std::string& error)> check;
	};

	struct TunableInfo
	{
		std::string name;
		std::string module;
		std::string description;
		float defaultValue = 0.0f;
		float min = 0.0f;
		float max = 0.0f;
	};

	struct RandomStreamInfo
	{
		std::string name;
		std::string module;
	};

	class TypeRegistry
	{
	public:
		const std::vector<EventTypeInfo>& Events() const { return events; }
		const std::vector<CommandTypeInfo>& Commands() const { return commands; }
		const std::vector<RuleInfo>& Rules() const { return rules; }
		const std::vector<ComponentTypeInfo>& Components() const { return components; }
		const std::vector<SystemInfo>& Systems() const { return systems; }
		const std::vector<InvariantInfo>& Invariants() const { return invariants; }
		const std::vector<TunableInfo>& Tunables() const { return tunables; }
		const std::vector<RandomStreamInfo>& RandomStreams() const { return randomStreams; }
		const std::vector<std::string>& Modules() const { return modules; }

		const EventTypeInfo& Event(TypeId id) const { return events[id]; }
		const CommandTypeInfo& Command(TypeId id) const { return commands[id]; }
		const RuleInfo& Rule(RuleId id) const { return rules[id]; }

		template<class T> TypeId EventId() const { return Find(eventIds, entt::type_hash<T>::value()); }
		template<class T> TypeId CommandId() const { return Find(commandIds, entt::type_hash<T>::value()); }

		TypeId FindEvent(const std::string& name) const;
		TunableId FindTunable(const std::string& name) const;
		RandomStreamId FindRandomStream(const std::string& name) const;
		const ComponentTypeInfo* FindComponent(const std::string& name) const;

		// Rule names include the pseudo rules (input, system, setup).
		std::string RuleName(RuleId id) const;

		// Startup dump (design section 16.2): every module, record type, event binding and execution order.
		std::string Describe() const;

	private:
		friend class Registration;
		friend class RegistryBuilder;

		std::vector<std::string> modules;
		std::vector<EventTypeInfo> events;
		std::vector<CommandTypeInfo> commands;
		std::vector<RuleInfo> rules;
		std::vector<ComponentTypeInfo> components;
		std::vector<SystemInfo> systems;
		std::vector<InvariantInfo> invariants;
		std::vector<TunableInfo> tunables;
		std::vector<RandomStreamInfo> randomStreams;

		// Keyed by entt's compile-time type hash. dense_map iterates in insertion order, so even iteration would be
		// deterministic, but these are lookup only.
		entt::dense_map<entt::id_type, TypeId> eventIds;
		entt::dense_map<entt::id_type, TypeId> commandIds;
		entt::dense_map<entt::id_type, size_t> componentIndices;

		static TypeId Find(const entt::dense_map<entt::id_type, TypeId>& ids, entt::id_type key)
		{
			const auto found = ids.find(key);
			return found == ids.end() ? InvalidType : found->second;
		}
	};

	// Handed to Module::Register. Every call records the declaring module.
	class Registration
	{
	public:
		Registration(TypeRegistry& registry, std::string module, std::vector<std::string>& errors)
			: registry(registry), module(std::move(module)), errors(errors) {}

		const std::string& ModuleName() const { return module; }

		// A persistent component. `version` is stored in saves and handed back on load; bump it when the fields change
		// and add the migration to LoadStorage's caller when the first one is needed.
		template<class T>
		void Component(uint32_t version = 1)
		{
			ComponentTypeInfo info;
			info.name = T::TypeName;
			info.module = module;
			info.version = version;
			info.save = &SaveStorage<T>;
			info.load = &LoadStorage<T>;
			info.dump = [](const entt::registry& reg, Entity entity, TextArchive& ar)
			{
				if constexpr (std::is_empty_v<T>)
				{
					// entt stores no instance for empty types (tags).
					if (!reg.all_of<T>(entity))
						return false;
					ar.text += T::TypeName;
				}
				else
				{
					const T* value = reg.try_get<T>(entity);
					if (!value)
						return false;
					ar.text += T::TypeName;
					ar.text += ": ";
					ar.Add(*value);
				}
				return true;
			};
			info.count = [](const entt::registry& reg)
			{
				const auto* storage = reg.storage<T>();
				return storage ? static_cast<size_t>(storage->size()) : size_t{ 0 };
			};
			const entt::id_type key = entt::type_hash<T>::value();
			if (registry.componentIndices.contains(key) || registry.FindComponent(info.name))
			{
				errors.push_back("duplicate component " + info.name + " (module " + module + ")");
				return;
			}
			registry.componentIndices[key] = registry.components.size();
			registry.components.push_back(std::move(info));
		}

		template<class T>
		TypeId Event(uint32_t flags = EventNone)
		{
			const entt::id_type key = entt::type_hash<T>::value();
			if (registry.eventIds.contains(key) || registry.FindEvent(T::TypeName) != InvalidType)
			{
				errors.push_back(std::string("duplicate event ") + T::TypeName + " (module " + module + ")");
				return InvalidType;
			}
			EventTypeInfo info;
			info.name = T::TypeName;
			info.module = module;
			info.id = static_cast<TypeId>(registry.events.size());
			info.flags = flags;
			info.ops = PayloadOps::Make<T>();
			registry.eventIds[key] = info.id;
			registry.events.push_back(std::move(info));
			return registry.events.back().id;
		}

		// A rule reacting to event E: `void(RuleContext&, const E&)`. Rules only read the world and return commands
		// (rule R7). Rules on one event run in registration order.
		template<class E, class Fn>
		RuleId Rule(const char* name, Fn fn)
		{
			const TypeId type = registry.EventId<E>();
			if (type == InvalidType)
			{
				errors.push_back(module + "." + name + ": rule on unregistered event " + E::TypeName);
				return NoRule;
			}
			RuleInfo info;
			info.name = module + "." + name;
			info.module = module;
			info.eventType = type;
			info.run = [fn](RuleContext& context, const void* payload) { fn(context, *static_cast<const E*>(payload)); };
			for (const RuleInfo& existing : registry.rules)
				if (existing.name == info.name)
					errors.push_back("duplicate rule " + info.name);
			const RuleId id = static_cast<RuleId>(registry.rules.size());
			registry.rules.push_back(std::move(info));
			registry.events[type].rules.push_back(id);
			return id;
		}

		// A command type and the function that applies it: `void(ApplyContext&, const C&)`.
		template<class C, class Fn>
		TypeId Command(Fn apply)
		{
			const entt::id_type key = entt::type_hash<C>::value();
			if (registry.commandIds.contains(key))
			{
				errors.push_back(std::string("duplicate command ") + C::TypeName);
				return InvalidType;
			}
			CommandTypeInfo info;
			info.name = C::TypeName;
			info.module = module;
			info.id = static_cast<TypeId>(registry.commands.size());
			info.ops = PayloadOps::Make<C>();
			info.apply = [apply](ApplyContext& context, const void* payload) { apply(context, *static_cast<const C*>(payload)); };
			registry.commandIds[key] = info.id;
			registry.commands.push_back(std::move(info));
			return registry.commands.back().id;
		}

		// Runs every step with mutable access, in registration order. Only for continuous processes (movement
		// integration, encounter detection). Discrete changes still go through events and commands.
		void System(const char* name, std::function<void(ApplyContext&, Tick dt)> run)
		{
			registry.systems.push_back({ module + "." + name, module, std::move(run) });
		}

		// Checked after every step when World::checkInvariants is on (tests, soaks, development).
		void Invariant(const char* name, std::function<bool(const World&, std::string& error)> check)
		{
			registry.invariants.push_back({ module + "." + name, module, std::move(check) });
		}

		TunableId Tunable(const char* name, float defaultValue, float min, float max, const char* description)
		{
			const std::string fullName = module + "." + name;
			if (registry.FindTunable(fullName) != static_cast<TunableId>(-1))
				errors.push_back("duplicate tunable " + fullName);
			registry.tunables.push_back({ fullName, module, description, defaultValue, min, max });
			return static_cast<TunableId>(registry.tunables.size() - 1);
		}

		// A named random stream for systems and command apply functions. Rules use RuleContext::Random instead.
		RandomStreamId RandomStream(const char* name)
		{
			const std::string fullName = module + "." + name;
			if (registry.FindRandomStream(fullName) != static_cast<RandomStreamId>(-1))
				errors.push_back("duplicate random stream " + fullName);
			registry.randomStreams.push_back({ fullName, module });
			return static_cast<RandomStreamId>(registry.randomStreams.size() - 1);
		}

	private:
		TypeRegistry& registry;
		std::string module;
		std::vector<std::string>& errors;

		// Components are saved in packed order and restored in the same order, so entt iteration order (and with it
		// every result that depends on it) is identical after a load.
		template<class T>
		static void SaveStorage(const entt::registry& reg, BinaryWriter& ar)
		{
			const auto* storage = reg.storage<T>();
			uint32_t count = storage ? static_cast<uint32_t>(storage->size()) : 0u;
			ar.BeginArray(nullptr, count);
			for (uint32_t i = 0; i < count; ++i)
			{
				Entity entity = storage->data()[i];
				ar.EntityRef(nullptr, entity);
				if constexpr (!std::is_empty_v<T>)
					ar.Write(storage->get(entity));
			}
		}

		template<class T>
		static bool LoadStorage(entt::registry& reg, BinaryReader& ar, uint32_t /*savedVersion*/)
		{
			uint32_t count = 0;
			ar.BeginArray(nullptr, count);
			auto& storage = reg.storage<T>();
			for (uint32_t i = 0; i < count && !ar.Failed(); ++i)
			{
				Entity entity = NullEntity;
				ar.EntityRef(nullptr, entity);
				if (!reg.valid(entity))
					return false;
				if constexpr (std::is_empty_v<T>)
					storage.emplace(entity);
				else
				{
					T value{};
					ar.Read(value);
					storage.emplace(entity, std::move(value));
				}
			}
			return !ar.Failed();
		}
	};

	class Module
	{
	public:
		virtual ~Module() = default;
		virtual const char* Name() const = 0;
		virtual void Register(Registration& registration) = 0;
	};

	// Builds a frozen TypeRegistry from modules in the given order. The kernel's own module is always first.
	class RegistryBuilder
	{
	public:
		RegistryBuilder();

		void Add(Module& module);

		// Returns nullptr and fills `errors` if any declaration was invalid.
		std::shared_ptr<const TypeRegistry> Build(std::vector<std::string>& errors);

	private:
		std::shared_ptr<TypeRegistry> registry;
		std::vector<std::string> errors;
	};
}
