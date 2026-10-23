#pragma once

// Abstract movement (addendum 6 and 7, SLICE_M2_MOVEMENT.md): agents walk the world mesh continuously, in every level,
// stepped by tier. Position is written in one place only (AdvanceWalker's commit and Place); a rule that wants an agent
// somewhere asks for a walk, it never sets a position (design 5.4).
//
// Per step, in system order: movement.paths (solve waiting requests through PathService), movement.integrate (advance
// the walkers that are due for their tier). Rules see Arrived, PathFailed, PortalCrossed.

#include "SimSpeedTable.h"
#include "SimWalk.h"

#include <Sim/Core/SimWorld.h>
#include <Sim/Nav/WorldNav.h>

#include <Sim/Core/SimServer.h>

#include <glm/vec3.hpp>

namespace sim::movement
{
	// ---- Components -------------------------------------------------------------------------------------------------

	struct Position
	{
		static constexpr const char* TypeName = "Position";
		glm::vec3 nav{ 0.0f }; // metres, nav space (the level is derived, addendum 4.5)
		template<class Ar> void Visit(Ar& ar) { ar("nav", nav); }
	};

	struct Walker
	{
		enum class State : uint8_t { Idle, WaitingForPath, Walking };

		static constexpr const char* TypeName = "Walker";
		State state = State::Idle;
		MoverClass moverClass = MoverClass::Person;
		Gait gait = Gait::Walk;
		float speedScale = 1.0f;          // per agent, set at placement
		glm::vec3 destination{ 0.0f };    // nav space, while waiting or walking
		Tick requestedAt = 0;
		std::vector<glm::vec3> corners;   // the path, nav space (empty unless walking)
		std::vector<uint16_t> cornerLevels;
		Progress progress;
		float walkLength = 0.0f;          // metres of walking in the whole path
		Tick lastIntegrated = 0;          // progress is current up to this tick

		// What the last integration did, for the teleport detector (design 5.4).
		glm::vec3 lastTo{ 0.0f };         // the position the movement module wrote last
		float lastWalked = 0.0f;          // metres walked in that integration
		Tick lastTicks = 0;               // its length

		template<class Ar> void Visit(Ar& ar)
		{
			ar("state", state);
			ar("moverClass", moverClass);
			ar("gait", gait);
			ar("speedScale", speedScale);
			ar("destination", destination);
			ar("requestedAt", requestedAt);
			ar("corners", corners);
			ar("cornerLevels", cornerLevels);
			ar("progress", progress);
			ar("walkLength", walkLength);
			ar("lastIntegrated", lastIntegrated);
			ar("lastTo", lastTo);
			ar("lastWalked", lastWalked);
			ar("lastTicks", lastTicks);
		}

		PathView View() const { return { &corners, &cornerLevels }; }
	};

	// The player, as the game tells it (the one place a level is stored: the game knows which level is loaded).
	struct PlayerAnchor
	{
		static constexpr const char* TypeName = "PlayerAnchor";
		uint16_t level = 0;
		glm::vec3 nav{ 0.0f };
		template<class Ar> void Visit(Ar& ar) { ar("level", level); ar("nav", nav); }
	};

	// ---- Events -----------------------------------------------------------------------------------------------------

	// Input: the game reports the player (level index in the world file, level-local position).
	struct PlayerMoved
	{
		static constexpr const char* TypeName = "PlayerMoved";
		uint16_t level = 0;
		glm::vec3 local{ 0.0f };
		template<class Ar> void Visit(Ar& ar) { ar("level", level); ar("local", local); }
	};

	struct PathAssigned
	{
		static constexpr const char* TypeName = "PathAssigned";
		Entity walker = NullEntity;
		float length = 0.0f; // metres of walking
		uint16_t corners = 0;
		uint16_t portals = 0;
		template<class Ar> void Visit(Ar& ar) { ar("walker", walker); ar("length", length); ar("corners", corners); ar("portals", portals); }
	};

