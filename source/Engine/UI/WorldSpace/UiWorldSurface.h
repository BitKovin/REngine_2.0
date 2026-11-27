#pragma once

#include "../../StaticMesh.h"
#include "../Elements/UiElem.h"
#include "../UiElement.h"
#include "UiBillboardAtlas.h"

#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <glm/glm.hpp>

// ---------------------------------------------------------------------------------------------------------------------------------
// UiWorldSurface: a piece of UI on a quad in the world (a terminal, a screen, a sign, a status widget). It is a StaticMesh whose
// texture is a region of the shared billboard atlas.
//
//   auto* screen = new UiWorldSurface(this);
//   screen->Position = ...;
//   screen->SetContent("GameData/ui/terminal.xml");     // markup page; the CSS width / height of its root is the size in pixels
//   screen->On("Hit", [] { ... });                      // handlers of onclick="Hit()" and the like
//   Drawables.push_back(screen);
//
// With the default InteractionMode (Aim) there is nothing else to write: the player aims at the surface, the pointer follows, and
// the `interact` action is the click. For anything else (a cursor of your own, a test, a scripted demo) it takes two calls:
//
//   screen->Hover(x, y);     // the pointer is at this pixel of the surface; call it every frame the pointer is there
//   screen->Click();         // a click at the hovered pixel
//
// Each surface has its own pointer, hover, press, capture and focus: nothing leaks into the main UI or into other surfaces.
// ---------------------------------------------------------------------------------------------------------------------------------

// The private viewport of one surface: a fixed size, no screen dependence.
class UiSurfaceViewport : public UiElement
{
public:
    void SetPixelSize(glm::vec2 s) { m_pixels = s; }
    void Update() override
    {
        size = m_pixels;
        parentTopLeft = glm::vec2(0.f);
        parentBottomRight = size;
        UiElement::Update();
    }
    glm::vec2 GetSize() override { return m_pixels; }

private:
    glm::vec2 m_pixels = glm::vec2(512.f);
};

class UiWorldSurface : public StaticMesh
{
public:
    explicit UiWorldSurface(Entity* owner);
    ~UiWorldSurface() override;

    // ── content
    void SetContent(const std::string& layoutPath);              // a markup page: the bottom page of this surface
    void SetContent(std::shared_ptr<UiElement> element);         // any element, markup or legacy
    void ClearContent();
    UiElem* Content() const;                                     // the markup root, or null
    void On(const std::string& handler, std::function<void()> fn);   // handler of onclick="handler()" ... (also when set before the content)
    void SetVar(const std::string& name, const std::string& value);
    void SetVar(const std::string& name, double value);

    // ── interaction, the simple API
    void Hover(float x, float y);                // pointer at a pixel of the surface, (0,0) = top-left; it stays there until Unhover()
    void HoverUV(float u, float v);              // the same in 0..1 (u right, v down)
    void HoverRay(const glm::vec3& origin, const glm::vec3& direction);   // pointer where a world ray meets the surface
    void Unhover();                              // pointer left: aiming works again
    void Press();
    void Release();
    void Click();                                // press, then release on the next frame

    // ── settings (also read from the attributes of the content root: interaction, facing, pixels-per-meter, max-distance, emissive,
    // update-rate-distance, freeze-distance)
    enum class Facing { Fixed, Billboard, BillboardY };
    enum class Interaction { None, Aim, Focus, Both };
    Facing FacingMode = Facing::Fixed;
    Interaction InteractionMode = Interaction::Aim;
    glm::ivec2 ViewportSize = glm::ivec2(512, 512);
    float PixelsPerMeter = 512.f;
    float MaxDistance = 3.f;                     // aim and focus reach; beyond it the surface is :out-of-range up to twice this
    float MaxAngleDeg = 70.f;                    // largest angle between the aim ray and the surface normal
    float UpdateRateDistance = 15.f;             // beyond this the surface updates at 10 Hz
    float FreezeDistance = 40.f;                 // beyond this it does not update or render
    bool ConsumesUse = true;                     // the `interact` press that clicked is taken away from the game

