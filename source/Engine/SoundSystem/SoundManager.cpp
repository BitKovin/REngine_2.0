#include "SoundManager.hpp"

#include <AL/alext.h>

std::unordered_map<std::string, ALuint> SoundManager::loadedBuffers;
ALCdevice* SoundManager::device = nullptr;
ALCcontext* SoundManager::context = nullptr;

float SoundManager::GlobalVolume = 0.3f;
float SoundManager::SfxVolume = 1.0f;
float SoundManager::MusicVolume = 1.0f;

void SoundManager::Initialize()
{
    device = alcOpenDevice(nullptr);
    if (!device) {
        Logger::Log("Failed to open OpenAL device.\n");
        return;
    }

    ALCint ctxAttrs[] = {
    ALC_MONO_SOURCES,   256,
    ALC_STEREO_SOURCES,  64,
    0  // terminator
    };

    context = alcCreateContext(device, nullptr);


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

void SoundManager::Close()
{
    for (auto& kv : loadedBuffers) {
        alDeleteBuffers(1, &kv.second);
    }
    loadedBuffers.clear();

    alcMakeContextCurrent(nullptr);
    if (context) alcDestroyContext(context);
    context = nullptr;
    if (device) alcCloseDevice(device);
    device = nullptr;
}

void SoundManager::Update()
{
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

ALuint SoundManager::LoadOrGetSoundFileBuffer(std::string path)
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
        return 0;
    }

    printf("Loading WAV: channels=%d, format=%d, freq=%d\n",
        wavSpec.channels, wavSpec.format, wavSpec.freq);

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
            return 0;
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
            return 0;
        }
    }
    else {
        printf("Unsupported channel count: %d\n", wavSpec.channels);
        SDL_FreeWAV(wavBuffer);
        return 0;
    }

    ALuint buffer;
    alGenBuffers(1, &buffer);
    ALenum err = alGetError();
    if (err != AL_NO_ERROR) {
        printf("Error generating buffer: %s\n", alGetString(err));
        SDL_FreeWAV(wavBuffer);
        return 0;
    }

    alBufferData(buffer, format, wavBuffer, wavLength, wavSpec.freq);
    err = alGetError();
    if (err != AL_NO_ERROR) {
        printf("Error setting buffer data: %s\n", alGetString(err));
        alDeleteBuffers(1, &buffer);
        SDL_FreeWAV(wavBuffer);
        return 0;
    }

    SDL_FreeWAV(wavBuffer);
    loadedBuffers[path] = buffer;
    return buffer;
}

shared_ptr<SoundInstance> SoundManager::GetSoundFromPath(string path)
{
    ALuint buffer = LoadOrGetSoundFileBuffer(path);
    if (!buffer) return nullptr;

    ALuint source;
    //alGenSources(1, &source);
    //alSourcei(source, AL_BUFFER, buffer);

    return make_shared<SoundInstance>(buffer);
}
