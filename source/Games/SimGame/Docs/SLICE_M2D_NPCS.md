# Slice contract: M2d authored NPCs (the first residents)

Status: **implemented** (2026-10-23), after the owner pointed out that the M2c bodies existed only behind a flag and nothing in the
game used them. Decisions of the owner: NPCs are content in `GameData/sim/npcs.json`, always on (no launch flag).

## 1. What the player sees

Starting the game with no arguments opens `sim_test_a.bsp` and the sim starts with ten named NPCs. Five of them are within sight of the
pawn's start (`start` at -5.2, 0.2, 8.0 in level a):

| id | name | model | where / what |
|---|---|---|---|
| `warden` | Warden Harlan | guard | patrols a 20 x 18 m rectangle round the start, stops 3 to 6 s at each corner |
| `baker` | Edith Marsh | base | between (-12, 12) and (-2, 16), stays 6 to 15 s |
| `widow` | Mrs. Aldous | base, slow | between (-9, 4) and (-1, 4), stays 10 to 20 s |
| `newsboy` | Tom Pell | base, runs | between (12, 20) and (-20, 20) |
| `sergeant` | Sergeant Pike | guard, slow | paces by the road gate at (22, +-4) |
| `courier` | Nell Cutter | base | level a (20, -6) -> road level -> back, over the portal both ways |
| `tinker`, `drover` | Old Barnaby, Hob Tanner | base, guard | on the road level |
| `miller` | Gideon Wray | base | a four-corner loop in level b |
| `hermit` | Brother Alaric | base, slow | long stops in level c |

Within 120 m of the pawn (the whole of a test level) they have bodies (SLICE_M2C); an agent that has stepped onto a portal leg has none. Every named NPC has a
**beam** over its head so it can be found, coloured by its body: grey none, yellow asked for, green has one, orange being taken back. Console: `sim_npcs` lists them (level,
position, what they do, body or not); `sim_goto <id>` puts the pawn next to one in the loaded level; `sim_look <id>` (or launch `-sim_look <id>`) puts the pawn 6 m from one
and keeps the view on it, which with `-screenshot` is how to look at an NPC without playing; `sim_agents off` hides boxes and beams. `-sim_demo [count]` still adds the
anonymous crowd (stress test); the crowd rules leave named NPCs alone.

## 2. Authoring (`GameData/sim/npcs.json`, `Sim/Bridge/SimNpcFile.*`)

One entry per NPC: `id` (up to 23 characters), `name`, `model` ("base" or "guard"), optional `speed` (0.2 to 3, multiplies the gait's table speed),
`gait` ("walk"/"run"), `routine` ("loop"/"pingpong"), `level` (default for its stops), and `stops`: each `{ "at": [x, y, z] }` in level-local metres or
`{ "spawn": "name" }`, optionally its own `level`, and `wait` (seconds, or `[min, max]`). The header of `SimNpcFile.h` is the format reference.
Finding coordinates: `SimGame.exe -headless -sim_nav_report` (cwd = game folder) writes `SaveData/sim_nav_report.txt`: per level the walkable extent, spawn points,
portals, and walkable spots around each spawn with the walking distance to them.

At start every NPC is validated against the baked world: level exists, each stop is on the mesh (1.5 m sideways, 3 m vertically), every stop can be
walked to from stop 0, wait ranges, model, speed, unique ids. Problems are logged as `==== SIM NPC CONTENT: n problems ...` naming the NPC and the stop;
the NPCs that are fine are still placed. The self tests check the shipped file against the shipped `world.nav`, so a map or file edit that breaks the
other fails the suite.

## 3. Sim side (`Sim/Npcs/SimNpcs.*`)

`Identity {id, name}` (movement: also in the snapshot), `Routine {mode, gait, stops (level name + level-local position + wait range), current, direction}`,
`Appearance` (bodies) set from the definition. Stops are stored by **level name and level-local position**, so a rebake does not move them. Behaviour is
three rules and a timer: `Arrived` at a stop -> schedule `RoutineStep` after a random wait from that stop's range -> `RoutineAdvance` picks the next stop
(loop: round; pingpong: there and back; a stop that is no longer on the mesh is skipped) and starts a walk (`MovementModule::StartWalk`). `PathFailed`
and `Replaced` retry after `npcs.retry_delay` (3 s). NPCs start after a random delay up to `npcs.start_delay_max` (5 s) so they do not move in step.
A stop in another level is just a longer walk over the portals. Saves, replay and rebake migration work as for any walker.

## 4. Tests

Validation messages; loop order and dwell times (4 to 6 s stays measured from outside); ping-pong order 1, 2, 1, 0; a route over three levels with six
portal crossings; a one-stop NPC stays; save and load carry on identically for 1,500 ticks; a rebake keeps the NPC and its routine; replay of four NPCs for
five minutes; the snapshot carries ids; the JSON parser (every field, ten kinds of mistake, one bad NPC does not stop the others); the shipped content.
In game (`-sim_bridge_test`): all ten NPCs are in the sim and five near the pawn have bodies (the log lists each NPC's level, position, activity, body).

## 5. Looked at, and not

Looked at with `-sim_look` and `-screenshot` (2026-10-23): bodies stand at the sim's position, walk and idle with the model's clips, wear their tint (`base`, a mannequin: the
owner chose to keep it) or their own textures (`guard`), and the courier's trip over the portal reads well: it vanishes at the door (fade out), and comes back at the door
(fade in) 35 s later. That look found three defects, all fixed: the T-pose (the animator returned an empty pose), black citizens (`base.glb` has no textures), and bodies
appearing at old positions (a proposal was answered with a snapshot older than the proposal, so an agent that had just taken a portal got a body at another level's
coordinates). Spawn and despawn rules now follow the owner: bodies fade in and out, in sight too (SLICE_M2C section 4).

Not looked at: the residents of the other levels from the player's point of view, the stops and waits of each route in play, fades in motion at full frame rate, and the
`guard.glb` external texture names the engine logs as missing. Not done: NPC reactions to the player, schedules by time of day (the calendar exists only as a decision),
dialogue, and `sim_npc` map entities for placing NPCs in the editor.
