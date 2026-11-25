#include "AiTest.h"

#ifndef DISTRIBUTION

#include <EngineMain.h>
#include <Level.hpp>
#include <AssetRegistry.h>
#include <Logger.hpp>
#include <Time.hpp>
#include <FileSystem/FileSystem.h>
#include <Helpers/StringHelper.h>
#include <Helpers/JsonHelper.hpp>

#include <SDL2/SDL.h>

#include <algorithm>
#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <exception>

#include "../../Entities/Npc/Npc.h"
#include "../../Entities/Player/Player.hpp"

namespace AiTest
{
	namespace
	{
		// Thrown out of a scenario when it runs past its simulated time limit or asks to stop.
		struct StopScenario
		{
			std::string reason;
		};

		std::string Format(const char* format, va_list args)
		{
			char buffer[1024];
			vsnprintf(buffer, sizeof(buffer), format, args);
			return buffer;
		}

		double g_timeLimit = 180.0;

		// "No knowledge without perception" (NPC_AI_DESIGN.md 4.1 rule 2): this long after the last sighting or sound,
		// the squad's estimate of the puppet may not move toward it by more than the tolerance in one step.
		constexpr double KnowledgeGrace = 1.0;
		constexpr float KnowledgeTolerance = 0.3f;
		constexpr int KnowledgeReports = 3;
	}

	const char* PostureName(Posture posture)
	{
		switch (posture)
		{
		case Posture::Still: return "still";
		case Posture::Walk: return "walk";
		case Posture::Run: return "run";
		case Posture::CrouchWalk: return "crouch_walk";
		case Posture::CrouchStill: return "crouch_still";
		}
		return "?";
	}

	// ── Ctx ──────────────────────────────────────────────────────────────────────────────────────────────────────

	void Ctx::Reset(const char* name, int newSeed, bool watch)
	{
		scenarioName = name;
		seed = newSeed;
		now = 0.0;
		lastPerceived = -1.0;
		lastPerceivedGame = -1.0;
		watchKnowledge = watch;
		hadBelief = false;
		leaks = 0;
		npcLeaks = 0;
		failures = 0;
		aborted = false;
		log.clear();
		metrics.clear();
		probe.Begin(name);
	}

	void Ctx::Step()
	{
		if (now >= g_timeLimit)
			throw StopScenario{ "simulated time limit reached (" + std::to_string(static_cast<int>(g_timeLimit)) + " s)" };

		if (aborted)
			throw StopScenario{ "aborted" };

		EngineMain* engine = EngineMain::MainInstance;

		puppet.Update(StepSeconds);

		const float dt = StepSeconds;
		Time::DeltaTimeF = dt;
		Time::DeltaTime = static_cast<double>(dt);
		Time::DeltaTimeFNoTimeScale = dt;
		Time::DeltaTimeNoTimeScale = static_cast<double>(dt);
		Time::GameTime += dt;
		Time::GameTimeNoPause += dt;

		// SimulatingGameTicks stays off: the player (and so its observation target) does not update while it is on.
		engine->RunGameStep();
		engine->RunFinishFrame();

		// The perception system updates a quarter of its observers per frame, chosen by the frame counter.
		++engine->frame;

		now += dt;

		probe.Sample(now, puppet, world);
		if (probe.PuppetPerceived())
		{
			lastPerceived = now;
			lastPerceivedGame = Time::GameTime;
		}

		ReportPuppetProblem();
		if (watchKnowledge)
		{
			CheckKnowledge();
			CheckNpcKnowledge();
		}
	}

	// The same invariant for the new brain, read from the memories themselves: nobody may hold a record about the puppet that
	// is newer than the last time anybody perceived it (a little slack: the senses run every fourth frame).
	void Ctx::CheckNpcKnowledge()
	{
		constexpr double slack = 0.3;
		const std::string playerId = puppet.Id();

		for (NpcBase* base : world.Npcs())
		{
			const Npc* npc = dynamic_cast<const Npc*>(base);
			if (npc == nullptr || npc->CanAct() == false) continue;

			const NpcRecord* record = npc->GetMind().memory.Find(playerId, NpcRecordKind::Person);
			if (record == nullptr) continue;

			if (record->time > lastPerceivedGame + slack)
			{
				++npcLeaks;
				if (npcLeaks <= KnowledgeReports)
					Check(false, "knowledge without perception: %s holds a record about the puppet from %.1f s ago, but nothing perceived it since %.1f s ago",
						npc->Name.c_str(), Time::GameTime - record->time, lastPerceivedGame < 0.0 ? -1.0 : Time::GameTime - lastPerceivedGame);
			}
		}
	}

