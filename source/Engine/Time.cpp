#include "Time.hpp"

// Define static members
double Time::DeltaTime = 0.0;
float Time::DeltaTimeF = 0.0F;
double Time::GameTime = 0.0;
double Time::GameTimeNoPause = 0.0;
bool Time::GamePaused = false;
Uint64 Time::lastCounter = 0;
double Time::frequency = 0.0;
float Time::TimeScale = 1.0f;

void Time::Init()
{
    frequency = static_cast<double>(SDL_GetPerformanceFrequency());
    lastCounter = SDL_GetPerformanceCounter();
    DeltaTime = 0.0;
}

void Time::Update()
{
    Uint64 currentCounter = SDL_GetPerformanceCounter();
    double newDelta = (currentCounter - lastCounter) / frequency;
    lastCounter = currentCounter;



    if (newDelta > 0.1)
        newDelta = 0.1;

    newDelta *= TimeScale;

    DeltaTime = newDelta;

    if (GamePaused)
    {
		GameTimeNoPause += DeltaTime / TimeScale;
    }
    else
    {
        GameTimeNoPause += DeltaTime / TimeScale;
        GameTime += DeltaTime;
    }

    

    DeltaTimeF = (float)DeltaTime;
}
