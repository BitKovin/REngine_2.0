# Phase 6: UI Studio — complete UI authoring, debugging and testing environment

**Goal:** build a standalone Qt 6 application that provides the complete day-to-day development workflow for the new engine UI. It is not merely a visual layout editor. It must let a developer create, inspect, debug, validate, test, profile and maintain an entire UI project without repeatedly switching between a text editor, game, console and ad-hoc Python commands.

The Studio edits the actual `.xml` and `.css` files. Those files remain the only source of truth. The preview is rendered by the selected game's real engine runtime; Studio must not implement a second layout engine or approximate the engine's CSS rules. One generic Studio build discovers a project from its `GameData` directory and the generated `uidb.json` described in conventions §13.

**Prerequisites:** phase 1 is mandatory because Studio edits the markup DOM and consumes the schema. Phase 2 and 3 are strongly recommended for event, interaction and paint debugging. Phase 4 is recommended for the complete widget/settings kit. Phase 5 enables HUD, independent contexts and world-surface preview. Qt 6.11 MSVC is installed (`Desktop_Qt_6_11_1_MSVC2022_64bit`).

**Minimum useful milestone:** after phase 1, the application can open, edit, save, undo, hot-reload, inspect and lint an XML document with the real engine preview. The full workflow below becomes available progressively as later engine phases register their capabilities.

---

## 1. Non-negotiable design rules

1. **XML and CSS are authoritative.** Studio may keep caches, selection state, workspace state and temporary preview state, but it must never save a proprietary scene graph or generated layout format in place of the source files.
2. **The engine is the preview authority.** Preview layout, computed values, text metrics, hit testing, paint order and interaction must come from the selected game's actual engine build. Do not reimplement any of these in Qt.
3. **One Studio build for all games** (README decision D2). Studio has no `GAME_NAME`/`GAME_DIR` compile-time selection and no hard-coded game-name allowlist. The selected game supplies its runtime registries and preview providers through `uidb.json` and the `-ui_studio` runtime bridge.
4. **Rework scope remains restricted.** Do not edit, build or sync assets into `HorrorGame`. This is a restriction on this implementation task, not a game-name filter in the generic Studio application. In-scope games are ShooterGame, SimGame, SlasherGame and StealthAction.
5. **Respect the UI threading contract.** The selected game owns the actual engine runtime and its viewport. Studio communicates with it through a typed, local `UiStudioRuntimeBridge`; commands that touch UI state are marshalled to the engine main thread and obey conventions §2–3. The Qt GUI thread owns Studio widgets and IPC client state only. Do not call engine UI APIs from arbitrary Qt worker threads or start engine-owned threads from Studio.
6. **Do not mutate render snapshots.** Inspection reads the live processed state or immutable frame snapshot through a safe main-thread bridge. Debug overlays are drawn by Qt above the engine widget, not injected into a snapshot while rendering is in progress.
7. **Use the existing parsers and registries.** Schema, lint, completion, property editors and diagnostics are generated from the engine's tag registry, component registry, CSS property table, pseudo-class list and action registry. No parallel hand-maintained list of tags or CSS properties.
8. **Preserve runtime compatibility.** Existing `UiElem::Load`, `LoadLayout`, hot reload, `ui_dump`, `ui_pick`, `ui_reload`, legacy elements and the lightweight in-game inspector continue to work. Studio is optional tooling; games must not depend on it at runtime.
9. **Do not change the renderer's target size or view ordering for editor convenience.** The UI target remains 1080p-tall according to `UiManager::GetScaledUiHeight()` (README D10). A Studio preview preset changes the viewport aspect/size supplied to the engine, not the renderer's UI-unit convention. Never assume the UI pass is the first or last bgfx view.
10. **No new third-party libraries** beyond those already approved in earlier phases. Use Qt 6 modules already available and existing engine dependencies. Do not add another XML, CSS, JSON or rendering implementation.
11. **Source navigation must be unambiguous.** IDs are scoped to document/component instances and can repeat. Never identify a runtime element by a bare ID alone when an instance path or source location is available.
12. **Safe failure is preferable to a destructive edit.** Invalid markup, missing assets, a failed preview reload or a file conflict must not silently overwrite the last known-good document or corrupt a user's unsaved changes.

### Capability gates

Studio must query the generated schema/capability information at startup and enable only features supported by the current engine build:

| Capability | Available after | Studio behaviour before that phase |
|---|---:|---|
| XML, CSS, components, basic layout and computed style | 1 | Required baseline |
| Component Workbench and isolated component preview | 1 | Component catalogue/source editing works; preview uses a real engine-built instance |
| Basic Responsive Preview Matrix | 1 | Profiles can be captured sequentially through the one real preview host; no independent layout implementation |
| Events, pointer capture, simulated input and menu stack | 2 | Hide/disable those controls; do not fake them |
| Gradients, transforms, transitions, animation, shadows and filters from phase 3 | 3 | Show only registered properties and states |
| Native widgets, settings binding, text scale and pseudo-localisation | 4 | Show only registered tags/settings; pseudo-loc control disabled |
| HUD layers, tooltips, contexts, world panels and world surfaces | 5 | Host preview unavailable; ordinary viewport preview remains |
| Independent interactive preview contexts in the Responsive Preview Matrix | 5 | Before phase 5, use one live interactive preview and sequential profile captures; do not fake independent state |
| Layer atlas, multi-pass effects, drag/drop and Studio pick-file link | 7 | No layer graph or game-to-Studio pick-file watcher yet |
| 3D `SceneView`, `Model` and `Light` | 8 | No 3D scene authoring controls |

Do not make phase 6 a prerequisite for phase 7 or 8. Later phases may add panels/actions to Studio, but the phase 6 application must build and remain useful without them.

---

## 2. Deliverables and project layout

Create the standalone, game-independent application under `source/UiStudio/`, the common Qt-side tooling/IPC client under `source/EditorHost/`, and the small runtime bridge/preview-provider API under `source/Engine/UI/Preview/`. The game executable supplies the game-specific runtime; Studio itself is not linked against one selected game's sources.

Expected high-level layout (exact class splitting may follow the existing QtEditor conventions, but responsibilities must remain separate):

```text
source/EditorHost/
    CMakeLists.txt
    UiStudioRuntimeClient.h/.cpp            # typed local IPC client and protocol validation
    GamePreviewProcess.h/.cpp               # launch/attach/stop selected game's -ui_studio process
    GamePreviewWindow.h/.cpp                # host the runtime window where platform support allows
    UidbLoader.h/.cpp                       # parse/validate generated database and path safety
    EditorPreviewSession.h/.cpp             # project/target/scenario/profile lifecycle

source/UiStudio/
    CMakeLists.txt
    main.cpp
    UiStudioApplication.h/.cpp
    MainWindow.h/.cpp
    StudioProject.h/.cpp
    StudioWorkspace.h/.cpp
    StudioDocument.h/.cpp
    StudioDocumentManager.h/.cpp
    StudioElementRef.h
    StudioStyleSheet.h/.cpp
    StudioSchema.h/.cpp
    StudioUidbProject.h/.cpp                # GameData selection and adjacent UIDB discovery
    StudioPreviewCatalog.h/.cpp              # document/component/element/runtime target browser
    StudioDiagnostics.h/.cpp
    StudioCommandRunner.h/.cpp
    StudioTestManager.h/.cpp
    StudioTestResults.h/.cpp
    StudioAssetIndex.h/.cpp
    StudioLocalizationModel.h/.cpp
    StudioLocalizationDocument.h/.cpp
    StudioComponentWorkbench.h/.cpp
    StudioRuntimePreviewManager.h/.cpp
    StudioPreviewProfile.h/.cpp
    StudioSettings.h/.cpp
    Models/
        ProjectTreeModel.h/.cpp
        ElementTreeModel.h/.cpp
        ComponentPaletteModel.h/.cpp
        ComponentCatalogModel.h/.cpp
        LocalizationTableModel.h/.cpp
        DiagnosticsModel.h/.cpp
        TestTreeModel.h/.cpp
    Editors/
        XmlEditor.h/.cpp
        CssEditor.h/.cpp
        UiSyntaxHighlighter.h/.cpp
        CompletionProvider.h/.cpp
        PropertyEditorFactory.h/.cpp
    Panels/
        PreviewCatalogPanel.h/.cpp
        RuntimeFixturePanel.h/.cpp
        CanvasPanel.h/.cpp
        HierarchyPanel.h/.cpp
        InspectorPanel.h/.cpp
        ComponentsPanel.h/.cpp
        ComponentWorkbenchPanel.h/.cpp
        ComponentPlaygroundPanel.h/.cpp
        ResponsivePreviewPanel.h/.cpp
        AssetsPanel.h/.cpp
        StatesPanel.h/.cpp
        StackPreviewPanel.h/.cpp
        HostPreviewPanel.h/.cpp
        DiagnosticsPanel.h/.cpp
        TestRunnerPanel.h/.cpp
        OutputPanel.h/.cpp
        ProfilerPanel.h/.cpp
        LocalizationPanel.h/.cpp
        LocalizationPickerPanel.h/.cpp
        DocumentationPanel.h/.cpp
    Commands/
        StudioEditCommands.h/.cpp
        StudioRefactorCommands.h/.cpp
        StudioCanvasCommands.h/.cpp
```

This is a responsibility map, not a requirement to create empty files. A class may be combined with a closely related class when that makes the implementation smaller; do not collapse the document model, engine bridge, test runner and main window into one class.

`UiStudio/CMakeLists.txt` builds one generic application and does not accept `GAME_NAME` or `GAME_DIR`. `EditorHost` is a Qt-side library for UIDB parsing, path validation, local IPC and native-window hosting; it does not statically link a selected game's engine/game sources. Keep `QtEditor` compatibility separate: if it still uses `EngineInstance`/`EngineViewportWidget`, retain its existing host implementation or move shared code without making UiStudio game-specific. The selected game's executable is launched/attached through the runtime bridge described in task 6.2.

