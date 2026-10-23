// Time.hpp first: engine headers below use Time without including it.
#include <Time.hpp>

#include "SimAgents.h"

#include <Sim/Bodies/SimBodies.h>
#include <Sim/Bridge/SimWorldNavFile.h>

#include <SimNpc.h>

#include <Camera.h>
#include <EntityHandle.h>
#include <Level.hpp>
#include <Logger.hpp>
#include <Physics.h>

#include <entt/container/dense_map.hpp>
#include <glm/geometric.hpp>

#include <chrono>
#include <string>

namespace SimAgents
{
	namespace
	{
		using Clock = std::chrono::steady_clock;
		using namespace sim::bodies;

		struct Pending
		{
			BodySpawnProposed proposal;
			sim::Tick tick = 0; // the sim tick of the proposal: the snapshot must be at least this new to show where the agent is
			Clock::time_point since;
		};

		struct Body
		{
			EntityHandle handle;
			std::string label;
			Clock::time_point lastReport;
			bool despawning = false;
			BodyDespawnProposed::Reason reason = BodyDespawnProposed::Reason::Unknown;
		};

		// Keyed by the sim entity (its raw integer).
		entt::dense_map<uint32_t, Pending> pending;
		entt::dense_map<uint32_t, Body> bodies;
		Counters counters;
		int levelChangeId = -1;
		Clock::time_point levelChangedAt = Clock::now();

		uint32_t Key(sim::Entity entity) { return entt::to_integral(entity); }
		double Seconds(Clock::time_point since) { return std::chrono::duration<double>(Clock::now() - since).count(); }

		// "warden" for a named NPC, "agent #12" for the crowd.
		std::string LabelOf(const sim::movement::AgentView* agent, sim::Entity entity)
		{
			if (agent && agent->id.c_str()[0] != '\0')
				return agent->id.str();
			return "agent #" + std::to_string(sim::EntityIndex(entity));
		}

		const sim::movement::AgentView* FindAgent(sim::SimClient& client, sim::Entity entity)
		{
			for (const sim::movement::AgentView& agent : client.Snapshot().agents)
				if (agent.entity == entity)
					return &agent;
			return nullptr;
		}

		const char* ReasonText(BodyDespawnProposed::Reason reason)
		{
			switch (reason)
			{
			case BodyDespawnProposed::Reason::OutOfRange: return "out of range";
			case BodyDespawnProposed::Reason::OtherLevel: return "player left the level";
			case BodyDespawnProposed::Reason::NoPlayer: return "no player";
			case BodyDespawnProposed::Reason::AtPortal: return "reached a portal";
			default: return "the sim dropped it";
			}
		}

		// Out of the camera's cone, behind level geometry, or in a cluster the camera's cannot see.
		bool HiddenFromCamera(const glm::vec3& feet)
		{
			Level* level = Level::Current;
			if (!level)
				return true;
			const glm::vec3 eye = Camera::position;
			const glm::vec3 chest = feet + glm::vec3(0.0f, 1.0f, 0.0f);
			const glm::vec3 toPoint = chest - eye;
			const float distance = glm::length(toPoint);
			if (distance < 1e-3f)
				return false;
			if (glm::dot(toPoint / distance, Camera::Forward()) < 0.35f) // outside a 70 degree half cone
				return true;
			if (Physics::LineTrace(eye, chest, BodyType::World).hasHit)
				return true;
			if (level->BspData.m_numOfVerts)
			{
				const int cameraCluster = level->BspData.FindClusterAtPosition(eye);
				for (const float height : { 0.1f, 1.0f, 1.8f })
					if (level->BspData.IsClusterVisible(cameraCluster, level->BspData.FindClusterAtPosition(feet + glm::vec3(0.0f, height, 0.0f))))
						return false;
				return true;
			}
			return false;
		}

		SimNpc* Resolve(Body& body)
		{
			Entity* entity = body.handle.Resolve();
			return dynamic_cast<SimNpc*>(entity);
		}

		template<class E> void Send(sim::SimClient& client, const E& event) { client.GetHost().SendEvent(event); }

		void HandleOutputs(sim::SimClient& client)
		{
			const sim::TypeRegistry& types = client.GetHost().Types();
			for (sim::Message& message : client.TakeOutputs())
			{
				if (const auto* proposal = message.As<BodySpawnProposed>(types))
				{
					++counters.proposals;
					pending[Key(proposal->entity)] = { *proposal, message.tick, Clock::now() };
				}
				else if (const auto* despawn = message.As<BodyDespawnProposed>(types))
				{
					const uint32_t key = Key(despawn->entity);
					if (auto found = bodies.find(key); found != bodies.end())
					{
						if (!found->second.despawning)
						{
							found->second.despawning = true;
							found->second.reason = despawn->reason;
						}
					}
					else
					{
						// A proposal that never became a body, or a body that is already gone: close it for the sim.
						glm::vec3 where(0.0f);
						if (auto waiting = pending.find(key); waiting != pending.end())
						{
							where = waiting->second.proposal.local;
							pending.erase(waiting);
						}
						Send(client, BodyReleased{ despawn->entity, where });
					}
				}
				else if (const auto* goal = message.As<BodyGoal>(types))
				{
					++counters.goals;
					if (auto found = bodies.find(Key(goal->entity)); found != bodies.end())
						if (SimNpc* npc = Resolve(found->second))
						{
							if (goal->kind == BodyGoal::Kind::Move)
								npc->SetGoal(goal->local, goal->next, goal->hasNext, goal->gait, goal->final);
							else
								npc->Stop();
						}
				}
			}
		}

