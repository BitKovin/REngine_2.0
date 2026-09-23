#include "VideoInstance.h"
#include "pl_mpeg.h"
#include <mutex>
#include <atomic>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include "../SoundSystem/SoundManager.hpp"

// MP3 audio inside MP4 files. Define VIDEO_NO_MP3 to build without minimp3.
#ifndef VIDEO_NO_MP3
#define MINIMP3_IMPLEMENTATION
#include <minimp3.h>
struct Mp3State { mp3dec_t dec; };
#else
struct Mp3State { int unused; };
#endif


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

    ++grab->version;
    grab->have_frame.store(true, std::memory_order_release);
}

// Audio callback
static void plm_instance_audio_cb(plm_t* plm, plm_samples_t* samples, void* user) {
    if (!user || !samples || samples->count == 0) return;
    VideoInstance* self = (VideoInstance*)user;

    std::lock_guard<std::recursive_mutex> lock(SoundManager::audioMutex);
    alcMakeContextCurrent(SoundManager::contextStereo);

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
    if (!_video) std::cerr << "VideoInstance: Video* is null (asset failed to load?)\n";
    _frameGrab = new FrameGrab();

    std::lock_guard<std::recursive_mutex> lock(SoundManager::audioMutex);
    alcMakeContextCurrent(SoundManager::contextStereo);

    alGenSources(1, &_audioSource);
    CheckALError("alGenSources");

    alSourcef(_audioSource, AL_GAIN, 1.0f); // Ensure volume is full
    CheckALError("alSourcef gain");

    InitDecoder();
}

VideoInstance::~VideoInstance() {
    DestroyDecoder();

    std::lock_guard<std::recursive_mutex> lock(SoundManager::audioMutex);
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

    if (_video->IsMp4()) {
        // MP4: h264bsd video path. No audio (AAC can't be decoded here), so _sampleRate stays 0.
        _mp4Dec = _video->CreateMp4Decoder();
        _sampleRate = 0;
        if (!_mp4Dec) { std::cerr << "VideoInstance: failed to create MP4 decoder\n"; return; }
        _mp4Duration = _mp4Dec->Duration();
        std::cerr << "VideoInstance: MP4 path, duration " << _mp4Duration << "s\n";
        _lastMp4Sample = -1;
        UpdateMp4Frame();   // show frame 0 immediately

        _mp4Audio = _video->GetMp4Audio();
        if (_mp4Audio) {
#ifndef VIDEO_NO_MP3
            if (_mp4Audio->object_type == 0x6B || _mp4Audio->object_type == 0x69) {
                _mp3 = std::make_unique<Mp3State>();
                RestartMp4Audio(0.0);
            }
            else
#endif
            {
                std::cerr << "VideoInstance: MP4 audio codec (esds object type 0x" << std::hex
                    << (int)_mp4Audio->object_type << std::dec << ") is not supported; only MP3. "
                    << "Re-encode with: ffmpeg -i in.mp4 -c:v copy -c:a libmp3lame -b:a 192k out.mp4\n";
            }
        }
        return;
    }

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
        plm_set_audio_lead_time((plm_t*)_plm, _audioLeadTime); // Buffer ahead for smooth playback
        std::cout << "Audio stream selected, sample rate: " << _sampleRate << std::endl;
    }
    else {
        std::cerr << "VideoInstance: no audio streams in video\n";
        _sampleRate = 0;
    }
}

