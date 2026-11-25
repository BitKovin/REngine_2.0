// The search situation (NPC_AI_DESIGN.md §11.3 to §11.5, §12): a hostile that is not in sight any more.

#include "NpcSituation.h"

#include "NpcDirector.h"
#include "../AlarmPoint.h"
#include "../Npc.h"
#include "../Spatial/PostQuery.h"

#include <MathHelper.hpp>
#include <Physics.h>

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace
{
	const glm::vec3 Up(0.0f, 1.0f, 0.0f);

	float Flat(const glm::vec3& a, const glm::vec3& b)
	{
		return glm::length(MathHelper::XZ(a - b));
	}

	// Some member sees the subject right now.
	bool AnySees(const NpcDirector& director, const NpcSituation& situation)
	{
		for (const SituationMember& member : situation.Members())
		{
			const Npc* npc = director.FindActing(member.npcId);
			if (npc != nullptr && npc->GetMind().Sees(situation.Subject()))
				return true;
		}
		return false;
	}

	int CountRole(const std::vector<SituationMember>& members, NpcRole role)
	{
		return static_cast<int>(std::count_if(members.begin(), members.end(), [&](const SituationMember& m) { return m.order.role == role; }));
	}

	// The alarm point that stands at a position.
	const AlarmPoint* AlarmAt(const glm::vec3& position)
	{
		for (const AlarmPoint* point : AlarmPoint::All())
		{
			if (glm::distance(point->Position, position) < 1.0f)
				return point;
		}
		return nullptr;
	}

	// How far the search map of a body is spread: wide for a fresh kill (the killer is near), narrower the older it is.
	float BodyRadius(const NpcRecord& body, double now)
	{
		const NpcBodySearchTuning& tuning = NpcSituationTuning::Get().bodySearch;

		const float age = static_cast<float>(now - body.time);
		if (age <= tuning.freshAge) return tuning.radiusFresh;
		if (age >= tuning.oldAge) return tuning.radiusOld;

		return glm::mix(tuning.radiusFresh, tuning.radiusOld, (age - tuning.freshAge) / (tuning.oldAge - tuning.freshAge));
	}
}

// ── Phases ──────────────────────────────────────────────────────────────────────────────────────────────────────

void SearchSituation::Open(NpcDirector& director, double now)
{
	const NpcRecord* news = director.Freshest(*this);
	if (news == nullptr)
	{
		finished = true;
		return;
	}

	const NpcRecord latest = *news;

	// A body is not seen again and does not run: its search starts at once, round the place it lies.
	bodySubject = latest.kind == NpcRecordKind::Body;
	if (bodySubject)
	{
		EnterSearch(director, latest, false, now);
		return;
	}

	if (AnySees(director, *this))
	{
		lastContact = now;
		EnterContact(now);
	}
	else if (now - latest.lastSeen <= NpcSituationTuning::Get().chase.chaseWindow)
	{
		EnterChase(latest, now);
	}
	else
	{
		EnterSearch(director, latest, false, now);
	}
}

void SearchSituation::EnterContact(double now)
{
	phase = Phase::Contact;
	phaseSince = now;
	flood = ChaseFlood();
	floodRuns = 0;
	chaseArrived = false;
	map = SearchMap();
}

void SearchSituation::EnterChase(const NpcRecord& news, double now)
{
	phase = Phase::Chase;
	phaseSince = now;
	chaseArrived = false;
	floodRuns = 0;
	StartFlood(news);
}

// The flood and the map both begin where the news says it was last known; they know no more than that.
void SearchSituation::StartFlood(const NpcRecord& news)
{
	flood.Start(news.position, news.velocity, NpcSituationTuning::Get().chase);
	floodTime = news.time;
	++floodRuns;

	map.Seed(news.position, news.certainty, NpcSituationTuning::Get().search);
}

