#include "EmscriptenFileSystem.h"
#include <fstream>
#include <sstream>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <Helpers/Base64.h>

EM_JS(void, SetupPersistentSystemJS, (), {
    try {
        // Prevent double-mounting which causes the "Resource busy" error
        if (window.__fs_saves_mounted) {
            return;
        }

        // Create the root mount point if it doesn't exist
        if (!FS.analyzePath('/saves').exists) {
            FS.mkdir('/saves');
        }

        // Mount IndexedDB
        FS.mount(IDBFS,{}, '/saves');
        window.__fs_saves_mounted = true; // Mark as successfully mounted

        // Sync from IDB to Memory at startup
        FS.syncfs(true, function(err) {
            if (err) {
                console.error("Initial sync error:", err);
            }
 else {
  console.log("IDBFS sync complete.");
  // NOTE: It is safe to read files from C++ only AFTER this callback fires.
}
});
}
catch (e) {
 console.error("SetupPersistentSystemJS failed:", e);
}
    });

EM_JS(void, WriteFileJS, (const char* relPath, const char* content), {
    const path = UTF8ToString(relPath);
    const data = UTF8ToString(content);

    try {
        // Handle nested directories: "saves/subdir/another/file.txt"
        const parts = path.split('/');
        let current = '';
        // Loop through parts, but skip the last one (the filename)
        for (let i = 0; i < parts.length - 1; i++) {
            current += (i ? '/' : '') + parts[i];
            if (current.length > 0 && !FS.analyzePath(current).exists) {
                FS.mkdir(current);
            }
        }

        FS.writeFile(path, data);

        // Commit to IndexedDB
        FS.syncfs(false, function(err) {
            if (err) console.error("Write sync error:", err);
        });
    }
 catch (e) {
  console.error("WriteFileJS failed:", e);
}
    });

#endif

EmscriptenFileSystem::EmscriptenFileSystem()
{
    Init();
}

bool EmscriptenFileSystem::Init() {
#ifdef __EMSCRIPTEN__
    SetupPersistentSystemJS();
#endif
    return true;
}

bool EmscriptenFileSystem::WriteSaveFile(const std::string& path, const std::string& content) {
#ifdef __EMSCRIPTEN__
    // Prepend the persistent root
    std::string fullPath = "/saves/" + path;
    WriteFileJS(fullPath.c_str(), content.c_str());
    return true;
#else
    return NativeFileSystem::WriteSaveFile(path, content);
#endif
}

std::optional<std::string> EmscriptenFileSystem::ReadSaveFile(const std::string& path) {
#ifdef __EMSCRIPTEN__
    std::string fullPath = "/saves/" + path;
    std::ifstream file(fullPath);
    if (file.is_open()) {
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
    return std::nullopt;
#else
    return NativeFileSystem::ReadSaveFile(path);
#endif
}

bool EmscriptenFileSystem::WriteSaveFileBinary(const std::string& path, const std::vector<uint8_t>& data) {
#ifdef __EMSCRIPTEN__
    // Convert to text and save
    return WriteSaveFile(path, Base64::Encode(data));
#else
    return NativeFileSystem::WriteSaveFileBinary(path, data);
#endif
}

std::optional<std::vector<uint8_t>> EmscriptenFileSystem::ReadSaveFileBinary(const std::string& path) {
#ifdef __EMSCRIPTEN__
    auto text = ReadSaveFile(path);
    if (text) {
        return Base64::Decode(*text);
    }
    return std::nullopt;
#else
    return NativeFileSystem::ReadSaveFileBinary(path);
#endif
}

std::vector<std::string> EmscriptenFileSystem::GetFilesInPath(const std::string& path) {
#ifdef __EMSCRIPTEN__
    // If the logical path starts with "SaveData/", map it to the IndexedDB mount point
    // exactly the same way WriteSaveFile/ReadSaveFile do (no stripping of "SaveData/")
    if (path.rfind("SaveData/", 0) == 0) {
        std::string fullPath = "/saves/" + path;
        return NativeFileSystem::GetFilesInPath(fullPath);
    }
    // All other paths (assets, etc.) fall back to normal MEMFS behavior
    return NativeFileSystem::GetFilesInPath(path);
#else
    return NativeFileSystem::GetFilesInPath(path);
#endif
}