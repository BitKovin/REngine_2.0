#pragma once

// Data the NPC brain plays by (NPC_AI_DESIGN.md §19): perception and awareness tuning
// (GameData/ai/perception.json) and one profile per role (GameData/ai/roles/<role>.json).
// Every number has a default here, so a missing file or a missing key changes nothing but the data.

#include <Helpers/JsonHelper.hpp>
#include <Helpers/StringHelper.h> // the global `json` alias

#include <array>
#include <map>
#include <string>
#include <vector>

class Observer;

// How loud a kind of sound is and what it means to a listener. Names are the keys of "sounds" in perception.json.
struct NpcSoundTuning
{
	float radius = 10.0f;            // metres at which it is just heard (EmitSound loudness)
	std::string concern = "curious"; // "curious": adds awareness, may be investigated; "alarm": something is wrong, go there
	float suspicion = 0.5f;          // how much awareness it adds per unit of loudness (design 6.2: loudness x 0.5 x this)

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(NpcSoundTuning, radius, concern, suspicion)
};

struct NpcVisionTuning
{
	float focusRange = 25.0f;
	float focusHalfAngle = 30.0f;
	float focusWideRange = 5.0f;
	float focusWideHalfAngle = 60.0f;
	float peripheralRange = 8.0f;
	float peripheralHalfAngle = 100.0f;
	float peripheralFactor = 0.35f;
	float nearRange = 1.5f;
	float nearFactor = 0.6f;
	float hysteresis = 0.1f;
	float closeDistance = 6.0f;
	float farFactor = 0.2f;
	float darkFactor = 0.6f;
	float litLevel = 0.5f;

	// Range multiplier per alert level (Calm, Wary, Alarmed, Combat): the focus range is 25 m calm and 40 m in combat.
	std::array<float, 4> rangeScale = { 1.0f, 1.0f, 1.2f, 1.6f };

	// Gives an observer these eyes (the vision profile of every NPC class) and turns the profile on.
	// The owner sets VisionProfile::rangeScale from its state each frame.
	void ApplyTo(Observer& observer) const;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(NpcVisionTuning, focusRange, focusHalfAngle, focusWideRange, focusWideHalfAngle,
		peripheralRange, peripheralHalfAngle, peripheralFactor, nearRange, nearFactor, hysteresis, closeDistance, farFactor,
		darkFactor, litLevel, rangeScale)
};

struct NpcMeterTuning
{
	float fillRate = 0.6f;     // per second at strength 1, motion 1, state 1, acuity 1, suspicion 1
	float notice = 0.35f;      // glance, "Hm?"
	float suspect = 0.7f;      // investigate or confront
	float identify = 1.0f;     // react to what it is
	float hold = 1.5f;         // seconds a meter stays after the last stimulus
	float decay = 0.2f;        // per second afterwards
	float decayAlerted = 0.1f; // per second while Alarmed or in Combat
	float soundFill = 0.5f;    // a sound adds loudness x this x the sound's suspicion

	// Per kind of motion of the subject.
	float motionStill = 0.7f;
	float motionWalk = 1.0f;
	float motionRun = 1.6f;
	float motionCrouchWalk = 0.6f;
	float motionCrouchStill = 0.4f;

	// Per alert level (Calm, Wary, Alarmed, Combat).
	std::array<float, 4> stateMul = { 1.0f, 1.5f, 2.0f, 6.0f };

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(NpcMeterTuning, fillRate, notice, suspect, identify, hold, decay, decayAlerted, soundFill,
		motionStill, motionWalk, motionRun, motionCrouchWalk, motionCrouchStill, stateMul)
};

struct NpcAlertTuning
{
	float combatToAlarmed = 8.0f; // seconds without a fresh sighting of a hostile
	float wearyToCalm = 60.0f;    // Wary -> Calm after this long without a new concern
	float eventLifetime = 60.0f;  // an event record (a noise) is forgotten after this long
	float leadMaxAge = 90.0f;     // a lead older than this is not followed
	float targetLostAfter = 0.5f; // a hostile that is not sensed for this long is lost (the chase starts)

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(NpcAlertTuning, combatToAlarmed, wearyToCalm, eventLifetime, leadMaxAge, targetLostAfter)
};

