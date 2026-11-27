#include "UiWorldSurface.h"

#include "../Style/UiComputedStyle.h"
#include "../UiNavigation.h"
#include "../UiRenderer.h"
#include "../UiSystem.h"

#include <BgfxStateManager.h>
#include <Camera.h>
#include <EngineMain.h>
#include <Input.h>
#include <Physics.h>
#include <Renderer/Abstractions/ViewIdManager.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace
{
    std::recursive_mutex g_listMutex;
    std::vector<UiWorldSurface*> g_surfaces;
    UiWorldSurface* g_aimed = nullptr;
    UiWorldSurface* g_focused = nullptr;
    int g_renderCount = 0;
    int g_nextTouchId = 100000;

    // The quad of widgetPlane.glb spans -0.5..0.5 in local x and y. Seen from the front (the side a camera-facing surface shows), the
    // image grows to the right with local -x, and its top is local +y (checked with the p5_aimmap test).
    constexpr float kQuadHalf = 0.5f;

    bool ParseBool(const std::string& v) { return v.empty() || v == "true" || v == "1"; }

    glm::vec2 MouseUi()
    {
        const auto it = Input::TouchActions.find(1);
        return it != Input::TouchActions.end() ? it->second.position : Input::MousePos;
    }

    // World ray through the mouse (UI units of the viewport).
    glm::vec3 MouseRay()
    {
        const glm::vec2 size = EngineMain::Viewport.GetSize();
        const glm::vec2 m = MouseUi();
        const float nx = m.x / std::max(1.f, size.x) * 2.f - 1.f;
        const float ny = 1.f - m.y / std::max(1.f, size.y) * 2.f;
        const float px = std::max(0.0001f, std::abs(Camera::projection[0][0]));
        const float py = std::max(0.0001f, std::abs(Camera::projection[1][1]));
        return glm::normalize(Camera::Forward() + Camera::Right() * (nx / px) + Camera::Up() * (ny / py));
    }
}

struct UiWorldSurface::NavHolder { UiNavigation::State State; };

// ── construction

UiWorldSurface::UiWorldSurface(Entity* owner) : StaticMesh(owner)
{
    LoadFromFile("GameData/models/engine/widgetPlane.glb", AssetLoadTier::Visual);
    IncludeInEntityBounds = false;
    DepthPrePath = false;
    DepthWrite = false;
    PixelShader = "fs_unlit_rect";
    Transparent = true;
    blendMode = BgfxStateManager::Blend::Premultiplied;

    m_alive = std::make_shared<int>(0);
    m_nav = std::make_unique<NavHolder>();
    std::lock_guard<std::recursive_mutex> lock(g_listMutex);
    m_touchId = g_nextTouchId++;
    g_surfaces.push_back(this);
}

UiWorldSurface::~UiWorldSurface()
{
    {
        std::lock_guard<std::recursive_mutex> lock(g_listMutex);
        g_surfaces.erase(std::remove(g_surfaces.begin(), g_surfaces.end(), this), g_surfaces.end());
        if (g_aimed == this) g_aimed = nullptr;
        if (g_focused == this) { g_focused = nullptr; Input::LockCursor = m_savedLockCursor; }
    }
    UiBillboardAtlasManager::Get().Free(m_allocation);
}

// ── content

void UiWorldSurface::SetContent(const std::string& layoutPath)
{
    ClearContent();
    std::shared_ptr<UiElem> root = UiElem::Load(layoutPath);
    if (!root)
        return;
    m_content = root;
    m_viewport.AddChild(root);
    ReadContentAttributes();
    ApplyPendingHandlers();
    m_forceRender = true;
}

void UiWorldSurface::SetContent(std::shared_ptr<UiElement> element)
{
    ClearContent();
    if (!element)
        return;
    m_viewport.AddChild(element);
    if (UiElem::IsElem(element.get()))
    {
        m_content = std::static_pointer_cast<UiElem>(element);
        UiSystem::RegisterRoot(m_content);
        ReadContentAttributes();
        ApplyPendingHandlers();
    }
    m_forceRender = true;
}

void UiWorldSurface::ClearContent()
{
    m_viewport.ClearChildren();
    m_content.reset();
    m_forceRender = true;
}

UiElem* UiWorldSurface::Content() const { return m_content.get(); }

