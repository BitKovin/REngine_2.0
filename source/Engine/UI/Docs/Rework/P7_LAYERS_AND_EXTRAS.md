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

(implementer writes here)
