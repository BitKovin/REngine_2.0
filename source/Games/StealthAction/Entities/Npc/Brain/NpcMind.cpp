#include "NpcMind.h"

#include <AiPerception/AiPerceptionSystem.h>
#include <Level.hpp>
#include <Physics.h>

#include "../Npc.h"
#include "../Director/NpcDirector.h"

#include <algorithm>

namespace
{
	const hashed_string TagBody("body");
	const hashed_string TagObject("object");

	float MotionFactor(TargetMotion motion, const NpcMeterTuning& tuning)
	{
		switch (motion)
		{
		case TargetMotion::Still: return tuning.motionStill;
		case TargetMotion::Run: return tuning.motionRun;
		case TargetMotion::CrouchWalk: return tuning.motionCrouchWalk;
		case TargetMotion::CrouchStill: return tuning.motionCrouchStill;
		default: return tuning.motionWalk;
		}
	}

	NpcAlert AlertFor(NpcConcernType type)
	{
		switch (type)
		{
		case NpcConcernType::Threat: return NpcAlert::Combat;
		case NpcConcernType::Alarm:
		case NpcConcernType::Body: return NpcAlert::Alarmed;
		default: return NpcAlert::Wary;
		}
	}
}

bool NpcMind::IsPlayer(const std::string& subject)
{
	const auto cached = playerCache.find(subject);
	if (cached != playerCache.end())
		return cached->second;

	const std::shared_ptr<ObservationTarget> target = AiPerceptionSystem::GetObserverFromEntityId(subject);
	if (target == nullptr)
		return false; // not known yet: ask again next time

	const bool isPlayer = target->HasTag("player");
	playerCache.emplace(subject, isPlayer);
	return isPlayer;
}

NpcMeter& NpcMind::MeterOf(const std::string& subject)
{
	for (NpcMeter& meter : meters)
	{
		if (meter.subject == subject)
			return meter;
	}

	meters.emplace_back();
	meters.back().subject = subject;
	return meters.back();
}

float NpcMind::MeterValue(const std::string& subject) const
{
	for (const NpcMeter& meter : meters)
	{
		if (meter.subject == subject)
			return meter.value;
	}
	return 0.0f;
}

float NpcMind::VisionRangeScale() const
{
	return NpcPerceptionTuning::Get().vision.rangeScale[static_cast<int>(alert)];
}

void NpcMind::Emit(NpcMindEventType type, const std::string& subject, const vec3& position)
{
	NpcMindEvent event;
	event.type = type;
	event.subject = subject;
	event.position = position;
	events.push_back(std::move(event));
}

void NpcMind::Raise(NpcAlert level, double now)
{
	if (level > alert)
		alert = level;

	lastConcernTime = now;
}

void NpcMind::AddConcern(NpcConcernType type, const std::string& subject, double now)
{
	Raise(AlertFor(type), now);

	for (NpcConcern& concern : concerns)
	{
		if (concern.subject != subject) continue;

		if (type > concern.type)
			concern.type = type; // never downgraded while it stands

		concern.time = std::max(concern.time, now);
		return;
	}

	NpcConcern concern;
	concern.subject = subject;
	concern.type = type;
	concern.time = now;
	concerns.push_back(std::move(concern));
}

const NpcConcern* NpcMind::TopConcern() const
{
	const NpcConcern* best = nullptr;

	for (const NpcConcern& concern : concerns)
	{
		if (best == nullptr || concern.type > best->type || (concern.type == best->type && concern.time > best->time))
			best = &concern;
	}

	return best;
}

vec3 NpcMind::ConcernPosition(const NpcConcern& concern) const
{
	const NpcRecord* record = memory.FindPlace(concern.subject);
	return record ? record->position : vec3();
}

void NpcMind::Bark(const char* bark)
{
	for (const NpcBarkRequest& request : outbox.barks)
	{
		if (request.bark == bark) return;
	}

	NpcBarkRequest request;
	request.bark = bark;
	outbox.barks.push_back(std::move(request));
}

