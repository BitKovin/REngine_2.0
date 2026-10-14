#include "MoveBehavior.h"

#include <cassert>

std::vector<MoveBehaviorType>& MoveBehaviorRegistry::Types()
{
	static std::vector<MoveBehaviorType> types;
	return types;
}

int MoveBehaviorRegistry::Register(MoveBehaviorType type)
{
	auto& types = Types();
	for (size_t i = 0; i < types.size(); ++i)
	{
		if (types[i].Name == type.Name)
		{
			assert(false && "Move behaviour registered twice");
			return static_cast<int>(i);
		}
	}

	types.push_back(std::move(type));
	return static_cast<int>(types.size()) - 1;
}

int MoveBehaviorRegistry::Find(const std::string& name)
{
	const auto& types = Types();
	for (size_t i = 0; i < types.size(); ++i)
		if (types[i].Name == name)
			return static_cast<int>(i);
	return -1;
}

const MoveBehaviorType& MoveBehaviorRegistry::Get(int index)
{
	return Types()[static_cast<size_t>(index)];
}

int MoveBehaviorRegistry::Count()
{
	return static_cast<int>(Types().size());
}

std::string MoveBehaviorRegistry::KnownNames()
{
	std::string result;
	for (const MoveBehaviorType& type : Types())
	{
		if (!result.empty()) result += ", ";
		result += type.Name;
	}
	return result;
}
