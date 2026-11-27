#pragma once
#include <UI/Elements/UiElemPanel.h>
#include <EntityHandle.h>

#include <glm/glm.hpp>

namespace UiWorld
{
    // Projects a world position with the camera the renderer uses this frame (Camera::projection * Camera::view). The result is in UI
    // units (EngineMain::Viewport.GetSize()), y down. `behind` is true when the point is behind the camera (the result is then the
    // mirrored direction, far outside the screen). `onScreen` is true when the point is in front and inside the viewport.
    glm::vec2 Project(const glm::vec3& world, bool& behind, bool& onScreen);
    glm::vec2 Project(const glm::vec3& world);
}

// A panel that follows a world position: its border box is centred on the projected point. The panel's own flow position is ignored.
//   world-position="x y z"   world-offset="x y z"   clamp-to-edge  edge-margin  fade-distance  max-distance  scale-with-distance="near far minScale"
// A child with the class `arrow` is rotated toward the true position while the panel is clamped to the edge (0 deg = up).
class UiElemWorldPanel : public UiElemPanel
{
public:
    UiElemWorldPanel() { TagName = "WorldPanel"; }
    static std::shared_ptr<UiElem> Create() { return std::make_shared<UiElemWorldPanel>(); }

    bool ApplyAttribute(const std::string& name, const std::string& value) override;
    void Tick() override;

    void SetWorldPosition(const glm::vec3& position) { m_position = position; m_follow = false; }
    void Follow(const EntityHandle& entity) { m_entity = entity; m_follow = true; }   // the entity's position, read each Tick
    void SetWorldOffset(const glm::vec3& offset) { m_offset = offset; }

    // What the last Tick computed (tests, game code)
    glm::vec2 ScreenPosition() const { return m_screen; }   // the point the panel is centred on, UI units
    bool IsClamped() const { return m_clamped; }
    bool IsBehind() const { return m_behind; }
    float Distance() const { return m_distance; }

private:
    glm::vec3 m_position = glm::vec3(0.f), m_offset = glm::vec3(0.f);
    EntityHandle m_entity;
    bool m_follow = false;
    bool m_clampToEdge = false;
    float m_edgeMargin = 32.f;
    float m_fadeDistance = 0.f, m_maxDistance = 0.f;
    glm::vec3 m_scaleWithDistance = glm::vec3(0.f);   // near, far, minScale (near == far == 0: off)

    glm::vec2 m_screen = glm::vec2(0.f);
    bool m_clamped = false, m_behind = false, m_hidden = false;
    float m_distance = 0.f, m_fade = 1.f;
    glm::vec2 m_appliedShift = glm::vec2(1e9f);
    float m_appliedScale = -1.f, m_appliedArrow = 1e9f;
};

namespace UiHud
{
    void RegisterTags();   // WorldPanel (called from UiSystem::Init)
}
