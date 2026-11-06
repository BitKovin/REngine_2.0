# 00: Conventions every phase relies on

Read all of this before any phase. Every phase file assumes it.

---

## 1. Scope

- Games in scope: `source/Games/ShooterGame`, `SimGame`, `SlasherGame`, `StealthAction`.
- **Never touch `source/Games/HorrorGame`.** Don't sync into it, don't build it, don't port it.
- Engine UI code lives in `source/Engine/UI/`. New code goes into the new subfolders listed in §5.
- Don't change code outside the files a phase lists. If a task seems to need another file, **STOP** and ask.
- The working tree may contain the owner's uncommitted work (for example in `source/Games/StealthAction`). Never revert, reformat or stage files you didn't change.

---

## 2. Threading model (most important section)

How the frame runs today (`EngineMain::MainLoop`, `source/Engine/EngineMain.cpp`):

1. Main thread: SDL events, `Viewport.ResetTouchInputs()`, hit test, and dispatch of `Input::TouchActions` into `TouchEvents` of the hit `UiElement`.
2. Main thread: `Level::Current->LoadAssets()`, `Input::Update()`.
3. Main thread: `UiNavigation::Update(&Viewport)`, `Viewport.Update()`, `UiNavigation::LateUpdate()`, `Viewport.UpdateChildrenOffsetRecursive()`, `Viewport.FinalizeChildren()`.
4. `GameUpdate()` is queued on a **worker thread** (`asyncGameUpdate` is `true` by default). Entities, ECS and game code run there.
5. **At the same time**, the main thread runs `Render()`. It draws the UI (`Viewport.Draw()`) into `UiFrameBuffer`, then the level (where `UiBilboard::PreDraw` draws world UI), then the composite.
6. The main thread waits for the worker, then runs `FinishFrame()` and `Time::Update()`.

Legacy `Draw()` functions only read `finalized*` fields for exactly this reason: game code may change the live fields on the worker thread while the UI draws.

**Rules for all new code.** The model is the same as the legacy one: game code changes the live tree; the main thread processes it and **finalizes a copy** that rendering reads.

- **R1. Processing and finalizing happen on the main thread, in step 3, while the game thread is idle.**
  - `UiSystem::Update()` runs right **before `Viewport.Update()`** (after `UiNavigation::Update(&Viewport)`). It does input and pointer handling, event handlers, `Tick()`, hot reload, style and layout. Legacy children placed inside markup elements are then positioned from this frame's markup layout.
  - `UiSystem::Finalize()` runs right **after `Viewport.FinalizeChildren()`**. It records paint for dirty roots and **copies** each root's draw data into its frame snapshot (`UiFrameData`, phase 1). World surfaces are finalized here too (phase 5).
  - Every bgfx call the new UI makes outside rendering (font and atlas uploads) happens in `Finalize`.
- **R2. Rendering (step 5) reads only frame snapshots.**
  - A snapshot holds the recorded draw list plus `shared_ptr`s to any legacy children it draws, so game code removing an element mid-render can't free something being drawn.
  - `UiElem::Draw()` appends its root's snapshot. It never reads styles, layout, text, variables or the live tree.
  - A snapshot is replaced only in `Finalize`.
- **R3. Game-thread code may use the whole UI API at any time during `GameUpdate`:**
  - `UiElem::Load`, `LoadLayout`, creating elements, `AddChild`, `RemoveChild`
  - `SetVar`, `SetClass`, `SetAttribute`, `Style()`, `Find`
  - reading layout results (they are from the last processed frame)

  These calls only change state and set dirty flags. They never make bgfx calls, never draw, and never touch snapshots, `UiPrimBuffer` or font pages. Changes show up after the next step 3.

  Allowed callers: **the main thread and the game thread itself**. Never call the UI API from parallel jobs: ECS `ParallelFor`, `MainThreadPool` jobs, or the asset loader thread. Every public mutating UI function calls `UiSystem::AssertUiThread()` (debug builds). It checks the current thread is the main thread or the game thread. The game thread id is recorded by `UiSystem::NoteGameThread()`, called at the top of `EngineMain::GameUpdate()`.

  This is safe without locks because step 3 and `GameUpdate` never overlap: step 3 runs after the previous frame's `WaitForFinish()` and before the next `QueueJob`. With `asyncGameUpdate == false`, both run on the main thread.
