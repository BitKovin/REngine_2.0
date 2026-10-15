#include "RandomHelper.h"

#include <cstdint>
#include <functional>
#include <limits>
#include <random>
#include <thread>

namespace
{
	// One generator per thread. rand() kept its state per thread on MSVC (every worker thread started from the same
	// seed, so they produced identical sequences) and is not thread-safe on other platforms; entities call this from
	// the parallel update. Each thread is seeded once from random_device mixed with its id.
	std::mt19937& Generator()
	{
		thread_local std::mt19937 generator = []
		{
			std::random_device device;
			const auto thread = static_cast<std::uint32_t>(std::hash<std::thread::id>{}(std::this_thread::get_id()));
			std::seed_seq seed{ device(), device(), thread };
			return std::mt19937(seed);
		}();
		return generator;
	}
}

float RandomHelper::RandomFloat()
{
	return std::uniform_real_distribution<float>(0.0f, 1.0f)(Generator());
}

int RandomHelper::RandomInt()
{
	return std::uniform_int_distribution<int>(0, std::numeric_limits<int>::max())(Generator());
}

vec3 RandomHelper::RandomPosition(float radius)
{
    return glm::vec3(
        (RandomFloat() - 0.5f) * 2.0f * radius,
        (RandomFloat() - 0.5f) * 2.0f * radius,
        (RandomFloat() - 0.5f) * 2.0f * radius
    );
}
