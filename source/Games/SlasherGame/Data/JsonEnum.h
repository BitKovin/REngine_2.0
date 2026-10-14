#pragma once

#include <json.hpp>

#include <cstddef>
#include <utility>

// String <-> enum serialization with the same table syntax as NLOHMANN_JSON_SERIALIZE_ENUM, implemented as an
// explicit nlohmann::adl_serializer specialization (the pattern engine Helpers/JsonHelper.hpp uses for glm).
// NLOHMANN_JSON_SERIALIZE_ENUM defines namespace-scope to_json/from_json, which become ambiguous with
// nlohmann's own customization objects because EObject.hpp has a global `using namespace nlohmann`.
//
// The first table entry is the fallback for unknown strings (use { Enum::Invalid, nullptr } so typos are
// detected by validation). Must be used at global namespace scope.
#define SLASHER_JSON_ENUM(ENUM_TYPE, ...)                                                          \
	namespace nlohmann {                                                                           \
	template<> struct adl_serializer<ENUM_TYPE>                                                    \
	{                                                                                              \
		static const std::pair<ENUM_TYPE, const char*>* Table(std::size_t& count)                  \
		{                                                                                          \
			static const std::pair<ENUM_TYPE, const char*> table[] = __VA_ARGS__;                  \
			count = sizeof(table) / sizeof(table[0]);                                              \
			return table;                                                                          \
		}                                                                                          \
		static void to_json(json& j, const ENUM_TYPE& value)                                       \
		{                                                                                          \
			std::size_t count = 0;                                                                 \
			const auto* table = Table(count);                                                      \
			for (std::size_t i = 0; i < count; ++i)                                                \
			{                                                                                      \
				if (table[i].first == value)                                                       \
				{                                                                                  \
					if (table[i].second) j = table[i].second; else j = nullptr;                    \
					return;                                                                        \
				}                                                                                  \
			}                                                                                      \
			if (table[0].second) j = table[0].second; else j = nullptr;                            \
		}                                                                                          \
		static void from_json(const json& j, ENUM_TYPE& value)                                     \
		{                                                                                          \
			std::size_t count = 0;                                                                 \
			const auto* table = Table(count);                                                      \
			value = table[0].first;                                                                \
			if (!j.is_string()) return;                                                            \
			const auto& text = j.get_ref<const json::string_t&>();                                 \
			for (std::size_t i = 0; i < count; ++i)                                                \
			{                                                                                      \
				if (table[i].second && text == table[i].second)                                    \
				{                                                                                  \
					value = table[i].first;                                                        \
					return;                                                                        \
				}                                                                                  \
			}                                                                                      \
		}                                                                                          \
	};                                                                                             \
	}
