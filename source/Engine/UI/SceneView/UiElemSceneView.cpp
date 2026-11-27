#include "UiElemSceneView.h"

#include "UiSceneTargets.h"

#include <BgfxStateManager.h>
#include <Camera.h>
#include <EngineMain.h>
#include <LightSystem/LightManager.h>
#include <SkeletalMesh.hpp>
#include <StaticMesh.h>
#include <DrawCommands/StaticMeshDrawCommand.h>
#include <Renderer/Abstractions/ViewIdManager.h>
#include <UI/Markup/UiTagRegistry.h>
#include <UI/Render/UiGpu.h>
#include <UI/Render/UiPainter.h>
#include <UI/UiSystem.h>

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <mutex>

namespace
{
    // never destroyed: scene views are destroyed in static destruction, after function-local statics would be gone
    std::recursive_mutex& g_mutex = *new std::recursive_mutex();
    std::vector<UiElemSceneView*>& g_views = *new std::vector<UiElemSceneView*>();
    int g_totalRenders = 0;

    constexpr float kSettleSeconds = 0.5f;     // geometry is up: the model stays out of the picture until its textures could register
    constexpr float kWarmSeconds = 0.6f;       // after the first finalize
    constexpr float kTextureTimeout = 2.f;     // seconds to wait for textures once the model is up

    // Real seconds since the last UI frame, shared by every element of the frame (a long frame never counts for more than 0.1 s).
    float RealDt()
    {
        static int s_frame = -1;
        static float s_dt = 0.f;
        static uint64_t s_last = 0;
        const int frame = UiSystem::FrameIndex();
        if (frame != s_frame)
        {
            s_frame = frame;
            const uint64_t now = SDL_GetTicks64();
            s_dt = s_last == 0 ? 0.f : std::min(static_cast<float>(now - s_last) * 0.001f, 0.1f);
            s_last = now;
        }
        return s_dt;
    }

    bool ParseFloats(const std::string& v, float* out, int count)
    {
        const char* p = v.c_str();
        for (int i = 0; i < count; ++i)
        {
            char* end = nullptr;
            out[i] = std::strtof(p, &end);
            if (end == p)
                return false;
            p = end;
        }
        return true;
    }

    bool ParseVec3(const std::string& v, glm::vec3& out)
    {
        float f[3];
        if (!ParseFloats(v, f, 3))
            return false;
        out = glm::vec3(f[0], f[1], f[2]);
        return true;
    }

    // "transparent", #rgb, #rrggbb, #rrggbbaa
    bool ParseColor(const std::string& text, glm::vec4& out)
    {
        if (text == "transparent" || text.empty())
        {
            out = glm::vec4(0.f);
            return true;
        }
        if (text[0] != '#')
            return false;
        std::string hex = text.substr(1);
        if (hex.size() == 3)
            hex = std::string{ hex[0], hex[0], hex[1], hex[1], hex[2], hex[2] };
        if (hex.size() != 6 && hex.size() != 8)
            return false;
        auto channel = [&](size_t i) { return static_cast<float>(std::strtol(hex.substr(i * 2, 2).c_str(), nullptr, 16)) / 255.f; };
        out = glm::vec4(channel(0), channel(1), channel(2), hex.size() == 8 ? channel(3) : 1.f);
        return true;
    }

    // Geometry uploaded, and (when asked) every base colour texture requested so far arrived. Emissive maps are ignored: a missing one
    // never reaches the Visual tier.
    bool ModelReady(const StaticMesh* mesh, bool texturesRequested = false)
    {
        if (mesh == nullptr || mesh->model == nullptr)
            return false;
        if (mesh->model->loadState.currentTier.load(std::memory_order_acquire) != AssetLoadTier::Visual)
            return false;
        for (const auto& sub : mesh->model->meshes)
        {
            if (texturesRequested && !sub.cachedBaseColor)
                return false;
            if (sub.cachedBaseColor && sub.cachedBaseColor->loadState.currentTier.load(std::memory_order_acquire) != AssetLoadTier::Visual)
                return false;
        }
        return true;
    }