void SearchSituation::EnterSearch(NpcDirector& director, const NpcRecord& news, bool fromChase, double now)
{
	const NpcSearchTuning& search = NpcSituationTuning::Get().search;
	const bool seeded = map.Seeded() || (bodySubject ? map.SeedRadius(news.position, BodyRadius(news, now), search) : map.Seed(news.position, news.certainty, search));
	if (seeded == false)
	{
		finished = true; // nowhere on the grid to look
		return;
	}

	phase = Phase::Search;
	phaseSince = now;
	searchSince = now;
	nextClaims = now;
	clearCursor = 0;

	// The chasers have looked at the place it was lost; an alarm has not been looked at yet.
	checked = fromChase;
	checkPoint = news.position;

	for (SituationMember& member : members)
	{
		member.claim = -1;
		member.arrivedAt = -1.0;

		if (member.order.role != NpcRole::AlarmRunner)
			member.order = NpcOrder();
	}

	AssignSearchRoles(director, now);
}

void SearchSituation::Step(NpcDirector& director, double now, float dt)
{
	const NpcRecord* freshest = director.Freshest(*this);
	if (freshest == nullptr)
	{
		finished = true;
		return;
	}

	const NpcRecord news = *freshest;

	const bool contact = bodySubject == false && AnySees(director, *this);
	if (contact)
		lastContact = now;

	UpdateAlarmRunner(director, now);

	// Seeing it again ends whatever was planned; losing it for good starts the chase.
	if (contact && phase != Phase::Contact)
		EnterContact(now);
	else if (contact == false && phase == Phase::Contact && now - lastContact > NpcPerceptionTuning::Get().alert.targetLostAfter)
		EnterChase(news, now);

	switch (phase)
	{
	case Phase::Contact:
		StepContact(director, news);
		break;

	case Phase::Chase:
		StepChase(director, news, now);
		if (phase == Phase::Chase)
		{
			map.Spread(dt, now);
			ClearSeen(director, now);
		}
		break;

	case Phase::Search:
		StepSearch(director, now, dt);
		break;
	}
}

// Somebody sees it: whoever does not closes on the freshest news (the ones who see it fight, their skills see to that).
void SearchSituation::StepContact(NpcDirector& director, const NpcRecord& news)
{
	for (SituationMember& member : members)
	{
		if (member.order.role == NpcRole::AlarmRunner) continue;

		member.order = NpcOrder();
		member.order.role = NpcRole::Chaser;
		member.order.hasDestination = true;
		member.order.destination = news.position;
	}
}

void SearchSituation::StepChase(NpcDirector& director, const NpcRecord& news, double now)
{
	const NpcChaseTuning& chase = NpcSituationTuning::Get().chase;

	// After a load the flood is made again; new news makes it again (a few times).
	if (floodRuns == 0)
		StartFlood(news);
	else if (news.time > floodTime + 0.5 && floodRuns <= chase.maxReruns && Flat(news.position, flood.Origin()) > chase.rerunDistance)
		StartFlood(news);

	if (flood.Done() == false)
		flood.Advance(chase.stepsPerFrame);

	if (chaseArrived || now - phaseSince > chase.maxTime)
	{
		EnterSearch(director, news, true, now);
		return;
	}

	for (SituationMember& member : members)
	{
		if (member.order.role == NpcRole::AlarmRunner) continue;

		member.order = NpcOrder();
		member.order.role = NpcRole::Chaser;
		member.order.hasDestination = true;
		member.order.destination = flood.Destination();
	}
}

void SearchSituation::StepSearch(NpcDirector& director, double now, float dt)
{
	const NpcSearchTuning& search = NpcSituationTuning::Get().search;

	map.Spread(dt, now);
	ClearSeen(director, now);

	if (map.RemainingFraction() < search.endMass || now - searchSince > search.timeout)
	{
		finished = true;
		return;
	}

	AssignSearchRoles(director, now);

	if (now >= nextClaims)
	{
		nextClaims = now + search.claimInterval;
		UpdateClaims(director, now);
	}

	UpdateLooks(director, now);
}

void SearchSituation::OnProgress(NpcDirector& director, const std::string& npcId, NpcProgress progress, double now)
{
	const SituationMember* member = FindMember(npcId);
	if (member == nullptr) return;

	if (progress == NpcProgress::ChaseArrived && phase == Phase::Chase)
		chaseArrived = true;

	if (progress == NpcProgress::Checked && member->order.role == NpcRole::Checker)
		checked = true;
}

