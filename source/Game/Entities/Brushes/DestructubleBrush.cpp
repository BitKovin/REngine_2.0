#include <Entity.hpp>

class DestructubleBrush : public Entity
{
public:
	DestructubleBrush();
	~DestructubleBrush();

	virtual void OnDamage(float Damage, Entity* DamageCauser = nullptr, Entity* Weapon = nullptr)
	{
		Destroy();
	}

private:

};
REGISTER_LEVEL_OBJECT(DestructubleBrush,"destructible")

DestructubleBrush::DestructubleBrush()
{
}

DestructubleBrush::~DestructubleBrush()
{
}