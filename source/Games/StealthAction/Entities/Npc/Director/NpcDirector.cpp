#include "NpcDirector.h"

#include "../AlarmPoint.h"
#include "../Npc.h"

#include <AiPerception/AiPerceptionSystem.h>
#include <Level.hpp>
#include <LevelObjectFactory.h>
#include <MathHelper.hpp>
#include <Navigation/NavGrid.h>
#include <Time.hpp>

#include <imgui/imgui.h>

#include <tracy/tracy/Tracy.hpp>

#include <algorithm>
#include <cmath>
#include <functional>

namespace
{
	float Flat(const glm::vec3& a, const glm::vec3& b)
	{
		return glm::length(MathHelper::XZ(a - b));
	}

	// Where the area is told the intruder is: a fixed offset from the truth (the same intruder and the same alarm always give
	// the same answer), between the minimum and the radius of the data, on the navigation grid. Not the exact place.
	glm::vec3 RoughPosition(const glm::vec3& truth, const std::string& subject, int alarmNumber, const NpcAlarmTuning& tuning)
	{
		const size_t hash = std::hash<std::string>()(subject) ^ (static_cast<size_t>(alarmNumber) * 0x9E3779B97F4A7C15ull);
		const float first = static_cast<float>((hash >> 8) & 0xFFFF) / 65535.0f;
		const float second = static_cast<float>((hash >> 32) & 0xFFFF) / 65535.0f;

		const float distance = tuning.roughRadius * (tuning.roughMinFraction + (1.0f - tuning.roughMinFraction) * first);
		const float angle = second * glm::two_pi<float>();

		// A direction that leads somewhere walkable: turn on in sixths of a circle until the grid has a cell there.
		for (int turn = 0; turn < 6; ++turn)
		{
			const float a = angle + static_cast<float>(turn) * glm::pi<float>() / 3.0f;
			const glm::vec3 point = truth + glm::vec3(std::cos(a), 0.0f, std::sin(a)) * distance;

			if (NavGrid::IsReady() == false) return point;

			const int cell = NavGrid::FindCell(point, 2.0f);
			if (cell >= 0)
				return NavGrid::GetCell(cell).floor;
		}

		return truth;
	}

	const NpcFloorRule& RuleFor(const NpcFloorsTuning& tuning, NpcAreaEvent type)
	{
		switch (type)
		{
		case NpcAreaEvent::BodyFound: return tuning.bodyFound;
		case NpcAreaEvent::Fight: return tuning.fight;
		case NpcAreaEvent::Power: return tuning.power;
		case NpcAreaEvent::Alarm: return tuning.alarm;
		default: return tuning.noise;
		}
	}
}

NpcDirector::NpcDirector()
{
	ClassName = "npc_director";
	SaveGame = true;
}

NpcDirector::~NpcDirector()
{
	if (instance == this)
		instance = nullptr;
}

NpcDirector& NpcDirector::Ensure()
{
	if (instance != nullptr && Level::Current != nullptr && Level::Current->FindEntityWithId(instance->Id) == instance)
		return *instance;

	NpcDirector* director = static_cast<NpcDirector*>(LevelObjectFactory::instance().create("npc_director"));
	director->Name = "npcDirector";
	Level::Current->AddEntity(director);
	director->Start();
	return *director;
}

void NpcDirector::Start()
{
	Entity::Start();
	instance = this;
}

void NpcDirector::LateUpdate()
{
	ZoneScopedN("NpcDirector::Step");
	Step(Time::GameTime, Time::DeltaTimeF);
}

// ── NPCs ────────────────────────────────────────────────────────────────────────────────────────────────────────

void NpcDirector::Register(Npc* npc)
{
	if (std::find(npcs.begin(), npcs.end(), npc) == npcs.end())
		npcs.push_back(npc);
}

void NpcDirector::Unregister(Npc* npc)
{
	if (instance == nullptr) return;

	std::vector<Npc*>& list = instance->npcs;
	list.erase(std::remove(list.begin(), list.end(), npc), list.end());
}

Npc* NpcDirector::FindActing(const std::string& npcId) const
{
	for (Npc* npc : npcs)
	{
		if (npc->Id == npcId)
			return npc->CanAct() ? npc : nullptr;
	}
	return nullptr;
}

