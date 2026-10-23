#pragma once

// One field visitor drives saving, loading, state hashing, text dumps and entity references.
//
// A record (component, event, command, message) declares its fields once:
//
//     struct Health
//     {
//         float value = 100.0f;
//         Entity lastAttacker = NullEntity;
//         template<class Ar> void Visit(Ar& ar) { ar("value", value); ar("lastAttacker", lastAttacker); }
//     };
//
// and every archive below understands it. Supported field types: bool, integers, float, double, enums, Entity,
// std::string, std::vector<T>, std::array<T, N>, glm vectors, and any struct with a Visit member.

#include "SimTypes.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <array>
#include <bit>
#include <cstdio>
#include <cstring>
#include <string>
#include <type_traits>
#include <vector>

namespace sim
{
	// A short string that keeps a struct trivially copyable (event and command payloads). Truncates silently.
	template<size_t N>
	struct FixedString
	{
		std::array<char, N> chars{};

		FixedString() = default;
		FixedString(const char* text) { Assign(text); }
		FixedString(const std::string& text) { Assign(text.c_str()); }

		void Assign(const char* text)
		{
			chars.fill('\0');
			if (text)
				std::strncpy(chars.data(), text, N - 1);
		}
		const char* c_str() const { return chars.data(); }
		std::string str() const { return chars.data(); }
		bool operator==(const FixedString& other) const { return chars == other.chars; }
	};

	namespace detail
	{
		template<class T> struct IsFixedString : std::false_type {};
		template<size_t N> struct IsFixedString<FixedString<N>> : std::true_type {};

		template<class T> struct IsVector : std::false_type {};
		template<class T, class A> struct IsVector<std::vector<T, A>> : std::true_type {};

		template<class T> struct IsStdArray : std::false_type {};
		template<class T, size_t N> struct IsStdArray<std::array<T, N>> : std::true_type {};

		template<class T> struct IsGlmVec : std::false_type {};
		template<glm::length_t L, class U, glm::qualifier Q> struct IsGlmVec<glm::vec<L, U, Q>> : std::true_type {};
	}

	template<class Ar, class T>
	void Field(Ar& ar, const char* name, T& value)
	{
		if constexpr (std::is_same_v<T, Entity>)
			ar.EntityRef(name, value);
		else if constexpr (std::is_same_v<T, bool>)
		{
			uint8_t raw = value ? 1 : 0;
			ar.Primitive(name, raw);
			if constexpr (Ar::Reading)
				value = raw != 0;
		}
		else if constexpr (std::is_enum_v<T>)
		{
			auto raw = static_cast<std::underlying_type_t<T>>(value);
			ar.Primitive(name, raw);
			if constexpr (Ar::Reading)
				value = static_cast<T>(raw);
		}
		else if constexpr (std::is_arithmetic_v<T>)
			ar.Primitive(name, value);
		else if constexpr (std::is_same_v<T, std::string>)
			ar.String(name, value);
		else if constexpr (detail::IsFixedString<T>::value)
		{
			std::string text = value.str();
			ar.String(name, text);
			if constexpr (Ar::Reading)
				value.Assign(text.c_str());
		}
		else if constexpr (detail::IsVector<T>::value)
		{
			uint32_t count = static_cast<uint32_t>(value.size());
			ar.BeginArray(name, count);
			if constexpr (Ar::Reading)
				value.resize(count);
			for (auto& element : value)
				Field(ar, nullptr, element);
			ar.EndArray();
		}
		else if constexpr (detail::IsStdArray<T>::value)
		{
			uint32_t count = static_cast<uint32_t>(value.size());
			ar.BeginArray(name, count);
			for (auto& element : value)
				Field(ar, nullptr, element);
			ar.EndArray();
		}
		else if constexpr (detail::IsGlmVec<T>::value)
		{
			ar.BeginObject(name);
			static const char* axes[] = { "x", "y", "z", "w" };
			for (glm::length_t i = 0; i < T::length(); ++i)
				Field(ar, axes[i], value[i]);
			ar.EndObject();
		}
		else
		{
			ar.BeginObject(name);
			value.Visit(ar);
			ar.EndObject();
		}
	}

