#include <Entity.hpp>

class GameStart : public Entity
{
public:
	GameStart();
	~GameStart();

	void Start()
	{
		Level::LoadLevelFromFile("GameData/Maps/test.map");
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