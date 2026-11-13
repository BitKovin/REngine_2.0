#pragma once

#include <UI/Elements/UiElemPanel.h>
#include <Delay.hpp>

#include <string>
#include <vector>

class Player;
class UiElemImage;

// The player HUD, built from GameData/ui/hud/player_hud.xml: weapon grid (top
// centre), body figure + stamina bar (bottom left), weapon icon with magazine and total
// (bottom right), use prompt and messages. PlayerHud owns it and calls
// Refresh() once per player update.
class PlayerHudView : public UiElemPanel
{
public:

	PlayerHudView();
	~PlayerHudView() override;

	// The newest view built. UI hot reload replaces the HUD with a fresh
	// PlayerHudView (registered as the layout's code-behind); PlayerHud picks it up from here.
	static PlayerHudView* Latest() { return latest; }

	void Refresh(Player* player);

	// Main thread, every frame (also while the game is paused): fades the HUD
	// out under pages with hides-hud="true".
	void Tick() override;

	// The C++ crosshair; hidden together with the HUD and while the use prompt shows.
	std::weak_ptr<UiElement> crosshair;

	void ShowMessage(const std::string& message, float duration);
	void ShowMinorMessage(const std::string& message, float duration);

	// How long the weapon grid stays up after a switch, before it fades.
	static constexpr float GridShowTime = 2.2f;

private:

	void RefreshGrid(Player* player);
	void RefreshGridStates(Player* player);
	void RefreshGridLabels(Player* player);
	void RefreshVitals(Player* player);
	void RefreshAmmo(Player* player);
	void RefreshUse(Player* player);

	void SetHeight(UiElem* element, float height, float& last);

	struct Cell
	{
		UiElem* element = nullptr;
		std::string uuid;
		bool pair = false;
		int column = 0;
		bool cur = false, empty = false;
	};

	UiElem* gridArea = nullptr;
	UiElem* grid = nullptr;
	UiElem* message = nullptr;
	UiElem* minor = nullptr;
	UiElem* use = nullptr;
	UiElem* useBar = nullptr;
	UiElem* bodyBase = nullptr;
	UiElem* bodyBlood = nullptr;
	UiElem* bodyRed = nullptr;
	UiElem* staminaBar = nullptr;
	UiElem* stamina = nullptr;
	UiElem* reserve = nullptr;
	UiElem* magazine = nullptr;
	UiElem* curWeapon = nullptr;
	UiElem* curIcon = nullptr;
	UiElem* curIconBack = nullptr;

	std::vector<UiElem*> columns;
	std::vector<Cell> cells;

	// The left-hand column (before column 1).
	struct OffhandCell
	{
		UiElem* element = nullptr;
		int index = 0;          // into Player::offhandWeapons
	};
	UiElem* offhandColumn = nullptr;
	std::vector<OffhandCell> offhandCells;
	std::vector<std::string> lastOffhandList;
	int lastOffhandSelected = -2;
	int lastOffhandBlocked = -1;
	std::string lastLabelKey;

	int lastGridRevision = -1;
	int lastShowRequests = -1;
	int lastCurrentColumn = -2;
	Delay gridShowDelay;
	bool gridShown = false;

	Delay messageDelay;
	Delay minorMessageDelay;
	bool messageShown = false;
	bool minorShown = false;

	std::string silhouette;
	float lastBlood = -1.0f;
	float lastStamina = -1.0f;

	std::string lastMagazineText = "\x01";
	std::string lastReserveText = "\x01";
	std::string lastWeaponKey = "\x01";
	int lastIconRevision = -1;
	int lastWeaponEmpty = -1;
	int lastLow = -1;
	int lastStaminaLow = -1;
	int lastUseVisible = -1;
	float lastUseProgress = -1.0f;
	int lastHidden = -1;

	static inline PlayerHudView* latest = nullptr;

};
