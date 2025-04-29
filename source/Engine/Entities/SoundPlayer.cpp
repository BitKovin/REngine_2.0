#include "SoundPlayer.h"

#include "../MathHelper.hpp"

SoundPlayer::SoundPlayer()
{
}

SoundPlayer::~SoundPlayer()
{
	if (Sound)
	{
		delete(Sound);
	}
}

void SoundPlayer::LateUpdate()
{
	if (Sound)
	{

		Sound->Position = Position;
		Sound->Direction = MathHelper::GetForwardVector(Rotation);

		Sound->Volume = Volume;
		Sound->Pitch = Pitch;
		Sound->MinDistance = MinDistance;
		Sound->MaxDistance = MaxDistance;

		Sound->Update();
	}

	UpdateDestroyDelay();

}

void SoundPlayer::Play()
{
	if (Sound)
	{
		Sound->Play();
	}
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