void UiWorldSurface::On(const std::string& handler, std::function<void()> fn)
{
    m_handlers.emplace_back(handler, fn);
    if (m_content)
        m_content->On(handler, std::weak_ptr<void>(m_alive), fn);
}

void UiWorldSurface::ApplyPendingHandlers()
{
    if (!m_content)
        return;
    for (auto& [name, fn] : m_handlers)
        m_content->On(name, std::weak_ptr<void>(m_alive), fn);
    for (auto& [name, value] : m_pendingVars)
        m_content->SetVar(name, value);
    m_pendingVars.clear();
}

void UiWorldSurface::SetVar(const std::string& name, const std::string& value)
{
    if (m_content) m_content->SetVar(name, value);
    else m_pendingVars.emplace_back(name, value);
}

void UiWorldSurface::SetVar(const std::string& name, double value)
{
    SetVar(name, std::to_string(value));
}

void UiWorldSurface::ReadContentAttributes()
{
    if (!m_content)
        return;
    const auto& a = m_content->Attributes;
    auto number = [&](const char* key, float& out) {
        const auto it = a.find(key);
        if (it != a.end() && !it->second.empty())
            out = static_cast<float>(std::atof(it->second.c_str()));
    };
    number("pixels-per-meter", PixelsPerMeter);
    number("max-distance", MaxDistance);
    number("update-rate-distance", UpdateRateDistance);
    number("freeze-distance", FreezeDistance);
    const auto facing = a.find("facing");
    if (facing != a.end())
    {
        if (facing->second == "billboard") FacingMode = Facing::Billboard;
        else if (facing->second == "billboard-y") FacingMode = Facing::BillboardY;
        else if (facing->second == "fixed") FacingMode = Facing::Fixed;
    }
    const auto interaction = a.find("interaction");
    if (interaction != a.end())
    {
        if (interaction->second == "none") InteractionMode = Interaction::None;
        else if (interaction->second == "aim") InteractionMode = Interaction::Aim;
        else if (interaction->second == "focus") InteractionMode = Interaction::Focus;
        else if (interaction->second == "both") InteractionMode = Interaction::Both;
    }
}

// ── the pointer API

void UiWorldSurface::Hover(float x, float y)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_manualPixel = glm::vec2(x, y);
    m_manualActive = true;
    m_manualRay = false;
}

void UiWorldSurface::HoverUV(float u, float v)
{
    Hover(u * static_cast<float>(ViewportSize.x), v * static_cast<float>(ViewportSize.y));
}

void UiWorldSurface::HoverRay(const glm::vec3& origin, const glm::vec3& direction)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_rayOrigin = origin;
    m_rayDir = direction;
    m_manualRay = true;
    m_manualActive = true;
}

void UiWorldSurface::Unhover()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_manualActive = false;
    m_manualRay = false;
}

void UiWorldSurface::Press() { std::lock_guard<std::mutex> lock(m_mutex); m_buttonQueue.push_back(true); }
void UiWorldSurface::Release() { std::lock_guard<std::mutex> lock(m_mutex); m_buttonQueue.push_back(false); }
void UiWorldSurface::Click()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_buttonQueue.push_back(true);
    m_buttonQueue.push_back(false);
}

// ── geometry

glm::vec3 UiWorldSurface::Normal() const
{
    return glm::normalize(glm::vec3(const_cast<UiWorldSurface*>(this)->GetWorldMatrix() * glm::vec4(0.f, 0.f, 1.f, 0.f)));
}

bool UiWorldSurface::Intersect(const glm::vec3& origin, const glm::vec3& dir, AimHit& out) const
{
    const glm::mat4 world = const_cast<UiWorldSurface*>(this)->GetWorldMatrix();
    const glm::mat4 inv = glm::inverse(world);
    const glm::vec3 lo = glm::vec3(inv * glm::vec4(origin, 1.f));
    const glm::vec3 ld = glm::vec3(inv * glm::vec4(dir, 0.f));
    if (std::abs(ld.z) < 1e-7f)
        return false;
    const float t = -lo.z / ld.z;
    if (t < 0.f)
        return false;
    const glm::vec3 hit = lo + ld * t;
    if (std::abs(hit.x) > kQuadHalf || std::abs(hit.y) > kQuadHalf)
        return false;
    out.Pixel = glm::vec2((0.5f - hit.x / kQuadHalf * 0.5f) * static_cast<float>(ViewportSize.x),
                          (0.5f - hit.y / kQuadHalf * 0.5f) * static_cast<float>(ViewportSize.y));
    out.World = glm::vec3(world * glm::vec4(hit, 1.f));
    out.Distance = glm::length(out.World - origin);
    out.Cos = std::abs(glm::dot(glm::normalize(dir), Normal()));
    out.Valid = true;
    return true;
}

