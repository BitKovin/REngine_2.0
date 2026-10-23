#include "SimSaveFile.h"

#include <Helpers/ByteCompressor.h>

#include <exception>

namespace sim
{
	namespace
	{
		constexpr uint32_t SaveMagic = 0x4D495353; // "SSIM"
		constexpr uint32_t SaveFormatVersion = 1;
	}

	uint64_t SchemaHash(const TypeRegistry& types)
	{
		HashArchive hash;
		for (const ComponentTypeInfo& component : types.Components())
		{
			hash.Add(component.name);
			hash.Add(component.version);
		}
		return hash.hash;
	}

	std::vector<uint8_t> WriteSimSave(const SimSave& save)
	{
		BinaryWriter ar;
		uint32_t magic = SaveMagic, version = SaveFormatVersion;
		ar.Primitive(nullptr, magic);
		ar.Primitive(nullptr, version);
		ar.Write(save.header);
		ar.Write(save.world);
		return ByteCompressor::CompressData(ar.bytes);
	}

	bool ReadSimSave(const std::vector<uint8_t>& bytes, SimSave& out, std::string& error)
	{
		std::vector<uint8_t> raw;
		try
		{
			raw = ByteCompressor::DecompressData(bytes);
		}
		catch (const std::exception& exception)
		{
			error = std::string("cannot decompress the sim save: ") + exception.what();
			return false;
		}
		BinaryReader ar(raw.data(), raw.size());
		uint32_t magic = 0, version = 0;
		ar.Primitive(nullptr, magic);
		ar.Primitive(nullptr, version);
		if (ar.Failed() || magic != SaveMagic)
		{
			error = "not a sim save";
			return false;
		}
		if (version != SaveFormatVersion)
		{
			error = "sim save format " + std::to_string(version) + " is not readable (this build reads " + std::to_string(SaveFormatVersion) + ")";
			return false;
		}
		out = SimSave{};
		ar.Read(out.header);
		ar.Read(out.world);
		if (ar.Failed() || !ar.AtEnd())
		{
			error = "the sim save is truncated or corrupt";
			return false;
		}
		return true;
	}

	std::vector<uint8_t> WriteHeader(const SimSaveHeader& header)
	{
		BinaryWriter ar;
		ar.Write(header);
		return std::move(ar.bytes);
	}

	bool ReadHeader(const std::vector<uint8_t>& bytes, SimSaveHeader& out)
	{
		BinaryReader ar(bytes.data(), bytes.size());
		out = SimSaveHeader{};
		ar.Read(out);
		return !ar.Failed() && ar.AtEnd();
	}
}