void NpcMind::RequestAreaEvent(NpcAreaEvent type, const vec3& position, double now)
{
	// The area is told once in a while about the same kind of thing, not on every update of it.
	const auto last = lastAreaEvent.find(static_cast<int>(type));
	if (last != lastAreaEvent.end() && now - last->second < NpcSituationTuning::Get().messages.shoutCooldown) return;
	lastAreaEvent[static_cast<int>(type)] = now;

	NpcRequest request;
	request.type = NpcRequestType::AreaEvent;
	request.areaEvent = type;
	request.position = position;
	outbox.requests.push_back(std::move(request));
}

bool NpcMind::BodyInfoOf(const std::string& id, double now, NpcBodyInfo& info)
{
	if (Level::Current == nullptr) return false;

	Entity* entity = Level::Current->FindEntityWithId(id);
	if (entity == nullptr) return false;

	if (const Npc* npc = dynamic_cast<const Npc*>(entity))
	{
		info = npc->GetBodyInfo();
		return info.state != NpcBodyState::Alive;
	}

	// The older NPC classes only have dead ones, and no notion of when or how.
	info = NpcBodyInfo();
	info.since = now;
	return true;
}

bool NpcMind::Sees(const std::string& subject) const
{
	for (const NpcMeter& meter : meters)
	{
		if (meter.subject == subject)
			return meter.sensed && meter.source == NpcRecordSource::Saw;
	}
	return false;
}

double NpcMind::LastSeen(const std::string& subject) const
{
	const NpcRecord* record = memory.Find(subject, NpcRecordKind::Person);
	return record ? record->lastSeen : -1.0e9;
}

// A call: the part of what the sender knew that is about somebody hostile or about a body, to the others as a record they
// have only been told (certainty falls, the time stays). An NPC with a radio tells its squad too (7).
void NpcMind::Call(const NpcRecord& record, double now)
{
	const float cooldown = NpcSituationTuning::Get().messages.shoutCooldown;

	const auto last = lastShout.find(record.subject);
	if (last != lastShout.end() && now - last->second < cooldown) return;
	lastShout[record.subject] = now;

	NpcMessage message;
	message.kind = NpcMessageKind::Shout;
	message.records.push_back(record);

	if (profile->radio)
	{
		NpcMessage radio = message;
		radio.kind = NpcMessageKind::Radio;
		outbox.messages.push_back(std::move(radio));
	}

	outbox.messages.push_back(std::move(message));
}

// What a squadmate it meets is told (7): the identified hostiles and the bodies it knows of, at most once in a while.
void NpcMind::Report(const std::string& squadmate, double now)
{
	const NpcMessageTuning& messages = NpcSituationTuning::Get().messages;

	const auto last = lastReport.find(squadmate);
	if (last != lastReport.end() && now - last->second < messages.reportCooldown) return;

	NpcMessage message;
	message.kind = NpcMessageKind::Report;

	for (const NpcRecord& record : memory.Records())
	{
		const bool news = (record.kind == NpcRecordKind::Person && record.detail == "hostile")
			|| (record.kind == NpcRecordKind::Body && NpcBodyInfo::StateOfDetail(record.detail) == NpcBodyState::Dead);

		if (news && now - record.time <= NpcPerceptionTuning::Get().alert.leadMaxAge)
			message.records.push_back(record);
	}

	if (message.records.empty()) return;

	lastReport[squadmate] = now;
	outbox.messages.push_back(std::move(message));
}

void NpcMind::Receive(const NpcMessage& message, double now)
{
	const float toldCertainty = NpcSituationTuning::Get().messages.toldCertainty;

	for (NpcRecord record : message.records)
	{
		// Only what is newer than what it knows is news: the same thing told twice changes nothing.
		const NpcRecord* have = memory.Find(record.subject, record.kind);
		if (have != nullptr && record.time <= have->time) continue;

		record.source = NpcRecordSource::Told;
		record.certainty *= toldCertainty;

		memory.Remember(record);

		Emit(NpcMindEventType::Heard, record.subject, record.position);

		// Somebody hostile is around, and the NPC has not seen it: an alarm to follow up.
		if (record.kind == NpcRecordKind::Person && record.detail == "hostile")
		{
			AddConcern(NpcConcernType::Alarm, record.subject, now);
			Bark("heard_shot");
		}

		// A body somebody found: it is looked into, unless the NPC has a fight on its hands (noted only).
		if (record.kind == NpcRecordKind::Body && alert < NpcAlert::Combat && NpcBodyInfo::StateOfDetail(record.detail) == NpcBodyState::Dead)
			AddConcern(NpcConcernType::Body, record.subject, now);
	}
}

