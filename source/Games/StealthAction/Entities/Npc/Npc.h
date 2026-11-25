#pragma once

// The new NPC (NPC_AI_DESIGN.md): one class for every role, picked by a profile (GameData/ai/roles/<id>.json).
// NpcBase keeps the body (damage, ragdoll, animation, task points, doors, the controller); this class replaces the
// brain: Mind (what it perceives and makes of it) -> Skills (what it does) -> Body (how it moves and looks).
//
// Map entity "npc": keyvalues `profile` (role id, default "guard"), `target` (start task point, as the old NPCs) and `squad`
// (NPCs with the same squad name know each other: one that was around and goes missing is looked for).

#include "NpcBase.h"

#include "Brain/NpcBody.h"
#include "Brain/NpcBodyInfo.h"
#include "Brain/NpcData.h"
#include "Brain/NpcMind.h"
#include "Brain/NpcSkill.h"

#include <memory>
#include <mutex>
#include <string>
#include <vector>

class NpcDirector;

class Npc : public NpcBase
{
	friend class NpcDirector; // collects the outbox of the Mind and fills its inbox (main thread, between updates)

public:

	Npc();

	void FromData(EntityData data);
	void Start();
	void LateUpdate();

	void Destroy();

	void Serialize(json& target);
	void Deserialize(json& source);

	// Whether the NPC takes part in anything: it perceives, sends, holds roles. A dead, unconscious or destroyed one does not
	// (§4.1 rule 4).
	bool CanAct() const { return dead == false && Destroyed == false && unconscious == false; }

	// ── Bodies (§13.1) ──────────────────────────────────────────────────────────────────────────────────────────────
	// What lies there when the NPC is dead or out cold, as whoever finds it can tell. The hooks below are what the player's
	// verbs (takedowns, feeding, hiding places: later) call; nothing else changes an NPC's body state.

	NpcBodyInfo GetBodyInfo() const;
	bool IsUnconscious() const { return unconscious; }

	// Knocked out: down and out of everything for `bodies.wakeTime` seconds, or until woken.
	void KnockOut();

	// Shaken awake (a guard tending it, a verb): it stands up, alarmed, and remembers where it was and what it had seen.
	void WakeUp();

	// Dies of a cause that is not a hit (feeding).
	void Kill(NpcBodyCause cause);

	// In a hiding spot: nobody sees the body there.
	void SetBodyHidden(bool hidden) { bodyHidden = hidden; }

	// Says a line through the voice player (main thread). False when the bank has no such event: nothing is played then.
	bool Speak(const std::string& eventPath);

	void UpdateDebugUI();

	// Damage and near misses arrive on the main thread and are handed to the Mind on the worker thread.
	void OnDamage(float Damage, Entity* DamageCauser = nullptr, Entity* Weapon = nullptr) override;
	void UpdateObservationTarget() override;
	bool ShowsAwareness() const override { return dead == false && unconscious == false; }
	void OnNearMiss(Entity* shooter) override;

	// The old investigation system is not used: other code that still starts one on an NPC changes nothing here.
	void TryStartInvestigation(InvestigationReason reason, vec3 target, string causer, bool sharedByNpc = false) override {}

	// ── For the HUD (main thread, valid after LateUpdate) ──────────────────────────────────────────────────────────

	struct HudMeter
	{
		std::string subject;
		float value = 0.0f;
	};

	const std::vector<HudMeter>& GetHudMeters() const { return hudMeters; }

	void GetAwareness(std::vector<AwarenessReading>& out) const override
	{
		for (const HudMeter& meter : hudMeters)
		{
			AwarenessReading reading;
			reading.subject = meter.subject;
			reading.value = meter.value;
			out.push_back(reading);
		}
	}

	vec3 GetAwarenessAnchor() const override { return hudPosition; }
	vec3 GetHudPosition() const { return hudPosition; }
	NpcAlert GetAlert() const { return hudAlert; }
	bool IsAlive() const { return dead == false; }

	// The Mind, the body, the running skill and the profile as they are after the last update (the automated tests read them:
	// Debug/AiTests, which also queues messages through TestMind).
	const NpcMind& GetMind() const { return mind; }
	NpcMind& TestMind() { return mind; }
	const NpcBody& GetBody() const { return body; }
	const NpcProfile& GetProfile() const { return *profile; }
	const char* CurrentSkillId() const { return currentSkill ? currentSkill->Id() : ""; }
	std::string CurrentSkillDescription(double now) const { return currentSkill ? currentSkill->Describe(now) : std::string(); }

protected:

	void LoadAssets();
	void UpdateBrain() override;
	bool ComputeFacingLock(vec3& lockLookVector) override;
	void UpdateWeaponPosture() override;
	void UpdateObserver() override {} // the Body places the eye (NpcBody::UpdateObserver)

	std::shared_ptr<CombatBehaviorBase> CreateCombatBehavior() override { return nullptr; }

private:

	std::string profileId = "guard";
	std::string squad;                       // map key: NPCs of one squad know each other
	const NpcProfile* profile = nullptr;

	NpcMind mind;
	NpcBody body;
	std::unique_ptr<NpcContext> context;

	std::vector<std::unique_ptr<NpcSkill>> skills;
	NpcSkill* currentSkill = nullptr;
	double currentSkillSince = 0.0;

	struct PendingHit
	{
		float amount = 0.0f;
		std::string attackerId;
		vec3 origin = vec3();
	};

	std::mutex hitMutex;
	std::vector<PendingHit> pendingHits;

	// Body state: only the hooks above and the update below change it.
	bool unconscious = false;
	double unconsciousSince = -1.0e9;
	double wakeAt = 0.0;
	bool wokenPending = false;               // woken: the Mind is told on its own thread
	bool bodyHidden = false;
	double deathTime = -1.0;                 // game time it was first seen dead; negative while it lives
	NpcBodyCause deathCause = NpcBodyCause::Unknown;
	NpcBodyCause lastDamageCause = NpcBodyCause::Unknown;

	std::vector<HudMeter> hudMeters;
	vec3 hudPosition = vec3();
	NpcAlert hudAlert = NpcAlert::Calm;

	void ResolveProfile();
	void SetupObserver();
	void CreateSkills();

	void TakeHits(double now);
	void AnnounceDeath();                    // the first time it is seen dead: when, and the sound it makes falling
	void UpdateUnconscious(double now);
	void ChooseSkill(double now);
	NpcSkill* FindSkill(const std::string& id) const;

	// Asks the director to take the NPC into the situations its concerns call for and to let it go of the others.
	void SyncSituations(double now);
};
