# Slice contract: M2b the bridge (SimBridge) and a walking pawn

Status: **implemented** (2026-10-23; the owner said "continue developing" after the v2 proposal). Section 11 lists where the code differs from the text
below. The owner's answers to the first round are in section 10.
Follows addendum 17.4. Sources: addendum 3 (server and client), 3.3 (save barrier), 5.2 (transition), 6.1; SIM_CONTRACT 3.6.
Goal: the sim runs next to the game on its own thread, a walking pawn is the player the sim follows, demo agents walk the
levels and can be seen as boxes, and the sim is saved and loaded with the game.
Bodies, the spawn handshake and the level transition are **not** in this slice (section 8).

## 1. Done when

- The game starts the sim thread at launch and stops it cleanly at exit, in a normal and in a `-headless` run.
- You can walk the pawn through the test levels; the sim's `PlayerAnchor` follows it, and the tiers of the demo walkers follow
  the anchor, including after a level change by `-map` / console `map`.
- 40 demo walkers walk and cross portals while the game runs; the game draws each as a box coloured by tier.
- Save and load through the existing save menu and quick save/load give one file holding level and sim; after a load the sim's
  state hash equals the saved one and it continues identically to a sim that never stopped.
- A save made against an older `world.nav` loads: agents are re-placed by level (section 6.3).
- Headless tests cover all of it (section 7) and `-headless ... -sim_demo` runs clean.

## 2. Files

```
Sim/Bridge/
  SimClient.h/.cpp          game-thread side of the Host, no engine dependency: owns the Host (Thread or Pump), reports
                            the player, drains outputs and snapshots, blocking save/load, counters. Testable in self tests.
  SimSaveFile.h/.cpp        the sim save blob: header (format, fingerprint, level table, placements) + kernel blob; pure
  SimBridge.h/.cpp          engine glue: static singleton started by GameStart; Level::Current, pawn, PauseGameManager,
                            world file, GameSaveSystem extension, console commands, snapshot debug draw
Sim/Movement/
  SimDemoPopulation.h/.cpp  the demo module (section 5)
SimPawn.h/.cpp              the walking pawn (section 3.4), a plain Entity on the engine's CharacterController
```

`SimClient` has the logic; `SimBridge` is thin. SimGame stays one project (SIM_CONTRACT 1).

## 3. Lifecycle

1. `GameStart::Start` (first call only) calls `SimBridge::Start()`: load `world.nav` (stale check as today) into the bridge's own
   `WorldNav`, build the registry (movement + demo module, fixed order), create the Host with a fixed seed, run the demo
   setup, `Start(Mode::Thread)`. A missing world file leaves the bridge off with the existing loud error.
2. Each frame, early in `GameStart::Update`: `SimBridge::Update()`: report the player (3.1), forward pause (3.2), drain
   `Receive()`, keep the newest snapshot, draw it (4).
3. Exit: the thread must stop before the engine's statics die and `EngineMain` has no game shutdown hook, so the bridge stops
   the Host on `SDL_QUIT` (event watch) and again from `atexit`. The headless run proves it exits.
4. The game thread never reads sim state except through messages; the Host is untouched while its thread runs except through
   `Send` / `Receive` / the atomics.

### 3.1 Player report

`SimBridge` sends `PlayerMoved {level, local}` (input event, exists) when the level changes, the pawn moved at least 0.5 m since
the last report, or 0.5 s passed. Level index: `SimWorldNavFile::FindLevel(data, Level::Current->filePath)`; a level that is not in
the world file reports nothing and logs once. Position: the pawn's feet position.

### 3.2 Pause and speed

`PauseGameManager::GetGamePaused()` changes -> `Pause` / `Resume`; the sim does not run under the pause menu.
`SimClient::SetSpeed(multiplier, stepTicks)` exists for fast-forward later; this slice only exposes it to the console.

### 3.3 Console (development builds)

`sim_pause`, `sim_resume`, `sim_step <n>`, `sim_speed <x> [stepTicks]`, `sim_text <command>` (kernel text tools: `state`,
`dump #i`, `why <id>`, `timeline #i`, `log`, `scheduler`, `hash`, `types`, `tunables`, `set`), `sim_stats` (tick, messages,
agents per tier), `sim_demo [count]`, `sim_player <level> x y z` (moves the pawn to a level-local spot).

### 3.4 The walking pawn

A deliberately small `Entity` ("sim_pawn"): engine `CharacterController` (player quality), mouse look, the input actions
GameStart already registers (forward, backward, left, right, crouch, jump), walk speed from the shared speed table (so a pawn
and a walking agent cover ground at the same pace), eye camera through the engine's `Camera`. No weapons, UI, interaction or
animation. It is created at the level's `sim_spawn` marked as the default player start (spawnflags bit 0) by `SimSpawn` when
no pawn exists, and it is saved with the level like ShooterGame's Player (`SaveGame`). Taking it through a level transition is
the later transition slice; this slice only needs the pawn present in whichever level is loaded.
For the headless run the pawn accepts a scripted move (`-sim_pawn_walk <metres>`) so the in-game test can walk it and check
that the sim followed.

