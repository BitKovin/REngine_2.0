# Living World Simulation: Implementation Addendum

**Companion to:** *Living World Simulation: Design Document* (Draft 3, 2026-10-23)
**Status:** Draft 1, 2026-10-23
**Scope:** how the design is implemented on this engine (BSP levels, Detour navigation, entt ECS, wasm + native). Where this addendum and the design document disagree, **this addendum wins**. Everything not mentioned here stands as written.
**Not code.** This document contains decisions, structure and open items only.

---

## Contents

1. [Summary of decisions](#1-summary-of-decisions)
2. [Scale and what it simplifies](#2-scale-and-what-it-simplifies)
3. [Architecture: the sim is a server](#3-architecture-the-sim-is-a-server)
4. [World navigation](#4-world-navigation)
5. [Levels, travel and persistence](#5-levels-travel-and-persistence)
6. [Time: ticks, tiers and fast-forward](#6-time-ticks-tiers-and-fast-forward)
7. [Movement and materialization](#7-movement-and-materialization)
8. [Activities as step lists](#8-activities-as-step-lists)
9. [Items and containers](#9-items-and-containers)
10. [Body lifecycle and cleanup sweeps](#10-body-lifecycle-and-cleanup-sweeps)
11. [Blood moon](#11-blood-moon)
12. [Trains](#12-trains)
13. [Kernel: what is kept, simplified or dropped](#13-kernel-what-is-kept-simplified-or-dropped)
14. [Revised build order](#14-revised-build-order)
15. [Risks and things to prototype first](#15-risks-and-things-to-prototype-first)
16. [Open items](#16-open-items)
17. [Building this with an AI implementer](#17-building-this-with-an-ai-implementer)
- [Appendix A: Engine facts this plan relies on](#appendix-a-engine-facts-this-plan-relies-on)
- [Appendix B: Decisions log](#appendix-b-decisions-log)

---

## 1. Summary of decisions

| # | Decision | Replaces or amends (design doc) |
|---|---|---|
| D1 | The world is about 5 BSP levels plus the city as its own level. Level changes are always loads. | §5.11, open decision #1 |
| D2 | Agents in every level walk the **real Detour mesh**, not a coarse graph. | §5.1 (two meshes), §5.3 |
| D3 | The game owns a **standalone copy of navigation** ("WorldNav"): one combined mesh made from the levels' existing baked tiles, joined by links, built by a developer console command. | §5.1, §5.7, §5.8 |
| D4 | One universal agent height and radius. Differences between mover classes are area flags and query filters, not separate meshes. | §5.1 |
| D5 | The **sim runs on its own thread**; the game thread is a client. Messages stamped with sim ticks. | §3 (stage S2 becomes the baseline), §17.3 |
| D6 | Fixed **0.1 s** base tick. Agents step in **tiers** by proximity to the player. Fast-forward increases the step. | §5.3, §7.2, §13.10, §17.2 |
| D7 | Only **container-backed** props are sim items. Everything else in a level is static. | §8, open decision #3 |
| D8 | A custom **game level transition** replaces `TravelToLevel`. No re-simulation and no `TimeSkip`. | New |
| D9 | Bodies follow a **decay lifecycle** (fresh, decaying corpse, bones). Bones are removed by the **blood moon sweep**. Evidence stays on the map. | §11.3 step 10, §10.10 |
| D10 | The **blood moon** is a calendar-driven world state read by normal rules. It is not a scripted director and not a separate sim mode. | §10.15, §14 |
| D11 | The **train** is a mobile site with a timetable. Journeys are fast travel (time speed-up) with a physical carriage level. | New |
| D12 | Scale is **not reduced**. Population targets are the full game, not a demo. | §19 |

---

## 2. Scale and what it simplifies

**Target population.** About 400 entities outside the city, counting animals. The city is its own level with its own population on top of that. The design document's ranges (500 to 2,000 agents, hundreds of thousands of animals, millions of items) were far larger than this.

**What the real scale makes unnecessary**

| Design document mechanism | Status | Why |
|---|---|---|
| Analytic path tables with binary-search positions | Replaced | Walkers are stepped along their real polyline every tick. Position is exact and trivially cheap at this count. A "position at time t" extrapolation is kept only for low-frequency tiers (section 6). |
| Swept closest-approach encounters | Deferred | At 0.1 s steps an agent moves under 1 m per tick, so plain distance checks on a spatial hash cannot tunnel through a 4 m contact radius. The swept test returns only for coarse fast-forward (section 6.4). |
| Time wheels and dormancy | Dropped for now | Iterating a few hundred agents ten times per second is trivial. The scheduler is kept for events that really happen at a moment. |
| Sparse relationship storage | Dropped | A dense matrix of 400² pairs is tiny. |
| Hot/cold field splitting and 1M events/s gate | Relaxed | Measure instead: the sim tick must stay far under the frame budget. The throughput gate becomes a regression test. |
| Worker-thread path queries as a requirement | Optional | One sliced query per sim thread is enough to start. |

**What still matters at this scale:** the tooling. A 400-agent world is where emergent bugs are hardest to reproduce, so the why-trace, event log, replay, conservation audit and soak tests stay first-class.

---

## 3. Architecture: the sim is a server

### 3.1 Split of authority

| Sim thread (server) | Game thread (client) |
|---|---|
| Needs, mood, health, injuries, death | Pose, animation |
| Inventory, ownership, containers | Micro-position, collision |
| Knowledge, relationships, memory | Hit detection details, local steering |
| Activity intent and reason | Perception geometry (PVS, line of sight) |
| Who is physical, and when | Whether a spawn or despawn is hidden from the camera |

This is rule R10 of the design document made concrete. The client never holds state the sim cannot recover.

### 3.2 Messages

Every message carries a sim tick. Inputs are applied at the start of a tick in a fixed order (tick, then sequence number), so a replay reproduces the sim exactly. Thread timing decides only *which tick* an input lands in, and that is logged.

| Client to sim | Sim to client |
|---|---|
| Player position and level; body reports (position, health); item transfers; witness reports; door and flag states; interrupt acknowledgements; save requests; debug commands | Spawn and despawn proposals; activity assignments; container content changes for the loaded level; door and flag changes; events relevant to the player; agent context cards; fast-forward state |

### 3.3 Consequences of the split

- **Spawning is a handshake.** The sim proposes "spawn agent A at point P". The client alone knows the camera's PVS, so it accepts when the spot is hidden and replies with the body. Until accepted, the agent stays abstract and keeps moving. Despawn works the same way. A latency of a tick or two is covered by the radius margin (section 7).
- **Dialogue and barks cannot query the sim synchronously.** The sim replicates a small **context card** (need bands, mood band, activity and reason, a few top facts) for each materialized agent at about 1 Hz. The client reads local data.
- **Player item actions are optimistic.** The client applies the change to the prop at once and sends the transfer. The sim confirms or rejects. A rejection (another agent took the item in the same tick) is rare and simply reverts the prop.
- **Debug tools** (inspector, why-trace) use request and response over the same channel, plus a **pause at a tick boundary** for stepping and replay.
- **Saves are a barrier.** The client requests a save and queues its outgoing commands. The sim snapshots at the next tick boundary and returns the blob with the tick number. The client then captures its own state. This costs one or two frames.
- **The sim never touches engine statics.** It uses only its own WorldNav copy and its own entt registry. Door and flag changes are mirrored to the client by message.

### 3.4 Transport

One interface, two implementations: a thread with **mutex-guarded swap buffers** (the shipping path), and a same-thread pump used by headless tests and replays. Message volume is a few hundred per second, so lock-free queues buy nothing and are a common source of subtle bugs. The threaded code stays tiny (see section 17). Threads are fully supported on all targets, so the pump is a testing tool, not a fallback.

### 3.5 Sim storage

The sim owns a **separate `entt::registry`**, not `EcsWorld`. This gives generational handles, sparse-set storage and views without writing a table kernel. The sim never references `Entity` or engine types. The bridge maps a sim handle to an engine `Entity*` only for materialized bodies.

---

## 4. World navigation

### 4.1 Goal

Offline (abstract) agents walk the same walkable surfaces as physical agents, continuously, in every level, at all times. The cost is a path lookup and a polyline step, not a physics simulation.

### 4.2 WorldNav

A standalone copy of the navigation code owned by the game, containing:

- one combined `dtNavMesh` for all levels, built by the bake (section 4.3);
- a tile-to-level table and one placement offset per level;
- a long-lived `dtNavMeshQuery` owned by the sim thread, with a node pool sized for cross-level routes (the engine's own queries use 2,048 nodes, which is too few);
- a **sliced** path service (initSliced / updateSliced) that spreads long requests over ticks within a budget;
- a path cache keyed by (start door, end door, nav version);
- one **agent profile** (height, radius, climb, slope) shared with the levels' baked meshes.

The engine's static `Navigation` singleton is not touched. Physical bodies keep using the per-level mesh and crowd.

### 4.3 Bake command

A developer console command builds the world file. It is run by a developer, not at game start.

1. **Input** is a small manifest listing the levels. Portals are read from the maps (section 4.4).
2. **Refresh nav files.** Each level's `.nav` file is created lazily when the level is first loaded, and keyed by the map's modification time. The command loads or refreshes any that are missing or stale.
3. **Layout.** Solve one placement offset per level so that linked portals end up a few tiles apart and no tile slots overlap. The offset for a level is `(world grid slot) − (that level's nav origin)`, so every tile boundary of that level lands exactly on the world grid. This works because tile size (64 cells × 0.1 m = 6.4 m) is identical across levels. A spanning tree of the level graph always has a valid layout.
4. **Merge.** Copy each level's baked tiles into the combined mesh, translating vertices, detail vertices, bounds and off-mesh positions by the offset. The BVH is stored relative to tile bounds and needs no change. Only x and z are offset, so heights are unchanged.
5. **Links.** Add one off-mesh connection per portal pair (section 4.4). Links that close a cycle in the level graph are the risky ones (section 15).
6. **Output** is one world file: the mesh, the tile-to-level table, the per-level offsets, and a version stamp for each source level.
7. **Validation (fails the command):**
   - a portal or spawn-point endpoint that does not snap to a poly (names the map and entity);
   - any pair of levels with no route between them;
   - any authored site door that cannot reach the other doors it should;
   - overlapping tile slots.

### 4.4 Portals

- Transitions are **authored in the maps** as trigger entities with a target level and a spawn point name. The bake reads each trigger and the position of the spawn point it names, and projects both onto the mesh.
- **NPCs and the player use the same transitions, in both directions.** The player crosses by loading. An NPC crosses by walking to the trigger side and taking the link, which counts as a logged position exception (rule R4) with a fixed cost.
- When the player crosses, their sim agent moves through the same portal pair, so followers, pursuers and witnesses stay consistent.

### 4.5 Runtime coordinates

- An agent's position is stored **only in nav space**. Its level is derived from the tile-to-level table. No agent stores a level id.
- Level-local position is `nav position − that level's offset`, applied at spawn and despawn.
- Only agents in the **currently loaded** level can be materialized.
- The version stamp of each source level is checked at game load. A mismatch logs a loud warning in development builds. This is the one way the world mesh and a physical mesh can drift, and it is cheap to prevent.

### 4.6 Interiors and vertical layers

Interiors live inside the same level as the street, on the mesh's layers (up to 4 per tile). Two consequences:

- The encounter grid must bucket by floor as well, or require a height difference under about 3 m. Otherwise people upstairs and downstairs "meet".
- The design document's site abstraction still applies: an agent inside a building is pinned to the site and counted by occupancy and slots. Its exact position is assigned only when a body is needed.

### 4.7 Dynamic state

- Doors, gates and collapsed bridges are **poly flag toggles**, applied to the world mesh by the sim and mirrored to the loaded level's mesh through a message.
- The tile cache (dynamic obstacles) stays physical-only and applies only to the loaded level.

### 4.8 Agent profile

One universal height and radius. Cart, horse and large-animal differences are handled by **area flags and query filters**, not by extra meshes. If a class turns out to need a different radius, revisit this decision rather than adding a second mesh.

---

## 5. Levels, travel and persistence

### 5.1 What `TravelToLevel` does today

It saves the whole level, NPCs included, into `LevelMemory`, serializes the player, loads the new level, and then runs up to 100 s of real game ticks to catch the level up (`TimeSkip`, handled in `EngineMain.cpp`). That catch-up is exactly what this design removes.

### 5.2 The game's own transition

- Save only what the level still owns (section 5.3).
- Serialize the player and set the landing spawn point name, as today.
- Load the level with **no time skip**. The sim clock never stopped, and NPC state lives in the sim.
- NPC entities are marked non-saved, using the same mechanism the engine already uses for the player (`SaveGame = false`).

### 5.3 Ownership of persistence

| Owned by the sim (saved in the sim chunk) | Owned by the level (saved with the level) |
|---|---|
| Agents, animals, groups | Static prop state: doors, destroyed or moved scenery |
| Items, containers, ground piles, bodies | Level-local physics props that are not sim items |
| Households, positions, ownership | |
| Relationships, memory, institutions | |
| Calendar, moon state, weather | |

Sim state is saved through `GameSaveSystem` as its own chunk, via the save barrier in section 3.3.

---

## 6. Time: ticks, tiers and fast-forward

### 6.1 Base tick

A fixed **0.1 s** step. The sim thread runs an accumulator against real time with a cap on ticks per catch-up. Every step is the same size. A slow period never turns into a larger step.

### 6.2 Tiers

Agents are stepped by tier, based on proximity to the player rather than a fixed setting per level:

| Tier | Who | Step |
|---|---|---|
| T0 | Agents with bodies, or close to the player | 0.1 s |
| T1 | The rest of the loaded level | 0.1 to 0.2 s |
| T2 | Other levels, including the city while the player is away | 0.5 to 1 s |

- The city may be capped at a slower tier by a per-level override if it proves heavy. When the player is in the city, it is T0 near them as in any level.
- Each agent integrates with its own step. When another agent needs to compare positions, an agent's position at the current tick is extrapolated along its path.
- Up to about 2 s per step, plain distance checks still work for encounters.

### 6.3 Work per tick

- **Every tick:** movement integration for all walkers, the spatial-hash encounter pass, then scheduler events due in this tick.
- **Staggered:** needs and mood once per second per agent (by agent index). Decisions run only on triggers, with the minimum decision interval from the design document.
- **Path requests** are sliced into the tick budget. Results are applied at one fixed point in the tick order.

### 6.4 Fast-forward (speeding up time)

Used for sleeping, waiting and train travel. It is the same loop with a larger step:

- Bodies are suspended or removed on the client, since they cannot act meaningfully at high speed.
- Steps beyond about 2 s use the **swept closest-approach test** from the design document (§7.2) to catch crossings. This is the only place that test is required.
- Fast-forward **stops on wake events**: combat near the player, a scream, a body found, fire, the start of a blood moon night, and similar. Each event type declares whether it is a wake event.
- The step size is a tunable and can be raised until the tick cost fits the target speed-up.

---

## 7. Movement and materialization

### 7.1 Movement

Walkers follow their real path polyline, advanced each step by speed × step. Speed comes from **one shared table** (per area type and per mover class) used by both the sim and the physical layer, so an agent's arrival time is the same abstractly and physically apart from avoidance delay.

**Calibration check (early).** Walk one real body along a route that a headless agent also walks, and compare arrival times. This is run before anything else depends on abstract movement.

### 7.2 Materialization by distance

- Two radii with hysteresis: spawn at radius R, despawn at R + margin. R is larger than the longest sight range, so abstract and physical agents never need to perceive each other.
- The sim ranks candidates by distance, relevance (quest, relationship) and group membership (never spawn half a family), within a body budget.
- A spawn or despawn takes place only at a point **outside the camera's PVS**, using the BSP cluster visibility the engine already has (`IsClusterVisible`, `CheckPointBspVisible` in the Quake3 BSP code). If it stays blocked too long, fall back in order to a door the agent is about to use, then fog, then the nearest hidden point on its own path (logged as an exception).
- The body receives the **remaining waypoints** from the world path. The crowd then finds a corridor along those points on the level mesh. Since the polygons come from the same baked tiles, this should always succeed. A failure is logged as a bug.
- On despawn, position (projected onto the path), health and inventory deltas are written back, and the current activity step continues abstractly. Despawn is deferred while the body is in combat or mid-interaction.
- The body count is limited by the physical layer. The engine's crowd is initialized with a maximum of 512 agents (`InitCrowd(512)` in the nav loader); the real budget is set by measurement.

---

## 8. Activities as step lists

The design document describes an activity as one row with one executor. A real activity such as "buy bread" is a chain: walk, enter, queue, exchange, exit, walk home, eat. So:

- An **activity is a short list of primitive steps**: Travel(target), Enter(site), UseSlot(kind, duration), Transfer(spec), Wait(condition), Interact(target), Ride(vehicle).
- **Executors exist per primitive**, not per activity. There are about six abstract executors and six physical ones (behaviour tree subtrees). New activities are pure data.
- The decision function chooses an activity. A step runner advances through its steps. Interrupts cancel at step boundaries.
- A body materializing in the middle of an activity resumes the **current step** physically.
- **Agent knowledge** is a bitset over the sites of a settlement plus a small price-memory table. Candidate selection is then a bitset intersection with the affordance index, which is cheap enough to run on every trigger.

---

## 9. Items and containers

### 9.1 Rule

Only **container-backed props** are sim items. A container is a shelf, pantry, cashbox, barrel, wardrobe, wagon bed, or a body. Contents are data. Everything else in a level is static scenery.

### 9.2 Consequences

- Every container prop needs a **stable authored id** in the map. The sim holds the container record and its items.
- When a level loads, container props populate their visible contents from sim state. Taking an item sends a transfer command (optimistically applied on the client, section 3.3).
- Items that leave a container (dropped, looted from a body) go into a **ground-pile container** record with a position and level. When that level is loaded and the player is near, it appears as a pickup prop. This is what keeps conservation intact when something is dropped.
- Worn and held items are containers (worn, hand) that belong to the agent.
- Bulk goods (grain, coin, oil) stay as **lots**. Everything else stays as individual item records. At this scale per-item records are cheap, including on web.

---

## 10. Body lifecycle and cleanup sweeps

### 10.1 Stages

A body is a container record (design document §11.3) with a **stage** field. Stage changes come from expiry events scheduled at death, with timing affected by weather and location.

| Stage | Physical layer | Sim |
|---|---|---|
| Fresh | Ragdoll or lying body, full model and physics | Container holding belongings; odour event for scavengers |
| Decaying corpse | Frozen static mesh, no physics or animation; flies and smell as effects | Same container; longer odour tail |
| Bones | Small static prop | A tiny record with no contents |

### 10.2 Rules

- **Conservation holds.** Belongings are moved out through logged transfers before the bones stage, to a ground pile, a burial, or the parish or estate. Nothing is deleted. Any removal is a named sink.
- Scavengers and werewolves consume carcasses naturally, which is the design document's carcass mechanism. Unburied bodies thin out on their own, especially on the blood moon night.
- Burial (upkeep rule #9) turns a body into a grave record with no physical body.

### 10.3 Blood moon sweep

At the end of each blood moon, a batch pass handles everything already overdue. Each item is a logged transition, not a wipe:

- **Bones older than N days are removed.** N is a tunable. The starting value is about one moon cycle, so bones last at least until the following blood moon.
- Bodies overdue for their next stage advance a stage; unclaimed remains get pauper burial.
- Expired ground piles and other expired, sim-generated world objects are removed.
- Physical props are removed only outside the camera's PVS, or when the level is not loaded.

### 10.4 Evidence

- Evidence is **kept on the map**: anything tied to an open complaint, and anything the player has seen, is skipped by the sweep.
- Natural decay still applies. Tracks wash out in rain, blood stains fade.
- A case that stays open past a set age goes cold, and its evidence becomes sweepable. Otherwise one forgotten complaint could hold a body on the map forever.

---

## 11. Blood moon

### 11.1 Principle

The blood moon is a **calendar-driven world state** that existing rules read. It is not a scripted director and not a separate simulation mode. A separate mode would break the principle of one set of rules at every distance and double the testing.

### 11.2 Mechanism

| Piece | How |
|---|---|
| Trigger | A calendar rule emits `BloodMoonRising` and `BloodMoonEnded` at times computed from the moon cycle |
| People hide | A high-weight obligation "be home before dusk" activates. The deadline logic (design document §5.5) already computes departure from travel time, so people far from home leave early. Anyone caught outside heads for the nearest site that offers shelter: a friend's house, the church, a pub |
| Homes lock | Ordinary access rules lock houses at night. The same rule applies to the player |
| Werewolves | Species data reads the moon state: wider territory, satiation ignored, activity pattern extended, higher aggression |
| Mood | A dread thought family, and a notice board and church warning in the days before, so the event is legible (visibility pillar) |
| Dawn | `BloodMoonEnded` triggers the cleanup sweep (section 10.3) |

### 11.3 Simulation cost

When most agents are indoors, almost nobody is walking and encounters nearly disappear. The night is therefore cheap to fast-forward through, without any special simulation code. The start of the night is a wake event for skipping.

### 11.4 Rules for resets

A sweep may remove things the simulation itself generated (bones, expired piles, stale world objects) and clear bookkeeping (stuck or blocked states). It may not repopulate the dead, restore hunted species, or refund money. Those would break the permanence rules (R14 and §12.9 of the design document).

---

## 12. Trains

### 12.1 Model

- A **train** is a vehicle record with a timetable. Each **carriage** is a **site**: agents enter it the way they enter a building, and are pinned to it for the ride. Their world position is the train's position.
- The train moves along a **rail route** as timed legs. The route is its own small graph, not walkable mesh.
- Trains run according to the schedule whether or not the player is nearby.

### 12.2 Physical side

- The carriage interior is a small separate level, loaded when the player boards. It has its own small nav mesh and is **not part of the world mesh**.
- NPC passengers in the player's carriage materialize into it by the usual distance and budget rules, with seats as slots.
- A journey is **fast travel**: the sim fast-forwards while the carriage level is shown. On arrival the destination station level loads and the player is placed on the platform. Wake events (violence in the carriage, the start of a blood moon night) end the skip.

### 12.3 NPC use

- An NPC plans a trip as a list of legs: walk to the station, wait for departure, ride, walk from the destination station. The decision function compares walking time with the next departure plus ride time. A missed train means waiting for the next, which falls out of the deadline logic.
- Detour's A* handles only fixed costs. Station-to-station walking legs are precomputed from the world mesh. The timetable layer sits above, so a long trip is a short search over stations.
- Tickets are transfers to the railway institution (the design document's outside-world valve). Seats are capacity.

---

## 13. Kernel: what is kept, simplified or dropped

| Area | Decision |
|---|---|
| Time | Integer game-time. Lazy values stay as (value, timestamp, rate); the next crossing is rounded to an integer tick |
| Scheduler | Binary heap of (time, sequence, handle, version) with **lazy cancellation**: a popped event with a stale version is dropped. No time wheels until profiling asks |
| Needs | One pending threshold crossing per lazy value (the next band boundary) |
| Commands | Tagged structs in a per-wave buffer. The kernel keeps a "current cause" context, so cause stamping is automatic |
| Formulas | Start with named C++ scoring functions with data-driven parameters and curves. Add an expression VM only when designers need it |
| Tooling | **Event log and why-trace are part of the kernel from the start**, not a later milestone. The ImGui inspector follows immediately. Every tool also has a **text output** (why-trace, entity timeline, state dump, soak report as markdown, CSV and JSON), so headless runs can be read without a screen |
| Determinism | The sim core is deterministic for a given input log. The physical layer is not, and its reports are part of the log |
| Replay | Checkpoints plus recorded bridge inputs. Not byte-for-byte physics replay |
| Calibration of abstract versus physical combat | From logged real fights and tuned formulas, not from running physical combat headless thousands of times |
| Pre-roll | A shipped snapshot. Never computed on the client |

---

## 14. Revised build order

Estimates are intentionally omitted. Re-plan after M2 and M3 with measured numbers.

| ID | Scope | Done when |
|---|---|---|
| M0 | WorldNav and the bake command: tile merge, layout, portals, validation, world file, debug draw. The merge, layout and validation live in a **pure library tested against real `.nav` fixtures**; the console command is a thin engine shell around it | A path from a village door to a city door crosses levels, and the bake reports every validation failure by name |
| M1 | Sim server skeleton: sim thread, transport (thread and pump), input log, pause at tick boundary, snapshot, entt registry, event log with cause stamping | A killed agent stays dead across save and load; a recorded input log replays to identical state hashes |
| M2 | Movement and materialization: fixed ticks, tiers, shared speed table, spawn handshake with PVS hiding, custom level transition, abstract versus physical calibration, teleport detector | About 40 agents commute across levels with zero impossible speeds; bodies appear and disappear unseen; calibration within tolerance |
| M3 | Kernel and tooling: scheduler, lazy values, command buffer, why-trace, inspector, conservation audit, context cards, save barrier | A new rule and a data row can be added in an afternoon and traced end to end |
| M4 | Economy slice: needs, step-list activities, households, shops, items and containers, wages, the food chain | Remove the baker and prices, queues and hunger respond |
| M5 | Encounters, memory, expectations, upkeep (fill-position first) | Rumours spread; an off-screen absence is noticed by the right people; killing the publican is repaired after the grace period |
| M6 | Ecology (design document A1 to A3) | The ecology-only soak is stable |
| M7 | Body lifecycle, blood moon, trains | A blood moon night passes with people sheltering, the sweep runs at dawn, a timetable trip works end to end |
| M8 | Crime and law, institutions, news boards, barks from state | A theft goes through witness, complaint, investigation and consequence |
| M9 | Population generation, authored NPCs, scenarios, pre-roll | Every scenario starts in a living world that passes the soak suite |

**Review points** after M2 and M3: measured tick cost per tier, memory per record, path cache hit rate, and whether the village is fun to watch from first person.

---

## 15. Risks and things to prototype first

| Risk | Why it matters | Mitigation |
|---|---|---|
| **Long off-mesh links** | In stock Detour the landing end of a connection is found only in the same or an adjacent tile (to be verified). A link between far-apart tiles could silently fail to connect | Prototype first. Since the game owns the Detour copy, the fix is to look up the landing tile directly by position. Keep cycle-closing links to a minimum |
| **A\* heuristic across links** | The straight-line heuristic misleads when linked levels are laid out far apart, giving slow searches or slightly worse routes | Tree layout with portals close together; tune the heuristic scale if needed |
| **Node pool size** | Cross-level routes need a far larger pool than the engine's 2,048 | Size the sim's query object for the longest route; cache aggressively |
| **Abstract versus physical drift** | Speed or path differences produce visible pops and wrong arrival times | Shared speed table, early calibration test, nav version stamps |
| **Async surprises** | Every gameplay query into the sim is now asynchronous | Context cards, optimistic client prop changes, request/response for tools; design for it from M1 |
| **Stale nav files** | A level's `.nav` changes after the world file is baked | Version stamps checked at load, loud warning in dev builds |
| **Vertical overlap** | Stacked floors treated as neighbours by the encounter grid | Bucket by floor or require a height difference under about 3 m |
| **Cleanup removing wanted evidence** | A sweep could delete something an investigation needs | Evidence exemption with a cold-case age cap (section 10.4) |
| **Tuning swamp** | Unchanged from the design document | One system at a time, soak tests, tunable profiles |

---

## 16. Open items

| # | Item | Needed by |
|---|---|---|
| O1 | Value of N (bones removal) and the cold-case age cap | M7 |
| O2 | City population and whether it needs a slower-tier cap | M2 measurement |
| O3 | Which events are wake events for fast-forward | M2 |
| O4 | Exact set of NPC-only crossings, if any, beyond map-authored transitions (for example roads or rail) | M0 manifest |
| O5 | Whether to add a second mesh or filter profile for any large-animal class | After M2 |
| O6 | Designer scripting (formula language, rules) versus data only | Before M3 |
| O7 | Player survival mode (off, light, full) | Before M4 |
| O8 | Disease in v1 | Before M4 |
| O9 | Carriage level count and layout (one interior reused for all trains, or several) | M7 |
| O10 | Which sim-generated world objects the sweep covers beyond bones, piles, stains and tracks | M7 |
| O11 | May the implementer build the engine and run the headless runner, tests and soaks directly, or are builds and in-game checks done only by the owner? (section 17.3) | Before M0 |
| O12 | How deep is the owner's review: interfaces and schemas only, or also implementations? (section 17.6) | Before M0 |

---

## 17. Building this with an AI implementer

**Premise.** An AI model writes about 99% of the code. The owner decides design, judges feel, approves contracts and playtests. This changes what the real constraints are, so this section overrides the milestone ordering and process of the design document wherever they conflict.

### 17.1 What the real bottleneck is

Typing code is no longer the cost. The costs are, in order:

1. **Verification.** Is the behaviour actually right? Every system must come with a way to check it that does not depend on anyone reading the code.
2. **Feedback loop time.** Engine builds, in-game checks by the owner, and long soak runs. The cheaper and faster the loop, the more correct the result.
3. **Context.** No session can hold a 50,000-line codebase. Anything that is not written down or enforced by code will be forgotten or contradicted.
4. **Owner decisions.** Design questions, contracts and taste are the owner's. Waiting on them is the main source of idle time, so they should be batched.

The week estimates in the design document assumed a human typing. They are not meaningful here and should be replaced by measured progress (section 14 re-plan points).

### 17.2 Weaknesses to design around

| Weakness | Design response |
|---|---|
| Cannot judge feel, fun, or visual pops | Owner checkpoints at fixed points (section 17.4). Every system gets a text dump the implementer can read, plus overlays for the owner |
| Cannot hold global invariants in mind | Enforce them in code and run them after every change: conservation audit, teleport detector, per-tick state hash, handle validation, lints. A rule that only lives in this document will be broken |
| Concurrency bugs | One sim thread. Mutex-guarded swap buffers. No shared mutable state. The sim never touches engine statics. The threaded code is a few hundred lines and is reviewed by the owner. All other testing uses the same-thread pump |
| Nondeterminism slips (unordered iteration, uninitialized fields, float order) | No unordered containers iterated in the sim (wrapper types or a lint). A run-twice hash comparison runs inside every test run |
| Wrong guesses about third-party APIs (Detour, entt) | Read the vendored source before using a call, as is already the practice. Wrap third-party code in thin sim-owned headers. The compiler is the first check |
| Over-engineering | Defer the expression VM, time wheels, lock-free queues, custom allocators and SoA micro-optimization until a measurement asks. No abstraction before its second use. Each module must pass the visibility test (it has a number it reads, a number it writes, and a place the player sees the effect) |
| Binary format surgery (translating baked Detour tiles) | Round-trip tests on real `.nav` fixtures: path lengths and nearest-poly results on the merged mesh must equal the original level's. Failures name the tile |
| Balance of emergent systems | Soak reports as machine-readable files. The implementer adjusts parameters from the numbers. The owner judges whether the result feels right |
| Long chains of interacting rules (upkeep, gossip, ecology cycles) | Each rule ships with a scenario test written as a story, property tests where possible, and soak metrics. No rule is merged without all three |

### 17.3 Closed loop first

The single largest lever is letting the implementer run things.

- The sim library has **no engine dependency** and compiles in seconds. It has a headless runner, a test executable, a soak runner and a replay tool, all command-line, all with text output.
- Anything that can be checked headlessly is checked headlessly before the owner sees it: determinism, save round trip, conservation, movement speed limits, scenario stories, soak thresholds.
- The engine is needed only for the bridge: spawn, the PVS rule, executors, the level transition, the bake command's engine shell, and the abstract-versus-physical calibration. Those are the parts that need the owner to run the game, or an agreed way for the implementer to build and run it (open item O11).
- Logs from the physical layer are part of the replay input, so a bug seen in-game can be reproduced headlessly afterwards.

### 17.4 Process for each slice

1. **Contract.** A short written contract for the slice: public interface, messages, data schema, invariants, tunables, what "done" means. The owner approves it. This is the only mandatory review.
2. **Tests first.** Scenario tests written as stories ("the baker dies, within N ticks someone else bakes or prices rise"), property tests, and soak metrics.
3. **Implement** against the contract, in a small number of files.
4. **Run** the full headless suite: tests, determinism, save round trip, audits.
5. **Soak and tune** from the reports.
6. **Hand over** with a short note: what changed, how to see it in game, what was not verified.

### 17.5 Code structure for an implementer

| Library or tool | Contents | Rule |
|---|---|---|
| `SimCore` | Registry wrapper, handles, scheduler, events and cause stamping, commands, time, snapshots, transport | No engine includes. Small public headers. Never changes for gameplay features (that would be a missing extension point) |
| `SimNav` | World mesh, path service, bake library | No engine includes. Tested against `.nav` fixtures |
| `SimModules` | Needs, activities, economy, ecology, law, upkeep, calendar, one directory per module | A module sees only the public API of the others. One module per work session where possible |
| `SimData` | Data files for goods, species, activities, thoughts, positions | Validated against schemas on load, with did-you-mean errors |
| `GameBridge` | The only code that includes both the sim and the engine | Thin. Logic belongs in `SimModules` |
| Tools | `sim_run`, `sim_soak`, `sim_replay`, `sim_why`, `sim_dump` | Text and file output only |

### 17.6 Division of labour

| Owner | Implementer |
|---|---|
| Design decisions and open items, approving slice contracts | Everything else in code: libraries, bridge, tools, data rows, tests |
| Judging feel, pacing, tone, visual quality | Soak runs, tuning from numbers, performance work with measured reports |
| Content taste (names, archetypes, descriptions) and curation | First drafts of data and content for the owner to edit |
| Playtesting and running the engine where the implementer cannot | Documentation: contract files, generated reference, decision log |
| Reviewing interfaces, schemas and tunables | Self-review of implementations with the headless suite |

Review is aimed at the places where a human's judgment matters and at the small threaded core. Reviewing every implementation line is neither required nor realistic.

### 17.7 Keeping context across sessions

- A single `SIM_CONTRACT.md` in the sim directory: module map, public interfaces, invariants, tunables, what is tested where. Every session starts by reading it, and every slice updates it.
- **Generated reference docs** from registrations (events, rules, schemas, tunables), as in the design document, so they cannot go stale.
- This addendum and the design document stay the source of truth for decisions. New decisions are appended to the decisions log.
- Tests act as an executable specification. A failing scenario test is the first thing a new session reads.

### 17.8 Parallel work

The early phases are serial: M0 and M1 are independent of each other and can run in parallel, but the sim core, protocol and handle types must be frozen before modules begin. After that, modules with disjoint files can be developed in separate sessions, each against the frozen contract. The contract file is the coordination point. Two sessions never edit the same files.

### 17.9 Order changes this implies

- **Headless runner, state hashing and the event log come before any gameplay** (part of M1).
- **Scenario test harness and the soak runner come before the first module** (M3 starts with them, not ends with them).
- **Engine-dependent work is batched** so the owner's play sessions are used for the things only they can judge.
- **Text output is a requirement of every tool**, not an extra.

---

## Appendix A: Engine facts this plan relies on

Facts read from the engine source. Verify again if the engine changes.

- **Levels** load through `Level::LoadLevelFromFile`; `LevelTraversalSystem::TravelToLevel` saves the level, serializes the player, sets `TimeSkip`, then loads. `EngineMain` runs `SimulateGameTicksForTime(TimeSkip)` after the load.
- **Navigation** is a static singleton (`NavigationSystem`) holding one `navMesh`, one `tileCache` and one crowd. Each query allocates a `dtNavMeshQuery` with `init(navMesh, 2048)`.
- **Nav bake** (`GenerateNavData`): the level's nav-obstacle mesh gives the bounds; the grid origin is that minimum, so it is arbitrary per level. Cell size 0.1 m, cell height 0.2 m, tile size 64 cells (6.4 m), agent height 2.0 m, climb 0.6 m, radius 0.5 m, slope 40°, up to 4 layers per tile. The result is cached as `<level>.nav`, keyed by the map's modification time.
- **Detour** is built with `DT_POLYREF64` (defined in the engine's `DetourNavMesh.h`), so the tile and poly bit budget is not a concern for a combined mesh.
- **Crowd** is initialized with a capacity of 512 agents on level load.
- **ECS** is a static entt registry (`EcsWorld`) for rendered entities. The sim uses its own registry instead.
- **BSP** has cluster visibility (`IsClusterVisible`, `CheckPointBspVisible`), used for PVS-hidden spawning.
- **Threading:** wasm pthreads are enabled in the build (`ENABLE_PTHREADS` ON), and the engine has a `ThreadPool`. The user confirms the web host supports multithreading.

---

## Appendix B: Decisions log

| Question | Answer | Where used |
|---|---|---|
| Are NPCs spawned by distance within a level? | Yes. Levels are about 5 BSPs; NPCs pop by distance | §7.2 |
| Should offline simulation use the nav mesh? | Yes, as close to the physical world as possible | §4 |
| Own copy of navigation, combined across levels? | Yes, one mesh with links, built by a developer console command | §4.2, §4.3 |
| Do levels share a coordinate frame? | No, each has its own origin | §4.3 (layout offsets) |
| Do levels touch? | Maybe; level changes are always loads | §4.4, §5 |
| Same cell size, radius and tile size in all nav files? | Yes | §4.3 |
| Mover classes? | One universal height and radius; open spaces, interiors in the same level | §4.8 |
| How are transitions defined? | Authored in the maps | §4.4 |
| Do NPCs use transitions? | Yes, both directions | §4.4 |
| Replace `TravelToLevel`? | Yes, a custom transition with no re-simulation | §5 |
| Items in the world? | Only in containers (shelves, pantries, cashboxes, bodies) | §9 |
| Time step? | Fixed, about 0.1 s; larger steps allowed | §6 |
| Scale? | About 400 outside the city, counting animals; city separate | §2 |
| Platform? | Web is a target and supports threads | §3.4 |
| Scale down for a demo? | No. The demo is a later marketing version | D12 |
| Sim thread? | Yes, client–server, game thread as client | §3 |
| City tick? | May be slower | §6.2 |
| Skipping time? | Speed-up like KCD, also for trains | §6.4, §12 |
| Blood moon? | Once per full moon; werewolves aggressive, people hide; allows routine shortcuts and cleanup | §11 |
| What resets at the end? | Remove decaying bodies (decay to bones first) and other sim-generated stuff | §10 |
| Bones? | Removed after N days during the blood moon | §10.3 |
| Evidence? | Kept on the map | §10.4 |
| Trains? | Physical and scheduled, effectively fast travel | §12 |
