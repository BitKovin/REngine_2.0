#include "Npc.h"

#include <Physics.h>
#include <SoundSystem/FmodEventInstance.h>

#include <imgui/imgui.h>

#include <tracy/tracy/Tracy.hpp>

#include "../Player/Player.hpp"

#include "Director/NpcDirector.h"

#include <algorithm>

namespace
{
	// What a hit of this kind of weapon leaves a body dead of (the weapon entity is what the hit says it was).
	NpcBodyCause CauseOf(const Entity* weapon)
	{
		if (weapon == nullptr) return NpcBodyCause::Unknown;

		const std::string& name = weapon->ClassName;
		if (name == "explosion") return NpcBodyCause::Explosion;
		if (name.rfind("molotov", 0) == 0) return NpcBodyCause::Fire;
		if (name == "bullet") return NpcBodyCause::Gun;
		if (name.rfind("weapon_", 0) == 0) return NpcBodyCause::Blade;
		return NpcBodyCause::Unknown;
	}
}

Npc::Npc() : NpcBase(), body(*this)
{
	ClassName = "npc";
}

// The profile of this NPC and what follows from it. Safe to call again.
void Npc::ResolveProfile()
{
	if (profile != nullptr && profile->id == profileId) return;

	profile = &NpcProfile::Get(profileId);

	npcType = profile->role == "civilian" ? NpcType::Civilian : NpcType::Guard;
	modelPath = profile->model;

	if (fractionTag != profile->faction)
	{
		std::replace(Tags.begin(), Tags.end(), fractionTag, profile->faction);
		fractionTag = profile->faction;
	}

	mind.Setup(profile);
	body.Setup(profile);
}

void Npc::FromData(EntityData data)
{
	NpcBase::FromData(data);

	const std::string requested = data.GetPropertyString("profile");
	if (requested.empty() == false)
		profileId = requested;

	squad = data.GetPropertyString("squad");

	ResolveProfile();
}

void Npc::LoadAssets()
{
	ResolveProfile();
	NpcBase::LoadAssets();
}

void Npc::CreateSkills()
{
	skills.clear();
	currentSkill = nullptr;

	for (const std::string& id : profile->skills)
	{
		if (std::unique_ptr<NpcSkill> skill = CreateNpcSkill(id))
			skills.push_back(std::move(skill));
	}
}

// What the observer sees with (§5.1); its eye and gaze are set every frame by the Body.
void Npc::SetupObserver()
{
	observer = AiPerceptionSystem::CreateObserver(Position + vec3(0.0f, profile->eyeHeight, 0.0f), movingDirection, 90);
	observer->owner = Id;
	observer->ownerPtr = this;
	observer->searchForTriggeredNpc = false;
	observer->ownTarget = observationTarget.get();
	NpcPerceptionTuning::Get().vision.ApplyTo(*observer);
}

void Npc::Start()
{
	ResolveProfile();

	desiredDirection = MathHelper::XZ(MathHelper::GetForwardVector(Rotation));
	movingDirection = desiredDirection;
	desiredLookVector = movingDirection;

	VoiceSoundPlayer = SoundPlayer::Create();
	VoiceSoundPlayer->SetSound(FmodEventInstance::Create("event:/Character/Dialogue"));
	SetupSoundPlayer(VoiceSoundPlayer);

	observationTarget = AiPerceptionSystem::CreateTarget(Position, Id, {});
	observationTarget->noticeMaxDistanceMultiplier = 0.3f;
	observationTarget->npc = true;

	SetupObserver();

	pathFollow.reachedTarget = true;

	StartTask(defaultTask);

	controller = new CharacterController();
	controller->movementQuality = CharacterControllerMovementQuality::NpcGeneric;
	controller->Init(this, Position, 0.45f, 1.8f, 90);

	mesh->Position = Position - vec3(0, controller->height / 2.0f, 0);
	mesh->Rotation = Rotation;

	pathFollow.allowPartialPath = true;

	body.Start();

	context = std::make_unique<NpcContext>(NpcContext{ *this, mind, body, *profile });
	CreateSkills();

	mind.SetSelf(Id, squad);

	NpcDirector::Ensure().Register(this);
}

void Npc::Destroy()
{
	NpcDirector::Unregister(this);
	NpcBase::Destroy();
}