void NpcMind::ProcessInbox(double now)
{
	std::vector<NpcMessage> messages;
	std::vector<NpcEnded> ended;
	messages.swap(inbox.messages);
	ended.swap(inbox.ended);

	for (const NpcMessage& message : messages)
		Receive(message, now);

	for (const NpcEnded& end : ended)
		Resolve(end.subject);

	if (ended.empty() == false && concerns.empty())
	{
		FinishSearch(now);
		Bark("search_end");
	}
}

void NpcMind::Resolve(const std::string& subject)
{
	concerns.erase(std::remove_if(concerns.begin(), concerns.end(),
		[&](const NpcConcern& concern) { return concern.subject == subject; }), concerns.end());
}

void NpcMind::FinishSearch(double now)
{
	if (alert == NpcAlert::Alarmed)
		alert = NpcAlert::Wary;

	lastConcernTime = now;
}

void NpcMind::Update(const Observer& observer, float dt, double now)
{
	const NpcPerceptionTuning& tuning = NpcPerceptionTuning::Get();

	hostileSensed = false;

	for (NpcMeter& meter : meters)
	{
		meter.previous = meter.value;
		meter.stimulated = false;
		meter.sensed = false;
		meter.curiousSound = false;
	}

	SenseSight(observer, dt, now, tuning);
	SenseTouch(observer, now);
	HearSounds(observer, now, tuning);
	DecayMeters(dt, now, tuning);

	for (NpcMeter& meter : meters)
		Evaluate(meter, now, tuning);

	meters.erase(std::remove_if(meters.begin(), meters.end(),
		[](const NpcMeter& meter) { return meter.value <= 0.0f && meter.stimulated == false; }), meters.end());

	UpdateMissing(observer, now, tuning);
	UpdateAlert(observer.position, dt, now, tuning);
}

void NpcMind::SenseSight(const Observer& observer, float dt, double now, const NpcPerceptionTuning& tuning)
{
	const NpcMeterTuning& meterTuning = tuning.meter;

	for (const SightResult& result : observer.sightResults)
	{
		if (result.sense != SenseKind::Sight || result.target == nullptr) continue;

		const ObservationTarget& target = *result.target;

		if (target.HasTag(TagBody)) { SenseBody(result, dt, now, tuning); continue; }
		if (target.HasTag(TagObject)) { SenseObject(result, now, tuning); continue; }
		if (target.npc) { SenseCompany(result, dt, now, tuning); continue; }

		const std::string& subject = result.target->ownerId;
		if (!IsPlayer(subject)) continue;

		// 0.6 x strength x motion x state x acuity x suspiciousness (§6.2).
		const float fill = meterTuning.fillRate * result.strength * MotionFactor(result.target->motion, meterTuning)
			* meterTuning.stateMul[static_cast<int>(alert)] * profile->acuity * profile->defaultSuspicion;

		NpcMeter& meter = MeterOf(subject);
		meter.value = std::min(1.0f, meter.value + fill * dt);
		meter.lastStimulus = now;
		meter.lastSight = now;
		meter.position = result.point;
		meter.source = NpcRecordSource::Saw;
		meter.stimulated = true;
		meter.sensed = true;
	}
}

// Somebody bumped into the NPC: it knows what it is at once.
void NpcMind::SenseTouch(const Observer& observer, double now)
{
	for (const SightResult& result : observer.touches)
	{
		if (result.target == nullptr) continue;

		const std::string& subject = result.target->ownerId;
		if (!IsPlayer(subject)) continue;

		NpcMeter& meter = MeterOf(subject);
		meter.value = 1.0f;
		meter.lastStimulus = now;
		meter.lastSight = now;
		meter.position = result.point;
		meter.source = NpcRecordSource::Felt;
		meter.stimulated = true;
		meter.sensed = true;
	}
}

