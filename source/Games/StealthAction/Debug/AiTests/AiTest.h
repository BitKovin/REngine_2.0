#pragma once

// Automated AI tests (-ai_test). They run inside the real game executable: a real level, the real player entity moved
// by a scripted puppet, the real perception and AI, stepped with a fixed time step much faster than real time.
//
//   StealthAction.exe -headless -no_map -ai_test [filter ...] [-ai_runs N]
//
// A filter is a scenario name, a tier ("sense", "chase", "squad") or "all" (the default).
// The results go to SaveData/ai_tests/: summary.log, result.json and one trace_<scenario>.csv per scenario.
// run_ai_tests.ps1 (next to this file) is the wrapper that also sets the process exit code.

#ifndef DISTRIBUTION

#include <glm.h>

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

class Entity;
class NpcBase;
class Player;
enum class NpcSoundType : int;

namespace AiTest
{
	constexpr float StepSeconds = 1.0f / 30.0f;

	// How the puppet looks and sounds to the NPCs.
	enum class Posture
	{
		Still,
		Walk,
		Run,
		CrouchWalk,
		CrouchStill
	};

	const char* PostureName(Posture posture);

	// One NPC at one moment (what a trace row and most checks read).
	struct Snapshot
	{
		std::string name;
		std::string id;
		std::string className;
		bool alive = true;
		glm::vec3 position{};
		float speed = 0.0f;       // horizontal metres per second, measured from the position change
		float yaw = 0.0f;
		float detection = 0.0f;   // the awareness value the HUD shows for the player
		bool attacking = false;   // target_attack
		bool following = false;   // target_follow
		bool sees = false;        // target_sees
		std::string tactic;       // current combat tactic id ("" without a combat brain)
		std::string role;         // squad role name
		std::string lane;         // lane phase name ("" without a lane order)
		std::string contact;      // squad belief about the player: none / visible / concealed / lost
		float estimateError = -1.0f; // metres between where the squad thinks the player is and where he is (-1: no belief)
		float distanceToPlayer = 0.0f;
		bool perceives = false;   // the player is in its observer's sight results this step
		bool hears = false;       // a sound the player made reached its observer this step
		std::string skill;        // new brain: the running skill ("" without one)
		int alert = -1;           // new brain: NpcAlert as a number (-1 without a Mind)
	};

	// What the player carries (a real weapon of the Player entity, so the guards' view of the target is the real one).
	enum class Loadout
	{
		Gun,      // an illegal firearm: guards hold it against him on sight, CombatTargetView::hasGun
		Blade,    // a sword: melee armed, nothing illegal
		Unarmed
	};

	// Scripted player: the real Player entity, walked along a navigation path through its real character controller
	// (the inputs of a player who walks, runs or crouches), so the footsteps, the observation target and the combat AI's
	// view of the target are the real ones. Only placing it (TeleportTo, Park) and the gun's noise (Shoot) are scripted.
	class Puppet
	{
	public:
		// Binds the player of the level and gives it the default loadout (Gun).
		bool Bind();
		bool IsBound() const { return player != nullptr; }
		Player* GetPlayer() const { return player; }

		glm::vec3 Position() const;
		std::string Id() const;

		void SetLoadout(Loadout loadout);

		// Placing: the body is put there at once (the same call a level change uses).
		void TeleportTo(const glm::vec3& position);

		// Moving: along the navigation path to `position` at the posture's pace. Arrived() once it is within 0.6 m of it.
		// No path, or no progress for 2 s, ends the move as a problem (TakeProblem), never silently.
		void MoveTo(const glm::vec3& position, Posture posture);
		void Hold(Posture posture); // stand where it is
		bool Moving() const { return state == State::Moving; }
		bool Arrived() const { return state == State::Arrived; }

		// Far away and held there (no falling): "gone". Cleared by the next TeleportTo or MoveTo.
		void Park(const glm::vec3& position);

