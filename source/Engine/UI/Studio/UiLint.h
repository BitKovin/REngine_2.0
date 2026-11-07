#pragma once
#include <json.hpp>
#include <string>
#include <vector>

// `-ui_lint`: loads every .xml and .css under GameData/ui/ through the real parser, builder and style code and reports
// problems with their source positions. `-ui_schema <file>`: writes the schema part of the UI database and exits.
// Both run during startup and request a quit; Studio runs the same code through the bridge (command "lint").
namespace UiLint
{
    struct Diagnostic
    {
        std::string Severity;   // "error", "warning" or "info"
        std::string Code;       // stable id such as "markup.parse" or "resource.missing"
        std::string Path;
        int Line = 0, Column = 0;
        std::string Element;    // tag, selector or key the message is about, when known
        std::string Message;
        std::string Help;       // optional hint on how to fix it
    };

    // Lints the whole UI folder. Never modifies source files. Results are sorted, so two runs give the same list.
    std::vector<Diagnostic> Run();
    nlohmann::json ToJson(const std::vector<Diagnostic>& diagnostics);
    // The text report (one line per diagnostic, then LINT PASS or LINT FAIL <n>).
    std::string ToText(const std::vector<Diagnostic>& diagnostics);

    // Writes SaveData/ui_lint.txt and SaveData/ui_lint.json.
    void WriteReports(const std::vector<Diagnostic>& diagnostics);

    // Handles -ui_lint and -ui_schema from the launch arguments. Returns true when it ran one and asked the engine to quit.
    bool HandleLaunchArguments();

    // ui_lint: logs the summary (the reports are written as for the launch argument).
    void RegisterConsoleCommands();
}