	// Visits a record that is only read by the archive (writers, hash, text). Visit is non-const so that the same
	// function serves loading; writers never modify the object.
	template<class Ar, class T>
	void VisitConst(Ar& ar, const T& value)
	{
		static_assert(!Ar::Reading, "VisitConst is for archives that do not write into the object");
		Field(ar, nullptr, const_cast<T&>(value));
	}

	template<class Derived>
	struct ArchiveBase
	{
		template<class T>
		void operator()(const char* name, T& value) { Field(static_cast<Derived&>(*this), name, value); }
	};

	// ------------------------------------------------------------------------------------------------------------
	// Binary

	class BinaryWriter : public ArchiveBase<BinaryWriter>
	{
	public:
		static constexpr bool Reading = false;

		std::vector<uint8_t> bytes;

		void Raw(const void* data, size_t size)
		{
			const size_t offset = bytes.size();
			bytes.resize(offset + size);
			if (size)
				std::memcpy(bytes.data() + offset, data, size);
		}

		template<class T> void Primitive(const char*, T& value) { Raw(&value, sizeof(T)); }
		void EntityRef(const char*, Entity& entity) { const uint32_t raw = entt::to_integral(entity); Raw(&raw, sizeof(raw)); }
		void String(const char*, std::string& text)
		{
			const uint32_t size = static_cast<uint32_t>(text.size());
			Raw(&size, sizeof(size));
			Raw(text.data(), text.size());
		}
		void BeginObject(const char*) {}
		void EndObject() {}
		void BeginArray(const char*, uint32_t& count) { Raw(&count, sizeof(count)); }
		void EndArray() {}

		template<class T> void Write(const T& value) { VisitConst(*this, value); }
	};

	class BinaryReader : public ArchiveBase<BinaryReader>
	{
	public:
		static constexpr bool Reading = true;

		BinaryReader(const uint8_t* data, size_t size) : data(data), size(size) {}

		bool Failed() const { return failed; }
		bool AtEnd() const { return offset == size; }
		size_t Offset() const { return offset; }
		size_t Remaining() const { return size - offset; }

		void Raw(void* out, size_t count)
		{
			if (failed || count > size - offset)
			{
				failed = true;
				std::memset(out, 0, count);
				return;
			}
			std::memcpy(out, data + offset, count);
			offset += count;
		}

		void Skip(size_t count)
		{
			if (failed || count > size - offset)
			{
				failed = true;
				return;
			}
			offset += count;
		}

		template<class T> void Primitive(const char*, T& value) { Raw(&value, sizeof(T)); }
		void EntityRef(const char*, Entity& entity)
		{
			uint32_t raw = 0;
			Raw(&raw, sizeof(raw));
			entity = static_cast<Entity>(raw);
		}
		void String(const char*, std::string& text)
		{
			uint32_t length = 0;
			Raw(&length, sizeof(length));
			if (failed || length > size - offset)
			{
				failed = true;
				text.clear();
				return;
			}
			text.assign(reinterpret_cast<const char*>(data + offset), length);
			offset += length;
		}
		void BeginObject(const char*) {}
		void EndObject() {}
		void BeginArray(const char*, uint32_t& count)
		{
			Raw(&count, sizeof(count));
			// Every element takes at least one byte, so a larger count is corrupt data, not a huge allocation.
			if (count > size - offset)
			{
				failed = true;
				count = 0;
			}
		}
		void EndArray() {}

		template<class T> void Read(T& value) { Field(*this, nullptr, value); }

	private:
		const uint8_t* data = nullptr;
		size_t size = 0;
		size_t offset = 0;
		bool failed = false;
	};

	// ------------------------------------------------------------------------------------------------------------
	// Hash: FNV-1a 64 over field values (names are ignored). Floats hash by bit pattern.

	class HashArchive : public ArchiveBase<HashArchive>
	{
	public:
		static constexpr bool Reading = false;

		uint64_t hash = 14695981039346656037ull;

		void Bytes(const void* data, size_t size)
		{
			const uint8_t* bytes = static_cast<const uint8_t*>(data);
			for (size_t i = 0; i < size; ++i)
			{
				hash ^= bytes[i];
				hash *= 1099511628211ull;
			}
		}

