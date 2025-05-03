#pragma once

#include <AL/al.h>
#include <AL/alc.h>


#include "../Logger.hpp"

#include "../Camera.h"

#include "SoundInstance.hpp"

#include <AL/alext.h>

#include <SDL2/SDL_audio.h>
#include <unordered_map>

#include <memory>

using namespace std;

class SoundManager
{

private:

	static ALCdevice* device;
	static ALCcontext* context;

	static std::unordered_map<std::string, ALuint> loadedBuffers;

public:

    static float GlobalVolume;
    static float SfxVolume;
    static float MusicVolume;

    // Initialize OpenAL device, context, HRTF, and distance model
    static void Initialize();

    // Shutdown all sounds and tear down OpenAL
    static void Close();

    // Update listener and all sounds
    static void Update();

	static ALuint LoadOrGetSoundFileBuffer(std::string path);


    static shared_ptr<SoundInstance> GetSoundFromPath(string path);


};
