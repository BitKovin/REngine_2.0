#include <Entity.hpp>
#include <Input.h>

class GameStart : public Entity
{
public:
	GameStart();
	~GameStart();

	void Start()
	{
		Level::OpenLevel("GameData/Maps/test.map");

        for (size_t i = 0; i < 00; i++)
        {
            float angle = i * 0.1f; // Angle step controls tightness of the spiral
            float radius = 1 + 0.05f * i; // Radius increases over time
            float x = radius * cos(angle) + 10;
            float y = 50;
            float z = radius * sin(angle);

            Entity* ent = LevelObjectFactory::instance().create("testnpc");

            ent->Position = vec3(x, y, z);
            ent->Start();

            Level::Current->AddEntity(ent);
        }

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

    Input::AddAction("bike")->AddKeyboardKey(SDL_KeyCode::SDLK_LSHIFT)->AddButton(GamepadButton::LeftShoulder);

}

GameStart::~GameStart()
{
}

REGISTER_ENTITY(GameStart,"gamestart")