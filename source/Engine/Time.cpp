#include "Time.hpp"

#include "EngineMain.h"

// static member definitions
double Time::DeltaTime = 0.0;
float Time::DeltaTimeF = 0.0f;
double Time::DeltaTimeNoTimeScale = 0.0;
float Time::DeltaTimeFNoTimeScale = 0.0f;
double Time::GameTime = 0.0;
double Time::GameTimeNoPause = 0.0;
float Time::TimeScale = 1.0f;
Uint64 Time::lastCounter = 0;
double Time::frequency = 0.0;
std::vector<TimeScaleEffect> Time::timeScaleEffects;

void Time::Init() {
    frequency = static_cast<double>(SDL_GetPerformanceFrequency());
    lastCounter = SDL_GetPerformanceCounter();
    DeltaTime = 0.0;
}

void Time::Update() {
    // Measure raw delta time (unscaled)
    Uint64 currentCounter = SDL_GetPerformanceCounter();
    double rawDelta = (currentCounter - lastCounter) / frequency;
    lastCounter = currentCounter;

    // Avoid very large spikes
    if (rawDelta > 0.1)
        rawDelta = 0.1;

    // Update effect durations (using unscaled time)
    for (auto it = timeScaleEffects.begin(); it != timeScaleEffects.end();) {
        it->remainingDuration -= rawDelta;
        if (it->remainingDuration <= 0.0f) {
            it = timeScaleEffects.erase(it);
        }
        else {
            ++it;
        }
    }

    DeltaTimeNoTimeScale = rawDelta;
    DeltaTimeFNoTimeScale = rawDelta;

    // Compute compounded time scales
    float finalScale = GetFinalTimeScale();
    

    // Apply to delta
    double scaledDelta = rawDelta * finalScale;
    DeltaTime = scaledDelta;
    DeltaTimeF = static_cast<float>(scaledDelta);

    // Update game times
    GameTimeNoPause += rawDelta;
    if (!EngineMain::MainInstance->Paused) 
    {

        GameTime += scaledDelta;
    }
}

void Time::AddTimeScaleEffect(float duration, float scale,
    bool affectSound, const std::string& key) 
{
    // Remove existing effects with same key if applicable
    if (!key.empty()) {
        timeScaleEffects.erase(
            std::remove_if(
                timeScaleEffects.begin(),
                timeScaleEffects.end(),
                [&](const TimeScaleEffect& e) {
                    return e.key == key;
                }
            ),
            timeScaleEffects.end()
        );
    }

    // Create and add new effect
    TimeScaleEffect e;
    e.remainingDuration = duration;
    e.timeScale = scale;
    e.affectSound = affectSound;
    e.key = key;  // Store key (empty if not provided)
    timeScaleEffects.push_back(e);
}

float Time::GetFinalTimeScale() {
    float result = TimeScale;
    for (const auto& e : timeScaleEffects)
        result *= e.timeScale;
    return result;
}

float Time::GetSoundFinalTimeScale() {
    float result = TimeScale;
    for (const auto& e : timeScaleEffects)
        if (e.affectSound)
            result *= e.timeScale;
    return result;
}
