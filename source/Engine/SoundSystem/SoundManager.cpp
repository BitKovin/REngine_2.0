#include "SoundManager.hpp"

#include <AL/alext.h>
#include <Fmod/fmod.hpp>
#include <Fmod/fmod_errors.h>
#include <Fmod/fmod_studio.hpp>

std::unordered_map<std::string, SoundBufferData> SoundManager::loadedBuffers;
std::unordered_map<std::string, FMOD::Studio::Bank*> SoundManager::loadedBanks;
ALCdevice* SoundManager::device = nullptr;
ALCcontext* SoundManager::contextMono = nullptr;
ALCcontext* SoundManager::contextStereo = nullptr;

float SoundManager::GlobalVolume = 0.3f;
float SoundManager::SfxVolume = 1.0f; //unused after FMOD implementation
float SoundManager::MusicVolume = 1.0f; //unused after FMOD implementation

void SoundManager::InitContext(ALCcontext* context)
{
    if (!context || alcMakeContextCurrent(context) == ALC_FALSE) {
        Logger::Log("Failed to create or make current OpenAL context.\n");
        if (context) alcDestroyContext(context);
        alcCloseDevice(device);
        return;
    }

    printf("Vendor:   %s\n", alGetString(AL_VENDOR));
    printf("Renderer: %s\n", alGetString(AL_RENDERER));

    if (!alcIsExtensionPresent(device, "ALC_SOFT_HRTF"))
        fprintf(stderr, "Error: ALC_SOFT_HRTF not supported\n");

    if (!alcIsExtensionPresent(device, "ALC_EXT_EFX"))
        fprintf(stderr, "Error: ALC_EXT_EFX not supported\n");

    ALboolean hasFloat32 = alIsExtensionPresent("AL_EXT_float32");
    if (!hasFloat32)
        printf("AL_EXT_float32 not supported\n");

    alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
}

void SoundManager::InitFmod()
{

    FMOD_RESULT result;

    // Create the FMOD Studio System
    result = FMOD::Studio::System::create(&studioSystem);
    if (result != FMOD_OK)
    {
        std::cerr << "FMOD Studio create error: " << FMOD_ErrorString(result) << "\n";
        return;
    }

    // Initialize Studio system with Live Update
    result = studioSystem->initialize(
        1024,
        FMOD_STUDIO_INIT_LIVEUPDATE | FMOD_STUDIO_INIT_ALLOW_MISSING_PLUGINS,
        FMOD_INIT_NORMAL,
        nullptr
    );
    if (result != FMOD_OK)
    {
        std::cerr << "FMOD Studio init error: " << FMOD_ErrorString(result) << "\n";
        return;
    }

    // Retrieve the Core (formerly LowLevel) system
    result = studioSystem->getCoreSystem(&coreSystem);
    if (result != FMOD_OK)
    {
        std::cerr << "Failed to get Core system: " << FMOD_ErrorString(result) << "\n";
        return;
    }

}

void SoundManager::UpdateContext(ALCcontext* context)
{
    alcMakeContextCurrent(context);

    vec3 camPos = Camera::position;
    vec3 camForward = Camera::Forward();
    vec3 camUp = Camera::Up();
    vec3 camVel = Camera::velocity;

    alListener3f(AL_POSITION, camPos.x, camPos.y, camPos.z);
    alListener3f(AL_VELOCITY, camVel.x, camVel.y, camVel.z);

    float orient[6] = {
        camForward.x, camForward.y, camForward.z,
        camUp.x,      camUp.y,      camUp.z
    };
    alListenerfv(AL_ORIENTATION, orient);

}

void SoundManager::UpdateFmod()
{
    // 1) Build your FMOD_VECTORs
    FMOD_VECTOR position = { Camera::position.x, Camera::position.y, Camera::position.z };
    FMOD_VECTOR velocity = { Camera::velocity.x, Camera::velocity.y, Camera::velocity.z };
    FMOD_VECTOR forward = { Camera::Forward().x, Camera::Forward().y, Camera::Forward().z };
    FMOD_VECTOR up = { Camera::Up().x,      Camera::Up().y,      Camera::Up().z };

    // 2) Pack into the attributes struct
    FMOD_3D_ATTRIBUTES listenerAttr = { position, velocity, forward, up };

    // 3) Call the Studio API
    FMOD_RESULT result = studioSystem->setListenerAttributes(0, &listenerAttr);
    if (result != FMOD_OK)
    {
        std::cerr << "FMOD Studio setListenerAttributes failed ("
            << result << "): " << FMOD_ErrorString(result) << "\n";
    }

    studioSystem->update();

}

