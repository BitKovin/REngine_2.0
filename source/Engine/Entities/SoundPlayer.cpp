#include "SoundPlayer.h"

#include "../MathHelper.hpp"

SoundPlayer::SoundPlayer()
{
}

SoundPlayer::~SoundPlayer()
{

}

void SoundPlayer::SetSound(shared_ptr<SoundInstance> sound)
{
	Sound = sound;
}

void SoundPlayer::LateUpdate()
{
	if (Sound.get())
	{

		Sound->Position = Position;
		Sound->Direction = MathHelper::GetForwardVector(Rotation);

		Sound->Volume = CalculateVolume();
		Sound->Pitch = Pitch;
		Sound->MinDistance = MinDistance;
		Sound->MaxDistance = MaxDistance;

		Sound->Update();
	}

	UpdateDestroyDelay();

}

void SoundPlayer::Play()
{
	if (Sound.get())
	{
		LateUpdate();
		Sound->Play();
	}
}

float SoundPlayer::CalculateVolume()
{
	return Volume * SoundManager::GlobalVolume * (IsMusic ? SoundManager::MusicVolume : SoundManager::SfxVolume);
}

void SoundPlayer::Pause()
{
	if (Sound)
	{
		Sound->Pause();
	}
}

void SoundPlayer::Stop()
{
	if (Sound)
	{
		Sound->Stop();
	}
}

SoundPlayer* SoundPlayer::Create()
{

	SoundPlayer* player = new SoundPlayer();

	Level::Current->AddEntity(player);
	player->Start();

	return player;

}
