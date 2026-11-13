#include "PlayerHudView.h"

#include "../../Entities/Player/Player.hpp"
#include "../../Entities/Player/Weapons/WeaponFirearm.h"

#include <WeaponDefs.h>

#include <UI/Markup/UiBuilder.h>
#include <UI/Markup/UiTagRegistry.h>
#include <UI/Menus/UiMenuStack.h>
#include <UI/Menus/UiInputPrompts.h>
#include <Input.h>
#include <FileSystem/FileSystem.h>
#include <Logger.hpp>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <set>

namespace
{
	constexpr const char* kLayout = "GameData/ui/hud/player_hud.xml";
	constexpr const char* kMissingIcon = "GameData/textures/ui/weapon_icons/missing.svg";
	constexpr const char* kDefaultSilhouette = "GameData/textures/ui/hud/silhouettes/default.png";

	constexpr float kBodyHeight = 212.0f;   // .body-layer height in player_hud.css
	constexpr float kBarHeight = 168.0f;    // .stamina height
	constexpr float kLowStamina = 0.25f;
	constexpr float kMaxStamina = 3.0f;

	// Icons present in the running GameData, re-read whenever the grid is rebuilt
	// (the development icon baker bumps the grid revision after writing one).
	std::set<std::string> iconFiles;

	void ScanIcons()
	{
		iconFiles.clear();
		for (const std::string& file : FileSystemEngine::GetFilesInPath("GameData/textures/ui/weapon_icons/"))
			iconFiles.insert(file);
	}

	std::string IconFor(const std::string& className)
	{
		const WeaponDef* def = WeaponDefs::Find(className);
		if (def == nullptr)
			return kMissingIcon;

		const std::string path = def->IconPath();
		if (iconFiles.count(className + ".png"))
			return path;

		static std::set<std::string> warned;
		if (warned.insert(className).second)
			Logger::Warning("PlayerHud: no icon %s yet (it is generated in development builds)", path.c_str());
		return kMissingIcon;
	}
}

// Hot reload rebuilds the HUD through UiElem::Load(kLayout): with this, it builds a PlayerHudView again.
UI_REGISTER_CODE_BEHIND(kLayout, PlayerHudView);

PlayerHudView::~PlayerHudView()
{
	if (latest == this)
		latest = nullptr;
}

PlayerHudView::PlayerHudView()
{
	latest = this;
	LoadLayout(kLayout);

	gridArea = FindElem("gridArea");
	grid = FindElem("grid");
	message = FindElem("message");
	minor = FindElem("minor");
	use = FindElem("use");
	useBar = FindElem("useBar");
	bodyBase = FindElem("bodyBase");
	bodyBlood = FindElem("bodyBlood");
	bodyRed = FindElem("bodyRed");
	staminaBar = FindElem("staminaFill");
	stamina = FindElem("stamina");
	reserve = FindElem("reserve");
	magazine = FindElem("magazine");
	curWeapon = FindElem("curWeapon");
	curIcon = FindElem("curIcon");
	curIconBack = FindElem("curIconBack");

	Style().Width = UiLength{ UiLenKind::Percent, 0.f, 100.f };
	Style().Height = UiLength{ UiLenKind::Percent, 0.f, 100.f };

	if (gridArea) gridArea->SetClass("hidden", true);   // markup classes can't be removed from code, so the start state is set here
	if (message) message->SetAttribute("visible", "false");
	if (minor) minor->SetAttribute("visible", "false");
}

void PlayerHudView::ShowMessage(const std::string& text, float duration)
{
	SetVar("message", text);
	messageDelay.AddDelay(duration);
}

void PlayerHudView::ShowMinorMessage(const std::string& text, float duration)
{
	SetVar("minorMessage", text);
	minorMessageDelay.AddDelay(duration);
}

void PlayerHudView::SetHeight(UiElem* element, float height, float& last)
{
	if (element == nullptr)
		return;
	height = std::round(height);
	if (height == last)
		return;
	last = height;
	element->Style().Height = UiLength{ UiLenKind::Px, height };
}

void PlayerHudView::Tick()
{
	UiElemPanel::Tick();

	// The pause menu and other pages with hides-hud="true" fade the HUD out.
	const int hidden = UiMenus::Stack().HidesHud() ? 1 : 0;
	if (hidden != lastHidden)
	{
		SetClass("off", hidden != 0);   // `hud` is the layout root, which FindElem does not return: the class goes on the view itself
		lastHidden = hidden;
	}

	if (auto cross = crosshair.lock())
		cross->visible = hidden == 0 && lastUseVisible != 1;
}

