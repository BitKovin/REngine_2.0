#pragma once

// The sim's part of a game save (SLICE_M2B_BRIDGE.md section 6.3): a header that ties the snapshot to the world it was made in,
// then the kernel snapshot. Pure data and bytes, no engine and no World.

#include <Sim/Core/SimArchive.h>
#include <Sim/Core/SimTypeRegistry.h>

#include <string>
#include <vector>

namespace sim
{
	struct SimSaveHeader
	{
		Tick tick = 0;
		uint64_t schema = 0;      // SchemaHash of the registry that wrote it: component names and versions
		uint64_t fingerprint = 0; // WorldNavData::Fingerprint of the layout the positions are in
		std::vector<std::string> levels;  // level names of that layout, in order
		std::vector<uint8_t> placements;  // MovementModule::WritePlacements, opaque here
		template<class Ar> void Visit(Ar& ar)
		{
			ar("tick", tick);
			ar("schema", schema);
			ar("fingerprint", fingerprint);
			ar("levels", levels);
			ar("placements", placements);
		}
	};

	struct SimSave
	{
		SimSaveHeader header;
		std::vector<uint8_t> world; // World::Save
	};

	// Hash of what decides whether saved components can be read back: every component's name and version, in order.
	uint64_t SchemaHash(const TypeRegistry& types);

	// Compressed file bytes: magic, format version, header, world.
	std::vector<uint8_t> WriteSimSave(const SimSave& save);
	// False with `error` for anything that is not a sim save of this format version (corrupt, truncated, older, newer).
	bool ReadSimSave(const std::vector<uint8_t>& bytes, SimSave& out, std::string& error);

	// The header alone, for the Host's Load message (the world blob travels separately).
	std::vector<uint8_t> WriteHeader(const SimSaveHeader& header);
	bool ReadHeader(const std::vector<uint8_t>& bytes, SimSaveHeader& out);
}
