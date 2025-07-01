#pragma once

#include "../glm.h"

class SoundInstanceBase
{
public:
	
	/// Begins or resumes playback. Will acquire a source if needed.
	virtual void Play(){}


	/// Stops and resets playback
	virtual void Stop(){}

	/// Must be called every frame to advance virtual playhead and reclaim sources
	virtual void Update(float deltaTime){}

	// ─── Public Properties ──────────────────────────────────────────────

	float Priority = 0.0f;    // higher = more important

	bool Paused = false;

	bool   Is2D = false;
	bool   IsUISound = false;
	bool   Loop = false;

	float  Volume = 1.0f;
	float  Pitch = 1.0f;
	float  MinDistance = 1.0f;
	float  MaxDistance = 20.0f;

	glm::vec3 Position = { 0,0,0 };
	glm::vec3 Velocity = { 0,0,0 };
	glm::vec3 Direction = { 0,0,1 };

protected:

	virtual bool IsGamePaused() const{ return false; }
	virtual float GetPitchScale() const { return 1; }
	virtual float GetFinalVolume() const{ return 1; }

};

