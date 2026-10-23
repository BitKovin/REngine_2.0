#include "SimHost.h"

#include <Logger.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>

namespace sim
{
	Host::Host(std::shared_ptr<const TypeRegistry> types, uint64_t seed) : types(types), server(types, seed)
	{
	}

	Host::~Host()
	{
		Stop();
	}

	void Host::Start(Mode newMode)
	{
		if (started)
			return;
		mode = newMode;
		started = true;
		currentTick = server.GetWorld().Now();
		if (mode == Mode::Thread)
		{
			stopRequested = false;
			threadRunning = true;
			thread = std::thread(&Host::ThreadMain, this);
		}
	}

	void Host::Stop()
	{
		if (!started)
			return;
		if (mode == Mode::Thread)
		{
			{
				std::lock_guard<std::mutex> lock(inboxMutex);
				stopRequested = true;
			}
			inboxSignal.notify_all();
			if (thread.joinable())
				thread.join();
			threadRunning = false;
		}
		started = false;
	}

	void Host::Send(Message message)
	{
		{
			std::lock_guard<std::mutex> lock(inboxMutex);
			inbox.push_back(std::move(message));
		}
		inboxSignal.notify_one();
	}

	uint32_t Host::SendRequest(Message::Kind kind, std::string text, std::vector<uint8_t> blob)
	{
		Message message;
		message.kind = kind;
		message.requestId = nextRequestId++;
		message.text = std::move(text);
		message.blob = std::move(blob);
		const uint32_t id = message.requestId;
		Send(std::move(message));
		return id;
	}

	std::vector<Message> Host::Receive()
	{
		std::vector<Message> result;
		std::lock_guard<std::mutex> lock(outboxMutex);
		result.swap(outbox);
		return result;
	}

	void Host::Pump(double realSeconds)
	{
		if (mode != Mode::Pump || !started)
		{
			Logger::Error("sim: Host::Pump needs Start(Mode::Pump)");
			return;
		}
		Service(realSeconds);
	}

	void Host::HandleMessage(Message& message, std::vector<Message>& replies)
	{
		auto reply = [&](Message::Kind kind)
		{
			Message out;
			out.kind = kind;
			out.requestId = message.requestId;
			out.tick = server.GetWorld().Now();
			return out;
		};

		switch (message.kind)
		{
		case Message::Kind::Event:
			if (!server.QueueInput(message.type, message.payload.data(), message.type < types->Events().size() ? types->Event(message.type).ops.size : 0))
			{
				Message error = reply(Message::Kind::Error);
				error.text = "event is not a registered input";
				replies.push_back(std::move(error));
			}
			break;
		case Message::Kind::Pause:
			paused = true;
			accumulator = 0.0;
			break;
		case Message::Kind::Resume:
			paused = false;
			break;
		case Message::Kind::Step:
			pendingSteps += std::max<int64_t>(0, static_cast<int64_t>(message.value));
			break;
		case Message::Kind::SetSpeed:
			speed = std::max(0.0, message.value);
			stepTicks = std::max<Tick>(1, message.stepTicks);
			break;
		case Message::Kind::Save:
		{
			Message out = reply(Message::Kind::Reply);
			out.blob = server.GetWorld().Save(true);
			replies.push_back(std::move(out));
			break;
		}
		case Message::Kind::Load:
		{
			std::string error;
			if (server.Load(message.blob.data(), message.blob.size(), error))
				replies.push_back(reply(Message::Kind::Reply));
			else
			{
				Message out = reply(Message::Kind::Error);
				out.text = error;
				replies.push_back(std::move(out));
			}
			accumulator = 0.0;
			break;
		}
		case Message::Kind::Text:
		{
			Message out = reply(Message::Kind::Reply);
			out.text = server.RunTextCommand(message.text);
			replies.push_back(std::move(out));
			break;
		}
		default:
		{
			Message out = reply(Message::Kind::Error);
			out.text = "not a client message";
			replies.push_back(std::move(out));
			break;
		}
		}
	}

	void Host::Service(double realSeconds)
	{
		std::vector<Message> messages;
		{
			std::lock_guard<std::mutex> lock(inboxMutex);
			messages.swap(inbox);
		}

		// Control and requests are handled at this tick boundary, in arrival order.
		std::vector<Message> replies;
		for (Message& message : messages)
			HandleMessage(message, replies);

		int64_t steps = 0;
		if (paused)
		{
			steps = pendingSteps;
			pendingSteps = 0;
		}
		else
		{
			const double stepSeconds = TicksToSeconds(stepTicks);
			accumulator += std::max(0.0, realSeconds) * speed;
			steps = static_cast<int64_t>(std::floor(accumulator / stepSeconds));
			if (steps > maxStepsPerService)
			{
				steps = maxStepsPerService;
				accumulator = 0.0; // behind: drop the backlog instead of spiralling
			}
			else
				accumulator -= steps * stepSeconds;
		}

		std::vector<Message> outputs;
		for (int64_t i = 0; i < steps; ++i)
		{
			server.Step(paused ? 1 : stepTicks);
			for (const EventRecord& record : server.TakeOutputs())
			{
				Message out;
				out.kind = Message::Kind::Event;
				out.type = record.type;
				out.tick = record.tick;
				out.eventId = record.id;
				out.payload = record.payload;
				outputs.push_back(std::move(out));
			}
			if (server.GetWorld().TakeWake() && speed > 1.0)
			{
				speed = 1.0;
				stepTicks = 1;
				accumulator = 0.0;
				Message woke;
				woke.kind = Message::Kind::Woke;
				woke.tick = server.GetWorld().Now();
				woke.eventId = server.GetWorld().LastEventId();
				outputs.push_back(std::move(woke));
				break;
			}
		}

		currentTick = server.GetWorld().Now();
		pausedFlag = paused;

		if (!outputs.empty() || !replies.empty())
		{
			std::lock_guard<std::mutex> lock(outboxMutex);
			for (Message& message : outputs)
				outbox.push_back(std::move(message));
			for (Message& message : replies)
				outbox.push_back(std::move(message));
		}
	}

	void Host::ThreadMain()
	{
		using Clock = std::chrono::steady_clock;
		Clock::time_point last = Clock::now();
		while (true)
		{
			const Clock::time_point now = Clock::now();
			const double elapsed = std::chrono::duration<double>(now - last).count();
			last = now;
			Service(elapsed);

			// Sleep until the next step is due, or until a message or stop arrives.
			double wait = 0.05;
			if (!paused && speed > 0.0)
				wait = std::clamp((TicksToSeconds(stepTicks) - accumulator) / speed, 0.001, 0.05);
			std::unique_lock<std::mutex> lock(inboxMutex);
			if (stopRequested)
				break;
			inboxSignal.wait_for(lock, std::chrono::duration<double>(wait), [this] { return stopRequested.load() || !inbox.empty(); });
			if (stopRequested)
				break;
		}
	}
}
