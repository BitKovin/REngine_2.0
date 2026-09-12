#pragma once

#include <map>
#include <string>

// Item type defines what happens when the item is selected.
//
// Firearm / Melee / Tool items equip into Player's 3-slot loadout for that
// type (see Player::firearmSlotUUID / meleeSlotUUID / toolSlotUUID) and
// become candidates for the currentWeapon role-switching system.
// CustomLogic items run their interactionEntityClassname logic and never
// touch currentWeapon at all (consumables, keys, quest items, etc.).
enum class InventoryItemType
{
	Firearm,
	Melee,
	Tool,
	CustomLogic
};

// Parses the itemType CSV cell. Case-insensitive. Unknown/blank values fall
// back to CustomLogic (the safe no-op type) and log a warning rather than
// silently guessing Firearm/Melee/Tool, since guessing wrong would put an
// item in the wrong 3-slot loadout array.
InventoryItemType ParseInventoryItemType(const std::string& raw, const std::string& itemIDForLogging);

struct ItemDbEntry
{
	std::string itemID;
	std::string itemName;
	std::string description;
	std::string iconPath;
	std::string modelPath;

	bool destroyOnUse = false; // Whether the item should be removed from inventory when used (for consumables, etc.)
	int maxStackSize = 1;      // Maximum stack size for this item (for stackable items)

	std::string weaponClassName;            // Entity className to spawn for Firearm / Melee / Tool items
	std::string interactionEntityClassname; // For CustomLogic items that spawn an entity when used

	InventoryItemType itemType = InventoryItemType::CustomLogic;
};

class ItemsDataBase
{
public:

	static void LoadItemsDataBase();

	static ItemDbEntry GetItemData(const std::string& itemID);

private:

	static void loadFromCSV(const std::string& csvText);

	static inline std::map<std::string, ItemDbEntry> itemsMap;

};
