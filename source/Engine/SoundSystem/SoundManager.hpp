#pragma once

#include <AL/al.h>
#include <AL/alc.h>

#include "fmod_include.h"


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
    static std::unordered_map<std::string, FMOD::Studio::Bank*> loadedBanks;

    static void InitContext(ALCcontext* context);

    static void InitFmod();

    static void UpdateContext(ALCcontext* context);



public:

    inline static FMOD::Studio::System* studioSystem = nullptr;
    inline static FMOD::System* coreSystem = nullptr;

    static float GlobalVolume;
    static float SfxVolume;
    static float MusicVolume;

    static void UpdateFmod();

    static void Initialize();
    static void Close();
    static void Update();
    static SoundBufferData LoadOrGetSoundFileBuffer(std::string path);
    static shared_ptr<SoundInstance> GetSoundFromPath(std::string path);

    static void CleanAllData();

    /// Loads one FMOD Studio bank from disk.
/// @param studioSystem    Your initialized FMOD::Studio::System*
/// @param bankPath        Full path to the bank file (e.g. ".../Master.bank").
/// @param loadSampleData  If true, calls bank->loadSampleData() immediately.
/// @return                Pointer to the loaded Bank, or nullptr if loading failed.
    static FMOD::Studio::Bank* LoadBankFromPath(
        const std::string& bankPath,
        bool                  loadSampleData = true
    );

};
