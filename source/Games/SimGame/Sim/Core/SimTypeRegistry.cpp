#include "SimTypeRegistry.h"

#include <cstdio>

namespace sim
{
	void RegisterKernelTypes(Registration& registration); // SimWorld.cpp

	namespace
	{
		const char* FlagNames(uint32_t flags, std::string& out)
		{
			out.clear();
			if (flags & EventInput) out += " input";
			if (flags & EventOutput) out += " output";
			if (flags & EventWake) out += " wake";
			if (flags & EventSignificant) out += " significant";
			return out.c_str();
		}
	}

	TypeId TypeRegistry::FindEvent(const std::string& name) const
	{
		for (const EventTypeInfo& info : events)
			if (info.name == name)
				return info.id;
		return InvalidType;
	}

	TunableId TypeRegistry::FindTunable(const std::string& name) const
	{
		for (size_t i = 0; i < tunables.size(); ++i)
			if (tunables[i].name == name)
				return static_cast<TunableId>(i);
		return static_cast<TunableId>(-1);
	}

	RandomStreamId TypeRegistry::FindRandomStream(const std::string& name) const
	{
		for (size_t i = 0; i < randomStreams.size(); ++i)
			if (randomStreams[i].name == name)
				return static_cast<RandomStreamId>(i);
		return static_cast<RandomStreamId>(-1);
	}

	const ComponentTypeInfo* TypeRegistry::FindComponent(const std::string& name) const
	{
		for (const ComponentTypeInfo& info : components)
			if (info.name == name)
				return &info;
		return nullptr;
	}

	std::string TypeRegistry::RuleName(RuleId id) const
	{
		switch (id)
		{
		case NoRule: return "none";
		case InputRule: return "input";
		case SystemRule: return "system";
		case SetupRule: return "setup";
		default: return id < rules.size() ? rules[id].name : "rule?" + std::to_string(id);
		}
	}

	std::string TypeRegistry::Describe() const
	{
		std::string out;
		std::string flags;
		char line[256];

		out += "modules (in order):";
		for (const std::string& module : modules)
			out += " " + module;
		out += "\n\ncomponents:\n";
		for (const ComponentTypeInfo& info : components)
		{
			std::snprintf(line, sizeof(line), "  %-28s v%u  [%s]\n", info.name.c_str(), info.version, info.module.c_str());
			out += line;
		}

		out += "\nevents (rules in execution order):\n";
		for (const EventTypeInfo& info : events)
		{
			std::snprintf(line, sizeof(line), "  %-28s %2zu bytes [%s]%s\n", info.name.c_str(), info.ops.size, info.module.c_str(), FlagNames(info.flags, flags));
			out += line;
			for (RuleId rule : info.rules)
				out += "      -> " + rules[rule].name + "\n";
		}

		out += "\ncommands:\n";
		for (const CommandTypeInfo& info : commands)
		{
			std::snprintf(line, sizeof(line), "  %-28s %2zu bytes [%s]\n", info.name.c_str(), info.ops.size, info.module.c_str());
			out += line;
		}

		out += "\nsystems (in execution order):\n";
		for (const SystemInfo& info : systems)
			out += "  " + info.name + "\n";

		out += "\ninvariants:\n";
		for (const InvariantInfo& info : invariants)
			out += "  " + info.name + "\n";

		out += "\ntunables:\n";
		for (const TunableInfo& info : tunables)
		{
			std::snprintf(line, sizeof(line), "  %-36s default %g range [%g, %g]  %s\n", info.name.c_str(), info.defaultValue, info.min, info.max, info.description.c_str());
			out += line;
		}

		out += "\nrandom streams:\n";
		for (const RandomStreamInfo& info : randomStreams)
			out += "  " + info.name + "\n";
		return out;
	}

	RegistryBuilder::RegistryBuilder() : registry(std::make_shared<TypeRegistry>())
	{
		registry->modules.push_back("kernel");
		Registration registration(*registry, "kernel", errors);
		RegisterKernelTypes(registration);
	}

	void RegistryBuilder::Add(Module& module)
	{
		const std::string name = module.Name();
		for (const std::string& existing : registry->modules)
			if (existing == name)
				errors.push_back("module " + name + " added twice");
		registry->modules.push_back(name);
		Registration registration(*registry, name, errors);
		module.Register(registration);
	}

	std::shared_ptr<const TypeRegistry> RegistryBuilder::Build(std::vector<std::string>& outErrors)
	{
		if (registry->rules.size() >= SetupRule)
			errors.push_back("too many rules");
		for (const TunableInfo& info : registry->tunables)
			if (info.defaultValue < info.min || info.defaultValue > info.max)
				errors.push_back("tunable " + info.name + " default is outside its range");

		outErrors = errors;
		if (!errors.empty())
			return nullptr;
		std::shared_ptr<const TypeRegistry> result = registry;
		registry.reset();
		return result;
	}
}
