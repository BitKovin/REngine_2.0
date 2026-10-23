#pragma once

// Deterministic random numbers for the sim (rule R9). Never use std::rand, std::random_device, std::*_distribution
// (their output is implementation defined) or the engine's random helpers inside the sim.

#include <cstdint>
#include <string_view>

namespace sim
{
	inline uint64_t SplitMix64(uint64_t value)
	{
		value += 0x9E3779B97F4A7C15ull;
		value = (value ^ (value >> 30)) * 0xBF58476D1CE4E5B9ull;
		value = (value ^ (value >> 27)) * 0x94D049BB133111EBull;
		return value ^ (value >> 31);
	}

	inline uint64_t HashCombine(uint64_t a, uint64_t b) { return SplitMix64(a ^ SplitMix64(b)); }

	inline uint64_t HashString(std::string_view text)
	{
		uint64_t hash = 14695981039346656037ull;
		for (const char c : text)
		{
			hash ^= static_cast<uint8_t>(c);
			hash *= 1099511628211ull;
		}
		return hash;
	}

	// PCG32 (O'Neill, XSH-RR). Small, fast, fully specified.
	class Rng
	{
	public:
		Rng() = default;
		explicit Rng(uint64_t seed, uint64_t stream = 0) { Seed(seed, stream); }

		void Seed(uint64_t seed, uint64_t stream = 0)
		{
			state = 0;
			increment = (stream << 1u) | 1u;
			NextU32();
			state += seed;
			NextU32();
		}

		uint32_t NextU32()
		{
			const uint64_t old = state;
			state = old * 6364136223846793005ull + increment;
			const uint32_t xorShifted = static_cast<uint32_t>(((old >> 18u) ^ old) >> 27u);
			const uint32_t rotation = static_cast<uint32_t>(old >> 59u);
			return (xorShifted >> rotation) | (xorShifted << ((32u - rotation) & 31u));
		}

		// Uniform in [0, bound) without modulo bias.
		uint32_t Below(uint32_t bound)
		{
			if (bound <= 1)
				return 0;
			const uint32_t threshold = (0u - bound) % bound;
			for (;;)
			{
				const uint32_t value = NextU32();
				if (value >= threshold)
					return value % bound;
			}
		}

		// Uniform in [min, max] (inclusive).
		int32_t Range(int32_t min, int32_t max)
		{
			if (max <= min)
				return min;
			return min + static_cast<int32_t>(Below(static_cast<uint32_t>(max - min) + 1u));
		}

		// Uniform in [0, 1). 24 bits, exactly representable as float.
		float Unit() { return static_cast<float>(NextU32() >> 8) * (1.0f / 16777216.0f); }
		float Range(float min, float max) { return min + (max - min) * Unit(); }
		bool Chance(float probability) { return Unit() < probability; }

		template<class Ar> void Visit(Ar& ar) { ar("state", state); ar("increment", increment); }

	private:
		uint64_t state = 0x853C49E6748FEA9Bull;
		uint64_t increment = 0xDA3E39CB94B95BDBull;
	};
}
