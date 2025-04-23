#include <Entity.hpp>

#include <EngineMain.h>
#include <Input.h>

class GameStart : public Entity
{
public:
	GameStart();
	~GameStart();

	void Start()
	{

		Input::AddAction("forward")->AddKeyboardKey(SDL_KeyCode::SDLK_w);
		Input::AddAction("backward")->AddKeyboardKey(SDL_KeyCode::SDLK_s);
		Input::AddAction("left")->AddKeyboardKey(SDL_KeyCode::SDLK_a);
		Input::AddAction("right")->AddKeyboardKey(SDL_KeyCode::SDLK_d);

		Input::AddAction("rmb")->RMB = true;

		Entity::Spawn("freecamera");
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