    UiElemSceneView* ViewOf(const UiElem& e)
    {
        for (const UiElem* p = e.AsParentElem(); p != nullptr; p = p->AsParentElem())
            if (auto* view = dynamic_cast<UiElemSceneView*>(const_cast<UiElem*>(p)))
                return view;
        return nullptr;
    }
}

// A model lit with the lights of its scene view instead of the level's light grid, outside any level.
class UiSceneMesh : public SkeletalMesh
{
public:
    UiSceneMesh() : SkeletalMesh(nullptr) {}
    LightVolPointData Light;
    LightVolPointData GetLightVolData() override { return Light; }
};

// ── Model

UiSceneModel::UiSceneModel()
{
    TagName = "Model";
    m_visibleOverrideCollapse = true;   // takes no room and paints nothing
    HitCheck = false;
}

UiSceneModel::~UiSceneModel() = default;

UiElemSceneView* UiSceneModel::View() const { return ViewOf(*this); }

void UiSceneModel::NotifyView()
{
    if (UiElemSceneView* view = View())
        view->MarkSceneDirty();
}

bool UiSceneModel::ApplyAttribute(const std::string& name, const std::string& value)
{
    if (name == "src") { SetSource(value); return true; }
    if (name == "position") { ParseVec3(value, m_position); m_transformDirty = true; NotifyView(); return true; }
    if (name == "rotation") { ParseVec3(value, m_rotation); m_transformDirty = true; NotifyView(); return true; }
    if (name == "scale")
    {
        float f[3];
        if (ParseFloats(value, f, 3)) m_scale = glm::vec3(f[0], f[1], f[2]);
        else if (ParseFloats(value, f, 1)) m_scale = glm::vec3(f[0]);
        m_transformDirty = true;
        NotifyView();
        return true;
    }
    if (name == "animation") { PlayAnimation(value); return true; }
    if (name == "two-sided") { m_twoSided = value.empty() || value == "true" || value == "1"; if (m_mesh) m_mesh->TwoSided = m_twoSided; NotifyView(); return true; }
    return false;
}

void UiSceneModel::SetSource(const std::string& path)
{
    if (path == m_src && (m_mesh != nullptr || path.empty()))
        return;
    m_src = path;
    m_mesh.reset();
    m_state = State::None;
    m_posed = false;
    m_timer = 0.f;
    if (!path.empty())
    {
        m_mesh = std::make_unique<UiSceneMesh>();
        m_mesh->CastShadows = false;
        m_mesh->IncludeInEntityBounds = false;
        m_mesh->TwoSided = m_twoSided;
        m_mesh->LoadFromFile(path, AssetLoadTier::Visual);
        m_state = State::Loading;
        m_transformDirty = true;
        if (m_animating)
            m_mesh->PlayAnimation(m_animation, true, 0.f);
    }
    NotifyView();
}

void UiSceneModel::SetTransform(const glm::vec3& position, const glm::vec3& rotationDegrees, const glm::vec3& scale)
{
    m_position = position;
    m_rotation = rotationDegrees;
    m_scale = scale;
    m_transformDirty = true;
    NotifyView();
}

void UiSceneModel::PlayAnimation(const std::string& name)
{
    m_animation = name;
    m_animating = !name.empty();
    m_animTime = 0.f;
    if (m_mesh && m_animating)
        m_mesh->PlayAnimation(name, true, 0.f);
    NotifyView();
}

void UiSceneModel::ApplyTransform()
{
    if (!m_mesh)
        return;
    m_mesh->Position = m_position;
    m_mesh->Rotation = m_rotation;
    m_mesh->Scale = m_scale;
    m_transformDirty = false;
}

