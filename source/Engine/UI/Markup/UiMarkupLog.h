#pragma once
#include <functional>
#include <string>

namespace UiMarkupLog
{
    // Receives every message too (lint and Studio collect them). Pass an empty function to remove it.
    using Sink = std::function<void(bool isError, const std::string& path, int line, int col, const std::string& message)>;
    void SetSink(Sink sink);

    // "[UI] <path>:<line>:<col>: error: <message>" (conventions §9). line/col 0 when unknown.
    void Error(const std::string& path, int line, int col, const std::string& message);
    void Warning(const std::string& path, int line, int col, const std::string& message);
    int ErrorCount();     // since process start
    int WarningCount();
    // 1-based line and column of a byte offset in `text`.
    void LineColumn(const std::string& text, size_t offset, int& line, int& col);
}