Workspace preferences (panel arrangement, recent files, preview presets, selected test backend, editor preferences) belong in Studio's own settings/workspace storage, not in `GameData/ui/` and not in the engine's canonical managed assets.

---

## Task 6.1: UIDB consumption/validation, schema export and `-ui_lint` (engine, headless)

**Files:** engine argument handling, the relevant UI registries/parser modules, `UiMarkupLog`, and the engine project/build registration files required by conventions §6. Keep the implementation in the engine so command-line tools, Studio and AI use the same information.

### 6.1a. Consume and validate the engine-generated UIDB

The normal-launch generator is an engine feature implemented in early Phase 1 (Task 1.2a) and first published from `UiSystem::Init` (Task 1.14). **Do not implement another generator or make Phase 6 the first point at which `uidb.json` appears.** Phase 6 consumes the common file and validates its contract:

- resolve `SaveData/UIStudio/uidb.json` from the selected `GameData` directory, with the portable `GameData/ui/uidb.json` fallback described in conventions §13;
- validate format/version, game identity, engine/build/protocol compatibility, path safety, capabilities and required fields;
- accept unknown optional fields, report unknown required features, and never silently interpret an incompatible newer format;
- display registry revision/freshness and distinguish a missing database from a corrupt or incompatible database;
- when UIDB is missing, keep source-only editing available and explain that the game must be launched with a build containing the Phase 1 bootstrap; when the disk write failed but the runtime is connected, use its in-memory metadata snapshot;
- refresh on a compatible runtime handshake or registry-revision notification. Do not parse partially-written files: the engine writer uses atomic replacement.

Phase 6 may add read-only consumption adapters for registry categories that earlier phases have published. It must not add new authoritative tag/property/action schemas in Qt. The runtime registries and engine serializer remain authoritative.

### 6.1b. `-ui_schema <out.json>` compatibility export

Keep this headless command for CI and debugging. It exports the schema/registry portion of the exact same UIDB serializer to the requested path and exits. Do not maintain a second schema generator. The export must include:

- `formatVersion`, engine/build/protocol version, stable `game.id`/`game.displayName`, and the game UI root (`GameData/ui/`);
- every built-in markup tag, registered component tag, legacy/reflection tag and code-behind tag available to this game;
- for each tag: category, documentation summary where available, attributes/parameters, type, default, required flag, accepted values/range, whether variables are accepted, and whether the value affects layout, paint, input or host behaviour;
- component parameters, default values, named/default slots and component source path;
- registered events and event attributes, including events raised by components;
- built-in actions, argument types and short descriptions;
- every CSS property, shorthand expansion where applicable, grammar summary, inherited flag, initial value and invalidation category (`style`, `layout`, `transform` or `paint`);
- supported selector pseudo-classes and their runtime meanings;
- supported at-rules, functions and currently unsupported CSS constructs, so completion does not advertise features the parser rejects;
- test-runner commands and argument syntax available in this engine build;
- feature/capability flags for phases 2–8, where known;
- registered preview targets, provider IDs/versions, host requirements, fixture parameter schemas and named scenarios; these descriptors explain how to present game-owned HUD/runtime items such as a crosshair or health bar.

Schema generation must not force-load every UI document or instantiate every game entity. It reads registries after normal registration has completed. If a component registry is discovered from `GameData/ui/`, include its discovered definitions and file locations.

The schema must be deterministic: stable ordering, stable key names, no pointer values or timestamps that change on every run. Write to a temporary file and rename into place so Studio never reads a partially written JSON file.

**Check 6.1a:** use the Phase 1-generated file from a normal game launch. Point Studio's project chooser at `GameData` and verify discovery without a game-name argument. Check compatibility/error states and confirm that document, component, tag, CSS property, action and preview-target descriptors appear when the selected game has implemented those phases. Also verify the in-memory snapshot path when disk serialization is unavailable.

**Check 6.1b:** run `-ui_schema <path>`, load it with Python's `json` module and confirm it is the schema/registry projection of UIDB, not an independently maintained format.

### 6.1c. `-ui_lint`

Keep the existing command-line behaviour: `-ui_lint` loads every `.xml` under `GameData/ui/`, recursively discovers imported components/snippets and all referenced `.css` files, builds each layout offscreen (not attached to the viewport), writes `SaveData/ui_lint.txt`, and exits. The final line remains `LINT PASS` or `LINT FAIL <n>`.

The lint path must use the actual pugixml, CSS tokenizer/parser, component builder, style resolver, tag registry, localisation data and engine asset lookup. Do not create a simplified parser just for lint. It must not render a frame or require an interactive window. It may initialise whatever engine subsystems are strictly required to parse and resolve the UI; it must shut them down cleanly.

At minimum, report the following, each with severity, file, line, column, element/selector context and a concise message:

**Markup and component errors**

- malformed XML and unsupported/missing root structures;
- unknown or unavailable tags, invalid attributes, missing required component parameters and unknown parameters;
- duplicate IDs within the same document/component instance scope (the same ID in separate component instances is valid);
- missing imports, missing component definitions, import cycles and missing named/default slots;
- invalid variable references, malformed event/action call syntax, unhandled actions and handler names that cannot resolve through the element/ancestor/code-behind chain;
- unsupported legacy nesting and other builder errors.

**CSS and style errors**

- malformed selectors, invalid property values, unknown properties, unsupported at-rules/functions and invalid custom-property substitutions;
- `var()` cycles and missing variables without fallbacks;
- invalid `@font-face` entries, missing font files and unavailable font families;
- declarations that are overridden or unreachable where that can be established safely (these are informational, not build-breaking);
- selector matches that appear ineffective because the target is collapsed/hidden or the declaration is invalid, where useful.

**Resources and localisation**

- missing image/texture, font, sound, video and other resource paths used by markup or supported widgets;
- missing localisation keys (`${KEY}`) against the active game's localisation data;
- duplicate localisation keys, malformed entries and conflicting values according to the existing localisation format;
- unreferenced localisation keys (informational by default; never delete them automatically);
- keys that exist in the source locale but are absent from one or more configured target locales;
- invalid paths after variable substitution when the value is statically resolvable;
- large images that bypass the atlas, atlas/page budget warnings and resources that cannot be decoded.

**Layout and text**

- `ui_overflow_report` results: unwrapped text wider than its box and wrapped text taller than its box;
- text clipped/ellipsised/shrunk where the linter can identify it, percentage sizing under a fit-children parent, fill-parent-flow without a compatible flow parent, invalid dimensions and likely zero-sized interactive controls;
- pseudo-localisation overflow when the phase 4 feature exists;
- components that depend on a missing font or have no content size when their style requires one.

**Input and usability**

- focusable controls with no useful accessible/localised label where one can be determined;
- controls that are disabled or permanently hidden but still configured as default focus or explicit navigation targets;
- unresolved `nav-up`, `nav-down`, `nav-left`, `nav-right` targets;
- conflicting hotkeys on simultaneously visible controls (warning only);
- event handlers that refer to unknown element paths or unsupported action arguments.

Do not report a speculative issue as an error. Use `info`, `warning` and `error`; only actual parsing/build failures and definite broken references make the final result `LINT FAIL`. Warnings remain visible and count separately. Add a machine-readable `SaveData/ui_lint.json` companion with stable fields (`severity`, `code`, `path`, `line`, `column`, `element`, `message`, optional `help`), while preserving `ui_lint.txt` for existing workflows.

`-ui_lint` must lint all files in the selected game's `GameData/ui/`, not only the files reachable from the currently opened document. It must not modify source files or create files inside a managed engine asset folder.

**Check 6.1c:** run lint on all four in-scope games. Create temporary fixtures for a malformed XML file, a missing image, an import cycle, an invalid CSS property and an unresolved action; verify each is reported at the correct source line. Restore the fixtures after the check. Run `-ui_lint` twice and confirm the result order is deterministic.

---

## Task 6.2: generic Studio-to-game runtime bridge

The generic Studio must preview the selected game's real runtime without being compiled once per game. Implement a small `-ui_studio` mode in the game executable and a local, authenticated IPC protocol between Studio and that runtime.

1. The game starts in a dedicated UI-preview mode: it initializes the same engine/UI registries, assets, renderer and game-specific preview providers, but it does not start gameplay, load a live player, connect to multiplayer, write settings or perform ordinary game actions.
2. Studio launches the executable path from UIDB when available, or lets the developer select it once. It may also attach to an already-running compatible `-ui_studio` instance. A normal gameplay process must not accept mutating Studio commands unless it explicitly entered an owner-approved read-only live-capture mode.
3. Use a local OS IPC transport with a protocol version, per-session random authentication token, bounded message sizes, request IDs, timeouts and explicit command allowlists. Do not expose a network listener on external interfaces.
4. Protocol operations include handshake/capability negotiation, list/refresh preview targets, create/destroy preview session, load/reload document, instantiate component/element, create runtime-provider fixture, apply scenario/typed values, resize viewport, forward simulated input, inspect tree/styles/events, run tests, capture screenshot, fetch logs/stats and orderly shutdown. Unsupported operations return structured errors.
5. The engine polls/dispatches IPC without blocking in its existing main-thread update point; do not start an engine-owned networking/IPC thread. Every command that mutates UI state executes on the engine main thread and satisfies conventions §2. The transport callback must not access live UI state from a socket/pipe worker.
6. Host the selected runtime's native SDL window inside Studio through a platform-supported foreign-window container where reliable. If native embedding is unavailable, show a linked external preview window and keep inspection/commands synchronized. Never create a second renderer in Qt or copy engine draw commands into a Qt renderer.
7. Runtime process lifecycle is explicit: launch, handshake, project/build identity validation, ready, preview, stopping, exited/crashed. Surface stderr/engine logs and preserve the last screenshot/diagnostics after a crash. Never leave orphaned processes.
8. Keep the transport local and development-only. `-ui_studio` must be opt-in and must not weaken normal game security or add a production remote-control endpoint.