glm::mat4 UiWorldSurface::FocusCameraPose() const
{
    const glm::vec3 centre = Position;
    glm::vec3 towardViewer = Normal();
    if (glm::dot(towardViewer, Camera::position - centre) < 0.f)
        towardViewer = -towardViewer;
    const float height = static_cast<float>(ViewportSize.y) / PixelsPerMeter;
    const float width = static_cast<float>(ViewportSize.x) / PixelsPerMeter;
    const float tanHalf = std::tan(glm::radians(Camera::FOV) * 0.5f);
    const float aspect = std::max(0.1f, static_cast<float>(Camera::AspectRatio));
    const float dist = std::max(height * 0.5f / tanHalf, width * 0.5f / (tanHalf * aspect)) * 1.15f;
    const glm::vec3 pos = centre + towardViewer * dist;
    const glm::vec3 rot = MathHelper::FindLookAtRotation(pos, centre);
    return glm::translate(glm::mat4(1.f), pos) * MathHelper::GetRotationMatrix(rot);
}

// ── focus

UiWorldSurface* UiWorldSurface::Aimed() { return g_aimed; }
UiWorldSurface* UiWorldSurface::Focused() { return g_focused; }
bool UiWorldSurface::IsFocused() const { return g_focused == this; }
bool UiWorldSurface::IsAimingAtInteractive() { return g_aimed != nullptr && g_aimed->m_overInteractive; }
int UiWorldSurface::RenderCount() { return g_renderCount; }
int UiWorldSurface::SurfaceCount() { std::lock_guard<std::recursive_mutex> lock(g_listMutex); return static_cast<int>(g_surfaces.size()); }

void UiWorldSurface::EnterFocus()
{
    if (g_focused == this)
        return;
    if (g_focused != nullptr)
        g_focused->ExitFocus();
    g_focused = this;
    m_inFocus = true;
    m_savedLockCursor = Input::LockCursor;
    Input::LockCursor = false;
    if (OnFocusEnter)
        OnFocusEnter();
}

void UiWorldSurface::ExitFocus()
{
    if (g_focused != this)
        return;
    g_focused = nullptr;
    m_inFocus = false;
    Input::LockCursor = m_savedLockCursor;
    // leave nothing focused behind
    UiNavigation::State saved;
    UiNavigation::Swap(m_nav->State);
    UiNavigation::ClearFocus();
    UiNavigation::Swap(m_nav->State);
    (void)saved;
    if (OnFocusExit)
        OnFocusExit();
}

void UiWorldSurface::SetOutOfRange(bool out)
{
    if (out == m_outOfRange)
        return;
    m_outOfRange = out;
    if (m_content)
    {
        if (out) m_content->States |= kStateOutOfRange;
        else m_content->States &= ~kStateOutOfRange;
        m_content->MarkStyleDirty();
    }
}

// ── the update of one surface (main thread, inside UiSystem::Update)

