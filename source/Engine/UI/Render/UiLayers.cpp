#include "UiLayers.h"

#include <Logger.hpp>
#include <Renderer/Abstractions/ViewIdManager.h>
#include <Shader.hpp>
#include <ShaderManager.h>
#include <UI/Render/UiGpu.h>
#include <UI/Render/UiImageAtlas.h>

#include <algorithm>
#include <cmath>
#include <cstring>

namespace
{
    constexpr int kMaxLayerViewsPerFrame = 96;

    int s_viewsThisFrame = 0;
    bool s_orderModified = false;
    bool s_orderResetPending = false;
    bool s_loggedTooLarge = false;
    bool s_loggedNoRoom = false;
    uint32_t s_totalRendered = 0, s_totalCached = 0;
    bgfx::ViewId s_order[256];

    struct SceneSource
    {
        bool Valid = false;
        bgfx::TextureHandle Tex = BGFX_INVALID_HANDLE;
        glm::vec4 Uv = glm::vec4(0.f, 0.f, 1.f, 1.f);
        glm::vec2 Size = glm::vec2(1.f);
    };
    SceneSource s_scene;
    std::string s_sceneOverride;
    bool s_loggedNoScene = false;

    // The source the chain is built from: the override image of a test, else the scene colour of this UI pass.
    bool ResolveScene(SceneSource& out)
    {
        if (!s_sceneOverride.empty())
        {
            const UiAtlasImage image = UiImageAtlas::Get(s_sceneOverride);
            if (!image.Valid)
                return false;
            out.Valid = true;
            out.Tex = image.Texture;
            out.Uv = image.Uv;
            out.Size = glm::vec2(static_cast<float>(std::max(image.TextureWidth, 1)), static_cast<float>(std::max(image.TextureHeight, 1)));
            return true;
        }
        if (!s_scene.Valid)
            return false;
        out = s_scene;
        const bgfx::Caps* caps = bgfx::getCaps();
        if (caps != nullptr && caps->originBottomLeft)
            std::swap(out.Uv.y, out.Uv.w);   // a render target is stored bottom-up there
        return true;
    }

    Shader* s_blur = nullptr;
    Shader* s_down = nullptr;
    Shader* s_shadow = nullptr;
    bool s_shadersTried = false;

    uint64_t Fnv(uint64_t h, const void* data, size_t size)
    {
        const unsigned char* bytes = static_cast<const unsigned char*>(data);
        for (size_t i = 0; i < size; ++i)
        {
            h ^= bytes[i];
            h *= 1099511628211ull;
        }
        return h;
    }

    template <class T>
    uint64_t Mix(uint64_t h, const T& value) { return Fnv(h, &value, sizeof(T)); }

    // Moves `views` (in order) to just before `before` in bgfx's view order.
    void PlaceBefore(const std::vector<uint16_t>& views, uint16_t before)
    {
        if (views.empty())
            return;
        if (s_orderResetPending || !s_orderModified)
        {
            for (int i = 0; i < 256; ++i)
                s_order[i] = static_cast<bgfx::ViewId>(i);
            s_orderResetPending = false;
        }
        std::vector<bgfx::ViewId> order;
        order.reserve(256);
        for (int i = 0; i < 256; ++i)
        {
            const bgfx::ViewId id = s_order[i];
            if (std::find(views.begin(), views.end(), id) != views.end())
                continue;
            if (id == before)
                order.insert(order.end(), views.begin(), views.end());
            order.push_back(id);
        }
        for (int i = 0; i < 256 && i < static_cast<int>(order.size()); ++i)
            s_order[i] = order[i];
        bgfx::setViewOrder(0, 256, s_order);
        s_orderModified = true;
    }

    void LoadShaders()
    {
        if (s_shadersTried)
            return;
        s_shadersTried = true;
        s_blur = ShaderManager::GetShaderProgram("ui2/vs_ui2_fx", "ui2/fs_ui2_blur");
        s_down = ShaderManager::GetShaderProgram("ui2/vs_ui2_fx", "ui2/fs_ui2_down");
        s_shadow = ShaderManager::GetShaderProgram("ui2/vs_ui2_fx", "ui2/fs_ui2_dropshadow");
        for (Shader* shader : { s_blur, s_down, s_shadow })
            if (shader == nullptr || !shader->IsValid())
                Logger::Error("[UiRenderer] a ui2 layer effect shader failed to load; layer effects are skipped");
    }