- **R4. Nothing in the new UI starts threads.** Don't use `std::thread`, the thread pool or async jobs.
- **R5. Handlers (`On(...)`) run on the main thread inside step 3**, while the game thread is idle, so gameplay code they call is safe.

Every phase repeats which rule applies where.

---

## 3. Frame boundaries in the renderer

- `bgfx::frame()` is called in several places. The UI renderer must reset its per-frame buffers **right before** the frames that can contain UI. Phase 0 adds `UiRenderer::OnBeforeBgfxFrame()` and calls it in exactly two places: the main present in `EngineMain.cpp` (the `bgfx::frame()` near the end of the main loop) and `LoadingScreenSystem::Draw()`.
- Never call `OnBeforeBgfxFrame()` anywhere else. Resetting without a real frame in between corrupts the data texture, because bgfx applies every texture update of a frame before any draw of that frame.

---

## 4. Code style

- C++20 (the root `CMakeLists.txt` sets it).
- **Line endings: CRLF**, like every existing engine source file. Keep CRLF when editing. Write new `.h`/`.cpp` files with CRLF (the Write tool writes LF, so after creating a file, convert it: `python -c "import sys;p=sys.argv[1];d=open(p,'rb').read().replace(b'\r\n',b'\n').replace(b'\n',b'\r\n');open(p,'wb').write(d)" <file>`).
- Indentation in new files: **4 spaces**. In existing files, match the file.
- `#pragma once` in every header.
- Engine headers are included with angle brackets from the engine root, which is on the include path. Examples: `#include <UI/Render/UiDrawList.h>`, `#include <Logger.hpp>`, `#include <FileSystem/FileSystem.h>`.
- Names:

  | Kind | Style | Example |
  |---|---|---|
  | types | PascalCase, `Ui` prefix | `UiDrawList` |
  | functions | PascalCase | `AddQuad` |
  | public fields | PascalCase | `CornerRadii` |
  | private members | `m_` + camelCase | `m_vertices` |
  | locals and parameters | camelCase | `quadCount` |
  | constants | `k` + PascalCase | `kPrimTexels` |
  | enums | `enum class`, PascalCase values | `UiBlend::Premultiplied` |

- No exceptions thrown from new UI code. Report problems with the logger (§9) and continue with a safe fallback (skip the element, use a default value).
- Don't use `dynamic_cast` in per-frame paths. Use it only at load time.
- No new third-party libraries except those a phase names: **pugixml** (phase 1) and **nanosvg** (phase 7), both vendored into `source/Engine/includedLibraries/`.
- Comments: one or two lines saying *why*. No essays, no change logs in comments.

---

## 5. Where new files go

