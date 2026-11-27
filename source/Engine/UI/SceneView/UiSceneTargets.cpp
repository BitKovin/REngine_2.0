#include "UiSceneTargets.h"

#include <SDL2/SDL.h>

#include <algorithm>
#include <list>

namespace
{
    constexpr uint64_t kFreeAfterMs = 3000;

    struct Entry
    {
        const void* Owner = nullptr;   // null: in the pool
        UiSceneTarget Target;
        uint64_t LastUsed = 0;
    };

    std::list<Entry>& Entries()
    {
        static std::list<Entry>* entries = new std::list<Entry>();   // never destroyed: views die in static destruction after this
        return *entries;
    }

    void Destroy(UiSceneTarget& t)
    {
        delete t.Buffer;
        delete t.Color;
        delete t.Depth;
        t = UiSceneTarget();
    }

    UiSceneTarget Create(glm::ivec2 size)
    {
        UiSceneTarget t;
        t.Size = size;
        t.Color = new RenderTexture(static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y), TextureFormat::RGBA8);
        t.Depth = new RenderTexture(static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y), TextureFormat::Depth24Stencil8);
        t.Buffer = new Framebuffer();
        t.Buffer->attachColor(t.Color, 0);
        t.Buffer->attachDepth(t.Depth);
        return t;
    }
}

UiSceneTarget* UiSceneTargets::Request(const void* owner, glm::ivec2 size)
{
    const uint64_t now = SDL_GetTicks64();
    auto& entries = Entries();
    for (Entry& e : entries)
        if (e.Owner == owner)
        {
            if (e.Target.Size == size)
            {
                e.LastUsed = now;
                return &e.Target;
            }
            e.Owner = nullptr;   // the size changed: the old target goes to the pool
            break;
        }
    for (Entry& e : entries)
        if (e.Owner == nullptr && e.Target.Size == size)
        {
            e.Owner = owner;
            e.LastUsed = now;
            return &e.Target;
        }
    Entry entry;
    entry.Owner = owner;
    entry.Target = Create(size);
    entry.LastUsed = now;
    entries.push_back(entry);
    return &entries.back().Target;
}

void UiSceneTargets::Release(const void* owner)
{
    for (Entry& e : Entries())
        if (e.Owner == owner)
        {
            e.Owner = nullptr;
            e.LastUsed = SDL_GetTicks64();
        }
}

void UiSceneTargets::EndFrame()
{
    const uint64_t now = SDL_GetTicks64();
    auto& entries = Entries();
    for (auto it = entries.begin(); it != entries.end();)
    {
        if (now - it->LastUsed > kFreeAfterMs)
        {
            Destroy(it->Target);
            it = entries.erase(it);
        }
        else
            ++it;
    }
}

int UiSceneTargets::Count() { return static_cast<int>(Entries().size()); }
