#include "ThreadPool.h"

#ifndef DISABLE_TREADPOOL

#include <thread>
#include <chrono>
using namespace std::chrono_literals;
#endif

void ThreadPool::Start(uint32_t num_threads) {
#ifdef DISABLE_THREADPOOL
    (void)num_threads;
#else
    std::lock_guard<std::mutex> lk(mtx_);
    if (!threads_.empty()) return; // already started
    stopping_ = false;
    threads_.reserve(num_threads);
    for (uint32_t i = 0; i < num_threads; ++i) {
        threads_.emplace_back(&ThreadPool::Worker, this);
    }
#endif
}

void ThreadPool::Worker() {
#ifndef DISABLE_THREADPOOL
    for (;;) {
        std::function<void()> job;
        {
            std::unique_lock<std::mutex> lk(mtx_);
            cv_job_.wait(lk, [this] {
                return stopping_ || !jobs_.empty();
                });
            if (stopping_ && jobs_.empty()) {
                return; // graceful exit after draining
            }
            job = std::move(jobs_.front());
            jobs_.pop();
            performingJobs.fetch_add(1, std::memory_order_relaxed);
        }

        job(); // run outside lock

        {
            std::lock_guard<std::mutex> lk(mtx_);
            performingJobs.fetch_sub(1, std::memory_order_relaxed);
            // Decrement work counter and signal completion when it hits zero.
            if (work_count_.fetch_sub(1, std::memory_order_acq_rel) == 1) {
                cv_done_.notify_all();
            }
        }
    }
#endif
}

void ThreadPool::QueueJob(const std::function<void()>& job) {
#ifdef DISABLE_THREADPOOL
    // Run inline when disabled.
    performingJobs.fetch_add(1, std::memory_order_relaxed);
    job();
    performingJobs.fetch_sub(1, std::memory_order_relaxed);
#else
    {
        std::lock_guard<std::mutex> lk(mtx_);
        jobs_.push(job); // copy here is fine; prefer the rvalue overload below for zero-copy
        work_count_.fetch_add(1, std::memory_order_acq_rel);
    }
    cv_job_.notify_one(); // notify after unlock
#endif
}

void ThreadPool::QueueJob(std::function<void()>&& job) {
#ifdef DISABLE_THREADPOOL
    performingJobs.fetch_add(1, std::memory_order_relaxed);
    job();
    performingJobs.fetch_sub(1, std::memory_order_relaxed);
#else
    {
        std::lock_guard<std::mutex> lk(mtx_);
        jobs_.push(std::move(job));
        work_count_.fetch_add(1, std::memory_order_acq_rel);
    }
    cv_job_.notify_one();
#endif
}

template <class F, class... Args>
auto ThreadPool::Enqueue(F&& f, Args&&... args)
-> std::future<std::invoke_result_t<F, Args...>> {
    using R = std::invoke_result_t<F, Args...>;
    auto task = std::make_shared<std::packaged_task<R()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));
    std::future<R> fut = task->get_future();
    QueueJob([task]() mutable { (*task)(); });
    return fut;
}

void ThreadPool::WaitForFinish() {
#ifndef DISABLE_THREADPOOL
    std::unique_lock<std::mutex> lk(mtx_);
    cv_done_.wait(lk, [this] { return work_count_.load(std::memory_order_acquire) == 0; });
#endif
}

void ThreadPool::Stop() {
#ifndef DISABLE_THREADPOOL
    {
        std::lock_guard<std::mutex> lk(mtx_);
        if (threads_.empty()) return;
        // Let workers exit after they drain jobs_
        stopping_ = true;
    }
    cv_job_.notify_all();
    for (auto& t : threads_) t.join();
    threads_.clear();
    // cleanup for potential restart
    {
        std::lock_guard<std::mutex> lk(mtx_);
        while (!jobs_.empty()) jobs_.pop();
        stopping_ = false;
        work_count_.store(0, std::memory_order_release);
        performingJobs.store(0, std::memory_order_release);
    }
#endif
}

int ThreadPool::GetMaxThreads() {
#ifdef DISABLE_THREADPOOL
    return 0;
#else
    unsigned hc = std::thread::hardware_concurrency(); // may be 0 (unknown)
    // Reserve threads for game/sound/etc., then clamp to [1, hc] sensibly.
    int reserve = 3;
    int hint = hc ? static_cast<int>(hc) : 0;
    int usable = hint > reserve ? (hint - reserve) : 1;
    return std::max(1, usable);
#endif
}

// physics + async update do not overlap
int ThreadPool::GetNumThreadsForPhysics() 
{

#ifdef DISABLE_THREADPOOL
    return 0;
#endif

    int mx = GetMaxThreads();
    int n = (mx * 7) / 10; // 70%
    return std::max(n, 1);
}
int ThreadPool::GetNumThreadsForAsyncUpdate() {
    int mx = GetMaxThreads();
    int n = (mx * 7) / 10; // 70%
    return std::max(n, 1);
}
int ThreadPool::GetNumThreadsForThreadPool() {
    int mx = GetMaxThreads();
    int n = (mx * 3) / 10; // 30%
    return std::max(n, 1);
}