// ── Reading ─────────────────────────────────────────────────────────────────────────────────────────────────────

bool NpcDirector::FindMembership(const std::string& npcId, SituationKind kind, Membership& membership) const
{
	for (const auto& situation : situations)
	{
		if (situation->Kind() != kind || situation->FindMember(npcId) == nullptr) continue;

		membership.situation = situation.get();
		membership.order = situation->OrderOf(npcId);
		return true;
	}
	return false;
}

bool NpcDirector::FindOrder(const std::string& npcId, SituationKind kind, NpcOrder& order, std::string& subject)
{
	Membership membership;
	if (instance == nullptr || instance->FindMembership(npcId, kind, membership) == false) return false;

	order = membership.order;
	subject = membership.situation->Subject();
	return true;
}

void NpcDirector::MembershipsOf(const std::string& npcId, std::vector<std::pair<SituationKind, std::string>>& out) const
{
	for (const auto& situation : situations)
	{
		if (situation->FindMember(npcId))
			out.emplace_back(situation->Kind(), situation->Subject());
	}
}

bool NpcDirector::IsMember(const std::string& npcId, SituationKind kind, const std::string& subject) const
{
	const NpcSituation* situation = FindSituation(kind, subject);
	return situation != nullptr && situation->FindMember(npcId) != nullptr;
}

const NpcSituation* NpcDirector::FindSituation(SituationKind kind, const std::string& subject) const
{
	for (const auto& situation : situations)
	{
		if (situation->Kind() == kind && situation->Subject() == subject)
			return situation.get();
	}
	return nullptr;
}

NpcSituation* NpcDirector::FindSituationMutable(SituationKind kind, const std::string& subject)
{
	return const_cast<NpcSituation*>(FindSituation(kind, subject));
}

const NpcRecord* NpcDirector::Freshest(const NpcSituation& situation) const
{
	const NpcRecord* best = nullptr;

	for (const SituationMember& member : situation.Members())
	{
		Npc* npc = FindActing(member.npcId);
		if (npc == nullptr) continue;

		// A person, or the body that is what the situation is round: whichever the member knows of, the newest.
		const NpcMemory& memory = npc->GetMind().memory;
		const NpcRecord* record = memory.Find(situation.Subject(), NpcRecordKind::Person);
		const NpcRecord* body = memory.Find(situation.Subject(), NpcRecordKind::Body);
		if (body != nullptr && (record == nullptr || body->time > record->time))
			record = body;

		if (record != nullptr && (best == nullptr || record->time > best->time))
			best = record;
	}

	return best;
}

// ── The step ────────────────────────────────────────────────────────────────────────────────────────────────────

void NpcDirector::Step(double now, float dt)
{
	CollectFromNpcs(now);
	DeliverMessages(now);
	StepSituations(now, dt);
	ArbitrateBarks(now);

	// A floor that has run out and leaves nothing behind is gone.
	floors.erase(std::remove_if(floors.begin(), floors.end(),
		[&](const AreaFloor& floor) { return floor.until >= 0.0 && now >= floor.until && floor.then == NpcAlert::Calm; }), floors.end());
}

NpcSituation& NpcDirector::OpenSituation(SituationKind kind, const std::string& subject)
{
	if (kind == SituationKind::Investigate)
		situations.push_back(std::make_unique<InvestigateSituation>(subject, nextSituationId++));
	else
		situations.push_back(std::make_unique<SearchSituation>(subject, nextSituationId++));

	return *situations.back();
}