struct NpcFearTuning
{
	float gunfireNear = 0.1f;   // an alarm sound this loud or louder at the listener
	float heavyDamage = 0.2f;   // damage of at least heavyDamageThreshold
	float heavyDamageThreshold = 20.0f;
	float safetyDecay = 0.05f;  // per second
	float bodySeen = 0.15f;     // an ally's body is found
	float bodyDrained = 0.3f;   // ... and it was fed upon

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(NpcFearTuning, gunfireNear, heavyDamage, heavyDamageThreshold, safetyDecay, bodySeen, bodyDrained)
};

// Bodies as things to be found (13.1).
struct NpcBodiesTuning
{
	float noticeMultiplier = 1.0f;   // how easy a body is to notice (the target of a living NPC counts as 3.3 times as far)
	float fillMotion = 1.0f;         // stands in for the motion factor of the meter: a body does not move
	float suspicion = 1.0f;          // a body is what it is at once (6.3)
	float darkLevel = 0.25f;         // light at the body below this is dark...
	float darkSeenDistance = 4.0f;   // ...and a body in the dark is seen from this close only
	float wakeTime = 45.0f;          // an unconscious NPC stands up on its own after this long

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(NpcBodiesTuning, noticeMultiplier, fillMotion, suspicion, darkLevel, darkSeenDistance, wakeTime)
};

// Doors and items that change while nobody is looking (13.3).
struct NpcObjectsTuning
{
	float settleTime = 6.0f;         // a new state counts once it has held this long (a door an NPC just walked through does not)
	float changedLifetime = 30.0f;   // the event of a changed object stays this long

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(NpcObjectsTuning, settleTime, changedLifetime)
};

// A squadmate that was around and is not any more (13.3, after Splinter Cell Blacklist).
struct NpcMissingTuning
{
	float earshot = 15.0f;           // a squadmate this near counts as in earshot
	float companyTime = 10.0f;       // seconds in earshot before its silence means anything
	float silentTime = 5.0f;         // seconds without a sign of it
	float areaRadius = 12.0f;        // the NPC must still be this near to where it last sensed the squadmate
	float cooldown = 60.0f;          // per squadmate

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(NpcMissingTuning, earshot, companyTime, silentTime, areaRadius, cooldown)
};

// GameData/ai/perception.json
struct NpcPerceptionTuning
{
	NpcVisionTuning vision;
	NpcMeterTuning meter;
	NpcAlertTuning alert;
	NpcFearTuning fear;
	NpcBodiesTuning bodies;
	NpcObjectsTuning objects;
	NpcMissingTuning missing;
	std::map<std::string, NpcSoundTuning> sounds;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(NpcPerceptionTuning, vision, meter, alert, fear, bodies, objects, missing, sounds)

	// Loads the file once (later calls do nothing); defaults when it is missing.
	static void Load();
	static const NpcPerceptionTuning& Get();

	const NpcSoundTuning& SoundFor(const std::string& name) const;
};

struct NpcInvestigateTuning
{
	float reactionTime = 0.3f;   // seconds from the glance to the first step
	float postMin = 2.0f;        // the spot to look from is this far from the point...
	float postMax = 4.0f;        // ...to this far, with a view of it
	float lookAroundTime = 3.0f;
	float lookAroundYaw = 70.0f; // degrees each side of the point the head sweeps
	float giveUpAfter = 30.0f;   // seconds before a lead is dropped
	float arriveRadius = 1.2f;
	float holdDistance = 5.0f;   // closing on a hostile it can see: stops here until it can fight (M3)
	float speed = 2.2f;          // pace of a curious investigation
	float threatSpeed = 4.5f;    // closing on a hostile
	float retargetDistance = 2.5f; // news of the same subject this far from the point being checked re-plans the approach
	float backupExtraDistance = 3.0f; // the backup covers from this much farther than the investigator looks from

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(NpcInvestigateTuning, reactionTime, postMin, postMax, lookAroundTime, lookAroundYaw,
		giveUpAfter, arriveRadius, holdDistance, speed, threatSpeed, retargetDistance, backupExtraDistance)
};