	void Ctx::ReportPuppetProblem()
	{
		const std::string problem = puppet.TakeProblem();
		if (problem.empty() == false)
			Check(false, "puppet: %s", problem.c_str());
	}

	void Ctx::CheckKnowledge()
	{
		const bool known = probe.HasBelief();
		const glm::vec3 belief = probe.BeliefPoint();
		const bool blind = lastPerceived < 0.0 || now - lastPerceived > KnowledgeGrace;

		// A change of contact level re-derives the point by design (last sighting plus a short lead, 5.2), so it is not compared.
		if (known && hadBelief && probe.BeliefContact() == lastContact && blind)
		{
			// Where the old point would be measured from the puppet's true position now: moving the puppet alone changes nothing.
			const glm::vec3 truth = puppet.Position();
			const float gained = glm::distance(lastBelief, truth) - glm::distance(belief, truth);

			if (gained > KnowledgeTolerance)
			{
				++leaks;
				if (leaks <= KnowledgeReports)
					Check(false, "knowledge without perception: nothing perceived the puppet for %.1f s, yet where the squad believes it is moved %.1f m toward it",
						lastPerceived < 0.0 ? now : now - lastPerceived, gained);
			}
		}

		hadBelief = known;
		lastBelief = belief;
		lastContact = probe.BeliefContact();
	}

	bool Ctx::Travel(const glm::vec3& position, Posture posture, float timeout)
	{
		puppet.MoveTo(position, posture);
		ReportPuppetProblem(); // no path: said now, not at the next step

		const bool done = WaitUntil([&] { return puppet.Moving() == false; }, timeout);

		if (puppet.Arrived()) return true;

		// A move that ended without arriving was reported by the step; one still going just took too long.
		if (done == false)
			Check(false, "the puppet did not reach %.1f %.1f %.1f within %.0f s", position.x, position.y, position.z, timeout);
		return false;
	}

	void Ctx::Wait(float seconds)
	{
		const double until = now + seconds;
		while (now < until)
			Step();
	}

	bool Ctx::WaitUntil(const std::function<bool()>& condition, float timeout)
	{
		const double until = now + timeout;
		while (true)
		{
			if (condition()) return true;
			if (now >= until) return false;
			Step();
		}
	}

	bool Ctx::HoldsFor(const std::function<bool()>& condition, float duration)
	{
		const double until = now + duration;
		while (now < until)
		{
			if (condition() == false) return false;
			Step();
		}
		return true;
	}

	void Ctx::Check(bool ok, const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		const std::string text = Format(format, args);
		va_end(args);

		char prefix[64];
		snprintf(prefix, sizeof(prefix), "  [%7.2f s] ", now);

		if (ok)
		{
			log += std::string(prefix) + "ok    " + text + "\n";
		}
		else
		{
			++failures;
			log += std::string(prefix) + "FAIL  " + text + "\n";
		}
	}

	void Ctx::Info(const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		const std::string text = Format(format, args);
		va_end(args);

		char prefix[64];
		snprintf(prefix, sizeof(prefix), "  [%7.2f s] ", now);
		log += std::string(prefix) + "      " + text + "\n";
	}

	void Ctx::Metric(const std::string& name, double value)
	{
		metrics.emplace_back(name, value);
	}

	void Ctx::Abort(const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		const std::string text = Format(format, args);
		va_end(args);

		++failures;
		log += "  ABORT " + text + "\n";
		aborted = true;
		throw StopScenario{ text };
	}

	// ── Runner ───────────────────────────────────────────────────────────────────────────────────────────────────

	namespace
	{
		struct Result
		{
			std::string name;
			std::string tier;
			int run = 1;
			enum class Outcome { Passed, Failed } outcome = Outcome::Passed;
			std::string note;
			std::string log;
			double simulated = 0.0;
			double wallSeconds = 0.0;
			std::vector<std::pair<std::string, double>> metrics;
		};