    // 9-tap Gaussian as one centre tap and four linear-sampled pairs: weights and texel offsets of the pairs.
    void BlurKernel(float sigma, float weights[5], float offsets[4])
    {
        float g[9];
        float sum = 0.f;
        const float s2 = std::max(sigma * sigma, 0.0001f);
        for (int i = 0; i < 9; ++i)
        {
            g[i] = std::exp(-0.5f * static_cast<float>(i * i) / s2);
            sum += (i == 0 ? 1.f : 2.f) * g[i];
        }
        for (float& v : g)
            v /= sum;
        weights[0] = g[0];
        for (int p = 0; p < 4; ++p)
        {
            const int a = 1 + p * 2, b = a + 1;
            const float w = g[a] + g[b];
            weights[1 + p] = w;
            offsets[p] = w > 1e-9f ? (static_cast<float>(a) * g[a] + static_cast<float>(b) * g[b]) / w : static_cast<float>(a);
        }
    }
}

namespace UiLayers
{
    void BeginSubmit()
    {
        if (s_orderResetPending)
        {
            bgfx::setViewOrder(0, 256, nullptr);   // last frame moved views; this frame has not (yet)
            s_orderModified = false;
            s_orderResetPending = false;
        }
    }

    void OnBeforeBgfxFrame()
    {
        s_orderResetPending = s_orderModified;
        s_viewsThisFrame = 0;
        s_loggedTooLarge = false;
        s_loggedNoRoom = false;
    }

    void SetScene(bgfx::TextureHandle texture, int width, int height)
    {
        s_scene.Valid = bgfx::isValid(texture) && width > 0 && height > 0;
        s_scene.Tex = texture;
        s_scene.Uv = glm::vec4(0.f, 0.f, 1.f, 1.f);
        s_scene.Size = glm::vec2(static_cast<float>(std::max(width, 1)), static_cast<float>(std::max(height, 1)));
    }

    void ClearScene() { s_scene.Valid = false; }

    void SetSceneOverride(const std::string& path) { s_sceneOverride = path; }

    uint32_t TotalRendered() { return s_totalRendered; }
    uint32_t TotalCached() { return s_totalCached; }

    void Shutdown()
    {
        s_blur = s_down = s_shadow = nullptr;
        s_shadersTried = false;
        UiLayerAtlas::Shutdown();
    }
}

// ── building ──────────────────────────────────────────────────────────────

bool UiLayerPlan::Build(UiDrawList& list, float width, float height, uint16_t passView)
{
    m_list = &list;
    m_nodes.clear();
    m_passes.clear();
    m_views.clear();
    m_owned.clear();
    m_rendered = m_hits = 0;
    m_width = width;
    m_height = height;
    m_anyCache = false;
    m_chainState = 0;
    m_chainPages = 0;
    for (UiLayerAtlas::Region& r : m_chain)
        r = UiLayerAtlas::Region();

    auto& items = list.MutableItems();
    m_atBegin.assign(items.size(), -1);
    m_atComposite.assign(items.size(), -1);

    std::vector<int> stack;
    for (size_t i = 0; i < items.size(); ++i)
    {
        if (items[i].Kind == UiDrawList::Role::LayerBegin)
        {
            Node node;
            node.Begin = static_cast<int>(i);
            node.Layer = items[i].Layer;
            node.Parent = stack.empty() ? -1 : stack.back();
            const int id = static_cast<int>(m_nodes.size());
            m_atBegin[i] = id;
            m_nodes.push_back(std::move(node));
            stack.push_back(id);
        }
        else if (items[i].Kind == UiDrawList::Role::LayerComposite && !stack.empty())
        {
            const int id = stack.back();
            stack.pop_back();
            m_nodes[id].End = static_cast<int>(i);
            m_atComposite[i] = id;
        }
    }
    // A begin without its end (the painter could not add the composite) is not a layer.
    for (int id : stack)
        m_atBegin[m_nodes[id].Begin] = -1;
    for (int id = 0; id < static_cast<int>(m_nodes.size()); ++id)
    {
        Node& node = m_nodes[id];
        if (node.End < 0)
            continue;
        while (node.Parent >= 0 && m_nodes[node.Parent].End < 0)
            node.Parent = m_nodes[node.Parent].Parent;
        if (node.Parent >= 0)
            m_nodes[node.Parent].Children.push_back(id);
        if (list.Layers()[node.Layer].Cache)
            m_anyCache = true;
    }

    bool any = false;
    for (int id = 0; id < static_cast<int>(m_nodes.size()); ++id)
    {
        if (m_nodes[id].End < 0 || m_nodes[id].Parent >= 0)
            continue;
        Prepare(id);
        Plan(id);
        any = true;
    }
    if (!m_views.empty())
        PlaceBefore(m_views, passView);
    return any;
}