The old `EditorHost` in-process engine embedding path may remain for `QtEditor` or a generic engine-only fallback, but it is not the mechanism for loading arbitrary game-specific C++ registries. `EditorHost` for UiStudio owns UIDB parsing, IPC, process lifecycle and native-window hosting; it must not statically link one selected game's engine/game sources.

**Check 6.2:** build Studio once, then point it at GameData directories for at least two different in-scope games without rebuilding Studio. Verify UIDB discovery/handshake, runtime preview, screenshot capture, input forwarding and orderly shutdown for both. Verify wrong protocol/build IDs and an invalid token are rejected. Confirm the normal game build remains unchanged when not launched with `-ui_studio`.

## Task 6.3: document model and source-safe editing

**Files:** `StudioDocument*`, `StudioDocumentManager*`, `StudioElementRef.h`, `StudioStyleSheet*`, edit-command classes and the required CMake entries.

### XML document model

`StudioDocument` wraps one `.xml` file and owns:

- canonical absolute path and project-relative path;
- the exact current source text (UTF-8), a pugixml DOM parsed from that text and the parse result;
- dirty, externally changed, read-only and parse-valid flags;
- file hash/modification metadata from the last load/save;
- source mapping for nodes/attributes (line/column and byte spans where available);
- a `QUndoStack` of semantic edit commands.

Supported semantic commands are: insert node, delete node, move/reparent node, reorder siblings, duplicate node, set/remove attribute, set node text, wrap selected nodes in `Panel`, extract a component, rename an ID and update explicit references, and edit a component parameter/default. Each command must be undoable and redoable. A completed canvas gesture is one undo item, not dozens of pixel-level changes.

When serialising XML:

- use `pugi::format_indent` with a two-space indent, preserving comments, processing instructions and the existing attribute order;
- do not write the file when the resulting bytes are unchanged;
- do not silently drop unknown attributes, nodes or comments;
- report a parse/serialisation failure without replacing the last valid file;
- write to a sibling temporary file and atomically replace the destination after a successful write;
- retain a recoverable in-memory copy of unsaved source text.

A formatting change caused by the first pugixml serialisation is acceptable only as a single explicit document operation; it must not cause repeated full-file churn on every subsequent edit. Preserve existing newline convention where practical. Never reformat unrelated CSS while editing XML.

### CSS document model

`StudioStyleSheet` stores the exact source text and token spans produced by `UiCssTokenize`. Editing an existing declaration replaces **only the value span**. Property names, comments, whitespace and unrelated rules stay byte-identical. A new declaration is inserted immediately before the rule's `}` on its own line, indented like neighbouring declarations. If a shorthand must be expanded, make that expansion explicit in the undo command and preserve the original declaration until the user accepts the change.

The CSS model must track:

- add byte offsets to `UiCssToken` (`BeginOffset`, `EndOffset`, end-exclusive) while keeping its existing `Line`/`Col`; this is diagnostic/source-mapping metadata and must not change tokenisation or parsing semantics;
- retain declaration value spans and rule/selector spans through `UiCssDeclaration` / `UiCssRule` (or a Studio-side source map derived from the token offsets);
- selector specificity and source order as parsed by the engine;
- `@import` dependency links;
- inline `<style>` blocks and their XML source location;
- custom properties (`--*`) and their resolved value/source;
- the winning declaration for a computed property.

### Stable element references

A `StudioElementRef` must survive hot reload and ordinary source edits as far as possible. It should identify an element using a combination of:

- canonical definition file;
- component-instance ancestry (component tag and source location at each nesting level);
- scoped ID path when IDs exist;
- source line/column or a stable DOM path fallback when there is no ID.

A bare ID is never sufficient for a duplicated component instance. When a hot reload invalidates a selection, attempt to resolve it by the stable reference; otherwise clear selection and show a non-modal notification rather than selecting a different element accidentally.

### Multi-document editing and file conflicts

- Support multiple XML and CSS tabs, split editor groups, pinned tabs and dirty markers.
- Undo/redo operates on the active document. Provide a workspace-level `QUndoGroup` so toolbar/menu actions follow the focused editor/document. A cross-file refactor (for example rename a component file and update imports) is one grouped undo transaction across affected documents.
- Track external file changes. When a file changes on disk while it has unsaved edits, do not overwrite either side. Offer **Reload external**, **Keep my version**, and a side-by-side diff/merge view.
- When the engine hot-reloads after a Studio save, restore the preview selection, scroll position, active host and simulated state whenever the corresponding elements still exist.
- Use the engine's existing 0.5 s hot-reload polling. Studio should not add a competing background reload thread.

**Check 6.3:** open `ui/kit/pages/pause.xml` and its imported stylesheets; edit an attribute, move a node, add/remove a node and edit a CSS value. Undo/redo every action. Confirm comments and unrelated CSS bytes remain intact, no-op edits do not rewrite files, and the engine hot-reloads the saved result.

---

## Task 6.4: the UiStudio application and intuitive authoring workflow

**Files:** `source/UiStudio/`, `EditorHost` integration, project/dock models, editors and panels.

### 6.4a. Project and workspace management

The application opens a project by selecting only its `GameData/` directory. It discovers the sibling game root and `SaveData/UIStudio/uidb.json`, validates the format/build, and opens `GameData/ui/` as the default UI source root. If UIDB is absent, it offers a basic source-only mode and explains that launching the game once (or manually selecting a compatible UIDB) is needed to discover runtime registries and game-specific preview providers. It must not ask for `GAME_NAME`, compile-time game selection or manual tag/schema registration. It must provide:

- project name/game identity, active game, UI root, engine executable/build folder and asset roots in a visible project-settings page;
- a project tree with folders, XML, CSS, localisation and asset filters; recursive search by filename, tag, ID, class, attribute, text, selector and localisation key;
- recent projects and recent documents; restore the last workspace layout and open documents;
- file creation from templates (empty layout, component, stylesheet, test layout, test script) with valid boilerplate;
- rename, move and delete with a confirmation that lists affected references; for a rename, offer to update known `<import>`, `<Frame src>`, image/font/sound and stylesheet references in one undoable refactor;
- a dependency view: which documents import/use a component, which stylesheets are imported, and which assets a document references;
- a search-results view with clickable file/line locations and a replace preview before applying multi-file edits.

Never rename/delete a file outside the selected game's UI/assets roots through a normal project-tree action. Never edit canonical engine-managed assets in a game's mirrored folders; display a clear read-only/managed-asset indicator and direct edits to `source/Engine/UI/Assets/GameData/` instead.

Persist workspace state separately from game assets. A workspace file may remember window geometry, dock placement, open tabs and the last preview selection, but it must not become a second source of layout truth.

### 6.4b. Default window layout and interaction modes

The default workspace should be approachable without configuration:

| Area | Default content |
|---|---|
| Left dock | Project tree, then Components and Assets tabs |
| Centre | Canvas/engine preview, with XML/CSS source tabs available as a split view |
| Right dock | Hierarchy above Inspector; Inspector tabs for Attributes, Style, Events and Accessibility |
| Bottom dock | Diagnostics, Test Runner, Output/Console and Profiler tabs |

All docks are movable, tabifiable, floatable and closable. Provide **Reset Workspace Layout**. Persist the arrangement per UIDB game identity/project, not per compiled Studio build. Do not open every advanced panel by default; make them available from View and the command palette.

Provide two explicit canvas modes:

- **Design mode:** clicks select elements and show editing handles. Engine input is not forwarded to the game UI.
- **Interact mode:** the engine receives pointer, keyboard and navigation input exactly as a player would. Selection overlays do not intercept input. A clear mode indicator prevents the user from confusing a selected design element with a focused runtime control.

Provide a single command palette (`Ctrl+Shift+P`) that searches commands, panels, files, components, actions and settings. It should be keyboard-navigable and show shortcuts next to commands.

### 6.4c. Canvas and visual editing

The Canvas hosts the selected game's runtime viewport (native child window where supported) with a transparent Qt overlay for editor-only guides and handles. It must display the real engine output; Qt may draw overlays but may not paint a substitute version of the UI.

Required canvas features:

- selection, additive selection, deselection, parent/child selection and selection cycling when elements overlap;
- element outlines and a distinct selected outline; hover outline in Design mode;
- eight resize handles, move handle, alignment guides and snapping to parent edges, sibling edges and centres (default 6 screen pixels, configurable);
- box-model visualisation for border, padding and content rect; show dimensions and coordinates while dragging;
- rulers, optional pixel grid, centre lines, safe-area guides and optional baseline/anchor guides;
- zoom in/out, fit-to-content, fit-to-window, 100%, pan with middle mouse/space-drag, and a minimap for large canvases;
- preview aspect-ratio presets (16:9, 16:10, 4:3, 21:9, portrait and custom); these resize the preview viewport only and do not change the engine's UI-unit or render-target rules;
- background choices: transparent checkerboard, solid colour, captured game scene and a neutral test background;
- optional overlay layers: bounds, content boxes, clipping, paint order, hit-test regions, focus target, navigation links and text baselines;
- multi-selection alignment/distribution tools and numeric position/size fields;
- keyboard nudging (1 UI unit; Shift = 10; Alt = 0.1 where meaningful), with one undo item per key-repeat gesture;
- drag from Components to instantiate a tag; drag an image/font from Assets into an `Image`/style field; drag a tree node to reorder/reparent it;
- context actions: duplicate, delete, wrap in Panel, extract component, open component definition, copy/paste markup, copy selector, reveal in Project and inspect computed style.

Canvas move/resize edits must create ordinary source edits, never a hidden Studio-only transform:

- **Move:** write `x` and `y` attributes/style values, using the existing Panorama layout vocabulary. Do not introduce absolute-position or flex concepts that the runtime does not support.
- **Resize:** write `width` and `height` values.
- Prefer an existing local stylesheet rule when it is clearly local to the document and only targets the selected element. Otherwise use the element's inline `style` attribute. The Inspector must show which choice will be made before a shared selector is edited.
- If the selected element uses `fill-parent-flow`, percentage sizing, flow alignment or another constraint that makes direct movement/resizing misleading, show the constraint and offer the appropriate parent/flow/property edit instead of silently overriding the layout.
- A canvas gesture updates the in-memory document during the gesture and commits one undo command on release. Save/hot-reload the completed command; do not write the file on every mouse-move event.