		struct QueueItem
		{
			Scenario scenario;
			int run = 1;
		};

		enum class Phase
		{
			Start,
			Loading,
			Finished
		};

		std::vector<Scenario>& Registry()
		{
			static std::vector<Scenario> scenarios;
			return scenarios;
		}

		bool g_active = false;
		Phase g_phase = Phase::Start;
		std::vector<QueueItem> g_queue;
		size_t g_index = 0;
		std::vector<Result> g_results;
		Ctx g_ctx;

		uint16_t g_levelChangeBefore = 0;
		int g_readyFrames = 0;
		int g_loadFrames = 0;
		std::chrono::steady_clock::time_point g_loadStarted;

		const char* OutcomeName(Result::Outcome outcome)
		{
			return outcome == Result::Outcome::Passed ? "passed" : "failed";
		}

		bool Matches(const Scenario& scenario, const std::vector<std::string>& filters)
		{
			if (filters.empty()) return true;

			// A filter is a name or tier; several can be given, separated by commas.
			for (const std::string& argument : filters)
			{
				size_t start = 0;
				while (start <= argument.size())
				{
					size_t end = argument.find(',', start);
					if (end == std::string::npos) end = argument.size();
					const std::string filter = argument.substr(start, end - start);
					if (filter == "all" || filter == scenario.name || filter == scenario.tier)
						return true;
					start = end + 1;
				}
			}
			return false;
		}

		void WriteReports()
		{
			int passed = 0, failed = 0;
			for (const Result& result : g_results)
			{
				if (result.outcome == Result::Outcome::Passed) ++passed;
				else ++failed;
			}

			std::string summary;
			summary += "AI tests: " + std::to_string(passed) + " passed, " + std::to_string(failed) + " failed\n\n";

			json out;
			out["passed"] = passed;
			out["failed"] = failed;
			out["scenarios"] = json::array();

			for (const Result& result : g_results)
			{
				char head[256];
				snprintf(head, sizeof(head), "%-8s %-34s run %d  %6.1f s simulated, %5.1f s real",
					OutcomeName(result.outcome), result.name.c_str(), result.run, result.simulated, result.wallSeconds);
				summary += std::string(head) + "\n";

				if (result.note.empty() == false)
					summary += "    " + result.note + "\n";
				if (result.log.empty() == false)
					summary += result.log;
				for (const auto& metric : result.metrics)
				{
					char line[160];
					snprintf(line, sizeof(line), "    metric %-40s %.3f\n", metric.first.c_str(), metric.second);
					summary += line;
				}
				summary += "\n";

				json entry;
				entry["name"] = result.name;
				entry["tier"] = result.tier;
				entry["run"] = result.run;
				entry["outcome"] = OutcomeName(result.outcome);
				entry["note"] = result.note;
				entry["simulatedSeconds"] = result.simulated;
				json metrics = json::object();
				for (const auto& metric : result.metrics)
					metrics[metric.first] = metric.second;
				entry["metrics"] = metrics;
				out["scenarios"].push_back(entry);
			}

			FileSystemEngine::WriteFile("SaveData/ai_tests/summary.log", summary);
			FileSystemEngine::WriteFile("SaveData/ai_tests/result.json", out.dump(2));

			Logger::Log("AI tests: %d passed, %d failed (SaveData/ai_tests/summary.log)", passed, failed);
		}

		void Finish()
		{
			g_phase = Phase::Finished;
			WriteReports();

			SDL_Event quit;
			quit.type = SDL_QUIT;
			SDL_PushEvent(&quit);
		}

		void RunCurrent()
		{
			const QueueItem& item = g_queue[g_index];

			Result result;
			result.name = item.scenario.name;
			result.tier = item.scenario.tier;
			result.run = item.run;

			g_ctx.Reset(item.scenario.name, item.run, item.scenario.watchKnowledge);

			if (g_ctx.puppet.Bind() == false)
			{
				result.outcome = Result::Outcome::Failed;
				result.note = "no player in the level (the map needs an info_player_start)";
				g_results.push_back(result);
				return;
			}

			g_timeLimit = item.scenario.timeLimit;
			const auto started = std::chrono::steady_clock::now();

			try
			{
				item.scenario.run(g_ctx);
			}
			catch (const StopScenario& stop)
			{
				if (stop.reason != "aborted")
				{
					g_ctx.Check(false, "stopped: %s", stop.reason.c_str());
				}
			}
			catch (const std::exception& exception)
			{
				g_ctx.Check(false, "exception: %s", exception.what());
			}

			g_ctx.puppet.Hold(Posture::Still);

			result.wallSeconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - started).count();
			result.simulated = g_ctx.Now();
			result.outcome = g_ctx.Failed() ? Result::Outcome::Failed : Result::Outcome::Passed;
			result.log = g_ctx.Log();
			result.metrics = g_ctx.Metrics();
			g_results.push_back(result);