// How one NPC moves and looks while it chases and searches (NPC_AI_DESIGN.md 11.3 to 11.5).
struct NpcPursuitTuning
{
	float chaseSpeed = 4.5f;     // running to where the chase flood points
	float chaseLookTime = 2.0f;  // the look-around on arrival without contact
	float chaseLookYaw = 70.0f;
	float searchSpeed = 2.6f;    // walking to a claimed cell, a post or the original spot
	float searchLookYaw = 50.0f; // sweep of the head round the frontier gaze while standing
	float searchLookPeriod = 3.0f;
	float arriveRadius = 1.5f;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(NpcPursuitTuning, chaseSpeed, chaseLookTime, chaseLookYaw, searchSpeed, searchLookYaw,
		searchLookPeriod, arriveRadius)
};

// GameData/ai/situations.json: how NPCs tell each other and how a group plans around one subject (NPC_AI_DESIGN.md 7, 10, 11).

// One kind of message: how far it carries and how long it takes to get there.
struct NpcMessageKindTuning
{
	float radius = 15.0f;  // metres (shout: walls count like they do for sounds; report and radio: straight distance)
	float delay = 0.0f;    // seconds from speaking to being heard: the sender must still be able to act when it ends

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(NpcMessageKindTuning, radius, delay)
};

struct NpcMessageTuning
{
	NpcMessageKindTuning shout{ 15.0f, 0.4f };
	NpcMessageKindTuning report{ 2.5f, 2.0f };
	NpcMessageKindTuning radio{ 100.0f, 1.0f };
	float toldCertainty = 0.8f;   // a record told on keeps this part of the teller's certainty
	float shoutCooldown = 6.0f;   // an NPC shouts about the same subject at most this often
	float reportCooldown = 20.0f; // an NPC tells the same squadmate what it knows at most this often
	NpcMessageKindTuning alarm{ 60.0f, 0.0f }; // the area of an alarm point without a radius of its own; the bell is heard at once

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(NpcMessageTuning, shout, report, radio, toldCertainty, shoutCooldown, reportCooldown, alarm)
};

struct NpcInvestigateRolesTuning
{
	int investigators = 1;
	int backupsWhenWary = 1; // extra NPCs that cover the investigator when somebody in the situation is Wary or worse

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(NpcInvestigateRolesTuning, investigators, backupsWhenWary)
};

// The chase flood (11.3).
struct NpcChaseTuning
{
	float heat = 20.0f;             // heat at the last known cell
	float heatFalloff = 0.97f;      // heat kept per step away from it
	int maxHeatedPerStep = 25;      // more cells than this heating in one step: an open room, the flood stops
	int maxSteps = 30;
	int stepsPerFrame = 10;         // the flood runs over three frames
	float barrierDot = -0.3f;       // neighbours whose direction to the last velocity is below this are behind it
	float minVelocity = 0.5f;       // m/s: below this there is no behind
	float chaseWindow = 4.0f;       // a hostile last seen longer ago than this is searched for, not chased
	int maxReruns = 3;              // new evidence re-runs the flood this many times in one chase
	float rerunDistance = 3.0f;     // ... when it is this far from where the flood started
	float maxTime = 40.0f;          // a chase that has not ended after this long turns into the search

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(NpcChaseTuning, heat, heatFalloff, maxHeatedPerStep, maxSteps, stepsPerFrame, barrierDot,
		minVelocity, chaseWindow, maxReruns, rerunDistance, maxTime)
};

