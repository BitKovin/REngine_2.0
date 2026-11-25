#pragma once

// What an NPC makes of what it perceives (NPC_AI_DESIGN.md §6): awareness meters, memory records, concerns, alert
// and fear. The Mind reads the observer's results and decides nothing about movement: the skills read the Mind.
// Worker thread only (it is updated from the NPC's AsyncUpdate).

#include <AiPerception/Observer.h>
#include <AiPerception/ObservationTarget.h>

#include "NpcBodyInfo.h"
#include "NpcData.h"
#include "NpcMemory.h"
#include "NpcMessage.h"
#include "NpcSounds.h"

#include <map>
#include <string>
#include <vector>

enum class NpcAlert : int
{
	Calm,
	Wary,
	Alarmed,
	Combat
};

inline const char* NpcAlertName(NpcAlert alert)
{
	switch (alert)
	{
	case NpcAlert::Calm: return "calm";
	case NpcAlert::Wary: return "wary";
	case NpcAlert::Alarmed: return "alarmed";
	default: return "combat";
	}
}

// The level a data file names ("wary"); calm when it is not one.
inline NpcAlert NpcAlertByName(const std::string& name)
{
	for (NpcAlert alert : { NpcAlert::Wary, NpcAlert::Alarmed, NpcAlert::Combat })
	{
		if (name == NpcAlertName(alert)) return alert;
	}
	return NpcAlert::Calm;
}

// What the NPC has to look into. Ordered by weight: a threat outranks an alarm outranks a body outranks curiosity.
enum class NpcConcernType : int
{
	Curious,
	Body,    // a body was found (dead or out cold): look after it
	Alarm,
	Threat
};

// Where to look is not stored here: it is the position of the subject's record in the NPC's memory (§4.1 rule 1: a person,
// a body, an event, an object), which always exists while the concern does.
struct NpcConcern
{
	std::string subject;                           // who or what it is about; one concern per subject
	NpcConcernType type = NpcConcernType::Curious;
	double time = 0.0;                             // when it was last raised

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(NpcConcern, subject, type, time)
};

// Awareness of one subject, 0..1 (§6.2).
struct NpcMeter
{
	std::string subject;
	NpcRecordKind kind = NpcRecordKind::Person;   // a person, or a body (what is being made out)
	float value = 0.0f;
	double lastStimulus = -1.0e9;
	double lastSight = -1.0e9;       // sound alone never identifies

	// Per update, not saved: where and how the subject was perceived this update. The place the NPC remembers is the
	// Person record in its memory.
	float previous = 0.0f;           // value when the update began (threshold crossings)
	bool stimulated = false;
	bool sensed = false;             // seen during this update
	vec3 position = vec3();
	NpcRecordSource source = NpcRecordSource::Saw;
	bool curiousSound = false;       // a sound that is worth looking into pushed it past Notice this update

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(NpcMeter, subject, kind, value, lastStimulus, lastSight)
};

enum class NpcMindEventType
{
	Noticed,    // a meter crossed Notice: glance
	Suspected,  // crossed Suspect
	Identified, // crossed Identify: it is the hostile
	Heard,      // a sound
	Hurt
};

struct NpcMindEvent
{
	NpcMindEventType type = NpcMindEventType::Noticed;
	std::string subject;
	vec3 position = vec3();
};

class NpcMind
{
public:

	NpcAlert alert = NpcAlert::Calm;
	float fear = 0.0f;
	NpcMemory memory;

	void Setup(const NpcProfile* newProfile) { profile = newProfile; }

	// Who this is, and the squad it belongs to (its members are the ones it notices missing, §13.3). Squads are named in the map.
	void SetSelf(const std::string& id, const std::string& squadName)
	{
		selfId = id;
		squad = squadName;
		squadTag = squadName.empty() ? hashed_string() : hashed_string("squad:" + squadName);
	}

	// One update from the observer's results. `now` is game time.
	void Update(const Observer& observer, float dt, double now);

	// A hit. `origin` is where it came from as far as the NPC can tell (the shot's direction); `attackerId` may be empty.
	void OnDamaged(float amount, const std::string& attackerId, const vec3& origin, double now);

	// The concern to act on now: the heaviest, the newest among equals. Null when there is none.
	const NpcConcern* TopConcern() const;

	// Where a concern points: the position of its subject's record.
	vec3 ConcernPosition(const NpcConcern& concern) const;

	// The NPC was woken after being out cold: it remembers where it went down and goes to look into it (§13.1).
	void OnWoken(const vec3& where, double now);

	const std::vector<NpcConcern>& Concerns() const { return concerns; }
	const std::vector<NpcMeter>& Meters() const { return meters; }

	// The investigator looked into a subject's concern and found nothing more.
	void Resolve(const std::string& subject);

	// What the director handed over since the last update (messages told on, situations that ended): called first in the
	// NPC's update, before the senses.
	void ProcessInbox(double now);

	// The subject is being seen right now (this update), and when it was last seen by this NPC or by whoever told it.
	bool Sees(const std::string& subject) const;
	double LastSeen(const std::string& subject) const;