void UiSceneModel::Tick()
{
    if (!m_mesh)
        return;
    const float dt = RealDt();
    if (m_state == State::Loading)
    {
        if (ModelReady(m_mesh.get()))
        {
            // Embedded textures register a little after the geometry: a texture asked for too early is cached as "not found".
            m_timer += dt;
            if (m_timer >= kSettleSeconds)
            {
                m_state = State::Warming;
                m_timer = 0.f;
                NotifyView();
            }
        }
    }
    else if (m_state == State::Warming)
    {
        m_timer += dt;
        // finalizing asks for the textures; give the request time to land (a model may have no texture at all), then wait for what exists
        if ((m_timer >= kWarmSeconds && ModelReady(m_mesh.get())) || m_timer > kTextureTimeout)
        {
            m_state = State::Ready;
            NotifyView();
        }
    }
    if (m_state != State::Loading && m_animating)
    {
        m_animTime += dt;
        NotifyView();
    }
}

void UiSceneModel::PrepareFrame()
{
    if (!m_mesh || (m_state != State::Warming && m_state != State::Ready))
        return;
    if (m_transformDirty)
        ApplyTransform();
    if (m_animating)
    {
        const float duration = m_mesh->GetAnimationDuration();
        m_mesh->SetAnimationTime(duration > 0.001f ? std::fmod(m_animTime, duration) : 0.f);
        m_mesh->Update(0.f);
    }
    else if (!m_posed)
    {
        m_mesh->PasteAnimationPose(AnimationPose());   // a blank pose is the rest pose
        m_posed = true;
    }
    m_mesh->PreFinalize();
    m_mesh->FinalizeFrameData();
}

// ── Light

UiSceneLight::UiSceneLight()
{
    TagName = "Light";
    m_visibleOverrideCollapse = true;
    HitCheck = false;
}

void UiSceneLight::NotifyView()
{
    if (UiElemSceneView* view = ViewOf(*this))
        view->MarkSceneDirty();
}

bool UiSceneLight::ApplyAttribute(const std::string& name, const std::string& value)
{
    if (name == "type")
    {
        LightType = value == "ambient" ? Type::Ambient : (value == "point" ? Type::Point : Type::Directional);
        NotifyView();
        return true;
    }
    if (name == "direction") { glm::vec3 d; if (ParseVec3(value, d) && glm::length(d) > 1e-5f) Direction = glm::normalize(d); NotifyView(); return true; }
    if (name == "position") { ParseVec3(value, Position); NotifyView(); return true; }
    if (name == "color") { glm::vec4 c; if (ParseColor(value, c)) Color = glm::vec3(c); NotifyView(); return true; }
    if (name == "intensity") { Intensity = static_cast<float>(std::atof(value.c_str())); NotifyView(); return true; }
    if (name == "range") { Range = static_cast<float>(std::atof(value.c_str())); NotifyView(); return true; }
    return false;
}

// ── SceneView

UiElemSceneView::UiElemSceneView()
{
    TagName = "SceneView";
    HitCheck = false;
    std::lock_guard<std::recursive_mutex> lock(g_mutex);
    g_views.push_back(this);
}

UiElemSceneView::~UiElemSceneView()
{
    std::lock_guard<std::recursive_mutex> lock(g_mutex);
    g_views.erase(std::remove(g_views.begin(), g_views.end(), this), g_views.end());
    UiSceneTargets::Release(this);
}

bool UiElemSceneView::ApplyAttribute(const std::string& name, const std::string& value)
{
    if (name == "camera-position") { ParseVec3(value, m_cameraPosition); m_dirty = true; return true; }
    if (name == "camera-target") { ParseVec3(value, m_cameraTarget); m_dirty = true; return true; }
    if (name == "fov") { m_fov = std::clamp(static_cast<float>(std::atof(value.c_str())), 5.f, 150.f); m_dirty = true; return true; }
    if (name == "background") { ParseColor(value, m_background); m_dirty = true; return true; }
    if (name == "orbit") { m_orbit = value.empty() || value == "true" || value == "1"; HitCheck = m_orbit; return true; }
    return false;
}

