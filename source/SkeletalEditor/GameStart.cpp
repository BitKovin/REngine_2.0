#include <Entity.hpp>

#include <EngineMain.h>

class GameStart : public Entity
{
public:
	GameStart();
	~GameStart();

	void Start()
	{
		Entity::Spawn("skeletal_editor");

		

	}

private:

};

GameStart::GameStart()
{
}

GameStart::~GameStart()
{
}

REGISTER_ENTITY(GameStart,"gamestart")