#pragma once

// The game-thread side of the sim (SLICE_M2B_BRIDGE.md): owns the Host, tells it about the player, keeps the newest snapshot,
// and runs save and load as barrier requests. It knows no engine statics, so the self tests run it in both Host modes.
// Not thread-safe: one game thread calls it; the Host's own thread is behind messages.

#include "SimSaveFile.h"

#include <Sim/Core/SimHost.h>
#include <Sim/Movement/SimMovement.h>

#include <memory>
#include <string>
#include <vector>

namespace sim
{
	class SimClient
	{
	public:
		struct Setup
		{
			std::shared_ptr<const TypeRegistry> types;
			const nav::WorldNav* worldNav = nullptr; // loaded; read for the layout fingerprint and level names
			const movement::MovementModule* movement = nullptr;
			uint64_t seed = 1;
		};

		explicit SimClient(Setup setup);
		~SimClient();

		// Before Start (or from the pumping thread in Pump mode): world population. Same rules as Host::ServerUnsafe.
		Server& ServerForSetup() { return host.ServerUnsafe(); }
		Host& GetHost() { return host; }

		void Start(Host::Mode mode);
		void Stop();
		bool IsRunning() const { return host.IsRunning(); }

		// ---- every frame
		// Pump mode: advance the sim by `realSeconds` on this thread. Both modes: then take what the sim sent.
		void Update(double realSeconds = 0.0);

		// The player's level (index in the world file) and level-local position.
		void ReportPlayer(uint16_t level, const glm::vec3& local);

		// Any registered input event.
		template<class E> void Send(const E& event) { host.SendEvent(event); }

		// Newest snapshot (decoded on demand), its tick, and how many arrived so far.
		const movement::SnapshotView& Snapshot();
		const std::vector<movement::AgentView>& Agents() { return Snapshot().agents; }
		Tick SnapshotTick() const { return snapshotTick; }
		uint64_t SnapshotsReceived() const { return snapshotsReceived; }
		uint64_t WakesReceived() const { return wakesReceived; }
		// Output events (EventOutput) the sim sent since the last call.
		std::vector<Message> TakeOutputs();

		// ---- control
		void Pause() { SendControl(Message::Kind::Pause); }
		void Resume() { SendControl(Message::Kind::Resume); }
		void Step(int64_t steps);
		void SetSpeed(double multiplier, Tick stepTicks = 1);
		Tick CurrentTick() const { return host.CurrentTick(); }
		bool IsPaused() const { return host.IsPaused(); }

		// ---- barrier requests. They wait for the sim's answer on this thread (it comes at the next tick boundary, even while
		// paused); false with `error` on timeout or refusal, and the sim is then untouched.
		bool SaveBlocking(std::vector<uint8_t>& out, std::string& error, double timeoutSeconds = 2.0, bool includeLog = DefaultIncludeLog());
		bool LoadBlocking(const std::vector<uint8_t>& bytes, std::string& error, double timeoutSeconds = 5.0);
		bool TextBlocking(const std::string& command, std::string& reply, double timeoutSeconds = 2.0);

		static constexpr bool DefaultIncludeLog()
		{
#ifdef DISTRIBUTION
			return false;
#else
			return true;
#endif
		}

	private:
		Setup setup;
		Host host;
		Host::Mode mode = Host::Mode::Pump;

		std::vector<uint8_t> snapshotBlob;
		movement::SnapshotView latest;
		bool agentsStale = false;
		Tick snapshotTick = 0;
		uint64_t snapshotsReceived = 0;
		uint64_t wakesReceived = 0;
		std::vector<Message> replies;
		std::vector<Message> outputs;

		void SendControl(Message::Kind kind);
		void Drain();
		bool WaitFor(uint32_t request, Message& reply, std::string& error, double timeoutSeconds);
	};
}
