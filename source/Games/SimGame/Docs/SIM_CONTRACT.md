# SIM_CONTRACT

The working contract of the SimGame simulation: module map, public interfaces, invariants, tunables, what is tested
where, and what comes next. **Every session starts by reading this file and updates it before ending** (addendum
section 17.7). Design and addendum stay the source of truth for *decisions*; this file is the source of truth for *code*.

Status: 2026-10-23. Kernel (M1 core, parts of M3) and WorldNav (M0, including the engine bake command) are done and tested.
The test world (`GameData/sim/world.manifest`: sim_test_a, sim_test_road, sim_test_b, sim_test_c) bakes with 6/6 portals linked.

---

## 1. Ground rules (decided by the owner, override the addendum where they differ)

| Rule | Detail |
|---|---|
| The sim lives in the game project | `source/Games/SimGame/Sim/`, compiled into SimGame.exe. There is **no** engine-free library and no separate test executables (overrides addendum 17.3 / 17.5). Use of engine code stays limited: entt, glm, Detour, `Logger`, `ByteCompressor`, `FileSystemEngine` (bridge only). |
| Prefer entt containers | `entt::dense_map` / `dense_set` over `std::map` / `unordered_map` in sim code. They iterate in insertion order, so iteration is deterministic. |
| The sim never touches engine statics | No `NavigationSystem`, `EcsWorld`, `Level`, `Time`, `Input` from sim code. The sim thread owns its own entt registry and its own `dtNavMesh`. |
| World nav is ONE merged Detour mesh (D3) | Built from the levels' navmesh tiles (the tiles the engine's tile-cache pipeline produced, so the same polygon grid). The tile cache itself is **not** part of the world mesh. |
| One engine change so far | `dtNavMesh::connectRemoteOffMeshLinks` + `DT_OFFMESH_SIDE_REMOTE` in `source/Engine/Navigation/Detour/DetourNavMesh.{h,cpp}`: purely additive, approved by the owner. List any further engine change here. |
| Builds and tests | The implementer may build and run headless tests (see section 7). Never run the game without `-headless`. |

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
  Bridge/                   engine side (the only code that uses both the sim and engine gameplay types)
    SimMapEntities.h/.cpp   sim_portal (TriggerBase; keeps targetLevel, targetSpawn, brush centre) and sim_spawn entities
    SimNavBaker.h/.cpp      bake shell: console `sim_bake_nav [manifest]`, launch `-sim_bake_nav [manifest]` (bakes, quits)
  Tests/
    SimTestFramework.h      TestContext (Check / Note / Section), Stopwatch, suite declarations
    SimSelfTests.h/.cpp     RunSimSelfTests + console command sim_selftest
    SimKernelTests.cpp      kernel suite, with a toy "test" module that exercises every kernel path
    SimNavTests.cpp         world nav suite on real GameData/maps/*.bsp.nav fixtures
GameData/entities.ent       level editor entity definitions (engine classes + sim_* classes, section 6)
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
- **World file**: `WorldNavData::Serialize/Deserialize` (zlib via ByteCompressor). Holds levels (name, source version =
  map mtime, offset, tile shift), links, translated tile blobs.
- **Runtime** (`WorldNav`, sim thread only): `Load` adds all tiles, then `connectRemoteOffMeshLinks` and checks every
  link landed. `Project`, `FindPath` (one-shot, tools), `PathService` (sliced A* with an iteration budget; results in
  request order; the result does not depend on the budget — tested). Positions are nav space;
  `ToNav(level, local)` / `ToLocal(level, nav)`; `LevelOfPoly`. `NavPath` has corners, corner levels, the polygon
  corridor, `complete`, length and `portalCrossings`. Query node pool 65,535 (Detour maximum).
- Known Detour behaviour: one-shot `findPath` and sliced A* may pick different equal-cost routes; translated meshes may
  break A* ties differently (float rounding). Reachability is always identical (tested).

---

## 5. Tests (all headless, ~2 s)

Run: `cd Build/SimGame/windows` then `bin/SimGame.exe -headless -sim_selftest`; the report is
`SaveData/sim_selftest.log` (first line: passed / failed / seconds). In a running game: console command `sim_selftest`.

| Suite | Covers |
|---|---|
| Kernel | archives, RNG, lazy values, scheduler (order, cancel, reuse, compaction), event log ring, registration errors, stagger, waves + causes + why-trace + timeline, rejection and stale handles, cascade cap, scheduled events in big steps, determinism (1,500 ticks × 40 agents, hash every tick), save round trip (byte-identical re-save, 1,500 ticks continued identically, killed agent stays dead, corrupt data rejected), replay incl. fast-forward steps and tamper detection, invariants and logged tunables, text commands, Host pump (accumulator, pause/step, fast-forward, wake, save/load, refusal of non-inputs), Host thread (input/output across threads, pause, step, save barrier, stop), throughput note |
| Nav | parse/rewrite of all 8 fixtures, translated level answers like the original (5 levels × 40 paths + 40 projections), 4-level chain bake with world file round trip, route across 3 portals both ways, level order of corners, sliced service, levels never stitch without links, validation messages |

Measured (Release, this machine): toy world 400 agents ~0.5–1 M events/s; state hash of 400 agents 0.5 ms.

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

1. ~~Bake console command~~ **done**: `bin/SimGame.exe -headless -sim_bake_nav` (cwd `Build/SimGame/windows`) loads every
   manifest map through `Level::LoadLevelFromFile` (the engine builds or loads each level's nav), captures the live
   navmesh tiles and the sim_portal / sim_spawn entities, bakes, writes `GameData/sim/world.nav` (build GameData),
   reloads it and routes from the root level's player start to every spawn of every other level. Report:
   `SaveData/sim_bake_nav.log`. Still to do: the stale check at game load (compare `WorldLevel::sourceVersion` with
   the map's mtime, loud warning in development builds), and deciding whether world.nav is copied into the source
   GameData (open question below).
2. **WorldNav debug draw** (bridge): draw the loaded level's part of the world mesh and the links with `DebugDraw`.
3. **M2 movement** on top of kernel + WorldNav: walker component (path corners, progress, speed), movement system
   stepping by tier (T0/T1/T2 by distance to the player, addendum 6.2), shared speed table, arrival events, teleport
   detector invariant, path requests through PathService applied at one point in the tick.
4. **Bridge entity** (`SimBridge`, persistent like `gamestart`): owns `Host` in Thread mode, sends player position,
   receives outputs; spawn handshake with PVS; custom level transition (addendum 5.2); save chunk through
   `GameSaveSystem` via the save barrier.

Open questions for the owner (ask before the step that needs them):
- Calendar time scale: movement is real world time (0.1 s ticks); how many calendar minutes per world second? Needed
  by needs rates and schedules (M4).
- Where the world manifest and world.nav live (`GameData/sim/` proposed).