void UiWorldSurface::ProcessUi(const AimHit* aimHit, bool inRange, bool outOfRange, bool aimed)
{
    SetOutOfRange(outOfRange);

    // size follows the CSS width / height of the content root
    if (m_content)
    {
        const UiComputedStyle& s = m_content->Computed();
        glm::ivec2 size = ViewportSize;
        if (s.Width.Kind == UiLenKind::Px && s.Width.Px >= 1.f) size.x = static_cast<int>(std::ceil(s.Width.Px));
        if (s.Height.Kind == UiLenKind::Px && s.Height.Px >= 1.f) size.y = static_cast<int>(std::ceil(s.Height.Px));
        size = glm::clamp(size, glm::ivec2(16), glm::ivec2(2048));
        if (size != ViewportSize)
        {
            ViewportSize = size;
            m_forceRender = true;
        }
    }
    m_viewport.SetPixelSize(glm::vec2(ViewportSize));

    // pointer sources: the game's Hover / HoverRay, else the aim
    bool press = false, release = false;
    glm::vec2 pointer(0.f);
    bool hasPointer = false;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_manualActive)
        {
            if (m_manualRay)
            {
                AimHit hit;
                if (Intersect(m_rayOrigin, m_rayDir, hit))
                {
                    pointer = hit.Pixel;
                    hasPointer = true;
                }
            }
            else
            {
                pointer = m_manualPixel;
                hasPointer = true;
            }
        }
        if (!m_buttonQueue.empty())
        {
            const bool down = m_buttonQueue.front();
            m_buttonQueue.erase(m_buttonQueue.begin());
            if (down) press = true; else release = true;
        }
    }
    if (!hasPointer && aimed && aimHit != nullptr && aimHit->Valid && inRange)
    {
        pointer = aimHit->Pixel;
        hasPointer = true;
        InputAction* use = Input::GetAction("interact");
        if (use->Pressed())
        {
            press = true;
            if (ConsumesUse && m_overInteractive)
                use->CleanPressed();
        }
        if (use->Released())
            release = true;
    }
    if (!hasPointer && m_pressed)
    {
        release = true;   // the pointer left while held
        pointer = m_pointer;
        hasPointer = true;
    }
    if (release && !m_pressed)
        release = false;
    if (press && m_pressed)
        press = false;

    TouchEvent ev;
    ev.id = m_touchId;
    ev.position = pointer;
    ev.delta = hasPointer ? pointer - m_lastPointer : glm::vec2(0.f);
    ev.pressed = press;
    ev.released = release;
    if (press) m_pressed = true;
    if (release) m_pressed = false;
    m_lastPointer = pointer;
    m_pointer = pointer;
    m_hasPointer = hasPointer;

    // this surface's navigation state (focus, hover focus, root) is swapped in for the whole update
    UiNavigation::Swap(m_nav->State);
    UiElement* const focusBefore = UiNavigation::Focused;

    m_viewport.Update();                      // positions of the legacy tree
    m_viewport.ResetTouchInputs();
    m_overInteractive = false;
    std::unordered_map<int, TouchEvent> touches;
    if (hasPointer)
    {
        touches[m_touchId] = ev;
        const std::shared_ptr<UiElement> hit = m_viewport.GetHitElementUnderPosition(pointer);
        if (hit)
        {
            hit->TouchEvents.push_back(ev);
            m_overInteractive = true;
        }
    }
    m_viewport.TouchInputPostProcessing();
    UiSystem::ProcessPointersFor(m_viewport, touches);

    if (m_inFocus)
    {
        UiNavigation::Update(&m_viewport);
        UiNavigation::LateUpdate();
    }
    else if (UiNavigation::Focused != nullptr)
        UiNavigation::ClearFocus();

    m_viewport.Update();
    const bool focusChanged = UiNavigation::Focused != focusBefore;
    UiSystem::UpdateRootsUnder(m_viewport, focusChanged);
    UiNavigation::Swap(m_nav->State);

    if (m_inFocus && Input::GetAction("ui_cancel")->Pressed())
        ExitFocus();
}

// ── the frame hooks

bool UiWorldSurfaces::IsSurfaceViewport(const UiElement* viewport)
{
    if (viewport == nullptr)
        return false;
    std::lock_guard<std::recursive_mutex> lock(g_listMutex);
    for (UiWorldSurface* s : g_surfaces)
        if (static_cast<const UiElement*>(&s->m_viewport) == viewport)
            return true;
    return false;
}

bool UiWorldSurfaces::AnyFocused() { return g_focused != nullptr; }

