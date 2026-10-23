#pragma once

// Every emitted event is logged with its cause (rule R3). The log is the source for the why-trace, entity timelines
// and the event stream. It is history, not state: it is saved, but the state hash does not include it.

#include "SimTypes.h"

#include <array>
#include <cstring>
#include <vector>

namespace sim
{
	class TypeRegistry;

	struct EventRecord
	{
		EventId id = NoEvent;
		Tick tick = 0;
		EventId cause = NoEvent; // the event whose rule or command produced this one; NoEvent for roots
		TypeId type = InvalidType;
		RuleId rule = NoRule;    // the rule that issued it, or one of the pseudo rules (InputRule, SystemRule, SetupRule)
		uint16_t wave = 0;       // cascade depth within its tick
		alignas(8) std::array<uint8_t, MaxPayloadSize> payload{};

		// The payload as T, or nullptr if this record is another type.
		template<class T>
		const T* As(const TypeRegistry& types) const;
	};

	// Fixed-capacity ring of the most recent events. Ids are contiguous, so lookup by id is O(1) while the event is
	// still retained. History compaction to significant events (design section 4.9) is not implemented yet.
	class EventLog
	{
	public:
		explicit EventLog(size_t capacity = 262144) : capacity(capacity) { records.reserve(capacity < 4096 ? capacity : 4096); }

		void Append(const EventRecord& record)
		{
			if (records.size() < capacity)
				records.push_back(record);
			else
				records[head] = record, head = (head + 1) % capacity;
		}

		const EventRecord* Find(EventId id) const
		{
			if (records.empty() || id == NoEvent)
				return nullptr;
			const EventId first = Oldest().id;
			if (id < first || id > first + records.size() - 1)
				return nullptr;
			return &At(static_cast<size_t>(id - first));
		}

		size_t Size() const { return records.size(); }
		size_t Capacity() const { return capacity; }

		// 0 is the oldest retained record.
		const EventRecord& At(size_t index) const { return records[(head + index) % records.size()]; }
		const EventRecord& Oldest() const { return At(0); }
		const EventRecord& Newest() const { return At(records.size() - 1); }

		void Clear()
		{
			records.clear();
			head = 0;
		}

	private:
		size_t capacity;
		size_t head = 0; // index of the oldest record once the ring is full
		std::vector<EventRecord> records;
	};
}