// ── The map and the searchers ───────────────────────────────────────────────────────────────────────────────────

// The runner for the bell (§17.11, an exclusive role): the member that has been told of the intruder and is nearest to an
// alarm point nobody has rung, as long as somebody sees the intruder. It runs while the others fight or search. The job ends
// when the bell has been rung (by it or by anybody), and with the runner: whoever cannot act holds no role.
void SearchSituation::UpdateAlarmRunner(NpcDirector& director, double now)
{
	const NpcAlarmTuning& tuning = NpcSituationTuning::Get().alarm;

	for (SituationMember& member : members)
	{
		if (member.order.role != NpcRole::AlarmRunner) continue;

		const AlarmPoint* point = AlarmAt(member.alarmPosition);
		if (point == nullptr || director.AlarmActive(*point, now))
		{
			member.order = NpcOrder();
			return;
		}

		member.order.hasDestination = true;
		member.order.destination = point->StandingPlace();
		member.order.hasLook = true;
		member.order.look = point->Position;
		return;
	}

	if (bodySubject || AnySees(director, *this) == false) return;

	SituationMember* pick = nullptr;
	const AlarmPoint* pickPoint = nullptr;
	float best = tuning.maxRunDistance;

	for (SituationMember& member : members)
	{
		const Npc* npc = director.FindActing(member.npcId);
		if (npc == nullptr || npc->GetProfile().alarmRunner == false) continue;

		// It runs for what it knows: an intruder it has seen or been told of.
		const NpcRecord* known = npc->GetMind().memory.Find(subject, NpcRecordKind::Person);
		if (known == nullptr || known->detail != "hostile") continue;

		for (const AlarmPoint* point : AlarmPoint::All())
		{
			if (director.AlarmActive(*point, now)) continue;

			const float distance = Flat(npc->Position, point->Position);
			if (distance < best)
			{
				best = distance;
				pick = &member;
				pickPoint = point;
			}
		}
	}

	if (pick == nullptr) return;

	pick->order = NpcOrder();
	pick->order.role = NpcRole::AlarmRunner;
	pick->order.hasDestination = true;
	pick->order.destination = pickPoint->StandingPlace();
	pick->order.hasLook = true;
	pick->order.look = pickPoint->Position;
	pick->alarmPosition = pickPoint->Position;
	pick->claim = -1;
}

// What the members' eyes reach clears the map: a cell inside a vision zone with a clear ray is seen (§11.4).
void SearchSituation::ClearSeen(NpcDirector& director, double now)
{
	if (map.Seeded() == false) return;

	const NpcSearchTuning& search = NpcSituationTuning::Get().search;

	std::vector<const Observer*> eyes;
	for (const SituationMember& member : members)
	{
		const Npc* npc = director.FindActing(member.npcId);
		if (npc != nullptr && npc->observer != nullptr)
			eyes.push_back(npc->observer.get());
	}

	if (eyes.empty()) return;

	const int count = map.Count();
	int rays = 0;

	for (int tested = 0; tested < count && rays < search.raysPerFrame; ++tested)
	{
		const int cell = clearCursor;
		clearCursor = (clearCursor + 1) % count;

		const glm::vec3 point = map.Floor(cell) + Up * search.clearHeight;

		for (const Observer* eye : eyes)
		{
			if (eye->ZoneAt(point) == VisionZone::None) continue;

			++rays;
			if (Physics::LineTrace(eye->position, point, BodyType::WorldOpaque).hasHit == false)
			{
				map.MarkSeen(cell, now);
				break;
			}
		}
	}
}

