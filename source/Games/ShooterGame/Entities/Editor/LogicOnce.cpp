#include <Entity.h>

class LogicOnce : public Entity
{
public:

	LogicOnce()
	{

		SaveGame = true;

	}

	std::string target = "";

	void FromData(EntityData data)
	{

		Entity::FromData(data);

		target = data.GetPropertyString("target", target);

	}

	void OnAction(std::string action)
	{

		Entity::OnAction(action);

		if (target != "")
		{

			CallActionOnEveryEntityWithName(target, action);

			Destroy();

		}

	}

private:

};