void Npc::OnDamage(float Damage, Entity* DamageCauser, Entity* Weapon)
{
	lastDamageCause = CauseOf(Weapon);

	NpcBase::OnDamage(Damage, DamageCauser, Weapon);

	if (dead)
	{
		AnnounceDeath();
		return;
	}

	PendingHit hit;
	hit.amount = Damage;
	hit.attackerId = DamageCauser ? DamageCauser->Id : std::string();
	hit.origin = DamageCauser ? DamageCauser->Position : Position;

	std::lock_guard<std::mutex> lock(hitMutex);
	pendingHits.push_back(std::move(hit));
}

// A bullet passed close: the same as a hit that did no damage (its shooter is the one to look for).
void Npc::OnNearMiss(Entity* shooter)
{
	if (dead || shooter == nullptr) return;

	PendingHit hit;
	hit.attackerId = shooter->Id;
	hit.origin = shooter->Position;

	std::lock_guard<std::mutex> lock(hitMutex);
	pendingHits.push_back(std::move(hit));
}

void Npc::TakeHits(double now)
{
	std::vector<PendingHit> hits;
	{
		std::lock_guard<std::mutex> lock(hitMutex);
		hits.swap(pendingHits);
	}

	for (const PendingHit& hit : hits)
		mind.OnDamaged(hit.amount, hit.attackerId, hit.origin, now);
}

// What the concerns call for, against what the director has the NPC in. Joining and leaving are requests, answered by the
// time of the next update; a settled concern ends the part it had.
void Npc::SyncSituations(double now)
{
	const NpcDirector* director = NpcDirector::Get();
	if (director == nullptr) return;

	const float lostAfter = NpcPerceptionTuning::Get().alert.targetLostAfter;
	const bool investigates = FindSkill("investigate") != nullptr;
	const bool searches = FindSkill("search") != nullptr;

	std::vector<std::pair<SituationKind, std::string>> wanted;

	for (const NpcConcern& concern : mind.Concerns())
	{
		switch (concern.type)
		{
		case NpcConcernType::Curious:
			if (investigates)
				wanted.emplace_back(SituationKind::Investigate, concern.subject);
			break;

		case NpcConcernType::Alarm:
		case NpcConcernType::Body:
			if (searches)
				wanted.emplace_back(SituationKind::Search, concern.subject);
			break;

		case NpcConcernType::Threat:
		{
			// A hostile in sight is a fight. It becomes a search when it is lost, and stays one while in it.
			const bool lost = mind.Sees(concern.subject) == false && now - mind.LastSeen(concern.subject) > lostAfter;
			if (searches && (lost || director->IsMember(Id, SituationKind::Search, concern.subject)))
				wanted.emplace_back(SituationKind::Search, concern.subject);
			break;
		}
		}
	}

	std::vector<std::pair<SituationKind, std::string>> actual;
	director->MembershipsOf(Id, actual);

	for (const auto& want : wanted)
	{
		if (std::find(actual.begin(), actual.end(), want) == actual.end())
			mind.Request(NpcRequestType::Join, want.first, want.second);
	}

	for (const auto& have : actual)
	{
		if (std::find(wanted.begin(), wanted.end(), have) == wanted.end())
			mind.Request(NpcRequestType::Leave, have.first, have.second);
	}
}

NpcSkill* Npc::FindSkill(const std::string& id) const
{
	for (const auto& skill : skills)
	{
		if (id == skill->Id())
			return skill.get();
	}
	return nullptr;
}

// The skill that wants to run most runs; the running one keeps a small bonus inside its MinCommit (§8).
void Npc::ChooseSkill(double now)
{
	NpcSkill* best = nullptr;
	float bestWant = 0.0f;

	for (const auto& skill : skills)
	{
		float want = skill->Want(*context);

		if (want > 0.0f && skill.get() == currentSkill && now - currentSkillSince < skill->MinCommit())
			want += 0.1f;

		if (want > bestWant)
		{
			bestWant = want;
			best = skill.get();
		}
	}

	if (best == currentSkill) return;

	if (currentSkill)
		currentSkill->Stop(*context);

	currentSkill = best;
	currentSkillSince = now;

	if (currentSkill)
		currentSkill->Start(*context);
}

