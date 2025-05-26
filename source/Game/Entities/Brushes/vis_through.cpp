#include <Entity.hpp>
#include <BSP/Quake3Bsp.h>

class vis_through : public Entity
{
public:

	vis_through()
	{
		Static = true;
	}

	~vis_through() = default;


private:

};

REGISTER_ENTITY(vis_through, "vis_through")