void NpcMind::HearSounds(const Observer& observer, double now, const NpcPerceptionTuning& tuning)
{
	const NpcMeterTuning& meterTuning = tuning.meter;

	for (const HeardSound& heard : observer.heardSounds)
	{
		const std::string name = NpcSounds::Name(static_cast<NpcSoundType>(heard.severity));
		const NpcSoundTuning& sound = tuning.SoundFor(name);

		const std::string subject = heard.causerId.empty() ? name : heard.causerId;
		const vec3 where = heard.perceivedPosition;

		NpcRecord record;
		record.subject = "sound:" + name + ":" + subject;
		record.kind = NpcRecordKind::Event;
		record.position = where;
		record.time = now;
		record.source = NpcRecordSource::Heard;
		record.certainty = heard.loudnessAtListener;
		record.detail = name;
		memory.Remember(record);

		Emit(NpcMindEventType::Heard, subject, where);

		if (sound.concern == "alarm")
		{
			// Where the one who made it is now, as far as the NPC can tell.
			NpcRecord person;
			person.subject = subject;
			person.kind = NpcRecordKind::Person;
			person.position = where;
			person.time = now;
			person.source = NpcRecordSource::Heard;
			person.certainty = heard.loudnessAtListener;
			person.detail = name;
			memory.Remember(person);

			AddConcern(NpcConcernType::Alarm, subject, now);
			Bark(heard.severity == static_cast<int>(NpcSoundType::Gunshot) ? "heard_shot" : "heard_noise");

			if (heard.loudnessAtListener >= 0.5f)
				fear = std::min(1.0f, fear + tuning.fear.gunfireNear);

			continue;
		}

		// A curious sound adds to the awareness of whoever made it; sound alone never reaches Identify.
		NpcMeter& meter = MeterOf(subject);
		const float cap = meter.lastSight > now - meterTuning.hold ? 1.0f : meterTuning.identify - 0.05f;
		const float fill = heard.loudnessAtListener * meterTuning.soundFill * sound.suspicion * profile->acuity;

		meter.value = std::max(meter.value, std::min(cap, meter.value + fill));
		meter.lastStimulus = now;
		meter.stimulated = true;

		// A sound of the curious kind that is loud enough to be noticed is worth looking into (§12), not only one that
		// takes the awareness all the way to Suspect.
		if (meter.value >= meterTuning.notice)
		{
			meter.curiousSound = true;
			Bark("heard_noise");
		}

		if (meter.sensed == false)
		{
			meter.position = where;
			meter.source = NpcRecordSource::Heard;
		}
	}
}

void NpcMind::DecayMeters(float dt, double now, const NpcPerceptionTuning& tuning)
{
	const NpcMeterTuning& meterTuning = tuning.meter;
	const float rate = alert >= NpcAlert::Alarmed ? meterTuning.decayAlerted : meterTuning.decay;

	for (NpcMeter& meter : meters)
	{
		if (meter.stimulated) continue;
		if (now - meter.lastStimulus < meterTuning.hold) continue;

		meter.value = std::max(0.0f, meter.value - rate * dt);
	}
}

