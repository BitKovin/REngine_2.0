#include "AiTest.h"

#ifndef DISTRIBUTION

#include <AiPerception/ObservationTarget.h>
#include <AiPerception/Observer.h>

#include "../../Entities/Npc/Npc.h"
#include "../../Entities/Npc/Combat/CombatBehaviorBase.h"
#include "../../Entities/Npc/Combat/CombatCoordinator.h"
#include "../../Entities/Npc/Combat/CombatTacticBase.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace AiTest
{
	namespace
	{
		const char* ContactName(TargetContact contact)
		{
			switch (contact)
			{
			case TargetContact::Visible: return "visible";
			case TargetContact::Concealed: return "concealed";
			case TargetContact::Lost: return "lost";
			default: return "none";
			}
		}
	}

	void Probe::Begin(const std::string& scenario)
	{
		latest.clear();
		lastPositions.clear();
		lastTime = 0.0;
		lastRow = -1.0;
		csv = "time,name,class,alive,x,y,z,speed,yaw,distance,detection,attacking,following,sees,perceives,hears,tactic,role,lane,contact,est_err,skill,alert\n";
	}

	const Snapshot* Probe::Find(const std::string& name) const
	{
		for (const Snapshot& snapshot : latest)
		{
			if (snapshot.name == name) return &snapshot;
		}
		return nullptr;
	}

	void Probe::Sample(double time, const Puppet& puppet, const World& world)
	{
		const double dt = time - lastTime;
		lastTime = time;

		const std::string playerId = puppet.Id();
		const glm::vec3 playerPosition = puppet.Position();

		const TargetBelief* belief = playerId.empty() ? nullptr : CombatCoordinator::FindBelief(playerId);

		std::vector<Snapshot> snapshots;
		perceived = false;

		for (NpcBase* npc : world.Npcs())
		{
			Snapshot snapshot;
			snapshot.name = npc->Name;
			snapshot.id = npc->Id;
			snapshot.className = npc->ClassName;
			snapshot.alive = npc->ShowsAwareness();
			snapshot.position = npc->Position;
			snapshot.yaw = npc->Rotation.y;
			snapshot.distanceToPlayer = glm::distance(glm::vec2(npc->Position.x, npc->Position.z), glm::vec2(playerPosition.x, playerPosition.z));

			if (npc->observer)
			{
				const glm::vec3 forward = npc->observer->forward;
				snapshot.yaw = glm::degrees(std::atan2(forward.x, forward.z));
			}

			for (const auto& last : lastPositions)
			{
				if (last.first == snapshot.id && dt > 1.0e-4)
				{
					const glm::vec3 moved = snapshot.position - last.second;
					snapshot.speed = static_cast<float>(glm::length(glm::vec2(moved.x, moved.z)) / dt);
				}
			}

			std::vector<NpcBase::AwarenessReading> readings;
			npc->GetAwareness(readings);
			for (const NpcBase::AwarenessReading& reading : readings)
			{
				if (reading.subject == playerId)
					snapshot.detection = std::max(snapshot.detection, reading.value);
			}

			if (npc->observer)
			{
				for (const std::shared_ptr<ObservationTarget>& target : npc->observer->visibleTargets)
				{
					if (target->ownerId == playerId) snapshot.perceives = true;
				}
				for (const HeardSound& sound : npc->observer->heardSounds)
				{
					if (sound.causerId == playerId) snapshot.hears = true;
				}
				perceived |= snapshot.perceives || snapshot.hears;
			}

			if (const Npc* brain = dynamic_cast<const Npc*>(npc))
			{
				snapshot.skill = brain->CurrentSkillId();
				snapshot.alert = static_cast<int>(brain->GetMind().alert);
			}

			snapshot.attacking = npc->WantsToAttackTarget();
			snapshot.following = npc->IsFollowingTarget();
			snapshot.sees = npc->SeesTarget();

			if (CombatBehaviorBase* brain = npc->GetCombatBehavior())
			{
				if (const CombatTacticBase* tactic = brain->GetCurrentTactic())
					snapshot.tactic = tactic->Id();

				const SquadOrder& order = brain->GetSquadOrder();
				snapshot.role = SquadRoleName(order.role);
				if (order.hasLane)
					snapshot.lane = LanePhaseName(brain->GetLanePhase());
			}

			snapshot.contact = ContactName(belief ? belief->contact : TargetContact::None);
				if (belief && belief->contact != TargetContact::None)
					snapshot.estimateError = glm::distance(belief->estimatePosition, playerPosition);

			snapshots.push_back(snapshot);
		}

		hasBelief = belief != nullptr && belief->contact != TargetContact::None;
		if (hasBelief)
		{
			beliefContact = static_cast<int>(belief->contact);
			beliefPoint = belief->contact == TargetContact::Visible ? belief->lastVisiblePosition : belief->estimatePosition;
		}

		lastPositions.clear();
		for (const Snapshot& snapshot : snapshots)
			lastPositions.emplace_back(snapshot.id, snapshot.position);

		latest = snapshots;

		// A trace row per NPC four times a second.
		if (time - lastRow < 0.25) return;
		lastRow = time;

		char row[512];
		for (const Snapshot& s : snapshots)
		{
			snprintf(row, sizeof(row), "%.2f,%s,%s,%d,%.2f,%.2f,%.2f,%.2f,%.0f,%.2f,%.2f,%d,%d,%d,%d,%d,%s,%s,%s,%s,%.1f,%s,%d\n",
				time, s.name.c_str(), s.className.c_str(), s.alive ? 1 : 0, s.position.x, s.position.y, s.position.z, s.speed,
				s.yaw, s.distanceToPlayer, s.detection, s.attacking ? 1 : 0, s.following ? 1 : 0, s.sees ? 1 : 0, s.perceives ? 1 : 0, s.hears ? 1 : 0,
				s.tactic.c_str(), s.role.c_str(), s.lane.c_str(), s.contact.c_str(), s.estimateError, s.skill.c_str(), s.alert);
			csv += row;
		}
	}
}

#endif
