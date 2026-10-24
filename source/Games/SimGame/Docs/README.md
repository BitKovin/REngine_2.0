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
| [SLICE_M2C_BODIES.md](SLICE_M2C_BODIES.md) | Slice contract of M2c: agent bodies, spawn handshake, sim-dictated goals (implemented) |
| [SLICE_M2D_NPCS.md](SLICE_M2D_NPCS.md) | Slice contract of M2d: the ten authored NPCs of `GameData/sim/npcs.json`, how to author more (implemented) |
| [SLICE_M2E_TRANSITION.md](SLICE_M2E_TRANSITION.md) | Slice contract of M2e: walking through a `sim_portal` into the next level (implemented) |
| [SLICE_M2F_CHARACTERS.md](SLICE_M2F_CHARACTERS.md) | Slice contract of M2f: one character body for NPCs and the player, first/third person, hidden warm-up ring (implemented) |
| [HANDOFF_M4_BAKER.md](HANDOFF_M4_BAKER.md) | Hand-over for starting the baker slice in a new session: reading order, state, first steps, how to run the map editor and the build, the start message |
| [SLICE_M4_BAKER.md](SLICE_M4_BAKER.md) | Slice contract of M4 "the baker": a playable village with calendar, needs, items, a bread chain, first player verbs, consequences (proposed) |
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
  Sim/Bodies/           materialization, sim side (proposals, goals, reports)
  Sim/Npcs/             authored NPCs (routines); content in GameData/sim/npcs.json
  Characters/           how characters look: state, models table, animator, body, ring visibility (NPCs and the pawn share them)
  SimPawn.h/.cpp        the walking pawn (first / third person)
  SimNpc.h/.cpp         the driver of an agent's character: physics, navigation, fade
  Sim/Tests/            headless self tests
  MapGen/               Python level kit: props, buildings, the three villages and the vale (maps/vale.map.json) generated as level editor prefabs and maps (SIM_CONTRACT section 14)
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
| `-sim_nav_report` | Write `SaveData/sim_nav_report.txt` (spawn points, portals, walkable spots per level, level-local) for authoring NPC routes, quit |
| `-sim_demo [count]` | Place commuting demo walkers (default 40) at the world's spawn points; drawn as boxes (console: `sim_demo`, `sim_agents`, `sim_stats`) |
| `-sim_look <npc id>` | Stand 6 m from an authored NPC and keep the view on it (console: `sim_look`); with `-screenshot` this is how to look at the game headless |
| `-sim_transition_test` | Automated in-game check of the level transition through the test chain; writes `SaveData/sim_transition.log`, quits |
| `-sim_start <spawn>` | Start at the `sim_spawn` with that targetname instead of the default player start (the vale has `view_c_square`, `view_c_terrace`, `view_c_bakery`, `view_c_mill`, `view_c_tavern`, `view_a_street`, `view_b_green` for pictures) |
| `-sim_view first\|third`, `-sim_pitch <deg>` | Start in that view / looking that far down (negative: up): for pictures (console `sim_view`, key V) |
| `-screenshot <seconds> [-screenshot_file <name>]` | Save the frame at that much game time to `SaveData/screenshots/<name>.png` (engine hook, `Engine/Screenshot.*`; console: `screenshot [name]`); one picture per run, combine with `-quit_after` |
| `-sim_bridge_test` | Automated in-game check of the bridge, pawn and save hook (starts in `sim_test_a`); writes `SaveData/sim_bridge.log`, quits |
| `-renderapi dx11\|dx12\|vk\|gl\|noop` | Renderer. `noop` does no GPU work but cannot find compiled shaders, so it only suits runs that never render |

Automated runs: always from the game folder, always `-headless`:

```
cd source\Games\SimGame
..\..\..\Build\SimGame\windows\bin\SimGame.exe -headless -sim_selftest
..\..\..\Build\SimGame\windows\bin\SimGame.exe -headless -sim_bake_nav
..\..\..\Build\SimGame\windows\bin\SimGame.exe -headless -map sim_test_a.bsp -sim_nav_draw -quit_after 5
..\..\..\Build\SimGame\windows\bin\SimGame.exe -headless -sim_bridge_test
..\..\..\Build\SimGame\windows\bin\SimGame.exe -headless -sim_look warden -screenshot 6 -screenshot_file warden -quit_after 9
```

The sim starts with the game. At game load, development builds check that `GameData/sim/world.nav` is newer than every map it was baked from and log a loud
error if not (rebake with `-sim_bake_nav`).

## Conventions

- Follow the process in addendum section 17: a short contract per slice, tests first, text output from every tool.
- Reuse engine facilities (ECS, entities, `EntityHandle`, `MathHelper`, `FileSystemEngine`) before writing helpers.
- The simulation lives in `Sim/` inside this project and uses engine code sparingly (owner decision, SIM_CONTRACT.md §1).
- Engine changes made for SimGame (all additive): the light style provider `CQuake3BSP::SetLightStyleProvider`; `dtNavMesh::connectRemoteOffMeshLinks` (Detour); `GameSaveSystem::RegisterExtension` and `Extensions` in the save data; `Helpers/Base64.h`.
