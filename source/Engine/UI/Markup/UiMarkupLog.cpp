#include "UiMarkupLog.h"

#include <Logger.hpp>

namespace
{
    int g_errors = 0;
    int g_warnings = 0;
}

void UiMarkupLog::Error(const std::string& path, int line, int col, const std::string& message)
{
    ++g_errors;
    Logger::Error("[UI] %s:%d:%d: error: %s", path.c_str(), line, col, message.c_str());
}

void UiMarkupLog::Warning(const std::string& path, int line, int col, const std::string& message)
{
    ++g_warnings;
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
