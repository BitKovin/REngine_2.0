# Phase 7: layers, multi-pass effects, extras

**Goal:**

- an offscreen **layer atlas** with multi-pass effect shaders: `filter: blur()`, `drop-shadow()`, glow, true group opacity (`isolation: isolate` and automatically during page transitions), scene-mode `backdrop-filter`, and `cache: layer`
- SVG icons
- drag and drop, and context menus
- the Studio "pick in game" link

**Prerequisites:** phases 3 and 5.

## Task 7.1: layer atlas

- `UiLayerAtlas`: RGBA8 colour-only pages (4096², up to 4).
  - Generalise the free-rect allocator from `UiBilboardAtlas` into a shared `UiRectAllocator`, and make `UiBilboardAtlas` use it too. No duplicate packer.
  - README constraint 5: this atlas is **2D colour only**. Phase 8 scene views have their own pool.
- `UiPainter::PushLayer(const UiRect& bounds, const LayerParams&)` / `PopLayer()` record `PushLayer` / `PopLayer` items into the draw list.
- **Compile step** (`UiGpu::Submit`):
  - each layer's commands are rendered **first**, into its atlas region, in its own bgfx view
  - views are allocated with `ViewIdManager` before the pass view, so they get lower ids
  - nested layers render innermost first
  - then the layer is composited into the parent pass as one quad (`Image` kind, premultiplied), with opacity, blend and effect
- Regions are re-rendered only when the layer's recorded command hash changes (`cache: layer`), or every frame otherwise.
- Unused regions are freed after 3 s. An over-budget request logs once and draws the content without the layer, so no effect is applied.

## Task 7.2: multi-pass effect shaders

Dedicated programs in `ui2/`, all full-screen-quad passes over a layer region (clamped to the region):

- `fs_ui2_blur.sc`: separable Gaussian, 9 taps per direction with linear-sampling offsets
  - two passes (horizontal, then vertical) between two scratch regions
  - for σ > 4 px, downsample by 2 (`fs_ui2_down.sc`, a 4-tap box) before blurring, as often as needed until σ_eff ≤ 4, then upsample bilinearly at composite
- `fs_ui2_dropshadow.sc`: takes the blurred alpha of the layer, tints it with the shadow colour, and offsets it. Then composite the shadow, then the layer.
- Glow is drop-shadow with offset 0. It is exposed as `filter: drop-shadow(0 0 R color)`; add no other syntax.

CSS that triggers a layer:

- `filter` containing `blur()` or `drop-shadow()`
- `isolation: isolate`
- `opacity < 1` together with `isolation: isolate`
- `cache: layer`
- `backdrop-filter` (task 7.3)
- pages during `:entering` / `:leaving` (automatic group opacity for page fades)

Layer bounds = the element's border box, inflated by the effect reach (3σ + |offset|).

## Task 7.3: backdrop (scene mode)

1. Reorder `EngineMain::Render()` so the UI pass runs **after** `RenderLevel` and before the composite, by moving the block. View order follows creation order (README constraint 6). Re-run all goldens afterwards.
2. When any visible element has `backdrop-filter`, build a blur chain of the **scene colour** once per frame: downsample ½, ¼, ⅛, then blur each with the blur program.
3. `backdrop-filter: blur(R)` picks the level whose σ is closest. The element's box draws that texture first, mapped by screen position and clipped by its rounded shape, then applies the colour-matrix part of `backdrop-filter` (brightness, saturate…).
4. Full mode (blurring UI below) isn't supported. Log a warning if an element asks for it through an unsupported value.

## Task 7.4: SVG icons

- Ask the owner, then vendor `nanosvg.h` and `nanosvgrast.h` (memononen/nanosvg, master at a pinned commit; record the hash) into `includedLibraries/nanosvg/`.
- `UiImageAtlas::Get(path, displaySizePx)`: for `.svg`, rasterise at the next power-of-two bucket ≥ the display size (16…512) and cache by `(path, bucket)`.
- `Image` and `background-image: url(x.svg)` pass their display size.

