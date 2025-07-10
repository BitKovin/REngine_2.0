#pragma once

#ifdef __EMSCRIPTEN__

#define DISABLE_TREADPOOL

#endif // __EMSCRIPTEN__

#define DISABLE_TREADPOOL

#include <mutex>
#include <queue>
#include <vector>
#include <functional>


#ifndef DISABLE_TREADPOOL
#include <atomic> // add this
#include <thread>
#include <condition_variable>
#endif

class ThreadPool {
public:
    void Start();
    void QueueJob(const std::function<void()>& job);
    void Stop();
    bool IsBusy();

    std::atomic<int> performingJobs{ 0 };

    std::condition_variable finished_condition;

    void WaitForFinish();

    static int GetMaxThreads();

    static inline bool Supported()
    {
#ifdef DISABLE_TREADPOOL
        return false;
#endif // DISABLE_TREADPOOL
        return true;
    }

private:
    void ThreadLoop();

#ifndef DISABLE_TREADPOOL

    bool should_terminate = false;           // Tells threads to stop looking for jobs
    std::mutex queue_mutex;                  // Prevents data races to the job queue
    std::condition_variable mutex_condition; // Allows threads to wait on new jobs or termination 
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> jobs;

#endif // !DISABLE_TREADPOOL



};