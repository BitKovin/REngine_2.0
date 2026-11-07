#include "ConsoleStdin.h"

#include "Console.h"
#include <EngineMain.h>

#include <iostream>
#include <string>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#elif defined(__linux__)
#include <poll.h>
#include <unistd.h>
#endif

namespace
{
    bool g_enabled = false;
    std::string g_pending;

    // Bytes that can be read right now without blocking (0 when none or when stdin is closed).
    size_t Available()
    {
#ifdef _WIN32
        HANDLE in = GetStdHandle(STD_INPUT_HANDLE);
        DWORD bytes = 0;
        if (in == INVALID_HANDLE_VALUE || !PeekNamedPipe(in, nullptr, 0, nullptr, &bytes, nullptr))
            return 0;
        return bytes;
#elif defined(__linux__)
        pollfd fd{ STDIN_FILENO, POLLIN, 0 };
        return poll(&fd, 1, 0) > 0 && (fd.revents & POLLIN) ? 1 : 0;
#else
        return 0;
#endif
    }

    size_t Read(char* buffer, size_t size)
    {
#ifdef _WIN32
        DWORD got = 0;
        return ReadFile(GetStdHandle(STD_INPUT_HANDLE), buffer, static_cast<DWORD>(size), &got, nullptr) ? got : 0;
#elif defined(__linux__)
        const ssize_t got = read(STDIN_FILENO, buffer, size);
        return got > 0 ? static_cast<size_t>(got) : 0;
#else
        return 0;
#endif
    }
}

void ConsoleStdin::InitFromArguments()
{
    if (EngineMain::MainInstance != nullptr)
        g_enabled = EngineMain::MainInstance->Arguments.count("console_stdin") != 0;
    if (g_enabled)
        std::cout << std::unitbuf;   // a pipe is block-buffered by default; the driver wants each log line at once
}

void ConsoleStdin::Poll()
{
    if (!g_enabled)
        return;
    while (size_t ready = Available())
    {
        char buffer[1024];
        const size_t got = Read(buffer, ready < sizeof(buffer) ? ready : sizeof(buffer));
        if (got == 0)
            break;
        g_pending.append(buffer, got);
        if (g_pending.size() > (1u << 20))
            g_pending.clear();   // a runaway writer never fills memory
    }
    size_t end;
    while ((end = g_pending.find('\n')) != std::string::npos)
    {
        std::string line = g_pending.substr(0, end);
        g_pending.erase(0, end + 1);
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        if (!line.empty())
            Console::Get().Execute(line);
    }
}
