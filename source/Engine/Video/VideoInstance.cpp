#include "VideoInstance.h"
#include "pl_mpeg.h"
#include <mutex>
#include <atomic>
#include <cstring>
#include <iostream>
#include "../SoundSystem/SoundManager.hpp"


static void CheckALError(const char* operation) {
    ALenum error;
    while ((error = alGetError()) != AL_NO_ERROR) {
        std::cerr << "OpenAL error " << error << " after " << operation << std::endl;
    }
}

// Video callback (Option 2 implementation)
static void plm_instance_video_cb(plm_t* plm, plm_frame_t* frame, void* user) {
    if (!user || !frame) return;
    VideoInstance::FrameGrab* grab = (VideoInstance::FrameGrab*)user;


#ifndef VIDEO_NO_THREADING

    std::lock_guard<std::mutex> lk(grab->mtx);

#endif // !VIDEO_NO_THREADING


    int w = frame->width ? frame->width : plm_get_width(plm);
    int h = frame->height ? frame->height : plm_get_height(plm);

    grab->width = w;
    grab->height = h;

    // Direct YUV -> RGB with library function (handles strides/padding)
    grab->rgb_data.resize(w * h * 3);
    plm_frame_to_rgb(frame, grab->rgb_data.data(), w * 3);

    // No need for plane storage or custom conversion loop

    grab->have_frame.store(true, std::memory_order_release);
}

// Audio callback
static void plm_instance_audio_cb(plm_t* plm, plm_samples_t* samples, void* user) {
    if (!user || !samples || samples->count == 0) return;
    VideoInstance* self = (VideoInstance*)user;

    //std::cout << "Audio callback: " << samples->count << " samples" << std::endl;

    ALuint buffer;
    alGenBuffers(1, &buffer);
    CheckALError("alGenBuffers");

    std::vector<int16_t> pcm(samples->count * 2);
    for (size_t i = 0; i < samples->count; ++i) {
        float left = samples->interleaved[i * 2];
        float right = samples->interleaved[i * 2 + 1];
        pcm[i * 2] = static_cast<int16_t>(std::min(std::max(left * 32767.0f, -32768.0f), 32767.0f));
        pcm[i * 2 + 1] = static_cast<int16_t>(std::min(std::max(right * 32767.0f, -32768.0f), 32767.0f));
    }

    alBufferData(buffer, AL_FORMAT_STEREO16, pcm.data(), pcm.size() * sizeof(int16_t), self->_sampleRate);
    CheckALError("alBufferData");

    alSourceQueueBuffers(self->_audioSource, 1, &buffer);
    CheckALError("alSourceQueueBuffers");
}


// ---------------- VideoInstance ------------------
VideoInstance::VideoInstance(Video* video)
    : _video(video), _plm(nullptr)
{
    _frameGrab = new FrameGrab();

    alcMakeContextCurrent(SoundManager::contextStereo);

    alGenSources(1, &_audioSource);
    CheckALError("alGenSources");

    alSourcef(_audioSource, AL_GAIN, 1.0f); // Ensure volume is full
    CheckALError("alSourcef gain");

    InitDecoder();
}

VideoInstance::~VideoInstance() {
    DestroyDecoder();

    alcMakeContextCurrent(SoundManager::contextStereo);

    alSourceStop(_audioSource);
    CheckALError("alSourceStop in dtor");

    ALint queued = 0;
    alGetSourcei(_audioSource, AL_BUFFERS_QUEUED, &queued);
    CheckALError("alGetSourcei queued in dtor");

    while (queued > 0) {
        ALuint buffer = 0;
        alSourceUnqueueBuffers(_audioSource, 1, &buffer);
        CheckALError("alSourceUnqueueBuffers in dtor");
        alDeleteBuffers(1, &buffer);
        CheckALError("alDeleteBuffers in dtor");
        queued--;
    }

    if (_audioSource) {
        alDeleteSources(1, &_audioSource);
        CheckALError("alDeleteSources");
    }
    delete _frameGrab;
}

void VideoInstance::InitDecoder() {
    if (!_video) return;
    _plm = plm_create_with_memory((uint8_t*)_video->_data.data(), _video->_data.size(), 0);
    if (!_plm) {
        std::cerr << "VideoInstance: failed to create plm_t\n";
        return;
    }
    plm_set_video_decode_callback((plm_t*)_plm, plm_instance_video_cb, _frameGrab);
    plm_set_audio_decode_callback((plm_t*)_plm, plm_instance_audio_cb, this);
    plm_set_audio_enabled((plm_t*)_plm, 1);
    if (plm_get_num_audio_streams((plm_t*)_plm) > 0) {
        plm_set_audio_stream((plm_t*)_plm, 0);
        _sampleRate = plm_get_samplerate((plm_t*)_plm);
        plm_set_audio_lead_time((plm_t*)_plm, 0.2); // Buffer 200ms ahead for smooth playback
        std::cout << "Audio stream selected, sample rate: " << _sampleRate << std::endl;
    }
    else {
        std::cerr << "VideoInstance: no audio streams in video\n";
        _sampleRate = 0;
    }
}

void VideoInstance::DestroyDecoder() {
    if (_plm) {
        plm_destroy((plm_t*)_plm);
        _plm = nullptr;
    }
}

