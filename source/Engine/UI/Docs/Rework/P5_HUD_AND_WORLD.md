# Phase 5: HUD layers, contexts, world surfaces, loading screens

**Goal:**

- HUD and overlay layers that host any element
- world-anchored HUD panels
- tooltips
- a per-host `UiContext`, so world surfaces get their own focus, hover, capture and stack
- `UiWorldSurface`, the in-world "screen" entity that draws any element and lets gameplay interact with it
- `UiBilboard` rebuilt on it
- markup loading screens

**Prerequisites:** phases 2 and 3. Reread conventions §2. World surfaces are the place where threading mistakes are easiest to make.

## Task 5.1: layers

- `UiLayers::Hud()`, `UiLayers::Menus()` (= `UiMenus::Stack()`) and `UiLayers::Overlay()` are created in `UiSystem::Init` as full-viewport `Panel` roots. Add them to `EngineMain::Viewport` in that order.
- The overlay root sets `useLateDraw = true`, so it draws after everything, including legacy elements added later.
- `hides-hud` (phase 2's bool) now toggles the class `hud-hidden` on the HUD root. The kit stylesheet gives `.hud-hidden { opacity: 0; transition: opacity 0.2s; }`.
- `Tick()` already runs for every element of every root (phase 1). HUD code-behind uses it.

## Task 5.2: `WorldPanel`

`UiElemWorldPanel : UiElemPanel`:

- **Attributes:**
  - `world-position="x y z"`
  - `world-offset="x y z"`
  - `clamp-to-edge` (bool, with an `edge-margin` px)
  - `fade-distance` (m, 0 = off)
  - `max-distance` (m)
  - `scale-with-distance="near far minScale"`
- **C++:** `Follow(EntityHandle)` reads the entity position each `Tick()` (main thread, R1). `SetWorldPosition(vec3)`.
- **Projection** (in `Tick()`, before layout):
  - `clip = Camera::projection * Camera::view * vec4(p, 1)`
  - when `clip.w <= 0` (behind the camera): hidden unless `clamp-to-edge`, in which case it is mirrored and clamped
  - UI position = `((ndc.x × 0.5 + 0.5) × uiW, (1 − (ndc.y × 0.5 + 0.5)) × uiH)`, where `uiW` and `uiH` = `EngineMain::Viewport.GetSize()`
  - **Check the matrix order** against how the renderer multiplies (`UiElement::WorldToScreenSpace` uses `view * projection`, which looks wrong and returns `vec2()` anyway). Verify with a test: an entity straight ahead projects to the screen centre. Fix `UiElement::WorldToScreenSpace` to call the same correct code, since it's broken today.
- The panel's own layout is ignored for positioning. The panel's border box is centred on the projected point, then `x` / `y` offsets are applied.
- **Clamp:** keep the centre inside `[margin, size − margin]`. A child with class `arrow` gets the CSS `transform` rotation pointing toward the true position (set through `Style()`).
- **Fade:** opacity × `clamp((fade-distance − d) / (fade-distance × 0.2), 0, 1)`. Beyond `max-distance` → collapsed.

## Task 5.3: tooltips

- Attribute `tooltip="${KEY}"` or `tooltip-layout="path.xml"`.
- After 0.5 s of hover, or 0.8 s of gamepad focus, build the tooltip in the overlay (`class tooltip`), positioned below-right of the pointer (or below the focused element), kept inside the viewport.
- It is removed when hover or focus leaves. One tooltip at a time.

## Task 5.4: `UiContext` and the navigator refactor

1. Create `class UiNavigator` with **instance** state: `Focused`, `MouseNavigation`, root, and the logic of every function of today's `UiNavigation`, moved verbatim.
2. `UiNavigation`'s static API stays and forwards to `UiNavigator::Main()`. `UiNavigation::Focused` becomes a reference-returning accessor, or keeps a static mirrored every frame. Pick whichever needs no changes at call sites. **Grep every use of `UiNavigation::Focused` first** (games included). If any call site would need to change, **STOP**.
3. `struct UiContext { UiNavigator Nav; UiMenuStack* Stack; std::vector<std::weak_ptr<UiElem>> Roots; capture map; hover; std::string RootClasses; }`.
   - The main context = the existing navigation, `UiMenus::Stack()` and the main roots.
   - Each world surface owns one. The loading screen system owns one.
4. Every `UiElem` lookup of "the navigator", "the stack" or "the captured element" goes through its root's context. `UiSystem` maps root → context.

## Task 5.5: `UiWorldSurface`

```cpp
class UiWorldSurface : public StaticMesh                       // like UiBilboard (same mesh/material setup)
{
public:
    explicit UiWorldSurface(Entity* owner);
    void SetContent(const std::string& layoutPath);            // bottom page of this surface's stack
    void SetContent(std::shared_ptr<UiElement> element);       // any element, markup or legacy
    void On(const std::string& handler, std::function<void()> fn);   // host-level handlers (resolved after content)
    virtual void Tick() {}                                     // main thread, before UI update (R1)
    // settings (also readable from content root attributes: interaction, facing, pixels-per-meter, max-distance)
    enum class Facing { Fixed, Billboard, BillboardY } FacingMode = Facing::Fixed;
    enum class Interaction { None, Aim, Focus, Both } InteractionMode = Interaction::Aim;   // D6
    float PixelsPerMeter = 512.f, MaxDistance = 3.f, MaxAngleDeg = 70.f;
    bool ConsumesUse = true;
    // target: own plane (default) or another mesh's material slot + an interaction plane in owner space
    void ShowOnMesh(StaticMesh* mesh, int materialIndex, const glm::mat4& interactionPlane);
    // gameplay hooks
    std::function<void()> OnAimEnter, OnAimLeave, OnFocusEnter, OnFocusExit;
    static UiWorldSurface* Aimed();            // surface under the aim ray this frame (main thread)
    static bool IsAimingAtInteractive();       // aimed surface has a hit-testable element under the pointer
    static UiWorldSurface* Focused();
    void EnterFocus();  void ExitFocus();
    glm::mat4 FocusCameraPose() const;         // the game's camera code blends to this; the engine never moves the camera
    void InjectPointer(const glm::vec3& origin, const glm::vec3& dir, bool pressed);
    UiContext& Context();
};
```

### Rules

- **Threading:**
  - the constructor registers the surface with `UiSystem` directly (game thread allowed, R3); game code may also call `SetContent`, `SetVar` and the rest of the API on the game thread
  - processing runs in `UiSystem::Update` (input, `Tick`, style, layout), and recording plus the snapshot copy in `UiSystem::Finalize`. None of it runs in the component's `Update()`
  - `PreDraw` (render) only submits the surface's **snapshot** (`UiFrameData`, R2) into the atlas region, and only when the snapshot changed since it was last submitted
  - destruction: read how `Level::RemoveEntity` defers deletion. If a component can be deleted while `Render()` runs, **STOP**. Otherwise unregister in the destructor through the queue, and make `PreDraw` the last thing to touch the list.
- **Resolution:** the content root's border box (CSS `width` / `height`) is the pixel size. The atlas region is allocated with `UiBillboardAtlasManager` (existing). The plane is scaled to `size / PixelsPerMeter` metres.
- **Rendering:** reuse `UiBilboard::DrawUi`'s view setup, but submit the surface's recorded `UiDrawList` with `UiGpu::Submit`.
  - Mipmaps: try `BGFX_RESOLVE_AUTO_GEN_MIPS` on the atlas framebuffer. If the platform has no auto mip generation, keep no mips and note it.
  - Emissive: material parameter `emissive` (content root attribute).
- **Aim interaction:**
  - each `UiSystem::Update`, ray from `Camera::position` along `Camera::Forward()`, or the mouse ray when `Input::LockCursor` is false
  - intersect the surface's interaction plane (the quad); reject if beyond `MaxDistance`, or if the angle to the normal exceeds `MaxAngleDeg`
  - occlusion: `Physics::LineTrace(origin, hit)` hitting something other than the owner entity before the hit point means not aimed
  - the nearest aimed surface wins
  - the hit UV → a pointer position in surface pixels → hover, press and capture in the surface context (same code path as `InjectPointer`)
  - the `interact` action → press/release; when `ConsumesUse`, `CleanPressed()` it
  - the `:out-of-range` state (add `kStateOutOfRange` and the pseudo `out-of-range`) when within 2 × `MaxDistance` but beyond `MaxDistance`
- **Focus mode:**
  - `EnterFocus()` routes `ui_*` actions and the mouse (projected onto the plane) to this surface's context, sets `Input::LockCursor = false` and calls `OnFocusEnter`
  - back on an empty surface stack → `ExitFocus()` (restores `LockCursor`)
  - `pauses-game` on the content root → `UiMenus::OnPauseChanged`, as for menus
- **Cost:**
  - skip update and render when the bounds aren't in the camera frustum (`Frustum::IsBoxVisible`; build the frustum the way the renderer does, find it and reuse it)
  - beyond 15 m update at 10 Hz; beyond 40 m freeze (attributes `update-rate-distance`, `freeze-distance`)
  - re-render only when the recorded list changed (compare a hash of vertex and prim bytes)
- **`UiBilboard`:** becomes `class UiBilboard : public UiWorldSurface` with `FacingMode = Billboard`, `InteractionMode = None`, and its `Canvas` / `ContentBox` public fields kept. `Canvas` is set as content. The legacy `Canvas.Update()` call moves from `UiBilboard::Update` (game thread) into the surface update on the main thread. That also fixes the existing race. `NpcHumanBase` and `VideoInWorldTest` must compile and behave unchanged.

## Task 5.6: loading screens

- `LoadingScreenSystem::SetLoadingScreen(const std::string& layoutPath)` creates a root in its **own** context, under its own `UiViewport viewport` (already there).
- `LoadingScreenSystem::Update(progress)`:
  1. `root->SetVar("progress", progress)`
  2. `UiSystem::UpdateContext(loadingContext, realDt)`: the same per-root work as the main update, for that context's roots only; `realDt` is measured with `SDL_GetTicks64` between calls
  3. `Draw()` as today: `BeginPass`, `viewport.Draw()`, `EndPass`, `EndFrame`, `OnBeforeBgfxFrame`, `bgfx::frame`
- The C++ `UiLoadingScreenBase` path keeps working.

## Tests

1. **`p5_layers`:** a HUD element under a pushed `hides-hud` page gets `hud-hidden`. An overlay element draws above a legacy element added later (probe).
2. **`p5_worldpanel`:** in a test level, place an entity 5 m in front of the camera (test command `spawn_marker x y z`). The `WorldPanel` following it is centred (`expect_rect` within 2 px of the viewport centre). Move it behind the camera with `clamp-to-edge` → it's on the screen edge.
3. **`p5_surface`:**
   - a test entity type `ui_test_terminal` (engine test code under `UI/Test/`), registered so the runner can spawn it 1.5 m ahead
   - the content has a button with `onactivate="Hit()"`, and the surface handles `Hit` with a counter
   - aim + `action interact` → counter == 1
   - `Aimed()` is set; `IsAimingAtInteractive()` is true while over the button and false over empty space
   - focus mode: `EnterFocus`, `ui_down`, `ui_confirm` → the second button activates; `ui_cancel` exits focus
   - 20 idle terminals: `ui_stats` shows 0 surface re-renders on frames where nothing changed
4. **`p5_bilboard`:** ShooterGame and StealthAction NPC billboards render as before. Run their phase 0 golden captures that include billboards, if any, or take a new baseline from before this phase and compare.
5. **`p5_loading`:** a test level-load script (`console level.load <small map>`) with a markup loading screen. The progress label text changes between at least two screenshots taken inside the load. The runner can't run during a blocking load, so have `LoadingScreenSystem::Update` take `ui_screenshot`s itself when `-ui_test_loading_shots` is set, at progress 0, 0.5 and 1.

**Phase done when:** all tests pass on every backend, with `cross` `EQUAL` for captures, and all earlier regressions pass.

## Implementation notes

(implementer writes here)
