#pragma once

#include <AL/al.h>
#include <AL/alc.h>


#include <SDL2/SDL_audio.h>
#include <unordered_map>
#include <memory>
#include <string>

#include "SoundBufferData.h"

#include "../Logger.hpp"
#include "../Camera.h"
#include "SoundInstance.hpp"



using namespace std;

class SoundManager
{
private:
    static ALCdevice* device;
    static ALCcontext* contextMono;
    static ALCcontext* contextStereo;

    static std::unordered_map<std::string, SoundBufferData> loadedBuffers;

    static void InitContext(ALCcontext* context);

    static void UpdateContext(ALCcontext* context);

public:
    static float GlobalVolume;
    static float SfxVolume;
    static float MusicVolume;

    static void Initialize();
    static void Close();
    static void Update();
    static SoundBufferData LoadOrGetSoundFileBuffer(std::string path);
    static shared_ptr<SoundInstance> GetSoundFromPath(std::string path);
};