// Thresholds crossed, the record kept up to date, concerns raised (§6.2, §6.6).
void NpcMind::Evaluate(NpcMeter& meter, double now, const NpcPerceptionTuning& tuning)
{
	const NpcMeterTuning& meterTuning = tuning.meter;

	const bool identified = meter.value >= meterTuning.identify && now - meter.lastSight < meterTuning.hold;

	if (meter.previous < meterTuning.notice && meter.value >= meterTuning.notice)
		Emit(NpcMindEventType::Noticed, meter.subject, meter.position);

	if (meter.previous < meterTuning.suspect && meter.value >= meterTuning.suspect)
		Emit(NpcMindEventType::Suspected, meter.subject, meter.position);

	if (meter.previous < meterTuning.identify && identified)
		Emit(NpcMindEventType::Identified, meter.subject, meter.position);

	// A body is made out, not suspected: what it means is decided when the awareness is full.
	if (meter.kind == NpcRecordKind::Body)
	{
		if (meter.previous < meterTuning.identify && identified && meter.sensed)
			OnBodyIdentified(meter, now);
		return;
	}

	if (meter.previous < meterTuning.notice && meter.value >= meterTuning.notice)
		Bark("notice");
	if (meter.previous < meterTuning.suspect && meter.value >= meterTuning.suspect)
		Bark("suspect");
	if (meter.previous < meterTuning.identify && identified)
	{
		Bark("identify_intruder");
		RequestAreaEvent(NpcAreaEvent::Fight, meter.position, now); // somebody who is not welcome is a fight (6.4)
	}

	if (meter.stimulated == false || meter.value < meterTuning.notice)
		return;

	// What it knows about where this subject is.
	NpcRecord record;
	record.subject = meter.subject;
	record.kind = NpcRecordKind::Person;
	record.position = meter.position;
	record.time = now;
	record.source = meter.source;
	record.certainty = meter.source == NpcRecordSource::Heard ? std::min(1.0f, meter.value) : 1.0f;
	record.detail = identified ? "hostile" : "suspect";
	record.lastSeen = meter.sensed ? now : -1.0e9; // the memory keeps the newest

	if (const NpcRecord* old = memory.Find(meter.subject, NpcRecordKind::Person))
	{
		const double elapsed = now - old->time;
		if (elapsed > 0.05 && elapsed < 1.0)
			record.velocity = (record.position - old->position) / static_cast<float>(elapsed);
	}

	memory.Remember(record);

	if (meter.value >= meterTuning.suspect && (meter.sensed || meter.previous < meterTuning.suspect))
		AddConcern(identified ? NpcConcernType::Threat : NpcConcernType::Curious, meter.subject, now);
	else if (meter.curiousSound)
		AddConcern(NpcConcernType::Curious, meter.subject, now);

	// Seeing the hostile for sure: call it out so the others know (a record they are told, not a look at the truth).
	if (meter.previous < meterTuning.identify && identified)
	{
		if (const NpcRecord* seen = memory.Find(meter.subject, NpcRecordKind::Person))
			Call(*seen, now);
	}

	if (identified && meter.sensed)
	{
		hostileSensed = true;
		lastHostileTime = now;
	}
}

void NpcMind::UpdateAlert(const vec3& where, float dt, double now, const NpcPerceptionTuning& tuning)
{
	const NpcAlertTuning& alertTuning = tuning.alert;

	memory.ExpireEvents(now, alertTuning.eventLifetime);

	// A concern is dropped when it is old, or when what it pointed at has been forgotten (an event that has run out).
	concerns.erase(std::remove_if(concerns.begin(), concerns.end(),
		[&](const NpcConcern& concern) { return now - concern.time > alertTuning.leadMaxAge || memory.FindPlace(concern.subject) == nullptr; }), concerns.end());

	if (alert == NpcAlert::Combat && now - lastHostileTime > alertTuning.combatToAlarmed)
		alert = NpcAlert::Alarmed;

	// Normally the search ends an alarm (FinishSearch); this is the net under it.
	if (concerns.empty() && now - lastConcernTime > alertTuning.wearyToCalm)
	{
		if (alert == NpcAlert::Alarmed)
		{
			alert = NpcAlert::Wary;
			lastConcernTime = now;
		}
		else if (alert == NpcAlert::Wary)
		{
			alert = NpcAlert::Calm;
		}
	}

	// What happened in this area keeps it from calming down below its floor (6.4).
	if (const NpcDirector* director = NpcDirector::Get())
		alert = std::max(alert, director->FloorAt(where, now));

	if (hostileSensed == false && alert < NpcAlert::Alarmed)
		fear = std::max(0.0f, fear - tuning.fear.safetyDecay * dt);
}

// ── Bodies ──────────────────────────────────────────────────────────────────────────────────────────────────────

