#pragma once

#include <glm.h>

#include <algorithm>
#include <cmath>

// Damped spring around 0 for procedural feedback (COMBAT_DESIGN.md §9.4): the victim's hit recoil and the camera's
// impact kick. Under-damped (dampingRatio < 1) it overshoots once and settles - a jolt, not a drift.
struct DampedSpring
{
	glm::vec3 value = glm::vec3(0.0f);
	glm::vec3 velocity = glm::vec3(0.0f);

	// Peak of the response to a velocity impulse v0, as a fraction of v0 / omega (omega = natural frequency in rad/s).
	static float PeakFactor(float dampingRatio)
	{
		const float zeta = std::clamp(dampingRatio, 0.0f, 0.999f);
		const float damped = std::sqrt(1.0f - zeta * zeta);
		return zeta <= 0.0f ? 1.0f : std::exp(-zeta / damped * std::atan2(damped, zeta));
	}

	// Kicks the spring so that the response peaks at `peak` (a vector: direction and size) about a quarter period later.
	void Kick(const glm::vec3& peak, float frequency, float dampingRatio)
	{
		const float omega = glm::two_pi<float>() * std::max(frequency, 0.01f);
		velocity += peak * (omega / PeakFactor(dampingRatio));
	}

	// Moves the value at once (the spring brings it back).
	void Displace(const glm::vec3& offset, float maxLength)
	{
		value += offset;
		const float length = glm::length(value);
		if (length > maxLength && length > 1e-6f)
			value *= maxLength / length;
	}

	// Exact solution of x'' = -omega^2 x - 2 zeta omega x' over deltaTime: frame-rate independent and stable for any
	// frame time (an explicit integrator loses a fifth of a stiff kick's peak at 60 Hz).
	void Step(float deltaTime, float frequency, float dampingRatio)
	{
		if (deltaTime <= 0.0f || IsSettled())
			return;

		const float omega = glm::two_pi<float>() * std::max(frequency, 0.01f);
		const float zeta = std::max(dampingRatio, 0.0f);
		const float t = deltaTime;
		const glm::vec3 x0 = value;
		const glm::vec3 v0 = velocity;

		if (zeta < 0.999f)
		{
			const float damped = omega * std::sqrt(1.0f - zeta * zeta);
			const float decay = std::exp(-zeta * omega * t);
			const float c = std::cos(damped * t);
			const float s = std::sin(damped * t);
			value = decay * (x0 * c + (v0 + zeta * omega * x0) * (s / damped));
			velocity = decay * (v0 * c - (omega * omega * x0 + zeta * omega * v0) * (s / damped));
		}
		else if (zeta <= 1.001f)
		{
			const float decay = std::exp(-omega * t);
			const glm::vec3 b = v0 + omega * x0;
			value = decay * (x0 + b * t);
			velocity = decay * (v0 - omega * b * t);
		}
		else
		{
			const float root = std::sqrt(zeta * zeta - 1.0f);
			const float r1 = -omega * (zeta - root);
			const float r2 = -omega * (zeta + root);
			const glm::vec3 c1 = (v0 - r2 * x0) / (r1 - r2);
			const glm::vec3 c2 = x0 - c1;
			const float e1 = std::exp(r1 * t);
			const float e2 = std::exp(r2 * t);
			value = c1 * e1 + c2 * e2;
			velocity = c1 * (r1 * e1) + c2 * (r2 * e2);
		}

		if (IsSettled())
			Reset();
	}

	bool IsSettled() const
	{
		return glm::dot(value, value) < 1e-10f && glm::dot(velocity, velocity) < 1e-8f;
	}

	void Reset()
	{
		value = glm::vec3(0.0f);
		velocity = glm::vec3(0.0f);
	}
};
