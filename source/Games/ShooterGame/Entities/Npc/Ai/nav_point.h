#pragma once

#include <Entity.h>

class nav_point : public Entity
{
public:
	
	void FromData(EntityData data) override;

	float WaitTimeAfterReach = 0;
	float acceptanceRadius = 0.3f;
	std::string NextPoint;

private:



};

