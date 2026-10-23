# SIM_CONTRACT

The working contract of the SimGame simulation: module map, public interfaces, invariants, tunables, what is tested
where, and what comes next. **Every session starts by reading this file and updates it before ending** (addendum
section 17.7). Design and addendum stay the source of truth for *decisions*; this file is the source of truth for *code*.

Status: 2026-10-23. Kernel (M1 core, parts of M3) and WorldNav (M0: bake command, stale check, debug draw) are done and tested.
The test world (`GameData/sim/world.manifest`: sim_test_a, sim_test_road, sim_test_b, sim_test_c) bakes with 6/6 portals linked.
M2a abstract movement (`Sim/Movement`, `SLICE_M2_MOVEMENT.md`) and M2b the bridge, walking pawn, snapshot and save hook (`SLICE_M2B_BRIDGE.md`) M2c agent bodies (`SLICE_M2C_BODIES.md`) and M2d ten authored NPCs from `GameData/sim/npcs.json` (`SLICE_M2D_NPCS.md`) are done and tested. Next: the level transition.

---

## 1. Ground rules (decided by the owner, override the addendum where they differ)

| Rule | Detail |
|---|---|
| The sim lives in the game project | `source/Games/SimGame/Sim/`, compiled into SimGame.exe. There is **no** engine-free library and no separate test executables (overrides addendum 17.3 / 17.5). Use of engine code stays limited: entt, glm, Detour, `Logger`, `ByteCompressor`, `FileSystemEngine` (bridge only). |
| Prefer entt containers | `entt::dense_map` / `dense_set` over `std::map` / `unordered_map` in sim code. They iterate in insertion order, so iteration is deterministic. |
| The sim never touches engine statics | No `NavigationSystem`, `EcsWorld`, `Level`, `Time`, `Input` from sim code. The sim thread owns its own entt registry and its own `dtNavMesh`. |
| World nav is ONE merged Detour mesh (D3) | Built from the levels' navmesh tiles (the tiles the engine's tile-cache pipeline produced, so the same polygon grid). The tile cache itself is **not** part of the world mesh. |
| Engine changes so far | (1) `dtNavMesh::connectRemoteOffMeshLinks` + `DT_OFFMESH_SIDE_REMOTE` in `source/Engine/Navigation/Detour/DetourNavMesh.{h,cpp}`: purely additive. (2) `GameSaveSystem::RegisterExtension` / `GameSaveExtension` and `Extensions` in `GameSaveData` (`source/Engine/SaveSystem/GameSaveSystem.{h,cpp}`): additive, a game registers state that is saved and loaded with the level; old saves load. (3) `Helpers/Base64.h`: the Base64 helpers that were file-static in `EmscriptenFileSystem.cpp`, moved to a shared header (the web file system includes it; **not built on this machine**, the move is literal apart from unsigned accumulators and a byte-safe table index). (4) **Screenshot hook** (2026-10-23): `source/Engine/Screenshot.{h,cpp}` saves a presented frame as a PNG (console `screenshot [name]`, launch `-screenshot <game seconds> [-screenshot_file <name>]`, files in `SaveData/screenshots/`). Wired in two places: `windows_main.hpp` passes `Screenshot::Callback()` as `bgfx::Init::callback` (this callback also takes over bgfx's fatal and trace output), and `EngineMain::Render` calls `Screenshot::BeforePresent` just before `bgfx::frame()`; `Engine.vcxproj` lists the two files. Purely additive; works in `-headless` runs. Other platform mains do not install the callback, so on them the requests do nothing (**not built on web / GDK on this machine**). All owner-approved. List any further engine change here. |
| Builds and tests | The implementer may build and run headless tests (see section 7). Never run the game without `-headless`. To look at the game: `SimGame.exe -headless -sim_look <npc id> -screenshot 6 -screenshot_file name -quit_after 9` from the game folder, then read `SaveData/screenshots/name.png` (one picture per run; the sim is deterministic, so runs repeat: the courier leaves level a at 3 s and is back at 38 s). The user builds and checks the game in their own runs. |

---

## 2. File map

```
Sim/
  Core/                     kernel: knows no game concepts (rule R12)
    SimTypes.h              Tick (0.1 s), Entity (entt), EventId, TypeId, RuleId, pseudo rule ids
    SimArchive.h            one Visit() per record drives binary save/load, FNV hash, text dump, entity collection;
                            FixedString<N> for trivially copyable payloads
    SimRandom.h             PCG32 Rng, SplitMix64, HashCombine, HashString
    LazyValue.h             (value, time, rate, min, max) with CrossingTick
    SimScheduler.h/.cpp     binary heap of (tick, sequence) with lazy cancellation via TimerHandle slot versions
    SimEventLog.h           EventRecord, fixed-capacity ring (default 262,144 events)
    SimTypeRegistry.h/.cpp  Module, Registration, RegistryBuilder, frozen TypeRegistry, startup dump (Describe)
    SimWorld.h/.cpp         World: step pipeline, waves, commands, rule/apply contexts, snapshots, state hash,
                            text tools (dump, why-trace, timeline, scheduler, state); kernel types Label,
                            SetupStarted, CommandRejected, SetTunableRequested
    SimServer.h/.cpp        Server: input queue, input log / Recording, ReplayRecording, text commands
    SimHost.h/.cpp          Host: Pump mode (same thread) and Thread mode (sim thread, mutex swap buffers),
                            Message protocol (events, pause/step/speed, save/load barrier, text requests, wake)
  Nav/                      world navigation (addendum section 4)
    NavTileData.h/.cpp      parse / edit / write Detour tile blobs (translate, add off-mesh link, mask border)
    WorldNavData.h/.cpp     LevelNavData, ReadLevelNavFile (engine .nav, tests only), bake inputs, world file
    WorldNavBake.h/.cpp     BakeWorldNav (pure), CaptureLevelNav (copy tiles from a live engine dtNavMesh)
    WorldNav.h/.cpp         runtime: combined mesh, Project, FindPath, PathService (sliced A*)
    WorldNavDebug.h/.cpp    pure debug geometry: CollectDebugSegments (level-local or whole-world line segments)
    WorldNavReport.h/.cpp   text report for content authors: extent, spawns, portals, walkable spots per level (launch -sim_nav_report)
  Movement/                 abstract movement (section 8): game concepts live here, not in Core
    SimSpeedTable.h         shared speed table: mover class x gait x area -> m/s (header only; the physical layer will read it too)
    SimWalk.h/.cpp          pure path walking: Advance, PositionOf, LevelOf, WalkLength (no World)
    SimMovement.h/.cpp      MovementModule(WorldNav&): components, events, commands, tunables, systems, invariant, snapshot, placements, rebake migration
    SimDemoPopulation.h/.cpp  development only: demo module (PopulateDemo, TripWanted, commute rules), the same rules the tests run
  Bodies/                   materialization, sim side (section 10)
    SimBodies.h/.cpp        BodiesModule: proposals, goals, input events, Appearance / BodyRetry components
  Npcs/                     authored NPCs (section 11)
    SimNpcs.h/.cpp          NpcModule: Routine component, RoutineStep timer rule, NpcDefinition (plain data), Validate, Place
  Bridge/                   engine side (the only code that uses both the sim and engine gameplay types)
    SimNpcFile.h/.cpp       GameData/sim/npcs.json -> NpcDefinition (format documented in the header), errors named by NPC and stop
    SimMapEntities.h/.cpp   sim_portal (TriggerBase; keeps targetLevel, targetSpawn, brush centre) and sim_spawn entities
    SimNavBaker.h/.cpp      bake shell: console `sim_bake_nav [manifest]`, launch `-sim_bake_nav [manifest]` (bakes, quits)
    SimWorldNavFile.h/.cpp  world.nav path, Load, map path <-> level, CheckFresh / CheckAtGameLoad (stale check)
    SimNavDebugDraw.h/.cpp  DebugDraw of the loaded level's world mesh + portal links: console `sim_nav_draw [on|off]`, launch `-sim_nav_draw`
    SimSaveFile.h/.cpp      the sim part of a game save: header (tick, schema hash, layout fingerprint, level names, placements) + kernel blob; pure
    SimClient.h/.cpp        game-thread side of the Host, no engine dependency: player reports, newest snapshot, blocking save/load/text, control
    SimAgents.h/.cpp        game side of bodies: answers proposals (hidden from the camera), owns the SimNpc entities, goals in, reports out
    SimBridge.h/.cpp        engine glue: static singleton started by GameStart (world file, modules, SimClient in Thread mode, pause, GameSaveSystem
                            extension "sim", agent boxes, sim_* console commands, the -sim_bridge_test run)
  Tests/
    SimTestFramework.h      TestContext (Check / Note / Section), Stopwatch, suite declarations
    SimSelfTests.h/.cpp     RunSimSelfTests + console command sim_selftest
    SimNavFixtures.h/.cpp   real baked levels shared by the nav and movement tests (LoadFixture, OriginalMesh, BuildChain)
    SimMovementTests.cpp    movement suite
    SimBridgeTests.cpp      bridge suite: SimClient in both Host modes, snapshot, save barrier, rebake
    SimBodiesTests.cpp      bodies suite against a scripted stand-in for the game
    SimNpcsTests.cpp        authored NPC suite: routines, validation, parser, saves, rebakes, replay, the shipped content
    SimKernelTests.cpp      kernel suite, with a toy "test" module that exercises every kernel path
    SimNavTests.cpp         world nav suite on real GameData/maps/*.bsp.nav fixtures
GameData/entities.ent       level editor entity definitions (engine classes + sim_* classes, section 6)
GameData/sim/npcs.json      the authored NPCs (section 11)
SimNpc.h/.cpp               the physical body of an agent: CharacterController + SkeletalMesh + own navigation (section 10)
SimPawn.h/.cpp              the walking pawn (section 9): Entity on the engine CharacterController, created by SimSpawn at the default player start
```

---

## 3. Kernel contract

### 3.1 Time and step pipeline
- `Tick` = 0.1 s of world time. `World::Step(ticks, inputs)` advances `ticks` (1 normally, more in fast-forward).
- Order inside a step: (1) cascade overflow from the last step, then inputs, at the first tick; waves.
  (2) Scheduled events due within the step, in (tick, sequence) order, each processed at its own tick; waves per tick.
  (3) Systems at the last tick, registration order; waves. (4) Invariants if `World::checkInvariants`.
- A **wave** runs every rule of every pending event against the unchanged world, collecting commands, then applies the
  commands in order. Events emitted by commands form the next wave. More than `cascadeDepthCap` (8) waves: the rest is
  deferred to the next step (counted in `Stats().cascadeOverflows`, logged as a warning). Nothing is ever dropped.
- `IsStaggerDue(entity, now, dt, period)`: true once per period per entity (staggered needs and mood updates).

### 3.2 Records
- Sim records are entities of the World's own `entt::registry`. Handles are generational: a destroyed entity's handle
  never becomes valid again. `World::TryGet<T>(e)` returns nullptr for stale or null handles, never asserts.
- Components: any struct with `static constexpr const char* TypeName` and a `template<class Ar> void Visit(Ar&)`.
  Fields may be arithmetic, enums, bool, Entity, std::string, vectors, std::array, glm vectors, FixedString, LazyValue,
  TimerHandle and nested Visit structs. Empty structs are tags.
- Events and commands: same, but trivially copyable, at most 48 bytes (`MaxPayloadSize`), alignment at most 8.
  Use `FixedString<N>` for short text.

### 3.3 Writing a module
```cpp
struct HungerModule : sim::Module
{
    const char* Name() const override { return "needs"; }
    void Register(sim::Registration& reg) override
    {
        reg.Component<Needs>();                              // saved, hashed, dumped
        reg.Event<HungerCrossed>(sim::EventSignificant);     // flags: EventInput, EventOutput, EventWake, EventSignificant
        reg.Command<SetHunger>([](sim::ApplyContext& ctx, const SetHunger& c) { /* mutate, ctx.Emit, ctx.Schedule */ });
        reg.Rule<HungerCrossed>("eat_when_hungry", [](sim::RuleContext& ctx, const HungerCrossed& e) { ctx.Command(SetHunger{...}); });
        reg.System("integrate", [](sim::ApplyContext& ctx, sim::Tick dt) { /* continuous processes only */ });
        reg.Invariant("hunger_in_range", [](const sim::World& w, std::string& error) { return true; });
        rate = reg.Tunable("hunger_rate", 0.01f, 0.0f, 1.0f, "per second");
        stream = reg.RandomStream("decisions");
    }
};
```
- Modules are added in an explicit order to `RegistryBuilder` (kernel types are always first). `Build` returns nullptr
  and lists errors for duplicates, rules on unregistered events, tunable defaults out of range.
- **Rules** get `RuleContext` (const World, the event, `Random(salt)` deterministic per event and rule) and may only
  `Emit`, `Schedule` (fire-and-forget) and `Command` (rule R7).
- **Command apply functions, systems and setup** get `ApplyContext`: mutable World, `Emit`, `Schedule` (returns a
  cancellable TimerHandle), `ScheduleUntracked`, `Cancel`, `Stream(id)`, `Create`, `Destroy`, `Reject(reason)`.
  Everything they emit is stamped with the cause (the event the rule reacted to) and the rule id automatically.
- Systems are only for continuous integration (movement, encounter sweep). Discrete changes go through events.
- `World::RunSetup(reason, fn)` applies world population, scenario placement and logged exceptions (rule R4) under a
  logged `SetupStarted` root event.

### 3.4 Scheduler and lazy values
- `ApplyContext::Schedule(at, event)` returns a `TimerHandle` (slot, version). `Cancel` invalidates it; popped stale
  entries are dropped. Store handles in components; they survive save and load.
- `LazyValue::CrossingTick(now, threshold)` gives the tick to schedule a band crossing. On rate change: `SetRate`,
  cancel the old handle, schedule the new crossing (see `TestModule::ScheduleHunger` in SimKernelTests.cpp).

### 3.5 Determinism (rule R9) — enforced, not hoped for
- No wall clock, no std random distributions, no engine random in sim code. Systems and apply functions use named
  streams (`Stream(id)`), rules use `RuleContext::Random`.
- Never iterate unordered containers in the sim. entt views iterate in packed order, which saves and loads preserve.
- Floats are fine (determinism is per build and platform, design open decision 11).
- `World::StateHash()` = FNV of the state snapshot without the log. Tests compare run-twice hashes every tick.

### 3.6 Saves, replay, transport
- `World::Save(includeLog)` / `World::Load(types, bytes, error)`: binary, versioned, name-mapped (event types, rules,
  components, tunables, streams), components as length-prefixed chunks (a removed component is skipped with a warning;
  changed fields need a version bump and a migration in `LoadStorage`). Entity storage is restored exactly (ids,
  versions, free list), so handles and future ids match. Corrupt or truncated data is rejected, never half-loaded.
- `Server::StartRecording` / `StopRecording` produce a `Recording` (start snapshot, inputs with ticks, non-1 step
  sizes, checkpoint hashes). `ReplayRecording` re-runs and reports the first diverging checkpoints.
- `Host`: `Start(Mode::Thread)` runs the sim on its own thread at real time × speed; `Mode::Pump` runs it on the caller's
  thread via `Pump(realSeconds)`. Client API: `SendEvent<E>` (input events only), `SendRequest(kind, text, blob)`,
  `Receive()`; `CurrentTick()` and `IsPaused()` are atomics. Save/Load/Text requests are answered at a tick boundary.
  A wake event during fast-forward resets speed to 1 and sends `Message::Kind::Woke`.
  `maxStepsPerService` (20) caps catch-up: when behind, real time is dropped, events are never skipped.

### 3.7 Text tools (Server::RunTextCommand, also through Host Text requests)
`state`, `dump #i`, `why <event id>`, `timeline #i`, `log [n]`, `scheduler`, `hash`, `types`, `tunables`,
`set <tunable> <value>` (queued as a logged `SetTunableRequested` input, so live tuning replays).

---

## 4. WorldNav contract

- **Bake** (`BakeWorldNav(levels, settings, out, report)`): input per level = name, `LevelNavData` (params + raw tile
  blobs), portals (`sim_portal`: name, trigger centre, targetLevel, targetSpawn) and spawns (`sim_spawn`: name,
  position). Steps: parse tiles and check one shared tile size and agent profile; snap portals and spawns to their
  level's mesh; lay levels out by BFS from the first level, each as close to its partner's portal as possible with at
  least one empty tile between tile bounding boxes; translate tiles by whole tiles (`tile.userId` = level index);
  add one one-way off-mesh connection per portal (trigger to named spawn, side `DT_OFFMESH_SIDE_REMOTE`); a level pair
  with portals only one way gets that portal made bidirectional (warning); mask tile borders facing another level.
  Validation (fails the bake, names map and entity): unknown target level or spawn, portal or spawn not on the mesh,
  unconnected level, link without a landing polygon, portal ends of one level that cannot reach each other, any level
  pair without a route.
- **World file** (format 2): `WorldNavData::Serialize/Deserialize` (zlib via ByteCompressor). Holds the spawn points (`WorldSpawn`: name, level, nav position,
  player start; snapped onto the mesh by the bake, a spawn that does not snap is skipped with a warning), `Fingerprint()` (FNV of level names,
  offsets, tile shifts and links: what saved positions depend on) and levels (name, source version =
  map mtime, offset, tile shift), links, translated tile blobs.
- **Runtime** (`WorldNav`, sim thread only): `Load` adds all tiles, then `connectRemoteOffMeshLinks` and checks every
  link landed. `Project`, `FindPath` (one-shot, tools), `PathService` (sliced A* with an iteration budget; results in
  request order; the result does not depend on the budget — tested). Positions are nav space;
  `ToNav(level, local)` / `ToLocal(level, nav)`; `LevelOfPoly`. `NavPath` has corners, corner levels, the polygon
  corridor, `complete`, length and `portalCrossings`. Query node pool 65,535 (Detour maximum).
- **Stale check** (`FindStaleLevels`, pure; bridge `SimWorldNavFile::CheckAtGameLoad`): a world level is stale when the map's
  modification time (`FileSystemEngine::GetFileModificationTime`, the value the bake stored in `WorldLevel::sourceVersion`)
  differs, or the map is gone. `GameStart` runs it once at game load in development builds (not for `-sim_selftest` and
  `-sim_bake_nav`) and logs `Logger::Error` banners: `==== SIM WORLD NAV IS STALE ...` (one line per level plus the fix), or
  `==== SIM WORLD NAV UNAVAILABLE` when the file is missing. The future SimBridge calls the same functions.
- **Debug draw**: `CollectDebugSegments(nav, level, centre, radius, out)` returns level-local lines (level < 0: the whole world in
  nav space): polygon edges (`Edge` shared, `Border` boundary), and for each portal a marker (diamond + 3 m beam):
  `LinkStart` (trigger end), `LinkStartBroken` (portal not in the mesh), `LinkEnd` (landing). Edges are culled by `radius`
  (an end within it), markers never. `SimNavDebugDraw` caches the level's segments, draws those within 100 m of the camera
  each frame with `DebugDraw::Line`, and reads its own copy of world.nav (game thread), never the sim's mesh.
- Known Detour behaviour: one-shot `findPath` and sliced A* may pick different equal-cost routes; translated meshes may
  break A* ties differently (float rounding). Reachability is always identical (tested).

---

## 5. Tests (all headless, ~4 s)

Run with the working directory set to the game folder `source/Games/SimGame` (its GameData is used directly; do not
rely on or copy the staged `Build/SimGame/windows/GameData`): `..\..\..\Build\SimGame\windows\bin\SimGame.exe -headless -sim_selftest`; the report is
`SaveData/sim_selftest.log` (first line: passed / failed / seconds). In a running game: console command `sim_selftest`.

| Suite | Covers |
|---|---|
| Kernel | archives, RNG, lazy values, scheduler (order, cancel, reuse, compaction), event log ring, registration errors, stagger, waves + causes + why-trace + timeline, rejection and stale handles, cascade cap, scheduled events in big steps, determinism (1,500 ticks × 40 agents, hash every tick), save round trip (byte-identical re-save, 1,500 ticks continued identically, killed agent stays dead, corrupt data rejected), replay incl. fast-forward steps and tamper detection, invariants and logged tunables, text commands, Host pump (accumulator, pause/step, fast-forward, wake, save/load, refusal of non-inputs), Host thread (input/output across threads, pause, step, save barrier, stop), throughput note |
| Nav | parse/rewrite of all 8 fixtures, translated level answers like the original (5 levels × 40 paths + 40 projections), 4-level chain bake with world file round trip, route across 3 portals both ways, level order of corners, sliced service, levels never stitch without links, validation messages, stale check (changed, missing and restored-older maps; map path to level name), debug geometry (edges and borders inside the level's bounds, one marker per portal end, levels add up to the whole world, range culling keeps markers) |
| Bridge | Base64 (known vectors, every length); world file v2 (spawns, fingerprint stability); sim save file (round trip, truncation, garbage, other format version); thread mode (20 start/stop cycles, speed, snapshots on the game thread, pause, text and save while paused, stop time, timeout); pump mode (same hashes twice, player anchor, snapshot tiers = TierOf, only the newest snapshot, level change, unknown level); save barrier (hash equal after load, 400 equal ticks, truncated / wrong schema / corrupt world refused with the sim untouched); rebake (other layout: 16 of 16 re-placed by level within 2.5 m, new walks, no violation; missing level removes its agents); snapshot and save cost at 400 walkers |

Measured (Release, this machine): toy world 400 agents ~0.5–1 M events/s; state hash of 400 agents 0.5 ms; longest test route (3 portals, 74 polygons) 55 us one-shot.

---

## 6. Level editor entities (GameData/entities.ent)

Engine and compiler classes copied from SlasherGame (worldspawn, func_group/detail, lights, static_mesh, misc_model,
func_door, triggers, navBlock, ...). Sim classes, not yet read by code except through the planned bake shell:

| Class | Kind | Keys | Read by |
|---|---|---|---|
| `sim_spawn` | point | targetname, angle, spawnflags bit 0 = default player start | bake (spawn), level transition (M2) |
| `sim_portal` | brush | targetname, targetLevel, targetSpawn | bake (portal), level transition (M2) |
| `sim_settlement` | brush | settlementId, displayName | M4+ |
| `sim_site` | brush | siteId, kind, displayName, capacity | M4 |
| `sim_door` | point | site, door (func_door targetname), angle | M2 path cache / M4 |
| `sim_slot` | point | site, slotType, angle | M4 |
| `sim_container` | point | containerId, site, containerType, access, model | M4 items |
| `sim_npc` | point | npcId, angle | M9 authored NPCs |

---

## 7. Next steps (in order) — for the next session

1. ~~Bake console command~~ **done**: `SimGame.exe -headless -sim_bake_nav` (cwd `source/Games/SimGame`, so the output lands in the
   source GameData) loads every manifest map through `Level::LoadLevelFromFile` (the engine builds or loads each level's nav),
   captures the live navmesh tiles and the sim_portal / sim_spawn entities, bakes, writes `GameData/sim/world.nav`, reloads it
   and routes from the root level's player start to every spawn of every other level. Report: `SaveData/sim_bake_nav.log`.
   Both headless runs were confirmed from the game folder (2026-10-23). The engine writes `<map>.bsp.nav` next to each map on
   first load; they and `world.nav` are untracked: commit them if the team wants the baked files in the repo.
2. ~~Stale check at game load~~ **done** (section 4).
3. ~~WorldNav debug draw~~ **done** (section 4). Verified headless with `-map sim_test_a.bsp -sim_nav_draw` (1,235 segments, no
   crash); the picture itself still needs a look in a visible run.
4. ~~M2a abstract movement~~ **done** (section 8, `SLICE_M2_MOVEMENT.md`).
5. ~~Bridge and walking pawn~~ **done** (section 9, `SLICE_M2B_BRIDGE.md`).
6. ~~Bodies and the spawn handshake~~ **done** (section 10, `SLICE_M2C_BODIES.md`). Next for bodies: doors and dynamic obstacles, bodies avoiding
   each other, reactions, hitboxes and death.
7. **Custom level transition** (addendum 5.2): `sim_portal` triggers act on the pawn, no `TimeSkip`, the pawn goes through the same portal pair
   in the sim. Needs a decision on how the pawn is carried between levels (`LevelTraversalSystem::PlayerSerializedData`).
8. **Abstract-versus-physical calibration** (addendum 7.1): walk a real body along a route a headless agent also walks; the pawn already uses the
   shared speed table.

Settled by the owner:
- Calendar time scale (2026-10-23): **0.5 game minutes per world second** (1 game minute = 2 s, 1 game hour = 2 min, a day =
  48 min). Movement stays in real world seconds (0.1 s ticks). Needs rates and schedules (M4) are derived from it.

Open questions for the owner (ask before the step that needs them):
- Where the world manifest and world.nav live: `GameData/sim/` is in use (source GameData); confirm, and whether world.nav
  and the `sim_test_*.bsp.nav` files are committed (they are untracked). world.nav is format 2 now: rebake once after pulling.
- The web build: nothing from M2b was built or run for wasm (threads, `Helpers/Base64.h` in `EmscriptenFileSystem.cpp`).

---

## 8. Movement contract (Sim/Movement, tests in SimMovementTests.cpp)

Use: `MovementModule movement(worldNav)` (WorldNav loaded, outlives every World), `builder.Add(movement)` after the kernel.
`movement.Place(ctx, navPosition)` in `RunSetup` creates a walker. A rule asks for a walk with the `RequestWalk` command; it
never sets a position (design 5.4). Everything is in nav space; the level is derived (addendum 4.5).

| Record | Notes |
|---|---|
| `Position {nav}`, `Walker {...}` | components; `Walker` holds state (Idle, WaitingForPath, Walking), gait, `speedScale` (per agent, from the `placement` stream), the path (`corners`, `cornerLevels`), `progress` (leg, metres into it, portal wait), `lastIntegrated` and the detector fields |
| `PlayerAnchor {level, nav}` | the player as the game tells it; the only stored level |
| input `PlayerMoved {level, local}` | rule `player_moved` -> command `SetPlayerLocus` (unknown level is rejected) |
| commands | `RequestWalk {walker, destination, gait}`, `StopWalk`, `SetGait`, `SetPlayerLocus`; each syncs a walking walker to now first, so speed changes and stops are exact |
| events | `PathAssigned {length, corners, portals}`, `PathFailed {reason}` (start_off_mesh, destination_off_mesh, no_path, unreachable; partial paths fail), `PortalCrossed`, `Arrived {at}` (`at` = exact tick, the event may come later), `PositionException {reason: placed, portal}` |
| tunables | `movement.tier0_radius` 50 m, `tier0_step` 1, `tier1_step` 2, `tier2_step` 10 ticks, `portal_delay` 1 s, `paths_per_tick` 16, `speed_scale_spread` 0.1 |

- **Systems** (kernel order): `movement.paths`, then `movement.integrate`.
  `paths`: walkers in WaitingForPath, oldest `requestedAt` first (ties by entity index), at most `paths_per_tick` per step; start
  and destination are projected, requests go through `PathService`, `Update(INT_MAX)` solves them to completion inside the
  step, results come back in request order. The queue is the set of waiting walkers (components), so nothing is lost across a
  save. Deviation from addendum 6.3 (slicing across ticks), owner-approved: the longest test route costs 55 us.
  `integrate`: each Walking walker is due by `IsStaggerDue(entity, now, dt, step of its tier)` and then advances by the time
  since `lastIntegrated`. Tier: T0 within `tier0_radius` (horizontal) of the player in the player's level, T1 the rest of that
  level, T2 other levels or no player. A larger kernel step (fast-forward) makes everybody due; results do not depend on it.
- **Walking**: `Advance` consumes `speed x time` along the corners (3D length). Speed = table(class, gait, area) x `speedScale`.
  A leg whose two corners lie in different levels is a portal: walk to its start, wait `portal_delay`, continue from its end.
  The straight line between the two ends is never walked and never counted. Progress is (leg, metres into the leg) in double
  precision, so one big step and many small steps end at the same place.
- **No-teleport rule**: `AdvanceWalker` (and `Place`) are the only writers of `Position`. Invariant `movement.no_teleport`
  (runs after every step when `World::checkInvariants`): `Position` equals what the module last wrote, the last integration did
  not walk further than the fastest gait x `speedScale` allows, and a position written this step is on the mesh (0.3 x 1 x 0.3 m
  projection). Position changes that are not walking are `PositionException` events: `placed`, `portal`.
- `PositionAt(world, entity, tick)` extrapolates along the path without changing anything; `TierOf`, `LevelOf` are queries.
- Not in this slice: bodies and materialization, calibration against a real body, per-area speeds (the path does not report
  areas yet), groups, doors, encounters.

Tests (added to `-sim_selftest`): walking maths alone (big step = small steps at every instant, portal delay, speed change,
degenerate paths); tiers (radius, level, player moving between levels, 50 / 10 integrations per 100 ticks for tier 1 / 2,
a portal crossing changes tier); path requests (cap, oldest first, destroyed walker, destination off the mesh, unreachable island);
gait change mid-walk, stop, extrapolation versus a tick-by-tick walker; detector (foreign write, over-speed, off-mesh);
fast-forward (blocks of 1, 10, 600 ticks: same arrival ticks); commute (40 walkers over 4 levels / 6 portals for 18,000 ticks =
30 min: 981 trips, 1203 portal crossings, every arrival within 1.5 ticks of length / speed + delays, no violation, replay and
mid-walk save/load identical). Cost: 400 walkers 36 us per tick (Release, this machine).

---

## 9. Bridge contract (Sim/Bridge, SimPawn, tests in SimBridgeTests.cpp; `SLICE_M2B_BRIDGE.md`)

- **Start**: `GameStart::Start` calls `SimBridge::Start(demoCount, test)` unless the run is `-sim_selftest` or `-sim_bake_nav` (those also disable
  the pawn). It loads `GameData/sim/world.nav` into the bridge's own `WorldNav` (stale check as before), builds movement (+ demo in development
  builds) into a registry, creates a `SimClient` (Host in **Thread mode**, seed 1) and registers the save extension. A missing or unreadable world
  file leaves the game running without a sim and logs `==== SIM NOT STARTED`. Stop: on `SDL_QUIT` (event watch) and from `atexit`.
- **Every frame** (`GameStart::Update`, after the pause state): pause menu -> `Pause`/`Resume`; `SimClient::Update` drains the Host; the pawn's feet
  position goes out as `PlayerMoved {level, local}` when the level changes, after 0.5 m, or after 0.5 s; development builds draw each agent of the loaded
  level as a box (tier 0 green, 1 yellow, 2 grey).
- **Pawn** (`SimPawn`, "sim_pawn"): CharacterController, mouse look, forward/backward/left/right/crouch/jump; run speed from the speed table (8.0 m/s),
  walk speed (2.4 m/s) while crouching; eye camera; saved with the level; `SimSpawn` creates it at the default player start (once per level load, using
  `Level::CurrentLevelChangeId`). `StartScriptedWalk(metres)` for automated runs. The editor camera is no longer the player.
- **Startup map**: without `-map` the game opens the first map of `GameData/sim/world.manifest` (was blank.bsp).
- **Snapshot** (Host addition): `HostHooks::publishSnapshot` runs on the sim thread every 2 ticks (5 Hz); `MovementModule::WriteSnapshot` writes a
  `SnapshotView` (player anchor + an `AgentView` per walker: entity, level, tier, state, nav position); the client keeps only the newest.
- **Host hooks** (project code): `beforeSave` (placements of every walker, computed at the barrier tick), `afterLoad` (rebake migration). `Message` gained
  `Snapshot` and `extra`; `Save` with `value > 0.5` leaves the event log out; `Host::SendRequest(Message)`.
- **Save**: the engine hook `GameSaveSystem::RegisterExtension` (section 1). `save` -> `SimClient::SaveBlocking` (the game thread waits for the sim's
  answer at its next tick boundary, even while paused; 2 s timeout fails the whole save, nothing is written). `load` runs before the level is touched:
  `LoadBlocking`, false cancels the load. A save without a sim entry starts a fresh sim (logged). The event log is kept in development builds only.
  The file: `SimSaveFile` (magic, format 1, tick, **schema hash** = component names + versions, **layout fingerprint**, level names, placements, kernel blob,
  zlib). A different schema or format refuses the load with the sim untouched; the kernel rejects corrupt blobs the same way.
- **Rebake** (fingerprint differs): after the kernel load, one logged setup (`rebake migration`) puts every walker back at its level (by name) and
  level-local position on the new mesh (projection 2, 10, 50 m), clears its path, emits `PositionException(rebake)` and `Replaced`; the demo rule sends it
  on a new trip. A level that is gone, or no mesh within 50 m, removes the agent with an error naming it. The player anchor is dropped (the pawn reports again).
- **Console** (development): `sim_pause`, `sim_resume`, `sim_step n`, `sim_speed x [stepTicks]`, `sim_text <command>`, `sim_stats`,
  `sim_demo [count]`, `sim_agents [on|off]`, `sim_player x y z`.
- **Automated in-game run**: `SimGame.exe -headless -sim_bridge_test` (cwd = game folder) walks the pawn 20 m with 40 demo walkers, checks the anchor and
  that agents moved, pauses, saves through `GameSaveSystem` (one `.savg` with the sim entry), lets the sim run on, loads, checks the state hash equals the
  saved one, checks the sim keeps running, writes `SaveData/sim_bridge.log` and quits. 13 checks pass; exit code 0.
- Not verified: how it looks and feels (boxes, pawn movement, camera) in a visible run; the web build.


---

## 10. Bodies contract (Sim/Bodies, Sim/Bridge/SimAgents, SimNpc; tests in SimBodiesTests.cpp; `SLICE_M2C_BODIES.md`)

- **Direction** (owner): the sim dictates targets, the body navigates by itself with its own movement and state code. Look: a mix of base.glb and guard.glb.
- **Sim**: `BodiesModule(movement, worldNav)`, registered after the movement module. Output events `BodySpawnProposed`, `BodyDespawnProposed`, `BodyGoal`;
  input events `BodyAccepted`, `BodyRefused`, `BodyReleased`, `BodyReport`, `BodiesCleared` (one command `ApplyBodyInput`, logged and replayable). Components
  `Embodied` (movement), `Appearance`, `BodyRetry`. Proposals every 5 ticks (0.5 s), nearest first within `spawn_radius` 120 m (release beyond `despawn_radius` 140 m, in another
  level, or at a portal; the despawn event carries the reason OutOfRange / OtherLevel / NoPlayer / AtPortal), budget 32. The radii are wider than the 64 m test levels on
  purpose, so everything inside a level has a body; a big open level wants about 80 and 100. While a body is Active it is the mover: `MovementModule::ApplyBodyPosition` moves the walker's progress to the nearest point on its route (forward only,
  never faster than the fastest gait), re-plans when the body stands more than 3 m off, arrives within 0.75 m of the end, steps onto a portal leg at the end of the leg
  before it (the sim crosses). Silent bodies (3 s), unanswered proposals (3 s) and unreleased despawns (6 s) are taken back. A load drops all `Embodied`.
- **Game**: `SimAgents::Update` (from `SimBridge::Update`) answers proposals. Every proposal is accepted (owner: bodies fade in and out, also in sight and close by). It is
  answered from the first snapshot at least as new as the proposal's tick (snapshots trail the sim by a tick or two; an older one still shows an agent on the far side of a
  portal it just took, which once put bodies at another level's coordinates, 45 m off, "out of sight"); if the agent is no longer in the player's level there is no body (refused,
  logged "left the level before its body appeared"). The body appears where the agent is then. **Appearing**: at once right after a level load (3 s window: the loading screen
  hid it) and when the spot is out of sight (outside a 70 degree half cone, behind world geometry, or in a BSP cluster the camera cannot see); otherwise it fades in over
  `SimNpc::FadeSeconds` (1 s). **Disappearing**: out of sight at once, in sight it fades out first (still walking and reporting meanwhile, so the sim keeps following it); the
  reason (portal, range, level) only decides when it is proposed. Bodies report their feet position every 0.2 s. Level changes destroy the bodies and send `BodiesCleared`.
  `SimNpc` ("sim_agent_body"): NPC-quality CharacterController, `PathFollowQuery` on the level mesh, speeds from the shared table x `speedScale`, Idle / Moving / Blocked
  states, walks on to the next corner without stopping, idle / walk / run animation from the model's clips by speed. **Fade**: the mesh tint alpha (`Color.a`) runs 0 to 1 or
  1 to 0; below 1 the engine draws the mesh as a blended Transparent surface (no depth write, so limbs show through each other for that second), at 1 it is back to the
  alpha-tested `Masked` draw (`SimNpc::UpdateFade`).
- **Look** (owner, 2026-10-23): `base.glb` has no textures, so citizens are a plain white-textured figure with a per-person tint (a mannequin, kept on purpose); `guard.glb`
  is textured and left alone. The tint comes from the sim entity number, so a person always looks the same. Both read well in screenshots at 5 to 25 m.
- **Tools**: `sim_stats` prints bodies, proposals, spawned at once / faded in, refusals, despawns, goals and reports; boxes are drawn only for agents without a body. Every
  appearance and disappearance is logged: `sim body: <id> appears at (x y z), N m away, <the level just loaded | out of sight | fading in, in sight>`, `sim body: <id> goes:
  <reached a portal | out of range | player left the level | no player> (<faded out | out of sight>)`, `sim body: <id> left the level before its body appeared; no body`.
  Named NPCs carry a beam over their head in the sim's position: grey no body, yellow asked for, green has one, orange being taken back (note: an agent that has stepped
  onto a portal leg is already in the next level for the sim, so its beam is gone while its body is still fading out at the portal).
