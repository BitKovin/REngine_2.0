#include "NpcSituation.h"

#include "NpcDirector.h"
#include "../Npc.h"

#include <algorithm>
#include <cstdio>
#include <limits>

// ── Base ────────────────────────────────────────────────────────────────────────────────────────────────────────

const SituationMember* NpcSituation::FindMember(const std::string& npcId) const
{
	for (const SituationMember& member : members)
	{
		if (member.npcId == npcId) return &member;
	}
	return nullptr;
}

SituationMember* NpcSituation::FindMember(const std::string& npcId)
{
	return const_cast<SituationMember*>(static_cast<const NpcSituation*>(this)->FindMember(npcId));
}

NpcOrder NpcSituation::OrderOf(const std::string& npcId) const
{
	const SituationMember* member = FindMember(npcId);
	return member ? member->order : NpcOrder();
}

void NpcSituation::Join(const std::string& npcId)
{
	SituationMember member;
	member.npcId = npcId;
	members.push_back(std::move(member));
}

void NpcSituation::Leave(const std::string& npcId)
{
	members.erase(std::remove_if(members.begin(), members.end(),
		[&](const SituationMember& member) { return member.npcId == npcId; }), members.end());
}

json NpcSituation::Serialize(double now) const
{
	json data;
	data["kind"] = kind;
	data["subject"] = subject;
	data["id"] = id;

	json list = json::array();
	for (const SituationMember& member : members)
	{
		json item;
		item["npcId"] = member.npcId;
		item["role"] = member.order.role;
		item["claim"] = member.claim;
		item["alarmPosition"] = member.alarmPosition;
		list.push_back(std::move(item));
	}
	data["members"] = std::move(list);

	return data;
}

void NpcSituation::Deserialize(const json& data, double now)
{
	members.clear();

	for (const json& item : data.value("members", json::array()))
	{
		SituationMember member;
		member.npcId = item.value("npcId", std::string());
		member.order.role = item.value("role", NpcRole::None);
		member.claim = item.value("claim", -1);
		member.alarmPosition = item.value("alarmPosition", glm::vec3(0.0f));
		members.push_back(std::move(member));
	}
}

// ── Investigate ─────────────────────────────────────────────────────────────────────────────────────────────────

void InvestigateSituation::Open(NpcDirector& director, double now)
{
	AssignRoles(director);
}

void InvestigateSituation::Step(NpcDirector& director, double now, float dt)
{
	AssignRoles(director);
}

// The investigator goes (the one nearest to the place when there are several to pick from), a backup covers it when
// somebody in the situation is on edge. Everybody else has no job: they glanced and carry on.
void InvestigateSituation::AssignRoles(NpcDirector& director)
{
	const NpcInvestigateRolesTuning& tuning = NpcSituationTuning::Get().investigate;

	const NpcRecord* news = director.Freshest(*this);
	if (news == nullptr) return;

	bool wary = false;
	for (const SituationMember& member : members)
	{
		if (const Npc* npc = director.FindActing(member.npcId))
			wary |= npc->GetMind().alert >= NpcAlert::Wary;
	}

	auto count = [&](NpcRole role)
	{
		return static_cast<int>(std::count_if(members.begin(), members.end(), [&](const SituationMember& m) { return m.order.role == role; }));
	};

	// The member with this role that is nearest to the place.
	auto pick = [&](NpcRole role) -> SituationMember*
	{
		SituationMember* best = nullptr;
		float bestDistance = std::numeric_limits<float>::max();

		for (SituationMember& member : members)
		{
			if (member.order.role != role) continue;

			const Npc* npc = director.FindActing(member.npcId);
			if (npc == nullptr) continue;

			const float distance = glm::distance(npc->Position, news->position);
			if (distance < bestDistance)
			{
				bestDistance = distance;
				best = &member;
			}
		}
		return best;
	};

	// A backup steps up when the investigator is gone.
	while (count(NpcRole::Investigator) < tuning.investigators)
	{
		SituationMember* member = pick(NpcRole::Backup);
		if (member == nullptr) member = pick(NpcRole::None);
		if (member == nullptr) break;
		member->order.role = NpcRole::Investigator;
	}

	const int backups = wary ? tuning.backupsWhenWary : 0;
	while (count(NpcRole::Backup) < backups)
	{
		SituationMember* member = pick(NpcRole::None);
		if (member == nullptr) break;
		member->order.role = NpcRole::Backup;
	}

	// Nobody is wary any more: a backup that is not needed goes back to its routine.
	if (count(NpcRole::Backup) > backups)
	{
		for (SituationMember& member : members)
		{
			if (member.order.role == NpcRole::Backup)
			{
				member.order.role = NpcRole::None;
				break;
			}
		}
	}
}

void InvestigateSituation::OnProgress(NpcDirector& director, const std::string& npcId, NpcProgress progress, double now)
{
	if (progress != NpcProgress::Resolved) return;

	const SituationMember* member = FindMember(npcId);
	if (member != nullptr && member->order.role == NpcRole::Investigator)
		finished = true;
}

std::string InvestigateSituation::Describe(double now) const
{
	char text[96];
	snprintf(text, sizeof(text), "%d members", static_cast<int>(members.size()));
	return text;
}