## 4. Snapshot (a Host addition, project code)

The sim must never be read by the game thread, yet the game wants to draw agents, and the later spawn handshake needs the same
path. The Host gets an optional publisher `std::function<void(const World&, Message&)>` and an interval (default every 2 ticks,
5 Hz). After a step that crosses the interval the publisher runs on the sim thread; its blob arrives from `Receive()` as
`Message::Kind::Snapshot`. The movement module's publisher writes, per walker, `{entity, level, nav position, tier, state}`
(about 28 bytes) with the player's level first. The client keeps only the newest snapshot. `SimBridge` draws each agent in the
loaded level as a `DebugDraw::Bounds` box (about 0.5 x 1.8 m) at its level-local position: tier 0 green, tier 1 yellow, tier 2
grey (tier 2 agents are drawn only when in the loaded level, which only happens before the player exists). `sim_stats` prints
the counts per tier.

## 5. Demo population (development only)

A module `demo`, registered when `-sim_demo [count]` or `sim_demo [count]` asks (default off; absent in Distribution builds):
places `count` walkers (default 40) at the manifest's spawn points round robin and keeps them commuting between spawns with the
trip rule the movement tests use now. That rule moves from `SimMovementTests.cpp` into this module and the tests use the module,
so the tested rule is the shipped one. Without the demo the bridge still runs with an empty world.
`sim_demo` at the console after the start rebuilds the sim (stop, new Host, place, start); the spawn list comes from a new
`spawns` table in the world file (name, level, nav position). The bake takes spawns as input but does not store them yet, so
this is a world file format bump (version 2, rebake needed; a version 1 file is refused with the rebake hint). The sim never reads maps.

## 6. Saving

### 6.1 Engine change: a save extension hook (owner-approved; list in SIM_CONTRACT 1 when done)

Additive, in `GameSaveSystem` (and its data), nothing changes for games that register no extension:

```cpp
struct GameSaveExtension {
    std::string id;                                          // key in the save file, e.g. "sim"
    std::function<std::vector<uint8_t>()> save;              // called by SaveGameToData
    std::function<bool(const std::vector<uint8_t>&)> load;   // called first by LoadGameFromData; empty vector = the save has none
};
static void GameSaveSystem::RegisterExtension(GameSaveExtension);
// GameSaveData gains:  std::unordered_map<std::string, std::string> Extensions;  // base64, DEFAULT-initialised so old saves load
```

`SaveGameToData` fills `Extensions[id]`; `LoadGameFromData` calls every extension's `load` before touching the level; if one
returns false the load stops there, logs why, and nothing is changed. Bytes in JSON need Base64, which exists only as two
file-static helpers in `EmscriptenFileSystem.cpp`; they move unchanged to `Helpers/Base64.h` (one header, `inline`), and that
file includes it. This is the second engine change: **it touches the web file system, which this machine cannot build, so I
will keep the move literal and say so in the report.**

### 6.2 The barrier

The extension's `save` calls `SimClient::SaveBlocking(timeout)`: sends the `Save` request, waits for the reply on the game
thread (the sim answers at its next tick boundary, at most one step plus the 50 ms service wait, even while paused), and
returns the blob. The game thread stalls for a few frames at most, the engine's own save is captured in the same call, so level and
sim always describe the same moment (addendum 3.3). A timeout (2 s) fails the save loudly; the engine save is then not written.
`load` mirrors it with `LoadBlocking`. Blobs are compressed with `ByteCompressor`. The event log is included in development
builds and left out of Distribution builds (log size is the cost; the why-trace needs it).

### 6.3 Format and the rebake case

`SimSaveFile`: magic, format version, tick, seed, module list hash, **world fingerprint** (FNV of the world file's levels: name,
offset, tile shift, and links), the **level table** at save time, **placements** (for every walker and the player: level name,
level-local position, computed on the sim thread by the module while the barrier holds), then the kernel blob.

