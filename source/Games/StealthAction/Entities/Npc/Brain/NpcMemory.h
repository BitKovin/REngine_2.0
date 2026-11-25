#pragma once

// What one NPC knows (NPC_AI_DESIGN.md §6.1). The single source of truth for knowledge: nothing else stores
// "where the target is". One record per (subject, kind); the newest `time` wins.

#include <Helpers/JsonHelper.hpp>
#include <Helpers/StringHelper.h> // the global `json` alias

#include <algorithm>
#include <string>
#include <vector>

enum class NpcRecordKind
{
	Person,   // someone seen or heard
	Body,
	Event,    // a noise, a shot
	Identity,
	Hazard,
	Object    // a door, an item: the state it was last seen in (§13.3)
};

enum class NpcRecordSource
{
	Saw,
	Heard,
	Told,
	Felt      // damage
};

struct NpcRecord
{
	std::string subject;                         // entity id, or an event id
	NpcRecordKind kind = NpcRecordKind::Person;
	vec3 position = vec3();
	vec3 velocity = vec3();
	double time = 0.0;                           // game time when it was true; never re-stamped when told on
	double lastSeen = -1.0e9;                    // game time somebody last saw the subject here (kept as-is when told on)
	NpcRecordSource source = NpcRecordSource::Saw;
	float certainty = 1.0f;                      // 0..1: seen 1, heard by loudness, told 0.8 of the teller's
	std::string detail;                          // concern type, apparent identity, body state and cause, object state ...
	float lifetime = 0.0f;                       // an event record is forgotten after this long; 0 = the default of the alert tuning

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(NpcRecord, subject, kind, position, velocity, time, lastSeen, source, certainty, detail, lifetime)
};

class NpcMemory
{
public:

	const std::vector<NpcRecord>& Records() const { return records; }

	// Stores a record unless a newer one of the same subject and kind exists; returns whether it was stored.
	bool Remember(const NpcRecord& record)
	{
		for (NpcRecord& existing : records)
		{
			if (existing.subject == record.subject && existing.kind == record.kind)
			{
				if (record.time < existing.time) return false;

				// When the subject was last seen only ever moves forward, whatever the newest news is made of.
				const double lastSeen = std::max(record.lastSeen, existing.lastSeen);
				existing = record;
				existing.lastSeen = lastSeen;
				return true;
			}
		}

		records.push_back(record);
		return true;
	}

	const NpcRecord* Find(const std::string& subject, NpcRecordKind kind) const
	{
		for (const NpcRecord& record : records)
		{
			if (record.subject == subject && record.kind == kind)
				return &record;
		}
		return nullptr;
	}

	// The newest record of a kind, or null.
	const NpcRecord* Freshest(NpcRecordKind kind) const
	{
		const NpcRecord* best = nullptr;
		for (const NpcRecord& record : records)
		{
			if (record.kind == kind && (best == nullptr || record.time > best->time))
				best = &record;
		}
		return best;
	}

	void Forget(const std::string& subject, NpcRecordKind kind)
	{
		records.erase(std::remove_if(records.begin(), records.end(),
			[&](const NpcRecord& r) { return r.subject == subject && r.kind == kind; }), records.end());
	}

	// The record that says where a subject is, whatever it is: a person, a body, an event, an object (what a concern points at).
	const NpcRecord* FindPlace(const std::string& subject) const
	{
		for (NpcRecordKind kind : { NpcRecordKind::Person, NpcRecordKind::Body, NpcRecordKind::Event, NpcRecordKind::Object })
		{
			if (const NpcRecord* record = Find(subject, kind))
				return record;
		}
		return nullptr;
	}

	// Event records are forgotten after their own lifetime, or `eventLifetime` seconds; the others stay until the situation ends.
	void ExpireEvents(double now, float eventLifetime)
	{
		records.erase(std::remove_if(records.begin(), records.end(),
			[&](const NpcRecord& r) { return r.kind == NpcRecordKind::Event && now - r.time > (r.lifetime > 0.0f ? r.lifetime : eventLifetime); }), records.end());
	}

	void Clear() { records.clear(); }

	// Saved as ages (seconds ago), so the records stay right whatever the game time is after a load.
	json Serialize(double now) const { return RecordsToJson(records, now); }
	void Deserialize(const json& data, double now) { records = RecordsFromJson(data, now); }

	static json RecordsToJson(const std::vector<NpcRecord>& list, double now)
	{
		json out = json::array();
		for (const NpcRecord& record : list)
		{
			json item = record;
			item["time"] = now - record.time;
			item["lastSeen"] = now - record.lastSeen;
			out.push_back(std::move(item));
		}
		return out;
	}

	static std::vector<NpcRecord> RecordsFromJson(const json& data, double now)
	{
		std::vector<NpcRecord> list;
		if (!data.is_array()) return list;

		for (const json& item : data)
		{
			NpcRecord record = item.get<NpcRecord>();
			record.time = now - record.time;
			record.lastSeen = now - record.lastSeen;
			list.push_back(std::move(record));
		}
		return list;
	}

private:

	std::vector<NpcRecord> records;
};