// The jobs of a search (§10): one checks the original spot, watchers hold doorways when there are enough of them, the rest
// search. Only vacancies are filled, so nobody is moved about for nothing.
void SearchSituation::AssignSearchRoles(NpcDirector& director, double now)
{
	const NpcSearchTuning& search = NpcSituationTuning::Get().search;

	std::vector<SituationMember*> acting;
	for (SituationMember& member : members)
	{
		if (member.order.role != NpcRole::AlarmRunner && director.FindActing(member.npcId) != nullptr)
			acting.push_back(&member);
	}

	if (acting.empty()) return;

	SituationMember* checker = nullptr;
	for (SituationMember* member : acting)
	{
		if (member->order.role == NpcRole::Checker)
			checker = member;
	}

	if (checked && checker != nullptr)
	{
		checker->order = NpcOrder();
		checker->order.role = NpcRole::Searcher;
		checker = nullptr;
	}
	else if (checked == false && checker == nullptr)
	{
		float best = 1.0e9f;
		for (SituationMember* member : acting)
		{
			const float distance = glm::distance(director.FindActing(member->npcId)->Position, checkPoint);
			if (distance < best)
			{
				best = distance;
				checker = member;
			}
		}

		checker->order = NpcOrder();
		checker->order.role = NpcRole::Checker;
		checker->claim = -1;

		if (bodySubject)
		{
			// The one who looks after a body goes to it (the tending skill reads the body from its memory).
			checker->order.hasDestination = true;
			checker->order.destination = checkPoint;
			checker->order.hasLook = true;
			checker->order.look = checkPoint;
		}
		else
		{
			PlanCheck(director, *checker);
		}
	}

	// One searcher at least, so the watchers are the ones beyond the second member.
	const int others = static_cast<int>(acting.size()) - (checker ? 1 : 0);
	const int allowedWatchers = std::clamp(others - 1, 0, search.maxWatchers);

	int watchers = CountRole(members, NpcRole::Watcher);
	for (auto it = acting.rbegin(); it != acting.rend() && watchers > allowedWatchers; ++it)
	{
		if ((*it)->order.role == NpcRole::Watcher)
		{
			(*it)->order = NpcOrder();
			(*it)->order.role = NpcRole::Searcher;
			(*it)->claim = -1;
			--watchers;
		}
	}

	for (SituationMember* member : acting)
	{
		if (member->order.role == NpcRole::None || member->order.role == NpcRole::Chaser)
		{
			member->order = NpcOrder();
			member->order.role = NpcRole::Searcher;
			member->claim = -1;
		}
	}
}

// Where the checker looks at the original spot from: a post with a view of it, like an investigator's.
void SearchSituation::PlanCheck(NpcDirector& director, SituationMember& member) const
{
	const Npc* npc = director.FindActing(member.npcId);
	const NpcInvestigateTuning& tuning = npc->GetProfile().investigate;

	member.order.hasLook = true;
	member.order.look = checkPoint;
	member.order.hasDestination = true;
	member.order.destination = checkPoint;

	glm::vec3 post;
	const std::vector<int> cells = PostQuery::CellsAround(checkPoint, tuning.postMax);
	if (PostQuery::Find(cells, PostQuery::OpenPost(checkPoint, npc->Position, tuning.postMin, tuning.postMax, {}), post))
		member.order.destination = post;
}

int SearchSituation::ChooseClaim(const glm::vec3& from, const std::vector<glm::vec3>& taken, double now) const
{
	const NpcSearchTuning& search = NpcSituationTuning::Get().search;

	int best = -1;
	float bestScore = 0.0f;

	for (int i = 0; i < map.Count(); ++i)
	{
		if (map.IsSeen(i, now)) continue;

		const float nearby = map.Nearby(i);
		if (nearby <= 1.0e-6f) continue;

		const glm::vec3& floor = map.Floor(i);

		bool crowded = false;
		for (const glm::vec3& other : taken)
			crowded |= Flat(floor, other) < search.claimSpacing;
		if (crowded) continue;

		// The most probability for the shortest walk.
		const float score = nearby / (1.0f + glm::distance(from, floor) / 10.0f);
		if (score > bestScore)
		{
			bestScore = score;
			best = i;
		}
	}

	return best;
}

// Chokes (doorways) and what a watcher there would guard: the probability within reach of each (§11.5).
std::unordered_map<int, float> SearchSituation::ChokeMasses() const
{
	const float radius = NpcSituationTuning::Get().search.watchRadius;

	std::unordered_map<int, float> masses;

	for (int i = 0; i < map.Count(); ++i)
	{
		if (map.IsChoke(i) == false) continue;

		float total = 0.0f;
		for (int j = 0; j < map.Count(); ++j)
		{
			if (map.Probability(j) > 0.0f && Flat(map.Floor(i), map.Floor(j)) <= radius)
				total += map.Probability(j);
		}

		if (total > 1.0e-3f)
			masses[map.GridIndex(i)] = total;
	}

	return masses;
}

