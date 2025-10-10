#pragma once

#include "../../Entity.h"


class DestructibleBrush : public Entity
{
public:

	virtual void OnDamage(float Damage, Entity* DamageCauser = nullptr, Entity* Weapon = nullptr)
	{
		Destroy();
	}

	void Start()
	{
		for (auto model : Drawables)
		{
			BSPModelRef* m = (BSPModelRef*)model;

			if (m)
			{
				m->Static = true;
			}

		}
	}

private:

};
