#pragma once
#include "Video.h"
#include <memory>
#include <atomic>
#include <mutex>
#include <vector>
#include <cstdint>
#include <AL/al.h>
#include <AL/alc.h>

struct plm_t;
struct Mp3State;   // minimp3 decoder state (defined in the .cpp)

class VideoInstance {
public:
    VideoInstance(Video* video);
    ~VideoInstance();

    void Start();
    void Pause();
    void SetTime(float time);
    void Update(double deltaTime);
    void SetLoop(bool loop);
    // Software gain for MP4 audio (1.0 = unchanged, values > 1 amplify with clipping).
    // Useful for quiet sources, since OpenAL's AL_GAIN is normally capped at 1.0.
    void SetAudioGain(float gain) { _mp4AudioGain = gain; }

    // Returns pointer to RGB24 frame, or nullptr if no frame decoded yet
    const std::vector<uint8_t>& GetCurrentFrameData() const;
    // Same frame as RGBA8 with alpha = 255 (use with bgfx::TextureFormat::RGBA8; RGB8 isn't
    // supported by every bgfx renderer, and an unsupported format shows up as alpha 0 / nothing drawn)
    const std::vector<uint8_t>& GetCurrentFrameRGBA() const;
    // Increases every time a new picture is decoded; lets the renderer skip redundant uploads
    uint64_t GetFrameVersion() const { return _frameGrab->version; }
    int GetWidth() const;
    int GetHeight() const;

    struct FrameGrab
    {
        std::vector<uint8_t> y_plane;
        std::vector<uint8_t> cb_plane;
        std::vector<uint8_t> cr_plane;
        int y_stride = 0;
        int c_stride = 0;
        int width = 0;
        int height = 0;
        std::atomic<bool> have_frame{ false };

#ifndef VIDEO_NO_THREADING
        std::mutex mtx;
#endif // !VIDEO_NO_THREADING

        std::vector<uint8_t> rgb_data; // last converted frame
        uint64_t version = 0;          // bumped whenever rgb_data changes
    };

    Video* _video;

    plm_t* _plm; // plm_t*

    FrameGrab* _frameGrab;

    bool _playing = false;
    bool _loop = true;
    float _currentTime = 0.0f;
    int _sampleRate = 0;
    ALuint _audioSource = 0;

    float _audioLeadTime = 0.5;

    // ---- MP4 / H.264 playback (video only; h264bsd has no audio decoder) ----
    std::unique_ptr<Video::Mp4Decoder> _mp4Dec;   // null for MPEG-1 videos
    double _mp4Duration = 0.0;
    int64_t _lastMp4Sample = -1;
    void UpdateMp4Frame();

    // MP4 audio (MP3 inside MP4, decoded with minimp3 and streamed through OpenAL)
    const Video::Mp4Info* _mp4Audio = nullptr;
    std::unique_ptr<Mp3State> _mp3;
    float _mp4AudioGain = 1.0f;
    bool _mp4AudioLogged = false;
    size_t _mp4AudioNext = 0;                 // next audio sample (= MP3 frame) to decode
    void RestartMp4Audio(double time);        // flush queue and reposition
    void UpdateMp4Audio();                    // keep the OpenAL queue topped up (audioMutex must be held)
    bool DecodeMp3Sample(size_t index, std::vector<int16_t>& pcm, int& channels, int& hz);

    mutable std::vector<uint8_t> _rgba;
    mutable uint64_t _rgbaVersion = ~0ull;

    void InitDecoder();
    void DestroyDecoder();
    void UpdateAudio();
};