void Npc::UpdateBrain()
{
	ZoneScopedN("Npc::UpdateBrain");

	const double now = Time::GameTime;
	const float dt = Time::DeltaTimeF;

	context->now = now;
	context->dt = dt;
	context->intent = BodyIntent();

	UpdateUnconscious(now);

	// Out cold: nothing is perceived, decided or said (4.1 rule 4); the body only lies there.
	if (unconscious)
	{
		if (currentSkill)
		{
			currentSkill->Stop(*context);
			currentSkill = nullptr;
		}

		body.Apply(context->intent, dt, now);
		return;
	}

	if (wokenPending)
	{
		wokenPending = false;
		mind.OnWoken(Position, now);
	}

	TakeHits(now);

	// The simulation's schedule picks the task point (the old mechanism, unchanged).
	UpdateScheduledTask();

	{
		ZoneScopedN("Mind");

		mind.ProcessInbox(now);
		mind.Update(*observer, dt, now);

		// Something noticed, heard or felt: the head turns to it (a glance), whatever the skill does.
		for (const NpcMindEvent& event : mind.Events())
		{
			if (event.type != NpcMindEventType::Suspected && event.type != NpcMindEventType::Identified)
				body.Glance(event.position, now);
		}
		mind.ClearEvents();

		SyncSituations(now);
	}

	if (isStunned() == false)
	{
		ZoneScopedN("Skill");

		ChooseSkill(now);

		if (currentSkill)
			currentSkill->Update(*context);
	}

	body.Apply(context->intent, dt, now);

	body.UpdateObserver(*observer, mind.VisionRangeScale());
	observer->traceHead = mind.alert == NpcAlert::Combat;
}

bool Npc::ComputeFacingLock(vec3& lockLookVector)
{
	return body.FacingLock(lockLookVector);
}

// The gun and the head follow the body's intent (§9); the aim pose turns the spine by what the spine can take and the
// head makes up the rest.
void Npc::UpdateWeaponPosture()
{
	const NpcWeaponState weapon = body.Weapon();
	const bool ragdoll = mesh->InRagdoll || returningFromRagdoll || stunnedRagdoll;

	animator->weapon_holds = ragdoll == false && weapon != NpcWeaponState::Holstered;
	animator->weapon_ready = ragdoll == false && (weapon == NpcWeaponState::Ready || weapon == NpcWeaponState::Aimed);
	animator->weapon_aims = ragdoll == false && weapon == NpcWeaponState::Aimed;
	animator->scared = false;

	float yaw = body.GazeYaw();
	float pitch = body.GazePitch();

	if (animator->weapon_aims)
	{
		const float spineYaw = std::clamp(yaw, -30.0f, 30.0f);
		animator->spineRotation = vec3(pitch, spineYaw, 0.0f);
		yaw -= spineYaw;
		pitch = 0.0f;
	}
	else
	{
		animator->spineRotation = vec3();
	}

	animator->gazeYaw = yaw;
	animator->gazePitch = pitch;
}

void Npc::LateUpdate()
{
	NpcBase::LateUpdate();

	if (dead)
		AnnounceDeath();

	// A snapshot for the HUD: the async update is over, so nothing writes the Mind while the HUD reads this.
	hudMeters.clear();

	if (dead) return;

	for (const NpcMeter& meter : mind.Meters())
	{
		HudMeter entry;
		entry.subject = meter.subject;
		entry.value = meter.value;
		hudMeters.push_back(std::move(entry));
	}

	hudPosition = body.EyePosition();
	hudAlert = mind.alert;
}

// ── Bodies (13.1) ───────────────────────────────────────────────────────────────────────────────────────────────

NpcBodyInfo Npc::GetBodyInfo() const
{
	NpcBodyInfo info;
	info.hidden = bodyHidden;

	if (dead)
	{
		info.state = NpcBodyState::Dead;
		info.cause = deathCause;
		info.since = deathTime;
	}
	else if (unconscious)
	{
		info.state = NpcBodyState::Unconscious;
		info.since = unconsciousSince;
	}
	else
	{
		info.state = NpcBodyState::Alive;
	}

	return info;
}

// The first time the NPC is seen dead, from whatever killed it: the time, the cause, and the thud (6 m, it may be heard).
void Npc::AnnounceDeath()
{
	if (deathTime >= 0.0) return;

	deathTime = Time::GameTime;
	deathCause = lastDamageCause;
	unconscious = false;

	NpcSounds::Emit(NpcSoundType::BodyFall, Position, "fall:" + Id);
}

