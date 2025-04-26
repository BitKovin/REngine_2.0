#pragma once

#include <UI/UiViewport.hpp>
#include <UI/UiCanvas.hpp>
#include <UI/UiText.hpp>
#include <UI/UiHorizontalBox.hpp>
#include <UI/UiButton.hpp>
#include <Entity.hpp>

class Player;

class PlayerHud
{
public:
	PlayerHud();
	~PlayerHud();

	void Init(Player* player);

	void Update();

private:

	Player* player = nullptr;

	std::shared_ptr<UiButton> img;

	std::shared_ptr<UiText> text;

};