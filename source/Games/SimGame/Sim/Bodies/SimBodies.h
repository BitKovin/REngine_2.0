#pragma once

// Bodies (addendum 7.2, SLICE_M2C_BODIES.md): the sim proposes a physical body for walkers near the player and takes it back
// when they are far; the game accepts when the spawn can be hidden. While a body is Active it is the mover: the sim tells it
// where to go (the next corner of the walker's path, "goals") and moves the walker's progress to where the body reports
// itself (movement.ApplyBodyPosition). The body runs its own navigation and state code; the sim only dictates targets.
//
//   sim -> game (EventOutput)      BodySpawnProposed, BodyDespawnProposed, BodyGoal
//   game -> sim (EventInput)       BodyAccepted, BodyRefused, BodyReleased, BodyReport, BodiesCleared
//
// Proposals are made every propose_period ticks, nearest first, within a budget of max_bodies. Hysteresis: spawn inside
// spawn_radius, release outside despawn_radius, or when the player is in another level, or when the walker reaches a portal.

#include <Sim/Movement/SimMovement.h>

namespace sim::bodies
{
	using movement::Embodied;

	// What an agent looks like (saved with the agent so a loaded game shows the same people).
	struct Appearance
	{
		static constexpr const char* TypeName = "Appearance";
		uint8_t model = 0; // index into the game's model list
		template<class Ar> void Visit(Ar& ar) { ar("model", model); }
	};

	// A body was refused or never answered: no new proposal before `until`.
	struct BodyRetry
	{
		static constexpr const char* TypeName = "BodyRetry";
		Tick until = 0;
		template<class Ar> void Visit(Ar& ar) { ar("until", until); }
	};

	// ---- sim -> game ----------------------------------------------------------------------------------------------------

	struct BodySpawnProposed
	{
		static constexpr const char* TypeName = "BodySpawnProposed";
		Entity entity = NullEntity;
		glm::vec3 local{ 0.0f };   // level-local feet position
		glm::vec3 heading{ 0.0f, 0.0f, 1.0f }; // horizontal direction the agent is walking
		float speedScale = 1.0f;
		uint8_t model = 0;
		template<class Ar> void Visit(Ar& ar) { ar("entity", entity); ar("local", local); ar("heading", heading); ar("speedScale", speedScale); ar("model", model); }
	};

	struct BodyDespawnProposed
	{
		// Why the sim takes the body back. The game releases the first three at once (the agent is leaving or the body is stale) and waits
		// for cover for OutOfRange, so nobody vanishes in plain sight because they walked 56 m away.
		enum class Reason : uint8_t { OutOfRange, OtherLevel, NoPlayer, AtPortal, Unknown };

		static constexpr const char* TypeName = "BodyDespawnProposed";
		Entity entity = NullEntity;
		Reason reason = Reason::Unknown;
		template<class Ar> void Visit(Ar& ar) { ar("entity", entity); ar("reason", reason); }
	};

	struct BodyGoal
	{
		enum class Kind : uint8_t { Move, Stop };

		static constexpr const char* TypeName = "BodyGoal";
		Entity entity = NullEntity;
		glm::vec3 local{ 0.0f };   // level-local: where to go (Move) or where the agent stands (Stop)
		glm::vec3 next{ 0.0f };    // the corner after `local`, when hasNext: the body walks on to it without stopping at `local`
		Kind kind = Kind::Move;
		movement::Gait gait = movement::Gait::Walk;
		bool final = false;        // `local` is the end of the walk
		bool hasNext = false;
		float speedScale = 1.0f;
		template<class Ar> void Visit(Ar& ar)
		{
			ar("entity", entity);
			ar("local", local);
			ar("next", next);
			ar("kind", kind);
			ar("gait", gait);
			ar("final", final);
			ar("hasNext", hasNext);
			ar("speedScale", speedScale);
		}
	};

	// ---- game -> sim ----------------------------------------------------------------------------------------------------

	struct BodyAccepted
	{
		static constexpr const char* TypeName = "BodyAccepted";
		Entity entity = NullEntity;
		glm::vec3 local{ 0.0f }; // where the body actually stands
		template<class Ar> void Visit(Ar& ar) { ar("entity", entity); ar("local", local); }
	};

	struct BodyRefused
	{
		static constexpr const char* TypeName = "BodyRefused";
		Entity entity = NullEntity;
		template<class Ar> void Visit(Ar& ar) { ar("entity", entity); }
	};

	// The body is gone; `local` is where it stood.
	struct BodyReleased
	{
		static constexpr const char* TypeName = "BodyReleased";
		Entity entity = NullEntity;
		glm::vec3 local{ 0.0f };
		template<class Ar> void Visit(Ar& ar) { ar("entity", entity); ar("local", local); }
	};

	struct BodyReport
	{
		static constexpr const char* TypeName = "BodyReport";
		Entity entity = NullEntity;
		glm::vec3 local{ 0.0f };
		template<class Ar> void Visit(Ar& ar) { ar("entity", entity); ar("local", local); }
	};

	// Every body of the game is gone (a level was unloaded).
	struct BodiesCleared
	{
		static constexpr const char* TypeName = "BodiesCleared";
		uint8_t unused = 0;
		template<class Ar> void Visit(Ar& ar) { ar("unused", unused); }
	};

	// One command for all the game's answers (rules map the input events onto it).
	struct ApplyBodyInput
	{
		enum class Kind : uint8_t { Accepted, Refused, Released, Report, Cleared };

		static constexpr const char* TypeName = "ApplyBodyInput";
		Entity entity = NullEntity;
		glm::vec3 local{ 0.0f };
		Kind kind = Kind::Report;
		template<class Ar> void Visit(Ar& ar) { ar("entity", entity); ar("local", local); ar("kind", kind); }
	};

	class BodiesModule : public Module
	{
	public:
		// `movement` must be registered before this module.
		BodiesModule(const movement::MovementModule& movement, const nav::WorldNav& worldNav);

		const char* Name() const override { return "bodies"; }
		void Register(Registration& registration) override;

		// Number of walkers with a body, proposed or not yet released.
		static size_t EmbodiedCount(const World& world);

		TunableId spawnRadius = 0, despawnRadius = 0, maxBodies = 0, proposePeriod = 0, proposalTimeout = 0, reportTimeout = 0, retryCooldown = 0, releaseTimeout = 0;
		RandomStreamId look = 0;
		static constexpr int ModelCount = 2; // base.glb and guard.glb

	private:
		const movement::MovementModule& movement;
		const nav::WorldNav& worldNav;

		void Update(ApplyContext& context, Tick dt) const;
		void Propose(ApplyContext& context) const;
		void Maintain(ApplyContext& context) const;
		void ApplyInput(ApplyContext& context, const ApplyBodyInput& input) const;
		glm::vec3 LocalOf(const World& world, Entity entity, const glm::vec3& nav) const;
	};
}