	// No concern is left to look into: an Alarmed NPC calms to Wary.
	void FinishSearch(double now);

	// A hostile is being seen right now.
	bool SeesHostile() const { return hostileSensed; }

	// Meter value for a subject, 0 when there is none.
	float MeterValue(const std::string& subject) const;

	// Messages and requests for the director, and what it handed back. The NPC stamps the sender on the messages and the
	// director empties the outbox (main thread, after the NPCs' updates); the inbox is filled the same way and read first.
	NpcOutbox outbox;
	NpcInbox inbox;

	// Asks the director for a part in a situation, gives it up, or reports how a role is going.
	void Request(NpcRequestType type, SituationKind kind, const std::string& subject, NpcProgress progress = NpcProgress::Resolved)
	{
		NpcRequest request;
		request.type = type;
		request.kind = kind;
		request.subject = subject;
		request.progress = progress;
		outbox.requests.push_back(std::move(request));
	}

	// The alarm runner rings the bell at `bell` and tells what it knows of the intruder; a guard has tended an unconscious
	// body and shakes it awake.
	void RequestAlarm(const vec3& bell, const NpcRecord& intruder)
	{
		NpcRequest request;
		request.type = NpcRequestType::RaiseAlarm;
		request.position = bell;
		request.record = intruder;
		outbox.requests.push_back(std::move(request));
	}

	void RequestWake(const std::string& body)
	{
		NpcRequest request;
		request.type = NpcRequestType::WakeBody;
		request.subject = body;
		outbox.requests.push_back(std::move(request));
	}

	// Something the NPC would like to say; the director decides whether it is heard (§18.2).
	void Bark(const char* bark);

	// An event the area keeps a floor under its alert for (§6.4).
	void RequestAreaEvent(NpcAreaEvent type, const vec3& position, double now);

	// Events of the last update (glance targets); the owner clears them after reading.
	const std::vector<NpcMindEvent>& Events() const { return events; }
	void ClearEvents() { events.clear(); }

	// Range multiplier of the vision zones for the current alert.
	float VisionRangeScale() const;

	// Times are saved as ages, so a load restores them whatever the game time is.
	json Serialize(double now) const;
	void Deserialize(const json& data, double now);

private:

	const NpcProfile* profile = nullptr;

	// Whether a subject is a player (the only hostile of M1), asked once per subject: tags change every frame.
	std::map<std::string, bool> playerCache;

	std::vector<NpcMeter> meters;
	std::vector<NpcConcern> concerns;
	std::vector<NpcMindEvent> events;

	bool hostileSensed = false;
	double lastHostileTime = -1.0e9;
	double lastConcernTime = -1.0e9;
	std::map<std::string, double> lastShout; // per subject, when this NPC last called out about it
	std::map<std::string, double> lastReport; // per squadmate, when this NPC last told it what it knows
	std::map<int, double> lastAreaEvent;      // per kind of event, when the area was last told

	std::string selfId;
	std::string squad;
	hashed_string squadTag;

	// A squadmate that is around (§13.3).
	struct Companion
	{
		std::string id;
		vec3 position = vec3();    // where it was last sensed
		double lastSensed = -1.0e9;
		float company = 0.0f;      // seconds in earshot
		double nextAllowed = 0.0;  // no new worry about it before this
	};

	std::vector<Companion> companions;

	NpcMeter& MeterOf(const std::string& subject);

	void SenseSight(const Observer& observer, float dt, double now, const NpcPerceptionTuning& tuning);
	void SenseBody(const SightResult& result, float dt, double now, const NpcPerceptionTuning& tuning);
	void SenseObject(const SightResult& result, double now, const NpcPerceptionTuning& tuning);
	void SenseCompany(const SightResult& result, float dt, double now, const NpcPerceptionTuning& tuning);
	void UpdateMissing(const Observer& observer, double now, const NpcPerceptionTuning& tuning);
	void SenseTouch(const Observer& observer, double now);
	void HearSounds(const Observer& observer, double now, const NpcPerceptionTuning& tuning);
	void DecayMeters(float dt, double now, const NpcPerceptionTuning& tuning);
	void Evaluate(NpcMeter& meter, double now, const NpcPerceptionTuning& tuning);
	void UpdateAlert(const vec3& where, float dt, double now, const NpcPerceptionTuning& tuning);
	void OnBodyIdentified(const NpcMeter& meter, double now);
	void Report(const std::string& squadmate, double now);

	// The subject's Person record is in the memory before this is called.
	void AddConcern(NpcConcernType type, const std::string& subject, double now);
	void Raise(NpcAlert level, double now);
	void Call(const NpcRecord& record, double now); // a shout, and on the radio when the NPC has one
	void Receive(const NpcMessage& message, double now);
	void Emit(NpcMindEventType type, const std::string& subject, const vec3& position);

	bool IsPlayer(const std::string& subject);

	// The state of the body of an NPC that is being looked at (what its owner says; false for something that is not one).
	static bool BodyInfoOf(const std::string& id, double now, NpcBodyInfo& info);
};
