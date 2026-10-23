#include "SimScheduler.h"

#include <algorithm>
#include <cstring>

namespace sim
{
	namespace
	{
		// std heap functions build a max-heap, so "less" means "fires later".
		bool FiresLater(const ScheduledEvent& a, const ScheduledEvent& b)
		{
			if (a.at != b.at)
				return a.at > b.at;
			return a.sequence > b.sequence;
		}
	}

	TimerHandle Scheduler::Schedule(Tick at, TypeId type, const void* payload, size_t payloadSize, EventId cause, RuleId rule, bool withHandle)
	{
		ScheduledEvent entry;
		entry.at = at;
		entry.sequence = nextSequence++;
		entry.cause = cause;
		entry.rule = rule;
		entry.type = type;
		std::memcpy(entry.payload.data(), payload, payloadSize);

		TimerHandle handle;
		if (withHandle)
		{
			uint32_t slot;
			if (!freeSlots.empty())
			{
				slot = freeSlots.back();
				freeSlots.pop_back();
			}
			else
			{
				slot = static_cast<uint32_t>(slotVersions.size());
				slotVersions.push_back(0);
			}
			handle.slot = slot;
			handle.version = ++slotVersions[slot];
			entry.slot = handle.slot;
			entry.version = handle.version;
		}

		entries.push_back(entry);
		std::push_heap(entries.begin(), entries.end(), FiresLater);
		return handle;
	}

	bool Scheduler::Cancel(TimerHandle& handle)
	{
		const bool pending = IsPending(handle);
		if (pending)
		{
			ReleaseSlot(handle.slot);
			++staleCount;
			CompactIfNeeded();
		}
		handle = TimerHandle{};
		return pending;
	}

	bool Scheduler::IsPending(const TimerHandle& handle) const
	{
		return handle.IsSet() && handle.slot < slotVersions.size() && slotVersions[handle.slot] == handle.version;
	}

	bool Scheduler::PopDue(Tick upTo, ScheduledEvent& out)
	{
		while (!entries.empty() && entries.front().at <= upTo)
		{
			std::pop_heap(entries.begin(), entries.end(), FiresLater);
			ScheduledEvent entry = entries.back();
			entries.pop_back();

			if (IsStale(entry))
			{
				--staleCount;
				continue;
			}
			if (entry.slot != TimerHandle::NoSlot)
				ReleaseSlot(entry.slot);
			out = entry;
			return true;
		}
		return false;
	}

	std::vector<ScheduledEvent> Scheduler::SortedLiveEntries() const
	{
		std::vector<ScheduledEvent> live;
		live.reserve(LiveCount());
		for (const ScheduledEvent& entry : entries)
			if (!IsStale(entry))
				live.push_back(entry);
		std::sort(live.begin(), live.end(), [](const ScheduledEvent& a, const ScheduledEvent& b) { return FiresLater(b, a); });
		return live;
	}

	void Scheduler::Clear()
	{
		entries.clear();
		slotVersions.clear();
		freeSlots.clear();
		nextSequence = 1;
		staleCount = 0;
	}

	void Scheduler::Restore(std::vector<ScheduledEvent> live, std::vector<uint32_t> versions, std::vector<uint32_t> free, uint64_t sequence)
	{
		entries = std::move(live);
		std::make_heap(entries.begin(), entries.end(), FiresLater);
		slotVersions = std::move(versions);
		freeSlots = std::move(free);
		nextSequence = sequence;
		staleCount = 0;
	}

	void Scheduler::ReleaseSlot(uint32_t slot)
	{
		// The bump invalidates both the handle held by the owner and the heap entry.
		++slotVersions[slot];
		freeSlots.push_back(slot);
	}

	void Scheduler::CompactIfNeeded()
	{
		// Pop order depends only on (at, sequence), which are unique, so dropping stale entries cannot change it.
		if (staleCount < 256 || staleCount * 2 < entries.size())
			return;
		entries.erase(std::remove_if(entries.begin(), entries.end(), [this](const ScheduledEvent& e) { return IsStale(e); }), entries.end());
		std::make_heap(entries.begin(), entries.end(), FiresLater);
		staleCount = 0;
	}
}
