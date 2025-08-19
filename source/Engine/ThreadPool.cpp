#include "ThreadPool.h"

#ifndef DISABLE_TREADPOOL

#include <thread>
#include <chrono>
using namespace std::chrono_literals;
#endif

void ThreadPool::Start(const uint32_t num_threads) 
{
#ifndef DISABLE_TREADPOOL
	for (uint32_t ii = 0; ii < num_threads; ++ii)
	{
		threads.emplace_back(std::thread(&ThreadPool::ThreadLoop, this));
	}
#endif
}


void ThreadPool::ThreadLoop() {
#ifndef DISABLE_TREADPOOL
	while (true) {
		std::function<void()> job;
		{
			std::unique_lock<std::mutex> lock(queue_mutex);
			mutex_condition.wait(lock, [this] {
				return !jobs.empty() || should_terminate;
				});

			if (should_terminate && jobs.empty()) {
				return;
			}

			job = std::move(jobs.front());
			jobs.pop();
			performingJobs++;
		}

		job();
		{
			std::unique_lock<std::mutex> lock(queue_mutex);
			performingJobs--;
			if (jobs.empty() && performingJobs == 0) {
				finished_condition.notify_all();
			}
		}
	}
#endif
}


void ThreadPool::QueueJob(const std::function<void()>& job)
{

#ifdef DISABLE_TREADPOOL
	job();
#else


	std::unique_lock<std::mutex> lock(queue_mutex);
	jobs.push(job);

	mutex_condition.notify_one();
#endif
}

bool ThreadPool::IsBusy() {
#ifdef DISABLE_TREADPOOL
	return false;
#else
	// Simply check the conditions without locking here
	return !jobs.empty() || performingJobs > 0;
#endif
}


void ThreadPool::WaitForFinish() {
#ifndef DISABLE_TREADPOOL
	std::unique_lock<std::mutex> lock(queue_mutex);
	finished_condition.wait(lock, [this] {
		return !IsBusy();
		});
#endif
}

int ThreadPool::GetMaxThreads()
{
#ifdef __EMSCRIPTEN__

	return 4;

#else

	return std::max(std::thread::hardware_concurrency() - 1 - 2, 1u); //1 is always taken by game thread and about 2 by sound engine

#endif 
}

//physics thread and async update aren't performed at the same time

int ThreadPool::GetNumThreadsForPhysics()
{

#ifdef DISABLE_TREADPOOL

	return 0;

#endif

	return ceil(GetMaxThreads() * 0.6);
}

int ThreadPool::GetNumThreadsForAsyncUpdate()
{
	return ceil(GetMaxThreads() * 0.6);
}

int ThreadPool::GetNumThreadsForThreadPool()
{
	return ceil(GetMaxThreads() * 0.4);
}


void ThreadPool::Stop()
{

#ifndef DISABLE_TREADPOOL

	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		should_terminate = true;
	}
	mutex_condition.notify_all();
	for (std::thread& active_thread : threads) {
		active_thread.join();
	}
	threads.clear();

#endif

}