void UiElemSceneView::SetCamera(const glm::vec3& position, const glm::vec3& target, float fovDegrees)
{
    m_cameraPosition = position;
    m_cameraTarget = target;
    m_fov = std::clamp(fovDegrees, 5.f, 150.f);
    m_dirty = true;
}

UiSceneModel* UiElemSceneView::AddModel(const std::string& path)
{
    auto model = std::make_shared<UiSceneModel>();
    AddChild(model);
    model->SetSource(path);
    return model.get();
}

void UiElemSceneView::RemoveModel(UiSceneModel* model)
{
    if (model == nullptr)
        return;
    for (auto& child : children)
        if (child.get() == model)
        {
            RemoveChild(child);
            m_dirty = true;
            return;
        }
}

void UiElemSceneView::Tick()
{
    // the target is the content rect in target pixels, rounded up to a multiple of 8
    const UiRect r = ContentRectScreen();
    const float scale = UiGpu::MainRenderScale();
    auto round8 = [](float v) { return static_cast<int>(std::ceil(std::max(0.f, v) / 8.f)) * 8; };
    glm::ivec2 want(round8(r.W * scale), round8(r.H * scale));
    if (want.x < 8 || want.y < 8)
        want = glm::ivec2(0);
    want = glm::min(want, glm::ivec2(2048));
    if (want != m_wantSize)
    {
        m_wantSize = want;
        m_dirty = true;
    }
    const glm::vec2 view = EngineMain::Viewport.GetSize();
    const bool onScreen = r.X < view.x && r.Y < view.y && r.X + r.W > 0.f && r.Y + r.H > 0.f;
    m_visibleNow = want.x > 0 && Computed().Visibility == UiVisibility::Visible && IsVisible() && onScreen;
}

void UiElemSceneView::OnPointerDown(glm::vec2 localPoint)
{
    if (!m_orbit)
        return;
    m_dragging = true;
    m_lastPointer = localPoint;
}

void UiElemSceneView::OnPointerMove(glm::vec2 localPoint)
{
    if (!m_dragging)
        return;
    const glm::vec2 delta = localPoint - m_lastPointer;
    m_lastPointer = localPoint;
    m_yaw -= delta.x * 0.4f;
    m_pitch = std::clamp(m_pitch + delta.y * 0.4f, -85.f, 85.f);
    m_dirty = true;
}

void UiElemSceneView::OnPointerUp(glm::vec2)
{
    m_dragging = false;
}

void UiElemSceneView::PaintContent(UiPainter& painter, const UiRect& c)
{
    if (m_handle == 0xffffffffu || c.W <= 0.f || c.H <= 0.f)
        return;
    bgfx::TextureHandle texture;
    texture.idx = static_cast<uint16_t>(m_handle);
    // FlipV is decided here once, for every scene view: a target whose origin is at the bottom (OpenGL) is read upside down
    const bool flip = bgfx::getCaps() != nullptr && bgfx::getCaps()->originBottomLeft;
    const glm::vec4 uv = flip ? glm::vec4(0.f, 1.f, 1.f, 0.f) : glm::vec4(0.f, 0.f, 1.f, 1.f);
    painter.Image(c, texture, uv, glm::vec4(1.f, 1.f, 1.f, m_paintOpacity));
}

glm::mat4 UiElemSceneView::ViewMatrix() const
{
    // orbit: the camera turns around the target
    glm::vec3 offset = m_cameraPosition - m_cameraTarget;
    offset = glm::vec3(glm::rotate(glm::mat4(1.f), glm::radians(m_yaw), glm::vec3(0.f, 1.f, 0.f)) * glm::vec4(offset, 0.f));
    const glm::vec3 right = glm::normalize(glm::cross(glm::vec3(0.f, 1.f, 0.f), -offset) + glm::vec3(1e-6f));
    offset = glm::vec3(glm::rotate(glm::mat4(1.f), glm::radians(m_pitch), right) * glm::vec4(offset, 0.f));
    return glm::lookAt(m_cameraTarget + offset, m_cameraTarget, glm::vec3(0.f, 1.f, 0.f));
}

