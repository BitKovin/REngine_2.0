#pragma once

#include <json.hpp>

#include <string>
#include <vector>

// Problems found while loading one combat data file. Errors reject the file (on dev reload the previous
// data stays active); warnings are logged and the data is used.
struct DataLoadReport
{
	std::string Source;
	std::vector<std::string> Errors;
	std::vector<std::string> Warnings;

	bool Ok() const { return Errors.empty(); }
	void Error(const std::string& path, const std::string& message) { Errors.push_back(path + ": " + message); }
	void Warning(const std::string& path, const std::string& message) { Warnings.push_back(path + ": " + message); }
	void Log() const;
};

// Warns about keys present in `source` but absent from `reserialized` (the parsed struct written back with
// to_json): exactly the keys no struct field consumed, i.e. typos or stale fields (the "remove" operations of
// json::diff). Keys starting with '_' are comments.
void ReportUnknownKeys(const nlohmann::json& source, const nlohmann::json& reserialized, const std::string& path, DataLoadReport& report);

// Parses text (comments allowed) without throwing. Returns false and records an error on failure.
bool ParseDataJson(const std::string& text, nlohmann::json& out, DataLoadReport& report);

// Converts parsed JSON to T with the struct's NLOHMANN from_json; type errors are reported, not thrown.
template<typename T>
bool ConvertDataJson(const nlohmann::json& source, T& out, const std::string& path, DataLoadReport& report)
{
	try
	{
		out = source.get<T>();
	}
	catch (const nlohmann::json::exception& e)
	{
		report.Error(path, e.what());
		return false;
	}

	ReportUnknownKeys(source, nlohmann::json(out), path, report);
	return true;
}
