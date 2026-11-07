#include "UiMarkupLog.h"

#include <Logger.hpp>

namespace
{
    int g_errors = 0;
    int g_warnings = 0;
    UiMarkupLog::Sink g_sink;
}

void UiMarkupLog::SetSink(Sink sink) { g_sink = std::move(sink); }

void UiMarkupLog::Error(const std::string& path, int line, int col, const std::string& message)
{
    ++g_errors;
    if (g_sink)
        g_sink(true, path, line, col, message);
    Logger::Error("[UI] %s:%d:%d: error: %s", path.c_str(), line, col, message.c_str());
}

void UiMarkupLog::Warning(const std::string& path, int line, int col, const std::string& message)
{
    ++g_warnings;
    if (g_sink)
        g_sink(false, path, line, col, message);
    Logger::Warning("[UI] %s:%d:%d: warning: %s", path.c_str(), line, col, message.c_str());
}

int UiMarkupLog::ErrorCount() { return g_errors; }
int UiMarkupLog::WarningCount() { return g_warnings; }

void UiMarkupLog::LineColumn(const std::string& text, size_t offset, int& line, int& col)
{
    if (offset > text.size())
        offset = text.size();
    line = 1;
    size_t lastNewline = std::string::npos;
    for (size_t i = 0; i < offset; ++i)
    {
        if (text[i] == '\n')
        {
            ++line;
            lastNewline = i;
        }
    }
    col = static_cast<int>(lastNewline == std::string::npos ? offset : offset - lastNewline - 1) + 1;
}
