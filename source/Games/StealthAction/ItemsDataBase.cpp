#include "ItemsDataBase.h"

#include <Helpers/CSVParser.h>

#include <FileSystem/FileSystem.h>

#include <algorithm> // for std::replace, std::transform
#include <cctype>
#include <string>    // for std::stoi, though already included indirectly

#include <Logger.hpp>

namespace
{
	std::string ToLowerCopy(const std::string& s)
	{
		std::string out = s;
		std::transform(out.begin(), out.end(), out.begin(),
			[](unsigned char c) { return (char)std::tolower(c); });
		return out;
	}
}

InventoryItemType ParseInventoryItemType(const std::string& raw, const std::string& itemIDForLogging)
{
	std::string v = ToLowerCopy(raw);

	if (v.size())
	{
		if (v[0] == 'f')		return InventoryItemType::Firearm;
		if (v[0] == 'm')		return InventoryItemType::Melee;
		if (v[0] == 't')		return InventoryItemType::Tool;
		if (v[0] == 'c')		return InventoryItemType::CustomLogic;
	}

	if (v == "firearm")     return InventoryItemType::Firearm;
	if (v == "melee")       return InventoryItemType::Melee;
	if (v == "tool")        return InventoryItemType::Tool;
	if (v == "customlogic" || v == "custom_logic" || v == "custom") return InventoryItemType::CustomLogic;

	Logger::Log("Item '" + itemIDForLogging + "' has missing/unrecognized itemType ('" + raw +
		"') in the items CSV - defaulting to CustomLogic. Expected one of: Firearm, Melee, Tool, CustomLogic.");

	return InventoryItemType::CustomLogic;
}

void ItemsDataBase::LoadItemsDataBase()
{
	auto files = FileSystemEngine::GetFilesInPath("GameData/tables/items/");

	for (auto& file : files)
	{
		loadFromCSV("GameData/tables/items/" + file);
	}

	//itemsMap = itemsMap;

}

ItemDbEntry ItemsDataBase::GetItemData(const std::string& itemID)
{
	if (itemsMap.count(itemID))
		return itemsMap[itemID];
	else
	{
		Logger::Log("Item ID not found in database: " + itemID);
		return ItemDbEntry(); // Return empty entry if not found
	}
}

// CSV column layout:
//   0 itemID
//   1 itemName
//   2 description
//   3 iconPath
//   4 modelPath
//   5 destroyOnUse
//   6 maxStackSize
//   7 weaponClassName             (Firearm / Melee / Tool items only)
//   8 itemType                    ("Firearm" | "Melee" | "Tool" | "CustomLogic")
//   9 interactionEntityClassname  (CustomLogic items only)
void ItemsDataBase::loadFromCSV(const std::string& csvText)
{

	std::string text = FileSystemEngine::ReadFile(csvText);

	text.erase(0, text.find_first_not_of("\r\n"));
	text.erase(text.find_last_not_of("\r\n") + 1);


	CSVParser parser(text);

	for (size_t row = 1; row < parser.rowCount(); ++row) {

		ItemDbEntry entry;

		entry.itemID = parser.at(row, 0).trimmed();
		entry.itemName = parser.at(row, 1).trimmed();
		entry.description = parser.at(row, 2).trimmed();
		entry.iconPath = parser.at(row, 3).trimmed();
		entry.modelPath = parser.at(row, 4).trimmed();

		std::string destroyStr = parser.at(row, 5).trimmed();
		entry.destroyOnUse = (destroyStr == "TRUE" || destroyStr == "true" || destroyStr == "1" || destroyStr == "t");

		std::string maxStr = parser.at(row, 6).trimmed();
		entry.maxStackSize = maxStr.empty() ? 1 : std::stoi(maxStr);

		entry.weaponClassName = parser.at(row, 7).trimmed();

		std::string itemTypeStr = parser.at(row, 8).trimmed();
		entry.itemType = ParseInventoryItemType(itemTypeStr, entry.itemID);

		entry.interactionEntityClassname = parser.at(row, 9).trimmed();

		itemsMap[entry.itemID] = entry;

	}
}
