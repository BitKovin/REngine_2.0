#pragma once

#if __EMSCRIPTEN__
// Disable EFX on Web builds
#define DISABLE_EFX
#endif

#ifdef DISABLE_EFX
#  define AL_ALEXT_PROTOTYPES
#  include <AL/al.h>
#  include <AL/alc.h>
#else
#  define AL_ALEXT_PROTOTYPES
#  include <AL/al.h>
#  include <AL/alc.h>
#  include <AL/efx.h>
#  include <AL/efx-presets.h>
#endif

#include <cmath>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <algorithm>
#include "../EObject.hpp"
#include "../glm.h"

class SoundInstance : public EObject {
public:
    // ─── Construction & Destruction ────────────────────────────────────

    /// buffer: pre-loaded OpenAL buffer
    /// isStereo: true → non-spatial stereo; false → 3D spatial mono
    SoundInstance(ALuint buffer, bool isStereo = false)
      : _buffer(buffer), _isStereo(isStereo)
    {
        // Ensure pool is initialized once
        SourcePool::Init();

        // Start in "not playing" state; we will Acquire in Play()
    }

    ~SoundInstance() {
        Stop();
    }

    // ─── Playback Control ───────────────────────────────────────────────

    /// Begins or resumes playback. Will acquire a source if needed.
    void Play();

    /// Pauses playback, preserving virtual playhead
    void Pause();

    /// Stops and resets playback
    void Stop();

    /// Must be called every frame to advance virtual playhead and reclaim sources
    void Update(float deltaTime) 
    {

        SourcePool::globalTimestamp = Time::GameTimeNoPause;

        if (!_active) return;

        if (_source) {
            ALint state;
            alGetSourcei(_source, AL_SOURCE_STATE, &state);
            if (state == AL_STOPPED) {
                // reached natural end
                ReleaseSource();
                _active = Loop;      // restart if looping
                if (_active) _virtualOffset = 0.0f;
            }
        } else {
            // no real source: advance virtual clock
            _virtualOffset += deltaTime;
            if (_virtualOffset >= _duration) {
                if (Loop) {
                    _virtualOffset = fmod(_virtualOffset, _duration);
                } else {
                    _active = false;
                }
            }
            // try to grab a source if high-priority now
            TryAcquire();
            if (_source && _active) {
                alSourcef(_source, AL_SEC_OFFSET, _virtualOffset);
                UpdateSourceParams();
                alSourcePlay(_source);
            }
        }
    }

    // ─── Public Properties ──────────────────────────────────────────────

    float Priority = 0.0f;    // higher = more important

    bool   Is2D           = false;
    bool   IsUISound      = false;
    bool   Loop           = false;

    float  Volume         = 1.0f;
    float  Pitch          = 1.0f;
    float  MinDistance    = 1.0f;
    float  MaxDistance    = 20.0f;

    glm::vec3 Position    = {0,0,0};
    glm::vec3 Velocity    = {0,0,0};
    glm::vec3 Direction   = {0,0,1};

    float  ConeInnerAngle = 360.0f;
    float  ConeOuterAngle = 360.0f;
    float  ConeOuterGain  = 1.0f;


    bool   EnableFilter   = false;
    float  LowPassGain    = 1.0f;
    float  LowPassGainHF  = 1.0f;

    bool   EnableEcho     = false;
    float  EchoDelay      = 0.1f;
    float  EchoLRDelay    = 0.1f;
    float  EchoDamping    = 0.5f;
    float  EchoFeedback   = 0.5f;
    float  EchoSpread     = 0.5f;

    bool   EnableReverb   = false;
    float  ReverbDensity  = 0.5f;
    float  ReverbGain     = 0.5f;
    float  ReverbGainHF   = 0.3f;
    float  ReverbDecayTime= 1.0f;


    /// Set the total duration of this sound (in seconds), used for virtual timing
    void SetDuration(float seconds) { _duration = seconds; }

protected:
    virtual bool IsGamePaused() const { return false; }
    virtual float GetPitchScale() const { return 1.0f; }

private:
    // ─── Internal State ────────────────────────────────────────────────

    ALuint   _buffer       = 0;
    ALuint   _source       = 0;
    bool     _isStereo     = false;

    bool     _active       = false;
    float    _virtualOffset= 0.0f;
    float    _duration     = 0.0f;

#ifndef DISABLE_EFX
    ALuint   _filter       = 0;
    ALuint   _effectEcho   = 0, _slotEcho    = 0;
    ALuint   _effectReverb = 0, _slotReverb  = 0;
#endif

    // ─── Try to acquire an OpenAL source from the pool ───────────────────

    void TryAcquire() {
        if (_source != 0) return;
        if (!_active)    return;

        _source = SourcePool::Acquire(_isStereo, this);
        if (_source) {
            // attach buffer now
            alSourcei(_source, AL_BUFFER, _buffer);
        }
    }

    // ─── Release back to pool ────────────────────────────────────────────

    void ReleaseSource() {
        if (_source == 0) return;
        SourcePool::Release(_source, _isStereo, this);
        _source = 0;
    }

    // ─── Parameter application ──────────────────────────────────────────

