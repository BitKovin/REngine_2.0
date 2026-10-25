#pragma once
// EcsWorld.h
//
// Static, no-instance access to the single registry backing every entity in
// the game. Every call is EcsWorld::Something(...) - there's no instance to
// fetch and no ::Get() anywhere.
//
// Naming mirrors Unity's GameObject/Component API on purpose - that's the
// vocabulary most C# gameplay programmers already have loaded:
//   TryGetComponent<T>(entity)   - returns something bool-checkable instead
//                                  of using an out parameter (C++ has none)
//   AddComponent<T>(entity, ...)
//   HasComponent<T>(entity)
//
// GetEntity() is the important one for this project specifically: it turns
// a bare entt::entity id (from a view, an event, wherever) back into the
// real, owning engine Entity - the only thing with actual identity
// (rendering, lifecycle). See EntityOwner.h.
//
// THREAD SAFETY (see EcsGuard.h for the full model)
// Every method here is safe to call from ANY thread. Structural changes
// (Create/Destroy/AddComponent/RemoveComponent/Reset) take the ECS gate
// EXCLUSIVELY; reads take it SHARED. Both are re-entrant per thread.
//
// The one rule: code running INSIDE a tick group (a system's Execute, or a
// ctx.ParallelFor body) holds the gate shared and must not make immediate
// structural changes - use the ctx versions (ctx.AddComponent / ctx.Destroy /
// ctx.Defer), which are deferred to the tick group's flush point. A violation
// is reported via EcsGuard::ReportViolation and never deadlocks.
//
// Anything that touches the raw entt::registry& directly (Registry(), or the
// registry passed to a Defer/Enqueue callback) bypasses these guards unless it
// is already inside one - the scheduler's flush is exclusive, so Defer
// callbacks are safe. Prefer the wrappers.

#include <Ecs/ComponentRef.h>
#include <Ecs/ComponentRegistry.h>
#include <Ecs/EcsGuard.h>
#include <Ecs/EntityOwner.h>

#include <entt/entt.hpp>
#include <atomic>
#include <cstdio>
#include <tuple>
#include <type_traits>
#include <utility>

class EcsWorld
{
public:
    EcsWorld() = delete;

    // Raw access. Caller is responsible for holding an EcsGuard (the scheduler
    // and CommandBuffer do). Prefer the wrappers below.
    static entt::registry& Registry() { return Storage(); }

    // Bare id with no owner. Entities should use CreateOwned.
    static entt::entity Create()
    {
        EcsGuard::ExclusiveGuard g("EcsWorld::Create");
        PrewarmLocked();
        return Storage().create();
    }

    // Atomically creates an id AND attaches its EntityOwner backlink, so no
    // other thread can ever observe an entity that exists but has no owner.
    // Called from Entity's constructor - which means from whatever thread is
    // constructing the Entity (game thread, async preload worker, network...).
    static entt::entity CreateOwned(Entity* owner)
    {
        EcsGuard::ExclusiveGuard g("EcsWorld::CreateOwned");
        PrewarmLocked();
        auto& r = Storage();
        const entt::entity e = r.create();
        r.emplace<EntityOwner>(e, EntityOwner{ owner });
        return e;
    }

    // Destroys the registry row for an Entity, but ONLY if the row still
    // belongs to that exact Entity. Protects against:
    //   * double-destroy,
    //   * a stale handle surviving a Reset() whose id has since been reused by
    //     an unrelated entity (which would otherwise tear out that entity's row).
    static void DestroyOwned(entt::entity e, Entity* owner)
    {
        if (e == entt::null) return;
        EcsGuard::ExclusiveGuard g("EcsWorld::DestroyOwned");
        auto& r = Storage();
        if (!r.valid(e)) return;
        auto* o = r.try_get<EntityOwner>(e);
        if (!o || o->Owner != owner)
        {
            EcsGuard::ReportViolation("DestroyOwned: registry row does not belong to this Entity "
                                      "(stale handle or double destroy) - skipped.");
            return;
        }
        r.destroy(e);
    }

    static bool IsValid(entt::entity e)
    {
        EcsGuard::SharedGuard g;
        return Storage().valid(e);
    }