void SoundManager::Initialize()
{
    device = alcOpenDevice(nullptr);
    if (!device) {
        Logger::Log("Failed to open OpenAL device.\n");
        return;
    }

    ALCint ctxAttrsMono[] = {
        ALC_MONO_SOURCES,      256,
        ALC_STEREO_SOURCES,     64,
        ALC_HRTF_SOFT,       ALC_TRUE,
        //ALC_OUTPUT_LIMITER_SOFT, ALC_FALSE,
        0
    };
    contextMono = alcCreateContext(device, ctxAttrsMono);

    InitContext(contextMono);


    ALCint ctxAttrsStereo[] = {
    ALC_MONO_SOURCES,      32,
    ALC_STEREO_SOURCES,     64,
    ALC_HRTF_SOFT,       ALC_FALSE,
    //ALC_OUTPUT_LIMITER_SOFT, ALC_FALSE,
    0
    };
    contextStereo = alcCreateContext(device, ctxAttrsStereo);

    InitContext(contextStereo);

    InitFmod();


}

void SoundManager::Close()
{

    for (auto& kv : loadedBuffers) {
        alDeleteBuffers(1, &kv.second.buffer);
    }
    loadedBuffers.clear();

    alcMakeContextCurrent(nullptr);
    if (contextMono) alcDestroyContext(contextMono);
    contextMono = nullptr;
    if (device) alcCloseDevice(device);
    device = nullptr;

}

void SoundManager::Update()
{
    UpdateContext(contextMono);
    UpdateContext(contextStereo);

    UpdateFmod();

}

SoundBufferData SoundManager::LoadOrGetSoundFileBuffer(std::string path)
{
    alGetError();

    auto found = loadedBuffers.find(path);
    if (found != loadedBuffers.end()) {
        return found->second;
    }

    SDL_AudioSpec wavSpec;
    Uint32 wavLength;
    Uint8* wavBuffer;

    if (SDL_LoadWAV(path.c_str(), &wavSpec, &wavBuffer, &wavLength) == NULL) {
        printf("Failed to load WAV file: %s\n", SDL_GetError());
        return SoundBufferData();
    }

    printf("Loading WAV: channels=%d, format=%d, freq=%d\n",
        wavSpec.channels, wavSpec.format, wavSpec.freq);

    bool isStereo = wavSpec.channels > 1;
    alcMakeContextCurrent(isStereo ? contextStereo : contextMono);

    ALenum format;

    if (wavSpec.channels == 1) {
        if (wavSpec.format == AUDIO_U8)
            format = AL_FORMAT_MONO8;
        else if (wavSpec.format == AUDIO_S16LSB || wavSpec.format == AUDIO_S16MSB)
            format = AL_FORMAT_MONO16;
        else if (wavSpec.format == AUDIO_F32LSB || wavSpec.format == AUDIO_F32MSB)
            format = AL_FORMAT_MONO_FLOAT32;
        else {
            printf("Unsupported format: %d\n", wavSpec.format);
            SDL_FreeWAV(wavBuffer);
            return SoundBufferData();
        }
    }
    else if (wavSpec.channels == 2) {
        if (wavSpec.format == AUDIO_U8)
            format = AL_FORMAT_STEREO8;
        else if (wavSpec.format == AUDIO_S16LSB || wavSpec.format == AUDIO_S16MSB)
            format = AL_FORMAT_STEREO16;
        else if (wavSpec.format == AUDIO_F32LSB || wavSpec.format == AUDIO_F32MSB)
            format = AL_FORMAT_STEREO_FLOAT32;
        else {
            printf("Unsupported format: %d\n", wavSpec.format);
            SDL_FreeWAV(wavBuffer);
            return SoundBufferData();
        }
    }
    else {
        printf("Unsupported channel count: %d\n", wavSpec.channels);
        SDL_FreeWAV(wavBuffer);
        return SoundBufferData();
    }

    ALuint buffer;
    alGenBuffers(1, &buffer);
    ALenum err = alGetError();
    if (err != AL_NO_ERROR) {
        printf("Error generating buffer: %s\n", alGetString(err));
        SDL_FreeWAV(wavBuffer);
        return SoundBufferData();
    }

    alBufferData(buffer, format, wavBuffer, wavLength, wavSpec.freq);
    err = alGetError();
    if (err != AL_NO_ERROR) {
        printf("Error setting buffer data: %s\n", alGetString(err));
        alDeleteBuffers(1, &buffer);
        SDL_FreeWAV(wavBuffer);
        return SoundBufferData();
    }

    SDL_FreeWAV(wavBuffer);



    SoundBufferData data;
    data.buffer = buffer;
    data.stereo = isStereo;
    data.context = isStereo ? contextStereo : contextMono;
    
    int bytesPerSample;
    switch (wavSpec.format) {
    case AUDIO_U8:       bytesPerSample = 1; break;
    case AUDIO_S16LSB:
    case AUDIO_S16MSB:   bytesPerSample = 2; break;
    case AUDIO_F32LSB:
    case AUDIO_F32MSB:   bytesPerSample = 4; break;
    default:
        printf("Unknown format for duration calculation: %d\n", wavSpec.format);
        alDeleteBuffers(1, &buffer);
        return SoundBufferData();
    }

    int frameSize = wavSpec.channels * bytesPerSample;
    if (frameSize == 0) frameSize = 1; // safety

    Uint32 numSamples = wavLength / frameSize;
    float duration = static_cast<float>(numSamples) / wavSpec.freq;
    data.duration = duration;
    data.sampleRate = wavSpec.freq;

    loadedBuffers[path] = data;
    return data;
}

