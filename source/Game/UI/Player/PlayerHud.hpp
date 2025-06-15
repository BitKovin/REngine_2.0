#pragma once

#include <UI/UiViewport.hpp>
#include <UI/UiCanvas.hpp>
#include <UI/UiText.hpp>
#include <UI/UiHorizontalBox.hpp>
#include <UI/UiButton.hpp>
#include <UI/UiImage.hpp>
#include <UI/UiHorizontalBox.hpp>
#include <Entity.h>

#include "ScreenMobileControls.h"

class Player;

class WeaponSlots : public UiHorizontalBox
{
public:
	
	Player* player = nullptr;

	void Update();

private:

};



class PlayerHud
{
public:
	PlayerHud();
	~PlayerHud();

	void Init(Player* player);

	void Update();

	std::shared_ptr<ScreenMobileControls> ScreenControls;

private:

	Player* player = nullptr;


	std::shared_ptr<UiCanvas> hudCanvas;

	std::shared_ptr<WeaponSlots> slots;

	std::shared_ptr<UiButton> img;
	std::shared_ptr<UiText> text;
	std::shared_ptr<UiImage> crosshair;

};