		// The body appears where the agent is NOW (the newest snapshot), not where it was when the sim asked. Right after a level loads
		// it appears at once (the loading screen hid it); out of the camera's sight it appears at once; in sight it fades in.
		void HandlePending(sim::SimClient& client, const sim::nav::WorldNavData& world)
		{
			const bool levelJustLoaded = Seconds(levelChangedAt) < InitialSpawnWindow;
			const int currentLevel = SimWorldNavFile::FindLevel(world, Level::Current->filePath);
			std::vector<uint32_t> done;
			for (auto&& entry : pending)
			{
				const BodySpawnProposed& proposal = entry.second.proposal;
				// Snapshots trail the sim by a tick or two: an older one can still show the agent on the other side of a portal it just took.
				const sim::movement::AgentView* agent = client.SnapshotTick() >= entry.second.tick ? FindAgent(client, proposal.entity) : nullptr;
				if (!agent)
				{
					if (Seconds(entry.second.since) >= SnapshotWait) // the snapshot did not catch up with the proposal
					{
						Send(client, BodyRefused{ proposal.entity });
						++counters.refused;
						done.push_back(entry.first);
					}
					continue;
				}
				done.push_back(entry.first);
				if (agent->level != currentLevel)
				{
					// It walked through a portal since the sim asked: its position now belongs to another level's frame.
					Logger::Log("sim body: %s left the level before its body appeared; no body", LabelOf(agent, proposal.entity).c_str());
					Send(client, BodyRefused{ proposal.entity });
					++counters.refused;
					continue;
				}
				const glm::vec3 feet = agent->nav - world.levels[agent->level].offset;
				const bool hidden = HiddenFromCamera(feet);
				const bool fadeIn = !hidden && !levelJustLoaded;
				SimNpc* npc = SimNpc::SpawnAt(feet, proposal.heading, proposal.model, proposal.speedScale, Key(proposal.entity));
				if (!npc)
				{
					Send(client, BodyRefused{ proposal.entity });
					++counters.refused;
					continue;
				}
				if (fadeIn)
					npc->FadeIn();
				Body body;
				body.handle = EntityHandle::FromEntity(npc);
				body.label = LabelOf(agent, proposal.entity);
				body.lastReport = Clock::now();
				bodies[entry.first] = body;
				(fadeIn ? counters.spawnedVisible : counters.spawnedHidden) += 1;
				Logger::Log("sim body: %s appears at (%.1f %.1f %.1f), %.0f m away, %s", body.label.c_str(), feet.x, feet.y, feet.z, glm::distance(feet, Camera::position),
					levelJustLoaded ? "the level just loaded" : hidden ? "out of sight" : "fading in, in sight");
				Send(client, BodyAccepted{ proposal.entity, npc->FeetPosition() });
			}
			for (const uint32_t key : done)
				pending.erase(key);
		}

		void HandleBodies(sim::SimClient& client)
		{
			std::vector<uint32_t> gone;
			for (auto&& entry : bodies)
			{
				Body& body = entry.second;
				const sim::Entity simEntity = static_cast<sim::Entity>(entry.first);
				SimNpc* npc = Resolve(body);
				if (!npc || npc->Destroyed)
				{
					Send(client, BodyReleased{ simEntity, glm::vec3(0.0f) });
					gone.push_back(entry.first);
					continue;
				}
				const glm::vec3 feet = npc->FeetPosition();
				if (body.despawning)
				{
					// Out of sight it goes at once; in sight it fades out first (still walking and reporting, so the sim keeps following it).
					if (!npc->FadingOut() && !HiddenFromCamera(feet))
						npc->FadeOut();
					if (!npc->FadingOut() || npc->FadedOut())
					{
						Logger::Log("sim body: %s goes: %s (%s)", body.label.c_str(), ReasonText(body.reason), npc->FadingOut() ? "faded out" : "out of sight");
						npc->Destroy();
						Send(client, BodyReleased{ simEntity, feet });
						++counters.despawned;
						gone.push_back(entry.first);
						continue;
					}
				}
				if (Seconds(body.lastReport) >= ReportInterval)
				{
					body.lastReport = Clock::now();
					Send(client, BodyReport{ simEntity, feet });
					++counters.reports;
				}
			}
			for (const uint32_t key : gone)
				bodies.erase(key);
		}
	}

	void Update(sim::SimClient& client, const sim::nav::WorldNavData& world)
	{
		if (levelChangeId != Level::CurrentLevelChangeId)
		{
			// The bodies went with the old level; the sim's agents walk on by themselves.
			if (levelChangeId >= 0)
			{
				bodies.clear();
				pending.clear();
				Send(client, BodiesCleared{});
			}
			levelChangeId = Level::CurrentLevelChangeId;
			levelChangedAt = Clock::now();
		}
		if (!Level::Current)
			return;
		HandleOutputs(client);
		HandlePending(client, world);
		HandleBodies(client);
	}

	void Reset()
	{
		bodies.clear();
		pending.clear();
		levelChangeId = -1;
	}

	size_t BodyCount() { return bodies.size(); }

	std::vector<std::pair<sim::Entity, glm::vec3>> BodyPositions()
	{
		std::vector<std::pair<sim::Entity, glm::vec3>> positions;
		for (auto&& entry : bodies)
			if (SimNpc* npc = Resolve(entry.second))
				positions.emplace_back(static_cast<sim::Entity>(entry.first), npc->FeetPosition());
		return positions;
	}

	size_t PendingCount() { return pending.size(); }
	const Counters& GetCounters() { return counters; }
}