bool VideoInstance::DecodeMp3Sample(size_t index, std::vector<int16_t>& pcm, int& channels, int& hz) {
#ifndef VIDEO_NO_MP3
    const Video::Mp4Info& a = *_mp4Audio;
    if (index >= a.sample_size.size()) return false;
    uint64_t off = a.sample_offset[index];
    uint64_t size = a.sample_size[index];
    if (off + size > _video->_data.size()) return false;

    // Normally one MPEG frame per MP4 sample, but some muxers pack several: decode until consumed
    const uint8_t* p = _video->_data.data() + off;
    int remaining = (int)size;
    while (remaining > 0) {
        mp3d_sample_t frame[MINIMP3_MAX_SAMPLES_PER_FRAME];
        mp3dec_frame_info_t info;
        int samples = mp3dec_decode_frame(&_mp3->dec, p, remaining, frame, &info);
        if (info.frame_bytes <= 0) break;   // no more valid frames in this sample
        p += info.frame_bytes;
        remaining -= info.frame_bytes;
        if (samples > 0 && info.channels > 0) {
            channels = info.channels;
            hz = info.hz;
            pcm.insert(pcm.end(), frame, frame + (size_t)samples * info.channels);
        }
    }
    return true;
#else
    (void)index; (void)pcm; (void)channels; (void)hz;
    return false;
#endif
}

// Flush everything queued on the source and reposition the MP3 stream at `time`.
void VideoInstance::RestartMp4Audio(double time) {
    if (!_mp3 || !_mp4Audio) return;

    std::lock_guard<std::recursive_mutex> lock(SoundManager::audioMutex);
    alcMakeContextCurrent(SoundManager::contextStereo);

    alSourceStop(_audioSource);
    CheckALError("alSourceStop (mp4 restart)");
    ALint queued = 0;
    alGetSourcei(_audioSource, AL_BUFFERS_QUEUED, &queued);
    while (queued-- > 0) {
        ALuint buffer = 0;
        alSourceUnqueueBuffers(_audioSource, 1, &buffer);
        alDeleteBuffers(1, &buffer);
    }
    CheckALError("flush (mp4 restart)");

#ifndef VIDEO_NO_MP3
    mp3dec_init(&_mp3->dec);
#endif
    const Video::Mp4Info& a = *_mp4Audio;
    uint64_t t = (uint64_t)(std::max(0.0, time) * a.timescale);
    size_t idx = (size_t)(std::upper_bound(a.sample_time.begin(), a.sample_time.end(), t) - a.sample_time.begin());
    idx = idx ? idx - 1 : 0;

    // MP3 frames depend on the previous ones (bit reservoir): decode two frames and throw them away
    size_t warm = std::min<size_t>(idx, 2);
    _mp4AudioNext = idx - warm;
    for (size_t i = 0; i < warm; ++i) {
        std::vector<int16_t> discard;
        int ch = 0, hz = 0;
        DecodeMp3Sample(_mp4AudioNext++, discard, ch, hz);
    }

    if (_playing) UpdateMp4Audio();
}

// Keeps ~4 buffers (~0.7 s) queued. Caller holds audioMutex and has the AL context current.
void VideoInstance::UpdateMp4Audio() {
    if (!_mp3 || !_mp4Audio) return;

    ALint processed = 0;
    alGetSourcei(_audioSource, AL_BUFFERS_PROCESSED, &processed);
    while (processed-- > 0) {
        ALuint buffer = 0;
        alSourceUnqueueBuffers(_audioSource, 1, &buffer);
        if (buffer) alDeleteBuffers(1, &buffer);
    }
    CheckALError("unqueue processed (mp4 audio)");

    ALint queued = 0;
    alGetSourcei(_audioSource, AL_BUFFERS_QUEUED, &queued);

    const size_t total = _mp4Audio->sample_size.size();
    const int MIN_BUFFERS = 4;

    while (queued < MIN_BUFFERS && _mp4AudioNext < total) {
        std::vector<int16_t> pcm;
        int channels = 0, hz = 0;
        // Fill a buffer with ~200 ms of audio (frame sizes differ per layer: 384 or 1152 samples)
        while (_mp4AudioNext < total) {
            if (!DecodeMp3Sample(_mp4AudioNext++, pcm, channels, hz)) break;
            if (channels > 0 && hz > 0 && pcm.size() / channels >= (size_t)hz / 5) break;
        }
        if (pcm.empty() || channels <= 0 || hz <= 0) continue;

        if (_mp4AudioGain != 1.0f) {
            for (auto& s : pcm) {
                float v = s * _mp4AudioGain;
                s = (int16_t)std::min(32767.0f, std::max(-32768.0f, v));
            }
        }
        if (!_mp4AudioLogged) {
            _mp4AudioLogged = true;
            int peak = 0;
            for (auto s : pcm) peak = std::max(peak, std::abs((int)s));
            std::cerr << "VideoInstance: MP4 audio streaming, " << channels << " ch, " << hz
                << " Hz, first buffer " << pcm.size() / channels << " samples/ch, peak "
                << peak << "/32767\n";
        }

        ALuint buffer = 0;
        alGenBuffers(1, &buffer);
        alBufferData(buffer, channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16,
            pcm.data(), (ALsizei)(pcm.size() * sizeof(int16_t)), hz);
        alSourceQueueBuffers(_audioSource, 1, &buffer);
        CheckALError("queue buffer (mp4 audio)");
        ++queued;
    }

    ALint state = 0;
    alGetSourcei(_audioSource, AL_SOURCE_STATE, &state);
    if (_playing && state != AL_PLAYING && queued > 0) {
        alSourcePlay(_audioSource);
        CheckALError("alSourcePlay (mp4 audio)");
    }
}

