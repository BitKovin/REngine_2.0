#pragma once

#include <UI/UiCanvas.hpp>
#include "Crosshair.h"
#include "PlayerHudView.h"
#include <Entity.h>

#include "ScreenMobileControls.h"

class Player;

// Owns the player's HUD: the markup view (GameData/ui/hud/player_hud.xml, see
// PlayerHudView), the C++ crosshair and the mobile touch controls.
class PlayerHud
{
public:
	PlayerHud();
	~PlayerHud();

	void Init(Player* player);

	void Update();

	void SetVisible(bool visible);

	std::shared_ptr<ScreenMobileControls> ScreenControls;

	void ShowMessage(const std::string& message, float duration = 3.0f)
	{
		if (view) view->ShowMessage(message, duration);
	}

	void ShowMinorMessage(const std::string& message, float duration = 3.0f)
	{
		if (view) view->ShowMinorMessage(message, duration);
	}

private:

	void AdoptReloadedView();

	Player* player = nullptr;

	std::shared_ptr<PlayerHudView> view;

	std::shared_ptr<UiCanvas> crosshairCanvas;
	std::shared_ptr<UiCrosshair> crosshair;

};
