#pragma once

// Transport between the game thread (client) and the sim (server), addendum section 3.4. One interface, two modes:
//   Thread: the sim runs on its own thread; messages cross through mutex-guarded swap buffers.
//   Pump:   the caller drives the sim with Pump(realSeconds) on its own thread (headless tests, tools, replays).
// Both modes run the same Service function, so they differ only in who calls it.
//
// The client never touches the Server or World while the thread runs. Everything goes through messages:
// event inputs, control (pause, step, speed), save and load at a tick boundary, and text requests for tools.

#include "SimServer.h"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>

namespace sim
{
	struct Message
	{
		enum class Kind : uint8_t
		{
			Event,      // both directions: an input event (client to sim) or an output event (sim to client)
			Pause,      // client: stop advancing at the next tick boundary
			Resume,
			Step,       // client: while paused, run `value` steps
			SetSpeed,   // client: world seconds per real second = value; ticks per step = stepTicks (fast-forward)
			Save,       // client: snapshot at the next tick boundary; reply carries `blob` (and `extra` from the beforeSave hook).
			            //         value > 0.5 leaves the event log out of the snapshot
			Load,       // client: replace the world with `blob`, then run the afterLoad hook with `extra`; reply carries ok or `text` error
			Text,       // client: run a text command (Server::RunTextCommand); reply carries `text`
			Reply,      // sim: answer to a request with the same requestId
			Woke,       // sim: a wake event stopped fast-forward; eventId names it
			Snapshot,   // sim: the publisher's blob (what the game may draw); only the newest matters
			Error,      // sim: a request failed; `text` says why
		};

		Kind kind = Kind::Event;
		uint32_t requestId = 0;
		TypeId type = InvalidType; // Event
		Tick tick = 0;             // sim: tick of the event or of the reply
		EventId eventId = NoEvent; // sim: id of an output or wake event
		double value = 0.0;
		Tick stepTicks = 1;
		alignas(8) std::array<uint8_t, MaxPayloadSize> payload{};
		std::vector<uint8_t> blob;
		std::vector<uint8_t> extra; // Save reply / Load request: game data that travels with the snapshot (see HostHooks)
		std::string text;

		// The payload of an output event as T, or nullptr.
		template<class T> const T* As(const TypeRegistry& types) const
		{
			return kind == Kind::Event && type == types.EventId<T>() ? reinterpret_cast<const T*>(payload.data()) : nullptr;
		}
	};

	// Game-specific work the Host does on the sim thread at tick boundaries. Set before Start.
	struct HostHooks
	{
		// Fills `blob` with what the client may draw. Runs after the step that crosses `snapshotInterval` ticks.
		std::function<void(const World&, std::vector<uint8_t>& blob)> publishSnapshot;
		Tick snapshotInterval = 2;
		// Save barrier: extra data computed from the world at the same tick as the snapshot.
		std::function<std::vector<uint8_t>(const World&)> beforeSave;
		// After a successful Load: `extra` is what the matching save's beforeSave returned (migrations).
		std::function<bool(Server&, const std::vector<uint8_t>& extra, std::string& error)> afterLoad;
	};

	class Host
	{
	public:
		enum class Mode { Pump, Thread };

		Host(std::shared_ptr<const TypeRegistry> types, uint64_t seed);
		~Host();
		Host(const Host&) = delete;
		Host& operator=(const Host&) = delete;

		const TypeRegistry& Types() const { return *types; }

		void SetHooks(HostHooks newHooks) { hooks = std::move(newHooks); }

		// Direct server access: only before Start, or from the pumping thread in Pump mode.
		Server& ServerUnsafe() { return server; }

		void Start(Mode mode);
		void Stop();
		bool IsRunning() const { return mode == Mode::Thread ? threadRunning.load() : started; }

		void Send(Message message);
		template<class E> void SendEvent(const E& payload)
		{
			Message message;
			message.kind = Message::Kind::Event;
			message.type = types->EventId<E>();
			std::memcpy(message.payload.data(), &payload, sizeof(E));
			Send(std::move(message));
		}
		uint32_t SendRequest(Message::Kind kind, std::string text = {}, std::vector<uint8_t> blob = {});
		// Same for a prepared message (Save's `value`, Load's `extra`); returns the request id the reply will carry.
		uint32_t SendRequest(Message message);

		// Messages from the sim since the last call.
		std::vector<Message> Receive();

		// Pump mode: advance by this much real time on the calling thread.
		void Pump(double realSeconds);

		// Readable from any thread.
		Tick CurrentTick() const { return currentTick.load(); }
		bool IsPaused() const { return pausedFlag.load(); }

		// Cap on steps per service call; beyond it real time is dropped (the sim falls behind, it never skips events).
		int maxStepsPerService = 20;

	private:
		std::shared_ptr<const TypeRegistry> types;
		Server server;
		HostHooks hooks;
		Tick lastSnapshotTick = 0;
		Mode mode = Mode::Pump;
		bool started = false;

		// Owned by whoever runs Service (the sim thread in Thread mode).
		double accumulator = 0.0;
		double speed = 1.0;
		Tick stepTicks = 1;
		bool paused = false;
		int64_t pendingSteps = 0;

		std::mutex inboxMutex;
		std::condition_variable inboxSignal;
		std::vector<Message> inbox;
		std::mutex outboxMutex;
		std::vector<Message> outbox;
		std::atomic<uint32_t> nextRequestId{ 1 };

		std::thread thread;
		std::atomic<bool> threadRunning{ false };
		std::atomic<bool> stopRequested{ false };
		std::atomic<Tick> currentTick{ 0 };
		std::atomic<bool> pausedFlag{ false };

		void Service(double realSeconds);
		void HandleMessage(Message& message, std::vector<Message>& replies);
		void ThreadMain();
	};
}