void PlayerHudView::Refresh(Player* player)
{
	if (player == nullptr)
		return;

	const bool showMessage = messageDelay.Wait();
	if (showMessage != messageShown && message)
	{
		message->SetAttribute("visible", showMessage ? "true" : "false");
		messageShown = showMessage;
	}
	const bool showMinor = minorMessageDelay.Wait();
	if (showMinor != minorShown && minor)
	{
		minor->SetAttribute("visible", showMinor ? "true" : "false");
		minorShown = showMinor;
	}

	RefreshGrid(player);
	RefreshVitals(player);
	RefreshAmmo(player);
	RefreshUse(player);
}

// ── weapon grid ──────────────────────────────────────────────────────────────

void PlayerHudView::RefreshGrid(Player* player)
{
	const bool slotsMode = player->GetWeaponSystemMode() == WeaponSystemMode::Slots;

	if (player->weaponGridRevision != lastGridRevision || player->offhandWeapons != lastOffhandList)
	{
		lastGridRevision = player->weaponGridRevision;
		lastOffhandList = player->offhandWeapons;

		if (grid != nullptr)
		{
			ScanIcons();
			grid->ClearChildren();
			columns.clear();
			cells.clear();
			offhandColumn = nullptr;
			offhandCells.clear();

			// The left-hand column comes first: one cell per offhand item (index 0
			// of offhandWeapons is the empty hand, which has no cell).
			if (auto col = UiBuilder::BuildSnippet(*this, *grid, "GridColumn"))
			{
				offhandColumn = col.get();
				col->SetClass("offhand", true);
				for (int index = 1; index < (int)player->offhandWeapons.size(); ++index)
				{
					auto cell = UiBuilder::BuildSnippet(*this, *col, "GridCell");
					if (!cell)
						continue;
					if (UiElem* image = cell->FindElem("icon"))
						image->SetAttribute("src", IconFor(player->offhandWeapons[index]));
					offhandCells.push_back({ cell.get(), index });
				}
				col->SetClass("none", offhandCells.empty());
			}

			for (int column = 0; column < 6; ++column)
			{
				auto col = UiBuilder::BuildSnippet(*this, *grid, "GridColumn");
				if (!col)
					continue;
				columns.push_back(col.get());

				bool any = false;
				for (const WeaponGridEntry& entry : player->weaponGrid)
				{
					if (entry.column != column)
						continue;
					any = true;

					auto cell = UiBuilder::BuildSnippet(*this, *col, entry.pair ? "GridPairCell" : "GridCell");
					if (!cell)
						continue;

					const std::string icon = IconFor(entry.className);
					for (const char* id : { "icon", "back", "front" })
					{
						const bool single = std::strcmp(id, "icon") == 0;
						if (single == entry.pair)
							continue;   // single cells have "icon", pair cells "back" and "front"
						if (UiElem* image = cell->FindElem(id))
							image->SetAttribute("src", icon);
					}

					cells.push_back({ cell.get(), entry.uuid, entry.pair, entry.column });
				}

				col->SetClass("none", !any);   // nothing on this key: the number stays, faint
			}
		}

		lastCurrentColumn = -2;
		lastOffhandSelected = -2;
		lastOffhandBlocked = -1;
		lastLabelKey.clear();
		for (Cell& cell : cells)
		{
			cell.cur = false;
			cell.empty = false;
		}

		// Building the grid (game start) and every selection change show it for a moment.
		gridShowDelay.AddDelay(GridShowTime);
	}

	if (player->weaponGridShowRequests != lastShowRequests)
	{
		if (lastShowRequests >= 0)
			gridShowDelay.AddDelay(GridShowTime);
		lastShowRequests = player->weaponGridShowRequests;
	}

	RefreshGridStates(player);

	const bool show = slotsMode && gridShowDelay.Wait() && (!cells.empty() || !offhandCells.empty());
	if (show != gridShown && gridArea != nullptr)
	{
		gridArea->SetClass("hidden", !show);
		gridShown = show;
	}
}

