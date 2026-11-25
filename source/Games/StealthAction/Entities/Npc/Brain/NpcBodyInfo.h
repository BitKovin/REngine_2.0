#pragma once

// What a body is, as an NPC that looks at it can tell (NPC_AI_DESIGN.md §13.1): whether the NPC is dead or only out cold,
// what killed it, and since when. The NPC that lies there owns the facts (Npc::GetBodyInfo); whoever finds it copies them
// into a Body record of its own memory, so the record is what it knows and the body may change afterwards.

#include <string>

enum class NpcBodyState : int
{
	Alive,
	Unconscious,
	Dead
};

enum class NpcBodyCause : int
{
	Unknown,
	Blade,
	Gun,
	Explosion,
	Fire,
	Drained
};

inline const char* NpcBodyStateName(NpcBodyState state)
{
	switch (state)
	{
	case NpcBodyState::Unconscious: return "unconscious";
	case NpcBodyState::Dead: return "dead";
	default: return "alive";
	}
}

inline const char* NpcBodyCauseName(NpcBodyCause cause)
{
	switch (cause)
	{
	case NpcBodyCause::Blade: return "blade";
	case NpcBodyCause::Gun: return "gun";
	case NpcBodyCause::Explosion: return "explosion";
	case NpcBodyCause::Fire: return "fire";
	case NpcBodyCause::Drained: return "drained";
	default: return "unknown";
	}
}

struct NpcBodyInfo
{
	NpcBodyState state = NpcBodyState::Dead;
	NpcBodyCause cause = NpcBodyCause::Unknown;
	double since = 0.0;            // game time it went down
	bool hidden = false;           // in a hiding spot: not a perception subject

	bool Drained() const { return state == NpcBodyState::Dead && cause == NpcBodyCause::Drained; }

	// The text a Body record keeps in its `detail`: "dead:blade", "unconscious:unknown".
	std::string Detail() const { return std::string(NpcBodyStateName(state)) + ":" + NpcBodyCauseName(cause); }

	static NpcBodyState StateOfDetail(const std::string& detail)
	{
		if (detail.rfind("unconscious", 0) == 0) return NpcBodyState::Unconscious;
		return NpcBodyState::Dead;
	}

	static bool DrainedDetail(const std::string& detail)
	{
		return detail.size() >= 7 && detail.compare(detail.size() - 7, 7, "drained") == 0;
	}
};