void VideoInstance::Start() {
    _playing = true;
    alcMakeContextCurrent(SoundManager::contextStereo);

    if (_plm && _sampleRate > 0) {
        // Pre-fill buffers using lead_time with 0 dt
        plm_decode((plm_t*)_plm, 0.0);
        UpdateAudio();
    }

    alSourcePlay(_audioSource);
    CheckALError("alSourcePlay");
}

void VideoInstance::Pause() {
    _playing = false;
    alcMakeContextCurrent(SoundManager::contextStereo);
    alSourcePause(_audioSource);
    CheckALError("alSourcePause");
}

void VideoInstance::SetTime(float time) {
    if (!_plm) return;
    if (time < 0.0f) time = 0.0f;
    _currentTime = time;

    // seek_exact = 0 (fast keyframe seek) or 1 (precise)
    int result = plm_seek((plm_t*)_plm, time, 0);
    if (result != 0) {
        std::cerr << "VideoInstance::SetTime: failed to seek to " << time << " seconds\n";
    }

    alcMakeContextCurrent(SoundManager::contextStereo);

    // Flush audio queues after seek
    alSourceStop(_audioSource);
    CheckALError("alSourceStop");

    ALint queued = 0;
    alGetSourcei(_audioSource, AL_BUFFERS_QUEUED, &queued);
    CheckALError("alGetSourcei queued");
    while (queued > 0) {
        ALuint buffer = 0;
        alSourceUnqueueBuffers(_audioSource, 1, &buffer);
        CheckALError("alSourceUnqueueBuffers");
        alDeleteBuffers(1, &buffer);
        CheckALError("alDeleteBuffers");
        queued--;
    }

    if (_playing) {
        // Pre-fill after seek
        plm_decode((plm_t*)_plm, 0.0);
        UpdateAudio();
        alSourcePlay(_audioSource);
        CheckALError("alSourcePlay after seek");
    }
}

void VideoInstance::Update(double deltaTime) {
    if (!_plm || !_playing) return;
    _currentTime += (float)deltaTime;

    if (_video == nullptr) return;

    alcMakeContextCurrent(SoundManager::contextStereo);

    plm_decode((plm_t*)_plm, deltaTime);
    UpdateAudio();

    if (plm_has_ended((plm_t*)_plm)) {
        if (_loop) {
            plm_rewind((plm_t*)_plm);
            _currentTime = 0.0f;
            // Pre-fill after rewind
            plm_decode((plm_t*)_plm, 0.0);
            UpdateAudio();
        }
        else {
            _playing = false;
            alSourceStop(_audioSource);
            CheckALError("alSourceStop at end");
        }
    }
}

void VideoInstance::UpdateAudio() {
    // 1. Unqueue and delete any buffers that have finished playing.
    ALint processed = 0;
    alGetSourcei(_audioSource, AL_BUFFERS_PROCESSED, &processed);
    CheckALError("alGetSourcei processed");

    while (processed > 0) {
        ALuint buffer = 0;
        alSourceUnqueueBuffers(_audioSource, 1, &buffer);
        CheckALError("alSourceUnqueueBuffers in UpdateAudio");
        if (buffer != 0) {
            alDeleteBuffers(1, &buffer);
            CheckALError("alDeleteBuffers in UpdateAudio");
        }
        processed--;
    }

    // 2. PROACTIVELY refill the queue to prevent underruns.
    // Define a minimum number of buffers we want to keep queued.
    const int MIN_BUFFERS_QUEUED = 2;

    ALint queued = 0;
    alGetSourcei(_audioSource, AL_BUFFERS_QUEUED, &queued);
    CheckALError("alGetSourcei queued in UpdateAudio");

    // While playing and the queue is below our minimum threshold...
    while (_playing && queued < MIN_BUFFERS_QUEUED && !plm_has_ended((plm_t*)_plm)) {
        // ...decode a small amount of audio to trigger the audio callback.
        // Using 0.0 for deltaTime just decodes what's already buffered by pl_mpeg.
        plm_decode((plm_t*)_plm, 0.0);

        ALint new_queued = 0;
        alGetSourcei(_audioSource, AL_BUFFERS_QUEUED, &new_queued);

        // If decoding didn't add a new buffer (e.g., end of stream), break to avoid an infinite loop.
        if (new_queued == queued) {
            break;
        }
        queued = new_queued;
    }

    // 3. As a safety net, restart the source if it did stop for some reason.
    ALint state;
    alGetSourcei(_audioSource, AL_SOURCE_STATE, &state);
    CheckALError("alGetSourcei state");

    if (_playing && state != AL_PLAYING) {
        // This will now mostly trigger on the very first start or after a seek.
        alSourcePlay(_audioSource);
        CheckALError("alSourcePlay on underrun/restart");
    }
}

void VideoInstance::SetLoop(bool loop) {
    _loop = loop;
}

const std::vector<uint8_t>& VideoInstance::GetCurrentFrameData() const {
    return _frameGrab->rgb_data;
}

int VideoInstance::GetWidth() const {
    return _frameGrab->width;
}
int VideoInstance::GetHeight() const {
    return _frameGrab->height;
}