	struct PathFailed
	{
		static constexpr const char* TypeName = "PathFailed";
		Entity walker = NullEntity;
		FixedString<24> reason; // start_off_mesh, destination_off_mesh, no_path, unreachable
		template<class Ar> void Visit(Ar& ar) { ar("walker", walker); ar("reason", reason); }
	};

	struct PortalCrossed
	{
		static constexpr const char* TypeName = "PortalCrossed";
		Entity walker = NullEntity;
		uint16_t fromLevel = 0;
		uint16_t toLevel = 0;
		template<class Ar> void Visit(Ar& ar) { ar("walker", walker); ar("fromLevel", fromLevel); ar("toLevel", toLevel); }
	};

	struct Arrived
	{
		static constexpr const char* TypeName = "Arrived";
		Entity walker = NullEntity;
		Tick at = 0; // the exact tick; the event itself may be emitted later (tier step, fast-forward)
		template<class Ar> void Visit(Ar& ar) { ar("walker", walker); ar("at", at); }
	};

	// A position change that is not path progress (design 5.4 (c)): placement, portal crossings, later respawns.
	struct PositionException
	{
		static constexpr const char* TypeName = "PositionException";
		Entity entity = NullEntity;
		glm::vec3 from{ 0.0f };
		glm::vec3 to{ 0.0f };
		FixedString<16> reason; // placed, portal
		template<class Ar> void Visit(Ar& ar) { ar("entity", entity); ar("from", from); ar("to", to); ar("reason", reason); }
	};

	// A walker was put back on the mesh after a rebake moved the levels: its path and request are gone (it is Idle). Rules
	// that keep agents busy react to it like to Arrived.
	struct Replaced
	{
		static constexpr const char* TypeName = "Replaced";
		Entity walker = NullEntity;
		template<class Ar> void Visit(Ar& ar) { ar("walker", walker); }
	};

	// ---- Commands ---------------------------------------------------------------------------------------------------

	struct RequestWalk
	{
		static constexpr const char* TypeName = "RequestWalk";
		Entity walker = NullEntity;
		glm::vec3 destination{ 0.0f }; // nav space
		Gait gait = Gait::Walk;
		template<class Ar> void Visit(Ar& ar) { ar("walker", walker); ar("destination", destination); ar("gait", gait); }
	};

	struct StopWalk
	{
		static constexpr const char* TypeName = "StopWalk";
		Entity walker = NullEntity;
		template<class Ar> void Visit(Ar& ar) { ar("walker", walker); }
	};

	struct SetGait
	{
		static constexpr const char* TypeName = "SetGait";
		Entity walker = NullEntity;
		Gait gait = Gait::Walk;
		template<class Ar> void Visit(Ar& ar) { ar("walker", walker); ar("gait", gait); }
	};

	struct SetPlayerLocus
	{
		static constexpr const char* TypeName = "SetPlayerLocus";
		uint16_t level = 0;
		glm::vec3 local{ 0.0f };
		template<class Ar> void Visit(Ar& ar) { ar("level", level); ar("local", local); }
	};

	// ---- Module -----------------------------------------------------------------------------------------------------

	enum class Tier : uint8_t
	{
		T0, // within tier0_radius of the player in the player's level
		T1, // the rest of the player's level
		T2, // other levels, or no player yet
	};

	// What the game may draw of one walker (the Snapshot blob): level and nav position, no sim internals.
	struct AgentView
	{
		Entity entity = NullEntity;
		uint16_t level = 0;
		uint8_t tier = 0;  // Tier
		uint8_t state = 0; // Walker::State
		glm::vec3 nav{ 0.0f };
		template<class Ar> void Visit(Ar& ar) { ar("entity", entity); ar("level", level); ar("tier", tier); ar("state", state); ar("nav", nav); }
	};

	// The whole Snapshot blob: the sim's idea of where the player is, and every walker.
	struct SnapshotView
	{
		bool hasPlayer = false;
		uint16_t playerLevel = 0;
		glm::vec3 playerNav{ 0.0f };
		std::vector<AgentView> agents;
		template<class Ar> void Visit(Ar& ar) { ar("hasPlayer", hasPlayer); ar("playerLevel", playerLevel); ar("playerNav", playerNav); ar("agents", agents); }
	};

