#include <Entity.hpp>

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



	}

private:

};
