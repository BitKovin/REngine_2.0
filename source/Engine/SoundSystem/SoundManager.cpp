#include "SoundManager.hpp"

#include <AL/alext.h>

std::unordered_map<std::string, SoundBufferData> SoundManager::loadedBuffers;
ALCdevice* SoundManager::device = nullptr;
ALCcontext* SoundManager::contextMono = nullptr;
ALCcontext* SoundManager::contextStereo = nullptr;

float SoundManager::GlobalVolume = 0.3f;
float SoundManager::SfxVolume = 1.0f;
float SoundManager::MusicVolume = 1.0f;

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
        ALC_OUTPUT_LIMITER_SOFT, ALC_FALSE,
        0
    };
    contextMono = alcCreateContext(device, ctxAttrsMono);

    InitContext(contextMono);


    ALCint ctxAttrsStereo[] = {
    ALC_MONO_SOURCES,      32,
    ALC_STEREO_SOURCES,     64,
    ALC_HRTF_SOFT,       ALC_FALSE,
    ALC_OUTPUT_LIMITER_SOFT, ALC_FALSE,
    0
    };
    contextStereo = alcCreateContext(device, ctxAttrsStereo);

    InitContext(contextStereo);


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