bool UiElemSceneView::NeedsRender() const
{
    if (m_dirty)
        return true;
    for (const auto& child : children)
        if (auto* model = dynamic_cast<const UiSceneModel*>(child.get()))
        {
            if (model->Animating())
                return true;
            if (model->GetState() == UiSceneModel::State::Loading || model->GetState() == UiSceneModel::State::Warming)
                return true;
        }
    return false;
}

void UiElemSceneView::Render()
{
    if (!m_visibleNow)
    {
        UiSceneTargets::Release(this);
        m_handle = 0xffffffffu;   // collapsed or offscreen: no target, no picture
        m_targetSize = glm::ivec2(0);
        return;
    }
    UiSceneTarget* target = UiSceneTargets::Request(this, m_wantSize);
    const unsigned handle = target->Color->textureHandle().idx;
    if (handle != m_handle || target->Size != m_targetSize)
    {
        m_handle = handle;
        m_targetSize = target->Size;
        m_dirty = true;
        MarkPaintDirty();
    }
    if (!NeedsRender())
        return;
    m_dirty = false;
    ++m_renders;
    ++g_totalRenders;

    // lights: the first directional one lights the models, ambient lights add up
    LightVolPointData lights;
    lights.ambientColor = glm::vec3(0.f);
    lights.directColor = glm::vec3(0.f);
    lights.direction = glm::normalize(glm::vec3(-0.3f, -1.f, -0.5f));
    std::vector<LightManager::PointLightInfo> points;   // real point lights, up to what the shader takes
    bool anyLight = false, haveDirect = false;
    for (const auto& child : children)
        if (auto* light = dynamic_cast<const UiSceneLight*>(child.get()))
        {
            anyLight = true;
            const glm::vec3 colour = light->Color * light->Intensity;
            if (light->LightType == UiSceneLight::Type::Ambient)
                lights.ambientColor += colour;
            else if (light->LightType == UiSceneLight::Type::Point)
            {
                LightManager::PointLightInfo info;
                info.position = light->Position;
                info.color = colour;
                info.radius = light->Range;
                info.direction = glm::vec3(0.f, 0.f, 1.f);
                info.innerConeAngleDegrees = 360.f;   // omni
                info.outerConeAngleDegrees = 360.f;
                points.push_back(info);
            }
            else if (!haveDirect)
            {
                haveDirect = true;
                lights.directColor = colour;
                lights.direction = light->Direction;
            }
        }
    if (!anyLight)
    {
        lights.ambientColor = glm::vec3(0.45f);
        lights.directColor = glm::vec3(1.1f);
    }

    const glm::mat4 view = ViewMatrix();
    const float aspect = static_cast<float>(target->Size.x) / static_cast<float>(std::max(1, target->Size.y));
    const glm::mat4 projection = glm::perspective(glm::radians(m_fov), aspect, 0.02f, 500.f);   // like Camera's projection

    target->Buffer->bind(0, 0, static_cast<uint16_t>(target->Size.x), static_cast<uint16_t>(target->Size.y));
    const bgfx::ViewId viewId = ViewIdManager::GetCurrentId();
    bgfx::setViewMode(viewId, bgfx::ViewMode::Sequential);
    const auto packed = [](float v) { return static_cast<uint32_t>(std::clamp(v, 0.f, 1.f) * 255.f + 0.5f); };
    const uint32_t clear = (packed(m_background.r) << 24) | (packed(m_background.g) << 16) | (packed(m_background.b) << 8) | packed(m_background.a);
    bgfx::setViewClear(viewId, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL, clear, 1.0f, 0);
    bgfx::setViewName(viewId, "UiSceneView");
    bgfx::touch(viewId);

    const auto savedState = BgfxStateManager::GetState();
    BgfxStateManager::Reset();
    BgfxStateManager::SetWriteDepth(true);
    BgfxStateManager::SetDepthTest(BgfxStateManager::DepthTest::LEqual);
    BgfxStateManager::SetCull(BgfxStateManager::Cull::CW);
    BgfxStateManager::SetMSAA(false);

    LightManager::SetPointLightOverride(&points);   // the level's point lights stay out of the picture
    for (const auto& child : children)
        if (auto* model = dynamic_cast<UiSceneModel*>(child.get()))
        {
            if (model->GetState() != UiSceneModel::State::Warming && model->GetState() != UiSceneModel::State::Ready)
                continue;
            UiSceneMesh* mesh = model->Mesh();
            mesh->Light = lights;
            model->PrepareFrame();
            std::vector<IDrawCommand*> commands;
            mesh->CollectDrawCommands(commands);
            for (IDrawCommand* command : commands)
                command->DrawForward(view, projection);
        }

    LightManager::SetPointLightOverride(nullptr);
    BgfxStateManager::SetState(savedState);
}