// The best choke to hold: the one with the most probability behind it, not near another watcher's, near the candidate.
// The watcher stands on the side of it that holds less probability and looks at it.
bool SearchSituation::FindWatchPost(const glm::vec3& from, const std::vector<glm::vec3>& taken, const std::unordered_map<int, float>& masses,
	int& choke, glm::vec3& post, glm::vec3& look) const
{
	std::vector<int> cells;
	cells.reserve(masses.size());
	for (const auto& entry : masses)
		cells.push_back(entry.first);

	// A fixed order, so equal scores always give the same choke.
	std::sort(cells.begin(), cells.end());

	PostSelector selector;
	selector.push_back({ [&masses](const PostCandidate& candidate)
	{
		const auto found = masses.find(candidate.cell);
		return found == masses.end() ? 0.0f : 1.0f - std::exp(-found->second / 0.05f);
	}, false });
	selector.push_back(PostQuery::NotClaimed(taken, NpcSituationTuning::Get().search.watchSpacing));
	selector.push_back(PostQuery::Near(from, 20.0f));

	glm::vec3 chokeFloor;
	if (PostQuery::Find(cells, selector, chokeFloor) == false) return false;

	choke = map.Nearest(chokeFloor);

	int side = choke;
	float least = map.Nearby(choke);
	for (const int neighbour : map.Links(choke))
	{
		if (neighbour >= 0 && map.Nearby(neighbour) < least)
		{
			least = map.Nearby(neighbour);
			side = neighbour;
		}
	}

	post = map.Floor(side);
	look = chokeFloor + Up * 1.4f;
	return true;
}

void SearchSituation::UpdateClaims(NpcDirector& director, double now)
{
	const NpcSearchTuning& search = NpcSituationTuning::Get().search;

	// Searchers: a claim is spent once its cell is seen, empty, or the searcher has stood there long enough to have looked.
	std::vector<SituationMember*> needClaim;
	std::vector<glm::vec3> taken;

	for (SituationMember& member : members)
	{
		if (member.order.role != NpcRole::Searcher) continue;

		const Npc* npc = director.FindActing(member.npcId);
		if (npc == nullptr) continue;

		bool valid = member.claim >= 0 && member.claim < map.Count() && map.IsSeen(member.claim, now) == false && map.Nearby(member.claim) > 1.0e-6f;

		if (valid)
		{
			if (Flat(npc->Position, map.Floor(member.claim)) <= npc->GetProfile().pursuit.arriveRadius)
			{
				if (member.arrivedAt < 0.0) member.arrivedAt = now;

				if (now - member.arrivedAt >= search.claimDwell)
				{
					map.MarkSeen(member.claim, now);
					valid = false;
				}
			}
			else
			{
				member.arrivedAt = -1.0;
			}
		}

		if (valid)
			taken.push_back(map.Floor(member.claim));
		else
			needClaim.push_back(&member);
	}

	for (SituationMember* member : needClaim)
	{
		const Npc* npc = director.FindActing(member->npcId);

		member->claim = ChooseClaim(npc->Position, taken, now);
		member->claimedAt = now;
		member->arrivedAt = -1.0;

		if (member->claim >= 0)
			taken.push_back(map.Floor(member->claim));
	}

	for (SituationMember& member : members)
	{
		if (member.order.role != NpcRole::Searcher) continue;

		member.order.hasDestination = member.claim >= 0;
		if (member.claim >= 0)
			member.order.destination = map.Floor(member.claim);
	}

	// Watchers: a watcher keeps its doorway while probability is left behind it, and gives the post up when there is none.
	const int others = static_cast<int>(members.size()) - CountRole(members, NpcRole::Checker) - CountRole(members, NpcRole::AlarmRunner);
	const int allowed = std::clamp(others - 1, 0, search.maxWatchers);

	const std::unordered_map<int, float> masses = (allowed > 0 || CountRole(members, NpcRole::Watcher) > 0) ? ChokeMasses() : std::unordered_map<int, float>();

	std::vector<glm::vec3> posts;
	for (SituationMember& member : members)
	{
		if (member.order.role != NpcRole::Watcher) continue;

		const bool held = member.claim >= 0 && member.claim < map.Count() && masses.count(map.GridIndex(member.claim)) > 0;
		if (held)
		{
			posts.push_back(map.Floor(member.claim));
			continue;
		}

		member.order = NpcOrder();
		member.order.role = NpcRole::Searcher;
		member.claim = -1;
	}

	int watchers = CountRole(members, NpcRole::Watcher);
	for (SituationMember& member : members)
	{
		if (watchers >= allowed) break;
		if (member.order.role != NpcRole::Searcher) continue;

		const Npc* npc = director.FindActing(member.npcId);
		if (npc == nullptr) continue;

		int choke = -1;
		glm::vec3 post, look;
		if (FindWatchPost(npc->Position, posts, masses, choke, post, look) == false) break;

		member.order = NpcOrder();
		member.order.role = NpcRole::Watcher;
		member.order.hasDestination = true;
		member.order.destination = post;
		member.order.hasLook = true;
		member.order.look = look;
		member.claim = choke;
		posts.push_back(map.Floor(choke));
		++watchers;
	}
}

