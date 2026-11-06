#pragma once
#include <UI/Render/UiRenderTypes.h>
#include <functional>
#include <vector>

class UiDrawList
{
public:
    using ExternalFn = std::function<void(uint16_t viewId)>;

    struct Item
    {
        bool External = false;
        // geometry
        UiRender::BatchKey Key;
        uint32_t FirstVertex = 0, VertexCount = 0;
        uint32_t FirstIndex = 0, IndexCount = 0;
        // external
        uint32_t ExternalIndex = 0;
    };

    enum class BreakReason { Texture, State, Stencil, External, Capacity, Count };

    void Clear();
    // The next AddTriangles starts a new item, so later geometry can never merge in front of whatever the caller
    // inserts at this position (UiElem legacy-child markers).
    void Barrier() { m_barrier = true; }
    bool Empty() const { return m_items.empty(); }

    // Lists are "global" by default (vertex Prim = index in UiGpu::Prims(), as phase 0 does).
    // A local list stores its own blocks; vertex Prim = index into LocalPrims().
    void SetLocalPrims(bool local) { m_local = local; }
    bool IsLocal() const { return m_local; }
    // Local lists append to m_localPrims and return that index; global lists call UiGpu::Prims().Add (-1 when full).
    int AddPrim(const UiRender::PrimBlock& block);
    const std::vector<UiRender::PrimBlock>& LocalPrims() const { return m_localPrims; }
    // Appends items [firstItem, lastItem) of a LOCAL list into this GLOBAL list: adds the local blocks to
    // UiGpu::Prims() once per source, rewrites vertex Prim fields, and raises every StencilRef to at least minStencilRef.
    void AppendResolved(const UiDrawList& local, size_t firstItem, size_t lastItem, uint8_t minStencilRef);

    // Appends triangles. `indices` are relative to `verts` (0..vertCount-1).
    void AddTriangles(const UiRender::BatchKey& key, const UiRender::Vertex* verts, uint32_t vertCount,
                      const uint16_t* indices, uint32_t indexCount);
    // Convenience: one quad, corners in order top-left, top-right, bottom-right, bottom-left.
    void AddQuad(const UiRender::BatchKey& key, const UiRender::Vertex corners[4]);
    // Runs `fn` at submit time, in order with the geometry around it.
    void AddExternal(ExternalFn fn);
    // Appends a copy of another list's items (used by UiElem chunks in phase 1).
    void AppendList(const UiDrawList& other);

    const std::vector<Item>& Items() const { return m_items; }
    const std::vector<UiRender::Vertex>& Vertices() const { return m_vertices; }
    const std::vector<uint16_t>& Indices() const { return m_indices; }
    const std::vector<ExternalFn>& Externals() const { return m_externals; }
    const uint32_t* BreakCounts() const { return m_breaks; }

private:
    std::vector<Item> m_items;
    std::vector<UiRender::Vertex> m_vertices;
    std::vector<uint16_t> m_indices;
    std::vector<ExternalFn> m_externals;
    uint32_t m_breaks[static_cast<int>(BreakReason::Count)] = {};
    bool m_local = false;
    bool m_barrier = false;
    std::vector<UiRender::PrimBlock> m_localPrims;
    const UiDrawList* m_resolvedFrom = nullptr;
    int m_resolvedBase = -1;
};