void UiLayerPlan::Prepare(int id)
{
    Node& n = m_nodes[id];
    for (int c : n.Children)
        if (m_nodes[c].End >= 0)
            Prepare(c);
    n.Prepared = true;

    const UiRender::LayerSpec& spec = m_list->Layers()[n.Layer];
    const float reach = spec.Reach();
    const float lo = -reach, hiX = m_width + reach, hiY = m_height + reach;
    const int x0 = static_cast<int>(std::floor(std::max(spec.Content.x - reach, lo)));
    const int y0 = static_cast<int>(std::floor(std::max(spec.Content.y - reach, lo)));
    const int x1 = static_cast<int>(std::ceil(std::min(spec.Content.z + reach, hiX)));
    const int y1 = static_cast<int>(std::ceil(std::min(spec.Content.w + reach, hiY)));
    if (x1 <= x0 || y1 <= y0)
    {
        n.S = State::Hidden;   // entirely off screen
        return;
    }
    n.X0 = x0;
    n.Y0 = y0;
    n.W = x1 - x0;
    n.H = y1 - y0;
    if (n.W > UiLayerAtlas::kPageSize || n.H > UiLayerAtlas::kPageSize)
    {
        if (!s_loggedTooLarge)
            Logger::Warning("[UiRenderer] a UI layer needs %d x %d pixels, more than an atlas page; it is drawn without its effect", n.W, n.H);
        s_loggedTooLarge = true;
        n.S = State::Failed;
        return;
    }
    n.S = State::Failed;   // Plan() makes it Ok
    if (m_anyCache)
        n.Hash = HashNode(id);
}

uint64_t UiLayerPlan::HashPrim(uint64_t h, int prim) const
{
    const UiPrimBuffer& prims = UiGpu::Prims();
    const glm::vec4* t = prims.PrimTexels(prim);
    if (t == nullptr)
        return h;
    h = Fnv(h, t, sizeof(glm::vec4) * UiRender::kPrimTexels);
    const int flags = static_cast<int>(t[0].y);
    if (t[0].w >= 0.f)
        if (const glm::vec4* a = prims.AuxTexels(static_cast<int>(t[0].w + 0.5f)))
            h = Fnv(h, a, sizeof(glm::vec4) * 5);
    if ((flags & UiRender::kFlagMask) != 0)
        if (const glm::vec4* a = prims.AuxTexels(static_cast<int>(t[15].z + 0.5f)))
            h = Fnv(h, a, sizeof(glm::vec4));
    if ((flags & UiRender::kFlagClipShape) != 0)
        if (const glm::vec4* a = prims.AuxTexels(static_cast<int>(t[15].w + 0.5f)))
            h = Fnv(h, a, sizeof(glm::vec4) * 2);
    return h;
}

// The hash covers everything that decides the layer's pixels: effects, geometry, batch keys, primitive data (and the auxiliary
// texels it points at), and the hashes of the layers inside it. Texture contents are not covered: cache: layer is for static content.
uint64_t UiLayerPlan::HashNode(int id) const
{
    const Node& n = m_nodes[id];
    const UiRender::LayerSpec& spec = m_list->Layers()[n.Layer];
    uint64_t h = 1469598103934665603ull;
    h = Mix(h, n.X0); h = Mix(h, n.Y0); h = Mix(h, n.W); h = Mix(h, n.H);
    for (const UiRender::LayerEffect& e : spec.Effects)
    {
        h = Mix(h, static_cast<int>(e.K)); h = Mix(h, e.Sigma); h = Mix(h, e.Offset); h = Mix(h, e.Color);
    }
    const auto& items = m_list->Items();
    for (int i = n.Begin + 1; i < n.End; ++i)
    {
        const UiDrawList::Item& item = items[i];
        if (item.Kind == UiDrawList::Role::LayerBegin)
        {
            const int child = m_atBegin[i];
            if (child >= 0 && m_nodes[child].End >= 0)
            {
                i = m_nodes[child].End - 1;   // the composite item is hashed next, with the child's hash
                continue;
            }
            continue;
        }
        if (item.Kind == UiDrawList::Role::LayerComposite)
        {
            const int child = m_atComposite[i];
            if (child >= 0)
            {
                const Node& c = m_nodes[child];
                h = Mix(h, c.Hash); h = Mix(h, c.X0); h = Mix(h, c.Y0); h = Mix(h, c.W); h = Mix(h, c.H);
                h = Mix(h, item.Key.State);
                if (!item.Verts.empty())
                    h = HashPrim(h, static_cast<int>(item.Verts[0].Prim + 0.5f));
            }
            continue;
        }
        if (item.External)
        {
            h = Mix(h, 0x45787465);
            continue;
        }
        h = Mix(h, item.Key.Program); h = Mix(h, item.Key.State); h = Mix(h, item.Key.TextureIdx);
        int lastPrim = -1;
        for (const UiRender::Vertex& v : item.Verts)
        {
            const float geo[6] = { v.X, v.Y, v.U, v.V, v.LX, v.LY };
            h = Fnv(h, geo, sizeof(geo));
            const int prim = static_cast<int>(v.Prim + 0.5f);
            if (prim != lastPrim)
            {
                h = HashPrim(h, prim);
                lastPrim = prim;
            }
        }
        h = Fnv(h, item.Indices.data(), item.Indices.size() * sizeof(uint16_t));
    }
    return h;
}