### 6.4d. Hierarchy and component authoring

**Hierarchy** is a `QTreeView` over the source DOM, not a separate runtime hierarchy. It must:

- display tag, ID, classes and a compact warning/state indicator;
- preserve document order and make component instance boundaries visible;
- distinguish definition nodes from slotted children and show which source file owns each node;
- support drag reorder/reparent with validation (no invalid XML parent/child relationships and no slot violations);
- select the same element in Canvas and Inspector;
- offer wrap in Panel, duplicate, delete, extract to component, open definition, reveal source line and copy ID path;
- show hidden/collapsed elements without making them impossible to select in the tree.

**Components** is generated from the component registry/schema. It must support search, descriptions, parameter help, slot documentation and drag-to-canvas insertion. When inserting a component, show required parameter editors and create valid markup. For a component instance, offer **Edit instance**, **Open definition** and **Override parameter** without confusing the using document's slotted content with the component's own source.

Extraction to a component creates a component definition with a valid tag name, moves selected markup into it, and replaces the selection with an instance. It must preserve slotted children, IDs within the proper scope, styles, imports and source ownership. If extraction cannot preserve semantics, disable the operation and explain why.

### 6.4e. Inspector and CSS cascade tools

**Attributes tab:** create schema-driven typed editors. Use checkboxes for booleans, combo boxes for enums, numeric fields with ranges/steps, vector editors, resource pickers for paths, multiline editors for text and dedicated editors for event action lists. Show required/default status, variable support and validation errors. Unknown attributes remain visible and editable rather than being discarded.

**Style tab:**

- list CSS properties grouped into Layout, Box, Text, Image, Motion, Effects and Input/host categories where those categories exist;
- filter/search properties; offer grammar-aware completion and a colour picker with alpha;
- display specified, computed and inherited values separately;
- for each computed property, show the winning declaration's `file:line:column`, selector, specificity, source order and cascade layer (component/document/inline/C++ override);
- provide a cascade trace showing losing declarations and why they lost;
- distinguish values from `style="…"`, stylesheet declarations, inherited values, defaults and C++ `Style()` overrides;
- when editing a property, edit the winning declaration only when it is unambiguous. Otherwise ask the developer to choose **edit this element inline**, **edit shared rule**, or **create a new class rule**;
- create/manage class chips, add/remove classes, rename a class across the project and jump to all its rules;
- show custom-property values and where each `var(--x)` value came from;
- explain invalidation (layout/paint/transform) and show a warning when an edit may change the flow or cause re-layout.

`ui_computed` is the source for runtime computed values. It must return valid machine-readable JSON including the element reference, border/content rectangles, absolute transform, computed property values and source provenance. Keep the existing console command usable; extend its argument form for a scoped ID path or picked element reference without breaking `ui_computed <id>`.

### 6.4f. Code editor and navigation

Provide `QPlainTextEdit`-based XML and CSS editors with:

- syntax highlighting for XML tags/attributes/strings/comments and CSS selectors/properties/values/functions/at-rules/comments;
- line numbers, current-line highlight, bracket/tag matching, auto-indent, configurable tabs/spaces and newline preservation;
- completion for tags, component names, attributes, event attributes, actions, IDs in selector/action contexts, CSS properties, values, variables, font families and known resource paths;
- hover help sourced from the schema, including property grammar, inherited status and invalidation category;
- diagnostics underlines and a Problems list; click a diagnostic to navigate to the exact line/column;
- go to definition for component tags, stylesheet imports, IDs, localisation keys and asset paths where a definition exists;
- find references for component tags, IDs, classes, variables, custom properties and localisation keys;
- rename-symbol refactors for IDs/classes/components/custom properties where references can be proven;
- code folding for XML elements and CSS rules; document outline; matching open tag/selector navigation;
- formatting commands for the current XML document or CSS rule, not a mandatory project-wide formatter;
- copy as markup, copy selector, copy ID path and paste a valid XML fragment.

Completion must reflect the selected project's current UIDB/runtime schema. It must not offer a tag, action or CSS property that the current engine build does not support.

### 6.4g. Save, reload and recovery workflow

- `Ctrl+S` saves the active document; `Ctrl+Shift+S` saves all dirty documents.
- Show dirty state in tabs and the Project tree. Prompt before closing a dirty document or exiting.
- Support optional autosave of the editor buffer to a recovery file. Autosave must never overwrite the real source file without the user's configured save policy.
- Save with an atomic temporary-file replacement. Keep a small rotating recovery copy before replacing a file; do not create noisy backups on every keystroke.
- On save, let the engine's existing hot-reload poll observe the completed file. Show **Saved**, **Preview reloaded**, or the actual reload error separately; a successful file write does not imply a successful engine reload.
- Preserve last-known-good preview if the newly edited document fails to parse/build. Show the new diagnostics and offer **Revert preview to last valid source**; do not rewrite the source automatically.
- If an external process changes a file, use the conflict workflow in task 6.3; never silently discard either version.

**Check 6.4:** in StealthAction, open `ui/kit/pages/pause.xml`. Use the canvas to move and resize an element, reorder it in Hierarchy, edit an attribute and a CSS property in Inspector, and edit the XML/CSS directly. Confirm all views stay synchronised, preview uses real engine layout, hot reload occurs, selection survives where possible, and every operation is undoable.

### 6.4h. Component Workbench and Playground

Provide a first-class **Component Workbench** for developing reusable UI components without repeatedly navigating through full game screens. This is a development surface over the existing component system, not a replacement component format.

**Discovery and catalogue**

- Index global component definitions discovered by the engine registry, including `GameData/ui/kit/components/` and `GameData/ui/components/`, plus project-local snippets from the currently open document.
- Display component tag, definition path, description/documentation, declared parameters, required/default values, available slots, imports, stylesheets, dependency count and usage count.
- Separate built-in tags, global components, imported components and document-local snippets. Never present a snippet as globally available when its scope is local to one document.
- Search by tag, description, parameter name, slot name, source path and usage. Filter by project/game, category and recently edited.
- Show duplicate tag registrations, import failures, cyclic dependencies and unresolved component references as diagnostics with direct source navigation.
- Show **Open definition**, **Find usages**, **Insert instance**, **Preview**, **Duplicate as new component** and **Extract selection to component** actions where valid.

**Isolated component preview**

- Instantiate the selected component through the real `UiDocumentCache`, component registry and builder. Do not render a Qt mock or recreate parameter/slot expansion in Studio.
- Build a temporary preview host document in memory using valid engine markup/import semantics. It must not be written into the game's `GameData` unless the developer explicitly chooses **Create preview fixture**.
- Generate typed controls for declared parameters using the same schema rules as Inspector. Required parameters must be supplied; optional parameters use their declared defaults. Unknown parameters and invalid values must be reported using engine diagnostics.
- Provide a slot editor for the default and named `<Children/>` slots. Allow developers to add, remove and rearrange temporary slotted content and show which source document owns each slotted node. Preserve the distinction between component-owned content and using-document content.
- Support preview variables, localisation, locale, theme, text scale and supported pseudo-states. These values belong to the preview session and must not silently modify game settings or source files.
- Preview normal, hover, focus, active, disabled and other states only when the current schema/runtime supports them. Use the engine's input/event path for interactive previews.
- Show component bounds, computed styles, cascade trace, runtime tree, event trace, overflow and diagnostics in the same way as a normal Canvas selection.
- Provide **Reset parameters**, **Reset slot content**, **Reset preview state** and **Reload definition** actions. A failed definition reload retains the last valid preview and reports the new errors.
- Let the developer compare the current saved definition with the unsaved editor buffer. The preview must clearly indicate whether it represents saved source or an unsaved buffer.

**Authoring and reuse workflow**

- Create a component file from the current selection or from a template. Validate the component tag, parameter declarations, defaults, required parameters, slots and import references before replacing any source selection.
- Edit the component definition directly in the XML/CSS editor while keeping the Workbench preview live. Source edits remain the only persisted representation.
- Allow parameter values to be promoted into declared component parameters, and allow repeated literal values to be proposed as parameters; show the exact source diff before applying.
- Show all usages and their supplied parameters. Selecting a usage navigates to the owning document and selects the correct component instance, including nested/duplicated instances.
- Add **Copy instance markup**, **Insert instance into document**, **Create fixture from this setup** and **Export documentation/example** actions. Generated markup must use the engine's actual syntax and must not invent tags or CSS properties.
- A component gallery page may be generated as a temporary preview arrangement, but it must remain preview-only unless explicitly saved as an ordinary `.xml` document.

**Safety and undo**

- Temporary playground edits are session-local by default. Make the distinction between temporary preview data and source edits visually obvious.
- Any action that changes a component definition, its imports, or consuming documents must be an explicit source edit represented by a grouped `QUndoCommand` transaction.
- Deleting or renaming a component must show all known consumers and update only references that can be resolved unambiguously. Never silently break imports or rewrite ambiguous tags.
- Rebuild the catalogue after source saves, external file changes and engine hot reload; preserve selection by stable component/source identity where possible.

**Check 6.4h:** open a global component with required and optional parameters plus default/named slots. Preview it without opening a game screen, change parameters, insert slotted content, exercise supported input states, edit its definition and confirm live preview reload. Verify temporary playground changes do not touch game files, and verify saved edits, extraction and rename operations are undoable and update Find Usages.

### 6.4i. Responsive Preview Matrix

Provide a **Responsive Preview Matrix** that displays the same document/component under multiple viewport profiles side by side. Its purpose is to reveal clipping, overflow, awkward flow changes, text expansion and alignment problems before they reach a game build.

**Preview profiles**

Each named `StudioPreviewProfile` stores:

- profile name and optional description;
- viewport width and height/aspect ratio;
- orientation and optional safe-area insets, where the host/platform exposes them;
- UI scale and text scale, kept as separate controls;
- locale and theme;
- simulated input mode (mouse/keyboard/gamepad) and optional deterministic input scenario;
- optional captured-scene/neutral/checkerboard background selection;
- whether layout/overflow/debug overlays are enabled.

Ship editable presets for common aspect ratios including 16:9, 16:10, 4:3, 21:9, portrait and a custom profile. Do not equate desktop window DPI with the engine's UI units. The renderer's 1080p-tall UI target and one-UI-unit convention remain unchanged (README D10); a profile changes the engine viewport/aspect supplied to the preview, not the target-resolution policy.

**Rendering and isolation**

- Every tile must display output from the selected game's actual engine build and layout implementation. Do not calculate a second layout in Qt, scale a single screenshot to impersonate other viewports, or approximate CSS behaviour.
- Use one selected-game runtime process by default. Multiple matrix tiles are separate preview sessions/contexts hosted by that runtime where supported; do not launch one full game process per tile or start extra engine threads to make the matrix work.
- Where the engine supports independent roots/contexts, create an isolated preview root/context per profile so each tile has its own layout, focus, hover, capture, variables and input state. Reuse loaded definitions/resources where safe, but do not share mutable per-instance state between profiles.
- Use distinct render targets/view submissions for tiles, respecting the renderer's explicit `viewId` contract and bgfx frame ownership. Never cache view IDs across frames or assume the UI pass is first/last.
- Before the independent-context capability exists (phase 5), keep one authoritative interactive preview. Additional profiles must be evaluated one at a time through the real engine and shown as clearly labelled captures; they must not be presented as simultaneously interactive live previews.
- If simultaneous independent roots cannot be implemented without violating the current engine update/finalize/snapshot lifecycle, use deterministic sequential profile evaluation and document that limitation. Do not weaken the threading contract or mutate a frame snapshot to obtain a shortcut.
- All engine calls and profile/context updates occur on the documented EditorHost/Qt GUI thread. Qt worker threads may perform image-diff calculations or file indexing only on copied data, never call UI APIs.

**Matrix interaction and diagnostics**

- Add/remove/reorder tiles; save named profile sets per project/workspace; reset to a standard set; duplicate and edit profiles.
- Select one tile as the active input target. Optionally broadcast the same deterministic scripted input sequence to all profile contexts; pointer positions must be mapped per profile and each context must retain independent focus/capture state. Do not broadcast ordinary live mouse events by default.
- Support synchronised zoom/pan for viewing and independent zoom/pan for inspecting a single tile. Clearly distinguish preview frame size from the visible tile size caused by Qt scaling.
- Show per-tile viewport dimensions, locale, UI scale, text scale, engine reload state and profile name.
- Overlay element bounds, content boxes, clipping, focus path, text baselines and overflow markers per tile.
- Highlight elements that overflow, clip, collapse unexpectedly, become unreachable or exceed a profile's safe area. Link each finding to the same source element in Hierarchy/Inspector/Code.
- Provide an element comparison mode: select an element once and show its rect, measured text, line count, computed values and visibility across all profiles. Values must come from each engine preview context.
- Provide side-by-side screenshot comparison and difference heatmaps for a chosen profile pair. Ignore Qt chrome and compare only the engine-rendered UI target; allow explicit masks for intentional dynamic regions.
- A changed viewport profile invalidates only that profile's preview/layout state. Preserve other profiles and selection where stable.
- Expose the same profiles to `.uitest` and visual regression tests so a profile used for manual review can be run in CI/headless test mode. Baselines are keyed by game/build/backend/profile/locale/theme/text scale and are changed only with explicit approval.

**Check 6.4i:** open a representative page in at least 16:9, 4:3, 21:9 and portrait profiles. Verify each tile uses real engine layout at the selected viewport, the UI target convention stays unchanged, and overflow markers link to source. Change locale/text scale and rerun the profile set. Before phase 5, verify additional profiles are labelled sequential captures; after phase 5, verify independent interactive contexts do not share focus, hover or variables. Capture and compare all configured profiles using the test runner.

---

## Task 6.5: debugging, validation and runtime inspection

This task is a first-class part of Studio, not a future optional add-on. Implement the engine-side data/commands and the Studio panels that consume them. Runtime inspection is read-only unless the developer explicitly uses a control such as Force State or Set Variable.

### 6.5a. Engine-to-Studio inspection commands

Add/extend these console commands. They must work in the selected game's runtime preview and remain useful from the ordinary in-game console:

| Command | Required behaviour |
|---|---|
| `ui_computed <id-or-path>` | Emit JSON for the selected element: source reference, tag, classes, state flags, rects, transform, computed values and winning declaration locations. |
| `ui_force_state <id-or-path> <state> <0/1|clear>` | Force a supported pseudo-state for preview/debugging; `clear` removes the override and restores normal runtime state. Clearly mark forced values. Do not expose impossible states as if they were real input. |
| `ui_pick_at <x> <y>` | Hit-test the given UI-target coordinate and log the exact instance/source path, tag, scoped ID path and file/line/column. |
| `ui_pick` | Arm the next preview/game click for picking; keep the in-game command from README decision D5. |
| `ui_dump [depth]` | Preserve the phase 1 tree dump, including layout rects, classes, pseudo-states and legacy children. |
| `ui_dump_json [depth]` | Emit a structured tree with stable element references, parent/child order, layout, style flags and source locations. |
| `ui_trace <categories> [id-or-path]` | Enable filtered event/input/layout/style/hot-reload/paint diagnostics; `off` disables tracing. Avoid unbounded per-frame log spam. |
| `ui_stats [verbose]` | Preserve phase 0 stats; verbose mode includes per-pass and, when available, cached-layer and scene-view counters. |
| `ui_stats_reset` | Reset Studio profiling counters without resetting live UI state. |
| `ui_overflow_report` | Preserve phase 4 overflow reporting, with element/source references. |

Machine-readable output must be a single complete JSON payload per command result (or a clearly delimited log record), not a JSON-like string mixed with normal prose. Escape paths/text correctly. Do not expose raw pointers as identifiers.

`ui_force_state` must be safely scoped to the preview session and must not leak into normal gameplay after Studio closes. It must not bypass validation or mutate the source markup. If a forced state is unsupported by the engine build, return an explicit error.

### 6.5b. Canvas debug overlays

Provide independently toggleable overlays:

- element border-box and content-box bounds;
- padding and border bands;
- absolute transform axes/origin;
- clipping rectangles and rounded clip boundaries;
- z-index/paint order numbers;
- hit-testable regions and the result of the last hit test;
- current hover, active, focused and focus-within elements;
- focus-trap boundary and navigation links (`nav-up/down/left/right`);
- pointer capture owner and pointer coordinates;
- collapsed/hidden state and the ancestor responsible for inherited visibility;
- text line boxes, baselines, glyph bounds, wrap width and ellipsis/shrink status;
- render chunk bounds and legacy-child markers when available.

Overlays must be drawn in the Qt layer and must not change runtime painting or the golden screenshot. Add a **Capture without editor overlays** option for accurate screenshots.

### 6.5c. Runtime tree, style and event debugger

Provide dedicated panels or tabs for:

**Live Tree:** inspect the actual runtime tree beside the source DOM. Show document/component instance boundaries, legacy elements, current parent, visibility and source owner. Selecting a runtime element selects its source node when possible.

**Computed Style / Cascade:** inspect computed properties and the exact winning rule. A cascade trace must explain why a declaration wins or loses. Display inherited origin, custom property expansion and invalid-value fallback. Clicking a source location opens the corresponding CSS or XML document.

**Variables and Classes:** list variables visible from the selected element (self and ancestors), their values and their defining scope. Allow setting/removing a temporary preview variable and toggling a code-added class. Runtime overrides must be visibly marked and resettable.

**State Inspector:** show actual and forced `hover`, `active`, `focus`, `focus-within`, `disabled`, `selected`, `checked`, `entering`, `leaving` and later-phase states. Do not claim a state is active merely because it is highlighted in the editor.

**Event/Action Trace:** show event name, sender, pointer/action source, resolved handler (element/ancestor/code-behind/built-in), arguments, dispatch order, duration and any unresolved action. Include a pause-on-next-event option for debugging a specific element/event. Never execute a handler twice just because Studio is listening.

**Input/Navigation:** show last input device, simulated device, focus target, focus trap, navigation candidate order, hotkey matches, pointer capture, mouse/touch coordinates and consumed actions. Provide controls to simulate keyboard/gamepad actions through the same engine input path as tests.

**Console and Logs:** provide a docked console that sends commands through the authenticated runtime bridge to the selected game's existing `Console::Get().Execute` path. Include command history, completion for registered commands, clear/copy, severity/category filters, pause-on-error, follow-tail and clickable `[UI] path:line:col` locations. It must distinguish a command accepted by the engine from an operation that later failed; never display a false success state. Keep engine logs available after preview reloads.

**Layout Diagnostics:** show measured/arranged sizes, available parent content size, margins, padding, min/max constraints, flow allocation, fill weights and percentage resolution. For a suspicious dimension, show the calculation or rule that produced it. Make fit-children/percentage and fill-parent-flow warnings actionable.

### 6.5d. Render and performance debugger

Build a Profiler panel around `ui_stats` and available engine timing data. At minimum show:

- per-frame UI update, style resolve, layout, paint recording, finalize/upload and submit time when measurable;
- root count, element count, dirty roots, style/layout/paint rebuild counts;
- primitive, vertex, index, geometry draw, external command and batch-break counts;
- batch breaks by texture/state/stencil/external/capacity and phase 3 merges;
- font/image atlas pages, occupancy, overflow/fallback textures and pending uploads when available;
- phase 5 world-surface update/render counts and skipped surfaces;
- phase 7 layer-cache hits/misses/rerenders and atlas occupancy;
- phase 8 scene-view target count/rerenders when available;
- per-root/per-element recorded command counts and approximate bounds where available.