- **Tested**: scripted stand-in game (budget, order, hysteresis, body-driven arrival times, goals, re-planning, time-outs, refusal cool-down, release, portal handoff,
  stray reports, load, replay with body reports) and the in-game run (9 bodies appear out of sight, walk, the sim follows within 3 m, they return after a load).
- **Looked at** (screenshots, 2026-10-23): bodies stand where the sim's agent is (the view is centred on the agent and the body is in the middle), animate, are textured or tinted,
  fade in at the portal exit 1.4 m from the door and out at the portal; a 6 s fade showed the blend. **Not verified**: the web build, whether the walk animation matches the
  speed table (nobody compared times), how a fade looks at 60 fps in motion (only stills), and the missing-file notes the engine logs for `guard.glb`'s external textures
  (`models/npc/guard.glb/body.png` and others; the model still draws textured).


---

## 11. Authored NPCs (Sim/Npcs, Sim/Bridge/SimNpcFile, GameData/sim/npcs.json; tests in SimNpcsTests.cpp; `SLICE_M2D_NPCS.md`)

- **Content is data** (owner): `GameData/sim/npcs.json`, always on, no launch flag. Ten NPCs ship (two models, one crossing levels). Format: header of `SimNpcFile.h`.
  `SimGame.exe -headless -sim_nav_report` writes `SaveData/sim_nav_report.txt` with coordinates to author against.
