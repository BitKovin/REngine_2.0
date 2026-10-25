#pragma once
// EcsGuard.h
//
// Thread-safety layer for the single global entt::registry.
//
// WHY THIS EXISTS
// entt::registry has NO internal synchronisation. Entity construction
// (create + emplace), destruction, and even the first use of a component
// type (which inserts a pool into the registry's pool map) all mutate shared
// structures. Doing any of that from two threads at once - or while a system
// is iterating a view - corrupts the sparse/packed arrays, and the damage
// surfaces later as an access violation inside storage::pop().
//
// THE MODEL (one readers/writer gate, ECS-wide)
//
//   SHARED    - "the registry's structure will not change while I hold this".
//               Held by the scheduler for the whole parallel part of a tick
//               group, and briefly by every read through EcsWorld /
//               ComponentRef. Any number of threads may hold it.
//
//   EXCLUSIVE - "I change the registry's structure". Taken by every
//               create / destroy / emplace / remove / reset. Waits for all
//               readers to leave, and blocks readers while held.
//
// Consequences:
//   * Worker threads (async preload, loaders, network) can construct and
//     destroy Entities freely; their registry work is serialised against each
//     other AND against tick groups. Their slow work (LoadAssets, IO) stays
//     outside the gate and remains parallel.
//   * Both sides are re-entrant on the owning thread, so wrapper calls can
//     nest (Entity ctor -> EcsWorld::Create -> ...) without self-deadlock.
//   * A thread that holds SHARED (a system running inside a tick group)
//     can NOT upgrade to EXCLUSIVE. That is a programming error (use
//     SystemContext::Defer/AddComponent/Destroy, which flush at the end of the
//     tick group). It is detected, reported through ReportViolation(), and
//     the call proceeds unguarded exactly as it did before this layer
//     existed - it never deadlocks. Define ECS_GUARD_STRICT to make it assert.
//   * Writers acquire by polling try_lock() rather than queueing on the
//     lock. A pending writer therefore never blocks new readers, which removes
//     the classic "reader waits for pool job, pool job waits behind queued
//     writer, writer waits for reader" deadlock. Stalls > 2s are logged.
//
// Kill switch: -DECS_GUARD_DISABLED turns every guard into a no-op (used for
// A/B testing the gate itself).

#include <atomic>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <shared_mutex>
#include <sstream>
#include <thread>

namespace EcsGuard
{
    struct ThreadState
    {
        int exclusiveDepth = 0; // re-entrancy count of EXCLUSIVE held by this thread
        int sharedDepth    = 0; // 1 if this thread holds SHARED
        int tickDepth      = 0; // >0 if this thread is a worker covered by some other thread's SHARED
    };
    inline thread_local ThreadState t_State;

    // True if the calling thread is already guaranteed a stable registry
    // structure by something it (or the thread that spawned it) holds.
    inline bool IsCovered()
    {
        return t_State.exclusiveDepth > 0 || t_State.sharedDepth > 0 || t_State.tickDepth > 0;
    }

    using ViolationHandler = void (*)(const char* message);
    inline std::atomic<ViolationHandler> g_ViolationHandler{ nullptr }; // route to your Logger
    inline std::atomic<uint32_t>         g_ViolationCount{ 0 };
    inline std::atomic<uint32_t>         g_StallCount{ 0 };

    inline void ReportViolation(const char* message)
    {
        const uint32_t n = g_ViolationCount.fetch_add(1, std::memory_order_relaxed) + 1;
        if (n > 32) return; // rate limit: first 32 only
        if (auto h = g_ViolationHandler.load()) h(message);
        else std::fprintf(stderr, "[EcsGuard] %s\n", message);
    }

#ifndef ECS_GUARD_DISABLED
    class Gate
    {
    public:
        static Gate& Get() { static Gate g; return g; }

