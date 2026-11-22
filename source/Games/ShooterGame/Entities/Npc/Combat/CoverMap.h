#pragma once

#include "CoverTypes.h"

#include <Physics.h>

#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

// Threat-independent, lazily filled cover knowledge of the level
// (COMBAT_AI_DESIGN.md §10.2).
//
// The level is divided into 1 m cells. For a cell and one of 16 compass
// sectors the map stores whether something solid stands within ProbeDistance
// at crouch head height and at standing head height. A cover query only asks
// for the sector facing its threat, so a sector is probed (two traces) the
// first time anyone needs it and never again. Props are cached too even
// though they can move; CoverFinder verifies the winner with a real trace.
//
// Thread safe: queries come from the NPCs' worker threads.
class CoverMap
{
public:
	static constexpr int SectorCount = 16;
	static constexpr float CellSize = 1.0f;
	static constexpr float ProbeDistance = 1.5f;
	static constexpr float ProbeRadius = 0.2f;
	static constexpr float CrouchHeadHeight = 0.9f;
	static constexpr float StandHeadHeight = 1.6f;
	static constexpr int TracesPerProbe = 2;

	// What stops a bullet and so counts as cover: level geometry and physics
	// props (crates, tables, doors). Characters are CharacterCapsule and don't count.
	static BodyType BlockerMask() { return BodyType::World | BodyType::MainBody; }

	struct Exposure
	{
		bool lowBlocked = false;  // solid at crouch head height in that direction
		bool highBlocked = false; // solid at standing head height in that direction
	};

	// Centre of the cell containing `floorPos`, at the same height.
	static vec3 SnapToCell(const vec3& floorPos);

	// Sector (0..15) a horizontal direction falls in.
	static int SectorOf(const vec3& direction);

	// Fills `out` for the cell at `cellCentre` and the sector facing `direction`.
	// Probing costs TracesPerProbe from `traceBudget`; returns false (and
	// leaves the budget) when the sector is unknown and the budget is spent.
	static bool Query(const vec3& cellCentre, const vec3& direction, Exposure& out, int& traceBudget);

	// Forget everything (level change).
	static void Clear();

	static size_t KnownSectors();

private:
	struct Cell
	{
		uint16_t known = 0;
		uint16_t low = 0;
		uint16_t high = 0;
	};

	static inline std::unordered_map<int64_t, Cell> cells;
	static inline std::mutex mutex;

	static int64_t KeyOf(const vec3& pos);
	static vec3 SectorDirection(int sector);
};
