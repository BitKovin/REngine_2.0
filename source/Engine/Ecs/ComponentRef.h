#pragma once
// ComponentRef.h
//
// A reference to a single component on a single entity that stays valid
// across frames and tick-group boundaries. Unlike a raw T*, it never goes
// stale when an unrelated emplace/remove elsewhere reallocates storage - it
// re-resolves through the registry on every dereference instead of caching
// a pointer.
//
// Thread safety: every registry lookup here (valid / all_of / get) is done
// under the ECS gate's SHARED side, so it cannot overlap a structural change
// from another thread (async entity preload, loaders, ...). The pointer
// returned by operator-> / operator* is only a snapshot: it stays good until
// the next structural change, which cannot happen while the caller is inside a
// tick group. Outside a tick group, don't hold it across anything that may
// destroy or re-add the component.
//
// Two ComponentRef<T> to the same entity from two threads still race on the
// component's VALUE exactly like registry.get<T>() would - the
// ParallelSafe()/RunsConcurrentlyWith() discipline on ISystem still applies.

#include <Ecs/EcsGuard.h>

#include <entt/entt.hpp>

template<typename T>
class ComponentRef
{
public:
    ComponentRef() = default;
    ComponentRef(entt::registry& registry, entt::entity entity)
        : m_Registry(&registry), m_Entity(entity) {}

    T* operator->() const { EcsGuard::SharedGuard g; return &m_Registry->get<T>(m_Entity); }
    T& operator*()  const { EcsGuard::SharedGuard g; return  m_Registry->get<T>(m_Entity); }

    // Use like an optional before dereferencing: if (auto h = entity->GetHealth()) h->Current -= dmg;
    explicit operator bool() const
    {
        if (m_Registry == nullptr) return false;
        EcsGuard::SharedGuard g;
        return m_Registry->valid(m_Entity) && m_Registry->all_of<T>(m_Entity);
    }

    void Remove() const
    {
        if (m_Registry == nullptr) return;
        EcsGuard::ExclusiveGuard g("ComponentRef::Remove");
        if (m_Registry->valid(m_Entity) && m_Registry->all_of<T>(m_Entity))
            m_Registry->remove<T>(m_Entity);
    }

    entt::entity    Entity()   const { return m_Entity; }
    entt::registry* Registry() const { return m_Registry; }

private:
    entt::registry* m_Registry = nullptr;
    entt::entity    m_Entity   = entt::null;
};
