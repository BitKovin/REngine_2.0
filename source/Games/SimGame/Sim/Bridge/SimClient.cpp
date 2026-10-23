#include "SimClient.h"

#include <Logger.hpp>

#include <chrono>
#include <thread>

namespace sim
{
	SimClient::SimClient(Setup newSetup) : setup(std::move(newSetup)), host(setup.types, setup.seed)
	{
		HostHooks hooks;
		const movement::MovementModule* movement = setup.movement;
		const nav::WorldNav* worldNav = setup.worldNav;
		hooks.publishSnapshot = [movement](const World& world, std::vector<uint8_t>& blob) { movement->WriteSnapshot(world, blob); };
		hooks.beforeSave = [movement](const World& world) { return movement::MovementModule::WritePlacements(movement->PlacementsOf(world)); };
		hooks.afterLoad = [movement, worldNav](Server& server, const std::vector<uint8_t>& extra, std::string& error)
		{
			SimSaveHeader header;
			if (!ReadHeader(extra, header))
			{
				error = "the load request carries no readable save header";
				return false;
			}
			if (header.fingerprint == worldNav->Data().Fingerprint())
				return true;
			std::vector<movement::Placement> placements;
			if (!movement::MovementModule::ReadPlacements(header.placements, placements))
			{
				error = "the save's placements are unreadable";
				return false;
			}
			const size_t lost = movement->ReplaceAfterRebake(server, header.levels, placements);
			Logger::Error("sim: the save was made against another world layout (world.nav was rebaked): %zu agents re-placed by level, %zu lost", placements.size() - lost, lost);
			return true;
		};
		host.SetHooks(std::move(hooks));
	}

	SimClient::~SimClient()
	{
		Stop();
	}

	void SimClient::Start(Host::Mode newMode)
	{
		mode = newMode;
		host.Start(newMode);
	}

	void SimClient::Stop()
	{
		host.Stop();
	}

	void SimClient::Update(double realSeconds)
	{
		if (mode == Host::Mode::Pump && host.IsRunning())
			host.Pump(realSeconds);
		Drain();
	}

	void SimClient::Drain()
	{
		for (Message& message : host.Receive())
		{
			switch (message.kind)
			{
			case Message::Kind::Snapshot:
				snapshotBlob = std::move(message.blob);
				snapshotTick = message.tick;
				agentsStale = true;
				++snapshotsReceived;
				break;
			case Message::Kind::Woke:
				++wakesReceived;
				break;
			case Message::Kind::Event:
				outputs.push_back(std::move(message));
				break;
			case Message::Kind::Reply:
			case Message::Kind::Error:
				if (message.requestId != 0)
					replies.push_back(std::move(message));
				else
					Logger::Warning("sim: %s", message.text.c_str());
				break;
			default:
				break;
			}
		}
	}

	const movement::SnapshotView& SimClient::Snapshot()
	{
		if (agentsStale)
		{
			if (!movement::MovementModule::ReadSnapshot(snapshotBlob, latest))
			{
				Logger::Error("sim: unreadable snapshot at tick %lld", static_cast<long long>(snapshotTick));
				latest = movement::SnapshotView{};
			}
			agentsStale = false;
		}
		return latest;
	}

	std::vector<Message> SimClient::TakeOutputs()
	{
		std::vector<Message> result;
		result.swap(outputs);
		return result;
	}

	void SimClient::ReportPlayer(uint16_t level, const glm::vec3& local)
	{
		host.SendEvent(movement::PlayerMoved{ level, local });
	}

	void SimClient::SendControl(Message::Kind kind)
	{
		Message message;
		message.kind = kind;
		host.Send(std::move(message));
	}

	void SimClient::Step(int64_t steps)
	{
		Message message;
		message.kind = Message::Kind::Step;
		message.value = static_cast<double>(steps);
		host.Send(std::move(message));
	}

	void SimClient::SetSpeed(double multiplier, Tick stepTicks)
	{
		Message message;
		message.kind = Message::Kind::SetSpeed;
		message.value = multiplier;
		message.stepTicks = stepTicks;
		host.Send(std::move(message));
	}

	bool SimClient::WaitFor(uint32_t request, Message& reply, std::string& error, double timeoutSeconds)
	{
		using Clock = std::chrono::steady_clock;
		const Clock::time_point start = Clock::now();
		for (;;)
		{
			Drain();
			for (size_t i = 0; i < replies.size(); ++i)
				if (replies[i].requestId == request)
				{
					reply = std::move(replies[i]);
					replies.erase(replies.begin() + static_cast<std::ptrdiff_t>(i));
					if (reply.kind == Message::Kind::Error)
					{
						error = reply.text;
						return false;
					}
					return true;
				}
			if (std::chrono::duration<double>(Clock::now() - start).count() > timeoutSeconds)
			{
				error = "the sim did not answer within " + std::to_string(timeoutSeconds) + " s";
				return false;
			}
			if (mode == Host::Mode::Pump)
				host.Pump(0.0); // answers messages; no time passes
			else
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}

	bool SimClient::SaveBlocking(std::vector<uint8_t>& out, std::string& error, double timeoutSeconds, bool includeLog)
	{
		Message request;
		request.kind = Message::Kind::Save;
		request.value = includeLog ? 0.0 : 1.0;
		Message reply;
		if (!WaitFor(host.SendRequest(std::move(request)), reply, error, timeoutSeconds))
			return false;

		SimSave save;
		save.header.tick = reply.tick;
		save.header.schema = SchemaHash(host.Types());
		save.header.fingerprint = setup.worldNav->Data().Fingerprint();
		for (const nav::WorldLevel& level : setup.worldNav->Data().levels)
			save.header.levels.push_back(level.name);
		save.header.placements = std::move(reply.extra);
		save.world = std::move(reply.blob);
		out = WriteSimSave(save);
		return true;
	}

	bool SimClient::LoadBlocking(const std::vector<uint8_t>& bytes, std::string& error, double timeoutSeconds)
	{
		SimSave save;
		if (!ReadSimSave(bytes, save, error))
			return false;
		if (save.header.schema != SchemaHash(host.Types()))
		{
			error = "the sim save was written by a build with different component types (schema hash differs)";
			return false;
		}
		Message request;
		request.kind = Message::Kind::Load;
		request.blob = std::move(save.world);
		request.extra = WriteHeader(save.header);
		Message reply;
		if (!WaitFor(host.SendRequest(std::move(request)), reply, error, timeoutSeconds))
			return false;
		// What the sim sent before the load describes the old world.
		snapshotBlob.clear();
		latest = movement::SnapshotView{};
		agentsStale = false;
		return true;
	}

	bool SimClient::TextBlocking(const std::string& command, std::string& reply, double timeoutSeconds)
	{
		Message answer;
		std::string error;
		if (!WaitFor(host.SendRequest(Message::Kind::Text, command), answer, error, timeoutSeconds))
		{
			reply = error;
			return false;
		}
		reply = std::move(answer.text);
		return true;
	}
}
