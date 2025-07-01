#pragma once
#include <Fmod/fmod_studio.hpp>
#include <Fmod/fmod.hpp>
#include <unordered_map>
#include <string>
#include <vector>
#include "../glm.h"
#include "SoundInstance.hpp"

class FmodEventInstance : public SoundInstanceBase
{
public:
    FmodEventInstance(FMOD::Studio::EventInstance* instance);
    ~FmodEventInstance();

    // SoundInstanceBase overrides
    void Play() override;
    void Stop() override;
    void Update(float deltaTime) override;

    // Custom methods
    void SetParameter(const std::string& name, float value);
    void SetProgrammerSound(const std::string& name, FMOD::Sound* sound);
    FMOD::Sound* GetProgrammerSound(const std::string& name);
    void SetSoundTableKey(const std::string& key) { soundTableKey = key; }
    void SetDefaultProgrammerSound(FMOD::Sound* sound) { defaultProgrammerSound = sound; }

    static std::shared_ptr<FmodEventInstance> Create(const std::string& eventPath);
    static std::shared_ptr<FmodEventInstance> CreateFromId(const std::string& guid);

protected:

    bool IsGamePaused() const;
    float GetPitchScale() const;
    float GetFinalVolume() const;

private:
    FMOD::Studio::EventInstance* eventInstance = nullptr;
    std::unordered_map<std::string, FMOD::Sound*> programmerSounds;
    FMOD::Sound* defaultProgrammerSound = nullptr;
    std::string soundTableKey;
    std::vector<FMOD::Sound*> loadedSounds; // For sounds created in callbacks

    //// Callback handling
    //static FMOD_RESULT F_CALLBACK ProgrammerSoundCallbackStatic(
    //    FMOD_STUDIO_EVENT_CALLBACK_TYPE type,
    //    FMOD_STUDIO_EVENTINSTANCE* event,
    //    void* parameters);

    //FMOD_RESULT ProgrammerSoundCallback(
    //    FMOD_STUDIO_EVENT_CALLBACK_TYPE type,
    //    void* parameters);

    void Apply3D();

    FMOD::Sound* GetSoundByName(const std::string& name, FMOD_STUDIO_SOUND_INFO* outInfo);
    
};