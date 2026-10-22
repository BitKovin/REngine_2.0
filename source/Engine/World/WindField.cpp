#include "WindField.h"

#include "../Console/ConsoleRegister.h"

#include <algorithm>
#include <cmath>

void WindField::Set(const glm::vec3& velocity, float strength, float frequency)
{
	baseVelocity = velocity;
	gustStrength = std::clamp(strength, 0.0f, 1.0f);
	gustFrequency = std::max(frequency, 0.0f);
}

glm::vec3 WindField::Sample(const glm::vec3& position, double time)
{
	const float speed = glm::length(baseVelocity);
	if (speed < 1e-4f || gustStrength <= 0.0f)
		return baseVelocity;

	// Gusts travel with the wind: the phase is the time a point downwind has waited for this air to arrive. Two
	// incommensurate waves so the gusts do not repeat visibly.
	const glm::vec3 direction = baseVelocity / speed;
	const double travel = static_cast<double>(glm::dot(position, direction)) / speed;
	const double phase = (time - travel) * gustFrequency * glm::two_pi<double>();
	const float gust = static_cast<float>(0.65 * std::sin(phase) + 0.35 * std::sin(phase * 2.37 + 1.3));
	return baseVelocity * (1.0f + gustStrength * gust);
}

CONSOLE_FUNC("wind", "wind <x> <y> <z> [gust 0..1] [gust Hz] - sets the world wind in m/s")
{
	if (args.size() < 3)
	{
		Console::Get().AddLog("Usage: wind <x> <y> <z> [gust 0..1] [gust Hz]");
		return;
	}

	const glm::vec3 velocity(Console::ArgFloat(args, 0), Console::ArgFloat(args, 1), Console::ArgFloat(args, 2));
	WindField::Set(velocity, Console::ArgFloat(args, 3, 0.0f), Console::ArgFloat(args, 4, 0.4f));
}
