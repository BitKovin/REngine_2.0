#pragma once

#include <UI/Elements/UiElemPanel.h>

#include <glm/glm.hpp>

#include <memory>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------------------------------------------------------------
// 3D scene views inside UI (phase 8). Three tags, nothing else to learn:
//
//   <SceneView id="preview" camera-position="0 1.6 3" camera-target="0 1 0" fov="40" background="transparent" orbit="true">
//       <Model id="hero" src="GameData/models/hero.glb" position="0 0 0" rotation="0 30 0" scale="1" animation="idle"/>
//       <Light type="directional" direction="-0.3 -1 -0.5" color="#ffffff" intensity="1.2"/>
//       <Light type="ambient" color="#404050"/>
//   </SceneView>
//
//   view->SetCamera(position, target, fovDegrees);
//   view->Find<UiSceneModel>("hero")->SetTransform(position, rotationDegrees, scale);
//   view->AddModel("GameData/models/helmet.glb");
//
// A SceneView is a normal element (laid out, styled, clipped, transformed like a Panel); its picture is its content. The picture is
// rendered before the UI pass into a render target of the content size (rounded up to a multiple of 8). A view whose models, camera,
// lights and size did not change, with no animation playing, keeps its picture and costs nothing.
// ---------------------------------------------------------------------------------------------------------------------------------

class UiSceneMesh;
class UiElemSceneView;

class UiSceneModel : public UiElem
{
public:
    UiSceneModel();
    ~UiSceneModel() override;
    static std::shared_ptr<UiElem> Create() { return std::make_shared<UiSceneModel>(); }

    bool ApplyAttribute(const std::string& name, const std::string& value) override;
    void Tick() override;

    void SetSource(const std::string& path);
    void SetTransform(const glm::vec3& position, const glm::vec3& rotationDegrees, const glm::vec3& scale);
    void PlayAnimation(const std::string& name);   // looped; empty stops
    const std::string& Source() const { return m_src; }

    enum class State { None, Loading, Warming, Ready };
    State GetState() const { return m_state; }
    bool Animating() const { return m_animating && m_state != State::None; }
    UiSceneMesh* Mesh() const { return m_mesh.get(); }
    void PrepareFrame();   // main thread, before drawing: pose, finalize

private:
    friend class UiElemSceneView;
    void ApplyTransform();
    UiElemSceneView* View() const;
    void NotifyView();

    std::string m_src, m_animation;
    glm::vec3 m_position = glm::vec3(0.f), m_rotation = glm::vec3(0.f), m_scale = glm::vec3(1.f);
    bool m_twoSided = false;
    std::unique_ptr<UiSceneMesh> m_mesh;
    State m_state = State::None;
    float m_timer = 0.f;
    bool m_animating = false;
    float m_animTime = 0.f;
    bool m_transformDirty = true;
    bool m_posed = false;
};

class UiSceneLight : public UiElem
{
public:
    UiSceneLight();
    static std::shared_ptr<UiElem> Create() { return std::make_shared<UiSceneLight>(); }

    bool ApplyAttribute(const std::string& name, const std::string& value) override;

    enum class Type { Directional, Ambient, Point };
    Type LightType = Type::Directional;
    glm::vec3 Direction = glm::normalize(glm::vec3(-0.3f, -1.f, -0.5f));
    glm::vec3 Position = glm::vec3(0.f);
    glm::vec3 Color = glm::vec3(1.f);
    float Intensity = 1.f;
    float Range = 10.f;

private:
    void NotifyView();
};

class UiElemSceneView : public UiElemPanel
{
public:
    UiElemSceneView();
    ~UiElemSceneView() override;
    static std::shared_ptr<UiElem> Create() { return std::make_shared<UiElemSceneView>(); }

    bool ApplyAttribute(const std::string& name, const std::string& value) override;
    void Tick() override;
    void PaintContent(UiPainter& painter, const UiRect& contentRect) override;
    void OnPointerDown(glm::vec2 localPoint) override;
    void OnPointerMove(glm::vec2 localPoint) override;
    void OnPointerUp(glm::vec2 localPoint) override;

    void SetCamera(const glm::vec3& position, const glm::vec3& target, float fovDegrees);
    UiSceneModel* AddModel(const std::string& path);
    void RemoveModel(UiSceneModel* model);
    void MarkSceneDirty() { m_dirty = true; }
    void Render();   // main thread, before the world pass (UiSceneViews::RenderAll)

    // For tests and ui_stats
    glm::ivec2 TargetSize() const { return m_targetSize; }
    int RenderCount() const { return m_renders; }
    float OrbitYaw() const { return m_yaw; }
    bool Dragging() const { return m_dragging; }

private:
    bool NeedsRender() const;
    glm::mat4 ViewMatrix() const;

    glm::vec3 m_cameraPosition = glm::vec3(0.f, 1.f, 3.f), m_cameraTarget = glm::vec3(0.f, 0.f, 0.f);
    float m_fov = 40.f;
    glm::vec4 m_background = glm::vec4(0.f);
    bool m_orbit = false;
    float m_yaw = 0.f, m_pitch = 0.f;   // orbit, degrees
    bool m_dragging = false;
    glm::vec2 m_lastPointer = glm::vec2(0.f);

    glm::ivec2 m_wantSize = glm::ivec2(0);
    glm::ivec2 m_targetSize = glm::ivec2(0);
    unsigned m_handle = 0xffffffffu;   // bgfx texture index of the target; the recorded paint references it
    bool m_dirty = true;
    int m_renders = 0;
    bool m_visibleNow = false;
};

namespace UiSceneViews
{
    void RegisterTags();   // SceneView, Model, Light (from UiSystem::Init)
    void RenderAll();      // EngineMain::Render, after the frame's UI update and before the world pass
    int TotalRenders();    // re-renders since the start
    int ViewCount();
}
