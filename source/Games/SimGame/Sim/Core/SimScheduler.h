#pragma once

// The scheduler holds only the moments when something must happen (rule R6): events to emit at a future tick.
// Binary heap ordered by (tick, sequence) with lazy cancellation (addendum section 13): cancelling bumps the version
// of the handle's slot, and a popped entry whose version no longer matches is dropped.

#include "SimArchive.h"
#include "SimTypes.h"

#include <array>
#include <vector>

namespace sim
{
	// A cancellable reference to a scheduled event. Store it in a component; it survives save and load.
	struct TimerHandle
	{
		static constexpr uint32_t NoSlot = 0xFFFFFFFFu;

		uint32_t slot = NoSlot;
		uint32_t version = 0;

		bool IsSet() const { return slot != NoSlot; }

		template<class Ar> void Visit(Ar& ar) { ar("slot", slot); ar("version", version); }
	};

	struct ScheduledEvent
	{
		Tick at = 0;
		uint64_t sequence = 0;
		uint32_t slot = TimerHandle::NoSlot;
		uint32_t version = 0;
		EventId cause = NoEvent;
		RuleId rule = NoRule;
		TypeId type = InvalidType;
		alignas(8) std::array<uint8_t, MaxPayloadSize> payload{};
	};

	class Scheduler
	{
	public:
		// `withHandle` false: fire-and-forget, cannot be cancelled (the returned handle is unset).
		TimerHandle Schedule(Tick at, TypeId type, const void* payload, size_t payloadSize, EventId cause, RuleId rule, bool withHandle);

		// Returns true if the handle referred to a pending event. Resets the handle.
		bool Cancel(TimerHandle& handle);

		bool IsPending(const TimerHandle& handle) const;

		// Pops the earliest live entry with `at <= upTo`. Stale entries are discarded on the way.
		bool PopDue(Tick upTo, ScheduledEvent& out);

		size_t LiveCount() const { return entries.size() - staleCount; }
		size_t EntryCount() const { return entries.size(); }

		// Live entries in firing order (tools and saves).
		std::vector<ScheduledEvent> SortedLiveEntries() const;

		void Clear();

		// Saved state: the live entries plus the slot table and sequence counter, so handles stay valid after a load.
		uint64_t NextSequence() const { return nextSequence; }
		const std::vector<uint32_t>& SlotVersions() const { return slotVersions; }
		const std::vector<uint32_t>& FreeSlots() const { return freeSlots; }
		void Restore(std::vector<ScheduledEvent> live, std::vector<uint32_t> versions, std::vector<uint32_t> free, uint64_t sequence);

	private:
		std::vector<ScheduledEvent> entries; // binary min-heap on (at, sequence)
		std::vector<uint32_t> slotVersions;
		std::vector<uint32_t> freeSlots;
		uint64_t nextSequence = 1;
		size_t staleCount = 0;

		bool IsStale(const ScheduledEvent& entry) const { return entry.slot != TimerHandle::NoSlot && slotVersions[entry.slot] != entry.version; }
		void ReleaseSlot(uint32_t slot);
		void CompactIfNeeded();
	};
}
