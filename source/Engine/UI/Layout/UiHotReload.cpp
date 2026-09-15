#include "UiHotReload.h"

#if UI_LAYOUT_HOT_RELOAD_ENABLED

#include "LoadedLayoutInstance.h"
#include "../UiElement.h"
#include <FileSystem/FileSystem.h>

namespace UiHotReload
{
    void PollForChangedFiles()
    {
        for (const LoadedLayoutInstancePtr& instance : LoadedLayoutRegistry::Instance().GetAll())
        {
            const uint32_t currentModTime = FileSystemEngine::GetFileModificationTime(instance->path);
            if (currentModTime != 0 && currentModTime != instance->lastLoadedModTime)
                instance->ownerElement->ReloadFromDisk(); // also refreshes lastLoadedModTime
        }
    }

    int ReloadPath(const std::string& path)
    {
        int count = 0;
        for (const LoadedLayoutInstancePtr& instance : LoadedLayoutRegistry::Instance().GetByPath(path))
        {
            instance->ownerElement->ReloadFromDisk();
            ++count;
        }
        return count;
    }

    int ReloadAll()
    {
        int count = 0;
        for (const LoadedLayoutInstancePtr& instance : LoadedLayoutRegistry::Instance().GetAll())
        {
            instance->ownerElement->ReloadFromDisk();
            ++count;
        }
        return count;
    }
}

#endif // UI_LAYOUT_HOT_RELOAD_ENABLED
