#include "UiDrawList.h"

#include <UI/Render/UiGpu.h>
#include <UI/Render/UiGradientRamps.h>

#include <algorithm>
#include <utility>

namespace
{
    bool Intersects(const glm::vec4& a, const glm::vec4& b)
    {
        return a.x < b.z && b.x < a.z && a.y < b.w && b.y < a.w;
    }

    glm::vec4 BoundsOf(const UiRender::Vertex* verts, uint32_t count)
    {
        glm::vec4 b(verts[0].X, verts[0].Y, verts[0].X, verts[0].Y);
        for (uint32_t i = 1; i < count; ++i)
        {
            b.x = std::min(b.x, verts[i].X);
            b.y = std::min(b.y, verts[i].Y);
            b.z = std::max(b.z, verts[i].X);
            b.w = std::max(b.w, verts[i].Y);
        }
        return b;
    }

    // Local ramp ids / aux indices become frame-global ones. Returns false when a ramp row could not be found.
    bool Resolve(UiRender::PrimBlock& block, int auxBase)
    {
        if (auxBase != 0)
        {
            if (block.T[0].w >= 0.f)
                block.T[0].w += static_cast<float>(auxBase);
            if (static_cast<int>(block.T[0].y) & UiRender::kFlagMask)
                block.T[15].z += static_cast<float>(auxBase);
            if (static_cast<int>(block.T[0].y) & UiRender::kFlagClipShape)
                block.T[15].w += static_cast<float>(auxBase);
        }
        if (block.T[11].x > 0.5f)
        {
            const int row = UiGradientRamps::Resolve(static_cast<int>(block.T[11].y + 0.5f));
            if (row < 0)
                return false;
            block.T[11].y = static_cast<float>(row);
        }
        if (static_cast<int>(block.T[0].y) & UiRender::kFlagMask)
        {
            const int row = UiGradientRamps::Resolve(static_cast<int>(block.T[15].y + 0.5f));
            if (row < 0)
                return false;
            block.T[15].y = static_cast<float>(row);
        }
        return true;
    }
}

void UiDrawList::Clear()
{
    m_items.clear();
    m_externals.clear();
    m_localPrims.clear();
    m_localAux.clear();
    m_barrier = false;
    m_fence = 0;
    m_resolvedFrom = nullptr;
    m_resolvedBase = -1;
    m_resolvedAuxBase = 0;
    m_merged = 0;
    for (uint32_t& count : m_breaks)
        count = 0;
}

uint32_t UiDrawList::TotalVertices() const
{
    uint32_t total = 0;
    for (const Item& item : m_items)
        total += static_cast<uint32_t>(item.Verts.size());
    return total;
}

uint32_t UiDrawList::TotalIndices() const
{
    uint32_t total = 0;
    for (const Item& item : m_items)
        total += static_cast<uint32_t>(item.Indices.size());
    return total;
}

void UiDrawList::AddTriangles(const UiRender::BatchKey& key, const UiRender::Vertex* verts, uint32_t vertCount,
                              const uint16_t* indices, uint32_t indexCount)
{
    if (vertCount == 0 || indexCount == 0)
        return;

    const glm::vec4 bounds = BoundsOf(verts, vertCount);

    auto append = [&](Item& item) {
        const uint16_t base = static_cast<uint16_t>(item.Verts.size());
        for (uint32_t i = 0; i < indexCount; ++i)
            item.Indices.push_back(static_cast<uint16_t>(indices[i] + base));
        item.Verts.insert(item.Verts.end(), verts, verts + vertCount);
        item.Bounds.x = std::min(item.Bounds.x, bounds.x);
        item.Bounds.y = std::min(item.Bounds.y, bounds.y);
        item.Bounds.z = std::max(item.Bounds.z, bounds.z);
        item.Bounds.w = std::max(item.Bounds.w, bounds.w);
    };

    if (m_barrier)
    {
        m_barrier = false;
    }
    else if (!m_items.empty())
    {
        // Join an item with an equal key: the last one, or one of the 7 before it that no later item overlaps.
        const size_t last = m_items.size() - 1;
        for (size_t i = last;; --i)
        {
            Item& candidate = m_items[i];
            if (!candidate.External && candidate.Key == key && candidate.Verts.size() + vertCount <= 65535)
            {
                append(candidate);
                if (i != last)
                    ++m_merged;
                return;
            }
            if (candidate.External || Intersects(candidate.Bounds, bounds))
                break;
            if (i == 0 || i <= m_fence || last - i >= 7)
                break;
        }

        const Item& previous = m_items.back();
        BreakReason reason;
        if (previous.External)
            reason = BreakReason::External;
        else if (previous.Key == key)
            reason = BreakReason::Capacity;
        else if (previous.Key.TextureIdx != key.TextureIdx)
            reason = BreakReason::Texture;
        else if (previous.Key.StencilRef != key.StencilRef)
            reason = BreakReason::Stencil;
        else
            reason = BreakReason::State;
        ++m_breaks[static_cast<int>(reason)];
    }

    Item item;
    item.Key = key;
    item.Bounds = bounds;
    item.Indices.assign(indices, indices + indexCount);
    item.Verts.assign(verts, verts + vertCount);
    m_items.push_back(std::move(item));
}