void UiLayerPlan::CollectFinals(int id, std::vector<int>& out) const
{
    for (int c : m_nodes[id].Children)
    {
        if (m_nodes[c].End < 0 || m_list->Layers()[m_nodes[c].Layer].Backdrop)
            continue;   // a backdrop shows a shared region of the scene chain: nothing to free or exclude per layer
        if (m_nodes[c].S == State::Ok)
            out.push_back(c);
        else if (m_nodes[c].S == State::Failed)
            CollectFinals(c, out);   // a layer that stayed inline draws its own layers into this one
    }
}

void UiLayerPlan::PlanChildren(int id)
{
    for (int c : m_nodes[id].Children)
        if (m_nodes[c].End >= 0)
            Plan(c);
}

void UiLayerPlan::Plan(int id)
{
    Node& n = m_nodes[id];
    if (n.S == State::Hidden)
        return;
    const UiRender::LayerSpec& spec = m_list->Layers()[n.Layer];
    if (spec.Backdrop)
    {
        PlanBackdrop(id);
        return;
    }
    if (spec.Cache && n.W > 0)
    {
        UiLayerAtlas::CacheEntry* entry = UiLayerAtlas::FindCached(spec.Id);
        if (entry != nullptr && entry->Hash == n.Hash)   // the hash covers the layer's size
        {
            n.Final = entry->R;
            n.S = State::Ok;
            n.Hit = true;
            ++m_hits;
            ++s_totalCached;
            Patch(id);
            return;
        }
    }
    PlanChildren(id);
    if (n.W <= 0 || n.W > UiLayerAtlas::kPageSize || n.H > UiLayerAtlas::kPageSize)
        return;   // too large: stays inline
    LoadShaders();
    if (PlanRender(id))
    {
        n.S = State::Ok;
        ++m_rendered;
        ++s_totalRendered;
        Patch(id);
    }
}

bool UiLayerPlan::AllocScratch(int w, int h, uint32_t excluded, UiLayerAtlas::Region& out)
{
    if (!UiLayerAtlas::AllocScratch(w, h, excluded | m_chainPages, out))   // a pass never renders into a page that holds the blurred scene
        return false;
    m_owned.push_back(out);
    return true;
}

void UiLayerPlan::FreeScratch(const UiLayerAtlas::Region& r)
{
    if (!r.Valid() || r.Page == UiLayerAtlas::kCachePage)
        return;
    UiLayerAtlas::FreeScratch(r);
    for (size_t i = 0; i < m_owned.size(); ++i)
        if (m_owned[i].Page == r.Page && m_owned[i].Rect.X == r.Rect.X && m_owned[i].Rect.Y == r.Rect.Y)
        {
            m_owned.erase(m_owned.begin() + i);
            break;
        }
}

uint16_t UiLayerPlan::NewView(bool& ok)
{
    if (!ok)
        return 0;
    if (s_viewsThisFrame >= kMaxLayerViewsPerFrame || ViewIdManager::Remaining() < 12)
    {
        if (!s_loggedNoRoom)
            Logger::Warning("[UiRenderer] no free bgfx views left for UI layers; the remaining layers are drawn without their effect");
        s_loggedNoRoom = true;
        ok = false;
        return 0;
    }
    ++s_viewsThisFrame;
    const uint16_t view = ViewIdManager::Reserve();
    m_views.push_back(view);
    return view;
}

