#include "PlayerHud.hpp"
#include <EngineMain.h>

#include "../../Entities/Player/Player.hpp"

PlayerHud::PlayerHud()
{
}

PlayerHud::~PlayerHud()
{
    EngineMain::Viewport.RemoveChild(img);
    EngineMain::Viewport.RemoveChild(text);
}

void PlayerHud::Init(Player* playerRef)
{
	player = playerRef;

    img = make_shared<UiButton>();

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

    EngineMain::Viewport.AddChild(img);
    EngineMain::Viewport.AddChild(text);

}

void PlayerHud::Update()
{
    text->text = std::to_string((int)player->Health);
}
