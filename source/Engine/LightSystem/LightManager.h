#pragma once

#include "../glm.h"

class LightManager
{
public:
	LightManager();
	~LightManager();

	static void Update();

	static vec3 LightDirection;

	static float LightDistance1;
	static float LightDistance2;
	static float LightDistance3;
	static float LightDistance4;
	static int ShadowMapResolution;
	static float LightDistanceMultiplier;

	static mat4 lightView1;
	static mat4 lightProjection1;

	static mat4 lightView2;
	static mat4 lightProjection2;

	static mat4 lightView3;
	static mat4 lightProjection3;

	static mat4 lightView4;
	static mat4 lightProjection4;

private:

	static void CalculateLightMatrices(
		float lightDistance,
		glm::mat4& outLightView,
		glm::mat4& outLightProjection
	);

};

