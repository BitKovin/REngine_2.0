#include "Video.h"
#include <iostream>
#include <mutex>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstring>
#include <thread>

#define PL_MPEG_IMPLEMENTATION
extern "C" {
#include "pl_mpeg.h"
}


// pl_mpeg callback
static void plm_video_cb(plm_t* plm, plm_frame_t* frame, void* user) {
    if (!user || !frame) return;
    Video::FrameGrab* grab = (Video::FrameGrab*)user;
#ifndef VIDEO_NO_THREADING
    std::lock_guard<std::mutex> lk(grab->mtx);
#endif

    int w = frame->width ? frame->width : plm_get_width(plm);
    int h = frame->height ? frame->height : plm_get_height(plm);

    grab->width = w;
    grab->height = h;

    // Y plane
    size_t y_size = (size_t)h * w;
    grab->y_plane.resize(y_size);
    std::memcpy(grab->y_plane.data(), frame->y.data, y_size);

    // Chroma planes (half resolution)
    int ch = (h + 1) / 2;
    int cw = (w + 1) / 2;

    size_t c_size = (size_t)ch * cw;
    grab->cb_plane.resize(c_size);
    grab->cr_plane.resize(c_size);

    std::memcpy(grab->cb_plane.data(), frame->cb.data, c_size);
    std::memcpy(grab->cr_plane.data(), frame->cr.data, c_size);

    grab->have_frame.store(true, std::memory_order_release);
}


// ---- Implementation ----
Video* Video::FromMemory(const uint8_t* bytes, size_t length) {
    Video* v = new Video();
    v->_data.assign(bytes, bytes + length);
    return v;
}

Video* Video::FromVector(const std::vector<uint8_t>& vec) {
    Video* v = new Video();
    v->_data = vec;
    return v;
}

Video::~Video() {
#ifndef VIDEO_NO_THREADING
    stop_worker();
#endif
}

bool Video::GetFrameAtTime(double seconds, std::vector<uint8_t>& out_rgb,
    int& out_w, int& out_h, double timeout_seconds) {
    out_rgb.clear();
    out_w = out_h = 0;
    if (_data.empty()) return false;
    if (seconds < 0.0) seconds = 0.0;

#ifndef VIDEO_NO_THREADING
    ensure_worker_running();

    {
        std::unique_lock<std::mutex> lk(_ctl_mutex);
        _target_time = seconds;
        _frame_requested = true;
        _ctl_cv.notify_one();
    }

    auto deadline = std::chrono::steady_clock::now() + std::chrono::duration<double>(timeout_seconds);
    std::unique_lock<std::mutex> lk(_frame_cv_mutex);
    while (!_latest_grab.have_frame.load(std::memory_order_acquire)) {
        if (std::chrono::steady_clock::now() >= deadline) return false;
        _frame_cv.wait_until(lk, deadline);
    }

    // convert grabbed frame to RGB
    {
        std::lock_guard<std::mutex> glk(_latest_grab.mtx);
        int w = _latest_grab.width;
        int h = _latest_grab.height;
        if (w <= 0 || h <= 0) return false;
        out_w = w; out_h = h;
        out_rgb.resize((size_t)w * h * 3);

        const uint8_t* yptr = _latest_grab.y_plane.data();
        const uint8_t* cbptr = _latest_grab.cb_plane.data();
        const uint8_t* crptr = _latest_grab.cr_plane.data();
        int ys = _latest_grab.y_stride;
        int cs = _latest_grab.c_stride;

        uint8_t* dst = out_rgb.data();
        for (int r = 0; r < h; ++r) {
            const uint8_t* yrow = yptr + r * ys;
            const uint8_t* cbrow = cbptr + (r / 2) * cs;
            const uint8_t* crrow = crptr + (r / 2) * cs;
            for (int c = 0; c < w; ++c) {
                int Y = yrow[c];
                int Cx = c / 2;
                int Cb = cbrow[Cx];
                int Cr = crrow[Cx];
                uint8_t R, G, B;
                yuv_to_rgb_pixel(Y, Cb, Cr, R, G, B);
                *dst++ = R; *dst++ = G; *dst++ = B;
            }
        }
    }
    _latest_grab.have_frame.store(false, std::memory_order_release);
    return true;

#else
    // Single-threaded: create plm_t each call
    plm_t* plm = plm_create_with_memory((uint8_t*)_data.data(), _data.size(), 0);
    if (!plm) return false;

    FrameGrab grab;
    plm_set_video_decode_callback(plm, plm_video_cb, &grab);

    const double dt_step = 1.0 / 60.0;
    double acc_time = 0.0;
    const auto tstart = std::chrono::steady_clock::now();
    while (acc_time < seconds + 1e-6) {
        plm_decode(plm, dt_step);
        acc_time += dt_step;
        if (std::chrono::duration<double>(std::chrono::steady_clock::now() - tstart).count() > timeout_seconds) {
            plm_destroy(plm);
            return false;
        }
    }

    if (!grab.have_frame.load(std::memory_order_acquire)) {
        plm_destroy(plm);
        return false;
    }

    // convert to RGB
    {
#ifndef VIDEO_NO_THREADING
        std::lock_guard<std::mutex> lk(grab.mtx);
#endif
        int w = grab.width;
        int h = grab.height;
        if (w <= 0 || h <= 0) { plm_destroy(plm); return false; }
        out_w = w; out_h = h;
        out_rgb.resize((size_t)w * h * 3);

        const uint8_t* yptr = grab.y_plane.data();
        const uint8_t* cbptr = grab.cb_plane.data();
        const uint8_t* crptr = grab.cr_plane.data();
        int ys = grab.y_stride;
        int cs = grab.c_stride;

        uint8_t* dst = out_rgb.data();
        for (int r = 0; r < h; ++r) {
            const uint8_t* yrow = yptr + r * ys;
            const uint8_t* cbrow = cbptr + (r / 2) * cs;
            const uint8_t* crrow = crptr + (r / 2) * cs;
            for (int c = 0; c < w; ++c) {
                int Y = yrow[c];
                int Cx = c / 2;
                int Cb = cbrow[Cx];
                int Cr = crrow[Cx];
                uint8_t R, G, B;
                yuv_to_rgb_pixel(Y, Cb, Cr, R, G, B);
                *dst++ = R; *dst++ = G; *dst++ = B;
            }
        }
    }

    plm_destroy(plm);
    return true;
#endif
}

