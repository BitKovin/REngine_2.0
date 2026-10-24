# Hand-over: start of the baker slice (M4) in a new session

Written 2026-10-24 at the end of the session that did M2e (level transition), M2f (characters) and drafted the baker contract. Everything below is in the working tree of
`F:\Git\SdlWasmEngine` on branch `main`; **nothing of it is committed** (the owner said "don't commit yet"). Ask before committing.

## 1. Read, in this order

1. `Docs/SLICE_M4_BAKER.md` - the contract of the work (owner's decisions are in section 13; props are the level editor's prefabs).
2. `Docs/SIM_CONTRACT.md` - the code truth: module map, interfaces, invariants, tests. Sections 9 to 13 are the newest (bridge, bodies, NPCs, transition, characters).
3. `Docs/README.md` (layout, build, launch arguments), then `Docs/SLICE_M2E_TRANSITION.md` and `Docs/SLICE_M2F_CHARACTERS.md` only if you touch those areas.
4. Memory files (auto-loaded): `simgame-progress-and-gotchas.md` first, then `slasher-quality-bar.md`, `use-engine-framework.md`, `search-engine-before-helpers.md`, `simgame-sim-in-project.md`,
   `simgame-screenshot-workflow.md`, `headless-launch-mode.md`.
5. The map editor: `F:\Git\mapeditor\README.md`, `DESIGN.md` sections 5.4 (prefabs), 6.8 (light styles), 6.9 (light entities); its skill `F:\Git\mapeditor\.claude\skills\level-blockout\SKILL.md`
   (its scale is the Slasher far-camera scale: **do not use its sizes**, SimGame is real human scale) and `MCP_DESIGN.md` section 2 (lessons L1 to L10: path first, variants first, you report measured
   facts and the owner judges taste).

## 2. State of the world (what exists and works)

- Sim kernel, world nav bake, movement, bridge (sim thread, saves), bodies, ten authored NPCs, level transition, character bodies (`Characters/`), first / third person (V), the warm-up ring
  (visible 45 m, spawn 65, despawn 80). Tests: `SimGame.exe -headless -sim_selftest` (11,274 checks pass), `-sim_bridge_test`, `-sim_transition_test` (both pass).