- **Start**: `SimBridge::Start` parses the file, validates every NPC against the baked world (`NpcModule::Validate`), logs `==== SIM NPC CONTENT` with every problem,
  and places the valid ones in each fresh sim (not after a load: the save holds them). Module order: movement, bodies, npcs, demo.
- **Records**: `movement::Identity {id, name}`, `npcs::Routine`, `bodies::Appearance`. Stops are level name + level-local position (rebake-proof). Events: `RoutineStep`
  (scheduled). Rules: `Arrived` -> wait in the stop's range -> `RoutineStep` -> command `RoutineAdvance` -> `MovementModule::StartWalk`; `PathFailed` / `Replaced` retry after 3 s.
  The crowd (demo) rules skip anything with an `Identity`.
- **Tools**: console `sim_npcs`, `sim_goto <id>`, `sim_look <id>` (the pawn stands 6 m from the NPC once and the view stays on it; launch `-sim_look <id>`, with `-screenshot` for a
  picture), beam over named NPCs coloured by body state (section 10; `sim_agents`); snapshot `AgentView.id`.
- **Looked at** (screenshots, 2026-10-23): the NPCs of level a (warden, baker, widow, newsboy, sergeant, courier) have bodies near the pawn and walk; the courier crosses the portal
  both ways (out at 3 s, back at 38 s, appears at the door). Not looked at: the residents of the other levels, whether each route's stops and waits look right in play, NPC
  reactions (not built yet).