// A body in sight: the awareness of it fills like any other (a body is what it is at once, 6.3), but a body in the dark is
// only made out from close by. What was found already is not found again.
void NpcMind::SenseBody(const SightResult& result, float dt, double now, const NpcPerceptionTuning& tuning)
{
	const NpcBodiesTuning& bodies = tuning.bodies;
	const std::string& subject = result.target->ownerId;

	NpcBodyInfo info;
	if (BodyInfoOf(subject, now, info) == false || info.hidden) return;

	if (result.target->lightLevel < bodies.darkLevel && result.distance > bodies.darkSeenDistance) return;

	// Known already, in this state: only where it lies and when it was seen changes (a moved body is found where it is now).
	if (const NpcRecord* known = memory.Find(subject, NpcRecordKind::Body))
	{
		if (known->detail == info.Detail() || (info.state == NpcBodyState::Dead && NpcBodyInfo::StateOfDetail(known->detail) == NpcBodyState::Dead))
		{
			NpcRecord seen = *known;
			seen.position = result.target->position;
			seen.lastSeen = now;
			memory.Remember(seen);
			return;
		}
	}

	const NpcMeterTuning& meterTuning = tuning.meter;
	const float fill = meterTuning.fillRate * result.strength * bodies.fillMotion
		* meterTuning.stateMul[static_cast<int>(alert)] * profile->acuity * bodies.suspicion;

	NpcMeter& meter = MeterOf(subject);
	meter.kind = NpcRecordKind::Body;
	meter.value = std::min(1.0f, meter.value + fill * dt);
	meter.lastStimulus = now;
	meter.lastSight = now;
	meter.position = result.target->position;
	meter.source = NpcRecordSource::Saw;
	meter.stimulated = true;
	meter.sensed = true;
}

// The awareness of a body is full: it is found. The record says what it is, since when and by what; what follows depends on
// where the NPC is (in a fight a body is only noted).
void NpcMind::OnBodyIdentified(const NpcMeter& meter, double now)
{
	NpcBodyInfo info;
	if (BodyInfoOf(meter.subject, now, info) == false) return;

	NpcRecord record;
	record.subject = meter.subject;
	record.kind = NpcRecordKind::Body;
	record.position = meter.position;
	record.time = info.since;
	record.lastSeen = now;
	record.source = NpcRecordSource::Saw;
	record.certainty = 1.0f;
	record.detail = info.Detail();
	memory.Remember(record);

	if (alert == NpcAlert::Combat) return;

	const NpcFearTuning& fearTuning = NpcPerceptionTuning::Get().fear;
	fear = std::min(1.0f, fear + (info.Drained() ? fearTuning.bodyDrained : fearTuning.bodySeen));

	AddConcern(NpcConcernType::Body, meter.subject, now);

	// Dead: a shout. Out cold: nobody else needs to hear, a guard goes to wake it (13.1).
	if (info.state == NpcBodyState::Dead)
	{
		Call(record, now);
		Bark(info.Drained() ? "body_drained" : "body_dead");
	}
}

// Out cold and woken: the NPC knows where it went down, and the search round that place is its business too.
void NpcMind::OnWoken(const vec3& where, double now)
{
	NpcBodyInfo info;
	info.state = NpcBodyState::Alive;

	NpcRecord record;
	record.subject = selfId;
	record.kind = NpcRecordKind::Body;
	record.position = where;
	record.time = now;
	record.lastSeen = now;
	record.source = NpcRecordSource::Felt;
	record.certainty = 1.0f;
	record.detail = info.Detail();
	memory.Remember(record);

	AddConcern(NpcConcernType::Alarm, selfId, now);

	// What it saw before it went down is still what it knows: the one it was fighting is still the one to look for.
	if (const NpcRecord* hostile = memory.Freshest(NpcRecordKind::Person))
	{
		if (hostile->detail == "hostile")
			AddConcern(NpcConcernType::Alarm, hostile->subject, now);
	}

	Bark("wake_up");
}

// ── Objects ─────────────────────────────────────────────────────────────────────────────────────────────────────

