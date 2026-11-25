#pragma once

// A shared plan around one subject (NPC_AI_DESIGN.md §10). The first NPC that meets a concern opens it, the others join
// when they learn of the subject. A situation keeps planning data (who does what, the search map, the chase flood) and
// no knowledge: where the subject is comes from its members' memories every time (§4.1 rule 1).
//
// Situations change on the main thread only, between the NPCs' updates, so the NPCs read them freely while they run.

#include "../Brain/NpcMessage.h"
#include "../Spatial/ChaseFlood.h"
#include "../Spatial/SearchMap.h"

#include <glm.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class NpcDirector;

// What a role holder is told to do: where to go, where to look.
struct NpcOrder
{
	NpcRole role = NpcRole::None;
	bool hasDestination = false;
	glm::vec3 destination = glm::vec3(0.0f);
	bool hasLook = false;
	glm::vec3 look = glm::vec3(0.0f);
};

struct SituationMember
{
	std::string npcId;
	NpcOrder order;

	// Alarm: the bell the alarm runner is sent to (an alarm point's position)
	glm::vec3 alarmPosition = glm::vec3(0.0f);

	// Search planning
	int claim = -1;                 // search map cell a searcher walks to
	double claimedAt = 0.0;
	double arrivedAt = -1.0;        // since when the searcher has stood at its claim
	double lookChosenAt = -1.0e9;   // when the look point was last picked
};

class NpcSituation
{
public:

	NpcSituation(SituationKind newKind, const std::string& newSubject, int newId) : kind(newKind), subject(newSubject), id(newId) {}
	virtual ~NpcSituation() = default;

	SituationKind Kind() const { return kind; }
	const std::string& Subject() const { return subject; }
	int Id() const { return id; }
	bool Finished() const { return finished; }

	const std::vector<SituationMember>& Members() const { return members; }
	std::vector<SituationMember>& Members() { return members; }

	const SituationMember* FindMember(const std::string& npcId) const;
	SituationMember* FindMember(const std::string& npcId);

	// The role and the orders of an NPC in this situation (None / empty when it is not a member).
	NpcOrder OrderOf(const std::string& npcId) const;

	void Join(const std::string& npcId);
	void Leave(const std::string& npcId);

	// The first member has joined: where the situation starts from.
	virtual void Open(NpcDirector& director, double now) = 0;

	// Once a frame, after the members that cannot act have left.
	virtual void Step(NpcDirector& director, double now, float dt) = 0;

	// A role holder reports.
	virtual void OnProgress(NpcDirector& director, const std::string& npcId, NpcProgress progress, double now) {}

	virtual json Serialize(double now) const;
	virtual void Deserialize(const json& data, double now);

	// One line for the debug panel.
	virtual std::string Describe(double now) const = 0;

protected:

	SituationKind kind;
	std::string subject;
	int id = 0;
	bool finished = false;
	std::vector<SituationMember> members;
};

// A noise or a sighting to look into: one investigator goes, a backup covers when somebody is Wary, the others glance
// and keep their routine (§12).
class InvestigateSituation : public NpcSituation
{
public:

	InvestigateSituation(const std::string& subject, int id) : NpcSituation(SituationKind::Investigate, subject, id) {}

	void Open(NpcDirector& director, double now) override;
	void Step(NpcDirector& director, double now, float dt) override;
	void OnProgress(NpcDirector& director, const std::string& npcId, NpcProgress progress, double now) override;

	std::string Describe(double now) const override;

private:

	void AssignRoles(NpcDirector& director);
};

// A hostile to find (§11.3, §11.4): while somebody sees it nothing is planned, after that the chasers run to where the
// flood points, then the map decides where the searchers go and what the watchers hold. The subject can also be a body
// (§13.1): then there is nobody to chase, the map is spread round the body (wider for a fresh kill), and the one who checks
// the place tends the body.
class SearchSituation : public NpcSituation
{
public:

	enum class Phase : int
	{
		Contact,   // a member sees the subject: the others close on the freshest news
		Chase,     // it was lost: the flood says where it went
		Search     // the chasers found nothing (or never chased): the map says where to look
	};

	SearchSituation(const std::string& subject, int id) : NpcSituation(SituationKind::Search, subject, id) {}

	void Open(NpcDirector& director, double now) override;
	void Step(NpcDirector& director, double now, float dt) override;
	void OnProgress(NpcDirector& director, const std::string& npcId, NpcProgress progress, double now) override;

	json Serialize(double now) const override;
	void Deserialize(const json& data, double now) override;

	std::string Describe(double now) const override;

	Phase GetPhase() const { return phase; }

	// The subject is a body, not somebody to be seen again.
	bool BodySubject() const { return bodySubject; }
	const ChaseFlood& GetFlood() const { return flood; }
	const SearchMap& GetMap() const { return map; }

private:

	Phase phase = Phase::Search;
	bool bodySubject = false;
	double phaseSince = 0.0;
	double lastContact = -1.0e9;

	// Chase
	ChaseFlood flood;
	double floodTime = 0.0;          // the time of the record the flood was made from
	int floodRuns = 0;
	bool chaseArrived = false;

	// Search
	SearchMap map;
	glm::vec3 checkPoint = glm::vec3(0.0f);   // the original spot, for the one who checks it
	bool checked = false;
	double searchSince = 0.0;
	double nextClaims = 0.0;
	int clearCursor = 0;

	void EnterContact(double now);
	void EnterChase(const NpcRecord& news, double now);
	void StartFlood(const NpcRecord& news);
	void EnterSearch(NpcDirector& director, const NpcRecord& news, bool fromChase, double now);

	void StepContact(NpcDirector& director, const NpcRecord& news);
	void StepChase(NpcDirector& director, const NpcRecord& news, double now);
	void StepSearch(NpcDirector& director, double now, float dt);

	void UpdateAlarmRunner(NpcDirector& director, double now);
	void ClearSeen(NpcDirector& director, double now);
	void AssignSearchRoles(NpcDirector& director, double now);
	void UpdateClaims(NpcDirector& director, double now);
	void UpdateLooks(NpcDirector& director, double now);

	int ChooseClaim(const glm::vec3& from, const std::vector<glm::vec3>& taken, double now) const;
	std::unordered_map<int, float> ChokeMasses() const;
	bool FindWatchPost(const glm::vec3& from, const std::vector<glm::vec3>& taken, const std::unordered_map<int, float>& masses,
		int& choke, glm::vec3& post, glm::vec3& look) const;
	void PlanCheck(NpcDirector& director, SituationMember& member) const;
};
