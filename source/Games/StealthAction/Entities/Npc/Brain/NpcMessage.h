#pragma once

// How NPCs tell each other (NPC_AI_DESIGN.md §7) and how they ask the NpcDirector for a part in a situation (§10).
// An NPC never writes another NPC: it queues messages and requests in its outbox on its own thread, the NpcDirector
// collects them on the main thread and puts what is for the others into their inboxes.

#include "NpcMemory.h"

#include <string>
#include <vector>

enum class NpcMessageKind : int
{
	Shout,   // a call: carries as far as a sound, walls muffle it
	Report,  // face to face: only the ones standing close
	Radio,   // the squad: everyone of the sender's faction in reach, walls do not matter
	Alarm    // a bell: everybody in the area of its alarm point (§17.11)
};

inline const char* NpcMessageKindName(NpcMessageKind kind)
{
	switch (kind)
	{
	case NpcMessageKind::Shout: return "shout";
	case NpcMessageKind::Report: return "report";
	case NpcMessageKind::Radio: return "radio";
	default: return "alarm";
	}
}

// What a message carries is what the sender knew: records keep their own time, and the receiver never gets more.
struct NpcMessage
{
	NpcMessageKind kind = NpcMessageKind::Shout;
	std::string senderId;
	vec3 origin = vec3();          // where the sender stood when it spoke (for an alarm: where the bell is)
	double sentAt = 0.0;
	double deliverAt = 0.0;        // the director delivers it then, if the sender can still act
	float radius = 0.0f;           // alarm only: the area it reaches
	std::vector<NpcRecord> records;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(NpcMessage, kind, senderId, origin, sentAt, deliverAt, radius, records)
};

// The shared plans an NPC can be part of.
enum class SituationKind : int
{
	Investigate,   // a noise or a sighting to check: one investigator, the rest glance
	Search         // a hostile to find, or a body to look round: chase, then search (target lost, an alarm, a told hostile, a body)
};

inline const char* SituationKindName(SituationKind kind)
{
	return kind == SituationKind::Investigate ? "investigate" : "search";
}

// What the NPC does inside a situation. None: a member without a job (it glances and keeps its routine).
enum class NpcRole : int
{
	None,
	Investigator,
	Backup,
	Chaser,
	Checker,
	Searcher,
	Watcher,
	AlarmRunner    // runs to an alarm point (exclusive: one per search, §10)
};

inline const char* NpcRoleName(NpcRole role)
{
	switch (role)
	{
	case NpcRole::Investigator: return "investigator";
	case NpcRole::Backup: return "backup";
	case NpcRole::Chaser: return "chaser";
	case NpcRole::Checker: return "checker";
	case NpcRole::Searcher: return "searcher";
	case NpcRole::Watcher: return "watcher";
	case NpcRole::AlarmRunner: return "alarm runner";
	default: return "none";
	}
}

// What a role holder reports back to its situation.
enum class NpcProgress : int
{
	Resolved,       // the investigation is settled
	ChaseArrived,   // got where the chase led and looked round without finding anything
	Checked         // looked at the original spot (or tended the body)
};

// What an event leaves in its area (§6.4). Noise, curiosity and a trespass warning leave nothing.
enum class NpcAreaEvent : int
{
	Noise,
	BodyFound,
	Fight,
	Power,
	Alarm
};

enum class NpcRequestType : int
{
	Join,
	Leave,
	Progress,
	AreaEvent,    // something happened here: the area keeps a floor under its alert (`areaEvent`, `position`)
	RaiseAlarm,   // the runner rings the bell it stands at (`position`: the alarm point, `record`: what it knows of the intruder)
	WakeBody      // an unconscious NPC (`subject`) is shaken awake
};

struct NpcRequest
{
	NpcRequestType type = NpcRequestType::Join;
	SituationKind kind = SituationKind::Investigate;
	std::string subject;
	NpcProgress progress = NpcProgress::Resolved;
	NpcAreaEvent areaEvent = NpcAreaEvent::Noise;
	vec3 position = vec3();
	NpcRecord record;
};

// A situation the NPC was part of has ended: the concern about its subject is settled.
struct NpcEnded
{
	SituationKind kind = SituationKind::Investigate;
	std::string subject;
};

// A line the NPC would like to say. The director decides whether it is said (§18.2).
struct NpcBarkRequest
{
	std::string bark;
};

struct NpcOutbox
{
	std::vector<NpcMessage> messages;
	std::vector<NpcRequest> requests;
	std::vector<NpcBarkRequest> barks;
};

struct NpcInbox
{
	std::vector<NpcMessage> messages;
	std::vector<NpcEnded> ended;
};