// What the NPCs queued on their own threads: messages wait for their time, requests are handled at once.
void NpcDirector::CollectFromNpcs(double now)
{
	const NpcMessageTuning& tuning = NpcSituationTuning::Get().messages;

	// Copied: a request can change who is registered (nothing does yet, but the loop must not care).
	const std::vector<Npc*> list = npcs;

	for (Npc* npc : list)
	{
		NpcOutbox& outbox = npc->mind.outbox;

		// Off means off: whoever cannot act sends nothing, not even what it queued just before.
		if (npc->CanAct() == false)
		{
			outbox = NpcOutbox();
			continue;
		}

		for (const NpcBarkRequest& bark : outbox.barks)
		{
			PendingBark pending;
			pending.npc = npc;
			pending.bark = bark.bark;
			pending.priority = NpcBarkTuning::Get().PriorityOf(bark.bark);
			barkQueue.push_back(std::move(pending));
		}

		for (NpcMessage& message : outbox.messages)
		{
			// Only those with a radio can use one; a bell is rung at an alarm point (RaiseAlarm), not sent.
			if (message.kind == NpcMessageKind::Radio && npc->GetProfile().radio == false) continue;
			if (message.kind == NpcMessageKind::Alarm) continue;

			const NpcMessageKindTuning& kind = message.kind == NpcMessageKind::Shout ? tuning.shout
				: (message.kind == NpcMessageKind::Report ? tuning.report : tuning.radio);

			message.senderId = npc->Id;
			message.origin = npc->Position;
			message.sentAt = now;
			message.deliverAt = now + kind.delay;
			pending.push_back(std::move(message));
		}

		const std::vector<NpcRequest> requests = std::move(outbox.requests);
		outbox = NpcOutbox();

		for (const NpcRequest& request : requests)
			HandleRequest(*npc, request, now);
	}
}

void NpcDirector::HandleRequest(Npc& npc, const NpcRequest& request, double now)
{
	NpcSituation* situation = FindSituationMutable(request.kind, request.subject);

	switch (request.type)
	{
	case NpcRequestType::Join:
	{
		const bool opening = situation == nullptr;
		if (opening)
			situation = &OpenSituation(request.kind, request.subject);

		if (situation->FindMember(npc.Id) == nullptr)
			situation->Join(npc.Id);

		if (opening)
			situation->Open(*this, now);
		break;
	}

	case NpcRequestType::Leave:
		if (situation)
			situation->Leave(npc.Id);
		break;

	case NpcRequestType::Progress:
		if (situation && situation->FindMember(npc.Id))
			situation->OnProgress(*this, npc.Id, request.progress, now);
		break;

	case NpcRequestType::AreaEvent:
		RaiseAreaEvent(request.areaEvent, request.position, now);
		break;

	case NpcRequestType::RaiseAlarm:
		RingAlarm(npc, request, now);
		break;

	case NpcRequestType::WakeBody:
	{
		// Somebody shakes a body awake: the one that lies there (it cannot act, so it is not looked for among those who can).
		for (Npc* other : npcs)
		{
			if (other->Id == request.subject && other->IsUnconscious())
				other->WakeUp();
		}
		break;
	}
	}
}

// ── Areas and alarms ────────────────────────────────────────────────────────────────────────────────────────────

void NpcDirector::RaiseAreaEvent(NpcAreaEvent type, const glm::vec3& position, double now)
{
	RaiseFloor(type, position, NpcSituationTuning::Get().floors.radius, now);
}

void NpcDirector::RaiseFloor(NpcAreaEvent type, const glm::vec3& position, float radius, double now)
{
	const NpcFloorRule& rule = RuleFor(NpcSituationTuning::Get().floors, type);

	const NpcAlert level = NpcAlertByName(rule.level);
	if (level == NpcAlert::Calm) return;

	// The same kind of event close by keeps the area it already has; it only holds the floor for longer.
	for (AreaFloor& floor : floors)
	{
		if (floor.type != type || Flat(floor.center, position) > floor.radius * 0.5f) continue;

		if (rule.duration < 0.0f)
			floor.until = -1.0;
		else if (floor.until >= 0.0)
			floor.until = std::max(floor.until, now + rule.duration);

		return;
	}

	AreaFloor floor;
	floor.type = type;
	floor.center = position;
	floor.radius = radius;
	floor.level = level;
	floor.then = NpcAlertByName(rule.then);
	floor.until = rule.duration < 0.0f ? -1.0 : now + rule.duration;
	floors.push_back(floor);
}

NpcAlert NpcDirector::FloorAt(const glm::vec3& position, double now) const
{
	NpcAlert result = NpcAlert::Calm;

	for (const AreaFloor& floor : floors)
	{
		if (glm::distance(floor.center, position) > floor.radius) continue;

		result = std::max(result, floor.until < 0.0 || now < floor.until ? floor.level : floor.then);
	}

	return result;
}