		template<class T> void Primitive(const char*, T& value) { Bytes(&value, sizeof(T)); }
		void EntityRef(const char*, Entity& entity) { const uint32_t raw = entt::to_integral(entity); Bytes(&raw, sizeof(raw)); }
		void String(const char*, std::string& text)
		{
			const uint32_t size = static_cast<uint32_t>(text.size());
			Bytes(&size, sizeof(size));
			Bytes(text.data(), text.size());
		}
		void BeginObject(const char*) {}
		void EndObject() {}
		void BeginArray(const char*, uint32_t& count) { Bytes(&count, sizeof(count)); }
		void EndArray() {}

		template<class T> void Add(const T& value) { VisitConst(*this, value); }
	};

	inline uint64_t HashBytes(const void* data, size_t size)
	{
		HashArchive archive;
		archive.Bytes(data, size);
		return archive.hash;
	}

	// ------------------------------------------------------------------------------------------------------------
	// Text: "name=value name2={x=1 y=2} list=[1 2 3]" on one line. Entities print through an optional namer.

	class TextArchive : public ArchiveBase<TextArchive>
	{
	public:
		static constexpr bool Reading = false;

		using EntityNamer = std::string (*)(const void* context, Entity entity);

		std::string text;
		EntityNamer namer = nullptr;
		const void* namerContext = nullptr;

		template<class T>
		void Primitive(const char* name, T& value)
		{
			Name(name);
			char buffer[64];
			if constexpr (std::is_floating_point_v<T>)
				std::snprintf(buffer, sizeof(buffer), "%.4g", static_cast<double>(value));
			else if constexpr (std::is_signed_v<T>)
				std::snprintf(buffer, sizeof(buffer), "%lld", static_cast<long long>(value));
			else
				std::snprintf(buffer, sizeof(buffer), "%llu", static_cast<unsigned long long>(value));
			text += buffer;
		}
		void EntityRef(const char* name, Entity& entity)
		{
			Name(name);
			text += namer ? namer(namerContext, entity) : DefaultEntityName(entity);
		}
		void String(const char* name, std::string& value)
		{
			Name(name);
			text += '"';
			text += value;
			text += '"';
		}
		void BeginObject(const char* name)
		{
			if (depth++ == 0 && name == nullptr)
			{
				topLevel = true;
				return;
			}
			Name(name);
			text += '{';
			first = true;
		}
		void EndObject()
		{
			if (--depth == 0 && topLevel)
			{
				topLevel = false;
				return;
			}
			text += '}';
			first = false;
		}
		void BeginArray(const char* name, uint32_t&)
		{
			Name(name);
			text += '[';
			first = true;
			++depth;
		}
		void EndArray()
		{
			--depth;
			text += ']';
			first = false;
		}

		template<class T> void Add(const T& value) { VisitConst(*this, value); }

		static std::string DefaultEntityName(Entity entity)
		{
			if (entity == NullEntity)
				return "null";
			char buffer[32];
			std::snprintf(buffer, sizeof(buffer), "#%u.%u", EntityIndex(entity), EntityVersion(entity));
			return buffer;
		}

	private:
		int depth = 0;
		bool first = true;
		bool topLevel = false;

		void Name(const char* name)
		{
			if (!first)
				text += ' ';
			first = false;
			if (name)
			{
				text += name;
				text += '=';
			}
		}
	};

	// ------------------------------------------------------------------------------------------------------------
	// Collects every Entity field of a record (entity timelines, handle validation).

	class EntityCollector : public ArchiveBase<EntityCollector>
	{
	public:
		static constexpr bool Reading = false;

		std::vector<Entity> entities;

		template<class T> void Primitive(const char*, T&) {}
		void EntityRef(const char*, Entity& entity)
		{
			if (entity != NullEntity)
				entities.push_back(entity);
		}
		void String(const char*, std::string&) {}
		void BeginObject(const char*) {}
		void EndObject() {}
		void BeginArray(const char*, uint32_t&) {}
		void EndArray() {}

		template<class T> void Add(const T& value) { VisitConst(*this, value); }
	};
}