## Task 7.5: drag and drop, context menus

- **Drag and drop:**
  - attribute `draggable="true"`
  - events `ondragstart`, `ondragenter`, `ondragleave` and `ondrop`, with drop targets marked `droptarget="true"`
  - pointer drag beyond 6 px starts the drag: a ghost (a copy of the element's chunk drawn in the overlay at the pointer, opacity 0.7), and the source gets `:dragging`
  - gamepad: confirm on a draggable picks it up (ghost follows focus), confirm on a target drops, cancel aborts
  - C++ payload: `UiElem::DragPayload` (`std::any`)
- **Context menus:** `oncontextmenu` (right mouse button, long press of 0.5 s on touch, or the `ui_context` action) fires with the pointer position. The kit's `ContextMenu` component pushes a `shows-below` page positioned at that point.

## Task 7.6: Studio link

- The game writes `SaveData/ui_pick.json` (`{ "path": "…", "line": N, "id": "…" }`) when `ui_pick` resolves.
- UiStudio watches that file and selects the node.
- Studio communicates with the selected game's compatible `-ui_studio` runtime bridge using the versioned protocol from Phase 6. It must not call `Console::Get().Execute` in-process or assume that Studio embeds/links the selected game's C++ code. The lightweight `ui_pick` file remains a source-navigation fallback when the runtime bridge is unavailable.

## UIDB and advanced preview integration

Extend the shared `uidb.json` with. Publish each capability/descriptor from the same registration code that introduces it and call `UiDatabase::MarkDirty()`; the Phase 1 serializer and canonical file remain the only database implementation:

- layer/effect capability metadata: available filters, blend modes, isolation/group-opacity rules, backdrop modes, cache support, atlas limits and unsupported combinations;
- SVG support, raster-size buckets and source asset references;
- drag/drop event names, payload restrictions, context-menu host/action support and required input capabilities;
- preview target requirements for offscreen layer rendering and scene backdrop access.

Built-in preview fixtures should cover blur, drop-shadow, group opacity, backdrop, SVG, drag/drop and context menu. Runtime providers may describe notification stacks, contextual HUD menus or drag-enabled inventory widgets. Their descriptors specify a real overlay/menu host, a safe fixture payload and supported interaction scenarios. Drag/drop fixture payloads must be test data, never live inventory/game objects.

For `ui_pick`, add a stable target/source reference to the runtime-to-Studio message where possible; keep the existing `SaveData/ui_pick.json` fallback for the lightweight editor link. The UIDB path and pick-file path are distinct: UIDB describes the project, pick data identifies the current source selection.

**Check:** verify phase 7 features/capabilities and preview scenarios are in UIDB; open each fixture through the real runtime, including a drag/drop target, and confirm the visual output matches existing all-backend test goldens.

## Tests (all backends, `cross` `EQUAL`)

1. **Blur:** a 1 px white line blurred with `blur(4px)`. Its centre column alpha falls off symmetrically, with a probe at ±4 px ≈ 60% of the centre (tol 6).
2. **Drop-shadow** on a transparent PNG icon: shadow pixels follow the icon's alpha shape, not its box (probe at a transparent corner of the icon's box = 0).
3. **Group opacity:** two overlapping children under `opacity: 0.5; isolation: isolate`. The overlap pixel has the same alpha as a non-overlap pixel.
4. **Backdrop:** a panel over a known scene colour. The panel pixel ≠ the unblurred scene pixel at a sharp scene edge. Use a test level with a high-contrast quad.
5. **`cache: layer`:** a static complex panel. `ui_stats` shows its layer re-rendered 0 times over 30 frames.
6. **SVG:** the same icon at 16 and 128 px is crisp. Visual check, plus a probe at the shape edge for alpha between 0 and 255.
7. **Drag and drop:** mouse and gamepad paths move an item between two slots (`expect` the payload handler count).

## Implementation notes

Status: 7.1 to 7.6 are done on StealthAction and verified on dx11 only. 7.4 (SVG) is done too. Other backends and the other three games were not run (the GL v-flip of `UiLayerAtlas::UvRect` and of the scene source is written but untested).