// The search map (11.4) and what the searchers do with it.
struct NpcSearchTuning
{
	int maxCells = 3000;
	float spreadSpeed = 1.9f;        // m/s the probability moves outward: the walk speed of the hidden one
	float spreadInterval = 0.25f;    // seconds between spreading steps
	float uncertaintyRadius = 6.0f;  // metres the first probability is spread over when the certainty is 0
	int raysPerFrame = 64;
	float clearHeight = 1.0f;        // metres above the floor a cell is looked at
	float visibleHold = 2.0f;        // a cell stays seen this long after the last ray reached it
	float endMass = 0.05f;           // the search is over below this part of the probability it began with
	float timeout = 90.0f;           // ... or after this long
	float claimInterval = 1.0f;      // seconds between claims being checked
	float claimSpacing = 5.0f;       // searchers claim cells at least this far apart
	float claimDwell = 1.5f;         // a searcher that has stood at its claim this long has looked there
	int maxWatchers = 2;
	float watchRadius = 8.0f;        // a choke is worth watching for the probability within this reach of it
	float watchSpacing = 6.0f;
	float frontierInterval = 0.5f;   // seconds between gaze points being chosen
	float gazeRange = 25.0f;
	float saveRelative = 0.01f;      // a saved search map keeps the cells above this part of its peak

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(NpcSearchTuning, maxCells, spreadSpeed, spreadInterval, uncertaintyRadius, raysPerFrame,
		clearHeight, visibleHold, endMass, timeout, claimInterval, claimSpacing, claimDwell, maxWatchers, watchRadius, watchSpacing, frontierInterval,
		gazeRange, saveRelative)
};

// A search round a body (13.1): the killer is more likely close to a fresh kill.
struct NpcBodySearchTuning
{
	float freshAge = 20.0f;        // a kill younger than this is fresh: the map is spread over the whole `radiusFresh`
	float oldAge = 120.0f;         // one older than this gets `radiusOld`; in between the radius shrinks evenly
	float radiusFresh = 12.0f;
	float radiusOld = 5.0f;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(NpcBodySearchTuning, freshAge, oldAge, radiusFresh, radiusOld)
};

// What an event leaves in the area it happened in (6.4): the lowest alert level its NPCs go back to. `level` holds for
// `duration` seconds (negative: for the rest of the level), then `then` does.
struct NpcFloorRule
{
	std::string level = "calm";
	float duration = -1.0f;
	std::string then = "calm";

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(NpcFloorRule, level, duration, then)
};

struct NpcFloorsTuning
{
	float radius = 30.0f;          // the circle round the event, when no zone gives the area (zones are M5)
	NpcFloorRule noise;            // minor scares leave nothing: noise, curiosity, a trespass warning
	NpcFloorRule bodyFound{ "wary", -1.0f, "wary" };
	NpcFloorRule fight{ "wary", -1.0f, "wary" };
	NpcFloorRule power{ "wary", -1.0f, "wary" };
	NpcFloorRule alarm{ "alarmed", 180.0f, "wary" };

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(NpcFloorsTuning, radius, noise, bodyFound, fight, power, alarm)
};

// Alarm points (17.11): who runs, how long ringing takes, how well the area is told where the intruder is.
struct NpcAlarmTuning
{
	float ringTime = 2.0f;         // seconds at the alarm point before it rings
	float arriveRadius = 1.2f;     // how near to the point the runner has to be
	float maxRunDistance = 60.0f;  // a guard farther from the point than this does not run for it
	float roughRadius = 10.0f;     // the area is told a position this far off at most...
	float roughMinFraction = 0.5f; // ...and at least this part of that
	float roughCertainty = 0.4f;   // what the area makes of it (the search is spread wider the lower it is)

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(NpcAlarmTuning, ringTime, arriveRadius, maxRunDistance, roughRadius, roughMinFraction, roughCertainty)
};

struct NpcSituationTuning
{
	NpcMessageTuning messages;
	NpcInvestigateRolesTuning investigate;
	NpcChaseTuning chase;
	NpcSearchTuning search;
	NpcBodySearchTuning bodySearch;
	NpcFloorsTuning floors;
	NpcAlarmTuning alarm;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(NpcSituationTuning, messages, investigate, chase, search, bodySearch, floors, alarm)

	// Loads the file once (later calls do nothing); defaults when it is missing.
	static void Load();
	static const NpcSituationTuning& Get();
};

