# UI rework: implementation plan

These files tell an implementer exactly what to build, in what order, and how to check it. The design and the reasons behind it are in `Docs/UI/UI_REWORK_PLAN.md` ("the design"). Read the design once for context. When a detail here and the design disagree, **this plan wins**. When neither answers a question, **stop and ask the owner**. Do not invent an answer.

## How to use these files

1. Read `00_CONVENTIONS.md` completely before writing any code. It holds rules that every phase relies on: the threading model, file registration, build and test commands, shader workflow and code style.
2. Do **one phase per session**, in order. Each phase file is self-contained. It lists prerequisites, every file to create or change, the exact data structures, the algorithms, the tests, and the acceptance checks.
3. Inside a phase, do the tasks **in the listed order**. Every task ends with a **Check** step. Do not start the next task until the check passes.
4. When a phase is done, fill in its row in the status table below (date, result, notes), and append anything surprising to the phase file under "Implementation notes".
5. A step marked **STOP** means: stop, report what you found to the owner, and wait.

## Phases

| Phase | File | Depends on | Status |
|---|---|---|---|
| 0 | `P0_RENDERER_CORE.md`: test harness, base shader, draw lists, legacy routing, painter | nothing | done on StealthAction (see its Implementation notes) |
| 1 | `P1_MARKUP_CORE.md`: XML, CSS, cascade, flow layout, SDF text, basic elements, components; establish UIDB model/serializer early (Task 1.2a), then publish the first complete launch snapshot with `UiSystem` (Task 1.14) | 0 | done on StealthAction (not run on the other three games) |
| 2 | `P2_MENUS_AND_INPUT.md`: menu stack (any element), host options, events/actions/input metadata and preview scenarios, prompts, pilot pause menu | 1 | done on StealthAction, pilot look awaiting owner approval (see its Implementation notes) |
| 3 | `P3_PAINTING_AND_MOTION.md`: full CSS painting, image atlas, transforms, transitions, keyframes and paint/animation preview metadata | 1 (2 recommended) | done |
| 4 | `P4_WIDGETS_AND_KIT.md`: wrapped widgets, settings/localisation metadata and fixtures, engine kit, port all games, delete RmlUi | 2, 3 | done |
| 5 | `P5_HUD_AND_WORLD.md`: layers/HUD/host preview metadata, world panels, `UiContext`, world surfaces, loading screens | 2, 3 | not started |
| 6 | `P6_UI_STUDIO.md`: game-independent Qt Studio discovered from `GameData` + `uidb.json`; delete the ImGui editor and JSON `.ui` layouts | 1 (better after 4) | in progress: lint, stdin console, Qt Studio first slice (see P6 notes) |
| 7 | `P7_LAYERS_AND_EXTRAS.md`: layer atlas/effect preview metadata, blur, backdrop, group opacity, SVG, drag/drop, context menus | 3, 5 | not started |
| 8 | `P8_SCENE_VIEWS.md`: scene-view preview recipes and 3D scenes drawn into UI (character preview, item inspect) | 5, 7 | not started |

## Constraints that keep phase 8 possible (apply in every earlier phase)

Phase 8 renders 3D objects into render targets that UI elements then show. Earlier phases must not block it:

1. `UiGpu::Submit` always takes an explicit `viewId` and never assumes its view is the first or only one in the frame. Never cache bgfx view ids across frames.
2. A batch texture can be **any** bgfx texture, including a render target written earlier in the same frame. Don't add checks that only allow `AssetRegistry` textures.
3. Every `UiElem` exposes its final content rect in target pixels after `UiSystem::Update()` (`UiElem::ContentRectScreen()`, phase 1). Phase 8 sizes its render targets from it before rendering starts.
4. Pointer events (phase 2) support **capture**: an element that receives a press keeps getting move and release events until release, even outside its rect. A scene view uses this to rotate a model by dragging.
5. The phase 7 layer atlas is colour-only, for 2D layers. Don't make it the only offscreen mechanism: phase 8 needs targets with depth, from its own pool.
6. Nothing in phases 0–7 may assume the UI pass is the first view after scene rendering, or the last one. Draw order between views is decided only where views are created.

## Decisions already taken

The design left open questions. These are the answers. Do not change them without the owner.