**Layers (7.1, 7.2)**
- `UiRectAllocator` (guillotine packer) is shared by `UiBilboardAtlas` and `UiLayerAtlas`. Atlas pages are 4096² RGBA8, created lazily (about 64 MB each). Page 0 keeps `cache: layer` regions, pages 1-3 are scratch and are emptied every frame.
- The painter records `PushLayer` / `PopLayer` as a `LayerBegin` marker and a `LayerComposite` quad (an Image prim with `kFlagPremulTexture`); nothing is ever erased from a list, because legacy markers index items. `UiLayerPlan` (`UiLayers.cpp`) decides at submit which layers are rendered, allocates regions (a pass never reads the page it renders into: D3D11), orders the passes children first and patches the composite quads. A layer that cannot be built stays inline, drawn without its effect.
- Passes: content, down (4-tap), blur (9 taps as 5 linear samples, downsampled above sigma 4), drop-shadow (blur radius / 2), copy into the cache page. View ids come from `ViewIdManager::Reserve()` and are placed before the UI pass with `bgfx::setViewOrder`; the budget is 96 views per frame.
- Triggers (`BuildLayerSpec` in `UiSystem.cpp`): `filter: blur() / drop-shadow()`, `isolation: isolate`, `cache: layer`, and any page with `opacity < 1`. A layer applies the group opacity at composite; inside it everything starts at opacity 1.
- The cache hash covers effects, geometry, keys, prim and aux texels and the child hashes, not texture contents.
- Limits: stencil clips inside a layer are ignored (the atlas has no stencil); external (legacy) elements inside a layer are skipped; legacy children inside layers get no content bounds.
- Console `ui_stats` shows layers rendered, cached and views. Test commands: `layer_mark`, `expect_layers rendered|cached <op> <n>`.

**Backdrop (7.3)**
- `EngineMain::Render()` now runs the world pass first and the UI pass right after `RenderLevel` (UI takes the next view id; the swapchain copy and composite follow). `UiLayers::SetScene` hands the scene colour (`FinalFrameRenderTexture`) to the UI pass and clears it afterwards, so loading screens and world-space UI have no scene.
- `backdrop-filter` is recorded as a degenerate layer (marker plus one Box quad with a texture, `AddBackdrop`). When one is visible the plan builds the chain once per frame: scene to half, quarter and eighth size (colour only, alpha kept at 1), each blurred with sigma 2 texels = 4, 8, 16 UI units. `blur(R)` uses the nearest level; a colour function alone uses the sigma-4 level. The colour functions of the property are applied to the backdrop quad only. Pages holding the chain are excluded from later scratch allocation.
- Unsupported: blurring the UI under an element ("full mode"). A pass without a scene logs one warning and draws no backdrop.
- Test hook: `backdrop_scene <image|off>` stands an image in for the scene (`p7_edges.png`, black | white at x = 720), so the test does not depend on a level. The plan's "test level with a high-contrast quad" was replaced by this.

**Drag and drop, context menus (7.5)** (`UI/UiDragDrop.h/.cpp`)
- `draggable`, `droptarget`, `ondragstart / ondragenter / ondragleave / ondrop`, `:dragging`, `UiElem::DragPayload` (`std::any`, C++ only). `dragleave` also fires just before `drop`. A drag takes the move away from the pressed element (no list scrolling under it); the release does not activate buttons.
- The ghost is the source subtree recorded again into an unattached root (`UiSystem::RecordSubtree`, shifted and faded as one layer at 0.7) and drawn after the late UI in `EngineMain`.
- Gamepad: `UiElem::OnNavConfirm` picks up and drops, `OnNavCancel` aborts (the page is not popped). The ghost follows focus.
- `oncontextmenu` for a right click, a 0.5 s touch hold (not the mouse) and the `ui_context` action; the built-in action `ContextMenu('page.xml')` pushes the page and sets the style of its `.context-menu` element to the pointer position. **`ui_context` is registered in StealthAction only** (right mouse button, gamepad Y); the other games must add it. The kit has no `ContextMenu` component: a page with a `context-menu` element is the whole contract.

