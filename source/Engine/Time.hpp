#pragma once
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_timer.h>

class Time 
{
public:
    static double DeltaTime;       
    static float DeltaTimeF;       
    static double GameTime;       
    static double GameTimeNoPause;       

    static bool GamePaused;

    static float TimeScale;

private:
    static Uint64 lastCounter;     // Stores the previous frame's counter.
    static double frequency;       // Stores the counter frequency.

public:
    // Call this once at the start to initialize the timer.
    static void Init();

    // Call this every frame to update DeltaTime.
    static void Update();
};