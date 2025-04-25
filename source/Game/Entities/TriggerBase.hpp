#include <Entity.hpp>
#include <Physics.h>

class TriggerBase : public Entity
{
public:

	void Start();

	void OnBodyEntered(Body* body, Entity* entity);
	void OnBodyExited(Body* body, Entity* entity);

private:

};