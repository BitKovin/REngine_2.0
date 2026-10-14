#include "DataLoadReport.h"

#include <Helpers/StringHelper.h>
#include <Logger.hpp>

void DataLoadReport::Log() const
{
	for (const std::string& error : Errors)
		Logger::Error("[%s] %s", Source.c_str(), error.c_str());
	for (const std::string& warning : Warnings)
		Logger::Warning("[%s] %s", Source.c_str(), warning.c_str());
}

void ReportUnknownKeys(const nlohmann::json& source, const nlohmann::json& reserialized, const std::string& path, DataLoadReport& report)
{
	// Source keys that did not survive the round trip are the ones the data structs don't know.
	for (const nlohmann::json& operation : nlohmann::json::diff(source, reserialized))
	{
		if (operation.at("op") != "remove")
			continue;

		const std::string pointer = operation.at("path").get<std::string>();   // "/moves/light_1/sped"
		const std::vector<std::string> tokens = StringHelper::Split(pointer.substr(1), '/');
		if (!tokens.back().empty() && tokens.back()[0] == '_')
			continue;   // "_comment"-style keys are allowed anywhere

		const std::string keyPath = StringHelper::Join(tokens, ".");
		report.Warning(path.empty() ? keyPath : path + "." + keyPath, "unknown key (typo?) - ignored");
	}
}

bool ParseDataJson(const std::string& text, nlohmann::json& out, DataLoadReport& report)
{
	if (text.empty())
	{
		report.Error("file", "empty or could not be read");
		return false;
	}

	out = nlohmann::json::parse(text, nullptr, /*allow_exceptions*/ false, /*ignore_comments*/ true);
	if (out.is_discarded())
	{
		report.Error("file", "invalid JSON syntax");
		return false;
	}
	if (!out.is_object())
	{
		report.Error("file", "root must be a JSON object");
		return false;
	}
	return true;
}
