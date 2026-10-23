# SimGame

First-person Victorian-gothic open-world action RPG built on a continuously simulated living world.
This folder holds the project's documentation and other non-code files. Source lives one level up.

## Documents

| File | What it is |
|---|---|
| [living_world_simulation_design.md](living_world_simulation_design.md) | The original design document (draft 3): pillars, rules R1 to R14, systems, milestones |
| [SIM_CONTRACT.md](SIM_CONTRACT.md) | **Start here when working on code.** Module map, interfaces, invariants, tests, next steps |
| [SLICE_M2_MOVEMENT.md](SLICE_M2_MOVEMENT.md) | Slice contract of M2a abstract movement (implemented) |
| [SLICE_M2B_BRIDGE.md](SLICE_M2B_BRIDGE.md) | Slice contract of M2b: SimBridge, walking pawn, snapshot, save hook (implemented) |
| [living_world_simulation_addendum.md](living_world_simulation_addendum.md) | How the design is implemented on this engine. **Wins wherever it disagrees with the design document.** Decisions D1 to D12, build order M0 to M9, process for an AI implementer (section 17), open items |

Keep new documents here: contracts per slice (see addendum section 17.4), `SIM_CONTRACT.md` once the sim library exists,
tuning notes, soak reports worth keeping, screenshots and references.

## Layout

```
SimGame/
  Docs/                 documentation and project files (this folder)
  GameData/             copied from ShooterGame as a base: shaders, fonts, localisation, UI, a few maps and models
  Settings/             copied from ShooterGame: video / audio / input settings data and the rebinding registry
  UI/Pause/             copied from ShooterGame: pause menu, settings screens, save/load menu
  PauseGameManager.hpp  copied from ShooterGame
  GameStart.cpp         bootstrap entity: input actions, settings, pause handling, startup map, -sim_selftest
  Sim/Core/             simulation kernel (see SIM_CONTRACT.md)
  Sim/Nav/              world navigation: tile surgery, bake, runtime paths
  Sim/Movement/         abstract movement: speed table, path walking, tiered movement module
  Sim/Bridge/           engine side: map entities, bake command, world file, nav debug draw, SimClient / SimBridge (the sim thread and the save hook)
  SimPawn.h/.cpp        the walking pawn
  Sim/Tests/            headless self tests
  GameData/entities.ent level editor entity definitions (engine classes + sim_* classes)
  SimGame.vcxproj       Visual Studio project (CMake globs sources, so the project file is only for the IDE)
```

Changes made to the copied code: `UiPauseMenu.h` no longer asks ShooterGame's `Player` whether saving is allowed (the
save button is always shown for now), and `UiSaveLoadMenu.h` no longer includes `Player.hpp`.

GameData is a full copy of ShooterGame's tracked GameData (about 300 MB). Prune what SimGame will not use as the game
takes shape; shaders, fonts, localisation and the UI assets are the parts the engine and the pause/settings screens need.

## Build

CMake globs the game's `.cpp` files, so re-run the configure step after adding one.

```
call "D:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
cmake F:\Git\SdlWasmEngine -B F:\Git\SdlWasmEngine\Build\simgame_main -GNinja -DCMAKE_BUILD_TYPE=Release ^
  -DCMAKE_TOOLCHAIN_FILE=F:/Git/SdlWasmEngine/cmake/toolchains/msvc.cmake -DGAME_NAME=SimGame ^
  -DGAME_PATH=F:/Git/SdlWasmEngine/source/Games/SimGame
cd F:\Git\SdlWasmEngine\Build\simgame_main
ninja
```

The executable is `Build/SimGame/windows/bin/SimGame.exe`; the build also stages `fmod.dll` and `fmodstudio.dll` next to
it (`fmod_stage_windows_dlls` in `sourceLibraries/fmod/CMakeLists.txt`). Run it with the working directory set to
the game folder `source/Games/SimGame`, so the source `GameData` is used directly (no copying). The staged
`Build/SimGame/windows/GameData` is for packaged runs only.

## Launch arguments

| Argument | Effect |
|---|---|
| `-map <file>` | Startup map inside `GameData/maps` (default: the first map of `GameData/sim/world.manifest`, else `blank.bsp`) |
| `-quit_after <seconds>` | Quit after that many seconds of game time (not in Distribution builds) |
| `-headless` | No visible window, muted sound, no real input devices, no crash dialog. For automated runs |
| `-sim_selftest` | Run the sim self tests headless, write `SaveData/sim_selftest.log`, quit |
| `-sim_bake_nav [manifest]` | Bake `GameData/sim/world.nav` from the manifest (default `GameData/sim/world.manifest`), write `SaveData/sim_bake_nav.log`, quit |
| `-sim_nav_draw` | Draw the loaded level's part of the world nav mesh and the portal links (console: `sim_nav_draw [on/off]`) |
| `-sim_demo [count]` | Place commuting demo walkers (default 40) at the world's spawn points; drawn as boxes (console: `sim_demo`, `sim_agents`, `sim_stats`) |
| `-sim_bridge_test` | Automated in-game check of the bridge, pawn and save hook; writes `SaveData/sim_bridge.log`, quits |
| `-renderapi dx11\|dx12\|vk\|gl\|noop` | Renderer. `noop` does no GPU work but cannot find compiled shaders, so it only suits runs that never render |

Automated runs: always from the game folder, always `-headless`:

```
cd source\Games\SimGame
..\..\..\Build\SimGame\windows\bin\SimGame.exe -headless -sim_selftest
..\..\..\Build\SimGame\windows\bin\SimGame.exe -headless -sim_bake_nav
..\..\..\Build\SimGame\windows\bin\SimGame.exe -headless -map sim_test_a.bsp -sim_nav_draw -quit_after 5
..\..\..\Build\SimGame\windows\bin\SimGame.exe -headless -sim_bridge_test
```

The sim starts with the game. At game load, development builds check that `GameData/sim/world.nav` is newer than every map it was baked from and log a loud
error if not (rebake with `-sim_bake_nav`).

## Conventions

- Follow the process in addendum section 17: a short contract per slice, tests first, text output from every tool.
- Reuse engine facilities (ECS, entities, `EntityHandle`, `MathHelper`, `FileSystemEngine`) before writing helpers.
- The simulation lives in `Sim/` inside this project and uses engine code sparingly (owner decision, SIM_CONTRACT.md §1).
- Engine changes made for SimGame (all additive): `dtNavMesh::connectRemoteOffMeshLinks` (Detour); `GameSaveSystem::RegisterExtension` and `Extensions` in the save data; `Helpers/Base64.h`.
