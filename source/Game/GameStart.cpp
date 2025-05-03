#include <Entity.hpp>
#include <Input.h>

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
    Input::AddAction("forward")->AddKeyboardKey(SDL_KeyCode::SDLK_w);
    Input::AddAction("backward")->AddKeyboardKey(SDL_KeyCode::SDLK_s);
    Input::AddAction("left")->AddKeyboardKey(SDL_KeyCode::SDLK_a);
    Input::AddAction("right")->AddKeyboardKey(SDL_KeyCode::SDLK_d);

    Input::AddAction("jump")->AddKeyboardKey(SDL_KeyCode::SDLK_SPACE)->AddButton(GamepadButton::A);

    Input::AddAction("attack")->AddButton(GamepadButton::RightTrigger)->LMB = true;

    Input::AddAction("qSave")->AddKeyboardKey(SDL_KeyCode::SDLK_F5);
    Input::AddAction("qLoad")->AddKeyboardKey(SDL_KeyCode::SDLK_F8);

    Input::AddAction("bike")->AddKeyboardKey(SDL_KeyCode::SDLK_LSHIFT);

}

GameStart::~GameStart()
{
}

REGISTER_ENTITY(GameStart,"gamestart")