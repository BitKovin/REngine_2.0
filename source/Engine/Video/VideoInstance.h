#pragma once
#include "Video.h"
#include <memory>
#include <vector>
#include <cstdint>
#include <AL/al.h>
#include <AL/alc.h>

struct plm_t;

class VideoInstance {
public:
    VideoInstance(Video* video);
    ~VideoInstance();

    void Start();
    void Pause();
    void SetTime(float time);
    void Update(double deltaTime);
    void SetLoop(bool loop);

    // Returns pointer to RGB24 frame, or nullptr if no frame decoded yet
    const std::vector<uint8_t>& GetCurrentFrameData() const;
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

    void InitDecoder();
    void DestroyDecoder();
    void UpdateAudio();
};