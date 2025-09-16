#pragma once

#include "../Entity.h"

class PointLight : public Entity
{
public:

	vec3 color = vec3(1);
	float intensity = 1;
	float radius = 5;
	float innerConeAngleDegrees = 360;
	float outerConeAngleDegrees = 360;

	PointLight();
	~PointLight();

	void Finalize();

private:

};