// ── registry

int UiSceneViews::TotalRenders() { return g_totalRenders; }

int UiSceneViews::ViewCount()
{
    std::lock_guard<std::recursive_mutex> lock(g_mutex);
    return static_cast<int>(g_views.size());
}

void UiSceneViews::RenderAll()
{
    std::lock_guard<std::recursive_mutex> lock(g_mutex);
    for (UiElemSceneView* view : g_views)
        view->Render();
    UiSceneTargets::EndFrame();
}

void UiSceneViews::RegisterTags()
{
    using UiTagRegistry::AttributeInfo;
    using UiTagRegistry::TagInfo;
    UiTagRegistry::RegisterBuiltin("SceneView", &UiElemSceneView::Create, TagInfo{ "SceneView", "layout",
        "A 3D scene as the content of an element: a camera and Model / Light children. Rendered into a target of the content size before the UI pass; it re-renders only when something changed",
        { AttributeInfo{ "camera-position", "text", "0 1 3", false, "Camera position 'x y z' (metres)" },
          AttributeInfo{ "camera-target", "text", "0 0 0", false, "The point the camera looks at 'x y z'" },
          AttributeInfo{ "fov", "number", "40", false, "Vertical field of view in degrees" },
          AttributeInfo{ "background", "text", "transparent", false, "transparent, or #rgb / #rrggbb / #rrggbbaa" },
          AttributeInfo{ "orbit", "bool", "false", false, "A pointer drag turns the camera around camera-target" } } });
    UiTagRegistry::RegisterBuiltin("Model", &UiSceneModel::Create, TagInfo{ "Model", "layout",
        "A 3D model inside a SceneView (.glb); takes no room in the layout",
        { AttributeInfo{ "src", "asset", "", false, "GameData-relative path of the model" },
          AttributeInfo{ "position", "text", "0 0 0", false, "'x y z' (metres)" },
          AttributeInfo{ "rotation", "text", "0 0 0", false, "Degrees about x, y, z" },
          AttributeInfo{ "scale", "text", "1", false, "One number, or 'x y z'" },
          AttributeInfo{ "animation", "text", "", false, "Name of an animation clip, played looped" },
          AttributeInfo{ "two-sided", "bool", "false", false, "Draws both sides of the faces" } } });
    UiTagRegistry::RegisterBuiltin("Light", &UiSceneLight::Create, TagInfo{ "Light", "layout",
        "A light inside a SceneView: the first directional (or point) light lights the models, ambient lights add up",
        { AttributeInfo{ "type", "choice", "directional", false, "directional | ambient | point" },
          AttributeInfo{ "direction", "text", "-0.3 -1 -0.5", false, "Direction the light travels 'x y z'" },
          AttributeInfo{ "position", "text", "0 0 0", false, "Position of a point light" },
          AttributeInfo{ "color", "text", "#ffffff", false, "#rrggbb" },
          AttributeInfo{ "intensity", "number", "1", false, "Multiplies the colour" },
          AttributeInfo{ "range", "number", "10", false, "Reach of a point light (m)" } } });
}
