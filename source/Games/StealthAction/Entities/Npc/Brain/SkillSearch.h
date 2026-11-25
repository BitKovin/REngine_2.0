#pragma once

#include "NpcSkill.h"

// Looks for a hostile that is out of sight (NPC_AI_DESIGN.md §11.4, §11.5, §12). The roles of a search situation say
// what it does: the checker walks to the original spot and looks round, a searcher walks to the cell it claims on the
// search map, a watcher holds a post by a doorway. All of them look at what the situation says (the edge of what is
// seen next to the most probability, or the doorway), not where the path bends.
class SearchSkill : public NpcSkill
{
public:

	const char* Id() const override { return "search"; }

	float Want(const NpcContext& context) const override;
	void Start(NpcContext& context) override { context.mind.Bark("search_start"); }
	void Update(NpcContext& context) override;
	void Stop(NpcContext& context) override { Reset(); }
	float MinCommit() const override { return 1.0f; }

	json Serialize(double now) const override;
	void Deserialize(const json& data, double now) override;
	std::string Describe(double now) const override;

private:

	NpcRole role = NpcRole::None;   // what it is doing now (for the panel and the checker's look)
	double standingSince = -1.0;    // since when it has been at its destination
	bool reported = false;          // the checker has told the situation it looked

	void Reset();
};
