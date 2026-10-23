#include "WorldNavData.h"

#include <Helpers/ByteCompressor.h>
#include <Navigation/NavigationFileHelper.h>

#include <cstring>
#include <exception>

namespace sim::nav
{
	namespace
	{
		constexpr uint32_t WorldMagic = 0x564E5753; // "SWNV"
	}

	bool ReadLevelNavFile(const std::vector<uint8_t>& fileBytes, LevelNavData& out, std::string& error)
	{
		using Helper = NavigationFileHelper;
		out = LevelNavData{};
		if (fileBytes.empty())
		{
			error = "empty file";
			return false;
		}
		std::vector<uint8_t> buffer;
		try
		{
			buffer = ByteCompressor::DecompressData(fileBytes);
		}
		catch (const std::exception& exception)
		{
			error = std::string("cannot decompress: ") + exception.what();
			return false;
		}
		size_t position = 0;
		auto read = [&](void* destination, size_t size)
		{
			if (position + size > buffer.size())
				return false;
			std::memcpy(destination, buffer.data() + position, size);
			position += size;
			return true;
		};

		Helper::NavMeshFileHeader fileHeader{};
		if (!read(&fileHeader, sizeof(fileHeader)) || fileHeader.magic != Helper::NAVMESH_FILE_MAGIC)
		{
			error = "not an engine .nav file";
			return false;
		}
		out.sourceVersion = fileHeader.version;

		position = fileHeader.navMeshOffset;
		Helper::NavMeshSetHeader setHeader{};
		if (!read(&setHeader, sizeof(setHeader)))
		{
			error = "truncated nav mesh header";
			return false;
		}
		out.params = setHeader.params;
		for (uint32_t i = 0; i < setHeader.numTiles; ++i)
		{
			Helper::NavMeshTileHeader tileHeader{};
			if (!read(&tileHeader, sizeof(tileHeader)) || tileHeader.dataSize > buffer.size() - position)
			{
				error = "truncated tile " + std::to_string(i);
				return false;
			}
			out.tiles.emplace_back(buffer.begin() + position, buffer.begin() + position + tileHeader.dataSize);
			position += tileHeader.dataSize;
		}
		return true;
	}

	std::vector<uint8_t> WorldNavData::Serialize() const
	{
		BinaryWriter ar;
		uint32_t magic = WorldMagic, version = FormatVersion;
		ar.Primitive(nullptr, magic);
		ar.Primitive(nullptr, version);
		ar.Write(*this);
		return ByteCompressor::CompressData(ar.bytes);
	}

	bool WorldNavData::Deserialize(const std::vector<uint8_t>& bytes, WorldNavData& out, std::string& error)
	{
		std::vector<uint8_t> raw;
		try
		{
			raw = ByteCompressor::DecompressData(bytes);
		}
		catch (const std::exception& exception)
		{
			error = std::string("cannot decompress: ") + exception.what();
			return false;
		}
		BinaryReader ar(raw.data(), raw.size());
		uint32_t magic = 0, version = 0;
		ar.Primitive(nullptr, magic);
		ar.Primitive(nullptr, version);
		if (magic != WorldMagic || version != FormatVersion)
		{
			error = "not a world nav file of format " + std::to_string(FormatVersion);
			return false;
		}
		out = WorldNavData{};
		ar.Read(out);
		if (ar.Failed() || !ar.AtEnd())
		{
			error = "world nav file is truncated or corrupt";
			return false;
		}
		return true;
	}

	uint64_t WorldNavData::Fingerprint() const
	{
		HashArchive hash;
		for (const WorldLevel& level : levels)
		{
			hash.Add(level.name);
			hash.Add(level.offset);
			hash.Add(level.tileShiftX);
			hash.Add(level.tileShiftY);
		}
		for (const WorldLink& link : links)
			hash.Add(link);
		return hash.hash;
	}

	int WorldNavData::FindLevel(const std::string& name) const
	{
		for (size_t i = 0; i < levels.size(); ++i)
			if (levels[i].name == name)
				return static_cast<int>(i);
		return -1;
	}

	std::string StaleLevel::Describe() const
	{
		if (current == 0)
			return name + ": the map file is missing (the world file was baked from version " + std::to_string(baked) + ")";
		return name + ": the map changed after the bake (baked from version " + std::to_string(baked) + ", map is now " + std::to_string(current) + ")";
	}

	std::vector<StaleLevel> FindStaleLevels(const WorldNavData& data, const std::function<uint32_t(const std::string&)>& mapVersion)
	{
		std::vector<StaleLevel> stale;
		for (const WorldLevel& level : data.levels)
		{
			const uint32_t current = mapVersion(level.name);
			if (current != level.sourceVersion)
				stale.push_back({ level.name, level.sourceVersion, current });
		}
		return stale;
	}
}
