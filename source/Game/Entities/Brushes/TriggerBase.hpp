#include <Entity.hpp>
#include <Physics.h>

class TriggerBase : public Entity
{
public:

	string targetName = "";
	string onEnterAction = "trigger_enter";
	string onExitAction = "trigger_exit";

	TriggerBase()
	{
		ClassName = "trigger";
		SaveGame = true;
	}

	void Start();

	void FromData(EntityData data);

	void OnBodyEntered(Body* body, Entity* entity);
	void OnBodyExited(Body* body, Entity* entity);

private:

};