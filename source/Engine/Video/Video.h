#pragma once
#include <vector>
#include <cstdint>
#include <cstddef>
#include <atomic>
#include <memory>

// Define VIDEO_NO_THREADING to disable background decoding thread
// (the threaded path only applies to MPEG-1; MP4/H.264 decoding is always synchronous)
#define VIDEO_NO_THREADING

#ifndef VIDEO_NO_THREADING
#include <mutex>
#include <condition_variable>
#include <thread>
#endif

class Video {
public:
    // Create from memory buffer. Container is auto-detected:
    //   .mp4 (ISO BMFF with an H.264/avc1 track) -> h264bsd
    //   anything else                            -> pl_mpeg (MPEG-1 program stream)
    // Returns nullptr if the data is an MP4 that can't be parsed.
    static Video* FromMemory(const uint8_t* bytes, size_t length);
    static Video* FromVector(const std::vector<uint8_t>& vec);

    Video() = default;
    ~Video();

    Video(const Video&) = delete;
    Video& operator=(const Video&) = delete;
    Video(Video&&) = delete;            // holds atomics / a raw decoder handle
    Video& operator=(Video&&) = delete;

    // Synchronously decode a frame at given time (seconds)
    // Returns true on success. RGB24 output in out_rgb, dimensions in out_w/out_h
    bool GetFrameAtTime(double seconds,
        std::vector<uint8_t>& out_rgb,
        int& out_w, int& out_h,
        double timeout_seconds = 10.0);

    // True if _data is an MP4 with a supported H.264 track. Probed lazily, so it also works
    // if _data was filled in directly instead of through FromMemory/FromVector.
    bool IsMp4() const;

    struct FrameGrab
    {
        std::vector<uint8_t> y_plane;
        std::vector<uint8_t> cb_plane;
        std::vector<uint8_t> cr_plane;
        int y_stride = 0;   // == width after StoreI420
        int c_stride = 0;   // == (width + 1) / 2
        int width = 0, height = 0;
        std::atomic<bool> have_frame{ false };

#ifndef VIDEO_NO_THREADING
        std::mutex mtx;
#endif
    };

    // Parsed MP4 sample table for the first H.264 video track
    struct Mp4Info {
        uint32_t timescale = 0;
        int nal_length_size = 4;
        std::vector<uint8_t> headers_annexb;   // SPS + PPS with start codes
        std::vector<uint64_t> sample_offset;
        std::vector<uint32_t> sample_size;
        std::vector<uint64_t> sample_time;     // decode time, in timescale units
        std::vector<uint32_t> sync_samples;    // 0-based, sorted. Empty => every sample is a sync sample
        // Audio tracks only:
        int channels = 0;
        int sample_rate = 0;
        uint8_t object_type = 0;               // esds objectTypeIndication: 0x40 = AAC, 0x69/0x6B = MP3
        std::vector<uint8_t> asc;              // AudioSpecificConfig (AAC)
    };

    // Stateful H.264 decoder over a parsed MP4. Each VideoInstance owns its own,
    // so several instances can play the same Video independently.
    // References `data` and `info`, so the owning Video must outlive it.
    class Mp4Decoder {
    public:
        Mp4Decoder(const std::vector<uint8_t>& data, const Mp4Info& info);
        ~Mp4Decoder();
        Mp4Decoder(const Mp4Decoder&) = delete;
        Mp4Decoder& operator=(const Mp4Decoder&) = delete;

        // Make the current picture the one shown at `seconds`. Moving forward inside the
        // same GOP just decodes the missing samples; anything else restarts at a keyframe.
        bool DecodeToTime(double seconds, double timeout_seconds = 10.0);

        // Index of the sample whose picture is current (-1 = nothing decoded yet).
        int64_t CurrentSample() const { return _cur; }
        bool HasFrame() const { return _grab.have_frame.load(std::memory_order_acquire); }
        bool ToRgb(std::vector<uint8_t>& out_rgb, int& out_w, int& out_h) const;
        double Duration() const;

    private:
        void Reset();
        bool DecodeSample(size_t index, bool prepend_headers);

        const std::vector<uint8_t>& _data;
        const Mp4Info& _m;
        void* _h264 = nullptr;      // storage_t*
        int64_t _cur = -1;
        FrameGrab _grab;
    };

    // First audio track of the MP4 (nullptr if none). Sample data lives in _data.
    const Mp4Info* GetMp4Audio() const;

    // Creates an independent decoder for this MP4 (nullptr if the Video isn't an MP4).
    std::unique_ptr<Mp4Decoder> CreateMp4Decoder() const;

    std::vector<uint8_t> _data;

    static inline void yuv_to_rgb_pixel(int Y, int Cb, int Cr, uint8_t& R, uint8_t& G, uint8_t& B) {
        int c = Y - 16;
        int d = Cb - 128;
        int e = Cr - 128;
        int r = (298 * c + 409 * e + 128) >> 8;
        int g = (298 * c - 100 * d - 208 * e + 128) >> 8;
        int b = (298 * c + 516 * d + 128) >> 8;
        if (r < 0) r = 0; else if (r > 255) r = 255;
        if (g < 0) g = 0; else if (g > 255) g = 255;
        if (b < 0) b = 0; else if (b > 255) b = 255;
        R = (uint8_t)r; G = (uint8_t)g; B = (uint8_t)b;
    }

    // Convert a grabbed I420 frame to RGB24
    static bool GrabToRgb(const FrameGrab& g, std::vector<uint8_t>& out_rgb, int& out_w, int& out_h);

private:
    bool ParseMp4() const;
    void ProbeMp4() const;

    mutable std::unique_ptr<Mp4Info> _mp4;
    mutable std::unique_ptr<Mp4Info> _mp4_audio;
    mutable bool _mp4_probed = false;
    std::unique_ptr<Mp4Decoder> _mp4_dec;   // used only by GetFrameAtTime()

#ifndef VIDEO_NO_THREADING
public:
    // Worker thread (MPEG-1 only)
    void ensure_worker_running();
    void stop_worker();
    void worker_thread_func();

    std::thread _worker;
    std::atomic<bool> _worker_running{ false };
    std::atomic<bool> _worker_stop{ false };
    std::mutex _ctl_mutex;
    std::condition_variable _ctl_cv;
    double _target_time = 0.0;
    bool _frame_requested = false;

    std::mutex _frame_cv_mutex;
    std::condition_variable _frame_cv;

    FrameGrab _latest_grab;
#endif
};