void Npc::KnockOut()
{
	if (dead || unconscious) return;

	unconscious = true;
	unconsciousSince = Time::GameTime;
	wakeAt = unconsciousSince + NpcPerceptionTuning::Get().bodies.wakeTime;

	StartStunnedRagdoll();
	NpcSounds::Emit(NpcSoundType::BodyFall, Position, "fall:" + Id);
}

void Npc::WakeUp()
{
	if (dead || unconscious == false) return;

	unconscious = false;
	wokenPending = true;

	// The ragdoll holds as long as the delay of the stun runs: let it go, and the NPC stands up.
	stunnedRagdollDelay.AddDelay(0.0);
}

void Npc::Kill(NpcBodyCause cause)
{
	if (dead) return;

	lastDamageCause = cause;
	NpcBase::OnDamage(Health + 1.0f, nullptr, nullptr);
	AnnounceDeath();
}

// While the NPC is out cold the ragdoll is kept down, and the timer wakes it (a woken NPC is told on its own thread).
void Npc::UpdateUnconscious(double now)
{
	if (unconscious == false) return;

	if (now >= wakeAt)
	{
		unconscious = false;
		wokenPending = true;
		stunnedRagdollDelay.AddDelay(0.0);
		return;
	}

	stunnedRagdollDelay.AddDelay(1.0);
}

// The observation target of a body is what the others find: the pelvis, with its state as tags, as hard to see as the light
// makes it, and not there at all in a hiding spot. A living NPC is a target as before, with its squad on it.
void Npc::UpdateObservationTarget()
{
	NpcBase::UpdateObservationTarget();

	const NpcBodyInfo info = GetBodyInfo();

	if (info.state == NpcBodyState::Alive)
	{
		if (squad.empty() == false)
			observationTarget->tags.insert("squad:" + squad);
		return;
	}

	observationTarget->tags.clear();
	observationTarget->tags.insert("body");
	observationTarget->tags.insert(info.state == NpcBodyState::Dead ? "dead" : "unconscious");
	observationTarget->tags.insert(fractionTag);

	if (Body* pelvis = mesh->FindHitboxByName("pelvis"))
		observationTarget->position = FromPhysics(pelvis->GetPosition()) + vec3(0.0f, 0.5f, 0.0f);

	observationTarget->active = true;
	observationTarget->visibility = info.hidden ? 0.0f : 1.0f;
	observationTarget->moving = false;
	observationTarget->motion = TargetMotion::Still;
	observationTarget->noticeMaxDistanceMultiplier = NpcPerceptionTuning::Get().bodies.noticeMultiplier;
	observationTarget->sampleLight = true;
}

bool Npc::Speak(const std::string& eventPath)
{
	if (VoiceSoundPlayer == nullptr) return false;

	const auto sound = FmodEventInstance::Create(eventPath);
	if (sound == nullptr) return false;

	VoiceSoundPlayer->Stop();
	VoiceSoundPlayer->SetSound(sound);
	VoiceSoundPlayer->Play();
	return true;
}

// ── Save and load ───────────────────────────────────────────────────────────────────────────────────────────────

void Npc::Serialize(json& target)
{
	NpcBase::Serialize(target);

	const double now = Time::GameTime;

	json brain;
	brain["profile"] = profileId;
	brain["mind"] = mind.Serialize(now);
	brain["skill"] = currentSkill ? currentSkill->Id() : "";

	json bodyState;
	bodyState["unconscious"] = unconscious;
	bodyState["unconsciousAge"] = now - unconsciousSince;
	bodyState["wakeIn"] = wakeAt - now;
	bodyState["woken"] = wokenPending;
	bodyState["hidden"] = bodyHidden;
	bodyState["deathAge"] = deathTime < 0.0 ? -1.0 : now - deathTime;
	bodyState["deathCause"] = deathCause;
	brain["body"] = std::move(bodyState);

	json skillStates = json::object();
	for (const auto& skill : skills)
		skillStates[skill->Id()] = skill->Serialize(now);
	brain["skills"] = std::move(skillStates);

	target["npcBrain"] = std::move(brain);
}

