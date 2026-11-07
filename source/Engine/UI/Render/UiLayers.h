#pragma once
#include <UI/Render/UiDrawList.h>
#include <UI/Render/UiLayerAtlas.h>
#include <UI/Render/UiPrimBuffer.h>

#include <functional>
#include <string>
#include <vector>

// Compiles the layers of a draw list (phase 7). Build() runs on the CPU before any geometry is uploaded: it decides which layers
// are rendered, allocates their atlas regions, orders the passes (children first) and patches every composite quad. Run() then
// submits the passes. Items inside an Ok layer are skipped by the main walk; a layer that cannot be built (too large, atlas or
// views exhausted) stays inline, and its content is drawn without the effect.
class UiLayerPlan
{
public:
    enum class State : uint8_t { Failed, Ok, Hidden };

    struct Node
    {
        int Begin = -1, End = -1;           // item indices of the LayerBegin marker and of the composite quad
        int Layer = -1;
        int Parent = -1;
        std::vector<int> Children;
        State S = State::Failed;
        int X0 = 0, Y0 = 0, W = 0, H = 0;   // the region's rectangle in UI units (whole pixels), content inflated by the effects' reach
        uint64_t Hash = 0;
        bool Hit = false;
        bool Prepared = false;
        UiLayerAtlas::Region Final;
    };

    struct Pass
    {
        enum class Type { Content, Down, Blur, Shadow, Copy } T = Type::Content;
        bool Scene = false;                 // Down: the source is the scene colour (or its stand-in image), not an atlas region
        bgfx::TextureHandle SrcTex = BGFX_INVALID_HANDLE;
        glm::vec4 SrcUv = glm::vec4(0.f), SrcClamp = glm::vec4(0.f);
        glm::vec2 SrcStep = glm::vec2(0.f);
        uint16_t View = 0;
        UiLayerAtlas::Region Src, Dst, Over;
        size_t First = 0, Last = 0;         // Content: item range
        int Rect[4] = {};                   // Content: UI rect rendered (x0, y0, x1, y1)
        bool Vertical = false;              // Blur
        float Sigma = 0.f;                  // Blur
        glm::vec2 Offset = glm::vec2(0.f);  // Shadow
        glm::vec4 Color = glm::vec4(0.f);   // Shadow (straight RGBA)
        bool HasOver = false;
    };

    // Returns false when the list has no layer to render. `passView` is the view the list itself is submitted to: layer views run before it.
    bool Build(UiDrawList& list, float width, float height, uint16_t passView);

    // Item walk helpers for the code that draws ranges of the list.
    int NodeAtBegin(size_t item) const { return item < m_atBegin.size() ? m_atBegin[item] : -1; }
    int NodeAtComposite(size_t item) const { return item < m_atComposite.size() ? m_atComposite[item] : -1; }
    const Node& GetNode(int id) const { return m_nodes[id]; }

    // Draws the items [first, last) of the list into `view` with projection `proj`. The plan's caller implements it (it owns the geometry buffers).
    using DrawRange = std::function<void(size_t first, size_t last, uint16_t view, const glm::mat4& proj)>;
    void Run(const DrawRange& draw, const bgfx::VertexLayout& layout);

    uint32_t Rendered() const { return m_rendered; }
    uint32_t CachedHits() const { return m_hits; }
    uint32_t ViewsUsed() const { return static_cast<uint32_t>(m_views.size()); }

private:
    UiDrawList* m_list = nullptr;
    std::vector<Node> m_nodes;
    std::vector<int> m_atBegin, m_atComposite;
    std::vector<Pass> m_passes;
    std::vector<uint16_t> m_views;
    int m_chainState = 0;                          // the blurred scene levels: 0 not planned, 1 ready, 2 failed
    UiLayerAtlas::Region m_chain[3];               // sigma 4, 8, 16 UI units
    uint32_t m_chainPages = 0;
    std::vector<UiLayerAtlas::Region> m_owned;      // scratch regions of the node being planned (rolled back on failure)
    float m_width = 0.f, m_height = 0.f;
    bool m_anyCache = false;
    uint32_t m_rendered = 0, m_hits = 0;

    void Prepare(int id);
    void Plan(int id);
    void PlanChildren(int id);
    bool PlanRender(int id);
    void CollectFinals(int id, std::vector<int>& out) const;
    void Patch(int id);
    void PlanBackdrop(int id);
    bool EnsureChain();
    void PatchBackdrop(int id);
    uint64_t HashNode(int id) const;
    uint64_t HashPrim(uint64_t h, int prim) const;

    bool AllocScratch(int w, int h, uint32_t excluded, UiLayerAtlas::Region& out);
    void FreeScratch(const UiLayerAtlas::Region& r);
    uint16_t NewView(bool& ok);
    UiLayerAtlas::Region PlanBlur(const UiLayerAtlas::Region& src, float sigma, bool& ok);
};

namespace UiLayers
{
    // Frame bookkeeping (UiGpu calls these).
    void BeginSubmit();           // first thing in every UiGpu::Submit: undoes last frame's view order if nothing used it again
    void OnBeforeBgfxFrame();
    uint32_t TotalRendered();     // layer content renders since start (tests compare deltas)
    uint32_t TotalCached();
    void Shutdown();

    // The scene colour backdrop-filter blurs. EngineMain sets it around the UI pass; passes that have no scene (loading screen,
    // world-space UI) leave it empty and backdrops are not drawn there.
    void SetScene(bgfx::TextureHandle texture, int width, int height);
    void ClearScene();
    // Tests: an image replaces the scene ("" restores it).
    void SetSceneOverride(const std::string& path);
}