    // ── gameplay hooks
    std::function<void()> OnAimEnter, OnAimLeave, OnFocusEnter, OnFocusExit;
    virtual void Tick() {}                       // main thread, before the UI of this surface is processed
    static UiWorldSurface* Aimed();              // the surface under the aim ray this frame (main thread)
    static bool IsAimingAtInteractive();         // the aimed surface has a hit-testable element under the pointer
    static UiWorldSurface* Focused();
    void EnterFocus();                           // ui_* actions and the mouse go to this surface; the cursor is released
    void ExitFocus();
    bool IsFocused() const;
    bool IsAimed() const { return Aimed() == this; }
    bool PointerOverInteractive() const { return m_overInteractive; }
    glm::mat4 FocusCameraPose() const;           // where a camera looks at the whole surface; the engine never moves the camera
    glm::vec2 PointerPixel() const { return m_pointer; }   // the pixel the pointer is on (valid while HasPointer)
    bool HasPointer() const { return m_hasPointer; }

    // ── mesh
    void FinalizeFrameData() override;
    void PreDraw() override;

    // ── statistics (ui_stats, tests)
    static int RenderCount();                    // surface re-renders since the start
    static int SurfaceCount();

    UiSurfaceViewport& SurfaceRoot() { return m_viewport; }

protected:
    // The element tree the surface draws and updates. A markup surface: its own viewport. UiBilboard: its legacy canvas.
    virtual UiElement& ContentRoot() { return m_viewport; }
    virtual bool AlwaysRender() const { return false; }   // true: the content is legacy and has no snapshot to compare
    virtual void UpdateLegacyContent() {}

private:
    friend struct UiWorldSurfaces;
    struct AimHit { bool Valid = false; glm::vec2 Pixel; float Distance = 0.f; float Cos = 1.f; glm::vec3 World; };
    bool Intersect(const glm::vec3& origin, const glm::vec3& dir, AimHit& out) const;
    glm::vec3 Normal() const;
    glm::vec3 Centre() const { return Position; }
    void ProcessUi(const AimHit* aimHit, bool inRange, bool outOfRange, bool aimed);
    void ReadContentAttributes();
    void ApplyPendingHandlers();
    void DrawUi();
    void EnsureAtlasAllocation();
    bool NeedsRender() const;
    void RememberRendered();
    void SetOutOfRange(bool out);

    UiSurfaceViewport m_viewport;
    std::shared_ptr<UiElem> m_content;
    std::vector<std::pair<std::string, std::function<void()>>> m_handlers;
    std::vector<std::pair<std::string, std::string>> m_pendingVars;
    BillboardAllocation m_allocation;
    std::vector<const void*> m_renderedFrames;
    glm::ivec2 m_renderedSize = glm::ivec2(0);
    bool m_forceRender = true;

    // pointer state (written by game code on any thread, read in the surface update)
    std::mutex m_mutex;
    glm::vec2 m_manualPixel = glm::vec2(0.f);
    bool m_manualActive = false;                  // Hover / HoverRay was called and Unhover was not
    std::vector<bool> m_buttonQueue;              // true = press, false = release
    bool m_manualRay = false;
    glm::vec3 m_rayOrigin = glm::vec3(0.f), m_rayDir = glm::vec3(0.f);

    // pointer state, main thread
    int m_touchId = 0;
    glm::vec2 m_pointer = glm::vec2(0.f), m_lastPointer = glm::vec2(0.f);
    bool m_hasPointer = false, m_pressed = false, m_overInteractive = false, m_outOfRange = false;
    bool m_aimedLastFrame = false;
    glm::vec3 m_rayDirLast = glm::vec3(0.f);
    float m_accumulated = 0.f;                    // time since the last throttled update
    float m_distance = 0.f;
    bool m_savedLockCursor = true;
    bool m_savedNavInit = false;
    bool m_inFocus = false;
    bool m_visibleToCamera = true;
    std::shared_ptr<void> m_alive;                // guards handlers installed from this surface
    struct NavHolder;                             // UiNavigation::State of this surface
    std::unique_ptr<NavHolder> m_nav;
};

// Per-frame hooks, called from UiSystem::Update / Finalize (main thread).
struct UiWorldSurfaces
{
    static void Update();
    static void Finalize();
    static bool IsSurfaceViewport(const UiElement* viewport);
    static bool AnyFocused();                    // main UI input is paused while a surface has the focus
};