// A door or an item in view: its state is remembered, and a state that is not the one it saw last is worth a look (13.3).
void NpcMind::SenseObject(const SightResult& result, double now, const NpcPerceptionTuning& tuning)
{
	const ObservationTarget& target = *result.target;

	std::string state;
	for (const hashed_string& tag : target.tags)
	{
		if (tag.str().rfind("state:", 0) == 0)
			state = tag.str().substr(6);
	}

	if (state.empty()) return;

	const NpcRecord* old = memory.Find(target.ownerId, NpcRecordKind::Object);
	const bool changed = old != nullptr && old->detail != state;

	NpcRecord record;
	record.subject = target.ownerId;
	record.kind = NpcRecordKind::Object;
	record.position = target.SightPoint();
	record.time = now;
	record.lastSeen = now;
	record.source = NpcRecordSource::Saw;
	record.detail = state;
	memory.Remember(record);

	if (changed == false) return;

	// The first thing it saw of the place is how it is; a change is an event of its own that lasts a while.
	NpcRecord event;
	event.subject = "changed:" + target.ownerId;
	event.kind = NpcRecordKind::Event;
	event.position = record.position;
	event.time = now;
	event.lastSeen = now;
	event.source = NpcRecordSource::Saw;
	event.detail = "changed:" + state;
	event.lifetime = tuning.objects.changedLifetime;
	memory.Remember(event);

	Emit(NpcMindEventType::Noticed, event.subject, event.position);
	AddConcern(NpcConcernType::Curious, event.subject, now);
}

// ── Company ─────────────────────────────────────────────────────────────────────────────────────────────────────

// Another NPC in view: it may be somebody to tell what the NPC knows (7), and, when it is of the NPC's squad, somebody whose
// being around is noticed (13.3).
void NpcMind::SenseCompany(const SightResult& result, float dt, double now, const NpcPerceptionTuning& tuning)
{
	const ObservationTarget& target = *result.target;
	if (target.HasTag(profile->faction) == false) return;

	if (profile->reports && result.distance <= NpcSituationTuning::Get().messages.report.radius)
		Report(target.ownerId, now);

	if (squad.empty() || target.HasTag(squadTag) == false || result.distance > tuning.missing.earshot) return;

	Companion* companion = nullptr;
	for (Companion& candidate : companions)
	{
		if (candidate.id == target.ownerId)
			companion = &candidate;
	}

	if (companion == nullptr)
	{
		companions.emplace_back();
		companion = &companions.back();
		companion->id = target.ownerId;
	}

	companion->company = std::min(companion->company + dt, tuning.missing.companyTime * 2.0f);
	companion->lastSensed = now;
	companion->position = target.position;
}

// A squadmate that was around for a while and has gone quiet, while the NPC looks at the place it was: where did it go? (13.3)
void NpcMind::UpdateMissing(const Observer& observer, double now, const NpcPerceptionTuning& tuning)
{
	const NpcMissingTuning& missing = tuning.missing;

	for (Companion& companion : companions)
	{
		if (companion.company < missing.companyTime || now - companion.lastSensed < missing.silentTime || now < companion.nextAllowed) continue;

		// Not where it was a moment ago, and the NPC can tell: the place is in front of it, within reach of its eyes.
		const vec3 place = companion.position;
		if (glm::length(MathHelper::XZ(observer.position - place)) > missing.areaRadius) continue;
		if (observer.ZoneAt(place) == VisionZone::None) continue;
		if (Physics::LineTrace(observer.position, place, BodyType::WorldOpaque).hasHit) continue;

		companion.company = 0.0f;
		companion.nextAllowed = now + missing.cooldown;

		// A body of it that the NPC knows about answers the question.
		if (memory.Find(companion.id, NpcRecordKind::Body)) continue;

		NpcRecord record;
		record.subject = companion.id;
		record.kind = NpcRecordKind::Person;
		record.position = place;
		record.time = companion.lastSensed;
		record.lastSeen = companion.lastSensed;
		record.source = NpcRecordSource::Saw;
		record.certainty = 0.6f;
		record.detail = "missing";
		memory.Remember(record);

		AddConcern(NpcConcernType::Curious, companion.id, now);
		Bark("missing_ally");
	}

	// The ones that are long gone and were dealt with are forgotten.
	companions.erase(std::remove_if(companions.begin(), companions.end(),
		[&](const Companion& companion) { return companion.company <= 0.0f && now - companion.lastSensed > missing.cooldown * 2.0f; }), companions.end());
}