// Where each searcher looks: the edge of what is seen next to the most probability (§11.4), not the bend of its path.
void SearchSituation::UpdateLooks(NpcDirector& director, double now)
{
	const NpcSearchTuning& search = NpcSituationTuning::Get().search;

	for (SituationMember& member : members)
	{
		if (member.order.role != NpcRole::Searcher) continue;
		if (now - member.lookChosenAt < search.frontierInterval) continue;

		const Npc* npc = director.FindActing(member.npcId);
		if (npc == nullptr) continue;

		member.lookChosenAt = now;
		member.order.hasLook = map.FrontierGaze(npc->Position, search.gazeRange, now, member.order.look);
	}
}

// ── Save and load ───────────────────────────────────────────────────────────────────────────────────────────────

json SearchSituation::Serialize(double now) const
{
	json data = NpcSituation::Serialize(now);

	data["phase"] = static_cast<int>(phase);
	data["bodySubject"] = bodySubject;
	data["phaseAge"] = now - phaseSince;
	data["contactAge"] = now - lastContact;
	data["checked"] = checked;
	data["checkPoint"] = checkPoint;
	data["searchAge"] = now - searchSince;

	if (phase == Phase::Search)
		data["map"] = map.Serialize(NpcSituationTuning::Get().search.saveRelative);

	return data;
}

// The chase flood is not saved: it is made again from the memories (floodRuns 0 asks for that).
void SearchSituation::Deserialize(const json& data, double now)
{
	NpcSituation::Deserialize(data, now);

	phase = static_cast<Phase>(data.value("phase", static_cast<int>(Phase::Search)));
	bodySubject = data.value("bodySubject", false);
	phaseSince = now - data.value("phaseAge", 0.0);
	lastContact = now - data.value("contactAge", 1.0e9);
	checked = data.value("checked", false);
	checkPoint = data.value("checkPoint", glm::vec3(0.0f));
	searchSince = now - data.value("searchAge", 0.0);
	nextClaims = now;
	floodRuns = 0;
	chaseArrived = false;

	if (phase == Phase::Search && (data.contains("map") == false || map.Deserialize(data["map"], NpcSituationTuning::Get().search) == false))
		finished = true;
}

std::string SearchSituation::Describe(double now) const
{
	const char* name = phase == Phase::Contact ? "contact" : (phase == Phase::Chase ? "chase" : "search");

	char text[160];
	if (phase == Phase::Search)
		snprintf(text, sizeof(text), "%s %.0fs, %.0f%% of the map left, %d members", name, now - searchSince, map.RemainingFraction() * 100.0f, static_cast<int>(members.size()));
	else
		snprintf(text, sizeof(text), "%s %.0fs, %d members", name, now - phaseSince, static_cast<int>(members.size()));
	return text;
}
