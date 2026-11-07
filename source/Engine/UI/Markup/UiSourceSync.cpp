#include "UiSourceSync.h"

#ifndef DISTRIBUTION

#include <Logger.hpp>

#include <filesystem>
#include <fstream>
#include <iterator>
#include <system_error>

// Set by CMake for this file only: <game folder>/GameData.
#ifndef ENGINE_GAME_SOURCE_DATA
#define ENGINE_GAME_SOURCE_DATA ""
#endif

namespace
{
    namespace fs = std::filesystem;

    bool SameContent(const fs::path& a, const fs::path& b)
    {
        std::error_code ec;
        if (fs::file_size(a, ec) != fs::file_size(b, ec) || ec)
            return false;
        std::ifstream fa(a, std::ios::binary), fb(b, std::ios::binary);
        if (!fa || !fb)
            return false;
        return std::equal(std::istreambuf_iterator<char>(fa), std::istreambuf_iterator<char>(), std::istreambuf_iterator<char>(fb));
    }
}

std::vector<std::string> UiSourceSync::Poll(const std::vector<std::string>& paths)
{
    std::vector<std::string> replaced;
    const fs::path dataDir(ENGINE_GAME_SOURCE_DATA);
    if (dataDir.empty())
        return replaced;
    const fs::path gameDir = dataDir.parent_path();
    std::error_code ec;
    if (!fs::is_directory(dataDir, ec))
        return replaced;

    for (const std::string& path : paths)
    {
        const fs::path running(path);                       // the engine reads it relative to the working directory
        const fs::path source = gameDir / path;
        if (!fs::is_regular_file(source, ec) || !fs::is_regular_file(running, ec))
            continue;
        const auto sourceTime = fs::last_write_time(source, ec);
        if (ec) continue;
        const auto runningTime = fs::last_write_time(running, ec);
        if (ec || sourceTime == runningTime)
            continue;
        if (!SameContent(source, running))
        {
            fs::copy_file(source, running, fs::copy_options::overwrite_existing, ec);
            if (ec)
            {
                Logger::Warning("[UI] cannot copy %s over the running copy: %s", path.c_str(), ec.message().c_str());
                continue;
            }
            replaced.push_back(path);
        }
        fs::last_write_time(running, sourceTime, ec);        // equal times: not looked at again until the source changes
    }
    return replaced;
}

#endif
