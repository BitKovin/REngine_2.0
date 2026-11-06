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