void PlayerHudView::RefreshGridStates(Player* player)
{
	const WeaponGridEntry& current = player->currentEntry;

	for (Cell& cell : cells)
	{
		const bool cur = cell.uuid == current.uuid && cell.pair == current.pair;
		if (cur != cell.cur)
		{
			cell.element->SetClass("cur", cur);
			cell.cur = cur;
		}

		WeaponGridEntry entry;
		entry.uuid = cell.uuid;
		entry.pair = cell.pair;
		const bool empty = player->IsGridEntryEmpty(entry);
		if (empty != cell.empty)
		{
			cell.element->SetClass("empty", empty);
			cell.empty = empty;
		}
	}

	// The lit column: where the last grid input went (the left hand, or the weapon in hand).
	const int currentColumn = player->gridCursorOffhand ? -1 : (current.Empty() ? -2 : current.column);
	if (currentColumn != lastCurrentColumn)
	{
		for (int i = 0; i < (int)columns.size(); ++i)
			columns[i]->SetClass("on", i == currentColumn);
		if (offhandColumn) offhandColumn->SetClass("on", currentColumn == -1);
		lastCurrentColumn = currentColumn;
	}

	// Left hand: the chosen item, and whether the weapon in hand leaves the left hand free.
	const int selected = player->desiredOffhandWeapon;
	if (selected != lastOffhandSelected)
	{
		for (const OffhandCell& cell : offhandCells)
			cell.element->SetClass("cur", cell.index == selected);
		lastOffhandSelected = selected;
	}
	const int blocked = player->disableOffhandWeapon ? 1 : 0;
	if (blocked != lastOffhandBlocked && offhandColumn != nullptr)
	{
		offhandColumn->SetClass("blocked", blocked != 0);
		lastOffhandBlocked = blocked;
	}

	RefreshGridLabels(player);
}

// Column labels follow the device. Keyboard: the bound key of each column (F,
// 1..6). Gamepad: the columns have no buttons of their own, so the D-pad
// glyphs sit where they lead (left / right of the lit column, up / down on it).
void PlayerHudView::RefreshGridLabels(Player* player)
{
	if (offhandColumn == nullptr)
		return;

	// Visual columns: 0 is the left hand, 1..6 the weapon columns.
	std::vector<UiElem*> all;
	all.push_back(offhandColumn);
	all.insert(all.end(), columns.begin(), columns.end());

	const bool gamepad = UiInputPrompts::LastDevice() == UiInputDevice::Gamepad;
	const int lit = lastCurrentColumn == -1 ? 0 : (lastCurrentColumn >= 0 ? lastCurrentColumn + 1 : -1);

	std::string key = (gamepad ? "pad:" : "kb:") + std::to_string(lit) + ":" + std::to_string(UiInputPrompts::DeviceRevision());
	if (key == lastLabelKey)
		return;
	lastLabelKey = key;

	auto keyName = [](const char* action) -> std::string
		{
			InputAction* input = Input::GetAction(action);
			if (input == nullptr || input == &InputAction::NullAction || input->keys.empty())
				return std::string();
			return SDL_GetKeyName(SDL_GetKeyFromScancode(input->keys[0]));
		};

	auto glyph = [](const char* action) { return std::string("<icon=action:") + action + ",valign=middle>"; };

	// Where the D-pad leads from the lit column: the next column with something selectable.
	auto rowsIn = [&](int visual)
		{
			if (visual == 0)
				return (int)offhandCells.size();
			int count = 0;
			for (const Cell& cell : cells)
				if (cell.column == visual - 1 && !cell.empty)
					++count;
			return count;
		};
	int left = -1, right = -1;
	if (gamepad && lit >= 0)
	{
		for (int v = lit - 1; v >= 0 && left < 0; --v)
			if (rowsIn(v) > 0) left = v;
		for (int v = lit + 1; v < (int)all.size() && right < 0; ++v)
			if (rowsIn(v) > 0) right = v;
		if (lit == 0)
			right = player->currentEntry.Empty() ? right : player->currentEntry.column + 1;   // back to the weapon in hand
	}

	static const char* kSlotActions[] = { "slot1", "slot2", "slot3", "slot4", "slot5", "slot6" };
	for (int v = 0; v < (int)all.size(); ++v)
	{
		std::string text;
		if (!gamepad)
			text = keyName(v == 0 ? "slotMelee" : kSlotActions[v - 1]);
		else if (v == left)
			text = glyph("gridLeft");
		else if (v == right)
			text = glyph("gridRight");
		else if (v == lit && rowsIn(v) > 1)
			text = glyph("gridUp") + glyph("gridDown");
		all[v]->SetVar("num", text);
	}
}

// ── body figure and stamina bar ────────────────────────────────────────────────

