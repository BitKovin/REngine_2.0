#pragma once
#include <cstdint>
#include <string>
#include <vector>

class UiTestRunner
{
public:
    // Reads -ui_test from EngineMain::MainInstance->Arguments. No-op when absent.
    static void InitFromArguments();
    // Call once per frame from EngineMain::MainLoop, right after Input::Update().
    static void Tick();
    static bool IsActive() { return s_active; }
    // Names of every command a script can use (built-in and registered).
    static std::vector<std::string> CommandNames();

    // Used by later phases to add commands. Return false and set `error` to fail the line.
    using CommandFn = bool(*)(const std::string& args, std::string& error);
    static void RegisterCommand(const std::string& name, CommandFn fn);

    // Waiting helpers for commands registered by later phases.
    static void WaitFrames(int frames) { s_waitFrames = frames; }

private:
    struct Line { int number; std::string command; std::string args; };
    static void RunLine(const Line& line);
    static void Fail(int lineNumber, const std::string& text);
    static void WriteResult();
    static void BeginQuit();

    static inline bool s_active = false;
    static inline std::vector<Line> s_lines;
    static inline size_t s_next = 0;
    static inline int s_waitFrames = 0;
    static inline uint64_t s_waitUntilMs = 0;
    static inline uint64_t s_startMs = 0;
    static inline bool s_failed = false;
    static inline std::string s_dir;
    static inline std::vector<std::string> s_resultLines;
    static inline int s_quitInFrames = -1;
    static inline bool s_quitIssued = false;
};
