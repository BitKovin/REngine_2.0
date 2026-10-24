# Slice contract: M2e the level transition (walking from one level into the next)

Status: **implemented** (2026-10-24) with the defaults of section 7 (owner: approved). Self tests pass (11,202 checks), `-sim_transition_test` passes (6 transitions,
save in b loaded from a). Two corrections to the first draft, found in the engine code while implementing: (1) `LevelMemory` **is** part of the game save
(`GameSaveData::LevelMemory`), so levels you left keep their state across save and load, and 7.2 is moot; (2) the engine resets `Time::GameTime` to 0 on every
level open and `LoadLevelFromData` restores the saved clock of the level, so the memory is restored as it was left (no "GameTime = now"), and
`Traveling` is cleared by the engine itself right after the restore (`Level::OpenLevel`), not by the game.

Why now: the world is five levels but the player can only stand in one of them. Everything after this (a village map, a clock, a bakery
with a queue) needs the player to be able to walk where the NPCs already walk. The sim side is done: the world nav links every portal to
its landing, the courier already crosses level a and the road both ways, bodies are cleared and re-proposed on a level change. What is
missing is the player's half.

## 1. What the player sees

Walking into a `sim_portal` volume with the pawn loads the level the portal names and puts the pawn at the named `sim_spawn`, facing the
way the spawn faces (momentum is dropped unless the owner wants it kept, see 7.3). The engine's loading screen
covers the load. Walking back through the portal there returns to where the player left, in the other level. NPCs the player saw
walk to that portal appear on the other side as they would for any agent (the sim already moves them across; bodies fade in at the door).
Nothing is re-simulated and nothing jumps: the sim clock ran during the load.

Test world after this slice (portals and spawns already in the maps):

The six portals of the test chain a - road - b - c (from `SaveData/sim_nav_report.txt`; the spawn is named after where you came from):

| portal (level:name) | lands at (level, spawn, level-local position) |
|---|---|
| a:to_road | road, `from_a`, (-24.5, 0.1, 0) |
| road:to_a | a, `from_road`, (24.5, 0.6, 0) |
| road:to_b | b, `from_road`, (0.25, 0.6, -26.75) |
| b:to_road | road, `from_b`, (24.5, 0.1, 0) |
| b:to_c | c, `from_b`, (-5.5, 0.6, 0) |
| c:to_b | b, `from_c`, (0, 0.1, -8) |

## 2. Mechanism

**No change to the engine.** `LevelTraversalSystem` is public statics; the game fills them itself instead of calling `TravelToLevel`
(which also sets `TimeSkip`, the 100 s catch-up this design removes).

