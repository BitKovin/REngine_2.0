#pragma once
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_timer.h>
#include <vector>

// Represents a temporary time scale modifier
typedef struct {
    float remainingDuration;   // in seconds (unscaled)
    float timeScale;          // multiplier to apply
    bool affectSound;         // whether this effect applies to sound
} TimeScaleEffect;

class Time {
public:
    static double DeltaTime;           // scaled delta time (seconds)
    static float DeltaTimeF;           // scaled delta time (seconds) (float)
    static double GameTime;            // total scaled game time (seconds)
    static double GameTimeNoPause;     // total unscaled game time (seconds)
    static bool GamePaused;
    static float TimeScale;            // global time scale multiplier

    // Call at startup to initialize timing
    static void Init();
    // Call each frame to update timers and apply time scale effects
    static void Update();
    // Add a custom timescale effect (duration in seconds, multiplier, affectSound)
    static void AddTimeScaleEffect(float duration, float scale, bool affectSound = false);

    // Get the current compounded timescale (for gameplay)
    static float GetFinalTimeScale();
    // Get the current compounded timescale for sound processing
    static float GetSoundFinalTimeScale();

private:
    static Uint64 lastCounter;          // previous frame's performance counter
    static double frequency;            // performance counter frequency

    // Active time scale effects
    static std::vector<TimeScaleEffect> timeScaleEffects;
};  