#include "SoundInstance.hpp"

#define SOUND_POOL_DEBUG

std::recursive_mutex mtx;

void SoundInstance::SourcePool::Init()
{
    std::lock_guard<std::recursive_mutex> lock(mtx);
    if (initialized) return;
    initialized = true;

    // Clear any existing error state
    alGetError();

    // Try OpenAL Soft hints
    ALCdevice* dev = alcGetContextsDevice(alcGetCurrentContext());
    if (dev) {
        alcGetIntegerv(dev, ALC_MONO_SOURCES, 1, (ALCint*)&maxMono);
        alcGetIntegerv(dev, ALC_STEREO_SOURCES, 1, (ALCint*)&maxStereo);
        std::cerr << "[SourcePool] Device hints: maxMono=" << maxMono
            << ", maxStereo=" << maxStereo << "\n";
    }

    if (maxMono > 512)
        maxMono = 512;

    if(maxStereo > 256)
        maxStereo = 256;

    printf("max mono: %zu \n", maxMono);
    printf("max stereo: %zu \n", maxStereo);

    // Ensure at least a minimum limit
    if (maxMono < 8) maxMono = 8;  // Arbitrary safe minimum
    if (maxStereo == 0) maxStereo = maxMono;  // Default stereo to mono limit
}

ALuint SoundInstance::SourcePool::Acquire(bool stereo, SoundInstance* requester)
{
    std::lock_guard<std::recursive_mutex> lock(mtx);

    auto& pool = stereo ? freeStereo : freeMono;
    auto& alloc = stereo ? allocatedStereo : allocatedMono;
    size_t limit = stereo ? maxStereo : maxMono;
    size_t used = alloc - pool.size();

    // 1) Reuse
    if (!pool.empty()) {
        ALuint s = pool.back(); pool.pop_back();
        liveOwners[s] = { requester, ++globalTimestamp };
        return s;
    }

    // 2) Gen new if under limit
    if (used < limit) {
        ALuint s = 0;
        alGenSources(1, &s);
        if (alGetError() == AL_NO_ERROR) {
            ++alloc;
            liveOwners[s] = { requester, ++globalTimestamp };
            return s;
        }
    }

    // 3) Steal oldest lowest-prio non-looping
    ALuint victimSrc = 0;
    float  victimPrio = std::numeric_limits<float>::infinity();
    uint64_t victimTime = UINT64_MAX;

    for (auto& [src, info] : liveOwners) {
        auto* inst = info.inst;
        if (inst->Loop) continue;            // skip looping
        float pr = inst->Priority;
        uint64_t ts = info.timestamp;
        // choose if lower priority, or equal pr & older timestamp
        if (pr < victimPrio ||
            (pr == victimPrio && ts < victimTime))
        {
            victimPrio = pr;
            victimTime = ts;
            victimSrc = src;
        }
    }

    if (victimSrc) {
        // steal it
        alSourceStop(victimSrc);
        liveOwners.erase(victimSrc);
        liveOwners[victimSrc] = { requester, ++globalTimestamp };
        return victimSrc;
    }

    // 4) none to steal
    return 0;
}

void SoundInstance::SourcePool::Release(ALuint src, bool stereo, SoundInstance* inst)
{
    return;
    std::lock_guard<std::recursive_mutex> lock(mtx);
    alSourceStop(src);
    alSourcei(src, AL_BUFFER, 0);
    liveOwners.erase(src);
    auto& pool = stereo ? freeStereo : freeMono;
    
    pool.push_back(src);

#ifdef SOUND_POOL_DEBUG
    std::cerr << "[SourcePool::Release] Released source " << src
        << " back to " << (stereo ? "stereo" : "mono") << " pool\n";
#endif
}


void SoundInstance::Play()
{
    if (_active && _source != 0) {
        // already playing
        return;
    }

    std::lock_guard<std::recursive_mutex> lock(mtx);

    _active = true;
    TryAcquire();         // may leave us virtual if no source available
    if (_source) {
        // rewind or set offset
        alSourcef(_source, AL_SEC_OFFSET, _virtualOffset);
        UpdateSourceParams();
        alSourcePlay(_source);
    }
}

void SoundInstance::Pause()
{
    std::lock_guard<std::recursive_mutex> lock(mtx);
    if (_source) {
        alGetSourcef(_source, AL_SEC_OFFSET, &_virtualOffset);
        alSourcePause(_source);
        ReleaseSource();
    }
    _active = false;
}

void SoundInstance::Stop()
{
    std::lock_guard<std::recursive_mutex> lock(mtx);
    if (_source) {
        alSourceStop(_source);
        ReleaseSource();
    }
    _virtualOffset = 0.0f;
    _active = false;
}