UiLayerAtlas::Region UiLayerPlan::PlanBlur(const UiLayerAtlas::Region& src, float sigma, bool& ok)
{
    using Region = UiLayerAtlas::Region;
    Region s = src;
    bool intermediate = false;
    int w = src.Rect.W, h = src.Rect.H;
    float sg = sigma;
    while (ok && sg > 4.f && w >= 4 && h >= 4)
    {
        w = (w + 1) / 2;
        h = (h + 1) / 2;
        sg *= 0.5f;
        Region d;
        if (!AllocScratch(w, h, UiLayerAtlas::PageBit(s.Page), d))
        {
            ok = false;
            break;
        }
        Pass p;
        p.T = Pass::Type::Down;
        p.View = NewView(ok);
        p.Src = s;
        p.Dst = d;
        if (ok)
            m_passes.push_back(p);
        if (intermediate)
            FreeScratch(s);
        s = d;
        intermediate = true;
    }
    Region t, o;
    if (ok && !AllocScratch(w, h, UiLayerAtlas::PageBit(s.Page), t))
        ok = false;
    if (ok)
    {
        Pass p;
        p.T = Pass::Type::Blur;
        p.View = NewView(ok);
        p.Src = s;
        p.Dst = t;
        p.Sigma = sg;
        p.Vertical = false;
        if (ok)
            m_passes.push_back(p);
    }
    if (intermediate)
        FreeScratch(s);
    if (ok && !AllocScratch(w, h, UiLayerAtlas::PageBit(t.Page), o))
        ok = false;
    if (ok)
    {
        Pass p;
        p.T = Pass::Type::Blur;
        p.View = NewView(ok);
        p.Src = t;
        p.Dst = o;
        p.Sigma = sg;
        p.Vertical = true;
        if (ok)
            m_passes.push_back(p);
    }
    FreeScratch(t);
    return o;
}

bool UiLayerPlan::PlanRender(int id)
{
    using Region = UiLayerAtlas::Region;
    Node& n = m_nodes[id];
    const UiRender::LayerSpec& spec = m_list->Layers()[n.Layer];
    const size_t passStart = m_passes.size();
    m_owned.clear();
    bool ok = true;

    std::vector<int> kids;
    CollectFinals(id, kids);
    uint32_t excluded = 0;
    for (int k : kids)
        excluded |= UiLayerAtlas::PageBit(m_nodes[k].Final.Page);

    Region cur;
    ok = AllocScratch(n.W, n.H, excluded, cur);
    if (ok)
    {
        Pass p;
        p.T = Pass::Type::Content;
        p.View = NewView(ok);
        p.Dst = cur;
        p.First = static_cast<size_t>(n.Begin) + 1;
        p.Last = static_cast<size_t>(n.End);
        p.Rect[0] = n.X0; p.Rect[1] = n.Y0; p.Rect[2] = n.X0 + n.W; p.Rect[3] = n.Y0 + n.H;
        if (ok)
            m_passes.push_back(p);
    }

    for (const UiRender::LayerEffect& fx : spec.Effects)
    {
        if (!ok)
            break;
        if (fx.K == UiRender::LayerEffect::Kind::Blur)
        {
            if (fx.Sigma < 0.05f)
                continue;
            Region out = PlanBlur(cur, fx.Sigma, ok);
            if (ok)
            {
                FreeScratch(cur);
                cur = out;
            }
        }
        else
        {
            Region shadowSrc = cur;
            bool blurred = false;
            if (fx.Sigma >= 0.05f)
            {
                shadowSrc = PlanBlur(cur, fx.Sigma, ok);
                blurred = true;
            }
            Region dst;
            if (ok && !AllocScratch(n.W, n.H, UiLayerAtlas::PageBit(cur.Page) | UiLayerAtlas::PageBit(shadowSrc.Page), dst))
                ok = false;
            if (ok)
            {
                Pass p;
                p.T = Pass::Type::Shadow;
                p.View = NewView(ok);
                p.Src = shadowSrc;
                p.Over = cur;
                p.HasOver = true;
                p.Dst = dst;
                p.Offset = fx.Offset;
                p.Color = fx.Color;
                if (ok)
                    m_passes.push_back(p);
            }
            if (ok)
            {
                if (blurred)
                    FreeScratch(shadowSrc);
                FreeScratch(cur);
                cur = dst;
            }
        }
    }

    if (ok && spec.Cache)
    {
        Region cached;
        if (!UiLayerAtlas::AllocCached(spec.Id, cur.Rect.W, cur.Rect.H, cached))
            ok = false;
        else
        {
            Pass p;
            p.T = Pass::Type::Copy;
            p.View = NewView(ok);
            p.Src = cur;
            p.Dst = cached;
            if (ok)
            {
                m_passes.push_back(p);
                FreeScratch(cur);
                cur = cached;
                if (UiLayerAtlas::CacheEntry* entry = UiLayerAtlas::FindCached(spec.Id))
                    entry->Hash = n.Hash;
            }
        }
    }

    if (!ok)
    {
        m_passes.resize(passStart);
        for (const Region& r : std::vector<Region>(m_owned))
            FreeScratch(r);
        m_owned.clear();
        return false;
    }
    // The layers inside have been drawn into this one: their regions are free for the passes that come later.
    for (int k : kids)
        FreeScratch(m_nodes[k].Final);
    n.Final = cur;
    m_owned.clear();
    return true;
}

