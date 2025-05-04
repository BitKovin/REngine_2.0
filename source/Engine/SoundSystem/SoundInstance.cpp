#include "SoundInstance.hpp"

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

    // Fallback: probe total sources until failure
    if (maxMono == 0 && maxStereo == 0) {
        std::vector<ALuint> tempSources;
        ALuint src;
        while (true) {
            alGenSources(1, &src);
            ALenum err = alGetError();
            if (err != AL_NO_ERROR) {
                std::cerr << "[SourcePool] Probing failed with error: 0x"
                    << std::hex << alGetString(err) << std::dec << "\n";
                break;
            }
            tempSources.push_back(src);
        }
        if (!tempSources.empty()) {
            freeMono = std::move(tempSources);
            maxMono = freeMono.size();
            std::cerr << "[SourcePool] Probed maxMono=" << maxMono << "\n";
        }
        else {
            // Fallback to a reasonable minimum for Emscripten
            maxMono = 16;  // Common minimum for WebAudio-backed OpenAL
            std::cerr << "[SourcePool] No sources probed; defaulting to maxMono="
                << maxMono << "\n";
        }
    }

    // Ensure at least a minimum limit
    if (maxMono < 8) maxMono = 8;  // Arbitrary safe minimum
    if (maxStereo == 0) maxStereo = maxMono;  // Default stereo to mono limit
}

ALuint SoundInstance::SourcePool::Acquire(bool stereo)
{
    std::lock_guard<std::recursive_mutex> lock(mtx);

    // 1) Try to reuse a released source
    auto& pool = stereo ? freeStereo : freeMono;
    if (!pool.empty()) {
        ALuint s = pool.back();
        pool.pop_back();
        std::cerr << "[SourcePool] Reusing source: " << s << "\n";
        return s;
    }

    // 2) No free source in pool → try to gen a new one (up to limit)
    size_t& allocated = stereo ? allocatedStereo : allocatedMono;
    size_t  limit = stereo ? maxStereo : maxMono;
    size_t  usedCount = allocated - /* unused */ pool.size();
    if (usedCount < limit) {
        ALuint s = 0;
        alGenSources(1, &s);
        ALenum err = alGetError();
        if (err == AL_NO_ERROR) {
            ++allocated;
            std::cerr << "[SourcePool] Generated new source: " << s << "\n";
            return s;
        }
        std::cerr << "[SourcePool] alGenSources failed: 0x"
            << std::hex << err << std::dec << "\n";
    }

    // 3) Out of capacity
    std::cerr << "[SourcePool] Out of sources (" << usedCount
        << "/" << limit << ")\n";
    return 0;
}


void SoundInstance::SourcePool::Release(ALuint src, bool stereo)
{
    std::lock_guard<std::recursive_mutex> lock(mtx);
    alSourceStop(src);
    alSourcei(src, AL_BUFFER, 0);
    auto& pool = stereo ? freeStereo : freeMono;
    pool.push_back(src);
    std::cerr << "[SourcePool] Released source: " << src << "\n";
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
