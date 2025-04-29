#pragma once

#if __EMSCRIPTEN__
// Automatically disable EFX on Web builds
#define DISABLE_EFX
#endif
// To force‑enable EFX on desktop, comment out the above line:
// #undef DISABLE_EFX

#ifdef DISABLE_EFX
#define AL_ALEXT_PROTOTYPES
#include <AL/al.h>
#include <AL/alc.h>
#else
#define AL_ALEXT_PROTOTYPES
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/efx.h>
#include <AL/efx-presets.h>
#endif

#include <cmath>          // for std::pow
#include "../EObject.hpp"
#include "../glm.h"

class SoundInstance : public EObject
{
    ALuint sourceRef = 0;

#ifndef DISABLE_EFX
    // EFX handles
    ALuint filterLP = 0;
    ALuint effectEcho = 0, slotEcho = 0;
    ALuint effectReverb = 0, slotReverb = 0;
#endif

    vec3 listenerPos = vec3(0.0f);

public:
    // ─── mode flags ─────────────────────────────────────────────────────
    bool Is2D = false;  // ignore spatialization
    bool IsUISound = false;  // UI sounds continue when game paused

    // ─── basic params ────────────────────────────────────────────────────
    float Volume = 1.0f;
    float Pitch = 1.0f;
    bool  Loop = false;

    vec3 Position = vec3(0.0f);
    vec3 Velocity = vec3(0.0f);
    vec3 Direction = vec3(0.0f, 0.0f, 1.0f);

    float MinDistance = 1.0f;
    float MaxDistance = 20.0f;
    float RolloffFactor = 1.0f;  // unused when built‑in attenuation is off

    // ─── source cone ─────────────────────────────────────────────────────
    float ConeInnerAngle = 360.0f;
    float ConeOuterAngle = 360.0f;
    float ConeOuterGain = 1.0f;

    // ─── effect toggles ──────────────────────────────────────────────────
    bool EnableFilter = false;
    bool EnableEcho = false;
    bool EnableReverb = false;

#ifndef DISABLE_EFX
    // Low‑pass filter params
    float LowPassGain = 1.0f;
    float LowPassGainHF = 1.0f;

    // Echo params
    float EchoDelay = 0.1f;
    float EchoLRDelay = 0.1f;
    float EchoDamping = 0.5f;
    float EchoFeedback = 0.5f;
    float EchoSpread = 0.5f;

    // Reverb params
    float ReverbDensity = 0.5f;
    float ReverbGain = 0.5f;
    float ReverbGainHF = 0.3f;
    float ReverbDecayTime = 1.0f;
#endif

    SoundInstance(ALuint src)
        : sourceRef(src)
    {
        // Turn off OpenAL’s built‑in distance attenuation entirely:
#ifdef AL_DISTANCE_MODEL_NONE
        alDistanceModel(AL_DISTANCE_MODEL_NONE);
#else
        alDistanceModel(0xD000); // numeric value for AL_DISTANCE_MODEL_NONE
#endif
    }

    ~SoundInstance()
    {
        if (alIsSource(sourceRef)) alDeleteSources(1, &sourceRef);
#ifndef DISABLE_EFX
        if (filterLP)       alDeleteFilters(1, &filterLP);
        if (effectEcho) {
            alDeleteEffects(1, &effectEcho);
            alDeleteAuxiliaryEffectSlots(1, &slotEcho);
        }
        if (effectReverb) {
            alDeleteEffects(1, &effectReverb);
            alDeleteAuxiliaryEffectSlots(1, &slotReverb);
        }
#endif
    }

    // Override to hook into your game’s pause state
    virtual bool IsGamePaused() const { return false; }

