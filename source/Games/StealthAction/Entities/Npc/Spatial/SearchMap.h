#pragma once

// Where a hidden target may be (NPC_AI_DESIGN.md §11.4, after The Last of Us): a probability per NavGrid cell, seeded
// where it was last known and spreading from there at the walk speed of the hidden one. Cells the searchers can see
// hold nothing and take nothing in, so the probability is squeezed into what nobody has looked at.
//
// The map knows nothing of the target: it is made from a place and a certainty, and changed by what is seen.

#include "../Brain/NpcData.h"

#include <Helpers/JsonHelper.hpp>
#include <Helpers/StringHelper.h> // the global `json` alias
#include <glm.h>

#include <array>
#include <vector>

class SearchMap
{
public:

	// Starts over around a place: the cells reachable from it (up to maxCells), one unit of probability spread over the
	// cells within `(1 - certainty) * uncertaintyRadius` of it. False when the place is not on the navigation grid.
	bool Seed(const glm::vec3& place, float certainty, const NpcSearchTuning& tuning);

	// The same with the radius given instead of worked out from a certainty (a body: how fresh the kill is).
	bool SeedRadius(const glm::vec3& place, float radius, const NpcSearchTuning& tuning);

	bool Seeded() const { return cells.empty() == false; }

	// Spreads the probability for the time passed (in steps of `spreadInterval`).
	void Spread(float dt, double now);

	// A ray from a searcher reached the cell: it is seen, and holds no probability while it stays so.
	void MarkSeen(int cell, double now);
	bool IsSeen(int cell, double now) const;

	int Count() const { return static_cast<int>(cells.size()); }
	const glm::vec3& Floor(int cell) const { return cells[cell].floor; }
	int GridIndex(int cell) const { return cells[cell].grid; }
	const std::array<int, 8>& Links(int cell) const { return cells[cell].link; }
	float Probability(int cell) const { return cells[cell].probability; }

	// What is left to look for, as a part of what was there at the start.
	float Mass() const { return mass; }
	float RemainingFraction() const { return startMass > 0.0f ? mass / startMass : 0.0f; }

	// The probability in a cell and its neighbours: what a searcher standing there would be close to.
	float Nearby(int cell) const;

	// The cell nearest to a position (horizontally), -1 when the map is empty.
	int Nearest(const glm::vec3& position) const;

	// A doorway: a cell that can be left only in two opposite directions.
	bool IsChoke(int cell) const { return cells[cell].choke; }

	// Where a searcher at `from` should look: the unseen cell with the most probability next to what is seen (the edge of
	// the known, where a hiding target is most likely), within `range`. False when no probability is left in reach.
	bool FrontierGaze(const glm::vec3& from, float range, double now, glm::vec3& point) const;

	// The cells that still hold probability above a part of the peak, with their grid index, to save.
	json Serialize(float relativeThreshold) const;
	bool Deserialize(const json& data, const NpcSearchTuning& tuning);

private:

	struct Cell
	{
		int grid = -1;                    // NavGrid index
		glm::vec3 floor = glm::vec3(0.0f);
		std::array<int, 8> link = { -1, -1, -1, -1, -1, -1, -1, -1 }; // index in this map, -1 = none
		float probability = 0.0f;
		double seenUntil = -1.0e9;
		bool choke = false;
	};

	NpcSearchTuning tuning;
	glm::vec3 origin = glm::vec3(0.0f);
	std::vector<Cell> cells;
	float mass = 0.0f;
	float startMass = 0.0f;
	double lastSpread = 0.0;

	bool Build(const glm::vec3& place, const NpcSearchTuning& newTuning);
	void RecountMass();
};