void VideoInstance::UpdateMp4Frame() {
    if (!_mp4Dec) return;
    bool ok = _mp4Dec->DecodeToTime(_currentTime, 1.0);
    if (!ok && _lastMp4Sample < 0)
        std::cerr << "VideoInstance: MP4 decode produced no picture at t=" << _currentTime << "\n";
    int64_t cur = _mp4Dec->CurrentSample();
    if (cur < 0 || cur == _lastMp4Sample) return;   // nothing new to convert

#ifndef VIDEO_NO_THREADING
    std::lock_guard<std::mutex> lk(_frameGrab->mtx);
#endif
    int w = 0, h = 0;
    if (_mp4Dec->ToRgb(_frameGrab->rgb_data, w, h)) {
        _frameGrab->width = w;
        _frameGrab->height = h;
        ++_frameGrab->version;
        _frameGrab->have_frame.store(true, std::memory_order_release);
        _lastMp4Sample = cur;
    }
}

void VideoInstance::DestroyDecoder() {
    _mp4Dec.reset();
    _mp3.reset();
    if (_plm) {
        plm_destroy((plm_t*)_plm);
        _plm = nullptr;
    }
}

void VideoInstance::Start() {
    _playing = true;
    if (_mp4Dec) {
        if (_mp3) {
            std::lock_guard<std::recursive_mutex> lock(SoundManager::audioMutex);
            alcMakeContextCurrent(SoundManager::contextStereo);
            UpdateMp4Audio();   // fills the queue and starts (or resumes) the source
        }
        return;
    }

    std::lock_guard<std::recursive_mutex> lock(SoundManager::audioMutex);

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
    if (_mp4Dec) {
        if (_mp3) {
            std::lock_guard<std::recursive_mutex> lock(SoundManager::audioMutex);
            alcMakeContextCurrent(SoundManager::contextStereo);
            alSourcePause(_audioSource);
            CheckALError("alSourcePause (mp4)");
        }
        return;
    }

    std::lock_guard<std::recursive_mutex> lock(SoundManager::audioMutex);
    alcMakeContextCurrent(SoundManager::contextStereo);
    alSourcePause(_audioSource);
    CheckALError("alSourcePause");
}

