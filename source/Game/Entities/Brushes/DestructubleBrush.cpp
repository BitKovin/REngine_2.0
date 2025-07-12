#include <Entity.h>

class DestructubleBrush : public Entity
{
public:
	DestructubleBrush();
	~DestructubleBrush();

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
REGISTER_ENTITY(DestructubleBrush,"destructible")

DestructubleBrush::DestructubleBrush()
{
}

DestructubleBrush::~DestructubleBrush()
{
}