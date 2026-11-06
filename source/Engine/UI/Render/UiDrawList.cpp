#include "UiDrawList.h"

#include <UI/Render/UiGpu.h>

#include <utility>

void UiDrawList::Clear()
{
    m_items.clear();
    m_vertices.clear();
    m_indices.clear();
    m_externals.clear();
    m_localPrims.clear();
    m_barrier = false;
    m_resolvedFrom = nullptr;
    m_resolvedBase = -1;
    for (uint32_t& count : m_breaks)
        count = 0;
}

void UiDrawList::AddTriangles(const UiRender::BatchKey& key, const UiRender::Vertex* verts, uint32_t vertCount,
                              const uint16_t* indices, uint32_t indexCount)
{
    if (vertCount == 0 || indexCount == 0)
        return;

    if (m_barrier)
    {
        m_barrier = false;
    }
    else if (!m_items.empty())
    {
        Item& last = m_items.back();
        if (!last.External && last.Key == key && last.VertexCount + vertCount <= 65535)
        {
            const uint16_t base = static_cast<uint16_t>(last.VertexCount);
            for (uint32_t i = 0; i < indexCount; ++i)
                m_indices.push_back(static_cast<uint16_t>(indices[i] + base));
            m_vertices.insert(m_vertices.end(), verts, verts + vertCount);
            last.VertexCount += vertCount;
            last.IndexCount += indexCount;
            return;
        }

        BreakReason reason;
        if (last.External)
            reason = BreakReason::External;
        else if (last.Key == key)
            reason = BreakReason::Capacity;
        else if (last.Key.TextureIdx != key.TextureIdx)
            reason = BreakReason::Texture;
        else if (last.Key.StencilRef != key.StencilRef)
            reason = BreakReason::Stencil;
        else
            reason = BreakReason::State;
        ++m_breaks[static_cast<int>(reason)];
    }

    Item item;
    item.Key = key;
    item.FirstVertex = static_cast<uint32_t>(m_vertices.size());
    item.FirstIndex = static_cast<uint32_t>(m_indices.size());
    item.VertexCount = vertCount;
    item.IndexCount = indexCount;
    m_indices.insert(m_indices.end(), indices, indices + indexCount);
    m_vertices.insert(m_vertices.end(), verts, verts + vertCount);
    m_items.push_back(item);
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
    m_items.push_back(item);
}

int UiDrawList::AddPrim(const UiRender::PrimBlock& block)
{
    if (m_local)
    {
        m_localPrims.push_back(block);
        return static_cast<int>(m_localPrims.size()) - 1;
    }
    return UiGpu::Prims().Add(block);
}

void UiDrawList::AppendResolved(const UiDrawList& local, size_t firstItem, size_t lastItem, uint8_t minStencilRef)
{
    if (m_resolvedFrom != &local)
    {
        m_resolvedFrom = &local;
        m_resolvedBase = -2;
        for (size_t i = 0; i < local.m_localPrims.size(); ++i)
        {
            const int index = UiGpu::Prims().Add(local.m_localPrims[i]);
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
        verts.assign(local.m_vertices.begin() + item.FirstVertex, local.m_vertices.begin() + item.FirstVertex + item.VertexCount);
        for (UiRender::Vertex& v : verts)
            v.Prim += static_cast<float>(m_resolvedBase);
        UiRender::BatchKey key = item.Key;
        if (key.StencilRef < minStencilRef)
            key.StencilRef = minStencilRef;
        AddTriangles(key, verts.data(), item.VertexCount, local.m_indices.data() + item.FirstIndex, item.IndexCount);
    }
}

void UiDrawList::AppendList(const UiDrawList& other)
{
    const uint32_t vertexBase = static_cast<uint32_t>(m_vertices.size());
    const uint32_t indexBase = static_cast<uint32_t>(m_indices.size());
    const uint32_t externalBase = static_cast<uint32_t>(m_externals.size());

    for (Item item : other.m_items)
    {
        if (item.External)
            item.ExternalIndex += externalBase;
        else
        {
            item.FirstVertex += vertexBase;
            item.FirstIndex += indexBase;
        }
        m_items.push_back(item);
    }
    m_vertices.insert(m_vertices.end(), other.m_vertices.begin(), other.m_vertices.end());
    m_indices.insert(m_indices.end(), other.m_indices.begin(), other.m_indices.end());
    m_externals.insert(m_externals.end(), other.m_externals.begin(), other.m_externals.end());
}