1. `SimPortal::OnBodyEntered` (override; same shape as StealthAction's `TriggerTravel`): act only on an entity tagged `player`, only when the
   pawn has existed for 2 s (`Time::GameTime - SpawnTime`; stops a landing next to the return portal from bouncing back). It does **not** load
   anything inside the physics callback: it calls `SimTransition::Request(targetLevel, targetSpawn)`.
2. `SimTransition` (new, `Sim/Bridge/SimTransition.{h,cpp}`; pure decision part separate from the engine part, like the other bridge files):
   - `Request` stores one pending transition (a second request while one is pending is ignored; the first wins).
   - `SimBridge::Update` runs it at a safe point: resolve `targetLevel` to a world level with `SimWorldNavFile::FindLevel` (the mapping the bake uses),
     check the level and the spawn exist in the loaded `world.nav` (`WorldSpawn`), otherwise log `sim transition refused: ...` naming the portal's
     level and keys, and stay where we are.
   - Fill `LevelTraversalSystem::PlayerSerializedData` (the pawn's `Serialize`), `DesiredSpawnPointName` = spawn, `Traveling` = true;
     `TimeSkip` stays 0; `Level::LoadLevelFromFile(map)`.
3. `SimSpawn::FromData` (change): while a transition is under way (`SimTransition::Arriving()`), only the spawn whose name matches creates the
   pawn (`SimPawn::SpawnAt(position, yaw from the spawn's angle, the pitch the pawn had)`; the pitch is carried by `SimTransition`, not through
   `PlayerSerializedData`, which stays unused).
   Otherwise the default player start (flag bit 0) spawns the pawn as today, and only when not travelling. A traveling load where the named
   spawn does not exist falls back to the default start and logs an error naming both.
4. Sim: nothing new in the kernel. The pawn's first report after the load (`PlayerMoved` on level change, already there) puts the
   player's anchor in the new level. "The pawn goes through the same portal pair in the sim" holds by construction: the spawn's nav position
   *is* the link's landing (`WorldSpawn`). A test checks it (section 5).
5. Level-owned state (addendum 5.3): on leaving, `LevelSaveSystem::SaveLevelToData()` goes into `LevelTraversalSystem::LevelMemory` (the pawn
   is `SaveGame = false` for the call, as the engine does; `SimNpc` already is) and, on entering a level that has an entry, `pendingSave` is set
   to it as it was left; `TimeSkip` is 0. `LevelMemory` is saved with the game.

## 3. Map side (what the level editor must provide)

`sim_portal` brush: `targetname`, `targetLevel` (the map file as listed in `world.manifest`, e.g. `sim_test_road.bsp`), `targetSpawn` (a
`sim_spawn` **in that level**; the convention is `from_<where you came from>`). `sim_spawn` point:
`targetname`, `angle`, `spawnflags`. Both exist in `GameData/entities.ent` and in the four test maps. The portal volume should be a doorway
that the pawn cannot go around (a closed door frame or a corridor cross-section), 1.5 m deep so a run does not skip over it; the landing is 3
to 5 m from the return portal. The map editor at `F:\Git\mapeditor` (Core WebSocket + Shell, `.map.json` -> `bsp_export_tool`) is the
tool for any new map; the real village map is a separate slice and uses it.

## 4. Failure handling

| Case | Behaviour |
|---|---|
| target level not in `world.nav` | refused, logged with the portal's keys, pawn stays |
| spawn not in the target level | refused, same |
| map file missing | the engine's own load error; the transition is logged first so the cause is visible |
| two portals in one frame | the first wins |
| load while the sim is paused (pause menu) | cannot happen: the pawn does not move while paused |
| `world.nav` stale | existing banner; the transition still works (levels are loaded by name) |
| save made while standing in level b, loaded later | the existing save carries the pawn and the sim; unchanged |

## 5. Tests

Pure: `SimTransition` decision (known / unknown level, unknown spawn, duplicate request, default start when not travelling) in the sim
self tests. Against the shipped `world.nav`: every `sim_portal` link in `world.nav` has a spawn of the same name in the target level and
that spawn is the landing of the link (distance < 0.05 m).

In game (`SimGame.exe -headless -sim_transition_test`, cwd = game folder, same pattern as `-sim_bridge_test`): start in a; teleport the
pawn into `a:to_road`; expect within 3 s: current level is the road, the pawn within 0.5 m of `from_a`, the sim's snapshot anchor is
in the road level at the same place, no transition ran twice; stay 4 s (no bounce); teleport into `road:to_b`; expect level b;
walk back along the chain to a; check the courier is still in the sim with a position the movement invariant accepts; save and load while
in b and check the pawn is in b. Writes `SaveData/sim_transition.log`; 0 on success.

Looked at (screenshots with `-screenshot`) before calling it done: the pawn right after landing in each of the four levels, to see the facing
and that the spawn is not inside geometry.

## 6. Not in this slice

A fade instead of the engine loading screen; keeping the player's last view direction relative to the portal; vehicles and the train (M7);
the pawn crossing a portal on foot being visible to NPC bodies (they have no reactions yet); real maps.

## 7. Decisions (owner, 2026-10-24: the defaults were approved)

1. **Loading screen**: the engine's, as it is (the sim runs on during the load).
2. **Level state of levels you left**: kept in `LevelMemory` and saved with the game (see the status note: this is what the engine already does).
3. **Speed and view across the portal**: yaw from the spawn's `angle`, pitch kept, momentum dropped.

## 8. As built

`Sim/Bridge/SimTransition.{h,cpp}` (pure `Resolve`, engine part `Request` / `Update` / `Arriving` / `TakeArrival`), `SimPortal::OnBodyEntered`
(player only, pawn older than 2 s, sim running), `SimSpawn::FromData`, `SimBridge::Update` calls `SimTransition::Update`, `SimPawn::SpawnAt(feet, yaw, pitch)`
and `Pitch()`. A load that does not bring the pawn (the map lost the spawn) places it from the world file's spawn (or the level's default start) and logs
an error. Tests: `Sim/Tests/SimTransitionTests.cpp` (the decision; every shipped link lands on a spawn within 5 cm), `Sim/Bridge/SimTransitionTest.{h,cpp}`
(`-sim_transition_test`, report `SaveData/sim_transition.log`: six transitions, each checked for level, landing (< 0.5 m), one transition only (no bounce), the
sim's player anchor within 1.5 m; a save in b loaded from a; all ten NPCs still in the sim). Looked at: one headless picture after a landing (floor and
walls, nothing clipped). Not looked at: the landing in each level, the feel of the loading screen.