#ifndef VIDEO_NO_THREADING
void Video::ensure_worker_running() {
    if (_worker_running.load(std::memory_order_acquire)) return;
    std::lock_guard<std::mutex> lk(_ctl_mutex);
    if (_worker_running.load(std::memory_order_relaxed)) return;
    _worker_stop.store(false, std::memory_order_release);
    _worker = std::thread(&Video::worker_thread_func, this);
    _worker_running.store(true, std::memory_order_release);
}

void Video::stop_worker() {
    if (!_worker_running.load(std::memory_order_acquire)) return;
    {
        std::lock_guard<std::mutex> lk(_ctl_mutex);
        _worker_stop.store(true, std::memory_order_release);
        _ctl_cv.notify_one();
    }
    if (_worker.joinable()) _worker.join();
    _worker_running.store(false, std::memory_order_release);
}

void Video::worker_thread_func() {
    plm_t* plm = plm_create_with_memory((uint8_t*)_data.data(), _data.size(), 0);
    if (!plm) { std::cerr << "worker: plm_create_with_memory failed\n"; return; }
    plm_set_video_decode_callback(plm, plm_video_cb, &_latest_grab);

    double local_time = 0.0;
    const double dt_step = 1.0 / 60.0;

    while (true) {
        double req_time = 0.0;
        {
            std::unique_lock<std::mutex> lk(_ctl_mutex);
            _ctl_cv.wait(lk, [this] { return _frame_requested || _worker_stop.load(std::memory_order_acquire); });
            if (_worker_stop.load(std::memory_order_acquire)) break;
            req_time = _target_time;
            _frame_requested = false;
        }

        while (local_time < req_time && !_worker_stop.load(std::memory_order_acquire)) {
            plm_decode(plm, dt_step);
            local_time += dt_step;

            if (_latest_grab.have_frame.load(std::memory_order_acquire)) {
                std::lock_guard<std::mutex> lk(_frame_cv_mutex);
                _frame_cv.notify_all();
            }
        }

        std::lock_guard<std::mutex> lk(_frame_cv_mutex);
        _frame_cv.notify_all();
    }

    plm_destroy(plm);
}
#endif