shared_ptr<SoundInstance> SoundManager::GetSoundFromPath(string path)
{
    auto buffer = LoadOrGetSoundFileBuffer(path);
    if (!buffer.context) return nullptr;

    ALuint source;
    //alGenSources(1, &source);
    //alSourcei(source, AL_BUFFER, buffer);

    return make_shared<SoundInstance>(buffer);
}

void SoundManager::CleanAllData()
{

    for (auto& kv : loadedBuffers) {
        alDeleteBuffers(1, &kv.second.buffer);
    }
    loadedBuffers.clear();

    for (auto b : loadedBanks)
    {
        if (b.second == nullptr) continue;

        b.second->unloadSampleData();
        b.second->unload();

    }

    loadedBanks.clear();

}

FMOD::Studio::Bank* SoundManager::LoadBankFromPath(const std::string& bankPath, bool loadSampleData)
{
    if (!studioSystem)
    {
        std::cerr << "[FMOD] LoadBankFromPath: studioSystem is null\n";
        return nullptr;
    }

    auto searchResult = loadedBanks.find(bankPath);

    if (searchResult != loadedBanks.end())
    {
        return searchResult->second;
    }

    // 1) Load the bank metadata (no sample data yet)
    FMOD::Studio::Bank* bank = nullptr;
    FMOD_RESULT result = studioSystem->loadBankFile(
        bankPath.c_str(),
        FMOD_STUDIO_LOAD_BANK_NORMAL,
        &bank
    );
    if (result != FMOD_OK || !bank)
    {
        std::cerr << "[FMOD] Failed to load bank \"" << bankPath << "\" ("
            << result << "): " << FMOD_ErrorString(result) << "\n";

        loadedBanks[bankPath] = nullptr;

        return nullptr;
    }
    std::cout << "[FMOD] Bank metadata loaded: " << bankPath << "\n";

    // 2) Optionally load all sample data now
    if (loadSampleData)
    {
        result = bank->loadSampleData();
        if (result != FMOD_OK)
        {
            std::cerr << "[FMOD] Warning: sample data failed for \"" << bankPath << "\" ("
                << result << "): " << FMOD_ErrorString(result) << "\n";
            // We still return bank—samples can be loaded later on-demand.
        }
        else
        {
            std::cout << "[FMOD] Sample data loaded for: " << bankPath << "\n";
        }
    }

    loadedBanks[bankPath] = bank;

    return bank;
}