// The blurred scene, planned once per frame when a backdrop needs it: the scene is halved three times and every level is blurred
// with sigma 2 texels, which is 4, 8 and 16 UI units. The three results stay allocated for the whole pass.
bool UiLayerPlan::EnsureChain()
{
    if (m_chainState != 0)
        return m_chainState == 1;
    m_chainState = 2;
    SceneSource scene;
    if (!ResolveScene(scene))
    {
        if (s_sceneOverride.empty() && !s_loggedNoScene)
            Logger::Warning("[UiRenderer] backdrop-filter needs the scene colour behind the UI; this pass has none, so backdrops are not drawn here");
        s_loggedNoScene = s_sceneOverride.empty();
        return false;
    }
    LoadShaders();
    if (s_down == nullptr || !s_down->IsValid() || s_blur == nullptr || !s_blur->IsValid())
        return false;

    using Region = UiLayerAtlas::Region;
    const size_t passStart = m_passes.size();
    m_owned.clear();
    bool ok = true;
    Region down[3], blurred[3];
    int w = static_cast<int>(m_width), h = static_cast<int>(m_height);
    for (int k = 0; k < 3 && ok; ++k)
    {
        w = std::max((w + 1) / 2, 1);
        h = std::max((h + 1) / 2, 1);
        if (!AllocScratch(w, h, k == 0 ? 0u : UiLayerAtlas::PageBit(down[k - 1].Page), down[k]))
        {
            ok = false;
            break;
        }
        Pass p;
        p.T = Pass::Type::Down;
        p.View = NewView(ok);
        p.Dst = down[k];
        if (k == 0)
        {
            const float halfU = 0.5f / scene.Size.x, halfV = 0.5f / scene.Size.y;
            p.Scene = true;
            p.SrcTex = scene.Tex;
            p.SrcUv = scene.Uv;
            p.SrcStep = glm::vec2(1.f / scene.Size.x, 1.f / scene.Size.y);
            p.SrcClamp = glm::vec4(std::min(scene.Uv.x, scene.Uv.z) + halfU, std::min(scene.Uv.y, scene.Uv.w) + halfV,
                                   std::max(scene.Uv.x, scene.Uv.z) - halfU, std::max(scene.Uv.y, scene.Uv.w) - halfV);
        }
        else
            p.Src = down[k - 1];
        if (ok)
            m_passes.push_back(p);
    }
    for (int k = 0; k < 3 && ok; ++k)
        blurred[k] = PlanBlur(down[k], 2.f, ok);
    for (int k = 0; k < 3; ++k)
        FreeScratch(down[k]);
    if (!ok)
    {
        m_passes.resize(passStart);
        for (const Region& r : std::vector<Region>(m_owned))
            FreeScratch(r);
        m_owned.clear();
        return false;
    }
    m_owned.clear();
    for (int k = 0; k < 3; ++k)
    {
        m_chain[k] = blurred[k];
        m_chainPages |= UiLayerAtlas::PageBit(blurred[k].Page);
    }
    m_chainState = 1;
    return true;
}

void UiLayerPlan::PlanBackdrop(int id)
{
    Node& n = m_nodes[id];
    if (n.S == State::Hidden)
        return;
    const float sigma = m_list->Layers()[n.Layer].BackdropSigma;
    const int level = sigma < 6.f ? 0 : (sigma < 12.f ? 1 : 2);   // the level whose sigma (4, 8, 16) is closest
    if (!EnsureChain())
        return;   // stays Failed: nothing is drawn
    n.Final = m_chain[level];
    n.S = State::Ok;
    PatchBackdrop(id);
}

