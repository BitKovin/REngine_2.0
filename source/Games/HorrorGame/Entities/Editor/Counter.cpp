#include <Entity.h>

class Counter : public Entity
{
public:
	
	Counter()
	{

		SaveGame = true;

	}

	int num = 0;

	int targetNum = 0;

	std::string target = "";
	std::string eventName = "reached";

	void FromData(EntityData data)
	{

		Entity::FromData(data);

		num = data.GetPropertyFloat("startNum", num);

		targetNum = data.GetPropertyFloat("targetNum", targetNum);

		target = data.GetPropertyString("target", target);
		eventName = data.GetPropertyString("onReached", eventName);

	}

	void CheckNum()
	{

		if (num == targetNum)
		{

			CallActionOnEveryEntityWithName(target, eventName);

		}

	}

	void OnAction(std::string action)
	{
		
		Entity::OnAction(action);

		if (action == "add")
		{
			num++;

			CheckNum();

		}
		else if(action == "sub")
		{

			num--;

			CheckNum();

		}

	}

	void Serialize(json& target)
	{

		Entity::Serialize(target);

		SERIALIZE_FIELD(target, num);

	}

	void Deserialize(json& source)
	{

		Entity::Deserialize(source);

		DESERIALIZE_FIELD(source, num);

	}

private:

};

REGISTER_ENTITY(Counter, "counter")