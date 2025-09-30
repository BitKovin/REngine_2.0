#pragma once

#include "../../Entity.h"
#include "../../BSP/Quake3Bsp.h"

class vis_through : public Entity
{
public:

	vis_through()
	{
		Static = true;
	}

	~vis_through() = default;

	void Start()
	{
		for (IDrawMesh* mesh : Drawables)
		{
			mesh->Transparent = true;
		}
	}

private:

};