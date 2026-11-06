#include "UiTestRunner.h"

#include <EngineMain.h>
#include <Input.h>
#include <Screenshot.h>
#include <Logger.hpp>
#include <Console/Console.h>
#include <FileSystem/FileSystem.h>

#include <SDL2/SDL.h>

#include <algorithm>
#include <cstdlib>
#include <map>
#include <sstream>

namespace
{
    constexpr uint64_t kTimeoutMs = 75000;

    std::map<std::string, UiTestRunner::CommandFn>& Commands()
    {
        static std::map<std::string, UiTestRunner::CommandFn> commands;
        return commands;
    }

    std::string Trim(const std::string& s)
    {
        const size_t a = s.find_first_not_of(" \t\r\n");
        if (a == std::string::npos)
            return "";
        const size_t b = s.find_last_not_of(" \t\r\n");
        return s.substr(a, b - a + 1);
    }
}

std::vector<std::string> UiTestRunner::CommandNames()
{
    std::vector<std::string> names = { "wait", "wait_frames", "action", "screenshot", "ui_screenshot", "console", "log", "quit" };
    for (const auto& [name, fn] : Commands())
        names.push_back(name);
    std::sort(names.begin(), names.end());
    return names;
}

void UiTestRunner::RegisterCommand(const std::string& name, CommandFn fn)
{
    Commands()[name] = fn;
}

void UiTestRunner::InitFromArguments()
{
    if (EngineMain::MainInstance == nullptr)
        return;
    const auto found = EngineMain::MainInstance->Arguments.find("ui_test");
    if (found == EngineMain::MainInstance->Arguments.end() || found->second.empty())
        return;

    const std::string scriptPath = found->second[0];
    std::string stem = scriptPath;
    const size_t slash = stem.find_last_of("/\\");
    if (slash != std::string::npos)
        stem = stem.substr(slash + 1);
    const size_t dot = stem.find_last_of('.');
    if (dot != std::string::npos)
        stem = stem.substr(0, dot);

    s_dir = "SaveData/ui_test/" + stem + "/";
    s_active = true;
    s_startMs = SDL_GetTicks64();

    const std::string text = FileSystemEngine::ReadFile(scriptPath);
    if (text.empty())
    {
        Logger::Error("[UiTest] cannot read script '%s'", scriptPath.c_str());
        Fail(0, "cannot read script");
        BeginQuit();
        return;
    }

    std::istringstream stream(text);
    std::string raw;
    int number = 0;
    while (std::getline(stream, raw))
    {
        ++number;
        const size_t hash = raw.find('#');
        if (hash != std::string::npos)
            raw = raw.substr(0, hash);
        raw = Trim(raw);
        if (raw.empty())
            continue;
        Line line;
        line.number = number;
        const size_t space = raw.find_first_of(" \t");
        if (space == std::string::npos)
        {
            line.command = raw;
        }
        else
        {
            line.command = raw.substr(0, space);
            line.args = Trim(raw.substr(space + 1));
        }
        s_lines.push_back(line);
    }
    Logger::Log("[UiTest] running %s (%d commands)", scriptPath.c_str(), (int)s_lines.size());
}

void UiTestRunner::Fail(int lineNumber, const std::string& text)
{
    s_failed = true;
    s_resultLines.push_back("FAIL " + std::to_string(lineNumber) + " " + text);
}

void UiTestRunner::WriteResult()
{
    std::string out;
    for (const std::string& line : s_resultLines)
        out += line + "\n";
    out += s_failed ? "RESULT FAIL\n" : "RESULT PASS\n";
    FileSystemEngine::WriteFile(s_dir + "result.txt", out);
}

void UiTestRunner::BeginQuit()
{
    if (s_quitIssued)
        return;
    s_quitIssued = true;
    WriteResult();
    s_quitInFrames = 3;
}

void UiTestRunner::Tick()
{
    if (!s_active)
        return;

    if (s_quitInFrames >= 0)
    {
        if (--s_quitInFrames <= 0)
        {
            SDL_Event e{};
            e.type = SDL_QUIT;
            SDL_PushEvent(&e);
            s_active = false;
        }
        return;
    }

    if (SDL_GetTicks64() - s_startMs > kTimeoutMs)
    {
        Fail(0, "timeout");
        BeginQuit();
        return;
    }

    if (s_waitFrames > 0)
    {
        --s_waitFrames;
        return;
    }
    if (SDL_GetTicks64() < s_waitUntilMs)
        return;

    if (s_next >= s_lines.size())
    {
        BeginQuit();
        return;
    }

    const Line& line = s_lines[s_next++];
    RunLine(line);
}

void UiTestRunner::RunLine(const Line& line)
{
    const std::string& cmd = line.command;
    const std::string& args = line.args;
    const std::string label = cmd + (args.empty() ? "" : " " + args);

    auto ok = [&]() { s_resultLines.push_back("OK " + std::to_string(line.number) + " " + label); };

    if (cmd == "wait")
    {
        const double seconds = std::atof(args.c_str());
        s_waitUntilMs = SDL_GetTicks64() + (uint64_t)(seconds * 1000.0);
        ok();
    }
    else if (cmd == "wait_frames")
    {
        s_waitFrames = std::atoi(args.c_str());
        ok();
    }
    else if (cmd == "action")
    {
        InputAction* action = Input::GetAction(args);
        if (action == nullptr || action == &InputAction::NullAction)
        {
            Fail(line.number, label + ": unknown action");
        }
        else
        {
            action->SimulatePressed();
            ok();
        }
        s_waitFrames = 2;
    }
    else if (cmd == "screenshot")
    {
        Screenshot::Request(s_dir + args + ".png");
        s_waitFrames = 3;
        ok();
    }
    else if (cmd == "ui_screenshot")
    {
        if (EngineMain::MainInstance == nullptr || EngineMain::MainInstance->UiFrameBuffer == nullptr)
        {
            Fail(line.number, label + ": no UI frame buffer");
        }
        else
        {
            auto* ui = EngineMain::MainInstance->UiRenderTexture;
            Screenshot::RequestTarget(ui->textureHandle(), static_cast<uint16_t>(ui->width()), static_cast<uint16_t>(ui->height()), s_dir + args + ".png");
            ok();
        }
        s_waitFrames = 8;
    }
    else if (cmd == "console")
    {
        Console::Get().Execute(args);
        ok();
    }
    else if (cmd == "log")
    {
        Logger::Log("[UiTest] %s", args.c_str());
        ok();
    }
    else if (cmd == "quit")
    {
        ok();
        BeginQuit();
    }
    else
    {
        const auto found = Commands().find(cmd);
        if (found == Commands().end())
        {
            Fail(line.number, "unknown command '" + cmd + "'");
        }
        else
        {
            std::string error;
            if (found->second(args, error))
                ok();
            else
                Fail(line.number, label + ": " + error);
        }
    }

    if (s_next >= s_lines.size() && cmd != "quit")
        BeginQuit();
}