    void Update()
    {
        if (!alIsSource(sourceRef)) return;

        // ─── pause logic ────────────────────────────────────────────────────
        if (!IsUISound && IsGamePaused()) {
            alSourcePause(sourceRef);
            return;
        }

        // ─── looping & pitch ─────────────────────────────────────────────────
        alSourcei(sourceRef, AL_LOOPING, Loop ? AL_TRUE : AL_FALSE);
        alSourcef(sourceRef, AL_PITCH, Pitch * GetPitchScale());

        // ─── spatialization toggle ───────────────────────────────────────────
        if (Is2D || IsUISound) {
            alSourcei(sourceRef, AL_SOURCE_RELATIVE, AL_TRUE);
        }
        else {
            alSourcei(sourceRef, AL_SOURCE_RELATIVE, AL_FALSE);
            alSource3f(sourceRef, AL_POSITION, Position.x, Position.y, Position.z);
            alSource3f(sourceRef, AL_VELOCITY, Velocity.x, Velocity.y, Velocity.z);
            alSource3f(sourceRef, AL_DIRECTION, Direction.x, Direction.y, Direction.z);

            // initial cone – will be modulated below
            alSourcef(sourceRef, AL_CONE_INNER_ANGLE, ConeInnerAngle);
            alSourcef(sourceRef, AL_CONE_OUTER_ANGLE, ConeOuterAngle);
            alSourcef(sourceRef, AL_CONE_OUTER_GAIN, ConeOuterGain);
        }

        // ─── on‑demand EFX init ───────────────────────────────────────────────
#ifndef DISABLE_EFX
        EnsureEFX();
        if (EnableFilter && filterLP) {
            alFilterf(filterLP, AL_LOWPASS_GAIN, LowPassGain);
            alFilterf(filterLP, AL_LOWPASS_GAINHF, LowPassGainHF);
            alSourcei(sourceRef, AL_DIRECT_FILTER, filterLP);
        }
        if (EnableEcho && slotEcho)
            alSource3i(sourceRef, AL_AUXILIARY_SEND_FILTER, slotEcho, 0, AL_FILTER_NULL);
        if (EnableReverb && slotReverb)
            alSource3i(sourceRef, AL_AUXILIARY_SEND_FILTER, slotReverb, 1, AL_FILTER_NULL);
#endif

        // ─── custom distance & directional falloff ─────────────────────────
        if (!Is2D && !IsUISound) {
            // fetch the real listener position
            ALfloat lp[3];
            alGetListenerfv(AL_POSITION, lp);
            listenerPos = vec3(lp[0], lp[1], lp[2]);

            // compute normalized distance [0..1]
            float d = length(Position - listenerPos);

            if (d < MinDistance)
                d = MinDistance;

            float t = 0.0f;
            float gain;
            if (d <= MinDistance) {
                gain = Volume;
            }
            else if (d < MaxDistance) {
                t = (d - MinDistance) / (MaxDistance - MinDistance);
                constexpr float Exponent = 2.0f;  // quadratic curve
                gain = Volume * std::pow(1.0f - t, Exponent);
            }
            else {
                gain = 0.0f;
            }

            if (t < 0)
                t = 0;

            alSourcef(sourceRef, AL_GAIN, gain);

            // directional “blur”: tighten cone at close, flatten at far
            float dirFactor = 1.0f - t;


#ifndef DISABLE_EFX
            // HF damping over distance
            if (EnableFilter && filterLP) {
                float hfGain = LowPassGainHF * dirFactor + 0.2f * (1.0f - dirFactor);
                alFilterf(filterLP, AL_LOWPASS_GAIN, LowPassGain);
                alFilterf(filterLP, AL_LOWPASS_GAINHF, hfGain);
                alSourcei(sourceRef, AL_DIRECT_FILTER, filterLP);
            }
#endif

        }
        else {
            // 2D/UI: volume only
            alSourcef(sourceRef, AL_GAIN, Volume);
        }
    }

    void Play() { Update(); alSourcePlay(sourceRef); }
    void Pause() { if (alIsSource(sourceRef)) alSourcePause(sourceRef); }
    void Stop() { if (alIsSource(sourceRef)) alSourceStop(sourceRef); }

#ifndef DISABLE_EFX
private:
    void EnsureEFX()
    {
        auto ctx = alcGetCurrentContext();
        auto device = alcGetContextsDevice(ctx);
        if (!alcIsExtensionPresent(device, "ALC_EXT_EFX"))
            return;

        if (EnableFilter && filterLP == 0) {
            alGenFilters(1, &filterLP);
            alFilteri(filterLP, AL_FILTER_TYPE, AL_FILTER_LOWPASS);
        }
        if (EnableEcho && effectEcho == 0) {
            alGenEffects(1, &effectEcho);
            alEffecti(effectEcho, AL_EFFECT_TYPE, AL_EFFECT_ECHO);
            alEffectf(effectEcho, AL_ECHO_DELAY, EchoDelay);
            alEffectf(effectEcho, AL_ECHO_LRDELAY, EchoLRDelay);
            alEffectf(effectEcho, AL_ECHO_DAMPING, EchoDamping);
            alEffectf(effectEcho, AL_ECHO_FEEDBACK, EchoFeedback);
            alEffectf(effectEcho, AL_ECHO_SPREAD, EchoSpread);
            alGenAuxiliaryEffectSlots(1, &slotEcho);
            alAuxiliaryEffectSloti(slotEcho, AL_EFFECTSLOT_EFFECT, effectEcho);
        }
        if (EnableReverb && effectReverb == 0) {
            alGenEffects(1, &effectReverb);
            alEffecti(effectReverb, AL_EFFECT_TYPE, AL_EFFECT_REVERB);
            alEffectf(effectReverb, AL_REVERB_DENSITY, ReverbDensity);
            alEffectf(effectReverb, AL_REVERB_GAIN, ReverbGain);
            alEffectf(effectReverb, AL_REVERB_GAINHF, ReverbGainHF);
            alEffectf(effectReverb, AL_REVERB_DECAY_TIME, ReverbDecayTime);
            alGenAuxiliaryEffectSlots(1, &slotReverb);
            alAuxiliaryEffectSloti(slotReverb, AL_EFFECTSLOT_EFFECT, effectReverb);
        }
    }
#endif

    virtual float GetPitchScale() const { return 1.0f * Pitch; }
};