void VideoInstance::SetTime(float time) {
    if (time < 0.0f) time = 0.0f;
    if (_mp4Dec) {
        if (_mp4Duration > 0.0 && time > _mp4Duration) time = (float)_mp4Duration;
        _currentTime = time;
        UpdateMp4Frame();   // exact seek (keyframe + decode forward), works while paused too
        RestartMp4Audio(time);
        return;
    }
    if (!_plm) return;
    _currentTime = time;

    // seek_exact = 0 (fast keyframe seek) or 1 (precise)
    int result = plm_seek((plm_t*)_plm, time, 0);
    if (result != 0) {
        std::cerr << "VideoInstance::SetTime: failed to seek to " << time << " seconds\n";
    }
    std::lock_guard<std::recursive_mutex> lock(SoundManager::audioMutex);
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
    if (_mp4Dec) {
        if (!_playing) return;
        _currentTime += (float)deltaTime;
        if (_mp4Duration > 0.0 && _currentTime >= _mp4Duration) {
            if (_loop) {
                _currentTime = (float)std::fmod((double)_currentTime, _mp4Duration);
                RestartMp4Audio(_currentTime);
            }
            else {
                _currentTime = (float)_mp4Duration;
                _playing = false;
            }
        }
        UpdateMp4Frame();
        if (_mp3) {
            std::lock_guard<std::recursive_mutex> lock(SoundManager::audioMutex);
            alcMakeContextCurrent(SoundManager::contextStereo);
            UpdateMp4Audio();
        }
        return;
    }

    if (!_plm || !_playing) return;
    _currentTime += (float)deltaTime;

    if (_video == nullptr) return;

    plm_decode((plm_t*)_plm, deltaTime);

    alcMakeContextCurrent(SoundManager::contextStereo);

    std::lock_guard<std::recursive_mutex> lock(SoundManager::audioMutex);

    UpdateAudio();

    if (plm_has_ended((plm_t*)_plm)) {
        if (_loop) {
            alSourceStop(_audioSource);
            CheckALError("alSourceStop in loop");

            ALint queued = 0;
            alGetSourcei(_audioSource, AL_BUFFERS_QUEUED, &queued);
            CheckALError("alGetSourcei queued in loop");
            while (queued > 0) {
                ALuint buffer = 0;
                alSourceUnqueueBuffers(_audioSource, 1, &buffer);
                CheckALError("alSourceUnqueueBuffers in loop");
                alDeleteBuffers(1, &buffer);
                CheckALError("alDeleteBuffers in loop");
                queued--;
            }

            plm_rewind((plm_t*)_plm);
            _currentTime = 0.0f;
            // Pre-fill after rewind
            plm_decode((plm_t*)_plm, 0.0);
            UpdateAudio();
            alSourcePlay(_audioSource);
            CheckALError("alSourcePlay in loop");
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
    const int MIN_BUFFERS_QUEUED = 3;

    ALint queued = 0;
    alGetSourcei(_audioSource, AL_BUFFERS_QUEUED, &queued);
    CheckALError("alGetSourcei queued in UpdateAudio");

    // While playing and the queue is below our minimum threshold...
    while (_playing && queued < MIN_BUFFERS_QUEUED && !plm_has_ended((plm_t*)_plm)) {
        // Temporarily boost lead time to decode more audio without advancing time
        plm_set_audio_lead_time((plm_t*)_plm, 2.0f);
        // ...decode a small amount of audio to trigger the audio callback.
        // Using 0.0 for deltaTime just decodes what's already buffered by pl_mpeg.
        plm_decode((plm_t*)_plm, 0.0);
        plm_set_audio_lead_time((plm_t*)_plm, _audioLeadTime);

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

    if (_playing && state != AL_PLAYING && queued > 0) {
        // This will now mostly trigger on the very first start or after a seek.
        alSourcePlay(_audioSource);
        CheckALError("alSourcePlay on underrun/restart");
    }
}

void VideoInstance::SetLoop(bool loop) {
    _loop = loop;
}

const std::vector<uint8_t>& VideoInstance::GetCurrentFrameRGBA() const {
    if (_rgbaVersion != _frameGrab->version) {
        const std::vector<uint8_t>& rgb = _frameGrab->rgb_data;
        size_t px = rgb.size() / 3;
        _rgba.resize(px * 4);
        for (size_t i = 0; i < px; ++i) {
            _rgba[i * 4 + 0] = rgb[i * 3 + 0];
            _rgba[i * 4 + 1] = rgb[i * 3 + 1];
            _rgba[i * 4 + 2] = rgb[i * 3 + 2];
            _rgba[i * 4 + 3] = 255;
        }
        _rgbaVersion = _frameGrab->version;
    }
    return _rgba;
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