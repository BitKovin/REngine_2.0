#include <Entity.h>
#include <Input.h>

#include <BSP/Quake3Bsp.h>
#include <UI/UiManager.h>
#include "Entities/TestBsp.h"
#include <LoadingScreen/LoadingScreenSystem.h>
#include "UI/LoadingScreen/UiDefaultLoadingScreen.h"

class GameStart : public Entity
{
public:
	GameStart();
	~GameStart();

	void Start()
	{

        LoadingScreenSystem::SetLoadingCanvas(std::make_shared<UiDefaultLoadingScreen>());

		Level::LoadLevelFromFile("GameData/maps/test.bsp");

        for (size_t i = 0; i < 000; i++)
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

        //Level::Current->AddEntity(new TestBsp());

	}

    void Update()
    {

        if (Input::IsScrenTouched)
        {
            UiManager::UiScale = 2;
        }

        if (Input::MouseDelta != vec2())
        {
            UiManager::UiScale = 1;
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

#if __EMSCRIPTEN__

    Input::AddAction("pause")->AddKeyboardKey(SDL_KeyCode::SDLK_BACKQUOTE)->AddButton(GamepadButton::Back);

#else
    Input::AddAction("pause")->AddKeyboardKey(SDL_KeyCode::SDLK_ESCAPE)->AddButton(GamepadButton::Back);
#endif

    Input::AddAction("slot1")->AddKeyboardKey(SDL_KeyCode::SDLK_1);
    Input::AddAction("slot2")->AddKeyboardKey(SDL_KeyCode::SDLK_2);
    Input::AddAction("slot3")->AddKeyboardKey(SDL_KeyCode::SDLK_3);
    Input::AddAction("slot4")->AddKeyboardKey(SDL_KeyCode::SDLK_4);
    Input::AddAction("lastSlot")->AddKeyboardKey(SDL_KeyCode::SDLK_q);

    Input::AddAction("slotMelee")->AddKeyboardKey(SDL_KeyCode::SDLK_f);

}

GameStart::~GameStart()
{
}

REGISTER_ENTITY(GameStart,"gamestart")