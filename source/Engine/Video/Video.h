#pragma once
#include <vector>
#include <cstdint>
#include <atomic>

// Define VIDEO_NO_THREADING to disable background decoding thread
#define VIDEO_NO_THREADING

class Video {
public:
    // Create from memory buffer
    static Video* FromMemory(const uint8_t* bytes, size_t length);
    static Video* FromVector(const std::vector<uint8_t>& vec);

    Video() = default;
    ~Video();

    Video(const Video&) = delete;
    Video& operator=(const Video&) = delete;

    Video(Video&&) noexcept = default;
    Video& operator=(Video&&) noexcept = default;



    // Synchronously decode a frame at given time (seconds)
    // Returns true on success. RGB24 output in out_rgb, dimensions in out_w/out_h
    bool GetFrameAtTime(double seconds,
        std::vector<uint8_t>& out_rgb,
        int& out_w, int& out_h,
        double timeout_seconds = 10.0);

    struct FrameGrab 
    {
         std::vector<uint8_t> y_plane;
        std::vector<uint8_t> cb_plane;
        std::vector<uint8_t> cr_plane;
        int y_stride = 0;
        int c_stride = 0;
        int width = 0, height = 0;
        std::atomic<bool> have_frame{ false };

#ifndef VIDEO_NO_THREADING
        std::mutex mtx;
#endif // !VIDEO_NO_THREADING

    };

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

#ifndef VIDEO_NO_THREADING
    // Worker thread
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