bool NpcDirector::AlarmActive(const AlarmPoint& point, double now) const
{
	for (const AreaFloor& floor : floors)
	{
		if (floor.type == NpcAreaEvent::Alarm && glm::distance(floor.center, point.Position) < 1.0f && (floor.until < 0.0 || now < floor.until))
			return true;
	}
	return false;
}

// The runner rings the bell it stands at: the area is on alert, and everybody in it is told where the intruder is, roughly.
void NpcDirector::RingAlarm(Npc& runner, const NpcRequest& request, double now)
{
	const NpcAlarmTuning& tuning = NpcSituationTuning::Get().alarm;

	const AlarmPoint* point = nullptr;
	for (const AlarmPoint* candidate : AlarmPoint::All())
	{
		if (glm::distance(candidate->Position, request.position) < 1.0f)
			point = candidate;
	}

	// There is such a bell, nobody has rung it already, and the one who rings it stands at it.
	if (point == nullptr || AlarmActive(*point, now)) return;
	if (Flat(runner.Position, point->StandingPlace()) > tuning.arriveRadius + 1.5f) return;

	const float area = point->Radius();
	RaiseFloor(NpcAreaEvent::Alarm, point->Position, area, now);

	NpcRecord rough = request.record;
	rough.position = RoughPosition(request.record.position, request.record.subject, ++alarmsRung, tuning);
	rough.velocity = glm::vec3(0.0f);
	rough.certainty = tuning.roughCertainty;
	rough.detail = "hostile";

	NpcMessage message;
	message.kind = NpcMessageKind::Alarm;
	message.senderId = runner.Id;
	message.origin = point->Position;
	message.radius = area;
	message.sentAt = now;
	message.deliverAt = now;
	message.records.push_back(std::move(rough));

	Deliver(message);
}

// A message is heard when its time comes and its sender can still act: killing the one who shouts before then keeps the
// others in the dark (§7).
void NpcDirector::DeliverMessages(double now)
{
	std::vector<NpcMessage> due;

	pending.erase(std::remove_if(pending.begin(), pending.end(), [&](NpcMessage& message)
	{
		if (message.deliverAt > now) return false;
		due.push_back(std::move(message));
		return true;
	}), pending.end());

	for (const NpcMessage& message : due)
	{
		if (FindActing(message.senderId) == nullptr) continue;
		Deliver(message);
	}
}

void NpcDirector::Deliver(const NpcMessage& message)
{
	const NpcMessageTuning& tuning = NpcSituationTuning::Get().messages;
	const Npc* sender = FindActing(message.senderId);

	for (Npc* receiver : npcs)
	{
		if (receiver->Id == message.senderId || receiver->CanAct() == false || receiver->observer == nullptr) continue;

		const glm::vec3 ear = receiver->observer->position;
		bool heard = false;

		switch (message.kind)
		{
		case NpcMessageKind::Shout:
			heard = AiPerceptionSystem::MuffledDistance(message.origin, ear) <= tuning.shout.radius;
			break;

		case NpcMessageKind::Report:
			heard = glm::distance(message.origin, ear) <= tuning.report.radius;
			break;

		case NpcMessageKind::Radio:
			heard = receiver->GetProfile().faction == sender->GetProfile().faction && glm::distance(message.origin, ear) <= tuning.radio.radius;
			break;

		case NpcMessageKind::Alarm:
			heard = glm::distance(message.origin, ear) <= message.radius;
			break;
		}

		if (heard)
			receiver->mind.inbox.messages.push_back(message);
	}
}

void NpcDirector::StepSituations(double now, float dt)
{
	for (auto& situation : situations)
	{
		// Off means off: whoever cannot act holds no role (§4.1 rule 4).
		std::vector<std::string> gone;
		for (const SituationMember& member : situation->Members())
		{
			if (FindActing(member.npcId) == nullptr)
				gone.push_back(member.npcId);
		}
		for (const std::string& npcId : gone)
			situation->Leave(npcId);

		if (situation->Finished() || situation->Members().empty())
			continue;

		situation->Step(*this, now, dt);
	}

	// A situation that is over tells whoever is left in it that the subject is settled.
	for (auto& situation : situations)
	{
		if (situation->Finished() == false && situation->Members().empty() == false) continue;

		for (const SituationMember& member : situation->Members())
		{
			if (Npc* npc = FindActing(member.npcId))
			{
				NpcEnded ended;
				ended.kind = situation->Kind();
				ended.subject = situation->Subject();
				npc->mind.inbox.ended.push_back(std::move(ended));
			}
		}
	}

	situations.erase(std::remove_if(situations.begin(), situations.end(), [](const std::unique_ptr<NpcSituation>& situation)
	{
		return situation->Finished() || situation->Members().empty();
	}), situations.end());
}

