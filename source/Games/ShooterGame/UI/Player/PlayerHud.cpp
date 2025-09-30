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
    crosshair = make_shared<UiImage>();



    crosshair->pivot = vec2(0.5);
    crosshair->origin = vec2(0.5);
    crosshair->size = vec2(12);
    crosshair->ImagePath = "GameData/textures/ui/crosshair.png";


    text = make_shared<UiText>();

    text->position = vec2(20, -20);

    text->origin = vec2(0, 1);
    text->pivot = vec2(0, 1);

    text->text = std::to_string((int)player->Health);


    hudCanvas->AddChild(text);
    hudCanvas->AddChild(crosshair);



    ScreenControls = make_shared<ScreenMobileControls>();
    EngineMain::Viewport.AddChild(ScreenControls);
    EngineMain::Viewport.AddChild(hudCanvas);

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

    if (oldSlot == player->currentSlot && oldSlots == player->weaponSlots)
    {
        UiHorizontalBox::Update();
        return;
    }

    children.clear();

    for (WeaponSlotData data : player->weaponSlots)
    {
        if (data.className == "") continue;

        auto img = make_shared<UiButton>();
        img->size = vec2(120,120);

        if (data.slot == player->currentSlot)
        {
            img->color = vec4(1,0.5,0.5,1);
        }
        else
        {
            img->color = vec4(1, 0.8, 0.8, 0.8);
        }

        img->OnlyTouch = true;
        img->OnlyNotPaused = true;

        img->onClick = [this, data]() {
            player->SwitchToSlot(data.slot);
            };


        auto text = make_shared<UiText>();
        text->origin = vec2(0,1);
        text->pivot = vec2(0, 1);
        text->text = to_string(data.slot + 1);
        text->fontSize = 50;
        text->position = vec2(5,-5);

        img->AddChild(text);

        AddChild(img);

    }

    oldSlot = player->currentSlot;
    oldSlots = player->weaponSlots;

    UiHorizontalBox::Update();

}