| # | Decision |
|---|---|
| D1 | Layout vocabulary is **Panorama's**: `flow-children`, `fit-children`, `fill-parent-flow()`, `horizontal-align`, `vertical-align`. No `display: flex`. |
| D2 | UI Studio is **one game-independent build**. The developer selects a `GameData` directory; Studio reads the game's generated `uidb.json` and connects to that game's compatible runtime preview host. No per-game `GAME_NAME`/`GAME_DIR` build of Studio and no hard-coded game allowlist. |
| D3 | The **main** screen stack pauses the game. It calls a hook, `UiMenus::OnPauseChanged`, and each game connects that hook to its own `PauseGameManager`. |
| D4 | SDF text is used by the **new `Label` only**. Legacy `UiText` keeps the bitmap atlas path unchanged. |
| D5 | The ImGui `UiEditor` is deleted in phase 6. A small in-game inspector remains: the console commands `ui_pick` and `ui_dump`. |
| D6 | World surfaces default to `interaction="aim"`. Focus mode is opt-in. |
| D7 | Drag and drop and context menus stay in phase 7. |
| D8 | The bitmap glyph path stays (legacy `UiText` needs it anyway). New `Label` gets `font-render: bitmap` only if the owner asks later. |
| D9 | Games in scope: **ShooterGame, SimGame, SlasherGame, StealthAction**. **HorrorGame is out of scope**: never edit it, never sync assets into it, never build it. |
| D10 | The UI render target stays **1080p-tall** (`UiManager::GetScaledUiHeight()` tall, width by aspect), exactly as today. One UI unit is one target pixel. Never change the render target resolution. |
| D11 | The pilot game, and the default game for every test script, is **StealthAction**. |
| D12 | **No special root element types.** There is no `<Screen>` and no `<Hud>` tag. The menu stack, the HUD layer and world surfaces are hosts that show **any** element: a loaded layout or a C++ `UiElem`. Host options (`pauses-game`, `close-on-back`, `shows-below`, `hides-hud`) are optional attributes on the root element, read only by the host that understands them. Lifecycle hooks (`OnPushed`, `OnBack`…) and `Tick()` are ordinary `UiElem` virtuals. The world-space "screen" entity is `UiWorldSurface`, which draws whatever element is put into it. |
| D13 | Every game generates a deterministic, versioned `uidb.json` from its live UI registries on launch. The model/serializer is established early in Phase 1; the first complete snapshot is published after Phase 1 registries initialize, and each later phase extends the same database as its features are implemented. Canonical writable location: `SaveData/UIStudio/uidb.json`; Studio discovers it from the selected `GameData` directory. It is metadata, not source of truth. |
| D14 | A preview target may be a document, component, built-in/legacy element or **game-owned runtime UI**. Runtime previews are registered through `UiPreviewRegistry` providers with explicit host requirements, typed fixture parameters and named scenarios. Studio never guesses or mocks a HUD. |
| D15 | Game-specific live previews use the selected game's compatible `-ui_studio` runtime bridge. Studio itself remains generic. If the executable/bridge is unavailable, file editing and static metadata browsing remain available, but runtime-only targets are clearly marked unavailable. |

## Glossary

| Term | Meaning |
|---|---|
| UI unit / `px` | The unit of all UI coordinates. The UI target is `GetScaledUiHeight()` units tall, so 1 unit is 1 target pixel. A CSS `px` is one UI unit. |
| Legacy element | Any existing `UiElement` subclass (`UiImage`, `UiText`, `UiButton`, …). Its code keeps working. |
| `UiElem` | Base class of the new markup elements (`UiElemPanel`, `UiElemLabel`, …). It derives from `UiElement`. |
| Markup | `.xml` layout files and `.css` style files under `GameData/ui/`. |
| Document | One parsed `.xml` file (`UiDocument`). |
| Component | A reusable tag defined by an `.xml` file whose top element is `<Component>`. |
| Snippet | A component visible only inside its own file. |
| Draw list | `UiDrawList`, the recorded drawing commands for one pass. |
| Pass | One submission of a draw list into one bgfx view: the main UI pass, a billboard's pass, or a loading-screen frame. |
| Prim block | The 16 RGBA32F texels that describe one primitive in the primitive data texture. |
| Chunk | The cached commands one `UiElem` recorded for itself. |
| Context | `UiContext` (phase 5): focus, hover, menu stack and variables for one host (the main screen, each world surface, the loading screen). |
| Host | What shows an element and gives it behaviour: the menu stack (menus), the HUD layer, a world surface (`UiWorldSurface`), or a plain parent. There are no special root element types. |
| Page | An element pushed on a menu stack. Any element can be a page. |
| UIDB | The generated `SaveData/UIStudio/uidb.json` game UI database. Its model/serializer is established early in Phase 1; all feature phases publish registered UI APIs, capabilities, source references and preview targets into the same database. |
| Preview target | A named Studio-previewable UI target. It may be a document/component or a runtime-provided HUD/widget/hosted element. |
| Preview provider | Game/engine code registered in `UiPreviewRegistry` that creates and drives a real-engine preview fixture for a runtime preview target. |
| Golden | A reference screenshot that output must match. |
