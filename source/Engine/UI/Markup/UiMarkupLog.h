#pragma once
#include <string>

namespace UiMarkupLog
{
    // "[UI] <path>:<line>:<col>: error: <message>" (conventions §9). line/col 0 when unknown.
    void Error(const std::string& path, int line, int col, const std::string& message);
    void Warning(const std::string& path, int line, int col, const std::string& message);
    int ErrorCount();     // since process start
    int WarningCount();
    // 1-based line and column of a byte offset in `text`.
    void LineColumn(const std::string& text, size_t offset, int& line, int& col);
}