// How a guard looks after a body (13.1).
struct NpcTendTuning
{
	float deadTime = 2.0f;         // crouched at a dead or drained body, reading what killed it
	float unconsciousTime = 3.0f;  // crouched at an unconscious one, waking it
	float speed = 2.4f;            // pace of the walk to it
	float arriveRadius = 1.0f;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(NpcTendTuning, deadTime, unconsciousTime, speed, arriveRadius)
};

// How the body turns (§9.2).
struct NpcBodyTuning
{
	float headYawLimit = 70.0f;    // degrees the head turns from the body; beyond that the body turns
	float headPitchLimit = 45.0f;
	float headSpeed = 420.0f;      // degrees per second
	float turnRateStand = 240.0f;  // body turn, standing
	float turnRateWalk = 120.0f;   // body turn, walking

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(NpcBodyTuning, headYawLimit, headPitchLimit, headSpeed, turnRateStand, turnRateWalk)
};

// GameData/ai/roles/<id>.json
struct NpcProfile
{
	std::string id = "guard";
	std::string role = "guard";                        // "guard" or "civilian": which NpcType the legacy code sees
	std::string model = "GameData/models/npc/guard.glb";
	std::string faction = "citizen";                   // the legacy fraction tag until the faction table (M5)
	std::string voice = "Guard1";                      // FMOD voice folder
	std::vector<std::string> skills = { "investigate", "routine" };

	float acuity = 1.0f;             // multiplies how fast awareness fills
	float defaultSuspicion = 1.0f;   // how suspicious the player looks to this role until factions and zones (M5)
	float walkSpeed = 1.9f;          // routine walking pace
	float runSpeed = 5.0f;
	float glanceTime = 1.0f;         // seconds the head stays on something that was noticed
	float eyeHeight = 0.7f;          // above the body centre
	bool armed = true;               // carries a gun: held when the NPC is wary or worse

	bool radio = false;              // can call its squad by radio (officers, hunters, organised factions: NPC_AI_DESIGN.md 7)
	bool reports = true;             // tells a squadmate it meets what it knows (NPC_AI_DESIGN.md 7)
	bool alarmRunner = false;        // runs to an alarm point when it has seen an intruder (NPC_AI_DESIGN.md 17.11)

	NpcBodyTuning body;
	NpcInvestigateTuning investigate;
	NpcPursuitTuning pursuit;
	NpcTendTuning tend;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(NpcProfile, id, role, model, faction, voice, skills, acuity, defaultSuspicion, walkSpeed,
		runSpeed, glanceTime, eyeHeight, armed, radio, reports, alarmRunner, body, investigate, pursuit, tend)

	// The profile of a role id, loaded on first use (a default profile when the file is missing).
	static const NpcProfile& Get(const std::string& id);
};

// GameData/ai/barks.json: what NPCs say and how the voices are kept from talking over each other (18.2).
struct NpcBarkTuning
{
	struct Bark
	{
		float priority = 10.0f;      // the most important of the barks asked for in a frame is the one that is said

		NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Bark, priority)
	};

	float situationSpacing = 2.0f;   // one bark per situation per this long
	float npcSpacing = 1.5f;         // and one per speaker
	float overlapRadius = 10.0f;     // nobody else starts talking this near to a voice...
	float overlapTime = 2.0f;        // ...that began less than this long ago
	int globalCap = 3;               // barks that may start within `globalWindow`
	float globalWindow = 1.0f;
	std::array<int, 2> tierUses = { 2, 6 }; // uses of a bark in a level: up to the first number the specific line (_t1), up to the second the generic (_t2), then _t3
	std::map<std::string, Bark> barks;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(NpcBarkTuning, situationSpacing, npcSpacing, overlapRadius, overlapTime, globalCap,
		globalWindow, tierUses, barks)

	static void Load();
	static const NpcBarkTuning& Get();

	// The FMOD event of a bark said by a voice: event:/Character/<Voice>/<bark>_t<tier>.
	static std::string EventFor(const std::string& voice, const std::string& bark, int tier);

	// 1..3 for the use-th time (counting from 0) a bark is said in a level.
	int TierForUse(int use) const;

	float PriorityOf(const std::string& bark) const;
};
