#include "CameraImpact.h"

#include <algorithm>
#include <cmath>

namespace
{
	const glm::vec2 Down(0.0f, -1.0f);

	// Smooth pseudo-noise in [-1, 1] made only of waves at or below `frequency` (no harmonics above it: nothing the
	// frame rate turns into jumps). The two ratios differ per axis so the axes never move in lockstep; every wave
	// starts at 0, so a rumble starting from rest does not jump.
	float Wave(float time, float frequency, float ratioA, float ratioB)
	{
		const float phase = glm::two_pi<float>() * frequency * time;
		return 0.65f * std::sin(phase * ratioA) + 0.35f * std::sin(phase * ratioB);
	}

	void LimitLength(glm::vec3& v, float maxLength)
	{
		const float length = glm::length(v);
		if (length > maxLength && length > 1e-6f)
			v *= maxLength / length;
	}
}

void CameraImpact::Reset()
{
	kick.Reset();
	trauma = 0.0f;
	time = 0.0f;
	rotation = glm::vec2(0.0f);
	offset = glm::vec2(0.0f);
}

void CameraImpact::Kick(const glm::vec2& screenDirection, float impact)
{
	if (impact <= 0.0f)
		return;

	glm::vec2 direction = glm::dot(screenDirection, screenDirection) > 1e-6f ? glm::normalize(screenDirection) : Down;
	direction = glm::mix(direction, Down, std::clamp(settings.kickVerticalBias, 0.0f, 1.0f));
	direction = glm::dot(direction, direction) > 1e-6f ? glm::normalize(direction) : Down;

	const float size = std::min(impact, settings.maxKick);
	kick.Kick(glm::vec3(direction * size, 0.0f), settings.kickFrequency, settings.kickDamping);

	// Stacked kicks (a multi-hit, a burst of bullets) never move the view faster than the strongest single kick.
	LimitLength(kick.velocity, settings.MaxKickVelocity());
}

void CameraImpact::Shake(float amount)
{
	trauma = std::max(trauma, std::clamp(amount, 0.0f, 1.0f));
}

void CameraImpact::Update(float deltaTime)
{
	kick.Step(deltaTime, settings.kickFrequency, settings.kickDamping);
	LimitLength(kick.value, settings.maxKick);
	LimitLength(kick.velocity, settings.MaxKickVelocity());   // a spring swinging back from a stacked jolt included

	// Kick: the image moves along the jolt; the camera turns with it a little and slides the other way.
	const glm::vec2 image(kick.value.x, kick.value.y);
	rotation = glm::vec2(image.y, image.x) * settings.kickAngle;
	offset = -image * settings.kickOffset;

	if (trauma <= 0.0f)
	{
		time = 0.0f;
		return;
	}

	time += deltaTime;
	const float amount = trauma * trauma;
	const float f = settings.shakeFrequency;
	const float h = settings.shakeHorizontal;
	rotation += glm::vec2(Wave(time, f, 1.0f, 0.57f), h * Wave(time, f, 0.83f, 0.41f)) * (amount * settings.shakeAngle);
	offset += glm::vec2(h * Wave(time, f, 0.77f, 0.53f), Wave(time, f, 0.91f, 0.47f)) * (amount * settings.shakeOffset);
	trauma = std::max(0.0f, trauma - settings.shakeDecay * deltaTime);
}
