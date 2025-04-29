#pragma once
#include "../Entity.hpp"
#include "../SoundSystem/SoundInstance.hpp"
#include "../SoundSystem/SoundManager.hpp"

#include "../Delay.hpp"

class SoundPlayer: public Entity
{
public:

	SoundInstance* Sound = nullptr;

	vec3 Velocity = vec3();
	float Volume = 1;
	float Pitch = 1;
	float MinDistance = 1;
	float MaxDistance = 20;

	SoundPlayer();
	~SoundPlayer();

	void LateUpdate();

	void Play();
	void Pause();
	void Stop();


	static SoundPlayer* Create();

private:


};