#pragma once

#include <Entity.h>

class nav_point : public Entity
{
public:
	
	void FromData(EntityData data) override;

	float WaitTimeAfterReach = 0;

	std::string NextPoint;

private:



};