Add a frame-history graph with selectable frames, minimum/average/maximum and a way to inspect spikes. Selecting a frame must not attempt to access freed transient GPU buffers; retain CPU-side counters/snapshots only. Do not add expensive per-element timing to normal release builds; detailed tracing is opt-in and visibly marked as intrusive.

Add toggles for paint order, batch boundaries and chunk bounds. These are diagnostic overlays, not changes to renderer ordering. A profiling session can be exported as a compact JSON/CSV file under Studio's output folder.

### 6.5e. Diagnostics and quick fixes

The Diagnostics panel merges engine markup messages, lint output, build/compiler errors and test failures. Each item shows severity, code, file, line, column and message; double-click navigates to source. Provide filters for errors/warnings/info, current document/project, source type and phase.

Where safe, offer quick fixes:

- create a missing file/tag only when the schema has an obvious valid template;
- add a missing required component parameter with its declared default or prompt for a value;
- correct a known misspelling of a registered tag/property/action only after showing the proposed edit;
- add an import for a discovered component/stylesheet;
- navigate to the declaration for a missing variable/selector;
- choose a valid enum value from the schema.

A quick fix must be a normal undoable document command. Do not auto-correct unknown attributes, change selector specificity or replace an asset path without explicit user selection.

**Check 6.5:** pick elements through the Canvas and `ui_pick_at`; verify source navigation resolves duplicate IDs in different component instances. Inspect computed CSS, force hover/focus temporarily, set a variable, trace an activation and inspect `ui_stats`. Confirm clearing every temporary override restores the original runtime state and does not modify any source file.

---

## Task 6.6: testing workbench, build/run controls and visual regression tools

A UI developer must be able to build the selected game, run its `-ui_studio` preview host, author and run tests from Studio without manually opening a terminal. Studio must reuse the phase 0 tools and `UiTestRunner`; do not write a second screenshot-diff implementation.

### 6.6a. Build and run controls

Provide a Build/Run toolbar and output view with:

- configure/build/rebuild actions when the source repository/build folder can be resolved from optional UIDB build metadata or a developer-selected source root; never guess a source checkout from GameData alone or invent a new build folder convention when one already exists;
- a build target selector for the selected game and available Studio targets; hide or mark source-build actions unavailable when only a packaged GameData folder/runtime executable is available;
- a Run/Stop action for the selected game's executable and a separate runtime-preview restart action;
- build output streamed through `QProcess`, with compiler errors parsed into clickable file/line diagnostics;
- a clear indication of configuration, executable path, selected renderer backend and whether the binary is stale compared with source;
- a safe reconfigure action when the CMake source set changes (the engine's source glob requires re-running configure after adding `.cpp` files);
- shader compilation action for the UI shader source when those assets are changed: sync canonical engine assets first, then run `ShaderCompilationAutomation.exe` for each of the four in-scope games and report every `[COMPILED]` / `[FAILED]` line. Never compile or sync `HorrorGame`;
- no automatic build on every XML/CSS save. Source hot reload is the normal iteration path; build only when C++/CMake/shader changes require it.

A test must not start against an executable that is still building or whose last build failed. If a build fails, preserve the current running preview if it is still healthy and show the build failure separately.


### 6.6b. Test Explorer and script editor

Provide a Test Runner panel that:

- discovers `*.uitest` scripts under `source/Engine/UI/Test/Scripts/` and the selected game's test folders;
- groups tests by game, phase, feature and script name; shows last result and last run date for each;
- opens `.uitest` scripts in a syntax-highlighted editor with command/argument completion based on the commands registered by the current engine;
- shows inline help for `wait`, `wait_frames`, `action`, `console`, `screenshot`, `ui_screenshot`, `mouse_move`, `mouse_down`, `mouse_up`, `click`, `mouse_off`, `expect*`, `set_var`, `set_class` and any later registered command;
- has controls for selected game, build configuration, backend(s), test script, label, timeout and optional capture comparison;
- displays live process output, per-backend status, command-level `result.txt` failures, structured `result.json` data and produced screenshots;
- supports cancellation and reports whether a process was terminated or completed;
- opens the output folder and copies the exact command line used.

Keep the existing `.uitest` syntax and result text backwards-compatible. Extend `UiTestRunner` to write a machine-readable `result.json` companion containing script path, game, backend, start/end, pass/fail, failed line numbers and capture names. Do not replace `result.txt` or change its final `RESULT PASS` / `RESULT FAIL` line.

### 6.6c. Running tests and backends

Use `Tools/ui/run_ui_tests.py` for the established `run`, `mask`, `compare` and `cross` workflows. Launch it through `QProcess` so Studio remains responsive and can stream stdout/stderr. Do not run a second copy of the engine inside a Qt worker thread.

The UI must support:

- run one script on one backend;
- run one script on all backends (`dx11`, `dx12`, `vk`, `gl`);
- run a selected suite against the selected project's game runtime;
- run the same suite across all four in-scope games when their build targets exist;
- compare a new run against a chosen baseline with a selected tolerance and optional mask;
- cross-backend comparison against a chosen reference backend;
- generate a run-to-run noise mask from two baseline runs;
- rerun only failed scripts/backends;
- stop a running test process cleanly and mark interrupted results distinctly from test failures.

Do not attempt to build `HorrorGame`. Do not imply that a backend passed when it was skipped or its executable was unavailable; represent `not built`, `not run`, `failed to launch`, `timed out`, `cancelled` and `test failed` as different states.

### 6.6d. Screenshot and image-diff viewer

For every capture, show:

- the baseline, new image, generated diff and optional alpha/channel view;
- a wipe slider and opacity overlay to compare baseline/new;
- zoom/pan, pixel coordinates and sampled RGBA values;
- image dimensions and backend/game/script metadata;
- failing pixel count, max channel difference, bounding box and tolerance;
- optional mask overlay and ignored-pixel count;
- quick navigation to the `.probes` file when one exists.

Provide a probe editor for `ui_image_probe.py` files: click a pixel in the capture to fill coordinates and sampled RGBA; allow manually specified expected channels/tolerance/label; run the probe against every backend and show pass/fail rows. Preserve the existing plain-text probe format.

**Baseline safety:** baseline creation/replacement is an explicit action with a confirmation and a visible source run. Never silently bless the latest output. Keep per-project/per-backend baselines separate; the phase 0 rule still applies that legacy content is compared against its own backend baseline, while new UI content is expected to be cross-backend equal unless a documented pre-existing exception exists.

### 6.6e. Test authoring helpers

Add practical helpers without inventing a new test language:

- **Insert assertion:** from a selected runtime element, insert an appropriate `expect`, `expect_rect`, `expect_exists` or `expect_missing` command into the open script;
- **Capture current state:** insert a `ui_screenshot` command with a unique, readable stem;
- **Insert interaction:** insert `action`, `click`, pointer or simulated-device commands;
- **Run from cursor:** create a temporary script containing the required setup and selected test lines only when setup dependencies are known; otherwise disable and explain;
- **Open failure:** jump from a failed assertion to its line in the `.uitest` editor and select its target in the live preview where the preview is still running.
- **Create component fixture:** take the Component Workbench's current parameter values and slot markup and create a normal `.xml` fixture or in-memory test setup. Do not persist temporary playground changes unless the developer explicitly chooses to create a fixture.
- **Select preview profiles:** add saved Responsive Preview Profiles to the test run and generate one capture/assertion result per profile. Show the profile name and viewport metadata in every result.
- **Insert localisation expectation:** choose a key and locale through the localisation picker, then generate an assertion against the engine's displayed, fully substituted text. Do not compare the raw `${KEY}` expression unless the test explicitly requests a source-level assertion.

Optional interaction recording may record discrete clicks, pointer presses/releases and named input actions into supported `.uitest` commands. It must not record every pointer-motion frame by default, and the output must be reviewable before insertion.

### 6.6f. Test coverage expected from the Studio workflow

Use the plan's actual test suites as built-in examples and run configurations:

- phase 0: legacy-menu goldens and painter-card probes;
- phase 1: layout, visual, text, component scope, errors, buttons and game-thread API;
- phase 2: menu stack, events, mouse capture, prompts and StealthAction pause pilot;
- phase 3: paint effects, text effects, motion and page transitions;
- phase 4: widgets, settings binding and per-game kit menus;
- phase 5: layers, world panel, world surface, billboard and loading screen;
- phase 7: blur, drop-shadow, group opacity, backdrop, layer cache, SVG and drag/drop;
- phase 8: scene view basic, variables and orbit.

Tests that are unavailable in the current build are displayed as gated by their prerequisite phase, not as failures.

**Check 6.6:** build and run the selected game from Studio, then run a visual test on all four backends from Studio, compare against its baseline, inspect a diff, create a noise mask from two runs and run a probe file. Confirm the GUI remains responsive, the result folder matches `Tools/ui/run_ui_tests.py` output, and the baseline changes only after an explicit approval action.

---

## Task 6.7: asset, localisation, accessibility and project quality-of-life tools

### 6.7a. Asset browser

Provide an asset browser rooted at the selected game's actual `GameData` and the engine's canonical UI assets. It should support:

- thumbnails for image/texture assets, with checkerboard transparency and dimensions;
- searchable paths and file types, including fonts, sounds, videos and SVG when phase 7 exists;
- a preview and metadata panel; image alpha/channel inspection where practical;
- copy path, insert path into the selected attribute/CSS field, reveal on disk and find references;
- missing/unresolved assets grouped from lint output;
- warning indicators for assets larger than the 512 px UI-atlas threshold or resources that exceed a documented atlas/page budget;
- clear source ownership for canonical engine assets versus per-game assets.

Asset browsing must not import, copy or convert an asset just because the user previews it. Any import/copy operation must be explicit and undoable where source references change.

### 6.7b. Localisation browser, picker and in-Studio string editor

Localisation must be a normal authoring workflow, not a list of key names that developers are expected to memorise. Developers must be able to find, insert, create and edit strings without leaving Studio.

**Compatibility and source-of-truth rules**

- Before implementing the editor, inspect the actual `Localisation::Initialize("GameData/localisation", "en")` implementation, `Localisation::LocalizeString`, the existing localisation files in each in-scope game and their real encoding, key rules, fallback order and file organisation.
- The existing runtime localisation format and lookup behaviour are authoritative. Do not introduce a new JSON/CSV/INI format, a parallel translation database, a Studio-only key syntax or a different fallback algorithm.
- If the runtime API cannot enumerate keys, locale values or source locations, expose the required metadata through a small shared localisation data/model API or refactor the existing parser so runtime and Studio use the same parser and validation rules. Do not maintain two parsers that can disagree about valid keys or values.
- Preserve comments, key order, whitespace, quoting/escaping and newline convention where the existing format permits it. Use source spans for targeted edits; do not rewrite unrelated entries when editing one string.
- Keep the runtime expression exactly `${KEY}`. Do not auto-convert it to `{KEY}` (which is a variable expression), alter the localisation-before-second-variable-substitution order, or apply localisation inside CSS declarations, IDs, classes or other fields that the runtime does not localise.
- Read/write localisation files using the existing VFS/project conventions. Studio edits the selected game's source localisation files, never the packaged/runtime copy or another game's files by accident.

**Localisation browser and key picker**

Provide a searchable, keyboard-friendly browser/picker available as a dock panel, popup and context action from a localisable XML text/attribute field.

- List keys with their active-locale value, source locale, target-locale availability, reference count and validation status.
- Search by key, displayed text, partial phrase, locale value and document usage. Support case-insensitive fuzzy search and exact-key lookup.
- Filter by current document, current project, referenced/unreferenced, missing translation, missing key, recently used and favourites.
- Show matching results as `KEY — translated preview`, not just opaque key identifiers. Display the selected locale and fallback source when a value is inherited/fallback-resolved.
- Preview a selected key in the active preview locale and, where available, compare its values across locales.
- Show recent keys, pinned/favourite keys and keys used near the current source location.
- Include **Insert selected key**, **Insert and keep picker open**, **Copy `${KEY}`**, **Open translation entry** and **Create new string** actions.
- In XML Code, provide context-aware completion for `${...}` and a picker at the caret. It must show key/value suggestions, mark missing keys and insert the canonical `${KEY}` expression. Completion must not trigger or replace ordinary `{variable}` expressions.
- In Canvas/Inspector, a localisable field gets a resource-picker-style control with a search button. The developer can choose a key rather than typing it. Fields not passed through `UiSubstitute`/`Localisation::LocalizeString` must not show this control.
- In the Properties/Inspector text editor, provide a clear toggle between literal text and a localisation reference. Switching to a key requires an explicit selection or creation; never silently turn arbitrary text into a key.

**Create and edit strings without leaving Studio**

Add a dedicated localisation table/editor inside the application. It must be possible to create a new key and edit existing translations directly in Studio, including while a layout is open.

- Provide **New String** with a key field, source-locale value, optional description/comment where the existing format supports it, and a preview of the exact markup reference `${KEY}`.
- Suggest a key from the selected text or nearby element context, but let the developer edit it. Validate uniqueness and allowed key syntax using the existing loader's rules. Do not impose a new naming convention that conflicts with current projects.
- Provide an editable locale table: one row per key, one column per configured locale, with explicit states for translated, empty, missing, inherited/fallback and invalid values. Double-click or press Enter to edit a cell inline; support multiline values when the source format allows them.
- Support add/remove locale configuration entries only where the game's existing localisation loader supports them. Creating a translation must not imply adding a new runtime locale if the project does not configure it.
- Allow editing the selected key's value in an inline detail editor with source-locale and target-locale tabs, previous/next key navigation, unsaved markers, validation feedback and live preview.
- Provide **Add translation**, **Copy source into translation** (explicit user action), **Clear translation**, **Revert cell**, **Revert key** and **Compare locales**. Do not automatically fill a missing translation with the source value; preserve missing/fallback status until the developer chooses to copy or translate it.
- Support filtering the table to keys referenced by the current document/selection, enabling rapid translation while working on a screen.
- A new key can optionally be added to configured locale files as an empty/missing entry, but only after the developer selects the target locales. Make the effect on every file explicit in a multi-file change preview.
- Integrate localisation edits into Studio's undo/recovery system. A cell edit is one undo item; creating a key across several locale files is one grouped transaction. Save atomically per file, preserve unrelated text, and use the existing external-change conflict workflow.
- Saving a translation refreshes the localisation model and preview without requiring a full game restart. If runtime reload cannot yet refresh localisation data, show the limitation and offer a controlled preview reload; never claim that a stale preview reflects the saved value.

**Insert and extract strings from markup**

- From a localisable field, choose an existing key and insert `${KEY}` at the caret or replace the selected literal text. Preserve surrounding markup, attributes, variables and formatting.
- Add **Extract to Localisation…** to the XML editor and Canvas/Hierarchy context menu when a selected literal text node or a known localisable attribute is eligible. The dialog proposes a key, captures the selected literal as the source-locale value, allows target locales to be selected, shows the exact XML diff and all localisation file changes, then replaces only the selected source span with `${KEY}` after confirmation.
- For `Label` text, `TextButton` text, `tooltip` and other fields, enable extraction only after confirming the field flows through the existing substitution/localisation path. Do not offer extraction on `id`, `class`, `style`, CSS values, resource paths or unsupported attributes.
- When the selected text contains `{variable}` placeholders, preserve them exactly in the source-locale value and warn that every translation should preserve the required placeholders. Show detected placeholders and validate that translated values contain the same placeholder names unless a project-specific rule says otherwise.
- Treat `${KEY}` as an existing localisation reference, not as literal text to be extracted. Detect missing-key references and offer **Create missing entry for this key** rather than generating a different key silently.
- For attributes containing a mixture of literal text and variables, preserve the full expression and let the developer choose the extraction boundary. Never flatten a dynamic value into a static translated string.

**Validation and project reports**

- List references from XML by key, document, component definition/instance and source location. Find References on a key must navigate to every known `${KEY}` use.
- Detect missing keys, duplicates, malformed entries, invalid escapes, empty source values, unreferenced keys and locale coverage gaps using existing format rules.
- Detect placeholder mismatches across translations (`{name}`, `{count:%.1f}`, etc.) and report exact key/locale/placeholder.
- Provide a locale coverage report and a per-document localisation report. Unreferenced keys are warnings/information only; never delete or rewrite them automatically.
- Provide a **Review localisation changes** view showing added/modified/deleted keys, affected locale files and source references before a multi-file save/refactor.
- Update live preview, overflow diagnostics and visual regression profiles when a key value changes. Allow switching preview locale without changing the game's saved language setting.

**Check 6.7b:** open `ui/kit/pages/pause.xml`, pick a known key by searching its displayed text and insert `${KEY}` into a localisable field. Create a new key from selected literal text, add a translation in a second configured locale, edit that translation inline, save, undo/redo and hot-reload. Verify comments and unrelated entries remain unchanged, all references update, placeholder mismatches are diagnosed, missing translations remain visibly missing, and the preview displays the chosen locale. Confirm that no localisation picker is offered for CSS, IDs or other non-localisable fields.

### 6.7c. Accessibility and controller review

Inspector and diagnostics should help developers review:

- visible text/name for interactive controls;
- focusability, disabled state and default focus;
- keyboard/gamepad focus order and explicit `nav-*` relationships;
- controls that are difficult to reach without a pointer;
- focus indicator visibility in the current theme;
- text size and clipping at normal and enlarged text scale;
- colour contrast for text against a known solid background when both colours can be resolved unambiguously (report as advisory, not a definitive result for gradients/images/transparency);
- pseudo-localisation expansion and text overflow.

Add a keyboard-only preview mode: mouse selection is disabled, all focus/navigation actions are shown, and the current focus path is visible. Simulate gamepad navigation through the engine's input action path, not by directly changing `UiNavigation::Focused`.

### 6.7d. Theme and reusable-style workflow

Add a custom-property/theme view showing `--*` variables in the selected document and imported stylesheets. Display resolved colours, radii, spacing and font tokens as swatches/values. Support edit, rename and find references. Renaming a token must update only proven references and must be one undoable project refactor.

Provide reusable templates/snippets for Panel, Label, Image, Button, TextButton, component definitions and common layout patterns. Templates must be valid for the current schema and must not introduce unsupported CSS vocabulary such as `display: flex`.

### 6.7e. Documentation and command discovery

A built-in Documentation panel is generated from schema descriptions and the relevant phase docs. It should provide quick references for:

- supported markup tags/attributes and component parameters;
- CSS property grammar, inheritance and invalidation;
- selector and pseudo-class syntax;
- variables, localisation and component slots;
- actions/events and host options;
- available `.uitest` commands;
- engine debug commands and the selected game's asset roots.

A command palette and shortcut reference must expose all Studio actions and panel visibility commands. Search results should include keyboard shortcuts and the panel/section where the command belongs.

**Check 6.7:** browse and insert an image path, search for and insert a localisation key by its displayed text, create and edit a key/translation in Studio, preview the chosen locale and pseudo-loc/text scale when phase 4 is available, locate a missing resource from lint and fix it, and rename a CSS custom property with undo/redo.

---

## Task 6.8: preview catalog, runtime targets, hosts and later-phase integrations

### 6.8a. UIDB Preview Catalog

The Preview Catalog is not limited to XML files. Display all UIDB `previewTargets` grouped by kind/category: documents, components, built-in/legacy elements, game runtime widgets, hosted HUD elements, world surfaces and scene views. Each card shows description, source references, host requirements, required capabilities, fixture/scenario availability and whether it supports read-only live capture.

Selecting a target asks the runtime bridge to create that target using its registered provider. Show typed fixture controls from UIDB and scenario presets. Changes affect only the isolated preview. A missing/unavailable provider is visible and actionable; do not create a generic fake rendering. Add examples in the acceptance fixtures for a player crosshair and HP bar: crosshair scenarios include idle/hip-fire/aiming/target/hit; HP bar scenarios include full/half/critical/shield/damage/healing. The exact fields and scenario names come from each game's UIDB, not Studio hardcoding.

### 6.8b. Main viewport preview

- Load the active document into the selected game's runtime using the existing preview/load path (`ui_load` or the equivalent typed runtime-bridge operation).
- Provide reload, unload, refresh assets and reset preview state.
- Provide a selectable preview background/scene. For the basic phase 1 preview, an empty/neutral game viewport is sufficient; for phase 4, preview the shared kit pages with the selected game's real theme and code-behind.
- Allow the selected document to be shown as a standalone root, pushed onto the real menu stack or placed inside a test host where supported.
- Keep the existing game content available as the background/host when useful; provide a clear control to switch between game scene and neutral preview.

### 6.8c. Stack preview

Use the real `UiMenuStack` when phase 2 is available:

- push the active document as a page;
- back/pop/replace and show current stack depth/top page;
- inspect `pauses-game`, `hides-hud`, `shows-below` and `close-on-back` behaviour;
- simulate `ui_up/down/left/right/confirm/cancel`, hotkeys and input-device changes;
- show saved/restored focus and focus traps.

The Studio preview should use the dedicated `-ui_studio` runtime mode; a read-only live-capture connection to normal gameplay is optional and must be visibly labelled. Do not silently alter a game's actual persistent settings while previewing the kit settings pages. Use test settings or restore a snapshot when the preview closes.

### 6.8d. Host preview

When phase 5 is available, expose the phase 5 test terminal/world surface and allow a document to be previewed in that real host context. Show the host's independent `UiContext`, surface dimensions, pixels-per-metre, facing and interaction mode. Test aim interaction, focus entry/exit, pointer capture, occlusion and out-of-range state through the actual engine. Do not fake a world surface by simply drawing the document in the main viewport.

### 6.8e. Later integrations

- **Phase 7:** consume `SaveData/ui_pick.json` and select the matching source element when the game writes it. Add a pick-in-game action and a link back from the element to its Studio document. Add layer-cache/atlas diagnostics when the engine exposes them.
- **Phase 8:** add `SceneView` inspection and preview controls only after the engine's approved scene-view entry point exists. Show camera/model/light attributes and target size; do not implement a second 3D renderer in Studio.

**Check 6.8:** preview a basic document, component, native element, game-owned runtime target (at least a crosshair or HP-bar fixture), menu-stack page and (when phase 5 is complete) a test world surface. Verify each uses its UIDB-declared provider/host/context and the real runtime, that fixture changes are isolated, and that the active preview mode is obvious.

---

## Task 6.9: usability, reliability and keyboard workflow

The following are acceptance requirements, not decorative polish:

- global Undo/Redo (`Ctrl+Z` / `Ctrl+Y`), Save (`Ctrl+S`), Save All (`Ctrl+Shift+S`), Find (`Ctrl+F`), Find in Project (`Ctrl+Shift+F`), command palette (`Ctrl+Shift+P`), build (`F7`), run selected test (`F6`), run current document lint (`Ctrl+F8`), and toggle Design/Interact mode (`F9`); avoid assigning shortcuts already used by the game preview viewport without a mode-aware routing rule;
- dockable panels, resettable workspace layout and per-project/UIDB-identity workspace persistence;
- clear unsaved/externally-modified/preview-stale indicators;
- undoable operations and multi-step transactions; destructive operations require confirmation;
- non-blocking build/test processes with streamed output and cancellation;
- errors that navigate to exact source positions;
- no modal popup for ordinary warnings or each hot reload; use Diagnostics/Output and a small notification area;
- settings for theme (light/dark/system), font size, editor tab width, line numbers, autosave/recovery, snap distance, canvas background and default test backend;
- proper DPI scaling and high-DPI Qt displays without changing engine UI units;
- resizable layouts and useful behaviour at 1280×720 and 1920×1080 desktop sizes;
- remember the last selected GameData project/document but warn clearly when UIDB or runtime build identity changes;
- a **Safe Mode** launch option that opens Studio without auto-loading the last preview document or running project lint, useful after a broken document or asset crashes preview;
- crash/recovery handling: preserve dirty buffers to recovery files and restore them on the next launch, without claiming they were saved to source;
- a status bar showing active game, active document, preview state, UIDB freshness, runtime connection/lifecycle state, build configuration and selected backend.

**Check 6.9:** complete a normal edit/test workflow using only the keyboard. Restart Studio and verify workspace recovery. Simulate an external file modification and verify conflict resolution. Confirm a broken document does not prevent opening Studio in Safe Mode.

---

## Task 6.10: retire the old editor and JSON `.ui` system

Only do this after the new Studio can open, edit, save, undo, hot-reload, lint, inspect and test a real game document.

1. Search the entire in-scope engine/game code for references before deleting anything:
   - `LoadLayoutFromFile`, `OwnedLayout`, `MemberOfLayout`, `FindNamed`, `UiHotReload`, `ui_editor_*` console commands;
   - `UI/Editor/`, `UI/Layout/`, `LayoutBlueprint`, `UiLayoutSerializer`, JSON `.ui` assets and editor-specific build entries.
2. Delete `source/Engine/UI/Editor/`, `UI/Layout/` (blueprint, serializer, hot reload and loaded instance) and every obsolete `.ui` JSON layout in the four in-scope games only after the reference search is clean.
3. Keep `UI/Reflection/`; legacy tags in markup still use it.
4. `UiLayoutSerializer::ApplyProperties` was used by the phase 1 builder. Move the required function into `UiBuilder.cpp` before deleting `UI/Layout/`; preserve its legacy property conversion behaviour.
5. Remove obsolete members from `UiElement` only after proving no remaining call sites depend on them.
6. Remove `ui_editor_*` commands and their registrations. Do not remove `ui_pick`, `ui_dump`, `ui_reload`, `ui_computed`, `ui_force_state` or other runtime debugging commands needed without Studio.
7. Keep the small in-game inspector required by README decision D5: `ui_pick` (next click logs the picked element's path and `file:line`) and `ui_dump`.
8. Update the relevant Visual Studio project entries and CMake sources. Never reformat or stage unrelated user changes.

**Check 6.10:** grep again for every obsolete symbol/path. Build all four in-scope games and Studio targets. Run all earlier phase regressions on every required backend. Launch StealthAction Studio (ask the owner before launching the GUI if the current workflow requires approval), open `ui/kit/pages/pause.xml`, edit through Canvas, Inspector and Code, save, undo/redo and confirm the game hot-reloads the change.

---

## Definition of done

Phase 6 is complete only when all of the following are true:

1. Every normal game launch produces deterministic valid `SaveData/UIStudio/uidb.json`; `-ui_schema` exports the same registry model.
2. `-ui_lint` covers every UI document and referenced stylesheet in each game, with correct source locations and both text and JSON reports.
3. The generic `UiStudio`/`EditorHost` build once with the documented Qt 6.11 MSVC kit; no per-game Studio rebuild is required. `QtEditor` retains its existing build and behaviour.
4. Studio discovers different games from `GameData` + UIDB and uses each selected game's real runtime bridge for preview/layout/input.
5. Canvas, Hierarchy, Inspector and Code remain synchronised; XML/CSS edits preserve unrelated source; undo/redo works after every kind of edit.
6. Live preview hot-reloads successfully and preserves selection/state when possible; invalid edits keep the last valid preview.
7. Runtime tree, computed style/cascade, state/input/event trace, layout diagnostics and render profiling tools work without violating the engine's threading/snapshot rules.
8. The Test Runner can run scripts on one or all backends, show failures and screenshots, compare/mask/cross-compare captures and manage baselines only with explicit approval.
9. Asset, localisation, pseudo-loc, text-scale, accessibility and theme tools work to the extent supported by the current engine phase. Existing strings can be searched by displayed value, inserted by selection, created and translated inside Studio, with source-safe undo/redo and no alternate localisation format.
10. The Component Workbench discovers real components/snippets and UIDB preview targets; it previews documents, components, elements and game-owned runtime widgets (for example crosshairs/health bars) through the actual engine using declared hosts, fixtures and scenarios. It supports direct definition editing and protects temporary playground state from accidental persistence.
11. The Responsive Preview Matrix uses real engine viewports/profiles, preserves the 1080p-tall UI target convention, diagnoses per-profile overflow and integrates saved profiles with visual regression tests. Any pre-phase-5 sequential-capture limitation is explicit.
12. The application remains usable through keyboard navigation, external-file conflicts, preview errors and recovery after an abnormal exit.
13. The obsolete ImGui editor and JSON `.ui` system are removed only after reference checks, while `ui_pick` and `ui_dump` remain available in game.
14. All four in-scope games build and all applicable phase 0–5 regressions pass on the required backends. Later phase tests pass when those phases are already implemented. `HorrorGame` is untouched.

## Implementation notes

Record:

- the Studio/runtime IPC protocol, native-window hosting strategy and engine-thread lifecycle;
- Qt modules and CMake targets used;
- UIDB format/protocol versions, discovery paths and examples of tag/property/action/preview-target entries;
- lint categories implemented and fixture results;
- source-preservation behaviour for XML and CSS;
- any schema limitations caused by legacy/reflection tags;
- Studio panels/tools completed and any capability gates;
- Component Workbench and Preview Catalog implementation, including runtime preview providers, HUD fixtures (crosshair/health bar), host selection, parameter/scenario coverage and temporary-versus-persisted edit rules;
- Responsive Preview Profile format, viewport/context isolation strategy, sequential versus independent preview capability and test-matrix integration;
- localisation source format discovered, shared parser/API used, fallback rules, source-preserving edit behaviour and locale coverage tests;
- test scripts, games and backends run, with pass/fail and baseline references;
- any issue that required an owner decision. A task marked **STOP** still requires owner review before proceeding.