void UiWorldSurfaces::Update()
{
    std::lock_guard<std::recursive_mutex> lock(g_listMutex);
    if (g_surfaces.empty())
    {
        g_aimed = nullptr;
        return;
    }
    static uint64_t lastTicks = 0;
    const uint64_t now = SDL_GetTicks64();
    const float dt = lastTicks == 0 ? 0.f : std::min(static_cast<float>(now - lastTicks) * 0.001f, 0.1f);
    lastTicks = now;

    const bool mouseAims = !Input::LockCursor && EngineMain::Viewport.GetHitElementUnderPosition(MouseUi()) == nullptr;
    const glm::vec3 origin = Camera::position;
    const glm::vec3 lookDir = mouseAims ? MouseRay() : Camera::Forward();

    // ── who is aimed: the nearest surface that is in reach, faces the ray and is not hidden behind something
    struct Candidate { UiWorldSurface* S; UiWorldSurface::AimHit Hit; bool InRange; };
    std::vector<Candidate> found;
    for (UiWorldSurface* s : g_surfaces)
    {
        s->m_distance = glm::length(s->Position - origin);
        const bool wantsAim = s->InteractionMode == UiWorldSurface::Interaction::Aim || s->InteractionMode == UiWorldSurface::Interaction::Both || s->m_inFocus;
        if (!wantsAim || !s->Visible || s->m_distance > s->FreezeDistance)
            continue;
        UiWorldSurface::AimHit hit;
        if (!s->Intersect(origin, lookDir, hit))
            continue;
        if (hit.Cos < std::cos(glm::radians(s->MaxAngleDeg)) || hit.Distance > s->MaxDistance * 2.f)
            continue;
        found.push_back({ s, hit, hit.Distance <= s->MaxDistance });
    }
    std::sort(found.begin(), found.end(), [](const Candidate& a, const Candidate& b) { return a.Hit.Distance < b.Hit.Distance; });
    UiWorldSurface* aimed = nullptr;
    const UiWorldSurface::AimHit* aimedHit = nullptr;
    for (Candidate& c : found)
    {
        if (!c.InRange)
            continue;
        const Physics::HitResult block = Physics::LineTrace(origin, c.Hit.World, BodyType::World, {}, { c.S->OwnerEntity });   // the world blocks the view of a surface; actors do not (the camera is inside the player)
        if (block.hasHit && block.entity != c.S->OwnerEntity)
            continue;   // something stands between
        aimed = c.S;
        aimedHit = &c.Hit;
        break;
    }
    if (g_focused != nullptr)
        aimed = g_focused;   // in focus mode the mouse ray belongs to the focused surface only
    if (g_focused != nullptr)
    {
        aimedHit = nullptr;
        for (Candidate& c : found)
            if (c.S == g_focused) aimedHit = &c.Hit;
    }

    if (aimed != g_aimed)
    {
        if (g_aimed != nullptr && g_aimed->OnAimLeave) g_aimed->OnAimLeave();
        g_aimed = aimed;
        if (g_aimed != nullptr && g_aimed->OnAimEnter) g_aimed->OnAimEnter();
    }

    // ── every surface
    for (UiWorldSurface* s : g_surfaces)
    {
        s->m_visibleToCamera = true;
        if (!s->Visible || s->m_distance > s->FreezeDistance)
        {
            s->m_visibleToCamera = false;
            continue;
        }
        if (s != g_aimed && !s->m_inFocus)
        {
            const glm::mat4 w = s->GetWorldMatrix();
            glm::vec3 lo(1e9f), hi(-1e9f);
            for (float cx : { -kQuadHalf, kQuadHalf })
                for (float cy : { -kQuadHalf, kQuadHalf })
                {
                    const glm::vec3 p = glm::vec3(w * glm::vec4(cx, cy, 0.f, 1.f));
                    lo = glm::min(lo, p);
                    hi = glm::max(hi, p);
                }
            if (!Camera::frustum.IsBoxVisible(lo, hi))
            {
                s->m_visibleToCamera = false;
                continue;
            }
        }
        s->m_accumulated += dt;
        if (s->m_distance > s->UpdateRateDistance && s != g_aimed && !s->m_inFocus)
        {
            if (s->m_accumulated < 0.1f)
                continue;   // 10 Hz in the distance
        }
        s->m_accumulated = 0.f;
        s->Tick();
        s->UpdateLegacyContent();
        const bool isAimed = s == g_aimed;
        const bool outOfRange = !isAimed && s->InteractionMode != UiWorldSurface::Interaction::None &&
            s->m_distance > s->MaxDistance && s->m_distance <= s->MaxDistance * 2.f;
        s->ProcessUi(isAimed ? aimedHit : nullptr, true, outOfRange, isAimed);
    }
}

void UiWorldSurfaces::Finalize()
{
    std::lock_guard<std::recursive_mutex> lock(g_listMutex);
    for (UiWorldSurface* s : g_surfaces)
    {
        if (!s->m_visibleToCamera)
            continue;
        UiNavigation::Swap(s->m_nav->State);
        UiSystem::FinalizeRootsUnder(s->m_viewport);
        UiNavigation::Swap(s->m_nav->State);
        s->m_viewport.FinalizeChildren();
    }
}

// ── mesh and rendering