    void UpdateSourceParams() {
        if (!_source) return;

        // global attenuation off
#ifdef AL_DISTANCE_MODEL_NONE
        alDistanceModel(AL_DISTANCE_MODEL_NONE);
#else
        alDistanceModel(0xD000);
#endif

        // looping & pitch
        alSourcei (_source, AL_LOOPING, Loop ? AL_TRUE : AL_FALSE);
        alSourcef (_source, AL_PITCH,   Pitch * GetPitchScale());

        // spatial vs 2D
        if (_isStereo || Is2D || IsUISound) {
            alSourcei(_source, AL_SOURCE_RELATIVE, AL_TRUE);
            alSourcef(_source, AL_GAIN, Volume);
        } else {
            alSourcei(_source, AL_SOURCE_RELATIVE, AL_FALSE);
            alSource3f(_source, AL_POSITION,    Position.x, Position.y, Position.z);
            alSource3f(_source, AL_VELOCITY,    Velocity.x, Velocity.y, Velocity.z);
            alSource3f(_source, AL_DIRECTION,   Direction.x, Direction.y, Direction.z);
            alSourcef(_source, AL_CONE_INNER_ANGLE, ConeInnerAngle);
            alSourcef(_source, AL_CONE_OUTER_ANGLE, ConeOuterAngle);
            alSourcef(_source, AL_CONE_OUTER_GAIN,  ConeOuterGain);

            float gain = ComputeDistanceGain();
            alSourcef(_source, AL_GAIN, gain);
        }

#ifndef DISABLE_EFX
        EnsureEFX();
        if (EnableFilter && _filter) ApplyFilter();
        if (EnableEcho   && _slotEcho)   alSource3i(_source, AL_AUXILIARY_SEND_FILTER, _slotEcho,   0, AL_FILTER_NULL);
        if (EnableReverb && _slotReverb) alSource3i(_source, AL_AUXILIARY_SEND_FILTER, _slotReverb, 1, AL_FILTER_NULL);
#endif
    }

    float ComputeDistanceGain() {
        ALfloat lp[3];
        alGetListenerfv(AL_POSITION, lp);
        glm::vec3 listener(lp[0], lp[1], lp[2]);
        float d = glm::length(Position - listener);
        if (d < MinDistance) d = MinDistance;
        if (d >= MaxDistance) return 0.0f;
        float t = (d - MinDistance) / (MaxDistance - MinDistance);
        return Volume * std::pow(1.0f - t, 2.0f);
    }

#ifndef DISABLE_EFX
    void EnsureEFX() {
        ALCcontext* ctx = alcGetCurrentContext();
        if (!ctx) return;
        ALCdevice* dev = alcGetContextsDevice(ctx);
        if (!alcIsExtensionPresent(dev, "ALC_EXT_EFX")) return;

        if (EnableFilter && _filter == 0) {
            alGenFilters(1, &_filter);
            alFilteri(_filter, AL_FILTER_TYPE, AL_FILTER_LOWPASS);
        }
        if (EnableEcho && _effectEcho == 0) {
            alGenEffects(1, &_effectEcho);
            alEffecti(_effectEcho, AL_EFFECT_TYPE, AL_EFFECT_ECHO);
            alEffectf(_effectEcho, AL_ECHO_DELAY,   EchoDelay);
            alEffectf(_effectEcho, AL_ECHO_LRDELAY, EchoLRDelay);
            alEffectf(_effectEcho, AL_ECHO_DAMPING,  EchoDamping);
            alEffectf(_effectEcho, AL_ECHO_FEEDBACK, EchoFeedback);
            alEffectf(_effectEcho, AL_ECHO_SPREAD,   EchoSpread);
            alGenAuxiliaryEffectSlots(1, &_slotEcho);
            alAuxiliaryEffectSloti(_slotEcho, AL_EFFECTSLOT_EFFECT, _effectEcho);
        }
        if (EnableReverb && _effectReverb == 0) {
            alGenEffects(1, &_effectReverb);
            alEffecti(_effectReverb, AL_EFFECT_TYPE, AL_EFFECT_REVERB);
            alEffectf(_effectReverb, AL_REVERB_DENSITY,    ReverbDensity);
            alEffectf(_effectReverb, AL_REVERB_GAIN,       ReverbGain);
            alEffectf(_effectReverb, AL_REVERB_GAINHF,     ReverbGainHF);
            alEffectf(_effectReverb, AL_REVERB_DECAY_TIME, ReverbDecayTime);
            alGenAuxiliaryEffectSlots(1, &_slotReverb);
            alAuxiliaryEffectSloti(_slotReverb, AL_EFFECTSLOT_EFFECT, _effectReverb);
        }
    }

    void ApplyFilter() {
        alFilterf(_filter,   AL_LOWPASS_GAIN,   LowPassGain);
        alFilterf(_filter,   AL_LOWPASS_GAINHF, LowPassGainHF);
        alSourcei (_source,  AL_DIRECT_FILTER,  _filter);
    }
#endif

    // ─── Static Pool Manager ────────────────────────────────────────────

    struct SourcePool 
    {
        inline static std::vector<ALuint>  freeMono, freeStereo;
        inline static size_t               allocatedMono = 0, allocatedStereo = 0;
        inline static size_t               maxMono = 0, maxStereo = 0;
        inline static bool                 initialized = false;

        struct OwnerInfo {
            SoundInstance* inst;
            uint64_t       timestamp;
        };
        inline static std::unordered_map<ALuint, OwnerInfo> liveOwners;
        inline static uint64_t globalTimestamp = 0;

        static void Init();

        static ALuint Acquire(bool stereo, SoundInstance* requester);

        static void Release(ALuint src, bool stereo, SoundInstance* inst);
    };
};
