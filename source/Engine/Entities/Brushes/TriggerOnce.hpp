#pragma once

#include "TriggerBase.hpp"

class TriggerOnce : public TriggerBase
{
public:

	TriggerOnce() : TriggerBase()
	{
		ClassName = "triggerOnce";
	}
	
	void OnBodyEntered(Body* body, Entity* entity)
	{
		TriggerBase::OnBodyEntered(body, entity);

		if (entity->HasTag("player"))
		{
			Destroy();
		}

	}

private:

};