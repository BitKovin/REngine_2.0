#pragma once

// What the NPCs of a level share (NPC_AI_DESIGN.md §4, §7, §10): the message queue, the situations and the roles in them.
// An Entity that the first NPC of a level makes (like the player start makes the player), saved with the level.
// `LateUpdate` is the once-a-frame step on the main thread, after every NPC's update: it collects what the NPCs queued,
// delivers the messages that are due, changes the situations and tells the NPCs what became of them. NPCs never write
// each other; everything between them goes through here (§4.1 rule 3). It also keeps what belongs to no single NPC: the
// floors under the alert of an area (§6.4), the alarms that were rung (§17.11) and who may speak when (§18.2).

#include <Entity.h>

#include "NpcSituation.h"
#include "../Brain/NpcMessage.h"
#include "../Brain/NpcMind.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

class Npc;
class AlarmPoint;

class NpcDirector : public Entity
{
public:

	NpcDirector();
	~NpcDirector();

	// The director of the level that is loaded, or null before the first NPC started.
	static NpcDirector* Get() { return instance; }

	// The director of this level: made (and started) when there is none yet.
	static NpcDirector& Ensure();

	void Start() override;
	void LateUpdate() override;
	void UpdateDebugUI() override;

	void Serialize(json& target) override;
	void Deserialize(json& source) override;

	// ── NPCs ────────────────────────────────────────────────────────────────────────────────────────────────────────

	// An NPC joins in when it starts and leaves when it is destroyed.
	void Register(Npc* npc);
	static void Unregister(Npc* npc);

	// The NPC with this id when it is there and can act (alive, not destroyed), null otherwise.
	Npc* FindActing(const std::string& npcId) const;
	const std::vector<Npc*>& Npcs() const { return npcs; }

	// ── Reading (NPC threads, while the situations do not change) ───────────────────────────────────────────────────

	struct Membership
	{
		const NpcSituation* situation = nullptr;
		NpcOrder order;
	};

	// The situation of this kind an NPC is part of (the first one), false when it is part of none.
	bool FindMembership(const std::string& npcId, SituationKind kind, Membership& membership) const;

	// The order of an NPC in the situation of this kind it is part of: false when there is no director or it is part of none.
	static bool FindOrder(const std::string& npcId, SituationKind kind, NpcOrder& order, std::string& subject);

	// Every situation an NPC is part of.
	void MembershipsOf(const std::string& npcId, std::vector<std::pair<SituationKind, std::string>>& out) const;

	bool IsMember(const std::string& npcId, SituationKind kind, const std::string& subject) const;

	const std::vector<std::unique_ptr<NpcSituation>>& Situations() const { return situations; }
	const NpcSituation* FindSituation(SituationKind kind, const std::string& subject) const;

	// The newest Person record about a situation's subject among its members (rule 1: nothing else says where it is).
	const NpcRecord* Freshest(const NpcSituation& situation) const;

	// Messages sent but not delivered yet (the tests look at them).
	int PendingMessages() const { return static_cast<int>(pending.size()); }

	// ── Areas (§6.4) ────────────────────────────────────────────────────────────────────────────────────────────────

	// Something happened here: the area round it keeps an alert floor by the data of the event (none for a minor one).
	// NPCs ask for it through their outbox; the player's verbs (powers, M6) call it directly.
	void RaiseAreaEvent(NpcAreaEvent type, const glm::vec3& position, double now);

	// The lowest alert level an NPC standing here goes back to (read by NPC threads while nothing changes the floors).
	NpcAlert FloorAt(const glm::vec3& position, double now) const;

	// ── Alarms (§17.11) ─────────────────────────────────────────────────────────────────────────────────────────────

	// The bell at `point` has been rung and its area is still on alert.
	bool AlarmActive(const class AlarmPoint& point, double now) const;

	// ── Voices (§18.2) ──────────────────────────────────────────────────────────────────────────────────────────────

	struct BarkEntry
	{
		double time = 0.0;
		std::string npcId;
		std::string bark;
		int tier = 1;
		std::string event;      // the FMOD event that was asked for (it plays nothing when the bank has no such event)
	};

	// The last barks that were said, newest last (the debug panel and the tests read it).
	const std::vector<BarkEntry>& BarkLog() const { return barkLog; }

	// Delivery of due messages, requests and the situations' steps; LateUpdate calls it once a frame.
	void Step(double now, float dt);

private:

	static inline NpcDirector* instance = nullptr;

	// The floor an event left under the alert of the NPCs round a place.
	struct AreaFloor
	{
		NpcAreaEvent type = NpcAreaEvent::BodyFound;
		glm::vec3 center = glm::vec3(0.0f);
		float radius = 30.0f;
		NpcAlert level = NpcAlert::Wary;
		NpcAlert then = NpcAlert::Wary;     // what is left when `until` has passed
		double until = -1.0;                // game time; negative: for the rest of the level
	};

	struct PendingBark
	{
		Npc* npc = nullptr;
		std::string bark;
		float priority = 0.0f;
	};

	struct Voice
	{
		std::string npcId;
		glm::vec3 position = glm::vec3(0.0f);
		double time = -1.0e9;
		int situation = 0;      // the situation it spoke for (0: none)
	};

	std::vector<AreaFloor> floors;
	std::vector<PendingBark> barkQueue;
	std::vector<Voice> voices;                // who spoke lately (spacing, overlap, the global cap)
	std::map<std::string, int> barkUses;      // per bark, how often it was said in this level (the tier)
	std::vector<BarkEntry> barkLog;

	std::vector<Npc*> npcs;                                  // in the order they started
	std::vector<std::unique_ptr<NpcSituation>> situations;
	std::vector<NpcMessage> pending;                         // sent, due later
	int nextSituationId = 1;
	int alarmsRung = 0;

	NpcSituation& OpenSituation(SituationKind kind, const std::string& subject);
	NpcSituation* FindSituationMutable(SituationKind kind, const std::string& subject);

	void CollectFromNpcs(double now);
	void HandleRequest(Npc& npc, const NpcRequest& request, double now);
	void DeliverMessages(double now);
	void Deliver(const NpcMessage& message);
	void StepSituations(double now, float dt);

	void RaiseFloor(NpcAreaEvent type, const glm::vec3& position, float radius, double now);
	void RingAlarm(Npc& runner, const NpcRequest& request, double now);
	void ArbitrateBarks(double now);
	int SituationIdOf(const std::string& npcId) const;
};