void Npc::Deserialize(json& source)
{
	NpcBase::Deserialize(source);

	if (source.contains("npcBrain") == false) return;

	const json& brain = source["npcBrain"];
	const double now = Time::GameTime;

	const json bodyState = brain.value("body", json::object());
	unconscious = bodyState.value("unconscious", false);
	unconsciousSince = now - bodyState.value("unconsciousAge", 1.0e9);
	wakeAt = now + bodyState.value("wakeIn", 0.0);
	wokenPending = bodyState.value("woken", false);
	bodyHidden = bodyState.value("hidden", false);
	const double deathAge = bodyState.value("deathAge", -1.0);
	deathTime = deathAge < 0.0 ? -1.0 : now - deathAge;
	deathCause = bodyState.value("deathCause", NpcBodyCause::Unknown);

	if (dead) return;

	mind.Deserialize(brain.value("mind", json::object()), now);

	if (brain.contains("skills"))
	{
		for (const auto& skill : skills)
		{
			if (brain["skills"].contains(skill->Id()))
				skill->Deserialize(brain["skills"][skill->Id()], now);
		}
	}

	// The skill that was running continues where it was: no Start, its state is what was saved.
	currentSkill = FindSkill(brain.value("skill", std::string()));
	currentSkillSince = now - 10.0;
}

// ── Debug panel: look at an NPC ─────────────────────────────────────────────────────────────────────────────────

void Npc::UpdateDebugUI()
{
	if (dead || context == nullptr) return;

	const auto hit = Physics::LineTrace(Camera::position, Camera::position + Camera::Forward() * 500.0f, BodyType::CharacterCapsule, {}, { Player::Instance });
	if (hit.entity != this) return;

	const double now = Time::GameTime;

	ImGui::Begin("look at npc (new brain)");

	ImGui::Text("%s  profile %s", Id.c_str(), profileId.c_str());
	ImGui::Text("alert: %s   fear: %.2f", NpcAlertName(mind.alert), mind.fear);
	ImGui::Text("skill: %s", currentSkill ? currentSkill->Id() : "none");

	if (currentSkill)
		ImGui::Text("  %s", currentSkill->Describe(now).c_str());

	ImGui::Text("task: %s%s", taskState.TaskName.c_str(), DoingTask ? "" : " (paused)");
	ImGui::Text("gaze: yaw %.0f pitch %.0f%s", body.GazeYaw(), body.GazePitch(), body.Glancing(now) ? "  (glance)" : "");

	ImGui::Separator();
	ImGui::Text("awareness");

	for (const NpcMeter& meter : mind.Meters())
	{
		ImGui::ProgressBar(meter.value, ImVec2(120.0f, 0.0f));
		ImGui::SameLine();
		ImGui::Text("%s (%s)", meter.subject.c_str(), meter.source == NpcRecordSource::Saw ? "saw" : "heard");
	}

	ImGui::Separator();
	ImGui::Text("concerns");

	for (const NpcConcern& concern : mind.Concerns())
	{
		const char* type = concern.type == NpcConcernType::Threat ? "threat" : (concern.type == NpcConcernType::Alarm ? "alarm" : "curious");
		ImGui::Text("%s  %s  age %.1fs", type, concern.subject.c_str(), now - concern.time);
	}

	if (const NpcDirector* director = NpcDirector::Get())
	{
		std::vector<std::pair<SituationKind, std::string>> parts;
		director->MembershipsOf(Id, parts);

		for (const auto& part : parts)
		{
			NpcOrder order;
			std::string subject;
			NpcDirector::FindOrder(Id, part.first, order, subject);
			ImGui::Text("%s of %s: %s", SituationKindName(part.first), part.second.c_str(), NpcRoleName(order.role));
		}
	}

	ImGui::Separator();
	ImGui::Text("memory");

	for (const NpcRecord& record : mind.memory.Records())
	{
		const char* source = record.source == NpcRecordSource::Saw ? "saw" : (record.source == NpcRecordSource::Heard ? "heard" : (record.source == NpcRecordSource::Told ? "told" : "felt"));
		ImGui::Text("%s  %s  age %.1fs  certainty %.2f  %s", record.subject.c_str(), source, now - record.time, record.certainty, record.detail.c_str());
	}

	ImGui::End();
}

REGISTER_ENTITY(Npc, "npc")
