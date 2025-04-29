#pragma once

#include <AL/al.h>
#include <AL/alc.h>

#include "../Logger.hpp"

#include "../Camera.h"

#include "SoundInstance.hpp"
#include <SDL2/SDL_audio.h>
#include <unordered_map>

using namespace std;

class SoundManager
{

private:

	static ALCdevice* device;
	static ALCcontext* context;

	static std::unordered_map<std::string, ALuint> loadedBuffers;

public:

    // Initialize OpenAL device, context, HRTF, and distance model
    static void Initialize()
    {
        device = alcOpenDevice(nullptr);
        if (!device) {
            Logger::Log("Failed to open OpenAL device.\n");
            return;
        }

        context = alcCreateContext(device, nullptr);
        if (!context || alcMakeContextCurrent(context) == ALC_FALSE) {
            Logger::Log("Failed to create or make current OpenAL context.\n");
            if (context) alcDestroyContext(context);
            alcCloseDevice(device);
            return;
        }
        printf("Vendor:   %s\n", alGetString(AL_VENDOR));
        printf("Renderer: %s\n", alGetString(AL_RENDERER));

        // Enable HRTF if supported
        if (!alcIsExtensionPresent(device, "ALC_SOFT_HRTF"))
        {
            fprintf(stderr, "Error: ALC_SOFT_HRTF not supported\n");
        }

        if (!alcIsExtensionPresent(device, "ALC_EXT_EFX"))
        {
            fprintf(stderr, "Error: ALC_EXT_EFX not supported\n");
        }

        // Set global distance model
        alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
    }

    // Shutdown all sounds and tear down OpenAL
    static void Close()
    {

        // Clear loaded buffer cache
        for (auto& kv : loadedBuffers) {
            alDeleteBuffers(1, &kv.second);
        }
        loadedBuffers.clear();

        // Destroy context and device
        alcMakeContextCurrent(nullptr);
        if (context) alcDestroyContext(context);
        context = nullptr;
        if (device) alcCloseDevice(device);
        device = nullptr;
    }

    // Update listener and all sounds
    static void Update()
    {
        // Update listener transform based on camera
        vec3 camPos = Camera::position;
        vec3 camForward = Camera::Forward();
        vec3 camUp = Camera::Up();
        vec3 camVel = Camera::velocity; // if available

        alListener3f(AL_POSITION, camPos.x, camPos.y, camPos.z);
        alListener3f(AL_VELOCITY, camVel.x, camVel.y, camVel.z);

        float orient[6] = {
            camForward.x, camForward.y, camForward.z,
            camUp.x,      camUp.y,      camUp.z
        };
        alListenerfv(AL_ORIENTATION, orient);

    }

	static ALuint LoadOrGetSoundFileBuffer(string path)
	{
		auto foundBuffer = loadedBuffers.find(path);
		if (foundBuffer != loadedBuffers.end())
		{
			return foundBuffer->second;
		}

		SDL_AudioSpec wavSpec;
		Uint32 wavLength;
		Uint8* wavBuffer;

		if (SDL_LoadWAV(path.c_str(), &wavSpec, &wavBuffer, &wavLength) == NULL) {
			printf("Failed to load WAV file: %s\n", SDL_GetError());
			return 0;
		}

		ALenum format;

		// Determine OpenAL format
		if (wavSpec.channels == 1) {
			format = (wavSpec.format == AUDIO_U8 || wavSpec.format == AUDIO_S8) ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16;
		}
		else if (wavSpec.channels == 2) {
			format = (wavSpec.format == AUDIO_U8 || wavSpec.format == AUDIO_S8) ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16;
		}
		else {
			printf("Unsupported channel count: %d\n", wavSpec.channels);
			SDL_FreeWAV(wavBuffer);
			return 0;
		}

		ALuint buffer;
		alGenBuffers(1, &buffer);
		alBufferData(buffer, format, wavBuffer, wavLength, wavSpec.freq);
		SDL_FreeWAV(wavBuffer);

		loadedBuffers[path] = buffer;
		return buffer;
	}


	static SoundInstance* GetSoundFromPath(string path)
	{

		ALuint buffer = LoadOrGetSoundFileBuffer(path);

		ALuint source;

		// Set up OpenAL source
		alGenSources(1, &source);
		alSourcei(source, AL_BUFFER, buffer);

		SoundInstance* soundInstance = new SoundInstance(source);

		return soundInstance;

	}


};
