#include "FmodEventInstance.h"
#include "SoundManager.hpp"
#include "../EngineMain.h"

FmodEventInstance::FmodEventInstance(FMOD::Studio::EventInstance* instance)
    : eventInstance(instance)
{
    // Set user data for callback access
    if (eventInstance) {
        eventInstance->setUserData(this);
        //eventInstance->setCallback(ProgrammerSoundCallbackStatic,
        //    FMOD_STUDIO_EVENT_CALLBACK_CREATE_PROGRAMMER_SOUND |
        //    FMOD_STUDIO_EVENT_CALLBACK_DESTROY_PROGRAMMER_SOUND);
    }
}

FmodEventInstance::~FmodEventInstance() {
    // Release loaded sounds
    for (auto* sound : loadedSounds) {
        sound->release();
    }
    loadedSounds.clear();

    // Release event instance
    if (eventInstance) {
        eventInstance->setUserData(nullptr);
        eventInstance->setCallback(nullptr);
        eventInstance->release();
    }
}

void FmodEventInstance::Play() {
    if (!eventInstance) return;

    FMOD_STUDIO_PLAYBACK_STATE state;
    eventInstance->getPlaybackState(&state);

    if (state == FMOD_STUDIO_PLAYBACK_PLAYING) {
        eventInstance->stop(FMOD_STUDIO_STOP_IMMEDIATE);
    }

    eventInstance->start();
    eventInstance->setVolume(0); // Start muted until first update
}

void FmodEventInstance::Stop() {
    if (eventInstance) {
        eventInstance->stop(FMOD_STUDIO_STOP_IMMEDIATE);
    }
}

void FmodEventInstance::Update(float deltaTime) {
    if (!eventInstance) return;

    // Update base properties
    eventInstance->setVolume(GetFinalVolume());
    eventInstance->setPitch(GetPitchScale() * Pitch);
    eventInstance->setPaused(Paused || IsGamePaused());

    // Apply 3D positioning if needed
    if (!Is2D) {
        Apply3D();
    }
}

void FmodEventInstance::Apply3D() {
    FMOD_3D_ATTRIBUTES attributes = {};
    attributes.position = { Position.x, Position.y, Position.z };
    attributes.velocity = { Velocity.x, Velocity.y, Velocity.z };
    attributes.forward = { Direction.x, Direction.y, Direction.z };
    attributes.up = { 0.0f, 1.0f, 0.0f }; // Default up vector

    eventInstance->set3DAttributes(&attributes);
}

void FmodEventInstance::SetParameter(const std::string& name, float value) {
    if (eventInstance) {
        eventInstance->setParameterByName(name.c_str(), value);
    }
}

void FmodEventInstance::SetProgrammerSound(const std::string& name, FMOD::Sound* sound) {
    programmerSounds[name] = sound;
}

FMOD::Sound* FmodEventInstance::GetProgrammerSound(const std::string& name) {
    auto it = programmerSounds.find(name);
    return (it != programmerSounds.end()) ? it->second : nullptr;
}



FMOD::Sound* FmodEventInstance::GetSoundByName(const std::string& name, FMOD_STUDIO_SOUND_INFO* outInfo) {
    if (!SoundManager::studioSystem || !SoundManager::coreSystem)
        return nullptr;

    FMOD_RESULT result = SoundManager::studioSystem->getSoundInfo(name.c_str(), outInfo);
    if (result != FMOD_OK)
        return nullptr;

    FMOD::Sound* sound = nullptr;
    SoundManager::coreSystem->createSound(
        outInfo->name_or_data,
        outInfo->mode,
        &outInfo->exinfo,
        &sound
    );
    return sound;
}

// Factory methods
std::shared_ptr<FmodEventInstance> FmodEventInstance::Create(const std::string& eventPath) {
    if (!SoundManager::studioSystem)
        return nullptr;

    FMOD::Studio::EventDescription* eventDesc = nullptr;
    SoundManager::studioSystem->getEvent(eventPath.c_str(), &eventDesc);

    if (!eventDesc)
        return nullptr;

    eventDesc->loadSampleData(); // Synchronous load

    FMOD::Studio::EventInstance* instance = nullptr;
    eventDesc->createInstance(&instance);

    return instance ? std::make_shared<FmodEventInstance>(instance) : nullptr;
}

std::shared_ptr<FmodEventInstance> FmodEventInstance::CreateFromId(const std::string& guid) {
    if (!SoundManager::studioSystem)
        return nullptr;

    FMOD_GUID fmodGuid;
    FMOD_Studio_ParseID(guid.c_str(), &fmodGuid);

    FMOD::Studio::EventDescription* eventDesc = nullptr;
    SoundManager::studioSystem->getEventByID(&fmodGuid, &eventDesc);

    if (!eventDesc)
        return nullptr;

    FMOD::Studio::EventInstance* instance = nullptr;
    eventDesc->createInstance(&instance);

    return instance ? std::make_shared<FmodEventInstance>(instance) : nullptr;
}

bool FmodEventInstance::IsGamePaused() const
{
    return EngineMain::MainInstance->Paused;
}

float FmodEventInstance::GetPitchScale() const
{
    if (IsUISound)
        return 1.0f;

    return Time::GetSoundFinalTimeScale();
}

float FmodEventInstance::GetFinalVolume() const
{
    return Volume * SoundManager::GlobalVolume;
}