void UiWorldSurface::FinalizeFrameData()
{
    switch (FacingMode)
    {
    case Facing::Billboard: Rotation = Camera::rotation; break;
    case Facing::BillboardY: Rotation = glm::vec3(0.f, Camera::rotation.y, 0.f); break;
    case Facing::Fixed: break;
    }

    Scale.x = static_cast<float>(ViewportSize.x) / PixelsPerMeter;
    Scale.y = static_cast<float>(ViewportSize.y) / PixelsPerMeter;

    ContentRoot().FinalizeChildren();

    if (drawCommands.size() > 0)
        GetMaterial(0).SurfaceTypeOverride = SurfaceType::Transparent;

    StaticMesh::FinalizeFrameData();
}

// PreDraw runs after FinalizeFrameData() (see Renderer::RenderLevel): the atlas slot is (re)allocated and filled here, so the
// command already carries the right texture and rectangle this frame.
void UiWorldSurface::PreDraw()
{
    StaticMesh::PreDraw();

    if (m_visibleToCamera)
        DrawUi();

    if (!m_allocation.IsValid()) return;

    ColorTexture = nullptr;
    ColorTextureId = m_allocation.atlas->GetTexture()->textureHandle().idx;
    MeshCustomShaderParams["u_atlasRect"] = m_allocation.uvRect;

    StaticMeshDrawCommand& cmd = GetDrawCommandAt(0);
    cmd.Material.ColorTexture = nullptr;
    cmd.Material.ColorTextureId = ColorTextureId;
    cmd.CustomShaderParams["u_atlasRect"] = m_allocation.uvRect;
}

bool UiWorldSurface::NeedsRender() const
{
    if (m_forceRender || AlwaysRender() || m_renderedSize != ViewportSize)
        return true;
    size_t index = 0;
    for (const auto& child : m_viewport.children)
    {
        if (!UiElem::IsElem(child.get()))
            return true;   // legacy content has no snapshot to compare
        const UiElem& root = static_cast<const UiElem&>(*child);
        if (index >= m_renderedFrames.size() || m_renderedFrames[index] != root.m_frame.get())
            return true;
        if (root.m_frame && !root.m_frame->Markers.empty())
            return true;
        ++index;
    }
    return index != m_renderedFrames.size();
}

void UiWorldSurface::RememberRendered()
{
    m_renderedFrames.clear();
    for (const auto& child : m_viewport.children)
        if (UiElem::IsElem(child.get()))
            m_renderedFrames.push_back(static_cast<const UiElem&>(*child).m_frame.get());
    m_renderedSize = ViewportSize;
    m_forceRender = false;
}

void UiWorldSurface::DrawUi()
{
    EnsureAtlasAllocation();
    if (!m_allocation.IsValid()) return;
    if (!NeedsRender())
        return;
    RememberRendered();
    ++g_renderCount;

    UiRenderer::customViewport = true;
    UiRenderer::customViewportSize = ViewportSize;

    m_allocation.atlas->GetTexture()->setAsRenderTarget();
    bgfx::setViewRect(ViewIdManager::GetCurrentId(), (uint16_t)m_allocation.rect.x, (uint16_t)m_allocation.rect.y,
                      (uint16_t)ViewportSize.x, (uint16_t)ViewportSize.y);
    bgfx::setViewClear(ViewIdManager::GetCurrentId(), BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x00000000, 1.0f, 0);

    auto savedState = BgfxStateManager::GetState();
    BgfxStateManager::Reset();
    BgfxStateManager::SetDepthTest(BgfxStateManager::DepthTest::Always);
    BgfxStateManager::SetBlend(BgfxStateManager::Blend::Premultiplied);
    UiRenderer::BeginPass();

    ContentRoot().Draw();
    UiElement::DrawingLate = true;
    for (auto elem : UiElement::pendingLateDrawElements)
        elem->Draw();
    UiElement::DrawingLate = false;
    UiElement::pendingLateDrawElements.clear();
    UiRenderer::EndPass(ViewIdManager::GetCurrentId());
    UiRenderer::EndFrame();

    BgfxStateManager::SetState(savedState);
    UiRenderer::customViewport = false;
}

void UiWorldSurface::EnsureAtlasAllocation()
{
    if (m_allocation.IsValid() && m_allocation.rect.z == ViewportSize.x && m_allocation.rect.w == ViewportSize.y)
        return;
    if (m_allocation.IsValid())
        UiBillboardAtlasManager::Get().Free(m_allocation);
    m_allocation = UiBillboardAtlasManager::Get().Allocate(ViewportSize);
    m_forceRender = true;   // a new slot is empty
}
