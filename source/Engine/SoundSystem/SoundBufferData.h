#pragma once


#include <AL/al.h>
#include <AL/alc.h>

struct SoundBufferData
{
	ALCcontext* context;
	bool stereo;
	ALuint buffer;
};