		// A shot makes him a violent criminal for a few seconds.
		void Shoot();                                    // a gunshot at the puppet's position
		void Emit(NpcSoundType type, float loudness = 0.0f);

		// What went wrong with the last move ("" when nothing); reading it clears it.
		std::string TakeProblem();

		// Called by the step before the game update.
		void Update(float dt);

	private:
		enum class State { Idle, Moving, Arrived, Failed };

		Player* player = nullptr;
		State state = State::Idle;
		bool parked = false;
		glm::vec3 parkPosition{};
		glm::vec3 destination{};
		Posture posture = Posture::Still;
		std::vector<glm::vec3> path;
		size_t waypoint = 0;
		float stillSeconds = 0.0f;
		std::string problem;

		void Release();
		void ApplyPosture(bool forward);
	};

	// The level as the scenarios see it.
	class World
	{
	public:
		std::vector<NpcBase*> Npcs() const;
		NpcBase* FindNpc(const std::string& name) const;

		// Removes every NPC the map came with (a clean room), returns how many.
		int RemoveNpcs();

		// A new entity of the class `className` ("npc", "nav_point"...) standing at `position`, looking along `yawDegrees`,
		// as if the map had it with these keyvalues ("target", "profile", "waitTime"...). `Entity::Name` is set to `name`.
		Entity* SpawnEntity(const std::string& className, const std::string& name, const glm::vec3& position, float yawDegrees,
			const std::map<std::string, std::string>& properties = {});

		// An NPC: SpawnEntity for the NPC classes.
		NpcBase* SpawnNpc(const std::string& className, const std::string& name, const glm::vec3& position, float yawDegrees,
			const std::map<std::string, std::string>& properties = {});

		// Direction -> yaw in degrees as the helpers below use it (0 = +z, 90 = +x).
		static float YawDegrees(const glm::vec3& direction);

		// Navigation grid helpers: places that can be walked to, picked without knowing the map.
		// The floor point on the navigation grid closest to `position`, false when there is none within 3 m.
		bool SnapToFloor(const glm::vec3& position, glm::vec3& out) const;

		// A floor point `distance` metres from `origin` (within `tolerance`), in the horizontal direction `yawDegrees`
		// (+-`angleTolerance` degrees) with a clear line of sight between eye heights. False when the map has none.
		bool FindClearSpot(const glm::vec3& origin, float distance, float yawDegrees, float angleTolerance,
			glm::vec3& out, float tolerance = 1.5f) const;

		// A floor point without line of sight to `from` (eye heights), `minDistance` to `maxDistance` away.
		bool FindHiddenSpot(const glm::vec3& from, float minDistance, float maxDistance, glm::vec3& out) const;

		// A floor point in plain sight of `eye` (`minDistance` to `maxDistance` away) with a floor point within `step` metres of it
		// that `eye` cannot see (standing or lowered): where one step takes a player out of sight. False when the map has none.
		bool FindEdgeSpot(const glm::vec3& eye, float minDistance, float maxDistance, float step, glm::vec3& seen, glm::vec3& hidden) const;

		// A floor point none of `eyes` can see (standing or lowered), at least `minDistance` from each of them and at most `maxDistance` from `around`.
		bool FindSpotHiddenFromAll(const std::vector<glm::vec3>& eyes, const glm::vec3& around, float minDistance, float maxDistance, glm::vec3& out) const;

		// Clear line of sight between two points at standing eye height.
		bool HasLineOfSight(const glm::vec3& a, const glm::vec3& b) const;
	};

	// Writes one CSV row per NPC every few steps; read by a few checks too.
	class Probe
	{
	public:
		void Begin(const std::string& scenario);
		void Sample(double time, const Puppet& puppet, const World& world);
		const std::vector<Snapshot>& Latest() const { return latest; }
		const Snapshot* Find(const std::string& name) const;
		std::string Csv() const { return csv; }

		// This step: did any NPC see the puppet or hear a sound it made?
		bool PuppetPerceived() const { return perceived; }