On load, equal fingerprint: nothing else happens. Different fingerprint (a rebake moved or reordered levels) - owner decision:
**re-place agents by level**. After the kernel load, on the sim thread, one logged setup (`rebake migration`):
- for each placement: find the level by name in the new world; project its local position onto the new mesh (2 m, then 10 m, then
  50 m); set `Position`, clear the path, state Idle, `lastTo` = the new position, emit `PositionException(rebake)` and a new
  event `Replaced {walker}` that rules can react to (the demo's rule starts a new trip);
- a level that no longer exists, or a position with no mesh within 50 m: the agent is destroyed and the loss is logged as an error
  naming the agent (an agent without a place cannot be simulated);
- `PlayerAnchor` is dropped; the pawn reports again within 0.5 s;
- pending path requests are dropped with the paths (state Idle).
A format version or module list hash mismatch (not just the layout) refuses the load as a whole, with a message, because component
fields may differ.

### 6.4 Where the game calls it

`UiSaveLoadMenu`, quick save and quick load already call `GameSaveSystem`; they need no change. The extension is registered by
`SimBridge::Start()`. A save without a sim entry (older save) loads the level and starts a fresh sim, logged.

## 7. Tests

Self tests (`SimClient` against the real chain world, Thread and Pump mode, no engine statics):
- Thread mode: starts, ticks follow real time x speed, stops within a bounded time with no leaked thread; start-stop 20 times.
- Pump mode: the same scenario twice gives equal state hashes; player reports reach `PlayerAnchor` (unknown level ignored); snapshot
  tier counts equal `TierOf`; a level change moves tiers; only the newest snapshot is kept.
- Pause freezes the tick, resume continues; speed and step; text replies (`state`, `why`).
- Save: save during a walk, keep running, load into a second client, hashes equal, both equal for N ticks; truncated blob, wrong
  format version and wrong module hash are refused and leave the sim untouched; a save requested while paused is answered; timeout path.
- Rebake: save on chain A, load onto chain B with a different level order: every agent is at its level and local position (within the
  projection), idle, with a `Replaced` event; they resume trips; no detector violation; a removed level destroys its agents with an error;
  equal fingerprint changes nothing.
- `SimSaveFile` round trip and fingerprint stability (same world twice = same hash, one moved level = different).
- Base64 helper round trip (including length 0, 1, 2, 3); `GameSaveExtension` JSON round trip: an old save without `Extensions` loads.
- Snapshot size and cost at 400 walkers.
In-game, headless (`-headless -map sim_test_a.bsp -sim_demo 40 -sim_pawn_walk 20 -quit_after 20`): writes `SaveData/sim_bridge.log`:
ticks reached, snapshots received, arrivals, pawn moved and the anchor followed, a save/load round trip through the real save
file (hashes), clean exit.

## 8. Not in this slice

Bodies and the spawn/despawn handshake (it will reuse the Snapshot and message path), the custom level transition (addendum 5.2;
needs the pawn and the `sim_portal` triggers acting on it), agent context cards, door/flag mirroring, calibration against a real
body, pawn polish (weapons, interaction, animation, UI).

## 9. Engine changes this slice makes (to be listed in SIM_CONTRACT 1)

1. `GameSaveSystem`: `GameSaveExtension`, `RegisterExtension`, `Extensions` in `GameSaveData`. Additive.
2. `Helpers/Base64.h`: Base64 encode/decode moved out of `EmscriptenFileSystem.cpp`. Not buildable here (web only).

## 10. Owner answers (2026-10-23)

- Player source: a **minimal walking pawn** (not the editor camera).
- Demo: **40 commuting walkers drawn as boxes** (needs the Snapshot message).
- Save files: **engine hook**, one file.
- World.nav mismatch on load: **re-place agents by level**.

Defaults I took without asking (change them if you disagree): fixed seed 1 for the demo world; event log kept in saves in
development builds; a destroyed agent when its level is gone; pawn speeds from the shared speed table; the pawn is created by
`SimSpawn`, not by a separate map entity.

## 11. As built (differences from the text above)

- The save header carries a **schema hash** (component names and versions) instead of a module list hash, so toggling the demo module does not
  invalidate saves; a different schema or format refuses the load, the layout fingerprint decides the rebake migration.
- The Snapshot also carries the sim's idea of the player (`hasPlayer`, level, nav position); "the player's level first" was dropped, the client filters by level.
- `sim_player` takes `x y z` in the loaded level (no level argument). `-sim_pawn_walk` was folded into `-sim_bridge_test`, which implies 40 demo walkers.
- The demo module is always registered in development builds (inert until `PopulateDemo` arrives), so `sim_demo` never restarts the sim.
- Without `-map` the game now opens the first map of the manifest instead of blank.bsp (owner report: it opened a blank map).
- The pawn is faster than a person walks: run speed (4.0 m/s) by default, walk speed (1.4 m/s) while crouching.
- `Helpers/Base64.h` also fixes two latent problems of the moved code (signed shift overflow, negative `char` as a table index); results are identical.
- `GameSaveSystem` got `ExtensionSaveFailed` so a failed extension save writes nothing.
- Checked: self tests (bridge suite) and `-headless -sim_bridge_test`. Not checked: a visible run, the web build.
