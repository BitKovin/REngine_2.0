#include "UiElemWorldPanel.h"

#include <Camera.h>
#include <EngineMain.h>
#include <UI/Markup/UiTagRegistry.h>
#include <UI/UiSystem.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>

glm::vec2 UiWorld::Project(const glm::vec3& world, bool& behind, bool& onScreen)
{
    const glm::vec4 clip = Camera::projection * Camera::view * glm::vec4(world, 1.f);
    const glm::vec2 size = EngineMain::Viewport.GetSize();
    behind = clip.w <= 0.0001f;
    glm::vec2 ndc;
    if (behind)
    {
        // mirrored: the direction on screen is the opposite of the clip xy; pushed far out so a clamp puts it on the edge
        ndc = -glm::vec2(clip.x, clip.y);
        const float len = std::max(std::abs(ndc.x), std::abs(ndc.y));
        ndc = len > 0.0001f ? ndc / len * 1000.f : glm::vec2(0.f, -1000.f);
    }
    else
        ndc = glm::vec2(clip.x, clip.y) / clip.w;
    onScreen = !behind && std::abs(ndc.x) <= 1.f && std::abs(ndc.y) <= 1.f;
    return glm::vec2((ndc.x * 0.5f + 0.5f) * size.x, (1.f - (ndc.y * 0.5f + 0.5f)) * size.y);
}

glm::vec2 UiWorld::Project(const glm::vec3& world)
{
    bool behind, onScreen;
    return Project(world, behind, onScreen);
}

namespace
{
    bool ParseBool(const std::string& v) { return v.empty() || v == "true" || v == "1"; }

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

    UiTransformFn Translate(float x, float y)
    {
        UiTransformFn f;
        f.Type = UiTransformFnType::Translate;
        f.A.Kind = UiLenKind::Px; f.A.Px = x;
        f.B.Kind = UiLenKind::Px; f.B.Px = y;
        return f;
    }

    UiTransformFn Scale(float s)
    {
        UiTransformFn f;
        f.Type = UiTransformFnType::Scale;
        f.V0 = f.V1 = s;
        return f;
    }

    UiTransformFn Rotate(float degrees)
    {
        UiTransformFn f;
        f.Type = UiTransformFnType::Rotate;
        f.V0 = degrees;
        return f;
    }
}

bool UiElemWorldPanel::ApplyAttribute(const std::string& name, const std::string& value)
{
    float v[3];
    if (name == "world-position") { if (ParseFloats(value, v, 3)) { m_position = glm::vec3(v[0], v[1], v[2]); m_follow = false; } return true; }
    if (name == "world-offset") { if (ParseFloats(value, v, 3)) m_offset = glm::vec3(v[0], v[1], v[2]); return true; }
    if (name == "clamp-to-edge") { m_clampToEdge = ParseBool(value); return true; }
    if (name == "edge-margin") { m_edgeMargin = std::max(0.f, static_cast<float>(std::atof(value.c_str()))); return true; }
    if (name == "fade-distance") { m_fadeDistance = std::max(0.f, static_cast<float>(std::atof(value.c_str()))); return true; }
    if (name == "max-distance") { m_maxDistance = std::max(0.f, static_cast<float>(std::atof(value.c_str()))); return true; }
    if (name == "scale-with-distance") { if (ParseFloats(value, v, 3)) m_scaleWithDistance = glm::vec3(v[0], v[1], v[2]); return true; }
    return false;
}

