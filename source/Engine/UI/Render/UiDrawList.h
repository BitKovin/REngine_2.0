#pragma once
#include <UI/Render/UiRenderTypes.h>
#include <functional>
#include <vector>

class UiDrawList
{
public:
    using ExternalFn = std::function<void(uint16_t viewId)>;

    // Geometry is the usual item. A LayerBegin / LayerComposite pair brackets the items of one offscreen layer: the items
    // between them are rendered into the layer atlas, and the composite item (a quad, patched at submit) draws the result.
    enum class Role : uint8_t { Geometry, LayerBegin, LayerComposite };

    struct Item
    {
        bool External = false;
        Role Kind = Role::Geometry;
        int Layer = -1;                       // LayerBegin / LayerComposite: index into Layers()
        // geometry: each item owns its vertices so later geometry can be merged into an earlier item
        UiRender::BatchKey Key;
        std::vector<UiRender::Vertex> Verts;
        std::vector<uint16_t> Indices;
        glm::vec4 Bounds = glm::vec4(0.f);   // screen-space AABB of the vertices: min x, min y, max x, max y
        // external
        uint32_t ExternalIndex = 0;
    };

    enum class BreakReason { Texture, State, Stencil, External, Capacity, Count };

    void Clear();
    // The next AddTriangles starts a new item, and nothing added later is merged into an item before this point
    // (UiElem legacy-child markers).
    void Barrier() { m_barrier = true; m_fence = m_items.size(); }
    bool Empty() const { return m_items.empty(); }

    // Lists are "global" by default (vertex Prim = index in UiGpu::Prims(), as phase 0 does).
    // A local list stores its own blocks; vertex Prim = index into LocalPrims().
    void SetLocalPrims(bool local) { m_local = local; }
    bool IsLocal() const { return m_local; }
    // Local lists append to m_localPrims and return that index; global lists call UiGpu::Prims().Add (-1 when full).
    int AddPrim(const UiRender::PrimBlock& block);
    // Auxiliary texels (colour matrices, mask and clip-shape geometry). Same local / global split as AddPrim; -1 when full.
    int AddAux(const glm::vec4* texels, int count);
    const std::vector<UiRender::PrimBlock>& LocalPrims() const { return m_localPrims; }
    // Appends items [firstItem, lastItem) of a LOCAL list into this GLOBAL list: adds the local blocks to
    // UiGpu::Prims() once per source (aux indices and ramp ids rebased), rewrites vertex Prim fields, and raises every
    // StencilRef to at least minStencilRef.
    void AppendResolved(const UiDrawList& local, size_t firstItem, size_t lastItem, uint8_t minStencilRef);

    // Appends triangles. `indices` are relative to `verts` (0..vertCount-1).
    // Geometry joins the last item with an equal key; failing that, it joins one of the 7 items before it with an equal key
    // when its bounds overlap none of the items after that one (and none is external), which keeps the painter's order.
    void AddTriangles(const UiRender::BatchKey& key, const UiRender::Vertex* verts, uint32_t vertCount,
                      const uint16_t* indices, uint32_t indexCount);
    // Layers. BeginLayer returns the item index of the LayerBegin marker; nothing recorded after it joins an item before it.
    // EndLayer adds the composite quad (corners as in AddQuad; positions and uv are patched at submit) and returns false
    // when the layer recorded no geometry (the marker stays without a composite and is ignored at submit).
    int BeginLayer(const UiRender::LayerSpec& spec);
    void AbandonLayer(int beginItem);   // the layer cannot be closed (no room for its composite prim): its content stays inline
    bool EndLayer(int beginItem, const UiRender::BatchKey& key, const UiRender::Vertex corners[4]);
    // A backdrop: a marker and a composite quad with no content between them. Its texture and uv are patched at submit.
    void AddBackdrop(const UiRender::LayerSpec& spec, const UiRender::BatchKey& key, const UiRender::Vertex corners[4]);
    // Union of the bounds of the geometry recorded after the marker; x0 > x1 when there is none.
    glm::vec4 LayerContentBounds(int beginItem) const;
    const std::vector<UiRender::LayerSpec>& Layers() const { return m_layers; }
    std::vector<Item>& MutableItems() { return m_items; }
    bool HasLayers() const { return !m_layers.empty(); }
    // Convenience: one quad, corners in order top-left, top-right, bottom-right, bottom-left.
    void AddQuad(const UiRender::BatchKey& key, const UiRender::Vertex corners[4]);
    // Runs `fn` at submit time, in order with the geometry around it.
    void AddExternal(ExternalFn fn);
    // Appends a copy of another list's items (used by UiElem chunks in phase 1).
    void AppendList(const UiDrawList& other);

    const std::vector<Item>& Items() const { return m_items; }
    const std::vector<ExternalFn>& Externals() const { return m_externals; }
    const uint32_t* BreakCounts() const { return m_breaks; }
    uint32_t MergedCount() const { return m_merged; }
    uint32_t TotalVertices() const;
    uint32_t TotalIndices() const;

private:
    std::vector<Item> m_items;
    std::vector<UiRender::LayerSpec> m_layers;
    std::vector<int> m_openLayers;   // layer indices whose composite is not recorded yet (innermost last)
    std::vector<ExternalFn> m_externals;
    uint32_t m_breaks[static_cast<int>(BreakReason::Count)] = {};
    uint32_t m_merged = 0;
    bool m_local = false;
    bool m_barrier = false;
    size_t m_fence = 0;
    std::vector<UiRender::PrimBlock> m_localPrims;
    std::vector<glm::vec4> m_localAux;
    const UiDrawList* m_resolvedFrom = nullptr;
    int m_resolvedBase = -1;
    int m_resolvedAuxBase = 0;
};