- Test levels `sim_test_a`, `sim_test_road`, `sim_test_b`, `sim_test_c` in `GameData/maps` (`.map.json` made with the editor's Python kit, exported `.bsp`, baked into `GameData/sim/world.nav`).
- Not built yet and part of this slice: calendar, items, needs, activities, work, shops, player verbs, HUD, the village, light-style provider (E1), sky sphere (E2).

## 3. The first things to do (the owner wants this order)

1. **Step 1, the village map.** Use the map editor (it is installed and built; see 4). Make **two or three different layout variants** first (cheap massing: street, houses, bakery, mill, green, field edge,
   about 100 x 100 m, real human scale), take small pictures, show them to the owner, ask which to build. Build only the chosen one, then sites, doors, slots, containers and **prefab props**.
   Design the prefabs first (they are reused): bread shelf with `sim_item_prop` slots, counter, oven, flour bins, grain bin, till, bed, table, stool, pantry, crate, barrel, lamp post, fence, well.
2. **Step 2b / E1, the light-style provider.** Small additive engine change in `Engine/BSP/Quake3Bsp.{h,cpp}` (`CQuake3BSP::GetStyleColor`): a static game-installed function
   `bool(uint8_t style, glm::vec3& colour)` asked first. Record it in SIM_CONTRACT section 1 (engine change list). E2 (sky sphere + shader) comes after, once the engine's current sky path has been read.
3. Then the steps of contract section 12 in order. Contract the details of each step with the owner where the contract leaves a question; ask design questions, do not guess them.

## 4. Running the map editor

- Assets root for SimGame is its `GameData` (`maps/`, `textures/`, `entities.ent`; `GameData/editor_settings.json` has `lastMap`). There is no `prefabs/` folder and no `game_profile.json` yet: create
  `GameData/prefabs/` for the prefabs; a `game_profile.json` is only needed to override `prefabRoot`, texture roots or the light-style patterns (`lighting.lightStyles`).
- Core: `F:\Git\mapeditor\core\build\mapeditor_core.exe 8787 "F:\Git\SdlWasmEngine\source\Games\SimGame\GameData"` (run in the background; port 8787 must be free; it is built).
  Shell: `preview_start` with the `shell` configuration of `F:\Git\mapeditor\.claude\launch.json` (Vite on 5173; `shell/node_modules` exists); then `http://localhost:5173/?debug` gives `window.__editor`
  (see the skill for camera and screenshot recipes; a hidden Browser pane does not draw: show it first). Stop both when done.
- Maps are `.map.json` (schema 0.2, metres, Y up, north = -Z); the skill's `mapgen.py` / `mansion.py` / `mansion4.py` / `reach.py` generate and check them (standard-library Python, set `LEVEL_OUT`).
  Never write test maps into the editor repo's `core/assets`. Export to `.bsp` with the editor's `bsp_export_tool.exe` (Core `bsp_export.h`; check how the existing `sim_test_*.bsp` were exported before
  assuming a command line), then `SimGame.exe -headless -sim_bake_nav` from the game folder (it rebakes `world.nav` and validates portals and spawns), and add the new map to `GameData/sim/world.manifest`
  (first line = layout root = startup map).
- Entity classes for the sim live in `GameData/entities.ent` (`sim_spawn`, `sim_portal`, `sim_site`, `sim_door`, `sim_slot`, `sim_container`, `sim_npc`); `sim_item_prop` and any missing keys are to be added there.

## 5. Build and test (the owner allows the implementer to build and run headless)

```
call "D:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
cd F:\Git\SdlWasmEngine\Build\simgame_main && cmake . && ninja         (re-run `cmake .` after adding .cpp files; also add them to SimGame.vcxproj + .filters)
cd F:\Git\SdlWasmEngine\source\Games\SimGame        (always run from the game folder, always with -headless)
..\..\..\Build\SimGame\windows\bin\SimGame.exe -headless -sim_selftest        (report SaveData/sim_selftest.log)
```

Pictures: `-sim_view first|third -sim_pitch <deg> -sim_look <npcId> -screenshot <seconds> -screenshot_file <name>` -> `SaveData/screenshots/<name>.png`, then read the PNG.

## 6. Working rules that cost time to learn

- Files are tab-indented; use the Edit tool or script files written with the Write tool (bash heredocs holding long Python with many quotes failed here). Match line endings (some files are CRLF).
- The engine lights a mesh from the light volume at its ENTITY's position (put entities mid-body, not on the floor); call `PreloadAssets()` on meshes; the engine's pitch is positive when looking down.
- Engine changes only when unavoidable, additive, approved, and listed in SIM_CONTRACT section 1. Use the engine's `Entity` / ECS / `EntityHandle` directly (no wrapper layers); search the engine for
  helpers before writing one; sim code uses entt containers; the sim never touches engine statics from its thread.
- Never run the game without `-headless`. Verify visually (pictures) before saying something looks right, and report measured facts; the owner judges taste and feel.
- Unverified so far (do not claim): the web build, cloth motion in play, third-person camera feel, the ring in a large level, loading-screen feel on transitions.

## 7. The start message for the new session

(also given to the owner in the chat; copy it as the first message)

> Continue the SimGame work: start the baker slice. Read `source/Games/SimGame/Docs/HANDOFF_M4_BAKER.md` first and follow its reading order. The contract `Docs/SLICE_M4_BAKER.md` is approved with
> these decisions: light survival for the player (Kingdom Come: Deliverance model), baked light styles + style provider + blended sky for time of day, shop ban only for theft, a village of about 100 m,
> and props are the level editor's prefabs (item props are `sim_item_prop` brush entities shown and hidden from container contents). Nothing from the previous session is committed; do not commit unless I ask.
> Begin with step 1: design the village layout through the map editor at `F:\Git\mapeditor` (connect to it as the hand-over describes) and show me 2 or 3 different layout variants as small
> pictures before building one; then the prefab set. After that, step 2b E1 (the light-style provider, a small additive engine change). Ask me the design questions the contract leaves open instead of
> guessing, and keep the contract and SIM_CONTRACT.md up to date as you go.