		// Where the squad believes the puppet is this step: the last place it was seen while someone sees it or has just lost
		// it, the estimate of its hiding place after. No belief while the squad has never met it.
		bool HasBelief() const { return hasBelief; }
		const glm::vec3& BeliefPoint() const { return beliefPoint; }
		int BeliefContact() const { return beliefContact; }

	private:
		bool perceived = false;
		bool hasBelief = false;
		glm::vec3 beliefPoint{};
		int beliefContact = 0;
		std::vector<Snapshot> latest;
		std::vector<std::pair<std::string, glm::vec3>> lastPositions;
		double lastTime = 0.0;
		double lastRow = -1.0;
		std::string csv;
	};

	// What a scenario gets. All the stepping happens through here.
	class Ctx
	{
	public:
		World world;
		Puppet puppet;
		Probe probe;

		int seed = 1; // the run number when a scenario is repeated (-ai_runs); the engine's random generators are not seedable
		const char* scenarioName = "";

		// Simulated time of the last step in which any NPC perceived the puppet (saw it or heard a sound it made); -1 before the first.
		double LastPerceived() const { return lastPerceived; }

		// The same moment in game time (what the NPCs' memories are stamped with); -1 before the first.
		double LastPerceivedGameTime() const { return lastPerceivedGame; }

		// Simulated seconds since the scenario started.
		double Now() const { return now; }

		void Step();
		void Wait(float seconds);

		// Steps until `condition` holds or `timeout` simulated seconds pass. True when the condition held.
		bool WaitUntil(const std::function<bool()>& condition, float timeout);

		// Steps for `duration` seconds and reports whether `condition` held on every step.
		bool HoldsFor(const std::function<bool()>& condition, float duration);

		// The puppet walks (or runs, crouches) to `position` and the call returns once it is there. False, with a failed
		// check saying why, when there is no path, it gets stuck or it does not arrive within `timeout` seconds.
		bool Travel(const glm::vec3& position, Posture posture, float timeout);

		// Checks: a failed check fails the scenario but lets it continue, so one run shows every problem.
		void Check(bool ok, const char* format, ...);
		void Info(const char* format, ...);
		void Metric(const std::string& name, double value);

		bool Failed() const { return failures > 0; }
		int Failures() const { return failures; }

		// Used by the runner.
		void Reset(const char* name, int newSeed, bool watchKnowledge);
		const std::string& Log() const { return log; }
		const std::vector<std::pair<std::string, double>>& Metrics() const { return metrics; }

		// The scenario asked to stop early (a setup problem it cannot continue from).
		bool aborted = false;
		void Abort(const char* format, ...);

	private:
		double now = 0.0;
		double lastPerceived = -1.0;
		double lastPerceivedGame = -1.0;
		int failures = 0;

		// The invariant of the chase and squad tiers: no knowledge without perception. While nothing perceived the puppet
		// (more than a second ago), the squad's estimate of where it is must not move toward where it really is.
		bool watchKnowledge = false;
		bool hadBelief = false;
		int lastContact = 0;
		glm::vec3 lastBelief{};
		int leaks = 0;
		void CheckKnowledge();
		void CheckNpcKnowledge();
		int npcLeaks = 0;
		void ReportPuppetProblem();
		std::string log;
		std::vector<std::pair<std::string, double>> metrics;
	};

	using ScenarioFn = void (*)(Ctx&);

	struct Scenario
	{
		const char* name = "";
		const char* tier = "";
		const char* map = "GameData/maps/aitest.bsp";
		ScenarioFn run = nullptr;
		bool watchKnowledge = false;   // check "no knowledge without perception" on every step (chase and squad tiers)
		float timeLimit = 180.0f;      // simulated seconds before the runner gives up
	};

	void Register(const Scenario& scenario);
	void RegisterAllScenarios();

	// Reads -ai_test from the command line and installs the frame hook. No-op when the flag is absent.
	void InitFromArguments();
	bool IsActive();
}

#endif
