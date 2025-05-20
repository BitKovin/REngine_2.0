#pragma once
#include "../Entity.hpp"
#include "../SoundSystem/SoundInstance.hpp"
#include "../SoundSystem/SoundManager.hpp"

#include "../Delay.hpp"

class SoundPlayer: public Entity
{
public:

	shared_ptr<SoundInstance> Sound = nullptr;

	vec3 Velocity = vec3();
	float Volume = 1;
	float Pitch = 1;
	float MinDistance = 1;
	float MaxDistance = 20;

	bool IsMusic = false;
	bool Is2D = false;

	bool Paused = false;

	SoundPlayer();
	~SoundPlayer();

	void SetSound(shared_ptr<SoundInstance> sound);

	void LateUpdate();

	void Play();
	void Stop();


	static SoundPlayer* Create();
	static SoundPlayer* Create(string soundPath);

private:

	float CalculateVolume();

};

#define SET_SOUND_SAFE(playerPtr, soundExpr)               \
    do {                                                  \
        shared_ptr<SoundInstance> __tmpSound = (soundExpr);          \
        if ((playerPtr) != nullptr)                       \
            (playerPtr)->SetSound(__tmpSound);            \
    } while (0)