// ── Damage ──────────────────────────────────────────────────────────────────────────────────────────────────────

void NpcMind::OnDamaged(float amount, const std::string& attackerId, const vec3& origin, double now)
{
	const NpcPerceptionTuning& tuning = NpcPerceptionTuning::Get();

	if (amount >= tuning.fear.heavyDamageThreshold)
		fear = std::min(1.0f, fear + tuning.fear.heavyDamage);

	Emit(NpcMindEventType::Hurt, attackerId, origin);

	// Hit by someone it is looking at: that one is the hostile. Otherwise a shot from over there.
	NpcMeter* meter = nullptr;
	for (NpcMeter& candidate : meters)
	{
		if (candidate.subject == attackerId)
			meter = &candidate;
	}

	const bool seen = meter != nullptr && now - meter->lastSight < 1.0 && IsPlayer(attackerId);
	const std::string subject = attackerId.empty() ? std::string("attacker") : attackerId;

	NpcRecord record;
	record.subject = subject;
	record.kind = NpcRecordKind::Person;
	record.position = origin;
	record.time = now;
	record.source = NpcRecordSource::Felt;
	record.detail = seen ? "hostile" : "shooter";
	record.lastSeen = seen ? now : -1.0e9;
	memory.Remember(record);

	if (seen)
	{
		meter->value = 1.0f;
		meter->lastStimulus = now;
		lastHostileTime = now;
	}

	AddConcern(seen ? NpcConcernType::Threat : NpcConcernType::Alarm, subject, now);
	RequestAreaEvent(NpcAreaEvent::Fight, origin, now);
}

json NpcMind::Serialize(double now) const
{
	json data;

	data["alert"] = alert;
	data["fear"] = fear;
	data["memory"] = memory.Serialize(now);
	data["lastHostileAge"] = now - lastHostileTime;
	data["lastConcernAge"] = now - lastConcernTime;

	json meterList = json::array();
	for (const NpcMeter& meter : meters)
	{
		json item = meter;
		item["lastStimulus"] = now - meter.lastStimulus;
		item["lastSight"] = now - meter.lastSight;
		meterList.push_back(std::move(item));
	}
	data["meters"] = std::move(meterList);

	json concernList = json::array();
	for (const NpcConcern& concern : concerns)
	{
		json item = concern;
		item["time"] = now - concern.time;
		concernList.push_back(std::move(item));
	}
	data["concerns"] = std::move(concernList);

	return data;
}

void NpcMind::Deserialize(const json& data, double now)
{
	meters.clear();
	concerns.clear();
	events.clear();
	playerCache.clear();
	lastShout.clear();
	lastReport.clear();
	lastAreaEvent.clear();
	companions.clear();
	outbox = NpcOutbox();
	inbox = NpcInbox();

	alert = data.value("alert", NpcAlert::Calm);
	fear = data.value("fear", 0.0f);
	memory.Deserialize(data.value("memory", json::array()), now);
	lastHostileTime = now - data.value("lastHostileAge", 1.0e9);
	lastConcernTime = now - data.value("lastConcernAge", 1.0e9);

	if (data.contains("meters"))
	{
		for (const json& item : data["meters"])
		{
			NpcMeter meter = item.get<NpcMeter>();
			meter.lastStimulus = now - meter.lastStimulus;
			meter.lastSight = now - meter.lastSight;
			meter.previous = meter.value;
			meters.push_back(std::move(meter));
		}
	}

	if (data.contains("concerns"))
	{
		for (const json& item : data["concerns"])
		{
			NpcConcern concern = item.get<NpcConcern>();
			concern.time = now - concern.time;

			// A concern needs the record it points at; one saved without it (an older save) is dropped.
			if (memory.FindPlace(concern.subject))
				concerns.push_back(std::move(concern));
		}
	}
}