// The quad keeps its corners; its uv follows the screen position over the (smaller) blurred scene.
void UiLayerPlan::PatchBackdrop(int id)
{
    Node& n = m_nodes[id];
    UiDrawList::Item& item = m_list->MutableItems()[n.End];
    const glm::vec4 uv = UiLayerAtlas::UvRect(n.Final);
    const glm::vec4 clamp = UiLayerAtlas::ClampRect(n.Final);
    for (UiRender::Vertex& v : item.Verts)
    {
        const float fx = std::clamp(v.X / std::max(m_width, 1.f), 0.f, 1.f);
        const float fy = std::clamp(v.Y / std::max(m_height, 1.f), 0.f, 1.f);
        v.U = std::clamp(uv.x + (uv.z - uv.x) * fx, clamp.x, clamp.z);
        v.V = std::clamp(uv.y + (uv.w - uv.y) * fy, clamp.y, clamp.w);
    }
    item.Key.TextureIdx = UiLayerAtlas::Texture(n.Final.Page).idx;
}

void UiLayerPlan::Patch(int id)
{
    Node& n = m_nodes[id];
    UiDrawList::Item& item = m_list->MutableItems()[n.End];
    const glm::vec4 uv = UiLayerAtlas::UvRect(n.Final);
    const float x0 = static_cast<float>(n.X0), y0 = static_cast<float>(n.Y0);
    const float x1 = static_cast<float>(n.X0 + n.W), y1 = static_cast<float>(n.Y0 + n.H);
    const float xs[4] = { x0, x1, x1, x0 };
    const float ys[4] = { y0, y0, y1, y1 };
    const float us[4] = { uv.x, uv.z, uv.z, uv.x };
    const float vs[4] = { uv.y, uv.y, uv.w, uv.w };
    for (int i = 0; i < 4 && i < static_cast<int>(item.Verts.size()); ++i)
    {
        item.Verts[i].X = xs[i];
        item.Verts[i].Y = ys[i];
        item.Verts[i].U = us[i];
        item.Verts[i].V = vs[i];
    }
    item.Key.TextureIdx = UiLayerAtlas::Texture(n.Final.Page).idx;
    item.Bounds = glm::vec4(x0, y0, x1, y1);
}

// ── running ───────────────────────────────────────────────────────────────

namespace
{
    // One quad over (x0, y0)-(x1, y1) in destination pixels, with the uv rectangle of a source region.
    bool FxQuad(Shader* shader, uint16_t view, uint64_t state, const bgfx::VertexLayout& layout, float x0, float y0, float x1, float y1, const glm::vec4& uv)
    {
        if (bgfx::getAvailTransientVertexBuffer(4, layout) < 4 || bgfx::getAvailTransientIndexBuffer(6) < 6)
            return false;
        bgfx::TransientVertexBuffer tvb;
        bgfx::TransientIndexBuffer tib;
        bgfx::allocTransientVertexBuffer(&tvb, 4, layout);
        bgfx::allocTransientIndexBuffer(&tib, 6);
        UiRender::Vertex* v = reinterpret_cast<UiRender::Vertex*>(tvb.data);
        const float xs[4] = { x0, x1, x1, x0 };
        const float ys[4] = { y0, y0, y1, y1 };
        const float us[4] = { uv.x, uv.z, uv.z, uv.x };
        const float vs[4] = { uv.y, uv.y, uv.w, uv.w };
        for (int i = 0; i < 4; ++i)
            v[i] = { xs[i], ys[i], us[i], vs[i], 0.f, 0.f, 0.f, 0.f };
        static const uint16_t kIndices[6] = { 0, 1, 2, 0, 2, 3 };
        std::memcpy(tib.data, kIndices, sizeof(kIndices));
        bgfx::setState(state);
        bgfx::setVertexBuffer(0, &tvb);
        bgfx::setIndexBuffer(&tib);
        shader->Submit(view);
        return true;
    }
}