void PlayerHudView::RefreshVitals(Player* player)
{
	// The character's own figure (Player::hudSilhouette, a PNG): drawn as it is, and again tinted red under the blood clip.
	std::string figure = player->hudSilhouette.empty() ? kDefaultSilhouette : player->hudSilhouette;
	if (figure != silhouette)
	{
		silhouette = figure;
		if (bodyBase) bodyBase->SetAttribute("src", figure);
		if (bodyRed) bodyRed->SetAttribute("src", figure);
	}

	// Dried blood rises from the feet as health drops.
	const float health = player->MaxHealth > 0 ? std::clamp(player->Health / player->MaxHealth, 0.0f, 1.0f) : 0.0f;
	SetHeight(bodyBlood, kBodyHeight * (1.0f - health), lastBlood);

	// Stamina: the bar beside the figure.
	const float staminaPart = std::clamp(player->stamina / kMaxStamina, 0.0f, 1.0f);
	SetHeight(staminaBar, kBarHeight * staminaPart, lastStamina);

	const int staminaLow = staminaPart < kLowStamina ? 1 : 0;
	if (staminaLow != lastStaminaLow && stamina != nullptr)
	{
		stamina->SetClass("low", staminaLow != 0);
		lastStaminaLow = staminaLow;
	}
}

// ── weapon in hand and ammunition ──────────────────────────────────────────────────────────────────────────────────────────

void PlayerHudView::RefreshAmmo(Player* player)
{
	Weapon* weapon = player->currentWeapon;
	WeaponFirearm* firearm = dynamic_cast<WeaponFirearm*>(weapon);

	// The weapon in hand, drawn as its icon (a pair shows two copies). A rebake
	// bumps the grid revision, which picks the new picture up here too.
	const bool pair = firearm != nullptr && firearm->akimbo;
	std::string weaponKey;
	if (weapon != nullptr)
		weaponKey = weapon->ClassName + (pair ? "|pair" : "");
	if (weaponKey != lastWeaponKey || lastIconRevision != lastGridRevision)
	{
		lastWeaponKey = weaponKey;
		lastIconRevision = lastGridRevision;

		const std::string icon = weapon != nullptr ? IconFor(weapon->ClassName) : std::string();
		if (curIcon) curIcon->SetAttribute("src", icon);
		if (curIconBack)
		{
			curIconBack->SetAttribute("src", icon);
			curIconBack->SetAttribute("visible", pair ? "true" : "false");
		}
		if (curWeapon)
		{
			curWeapon->SetClass("pair", pair);
			curWeapon->SetAttribute("visible", weapon != nullptr ? "true" : "false");
		}
	}

	const int weaponEmpty = !player->currentEntry.Empty() && player->IsGridEntryEmpty(player->currentEntry) ? 1 : 0;
	if (weaponEmpty != lastWeaponEmpty && curWeapon != nullptr)
	{
		curWeapon->SetClass("empty", weaponEmpty != 0);
		lastWeaponEmpty = weaponEmpty;
	}


	// Rounds in the magazine (down left) and the total carried (up right). Melee shows neither.
	std::string magazineText, reserveText;
	int low = 0;
	if (firearm != nullptr && firearm->GetAmmoType() != WeaponAmmoType::None)
	{
		const int rounds = std::max(0, firearm->Data.magazineAmmo);
		magazineText = std::to_string(rounds);
		reserveText = "+ " + std::to_string(player->GetAmmo(firearm->GetAmmoType()));
		low = rounds * 4 <= firearm->params.magazineSize ? 1 : 0;
	}

	if (magazineText != lastMagazineText)
	{
		lastMagazineText = magazineText;
		SetVar("magazineText", magazineText);
		if (magazine) magazine->SetAttribute("visible", magazineText.empty() ? "false" : "true");
	}
	if (reserveText != lastReserveText)
	{
		lastReserveText = reserveText;
		SetVar("reserveText", reserveText);
		if (reserve) reserve->SetAttribute("visible", reserveText.empty() ? "false" : "true");
	}
	if (low != lastLow && magazine != nullptr)
	{
		magazine->SetClass("low", low != 0);
		lastLow = low;
	}
}

// ── use prompt ───────────────────────────────────────────────────────────────

void PlayerHudView::RefreshUse(Player* player)
{
	const int visible = player->currentInteractionObject != nullptr ? 1 : 0;
	if (visible != lastUseVisible && use != nullptr)
	{
		use->SetAttribute("visible", visible ? "true" : "false");
		lastUseVisible = visible;
	}

	const float progress = std::round(std::clamp(player->interactionProgress, 0.0f, 1.0f) * 100.0f) / 100.0f;
	if (progress != lastUseProgress && useBar != nullptr)
	{
		char value[16];
		std::snprintf(value, sizeof(value), "%.2f", progress);
		useBar->SetAttribute("value", value);
		lastUseProgress = progress;
	}
}