        // Returns true if a release is owed (call UnlockExclusive exactly once).
        bool LockExclusive(const char* site)
        {
            auto& s = t_State;
            if (s.exclusiveDepth > 0) { ++s.exclusiveDepth; return true; }

            if (s.sharedDepth > 0 || s.tickDepth > 0)
            {
                std::ostringstream os;
                os << "Immediate structural ECS change (" << (site ? site : "?")
                   << ") requested from inside a tick group. Use SystemContext::Defer / AddComponent / "
                      "RemoveComponent / Destroy instead. Proceeding unguarded.";
                ReportViolation(os.str().c_str());
#ifdef ECS_GUARD_STRICT
                assert(false && "structural ECS change inside a tick group");
#endif
                return false;
            }

            using clock = std::chrono::steady_clock;
            const auto start = clock::now();
            auto lastReport = start;
            int spins = 0;
            while (!m_.try_lock())
            {
                if (++spins < 64) std::this_thread::yield();
                else std::this_thread::sleep_for(std::chrono::microseconds(100));

                const auto now = clock::now();
                if (now - lastReport > std::chrono::seconds(2))
                {
                    lastReport = now;
                    g_StallCount.fetch_add(1, std::memory_order_relaxed);
                    std::ostringstream os;
                    os << "ECS gate: exclusive request (" << (site ? site : "?") << ") has waited "
                       << std::chrono::duration_cast<std::chrono::seconds>(now - start).count()
                       << "s - a reader is holding the registry. Possible deadlock.";
                    ReportViolation(os.str().c_str());
                }
            }
            s.exclusiveDepth = 1;
            return true;
        }

        void UnlockExclusive()
        {
            if (--t_State.exclusiveDepth == 0) m_.unlock();
        }

        // Returns true if a release is owed (call UnlockShared exactly once).
        bool LockShared()
        {
            if (IsCovered()) return false; // nested / worker covered by owner: nothing to take
            m_.lock_shared();
            t_State.sharedDepth = 1;
            return true;
        }

        void UnlockShared()
        {
            t_State.sharedDepth = 0;
            m_.unlock_shared();
        }

    private:
        std::shared_mutex m_;
    };

    class ExclusiveGuard
    {
    public:
        explicit ExclusiveGuard(const char* site = "") : m_Held(Gate::Get().LockExclusive(site)) {}
        ~ExclusiveGuard() { if (m_Held) Gate::Get().UnlockExclusive(); }
        ExclusiveGuard(const ExclusiveGuard&) = delete;
        ExclusiveGuard& operator=(const ExclusiveGuard&) = delete;
    private:
        bool m_Held;
    };

    class SharedGuard
    {
    public:
        SharedGuard() : m_Held(Gate::Get().LockShared()) {}
        ~SharedGuard() { if (m_Held) Gate::Get().UnlockShared(); }
        SharedGuard(const SharedGuard&) = delete;
        SharedGuard& operator=(const SharedGuard&) = delete;
    private:
        bool m_Held;
    };
#else
    struct ExclusiveGuard { explicit ExclusiveGuard(const char* = "") {} };
    struct SharedGuard { SharedGuard() {} };
#endif

    // Marks the current thread as a worker running a job on behalf of a thread
    // that holds SHARED (scheduler batch jobs, SystemContext::ParallelFor).
    // Only apply it when the spawning thread is covered - see CoveredScope below.
    class WorkerScope
    {
    public:
        WorkerScope() { ++t_State.tickDepth; }
        ~WorkerScope() { --t_State.tickDepth; }
        WorkerScope(const WorkerScope&) = delete;
        WorkerScope& operator=(const WorkerScope&) = delete;
    };

    // Wraps a ParallelFor body so its worker threads inherit coverage, but only
    // if the thread calling ParallelFor actually holds the gate. Without that
    // check a ParallelFor issued outside a tick group would run unprotected.
    template<typename Fn>
    auto InheritCoverage(Fn fn)
    {
        const bool covered = IsCovered();
        return [fn = std::move(fn), covered](size_t i)
        {
            if (covered) { WorkerScope ws; fn(i); }
            else fn(i);
        };
    }
}