void UiLayerPlan::Run(const DrawRange& draw, const bgfx::VertexLayout& layout)
{
    const uint64_t stateReplace = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A;
    const uint64_t stateOver = stateReplace | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_INV_SRC_ALPHA);
    const float pageTexel = 1.f / static_cast<float>(UiLayerAtlas::kPageSize);

    for (const Pass& p : m_passes)
    {
        bgfx::setViewFrameBuffer(p.View, UiLayerAtlas::FrameBuffer(p.Dst.Page));
        bgfx::setViewRect(p.View, static_cast<uint16_t>(p.Dst.Rect.X), static_cast<uint16_t>(p.Dst.Rect.Y),
                          static_cast<uint16_t>(p.Dst.Rect.W), static_cast<uint16_t>(p.Dst.Rect.H));
        bgfx::setViewClear(p.View, BGFX_CLEAR_COLOR, p.Scene ? 0x000000ffu : 0x00000000u, 1.0f, 0);   // the scene copy keeps alpha 1
        bgfx::setViewMode(p.View, bgfx::ViewMode::Sequential);
        bgfx::touch(p.View);

        if (p.T == Pass::Type::Content)
        {
            const glm::mat4 proj = glm::ortho(static_cast<float>(p.Rect[0]), static_cast<float>(p.Rect[2]),
                                              static_cast<float>(p.Rect[3]), static_cast<float>(p.Rect[1]), -1.f, 1.f);
            draw(p.First, p.Last, p.View, proj);
            continue;
        }

        Shader* shader = p.T == Pass::Type::Blur ? s_blur : p.T == Pass::Type::Down ? s_down : s_shadow;
        if (shader == nullptr || !shader->IsValid())
            continue;
        const float dw = static_cast<float>(p.Dst.Rect.W), dh = static_cast<float>(p.Dst.Rect.H);
        const glm::mat4 proj = glm::ortho(0.f, dw, dh, 0.f, -1.f, 1.f);
        const glm::vec4 uv = UiLayerAtlas::UvRect(p.Src);

        auto begin = [&](const UiLayerAtlas::Region& src) {
            shader->UseProgram();
            shader->SetUniform("u_uiProjection", proj);
            shader->SetTexture("s_fxSrc", UiLayerAtlas::Texture(src.Page));
            shader->SetUniform("u_fxClamp", UiLayerAtlas::ClampRect(src));
        };

        switch (p.T)
        {
        case Pass::Type::Down:
            if (p.Scene)
            {
                shader->UseProgram();
                shader->SetUniform("u_uiProjection", proj);
                shader->SetTexture("s_fxSrc", p.SrcTex);
                shader->SetUniform("u_fxClamp", p.SrcClamp);
                shader->SetUniform("u_fxStep", glm::vec4(p.SrcStep.x, p.SrcStep.y, 0.f, 0.f));
                FxQuad(shader, p.View, BGFX_STATE_WRITE_RGB, layout, 0.f, 0.f, dw, dh, p.SrcUv);   // colour only: the cleared alpha stays 1
                break;
            }
            begin(p.Src);
            shader->SetUniform("u_fxStep", glm::vec4(pageTexel, pageTexel, 0.f, 0.f));
            FxQuad(shader, p.View, stateReplace, layout, 0.f, 0.f, dw, dh, uv);
            break;
        case Pass::Type::Blur:
        {
            float w[5], o[4];
            BlurKernel(p.Sigma, w, o);
            begin(p.Src);
            shader->SetUniform("u_fxStep", p.Vertical ? glm::vec4(0.f, pageTexel, 0.f, 0.f) : glm::vec4(pageTexel, 0.f, 0.f, 0.f));
            shader->SetUniform("u_fxW0", glm::vec4(w[0], w[1], w[2], w[3]));
            shader->SetUniform("u_fxW1", glm::vec4(w[4], 0.f, 0.f, 0.f));
            shader->SetUniform("u_fxO0", glm::vec4(o[0], o[1], o[2], o[3]));
            FxQuad(shader, p.View, stateReplace, layout, 0.f, 0.f, dw, dh, uv);
            break;
        }
        case Pass::Type::Shadow:
            begin(p.Src);
            shader->SetUniform("u_fxMode", glm::vec4(1.f, 0.f, 0.f, 0.f));
            shader->SetUniform("u_fxColor", p.Color);
            FxQuad(shader, p.View, stateOver, layout, p.Offset.x, p.Offset.y, dw + p.Offset.x, dh + p.Offset.y, uv);
            if (p.HasOver)
            {
                begin(p.Over);
                shader->SetUniform("u_fxMode", glm::vec4(0.f));
                shader->SetUniform("u_fxColor", glm::vec4(1.f));
                FxQuad(shader, p.View, stateOver, layout, 0.f, 0.f, dw, dh, UiLayerAtlas::UvRect(p.Over));
            }
            break;
        case Pass::Type::Copy:
            begin(p.Src);
            shader->SetUniform("u_fxMode", glm::vec4(0.f));
            shader->SetUniform("u_fxColor", glm::vec4(1.f));
            FxQuad(shader, p.View, stateReplace, layout, 0.f, 0.f, dw, dh, uv);
            break;
        default:
            break;
        }
    }
}
