#include <Entity.h>
#include <BSP/Quake3Bsp.h>

class WorldSpawn : public Entity
{
public:

	WorldSpawn()
	{
		Static = true;
	}
	~WorldSpawn() = default;
	


	void FromData(EntityData data)
	{
		Entity::FromData(data);

		printf("world spawn created\n");

		CQuake3BSP::lightVolGridSize = data.GetPropertyVector("gridsize", vec3(64, 64, 128));

	}

private:

};
