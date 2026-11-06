# Phase 8: 3D scene views inside UI

**Goal:** a **simple** way to show 3D objects inside UI: a character preview in an inventory, an item inspect view, a weapon on a loadout screen. One element, `<SceneView>`, with a camera and 3D children. Nothing else to learn.

**Prerequisites:** phases 5 and 7. README constraints 1–6 were followed in every earlier phase.

## The whole API

Markup:

```xml
<SceneView id="preview" class="preview"
           camera-position="0 1.6 3" camera-target="0 1 0" fov="40"
           background="transparent" orbit="true">
    <Model id="hero" src="GameData/models/hero.glb" position="0 0 0" rotation="0 30 0" scale="1" animation="idle"/>
    <Model src="GameData/models/sword.glb" position="0.3 1 0" rotation="0 0 90"/>
    <Light type="directional" direction="-0.3 -1 -0.5" color="#ffffff" intensity="1.2"/>
    <Light type="ambient" color="#404050"/>
</SceneView>
```

C++:

```cpp
auto* view = page->Find<UiElemSceneView>("preview");
view->SetCamera(glm::vec3(0, 1.6f, 3), glm::vec3(0, 1, 0), 40.f);   // position, target, fov (deg)
UiSceneModel* hero = view->Find<UiSceneModel>("hero");
hero->SetTransform(position, rotationDeg, scale);
hero->PlayAnimation("wave");
UiSceneModel* added = view->AddModel("GameData/models/helmet.glb");
view->RemoveModel(added);
```

Rules for the API:

- Every attribute can use variables: `rotation="0 {spin} 0"` works.
- `orbit="true"` makes a pointer drag (capture, phase 2) or the right stick (while focused) rotate the camera around `camera-target`.
- `background` is `transparent` or a colour.
- `SceneView` is a normal element in every other way: it is laid out, styled, clipped and transformed like a `Panel`, and draws its picture as its content.
- Its render-target size is its content rect in target pixels (`ContentRectScreen()`), rounded up to a multiple of 8.
- No other 3D tags exist: `Model`, `Light` and `SceneView` only.

## UIDB and SceneView preview descriptors

Extend the same `uidb.json` with. SceneView/Model/Light schemas and scene preview descriptors are published by their runtime registrations as they are implemented, with `UiDatabase::MarkDirty()`; do not add a Phase 8-specific schema file or serializer:

- `SceneView`, `Model` and `Light` tag schemas, including all attributes, defaults, accepted asset types and runtime capabilities;
- available preview model assets (GameData-relative paths), animation names where discoverable, supported light types and camera controls;
- scene-view preview target recipes with default camera, background, model list, transforms, lights, target dimensions and interaction mode;
- optional game-owned character/item preview providers for compositions that assemble models dynamically. Each provider supplies a safe default asset set and named scenarios; it must not spawn a gameplay entity or mutate the active level.

Studio displays the SceneView as part of the actual engine preview and exposes the declared camera/model/light fixture controls. It must not implement a Qt 3D viewport. If a referenced model or runtime provider is unavailable, show the source diagnostic and do not silently substitute an unrelated asset.

**Check:** UIDB lists the built-in `SceneView`/`Model`/`Light` schemas and a sample scene-view preview target. Open it by selecting only the game's GameData directory, then change camera/model/light parameters and verify the engine-rendered target updates.

## Task 8.0: investigation (STOP after it)

Read and write down in the notes:

1. **The renderer's entry point:**
   - how `Renderer::RenderLevel(Level*, FrameBufferHandle)` gathers draw commands (`FrameDrawCommandLists`)
   - how `StaticMesh` / `SkeletalMesh` produce them (`FinalizeFrameData`, `PreDraw`)
   - which code uses the static `Camera::view` / `projection` / `position`
2. Whether a mesh component can exist **without** being part of `Level::Current` (owner `Entity*` requirements), and how its asset loading works (`LoadFromFile`, `AssetLoadTier`).
3. How lighting finds lights for a mesh (the engine lights meshes from a light volume at the entity position).
4. Which thread builds draw commands today, given that rendering overlaps the game update.

Then propose, in the notes, the minimal engine entry point:

```cpp
// Renders a list of draw commands with an explicit camera into a target that has colour + depth.
void Renderer::RenderSceneView(const FrameDrawCommandLists& commands, const UiSceneCamera& camera,
                               const UiSceneLights& lights, bgfx::FrameBufferHandle target, glm::ivec2 size);
```

It must not read or write `Camera::*` statics. If it can't avoid them, it must save and restore them around the call, on the main thread, during `Render()`, before the UI pass views. **STOP** and get the owner's approval of the proposal before task 8.1.

## Task 8.1: scene-view targets

- `UiSceneTargets`: a pool of render targets with **colour (RGBA8) + depth (D24S8)**. This is separate from the phase 7 layer atlas (README constraint 5).
- Keyed by size, reused when the same view keeps its size, freed after 3 s unused.
- MSAA off on Emscripten and Android (as the main renderer does). Elsewhere use `MultiSampleCount` from the renderer, resolved before UI sampling.

## Task 8.2: elements

- **`UiElemSceneView : UiElemPanel`:** owns a list of `UiSceneModel` and `UiSceneLight`. The camera comes from attributes or `SetCamera`.
- **`UiSceneModel`:** a `UiElem` subclass with `TagName "Model"`. It doesn't paint or lay out itself (collapsed in layout). It owns the mesh component (`StaticMesh` or `SkeletalMesh`, chosen by what the asset is; follow what the engine does for `.glb`) through whatever owner the investigation found works without a level. Transform attributes: `position`, `rotation` (degrees XYZ), `scale` (1 or 3 numbers). `animation` plays by name, looped.
- **`UiSceneLight`:** `type` (`directional` \| `ambient` \| `point`), `direction`, `position`, `color`, `intensity`, `range`.
- **Update (main thread, R1):**
  - in `Tick()`: update the transforms, animations (the real-time delta) and the orbit
  - after layout: request a target of the content size
- **Render:** at the start of `EngineMain::Render()`, before the UI pass:
  - for each visible `SceneView` with a non-zero target: build draw commands from its models (main thread) and call `RenderSceneView`
  - the resulting texture handle is stored on the element
- **Paint:** the element's `PaintContent` draws `Image(content, targetTexture, uv, tint)` (README constraint 2). On OpenGL the target may need a V flip. Decide that once, in `RenderSceneView`'s output description (`bool FlipV`), never in UI code.
- **Cost:**
  - a view whose models, camera, lights and size didn't change, with no animation playing, doesn't re-render (keep the previous texture)
  - offscreen or collapsed views don't render

## Tests (StealthAction, all backends)

1. **`p8_scene_basic`:** a `SceneView` 400×400 with a cube model (pick a small model already in `GameData/models/engine/`; list the candidates in the notes) and a directional light.
   - the centre pixel isn't the background colour
   - the corner pixel is
   - the same on every backend within tolerance 2 (`cross`)
   - if the 3D renderer differs between backends **before** the UI is involved, record that as pre-existing, the same way as for legacy goldens
2. **`p8_scene_vars`:** `rotation="0 {spin} 0"`; `set_var` → the picture changes (diff between two captures ≠ 0); no change → `ui_stats` shows 0 scene re-renders.
3. **`p8_orbit`:** `mouse_move`/`mouse_down`/`mouse_move`/`mouse_up` over the view rotates the camera (the picture changes); releasing outside the view still ends the drag (capture).
4. Inside a world surface: a `SceneView` in a terminal's content renders. Nested scene views inside scene views aren't supported, and give a warning.

## Implementation notes

(implementer writes here)