			// Written at once: a crash right after a scenario still leaves what it said.
			FileSystemEngine::WriteFile("SaveData/ai_tests/last_scenario.log", std::string(item.scenario.name) + ": " + OutcomeName(result.outcome) + "\n" + result.log);

			const std::string runSuffix = item.run > 1 ? std::string("_run") + std::to_string(item.run) : std::string();
				FileSystemEngine::WriteFile(std::string("SaveData/ai_tests/trace_") + item.scenario.name + runSuffix + ".csv", g_ctx.probe.Csv());

			Logger::Log("ai_test %s: %s", item.scenario.name, OutcomeName(result.outcome));
		}

		// One main-loop frame. Returns true when it stepped the game itself (the loop then skips its own update).
		bool Frame()
		{
			if (g_phase == Phase::Finished) return true;

			if (g_phase == Phase::Start)
			{
				if (g_index >= g_queue.size())
				{
					Finish();
					return true;
				}

				const QueueItem& item = g_queue[g_index];

				Logger::Log("ai_test %s: loading %s", item.scenario.name, item.scenario.map);
				g_levelChangeBefore = Level::CurrentLevelChangeId;
				g_readyFrames = 0;
				g_loadFrames = 0;
				g_loadStarted = std::chrono::steady_clock::now();
				Level::LoadLevelFromFile(item.scenario.map);
				g_phase = Phase::Loading;
				return false;
			}

			// Loading: the normal frames run until the new level is in, its assets are loaded and the player exists.
			++g_loadFrames;

			const bool loaded = Level::Current != nullptr
				&& Level::CurrentLevelChangeId != g_levelChangeBefore
				&& Level::HasPendingLevelLoad() == false
				&& AssetRegistry::HasPendingWork() == false
				&& EngineMain::MainInstance->LoadingFrames == 0
				&& Player::Instance != nullptr;

			g_readyFrames = loaded ? g_readyFrames + 1 : 0;

			if (g_readyFrames >= 10)
			{
				RunCurrent();
				++g_index;
				g_phase = Phase::Start;
				return true;
			}

			const double waited = std::chrono::duration<double>(std::chrono::steady_clock::now() - g_loadStarted).count();
			if (waited > 120.0)
			{
				Result result;
				result.name = g_queue[g_index].scenario.name;
				result.tier = g_queue[g_index].scenario.tier;
				result.run = g_queue[g_index].run;
				result.outcome = Result::Outcome::Failed;
				result.note = "the level did not finish loading in 120 s";
				g_results.push_back(result);
				++g_index;
				g_phase = Phase::Start;
			}

			return false;
		}
	}

	void Register(const Scenario& scenario)
	{
		Registry().push_back(scenario);
	}

	bool IsActive()
	{
		return g_active;
	}

	void InitFromArguments()
	{
		EngineMain* engine = EngineMain::MainInstance;
		if (engine == nullptr || g_active) return;

		const auto argument = engine->Arguments.find("ai_test");
		if (argument == engine->Arguments.end()) return;

		g_active = true;

		RegisterAllScenarios();

		int runs = 1;
		const auto runsArgument = engine->Arguments.find("ai_runs");
		if (runsArgument != engine->Arguments.end() && runsArgument->second.empty() == false)
			runs = std::max(1, std::atoi(runsArgument->second[0].c_str()));

		for (const Scenario& scenario : Registry())
		{
			if (Matches(scenario, argument->second) == false) continue;

			for (int run = 1; run <= runs; ++run)
				g_queue.push_back({ scenario, run });
		}

		Logger::Log("ai_test: %d scenario runs queued", static_cast<int>(g_queue.size()));

		engine->FrameHook = []() { return Frame(); };
	}
}

#endif