	// Where a walker stood in level terms when a save was made: survives a rebake that moves levels (SLICE_M2B_BRIDGE 6.3).
	struct Placement
	{
		Entity entity = NullEntity;
		uint16_t level = 0; // index into the level names stored with the save
		glm::vec3 local{ 0.0f };
		template<class Ar> void Visit(Ar& ar) { ar("entity", entity); ar("level", level); ar("local", local); }
	};

	class MovementModule : public Module
	{
	public:
		// `nav` must be loaded and outlive the module and every World built from it. Used from the sim thread only.
		explicit MovementModule(nav::WorldNav& nav);

		const char* Name() const override { return "movement"; }
		void Register(Registration& registration) override;

		// Setup helper (RunSetup, scenario placement): a new walker at a nav-space position. Logs a PositionException.
		Entity Place(ApplyContext& context, const glm::vec3& navPosition, MoverClass moverClass = MoverClass::Person) const;

		// Where the agent is at `tick`, extrapolated along its path without changing anything (addendum 6.2).
		glm::vec3 PositionAt(const World& world, Entity entity, Tick tick) const;

		// The level the agent is in, derived from its path. -1 when it has none (stationary agents are on no path).
		int LevelOf(const World& world, Entity entity) const;

		// The tier the agent would be stepped at now.
		Tier TierOf(const World& world, Entity entity) const;

		// Ticks between integrations of a tier (tunables, at least 1).
		Tick StepOf(const World& world, Tier tier) const;

		// Snapshot for the game thread (Host publisher): the player anchor and every walker as an AgentView.
		void WriteSnapshot(const World& world, std::vector<uint8_t>& blob) const;
		static bool ReadSnapshot(const std::vector<uint8_t>& blob, SnapshotView& out);

		// Save barrier: the level and level-local position of every walker, in the current world's level order.
		std::vector<Placement> PlacementsOf(const World& world) const;
		static std::vector<uint8_t> WritePlacements(const std::vector<Placement>& placements);
		static bool ReadPlacements(const std::vector<uint8_t>& bytes, std::vector<Placement>& out);

		// After loading a save made against another layout: puts every walker back at its level and local position on the
		// current mesh (projection 2, 10, then 50 m), Idle, with PositionException(rebake) and Replaced events. A walker
		// whose level is gone or that has no mesh within 50 m is destroyed with an error. Drops the player anchor (the game
		// reports again). `oldLevels` are the level names the placements index. Returns the number destroyed.
		size_t ReplaceAfterRebake(Server& server, const std::vector<std::string>& oldLevels, const std::vector<Placement>& placements) const;

		TunableId tier0Radius = 0, tier0Step = 0, tier1Step = 0, tier2Step = 0, portalDelay = 0, pathsPerTick = 0, speedScaleSpread = 0;
		RandomStreamId placement = 0;

	private:
		struct PlayerInfo
		{
			bool present = false;
			uint16_t level = 0;
			glm::vec3 nav{ 0.0f };
		};

		nav::WorldNav& worldNav;
		nav::PathService pathService; // used inside movement.paths only; its queue is empty between steps

		static PlayerInfo FindPlayer(const entt::registry& registry);
		static Tier TierFor(const PlayerInfo& player, int level, const glm::vec3& position, float radius);
		float SpeedOf(const Walker& walker) const;

		// Brings a walking walker up to `now`; the only writer of Position after placement.
		void AdvanceWalker(ApplyContext& context, Entity entity, Walker& walker, Position& position) const;
		void SyncWalker(ApplyContext& context, Entity entity) const;

		void SolvePaths(ApplyContext& context);
		void Integrate(ApplyContext& context, Tick dt) const;
		bool CheckNoTeleport(const World& world, std::string& error) const;
	};
}