void UiElemWorldPanel::Tick()
{
    UiElemPanel::Tick();

    glm::vec3 position = m_position;
    if (m_follow)
        if (Entity* entity = m_entity.Resolve())
            position = entity->Position;
    position += m_offset;

    bool behind = false, onScreen = false;
    glm::vec2 screen = UiWorld::Project(position, behind, onScreen);
    const glm::vec2 viewport = EngineMain::Viewport.GetSize();
    m_distance = glm::length(position - Camera::position);
    m_behind = behind;

    // hidden when out of reach, or off screen without clamp-to-edge
    bool hidden = (m_maxDistance > 0.f && m_distance > m_maxDistance) || (!m_clampToEdge && !onScreen);
    bool clamped = false;
    glm::vec2 arrowDir(0.f);
    if (m_clampToEdge && !onScreen)
    {
        const glm::vec2 lo(m_edgeMargin), hi = viewport - glm::vec2(m_edgeMargin);
        const glm::vec2 centre = viewport * 0.5f;
        arrowDir = screen - centre;
        // keep the direction: shrink the vector from the centre until it touches the margin rectangle
        const glm::vec2 half = (hi - lo) * 0.5f;
        const float sx = std::abs(arrowDir.x) > 0.001f ? half.x / std::abs(arrowDir.x) : 1e9f;
        const float sy = std::abs(arrowDir.y) > 0.001f ? half.y / std::abs(arrowDir.y) : 1e9f;
        const float s = std::min(1.f, std::min(sx, sy));
        screen = centre + arrowDir * s;
        clamped = s < 1.f;
    }
    m_screen = screen;
    m_clamped = clamped;

    // fade with distance: 1 up to 80% of fade-distance, then down to 0 at fade-distance
    float fade = 1.f;
    if (m_fadeDistance > 0.f)
        fade = std::clamp((m_fadeDistance - m_distance) / (m_fadeDistance * 0.2f), 0.f, 1.f);
    if (fade <= 0.f)
        hidden = true;

    float scale = 1.f;
    if (m_scaleWithDistance.y > m_scaleWithDistance.x)
    {
        const float t = std::clamp((m_distance - m_scaleWithDistance.x) / (m_scaleWithDistance.y - m_scaleWithDistance.x), 0.f, 1.f);
        scale = 1.f + (std::clamp(m_scaleWithDistance.z, 0.f, 1.f) - 1.f) * t;
    }

    // The flow position of the panel is kept; a translate moves its centre onto the projected point (layout never sees it).
    const UiElem* parentElem = AsParentElem();
    const glm::mat3 parentAbs = parentElem != nullptr ? parentElem->AbsoluteTransform : glm::mat3(1.f);
    const glm::vec2 layoutCentre = glm::vec2(parentAbs * glm::vec3(BorderRect.X + BorderRect.W * 0.5f, BorderRect.Y + BorderRect.H * 0.5f, 1.f));
    const glm::vec2 shift = screen - layoutCentre;

    if (glm::distance(shift, m_appliedShift) > 0.05f || scale != m_appliedScale)
    {
        m_appliedShift = shift;
        m_appliedScale = scale;
        SetTransformOverride({ Translate(shift.x, shift.y), Scale(scale) });
    }

    if (hidden != m_hidden || fade != m_fade)
    {
        m_hidden = hidden;
        m_fade = fade;
        Style().Visibility = hidden ? UiVisibility::Hidden : UiVisibility::Visible;
        Style().Opacity = fade;
        m_computed.Visibility = Style().Visibility;
        m_computed.Opacity = fade;
        MarkPaintDirty();
    }

    // the arrow child points toward the true position while the panel sits on the edge
    std::vector<UiElem*> all;
    CollectElems(all);
    for (UiElem* e : all)
    {
        if (e == this || !e->HasClass("arrow"))
            continue;
        const float angle = clamped ? std::atan2(arrowDir.x, -arrowDir.y) * 57.29577951f : 0.f;
        if (std::abs(angle - m_appliedArrow) > 0.05f)
        {
            m_appliedArrow = angle;
            e->SetTransformOverride({ Rotate(angle) });
        }
        e->Style().Visibility = clamped ? UiVisibility::Visible : UiVisibility::Hidden;
        e->m_computed.Visibility = e->Style().Visibility;
    }
}

void UiHud::RegisterTags()
{
    using UiTagRegistry::AttributeInfo;
    using UiTagRegistry::TagInfo;
    UiTagRegistry::RegisterBuiltin("WorldPanel", &UiElemWorldPanel::Create, TagInfo{ "WorldPanel", "layout",
        "Panel that follows a world position: its border box is centred on the projected point. A child with class .arrow points toward the position while the panel is clamped to the screen edge",
        { AttributeInfo{ "world-position", "text", "0 0 0", false, "World position 'x y z' (metres)" },
          AttributeInfo{ "world-offset", "text", "0 0 0", false, "Added to the world position, 'x y z'" },
          AttributeInfo{ "clamp-to-edge", "bool", "false", false, "Stays on the screen edge when the position is off screen or behind the camera" },
          AttributeInfo{ "edge-margin", "number", "32", false, "Distance from the edge when clamped (px)" },
          AttributeInfo{ "fade-distance", "number", "0", false, "Fully faded at this distance (m); 0 = off" },
          AttributeInfo{ "max-distance", "number", "0", false, "Hidden beyond this distance (m); 0 = off" },
          AttributeInfo{ "scale-with-distance", "text", "", false, "'near far minScale': scale 1 at near, minScale at far" } } });
}
