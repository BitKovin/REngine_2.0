#pragma once

#include "../glm.h"

class LightManager
{
public:
	LightManager();
	~LightManager();

	static void Update();

	static vec3 LightDirection;

	static float LightDistance;
	static int ShadowMapResolution;
	static float LightDistanceMultiplier;

	static mat4 lightView;
	static mat4 lightProjection;

private:

	static void CalculateLightMatrices(
		float lightDistance,
		glm::mat4& outLightView,
		glm::mat4& outLightProjection
	);

};

