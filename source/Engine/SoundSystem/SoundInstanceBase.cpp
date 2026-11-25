#include "SoundInstanceBase.h"

#include <EngineMain.h>

bool SoundInstanceBase::IsGamePaused() const
{

    if(EngineMain::MainInstance->LoadingFrames > 0)
        return true;// so no sound plays while finishinh loading level

    if (IsUISound) return false;
    return EngineMain::MainInstance->Paused;
}
