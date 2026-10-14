#pragma once

#include <json.hpp>

#include <functional>
#include <string>
#include <vector>

// Edits a hand-authored JSON document in place (COMBAT_DESIGN.md §10.4). Only the values that differ between `base`
// and `edited` - two full serializations of the same data, before and after an edit - are rewritten in the text;
// everything else stays byte for byte: `_note` fields, key order, layout, how numbers are spelled.
//
//   changed value        its token is replaced (a float as the shortest text that reads back as it: 0.4, not
//                        0.4000000059604645; a token written with a decimal point keeps one)
//   key not in the text  inserted after the object's last member, in the object's style (same line, or a line of its
//                        own with the members' indentation); a missing object gets only its changed fields
//   array resized, new   written fresh: `minimize` drops default fields, small values stay on one line (tabs,
//   member               "{ ... }", "[ ... ]", keys in the order the file uses them)
//   member removed       cut with its separator
//
// Edits are relative to `base`, not to the file: anything changed in the file since `base` was read survives.
namespace JsonTextPatch
{
	using Path = std::vector<std::string>;   // keys, array indices as decimal strings
	// Default-free form of new content at `path` (identity if the caller has no defaults for it).
	using Minimize = std::function<nlohmann::json(const Path& path, const nlohmann::json& value)>;

	bool Apply(const std::string& text, const nlohmann::json& base, const nlohmann::json& edited, const Minimize& minimize,
		std::string& out, std::string& error);

	// The same edits on parsed data: `target` (a full serialization of what the file means now) takes the changes
	// between `base` and `edited`. What Apply's text must load back to.
	void Merge(nlohmann::json& target, const nlohmann::json& base, const nlohmann::json& edited);

	// Shortest text reading back as the same value (as a float when the value is one); `forceDecimal` keeps a ".0".
	std::string FormatNumber(double value, bool forceDecimal);
}