**Studio link (7.6)**
- `ui_pick` (the element under the pointer) and `ui_pick_at` write `SaveData/ui_pick.json` (`path`, `file`, `line`, `col`, `id`, `ref`); every element ref in the inspect JSON gained `ref` (`file:line:col`). UiStudio polls the file (400 ms) and selects the node; a file that was there before Studio looked is ignored. The runtime bridge (`ui_pick_at`) is unchanged.

**SVG (7.4)**
- nanosvg is vendored unmodified in `source/Engine/includedLibraries/nanosvg/` (`nanosvg.h`, `nanosvgrast.h`, licence, `README.txt` with the source URL and the pinned commit `239e102ec2c691f2902e20ace2ed36ee4a35cfe6`). The plan said `includedLibraries/nanosvg/` at the repo root; the engine's own third-party folder is `source/Engine/includedLibraries/`, so it went there. The implementation defines live in `UI/Render/UiSvg.cpp` (added to `Engine.vcxproj`).
- `UiImageAtlas::Get(path, displayLongSidePx = 0)`: an `.svg` is parsed once per path (`UiSvg::Load`), then rasterised at the smallest power-of-two longer side from 16 to 512 that covers the requested device-pixel size, packed like any other image, and cached under `path#bucket`. `Width`/`Height` of the result are the drawing's own size, so layout never changes with the raster. A drawing that cannot be read, parsed or packed is a cached failure with a log line.
- Callers pass the drawn size times the new `UiPainter::PixelScale()` (device pixels per UI unit under the current transform): `Image` (after object-fit), `background-image` (after background-size) and `border-image` (the border box). Fill-colour bleeding: nanosvg writes straight alpha, transparent texels are black, so a filtered edge may show a slight dark fringe on light backgrounds.
- Not supported by nanosvg: text, filters, masks, CSS styling, animation, external images.

**UIDB**
- `layers`, `dragDrop`, `contextMenu` capabilities are true. `paintEffects` lists blur, drop-shadow, isolation, group opacity, `cache: layer`, backdrop-filter and a `layer-limits` entry. `hosts` gained `drag-drop` and `context-menu`. Events and the `dragging` pseudo-class come from their registries. Recipes: effect scenarios (blurred, drop-shadow, glow, group-opacity, backdrop, cached), `engine.interaction.drag-drop`, `engine.interaction.context-menu` (fixtures `p7_dragdrop.xml`, `p7_ctx.xml`; test data only). Backdrop is an effect scenario that needs the scene. SVG: capability `svg`, a `svg-images` limits entry (buckets 16 to 512, supported and unsupported features) and the recipe `engine.image.svg` (fixture `p7_svg.xml`, asset `p7_icon.svg`).
- `UI_Reference.html` has new sections (layers, drag and drop, context menus, picking).

**Tests** (StealthAction, dx11): `p7_layers` (+ probes), `p7_dragdrop` (+ probes), `p7_backdrop` (+ probes) and `p7_svg` (+ probes: centre of 128 px, 16 px and background icons are solid, the corner is transparent, the circle edge has alpha 41) all pass. Regression on dx11: `p3_paint`, `p1_visual`, `p3_pages`, `p3_motion`, `kit_menus`, `p6_dropdown`, `p2_mouse`, `p2_events`, `kit_scrollbar`, the UIDB tests. `kit_nav` was not re-run (it failed before this phase).

**Gotchas**
- `Tools/ui/sync_ui_assets.py` overwrites the games' customised `ui/kit` folders (StealthAction's differ from the canonical copy). New shaders and fixtures were copied by hand into each game; do not run the sync without checking.
- `GameData` is staged into `Build/<game>/windows` by a post-build step that only runs when the exe relinks. After adding a fixture, run `python BuildScripts/finalize_gamedata.py <game GameData> <Build GameData>`.