void UiDrawList::AddQuad(const UiRender::BatchKey& key, const UiRender::Vertex corners[4])
{
    static const uint16_t kQuadIndices[6] = { 0, 1, 2, 0, 2, 3 };
    AddTriangles(key, corners, 4, kQuadIndices, 6);
}

void UiDrawList::AddExternal(ExternalFn fn)
{
    Item item;
    item.External = true;
    item.ExternalIndex = static_cast<uint32_t>(m_externals.size());
    m_externals.push_back(std::move(fn));
    m_items.push_back(std::move(item));
}

int UiDrawList::AddPrim(const UiRender::PrimBlock& block)
{
    if (m_local)
    {
        m_localPrims.push_back(block);
        return static_cast<int>(m_localPrims.size()) - 1;
    }
    UiRender::PrimBlock resolved = block;
    if (!Resolve(resolved, 0))
        return -1;
    return UiGpu::Prims().Add(resolved);
}

int UiDrawList::AddAux(const glm::vec4* texels, int count)
{
    if (m_local)
    {
        const int first = static_cast<int>(m_localAux.size());
        m_localAux.insert(m_localAux.end(), texels, texels + count);
        return first;
    }
    return UiGpu::Prims().AddAux(texels, count);
}

void UiDrawList::AppendResolved(const UiDrawList& local, size_t firstItem, size_t lastItem, uint8_t minStencilRef)
{
    if (m_resolvedFrom != &local)
    {
        m_resolvedFrom = &local;
        m_resolvedBase = -2;
        m_resolvedAuxBase = 0;
        if (!local.m_localAux.empty())
        {
            m_resolvedAuxBase = UiGpu::Prims().AddAux(local.m_localAux.data(), static_cast<int>(local.m_localAux.size()));
            if (m_resolvedAuxBase < 0)
            {
                m_resolvedAuxBase = 0;
                m_resolvedBase = -1;
            }
        }
        if (m_resolvedBase != -1)
        {
            for (size_t i = 0; i < local.m_localPrims.size(); ++i)
            {
                UiRender::PrimBlock block = local.m_localPrims[i];
                int index;
                if (!Resolve(block, m_resolvedAuxBase))
                {
                    // A prim whose ramp row is missing is drawn as a transparent box instead of leaving a hole in the numbering.
                    block.T[0].x = static_cast<float>(UiRender::PrimKind::Image);
                    block.T[0].y = 0.f;
                    block.T[0].w = -1.f;
                    block.T[1] = glm::vec4(0.f);
                }
                index = UiGpu::Prims().Add(block);
                if (i == 0)
                    m_resolvedBase = index;
                if (index < 0)
                {
                    m_resolvedBase = -1;
                    break;
                }
            }
            if (local.m_localPrims.empty())
                m_resolvedBase = 0;
        }
    }
    if (m_resolvedBase < 0)
        return;

    if (lastItem > local.m_items.size())
        lastItem = local.m_items.size();
    std::vector<UiRender::Vertex> verts;
    for (size_t i = firstItem; i < lastItem; ++i)
    {
        const Item& item = local.m_items[i];
        if (item.External)
        {
            AddExternal(local.m_externals[item.ExternalIndex]);
            continue;
        }
        verts = item.Verts;
        for (UiRender::Vertex& v : verts)
            v.Prim += static_cast<float>(m_resolvedBase);
        UiRender::BatchKey key = item.Key;
        if (key.StencilRef < minStencilRef)
            key.StencilRef = minStencilRef;
        AddTriangles(key, verts.data(), static_cast<uint32_t>(verts.size()), item.Indices.data(), static_cast<uint32_t>(item.Indices.size()));
    }
}

void UiDrawList::AppendList(const UiDrawList& other)
{
    const uint32_t externalBase = static_cast<uint32_t>(m_externals.size());
    for (Item item : other.m_items)
    {
        if (item.External)
            item.ExternalIndex += externalBase;
        m_items.push_back(std::move(item));
    }
    m_externals.insert(m_externals.end(), other.m_externals.begin(), other.m_externals.end());
}
