#pragma once

// A continuously changing value stored as (value, time, rate) and computed on demand (rule R6, design section 4.5).
// Nothing polls it: whoever changes the rate schedules the next threshold crossing as an event, with a TimerHandle
// so the old crossing can be cancelled (see World::Schedule and Scheduler).

#include "SimTypes.h"

#include <algorithm>
#include <cmath>

namespace sim
{
	struct LazyValue
	{
		float value = 0.0f;     // value at `time`
		Tick time = 0;
		float ratePerSecond = 0.0f;
		float min = 0.0f;
		float max = 1.0f;

		float At(Tick now) const
		{
			const double elapsed = TicksToSeconds(now - time);
			return static_cast<float>(std::clamp(static_cast<double>(value) + ratePerSecond * elapsed, static_cast<double>(min), static_cast<double>(max)));
		}

		// Folds the elapsed change into `value`. Call before changing the rate.
		void Rebase(Tick now)
		{
			value = At(now);
			time = now;
		}

		void SetRate(Tick now, float newRatePerSecond)
		{
			Rebase(now);
			ratePerSecond = newRatePerSecond;
		}

		void Set(Tick now, float newValue)
		{
			value = std::clamp(newValue, min, max);
			time = now;
		}

		void Add(Tick now, float delta) { Set(now, At(now) + delta); }

		// First tick at or after `now` at which the value has reached `threshold` moving in the current rate's direction,
		// or NeverTick. Rounded up, so the value at the returned tick is already across the threshold.
		Tick CrossingTick(Tick now, float threshold) const
		{
			const float current = At(now);
			if (ratePerSecond > 0.0f)
			{
				if (current >= threshold)
					return now;
				if (threshold > max)
					return NeverTick;
				const double seconds = (static_cast<double>(threshold) - current) / ratePerSecond;
				return now + static_cast<Tick>(std::ceil(seconds * TicksPerSecond));
			}
			if (ratePerSecond < 0.0f)
			{
				if (current <= threshold)
					return now;
				if (threshold < min)
					return NeverTick;
				const double seconds = (static_cast<double>(current) - threshold) / -ratePerSecond;
				return now + static_cast<Tick>(std::ceil(seconds * TicksPerSecond));
			}
			return NeverTick;
		}

		template<class Ar> void Visit(Ar& ar)
		{
			ar("value", value);
			ar("time", time);
			ar("rate", ratePerSecond);
			ar("min", min);
			ar("max", max);
		}
	};
}