// ── Voices ──────────────────────────────────────────────────────────────────────────────────────────────────────

int NpcDirector::SituationIdOf(const std::string& npcId) const
{
	for (const auto& situation : situations)
	{
		if (situation->FindMember(npcId))
			return situation->Id();
	}
	return 0;
}

// Who says what (18.2): the most important of the barks asked for goes first; a bark is dropped when its speaker or its
// situation spoke a moment ago, when another voice that began lately is within earshot, or when enough barks have started
// in the last second already. The use counts of a bark in this level choose the tier of its line.
void NpcDirector::ArbitrateBarks(double now)
{
	if (barkQueue.empty()) return;

	const NpcBarkTuning& tuning = NpcBarkTuning::Get();

	std::stable_sort(barkQueue.begin(), barkQueue.end(), [](const PendingBark& a, const PendingBark& b) { return a.priority > b.priority; });

	const double keep = std::max({ static_cast<double>(tuning.situationSpacing), static_cast<double>(tuning.npcSpacing),
		static_cast<double>(tuning.overlapTime), static_cast<double>(tuning.globalWindow) });
	voices.erase(std::remove_if(voices.begin(), voices.end(), [&](const Voice& voice) { return now - voice.time > keep; }), voices.end());

	for (const PendingBark& request : barkQueue)
	{
		Npc* npc = request.npc;
		if (npc->CanAct() == false) continue;

		const int situation = SituationIdOf(npc->Id);

		bool allowed = true;
		int started = 0;

		for (const Voice& voice : voices)
		{
			const double age = now - voice.time;

			if (voice.npcId == npc->Id && age < tuning.npcSpacing) allowed = false;
			if (situation != 0 && voice.situation == situation && age < tuning.situationSpacing) allowed = false;
			if (voice.npcId != npc->Id && age < tuning.overlapTime && glm::distance(voice.position, npc->Position) <= tuning.overlapRadius) allowed = false;
			if (age < tuning.globalWindow) ++started;
		}

		if (started >= tuning.globalCap) allowed = false;
		if (allowed == false) continue;

		BarkEntry entry;
		entry.time = now;
		entry.npcId = npc->Id;
		entry.bark = request.bark;
		entry.tier = tuning.TierForUse(barkUses[request.bark]++);
		entry.event = NpcBarkTuning::EventFor(npc->GetProfile().voice, request.bark, entry.tier);

		npc->Speak(entry.event);

		Voice voice;
		voice.npcId = npc->Id;
		voice.position = npc->Position;
		voice.time = now;
		voice.situation = situation;
		voices.push_back(std::move(voice));

		barkLog.push_back(std::move(entry));
		if (barkLog.size() > 128)
			barkLog.erase(barkLog.begin(), barkLog.begin() + 32);
	}

	barkQueue.clear();
}

// ── Save and load ───────────────────────────────────────────────────────────────────────────────────────────────

void NpcDirector::Serialize(json& target)
{
	Entity::Serialize(target);

	const double now = Time::GameTime;

	json data;
	data["nextSituationId"] = nextSituationId;

	json situationList = json::array();
	for (const auto& situation : situations)
		situationList.push_back(situation->Serialize(now));
	data["situations"] = std::move(situationList);

	json messageList = json::array();
	for (const NpcMessage& message : pending)
	{
		json item = message;
		item["sentAt"] = now - message.sentAt;
		item["deliverAt"] = message.deliverAt - now;
		item["records"] = NpcMemory::RecordsToJson(message.records, now);
		messageList.push_back(std::move(item));
	}
	data["pending"] = std::move(messageList);

	json floorList = json::array();
	for (const AreaFloor& floor : floors)
	{
		json item;
		item["type"] = floor.type;
		item["center"] = floor.center;
		item["radius"] = floor.radius;
		item["level"] = floor.level;
		item["then"] = floor.then;
		item["remaining"] = floor.until < 0.0 ? -1.0 : floor.until - now;
		floorList.push_back(std::move(item));
	}
	data["floors"] = std::move(floorList);
	data["alarmsRung"] = alarmsRung;
	data["barkUses"] = barkUses;

	target["npcDirector"] = std::move(data);
}