| Folder | Contents | Phase |
|---|---|---|
| `source/Engine/UI/Render/` | draw list, primitive buffer, GPU submit, painter, fonts (SDF), image atlas | 0, 1, 3 |
| `source/Engine/UI/Test/` | UI test runner | 0 |
| `source/Engine/UI/Markup/` | XML documents, components, variables, element registry for tags | 1 |
| `source/Engine/UI/Style/` | CSS tokenizer, parser, selectors, cascade, computed style | 1, 3 |
| `source/Engine/UI/Flow/` | flow layout (named `Flow` so it doesn't clash with the existing `UI/Layout/` JSON code) | 1 |
| `source/Engine/UI/Elements/` | `UiElem` and every `UiElem*` element | 1–5 |
| `source/Engine/UI/Menus/` | `UiMenuStack`, `UiMenus`, actions, code-behind registry | 2 |
| `source/Engine/UI/Hud/` | `UiLayers`, world panels | 5 |
| `source/Engine/UI/World/` | `UiContext`, `UiWorldSurface` | 5 |
| `source/Engine/UI/Kit/` | engine kit C++ (settings model, kit screens' code-behind) | 4 |
| `source/Engine/UI/Preview/` | generated UI database, preview descriptors/providers, preview fixtures and host adapters | 1–8 |
| `source/Engine/UI/Assets/GameData/` | **canonical** engine UI assets: `shaders/source/ui2/…`, `ui/kit/…`, `fonts/ui/…`. Synced into games (§7) | 0+ |
| `source/Engine/UI/Docs/Rework/` | these plan files | – |
| `Tools/ui/` | Python tools: asset sync, image diff, image probe | 0 |

---

## 6. Adding source files to the build

Do **all three** steps every time you add a `.cpp` or `.h` under `source/Engine/`:

1. Create the file under `source/Engine/...`.
2. Add it to `source/Engine/Engine.vcxproj`: a `<ClCompile Include="UI\Render\UiDrawList.cpp" />` line in the `ClCompile` item group (keep alphabetical order next to the other `UI\` entries), and a `<ClInclude Include="UI\Render\UiDrawList.h" />` line in the `ClInclude` item group. Paths are relative to `source/Engine`, with backslashes. There is no `.filters` file for the engine project.
3. **Re-run the CMake configure step** (§8). CMake collects engine sources with `file(GLOB_RECURSE …)` **without** `CONFIGURE_DEPENDS`, so a new `.cpp` isn't compiled until configure runs again.

`.c` and `.cpp` files under `source/Engine/includedLibraries/` are compiled automatically by the same glob (after configure), so vendored libraries need no extra CMake.

When a phase adds files to a **game** (for example `source/Games/StealthAction/UI/...`), also add them to that game's `<Game>.vcxproj` **and** `<Game>.vcxproj.filters`, then re-run configure.

---

## 7. Engine UI assets and shaders

UI shaders and engine kit markup must exist inside each game's `GameData` (each game ships its own `GameData`, and shaders are compiled per game). To avoid four drifting copies:

- The **only** place to edit engine UI assets is `source/Engine/UI/Assets/GameData/`, which mirrors the `GameData` layout.
- `Tools/ui/sync_ui_assets.py` (created in phase 0, task 0.5) mirrors the **managed folders** from there into each in-scope game:

  | Managed folder | Added in phase |
  |---|---|
  | `GameData/shaders/source/ui2/` | 0 |
  | `GameData/ui/kit/` | 4 |
  | `GameData/fonts/ui/` | 1 |

  A managed folder in a game is an exact mirror: files missing from the canonical folder are deleted there. Nothing outside managed folders is touched.
- **Never edit files inside a game's managed folders.** The next sync overwrites them.

After changing any shader, compile the shaders of **every** in-scope game:

```
python Tools/ui/sync_ui_assets.py
"source/Games/ShooterGame/GameData/shaders/ShaderCompilationAutomation.exe"
"source/Games/SimGame/GameData/shaders/ShaderCompilationAutomation.exe"
"source/Games/SlasherGame/GameData/shaders/ShaderCompilationAutomation.exe"
"source/Games/StealthAction/GameData/shaders/ShaderCompilationAutomation.exe"
```

Each `ShaderCompilationAutomation.exe` compiles everything under its own `shaders/source/` for Windows dx11/dx12/gl/spirv, Linux, Android (`300_es`) and web (`300_es`). It prints `[COMPILED]` or `[FAILED]` per shader. **Any `[FAILED]` line for a `ui2` shader is a failed check.** Read shaderc's error text above the line, fix it, and run again. The GLSL ES 3.00 (`300_es`) targets are the strictest. A shader that compiles for dx11 but fails for `300_es` is still broken.

Shader files go in `ui2/` with their own `varying.def.sc`. The tool picks the nearest `varying.def.sc`, so the `ui2` shaders never use the shared one. In code they are loaded as `ShaderManager::GetShaderProgram("ui2/vs_ui2", "ui2/fs_ui2")`.

Changed `GameData` reaches the run folder only when the game target re-links (a `POST_BUILD` step), or when you run the finalize step by hand (§8).

---

## 8. Build and run

Visual Studio 2022 is on **D:**. Use one dedicated build folder per game, created once:

```
call "D:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
cmake -S F:/Git/SdlWasmEngine -B F:/Git/SdlWasmEngine/Build/ui_stealth -GNinja -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=F:/Git/SdlWasmEngine/cmake/toolchains/msvc.cmake -DGAME_NAME=StealthAction -DGAME_PATH=F:/Git/SdlWasmEngine/source/Games/StealthAction
ninja -C F:/Git/SdlWasmEngine/Build/ui_stealth
```

Same for `ui_shooter` (`ShooterGame`), `ui_simgame` (`SimGame`) and `ui_slasher` (`SlasherGame`). Put the commands in a `.cmd` file and run it, because `vcvars64.bat` must run in the same `cmd` session. Re-running configure is the same `cmake -S … -B …` line.

- **Never build the other folders in `Build/`.** Several of them (`slasher_check`, `slasher_profile`, `wt_*`) point at old worktrees but write the same executable, so building them overwrites your work.
- The executable is `Build/<GameName>/windows/bin/<GameName>.exe`. Its working directory must be `Build/<GameName>/windows`, because `GameData` sits there, not in `bin`.
- To refresh `GameData` without re-linking: `python BuildScripts/finalize_gamedata.py source/Games/<GameName>/GameData Build/<GameName>/windows/GameData`.

**Running the game:**

- **Always pass `-headless`.** Without it a window flashes on the owner's desktop. Example from `Build/StealthAction/windows`: `bin\StealthAction.exe -headless -ui_test F:/Git/SdlWasmEngine/source/Engine/UI/Test/Scripts/StealthAction/legacy_menus.uitest`.
- The process exit code is always 0. Results come from the files the test runner writes (phase 0).
- A crash writes `CrashDump_*.dmp` next to `GameData`, and the crash handler then blocks. If a run doesn't finish within its expected time, look for a dump. Read it with `"C:\Program Files\WindowsApps\Microsoft.WinDbg_*\amd64\cdb.exe" -z <dmp> -y Build/<Game>/windows/bin -lines -c ".ecxr; kpn 8; q"`.
- The owner approved building and headless runs for this work by handing over this plan. Never run a game without `-headless`, and never commit unless the owner asks.

---

## 9. Logging and error format

- Use `Logger::Log`, `Logger::Warning` and `Logger::Error` (printf-style; header `<Logger.hpp>`).
- Every markup or style problem uses exactly this format, so tools and AI can parse it:

  ```
  [UI] <path>:<line>:<column>: error: <message>
  [UI] <path>:<line>:<column>: warning: <message>
  ```

  Paths are as passed to the loader (for example `GameData/ui/pause.xml`). Lines and columns are 1-based. Use `0:0` only when no position exists.
- Renderer problems (buffer full, missing shader) log once per frame at most. Keep a per-frame "already logged" flag, so a broken state can't flood the log.

---

## 10. Tests and definition of done

- Phase 0 builds the test runner (`-ui_test <script>`), UI-only screenshots, an image diff tool and an image probe tool. Every later phase adds `.uitest` scripts under `source/Engine/UI/Test/Scripts/`.
- A task is **done** when:
  1. the code builds for StealthAction (the pilot game) with no new warnings in the files you touched
  2. its Check step passes
  3. no earlier `.uitest` script regressed (run the phase's regression list)
- A phase is **done** when:
  1. every task is done
  2. all four in-scope games build
  3. every regression script of every game passes **on every rendering backend** (§11)
  4. the shaders compile for every target with no `[FAILED]`
- If a check fails and the cause isn't in your code, **STOP** and report. Don't "fix" unrelated engine code.

---

## 11. Every rendering backend, same pixels

The engine runs on several bgfx backends. On Windows, `-renderapi` selects them: `dx11` (the default), `dx12`, `vk` (Vulkan) and `gl` (OpenGL). **Every test runs on all four, and new UI drawing must produce the same pixels on all of them.**

- Run tests with `Tools/ui/run_ui_tests.py run …` (phase 0, task 0.3). It runs the script once per backend (`dx11,dx12,vk,gl` by default) and stores each result in `Build/ui_runs/<game>/<stem>/<label>/<backend>/`.
- "Same pixels" means `run_ui_tests.py cross` prints `EQUAL` for every backend against `dx11`. That is a per-channel difference of at most 2, with no ignore-mask other than the run-to-run noise mask of that script.
- **Legacy** drawing is only held to "unchanged against its own backend's baseline". Differences between backends that existed before phase 0 are recorded in the phase 0 notes, not fixed.
- **New** drawing (the painter, markup, everything from phase 0 task 0.12 on) must pass `cross`. A failing backend is a bug in the new code. Never raise the tolerance and never mask new content to make it pass.
- Rules that keep backends identical:
  - The anti-aliasing width comes from the CPU (texel 2 `.z` of each primitive), never from `dFdx`/`dFdy`/`fwidth`.
  - Implicit-LOD `texture2D` calls are made only at the top level of `main`. Inside branches, use `texture2DLod(…, 0.0)`.
  - Don't rely on texel-centre conventions. UVs come from the CPU and are the same on every backend.
  - Render targets: if OpenGL comes out upside down, the bug is in how a target is sampled or projected. Fix it in one place, and leave a comment saying why.
- Android and web (GLSL ES 3.00) can't be run here. Their shaders must still **compile** without `[FAILED]`. Every rule above also keeps them correct.

---

## 12. Things never to do

- Don't change the UI render target size or format (decision D10).
- Don't process, record or render UI on the game thread, and don't read live UI state while rendering (§2). Game code changing UI through the API is fine (R3).
- Don't delete or rename legacy APIs (`UiElement`, `UiRenderer::Draw*`, legacy widgets, `UiNavigation`) before a phase explicitly says so.
- Don't change the behaviour of legacy elements, except where a phase says so and gives a check.
- Don't reformat existing code, reorder includes in untouched code, or convert line endings of existing files.
- Don't add features a phase doesn't ask for, even small ones.
- Don't edit HorrorGame, or any game's managed asset folders (§7).
- Don't commit or push.


---

## 13. Game-generated UI database (`uidb.json`)

UI Studio is a **single game-independent application**. A game describes its registered UI API and previewable UI targets through a generated `uidb.json`; Studio must not contain per-game tag lists, hand-maintained schemas or hard-coded game-name branches.

### 13.1 Ownership and lifecycle

- The database is generated by the running game from the **actual runtime registries**. It is metadata, not a replacement source of truth.
- XML/CSS files, game code, tag/component/action/style registries, localisation files, settings registries and preview-provider registrations remain authoritative.
- Establish the UIDB data model, serializer and registry-publication API **early in Phase 1**, before implementing the markup/style registries that will publish into it. Do not postpone the generator until the end of Phase 1 or until Phase 6.
- The game writes an initial valid snapshot during normal startup as soon as the registries available at that stage have registered (after the game identity, GameData root, built-in legacy UI registrations and `Localisation::Initialize` are known). As Phase 1 adds the new markup registries, it refreshes the same snapshot; `UiSystem::Init` performs the first full Phase 1 publication after built-in/legacy tag registration, game UI registration and `UiComponentRegistry::Discover`, before accepting a Studio connection. A bootstrap snapshot is truthful about the capabilities currently present in that build; it must not advertise not-yet-implemented features.
- Every subsequent phase publishes metadata at the same time it implements/registers its feature. A registry change, component hot reload, preview-provider registration change or capability change marks the database dirty. Regenerate from the main thread at a safe point; do not serialize live trees while rendering.
- The generator/serializer has one owner in the engine (`UiDatabase`). Phase files extend its data through typed registries or registration hooks; they do not add phase-specific writers, sidecar databases or duplicate schemas. At each stage, absent/not-yet-implemented capabilities are reported as unavailable rather than advertised speculatively.
- Write deterministically and atomically: stable ordering, normalized project-relative `/` paths, UTF-8, no pointer values, no session IDs, no wall-clock timestamps, no absolute machine paths, no user save data, no runtime entity IDs.
- Use `SaveData/UIStudio/uidb.json` as the canonical writable location. This keeps generated data out of the content tree and works when `GameData` is read-only. Studio's only required project input is the selected `GameData` directory: it resolves the sibling game root and looks for `SaveData/UIStudio/uidb.json`. Also accept an explicitly supplied/cached `GameData/ui/uidb.json` as a portable fallback, but never require it or write it into a managed asset folder.
- If the canonical file cannot be written, log a warning and keep the in-memory database available to a connected Studio runtime. Never fail game startup because Studio metadata could not be written.
- Exclude `SaveData/UIStudio/uidb.json` from shipping builds and ordinary save backups. It is regenerated by the game. Do not package it as a content asset.
- The database is versioned. Unknown optional fields are ignored; unknown required features cause a clear compatibility warning. Studio must not silently interpret a newer incompatible format.

### 13.2 Early implementation and incremental publication

The UIDB work starts near the beginning of Phase 1, not as a late Studio task:

1. **Early Phase 1 bootstrap:** define the versioned data model, deterministic serializer, atomic file writer, path policy, stable game identity and registry-publication/dirty-notification API. Hook it into normal game startup after the currently available legacy UI registrations and localisation are known, so every game already emits a valid baseline UIDB from the first implementation milestone. It reports only capabilities/registrations that actually exist in that build.
2. **Phase 1 registry integration:** as the markup parser, CSS parser, tag registry, component registry, document discovery and preview recipes are implemented, publish their real descriptors into the common model. When `UiSystem::Init` is introduced, it refreshes the same UIDB after all new registrations/discovery have completed, producing the full Phase 1 snapshot.
3. **Phases 2–8:** each phase extends the same database in the task that implements the feature. The phase must add descriptors and a regression check that they appear after launch/re-registration. Phase 6 consumes and validates the database; it does not own or defer the engine generator.
4. **Startup and refresh:** a normal game launch produces a valid database before accepting Studio connections. During development, hot reload or a registry/capability change marks the database dirty and causes an atomic refresh at the next safe main-thread point. A failed disk write does not remove the in-memory registry or block the game.

The first implementation milestone is early Phase 1 Task 1.2a (see `P1_MARKUP_CORE.md`): normal launch begins writing a baseline UIDB from the registries available at that point. Phase 1 Task 1.14 refreshes it after `UiSystem` and the concrete markup registries exist. Task 1.18 is verification only; it must not introduce the generator for the first time.

### 13.3 Required top-level contract

The exact JSON serializer is implementation detail, but the stable semantic shape is:

```json
{
  "format": "engine-ui-database",
  "formatVersion": 1,
  "engine": { "version": "...", "buildId": "...", "previewProtocol": 1 },
  "game": { "id": "stable-game-id", "displayName": "..." },
  "roots": { "gameData": ".", "ui": "ui/" },
  "capabilities": { "markup": true, "events": true, "hudLayers": false },
  "registries": {
    "documents": [], "components": [], "tags": [], "cssProperties": [],
    "pseudoClasses": [], "actions": [], "inputActions": [], "hosts": [],
    "locales": [], "testCommands": []
  },
  "previewTargets": []
}
```

- `game.id` is a stable identifier registered by the game. If a game has not registered one yet, use a deterministic fallback derived from the executable/game target name; never derive it from an absolute path.
- Every path in the database is relative to the selected `GameData` directory unless a field explicitly says `gameRootRelative`. Preserve exact case because VFS lookup may be case-sensitive.
- Registry entries expose the same metadata used by runtime validation: stable ID/name, display label, category, source path/line where known, typed properties/attributes/parameters, defaults, accepted values/ranges, variable support, capability/invalidation flags and documentation text where available.
- The database may list localisation key names and locale availability, but it must not duplicate translation contents. Studio reads and edits the original localisation files through the existing localisation format.
- `capabilities` describe the actual build, not the theoretical maximum of the rework. A false or absent capability disables the corresponding Studio feature.

### 13.4 Preview targets are first-class database entries

A preview target is **not necessarily an XML document**. It is a registered, named recipe that tells Studio what to instantiate, what host to use, what fixture data to supply and how to present the result. It is how Studio can preview a player crosshair, health bar, damage indicator, ammo counter, world marker, loading screen, scene view or other UI that is created by game code or depends on gameplay data.

Each `previewTargets[]` entry must provide, where applicable:

- stable `id`, human-readable `name`, `category` and a short `description`;
- `kind`: `document`, `component`, `element`, `runtime`, `hosted`, `world-surface` or `scene-view`;
- `sourceRefs`: document/component paths and optional game source symbol/path; references aid navigation but do not imply that the referenced code is editable from `GameData`;
- `providerId` and `providerVersion` for runtime-created previews; the executable callback itself is never serialized;
- `host`: `viewport`, `menu-stack`, `hud-layer`, `overlay`, `world-surface`, `loading-screen` or `scene-view`, plus the registered layer/context requirements;
- default viewport/profile and presentation hints (transparent/scene background, safe-area handling, clipping, scale mode, expected aspect ratio, and whether the target is screen-space or world-space);
- typed fixture parameters with defaults, limits, units and descriptions;
- named deterministic scenarios/states, each with fixture values and an optional simulated input sequence;
- optional asset dependencies and required capabilities;
- `supportsLiveCapture` only when the target can be safely inspected from a running game. Live capture is read-only and is separate from deterministic fixture preview.

A document or component can use an engine-generated default recipe. A game-specific runtime UI must register an explicit provider. Do not guess how to preview arbitrary C++ objects from their class name.

### 13.5 Runtime preview provider contract

- Add a small `UiPreviewRegistry` and `IUiPreviewProvider` API under `source/Engine/UI/Preview/`.
- A provider is registered by a stable ID and emits a serializable descriptor plus a factory/update implementation compiled into the game. It may create a temporary `UiElem` root, configure a real engine host/context, and apply typed fixture values/scenarios.
- Keep the runtime API typed and small. The descriptor contains `Id`, `Name`, `Category`, `Description`, `Kind`, `ProviderId`, `ProviderVersion`, source refs, required host/capabilities, parameter definitions and scenarios. A parameter definition has a name, scalar/vector/colour/asset type, default, optional min/max/step/choices, unit and help text. A scenario has a stable ID/label and typed parameter values.
- `IUiPreviewProvider` exposes lifecycle operations equivalent to `CreatePreview(host, values, outRoot, error)`, `ApplyValues(host, root, values, error)`, `ApplyScenario(host, root, scenarioId, error)` and `DestroyPreview(host, root)`. `host` is an engine-owned preview host/context adapter; the returned root is an actual `UiElement`/`UiElem`, not a Qt widget. Exact helper names may follow local code style, but these responsibilities and ownership rules are mandatory.
- Register a provider through the game's explicit UI registration/bootstrap path once all dependencies are initialized. Do not rely on engine static-library static initializers. Registry changes call `UiDatabase::MarkDirty()`.
- Providers may create a UI tree from game-owned C++ elements or load/bind an existing XML composition. In both cases they must use the same production element, component, style, layout, paint and host paths as the game. Studio must not draw a mock crosshair/health bar in Qt or approximate a HUD layout.
- Preview fixtures must be deterministic, isolated and side-effect-free. They must not spawn real gameplay entities, write save data, modify player health/inventory/settings, emit gameplay actions, or require a live network session.
- A provider may optionally expose a read-only live-capture mode for a currently running game. It must never mutate the live game's state. The UI explicitly labels `Fixture` versus `Live` mode.
- Fixture values are applied through a typed preview model/variables or a provider's `ApplyScenario` method, not by inventing undocumented markup attributes. Parameter edits update the preview immediately and can be reset to descriptor defaults.
- A provider declares every required host and capability. If unavailable, Studio shows the target's metadata and source links with an actionable `preview provider unavailable` message; it must not silently substitute a fake rendering.
- Provider creation, updates and destruction run on the engine main thread under conventions §2. No provider starts threads or retains transient pointers beyond the preview session. Destroying a preview must release its temporary root, host/context and resources.
- The `-ui_studio` IPC endpoint is polled/dispatched non-blockingly from the existing main-thread update point. It must not create an engine-owned socket/pipe thread. Any OS transport callback that arrives off-thread may enqueue only opaque bounded bytes; parsing and all engine/UI access occur on the main thread.

### 13.6 Example preview descriptors

A game-owned crosshair may declare:

```json
{
  "id": "hud.player.crosshair",
  "name": "Player Crosshair",
  "category": "HUD/Combat",
  "kind": "runtime",
  "providerId": "game.ui.preview.player-crosshair",
  "providerVersion": 1,
  "host": { "kind": "hud-layer", "layer": "hud" },
  "sourceRefs": [{ "kind": "document", "path": "ui/hud/crosshair.xml" }],
  "parameters": [
    { "name": "spread", "type": "float", "default": 0.25, "min": 0.0, "max": 1.0 },
    { "name": "targetVisible", "type": "bool", "default": false },
    { "name": "hitMarker", "type": "bool", "default": false }
  ],
  "scenarios": [
    { "id": "hip-fire", "values": { "spread": 0.25 } },
    { "id": "aiming", "values": { "spread": 0.05 } },
    { "id": "target-hit", "values": { "spread": 0.4, "targetVisible": true, "hitMarker": true } }
  ]
}
```

A health bar can use the same mechanism with `currentHealth`, `maxHealth`, `shield`, `recentDamage` and scenarios such as `full`, `half`, `critical`, `taking-damage` and `healing`. These are examples of the contract, not mandatory global gameplay variables; each game defines the fixture fields that its UI actually consumes.

### 13.7 Security and robustness

- Treat `uidb.json` and every path/label inside it as untrusted input. Canonicalize paths and reject traversal outside the selected game roots.
- Do not execute commands, load arbitrary DLLs, evaluate scripts, or deserialize code pointers from UIDB. Runtime providers must already be registered by the selected game's executable.
- A UIDB mismatch (different game ID/build, missing provider, schema version mismatch) must produce a clear diagnostic and safe fallback.
- Studio may cache a parsed UIDB, but it must compare the database version/build identity and refresh it when the game launches or the runtime reports a registry revision change.
