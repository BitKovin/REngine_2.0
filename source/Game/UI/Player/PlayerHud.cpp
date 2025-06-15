#include "PlayerHud.hpp"
#include <EngineMain.h>

#include "../../Entities/Player/Player.hpp"

PlayerHud::PlayerHud()
{
}

PlayerHud::~PlayerHud()
{
    EngineMain::Viewport.RemoveChild(hudCanvas);
}

void PlayerHud::Init(Player* playerRef)
{


	player = playerRef;

    hudCanvas = make_shared<UiCanvas>();
    img = make_shared<UiButton>();
    crosshair = make_shared<UiImage>();

    crosshair->pivot = vec2(0.5);
    crosshair->origin = vec2(0.5);
    crosshair->size = vec2(8);
    crosshair->ImagePath = "GameData/cat.png";

    // Define a lambda function
    auto clickHandler = []() {
        std::cout << "Button clicked!" << std::endl;
        };

    // Assign it to the pointer (needs dynamic allocation)
    img->onClick = new std::function<void()>(clickHandler);


    img->position = vec2(100, 100);
    img->size = vec2(100);

    text = make_shared<UiText>();

    text->position = vec2(20, -20);

    text->origin = vec2(0, 1);
    text->pivot = vec2(0, 1);

    text->text = std::to_string((int)player->Health);

    hudCanvas->AddChild(img);
    hudCanvas->AddChild(text);
    hudCanvas->AddChild(crosshair);

    EngineMain::Viewport.AddChild(hudCanvas);

    ScreenControls = make_shared<ScreenMobileControls>();
    EngineMain::Viewport.AddChild(ScreenControls);

    slots = make_shared<WeaponSlots>();
    slots->player = player;
    slots->origin = vec2(0.5,1);
    slots->pivot = vec2(0.5, 1);
    slots->position = vec2(0,-20);
    hudCanvas->AddChild(slots);

}

void PlayerHud::Update()
{
    text->text = std::to_string((int)player->Health);
}

void WeaponSlots::Update()
{
    children.clear();

    for (WeaponSlotData data : player->weaponSlots)
    {
        if (data.className == "") continue;

        auto img = make_shared<UiImage>();
        img->size = vec2(120,120);

        if (data.slot == player->currentSlot)
        {
            img->color = vec4(1,0.5,0.5,1);
        }
        else
        {
            img->color = vec4(1, 0.8, 0.8, 0.8);
        }

        auto text = make_shared<UiText>();
        text->origin = vec2(0,1);
        text->pivot = vec2(0, 1);
        text->text = to_string(data.slot + 1);
        text->fontSize = 50;
        text->position = vec2(5,-5);

        img->AddChild(text);

        AddChild(img);

    }

    UiHorizontalBox::Update();

}