void NpcDirector::Deserialize(json& source)
{
	Entity::Deserialize(source);

	situations.clear();
	pending.clear();
	floors.clear();
	barkQueue.clear();
	voices.clear();
	barkLog.clear();
	barkUses.clear();

	if (source.contains("npcDirector") == false) return;

	const json& data = source["npcDirector"];
	const double now = Time::GameTime;

	nextSituationId = data.value("nextSituationId", 1);
	alarmsRung = data.value("alarmsRung", 0);
	barkUses = data.value("barkUses", std::map<std::string, int>());

	for (const json& item : data.value("floors", json::array()))
	{
		AreaFloor floor;
		floor.type = item.value("type", NpcAreaEvent::BodyFound);
		floor.center = item.value("center", glm::vec3(0.0f));
		floor.radius = item.value("radius", 30.0f);
		floor.level = item.value("level", NpcAlert::Wary);
		floor.then = item.value("then", NpcAlert::Wary);

		const double remaining = item.value("remaining", -1.0);
		floor.until = remaining < 0.0 ? -1.0 : now + remaining;
		floors.push_back(floor);
	}

	for (const json& item : data.value("situations", json::array()))
	{
		const SituationKind kind = item.value("kind", SituationKind::Investigate);
		const std::string subject = item.value("subject", std::string());
		const int id = item.value("id", 0);

		std::unique_ptr<NpcSituation> situation = kind == SituationKind::Investigate
			? std::unique_ptr<NpcSituation>(std::make_unique<InvestigateSituation>(subject, id))
			: std::unique_ptr<NpcSituation>(std::make_unique<SearchSituation>(subject, id));

		situation->Deserialize(item, now);
		situations.push_back(std::move(situation));
	}

	for (const json& item : data.value("pending", json::array()))
	{
		NpcMessage message = item.get<NpcMessage>();
		message.sentAt = now - message.sentAt;
		message.deliverAt = now + message.deliverAt;
		message.records = NpcMemory::RecordsFromJson(item.value("records", json::array()), now);
		pending.push_back(std::move(message));
	}
}

// ── Debug panel ─────────────────────────────────────────────────────────────────────────────────────────────────

void NpcDirector::UpdateDebugUI()
{
	if (situations.empty() && pending.empty() && floors.empty() && barkLog.empty()) return;

	const double now = Time::GameTime;

	ImGui::Begin("npc director");

	ImGui::Text("%d NPCs, %d messages on the way", static_cast<int>(npcs.size()), static_cast<int>(pending.size()));

	for (const AreaFloor& floor : floors)
	{
		ImGui::Text("floor %s round %.0f %.0f %.0f (%.0f m): %s%s", floor.type == NpcAreaEvent::Alarm ? "alarm" : (floor.type == NpcAreaEvent::BodyFound ? "body" : "event"),
			floor.center.x, floor.center.y, floor.center.z, floor.radius, NpcAlertName(floor.level),
			floor.until < 0.0 ? ", for good" : (std::string(", ") + std::to_string(static_cast<int>(std::max(0.0, floor.until - now))) + " s").c_str());
	}

	if (barkLog.empty() == false)
	{
		const BarkEntry& last = barkLog.back();
		ImGui::Text("last bark: %s (%s, t%d) %.1f s ago", last.bark.c_str(), last.npcId.c_str(), last.tier, now - last.time);
	}

	for (const auto& situation : situations)
	{
		ImGui::Separator();
		ImGui::Text("%s of %s: %s", SituationKindName(situation->Kind()), situation->Subject().c_str(), situation->Describe(now).c_str());

		for (const SituationMember& member : situation->Members())
			ImGui::Text("  %s: %s", member.npcId.c_str(), NpcRoleName(member.order.role));
	}

	ImGui::End();
}

REGISTER_ENTITY(NpcDirector, "npc_director")