    // Destroys an id, doing the right thing based on what it actually is:
    // for one backed by a real Entity, calls the entity's own Destroy()
    // (physics, owner notification, and actual Level removal - not just
    // the ECS-side row). For one with no owning Entity (e.g. one from
    // ComponentRegistry::Clone), just clears the row directly.
    //
    // Immediate, not deferred. Inside a system, prefer ctx.Destroy(), which
    // defers this to the tick group's flush point.
    static void Destroy(entt::entity e); // EcsWorld.cpp

    // Recovers the owning engine Entity from a raw entt::entity id.
    // Returns nullptr if e is invalid, was never a real Entity, or has
    // already had its row destroyed.
    static Entity* GetEntity(entt::entity e)
    {
        EcsGuard::SharedGuard g;
        auto& r = Storage();
        if (!r.valid(e)) return nullptr;
        auto* o = r.try_get<EntityOwner>(e);
        return o ? o->Owner : nullptr;
    }

    // Safe even if e is invalid or lacks T - check with `if (auto h = ...)`
    // before dereferencing, same pattern as Unity's TryGetComponent.
    template<typename T>
    static ComponentRef<T> TryGetComponent(entt::entity e) { return ComponentRef<T>(Storage(), e); }

    // Upsert (replaces the component if one already exists).
    template<typename T, typename... Args>
    static T& AddComponent(entt::entity e, Args&&... args)
    {
        static_assert(!std::is_same_v<T, EntityOwner>,
            "EntityOwner is managed internally by Entity's constructor/destructor - never add or remove it directly.");
        EcsGuard::ExclusiveGuard g("EcsWorld::AddComponent");
        return Storage().emplace_or_replace<T>(e, std::forward<Args>(args)...);
    }

    // Strict add (asserts if already present). Used by ECS_COMPONENT, which
    // passes its constructor args as a tuple so a zero-arg use needs no
    // __VA_OPT__ / C++20.
    template<typename T, typename Tuple>
    static void EmplaceFromTuple(entt::entity e, Tuple&& args)
    {
        EcsGuard::ExclusiveGuard g("EcsWorld::Emplace(ECS_COMPONENT)");
        std::apply([&](auto&&... a) { Storage().template emplace<T>(e, std::forward<decltype(a)>(a)...); },
                   std::forward<Tuple>(args));
    }

    template<typename T>
    static bool HasComponent(entt::entity e)
    {
        EcsGuard::SharedGuard g;
        return Storage().all_of<T>(e);
    }

    template<typename T>
    static void RemoveComponent(entt::entity e)
    {
        static_assert(!std::is_same_v<T, EntityOwner>,
            "EntityOwner is managed internally by Entity's constructor/destructor - never add or remove it directly.");
        EcsGuard::ExclusiveGuard g("EcsWorld::RemoveComponent");
        Storage().remove<T>(e);
    }

    // Wipes every entity but KEEPS the component pools, so (a) the pre-created
    // pools survive and (b) entity versions are bumped - any stale entt::handle
    // that outlived the level now reports !valid() instead of aliasing a new
    // entity. (The old `Storage() = entt::registry{}` reset versions to zero and
    // dropped every pool.)
    //
    // NOT safe to call from inside a system's Execute() or an event handler.
    static void Reset()
    {
        EcsGuard::ExclusiveGuard g("EcsWorld::Reset");
        Storage().clear();
    }

    // Creates a pool for every REGISTER_COMPONENT'd type (and EntityOwner) up
    // front. The first use of a component type otherwise inserts a pool into
    // the registry's pool map - a structural change that could happen while a
    // system is iterating. Idempotent and cheap; also invoked automatically by
    // Create/CreateOwned whenever new component types have been registered.
    static void PrewarmPools()
    {
        EcsGuard::ExclusiveGuard g("EcsWorld::PrewarmPools");
        PrewarmLocked();
    }

private:
    static void PrewarmLocked()
    {
        const uint32_t v = ComponentRegistry::Version();
        if (s_PrewarmedVersion == v) return;
        auto& r = Storage();
        (void)r.storage<EntityOwner>();
        for (auto& [id, info] : ComponentRegistry::All())
            if (info.EnsureStorage) info.EnsureStorage(r);
        s_PrewarmedVersion = v;
    }

    static inline uint32_t s_PrewarmedVersion = ~0u; // only read/written under the EXCLUSIVE gate

    static entt::registry& Storage()
    {
        static entt::registry registry;
        return registry;
    }
};
