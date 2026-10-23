# Living World Simulation: Design Document

**Project:** Victorian-gothic open-world first-person action RPG
**Status:** Draft 3, 2026-10-23 (section 12 upkeep rules are weak by design, so the player's changes persist)
**Scope:** the simulation that makes the world alive: people, animals, economy, movement, consequences, tooling, and the plan to build it.
**Out of scope:** combat feel, rendering, audio, UI, quest writing.

All numbers marked *(estimate)* are planning guesses to be replaced by measurements as soon as the kernel exists. Time estimates assume one programmer who knows the host engine.

---

## Contents

1. [Goals and pillars](#1-goals-and-pillars)
2. [Core rules](#2-core-rules)
3. [Architecture](#3-architecture)
4. [Kernel](#4-kernel)
5. [Space and movement](#5-space-and-movement)
6. [People](#6-people)
7. [Encounters, perception and knowledge](#7-encounters-perception-and-knowledge)
8. [Items, containers and production](#8-items-containers-and-production)
9. [Economy and labour](#9-economy-and-labour)
10. [Ecology](#10-ecology)
11. [Consequences, law and institutions](#11-consequences-law-and-institutions)
12. [Upkeep: how the world repairs itself](#12-upkeep-how-the-world-repairs-itself)
13. [Fidelity levels and the engine bridge](#13-fidelity-levels-and-the-engine-bridge)
14. [How the gothic theme emerges](#14-how-the-gothic-theme-emerges)
15. [World generation and start scenarios](#15-world-generation-and-start-scenarios)
16. [Programmer experience](#16-programmer-experience)
17. [Performance and memory](#17-performance-and-memory)
18. [Stability and anti-collapse measures](#18-stability-and-anti-collapse-measures)
19. [Build plan](#19-build-plan)
20. [Worked examples](#20-worked-examples)
21. [Open decisions](#21-open-decisions)
- [Appendix A: Decisions log](#appendix-a-decisions-log)
- [Appendix B: Deferred features](#appendix-b-deferred-features)
- [Appendix C: Glossary](#appendix-c-glossary)

---

## 1. Goals and pillars

### Goals

- An open world of small, sad villages and one large town, with **500+ people** and a full animal ecosystem, all alive whether or not the player is watching.
- People with **needs, skills and personalities** who walk to work, buy, sell, gossip, starve, steal, flee and die, and whose deaths and absences ripple through the world.
- **Jobs that follow from the economy.** When a bartender dies, the position is vacant, the pub suffers, and after a short grace period the best-fitting available person takes it, if one exists (section 12).
- **Permanent consequences.** The player can change the world for good, for worse or for better. Kill the only doctor, burn the mill, cull the wolves, fund a new bridge, and the world adapts around the change instead of undoing it (section 12.9).
- **Individual-level simulation** that scales from one craftsperson at a bench to a factory, with the same rules and more data.
- **Animals that matter:** herds, predators, scavengers, livestock. The player and NPCs can change the ecosystem, and the ecosystem pushes back.
- **Multiple start scenarios** in a world that already has history.
- A system that is **easy and safe for programmers to extend**, debug and tune.

### Non-goals

- Not a physics simulation. Physics belongs to the host engine and only runs near the player.
- No scripted event director in v1. Incidents must come from systems. A pacing layer that nudges parameters can be added later.
- No multiplayer in v1. The architecture does not prevent it.

### Pillars

1. **Causality over scripts.** Things happen because of recorded causes, and every cause can be traced.
2. **Individuals are the only writers.** Totals and averages are always derived, never stored.
3. **Same rules at every distance.** Off-screen is a different executor, not a different simulation.
4. **Legible.** If the player cannot see or infer a system's effects, the system is cut or reworked.
5. **Easy to extend and debug.** Tooling is a feature, not an afterthought.
6. **Always shippable.** Every milestone leaves a working game.

---

## 2. Core rules

These are invariants. Code review should reject anything that breaks one.

| # | Rule |
|---|---|
| R1 | **Individuals are the only writers.** Aggregates (factory output, unemployment, food supply) exist only as read-only derived views. |
| R2 | **Conservation.** Goods, coin, people and animals change only by recorded transfers, or through named, logged sources and sinks (harvest, import, consumption, spoilage, birth, immigration, death, emigration). |
| R3 | **Every state change has a cause.** Events carry a cause id, stamped automatically by the kernel. |
| R4 | **No teleporting.** Position changes only by walking a path, passing through a site door, or an explicit, logged exception. |
| R5 | **One decision function, any distance.** An agent chooses its next activity the same way whether it is visible or not. Only the executor differs. |
| R6 | **Store everything, schedule almost nothing.** Continuous values are (value, timestamp, rate). The scheduler holds only the moments when something must happen. |
| R7 | **Rules read and return commands.** They never mutate state directly. |
| R8 | **Handles, not pointers.** All references are typed generational handles. No globals. |
| R9 | **Deterministic** for a given build, seed and input log. |
| R10 | **Persistent state belongs to the core; transient state belongs to the physical layer.** They synchronize at defined events. |
| R11 | **Every system is visible to the player and testable headless.** |
| R12 | **The kernel knows no game concepts.** Hunger, bread and werewolves are content and modules. |
| R13 | **Structural continuity is kept by upkeep rules.** Individuals decide how to live. Upkeep rules decide who fills vacant roles, ownership, headship and offices, after a grace period and with real costs (section 12). |
| R14 | **Repair reallocates, it never creates.** Upkeep moves existing people, skills, money and goods. Anything that would create them is a slow, capped, conditional valve (section 12.9). |

---

## 3. Architecture

```
 +------------------------------------------------------------------+
 |  CONTENT        data files: goods, species, activities, positions,|
 |                 archetypes, thoughts, tunables, formulas          |
 +------------------------------------------------------------------+
 |  GAMEPLAY       needs, economy, ecology, law, ... each one a      |
 |  MODULES        module: record types, events, rules, activities   |
 +------------------------------------------------------------------+
 |  KERNEL         tables, handles, scheduler, commands, events,     |
 |                 fields, navigation, snapshots, tooling, replay    |
 +--------------------------+---------------------------------------+
                            |  narrow interface
 +--------------------------v---------------------------------------+
 |  ENGINE BRIDGE  spawn/despawn bodies, executors, physical events |
 +------------------------------------------------------------------+
```

| Layer | Who works here | Contains | Rule |
|---|---|---|---|
| Content | Designers and programmers | Data files, formulas | Never needs a recompile |
| Gameplay modules | Gameplay programmers | Rules, activity logic, queries, custom executors | Never touches kernel internals |
| Kernel | One or two core programmers | Storage, ids, scheduler, navigation, saves, tooling | Knows no game concepts |
| Bridge | Engine programmer | Spawning, executors, event reporting | Only crosses the narrow interface |

**The core is a library.** It has no dependency on the host engine, its entity system, or its rendering. It exposes:

- one call to advance time within a millisecond budget;
- an **input queue** (player actions, physical events, debug commands);
- an **output queue** (spawn, despawn, assign activity, world-object changes);
- read-only queries.

**Why this shape.** The sim can run headless for tests and soak runs, inside the game loop with a time budget, on its own thread, or as a separate process. Moving between those is a deployment change, not a redesign.

**Host engine assumptions** (verify early):

- Creating and destroying engine entities happens on the main thread.
- Threads may be unavailable on some target platforms (for example web builds).
- The engine's own entity system is built for rendered objects, so simulation records live in the core's tables, and only bodies near the player become engine entities.

---

## 4. Kernel

### 4.1 Tables and records

- One table per record kind: agents, animals, groups, items, lots, containers, plants, sites, stations, world objects, events.
- Fixed-size records in contiguous arrays (structure of arrays).
- **Hot fields** (position, next-event time, state flags) are separate from **cold fields** (names, provenance, history), so scans only touch what they need.
- Rare or variable-size data lives in side tables keyed by handle.
- No per-record heap allocation, no virtual calls, and no strings in records. Names are interned ids.

### 4.2 Handles

- A handle is 32 bits: index plus generation, typed per table.
- A stale handle gives a clear, loggable error and never a crash or silent reuse.
- Handles are stable across save and load.

### 4.3 Active and dormant records

Only **active** records are iterated. A dormant record (a sleeping person, a grazing animal in a quiet cell, an item in a pantry) costs nothing until woken by one of three things: a scheduled event, a spatial trigger from the encounter grid, or a message from another record.

### 4.4 Scheduler

- A global priority queue ordered by (time, sequence number).
- **Time wheels** for bulk expiry (spoilage, seasonal changes, history compaction), so expiry costs work only when something actually expires.
- **Cancellable tokens with versions**, so recomputing a process safely invalidates stale events.
- **Waves.** All events at time T are processed in waves. Rules emit commands, commands are applied at a safe point in a deterministic order, and any new events go to the next wave. A **cascade depth cap** (default 8, tunable) defers overflow to the next tick and logs it.

### 4.5 Continuous values

Anything that changes continuously is stored as `(value, timestamp, rate, bounds)`. Current value is computed on demand. When the rate changes, the kernel cancels and reschedules the next **threshold-crossing event**. This covers needs, container levels, plant growth, tool wear, healing, spoilage and environment fields.

### 4.6 Fields

Environmental quantities (forage biomass, fertility, moisture, pollution, temperature, light, fog) are **grid fields** at about 32 to 64 m per cell (tunable), using the same lazy (value, timestamp, rate) form. Fields are the physical medium, not decision-makers. Individuals (people, animals, plants, factories) read and write fields.

### 4.7 Determinism

- Seeded random streams per subsystem, never the wall clock.
- Iteration order is by id wherever order affects results.
- Guaranteed per build and platform. Cross-platform replays are not guaranteed unless sim-critical math is made platform-independent.
- The bridge's inputs are part of the replay log. Headless replays use abstract executors, or recorded physical reports.

### 4.8 Saves

- Binary, columnar, versioned and chunked, with dirty tracking and compression.
- Captured at a tick boundary and written on a background thread.
- Modules register **migrations**, so old saves keep loading when fields are added.
- Periodic in-memory checkpoints support replay and time travel for debugging.

### 4.9 History

- Full-detail events for the last 30 game days (tunable).
- Older history is compacted to significant events: deaths, births, crimes, ownership changes, large transactions.
- An item's provenance lives as long as the item, and is archived when the item is destroyed.
- Entities have timelines assembled from the event log.

---

## 5. Space and movement

Position is first-class and continuous for every agent at all times. Nobody is placed at a schedule slot. They get there by walking.

### 5.1 Two navigation meshes from one source

| Mesh | Resolution | Coverage | Used by |
|---|---|---|---|
| **Sim mesh** | Coarse (cells several times larger than the physical mesh) | Whole world, always resident | Abstract agents, long-distance routing, encounter prediction |
| **Physical mesh** | Fine | Streamed near the player | Bodies in the engine, local steering |

- Both are baked from the same world geometry by one bake step.
- Both use the **same area types**: road, cobbled street, path, field, meadow, forest, swamp, snow, shallow water, stairs, bridge, rail (hazard), and interior link.
- Each area type has a **speed multiplier** and a **cost multiplier** per mover class (human, child, horse, cart, species classes).
- **Off-mesh links** model doors, stairs, ladders, fords and ferries.
- **Bake validation (hard build error):** every site door must reach every other site door that it can reach on the other mesh, and area-type parity must hold within tolerance. A mismatch between the two meshes is the most dangerous bug this design can produce, because it silently strands people.
- Memory for the whole-world sim mesh is expected in the tens of MB for a large world *(estimate)*.

### 5.2 Sites, doors and slots

A **site** is any place that matters to the simulation: a house, pub, church, bakery, mill, field, well, market square, graveyard, clearing, station.

| Field | Meaning |
|---|---|
| kind, name | Template id plus a readable name |
| owner | Agent, household or institution |
| doors | One or more entrance positions on the sim mesh and the physical mesh |
| footprint | Bounds or polygon, used for ownership, privacy and fire adjacency |
| capacity | Maximum occupants |
| opening hours | Per weekday and season |
| offers | Tags: sleep, food, drink, company, worship, work, shelter, heat, water, medicine |
| slots | Typed positions (seat, bed, work position, queue spot, counter) with counts |
| containers, stations | Handles of what the site holds |
| positions | Jobs attached to the site |

- **Entering** a site: the agent walks to the door, an `Entered` event fires, and the agent is pinned to the site for the duration of the activity. **Exiting** places the agent at the door and starts a path.
- The sim tracks **occupancy and slot counts**. It never tracks positions inside a building. Exact slot positions are assigned only when the site is physically loaded and a body needs one.
- **Rooms** (bedroom, cellar, back office) are optional sub-sites, used when privacy or ownership differs inside a building.
- **Outdoor sites** (fields, forage zones, markets, graveyards) use a centre and radius or a polygon, with entry points where needed.

### 5.3 Analytic movement

Movement is a lookup, not an integration.

1. A path request (from, to, mover class) returns a polyline whose legs each carry an area type.
2. A **cumulative travel-time table** is built from leg length divided by speed (area multiplier × agent speed modifier).
3. The agent record stores the path handle and start time. **Position at time t is a binary search on the table plus interpolation.**
4. An `Arrived` event is scheduled at the end of the path.
5. If speed changes (injury, running, load, weather, fatigue), the path is split at the current progress. The remaining table is rebuilt, and the arrival event is cancelled and rescheduled through its versioned token.

Walking costs nothing until someone asks where an agent is.

### 5.4 The no-teleport rule

Position may only change by (a) path progress, (b) entering or exiting through a door, or (c) a **logged exception** (despawn and respawn recovery, supernatural events, scenario placement).

- The kernel exposes no general-purpose "set position" command to modules.
- A **teleport detector** runs in tests and soak runs. It records the implied speed of every position change per mover class and fails the run on violations.

### 5.5 Schedules are deadlines

There is no "be at X at 07:00". An activity has a **deadline** and a **location rule**. The decision function computes:

> departure time = deadline − (estimated travel time + buffer) − punctuality jitter

- Jitter is per agent, ±15 to 30 minutes, driven by the diligence trait, so crowds do not move in lockstep.
- Arriving late fires a `Late` event with consequences (docked pay, employer thought, missed service).
- Estimated travel time comes from the cached path length and is recomputed at departure.

### 5.6 Groups and vehicles

- Families, patrols, herds and caravans move as a **group**: a leader follows the path and followers keep offsets. One path query serves the group.
- A **wagon** is a vehicle plus a container. Cargo is individual items inside it, and it moves at the speed of the slowest draught animal, reduced by load.
- Groups split when a member's need interrupts or when something affects only that member.

### 5.7 Path service

- Path requests go to a queue that is **time-sliced** against the frame budget. If worker threads exist, queries can run there. The mesh is read-only, with one query object per worker.
- Results are applied at **deterministic points**, ordered by request id, so threading never changes outcomes.
- **Cache** site-to-site paths keyed by (door A, door B, mover class, nav version). Sites are static, so hit rates are high.
- **Long routes** use a coarse **road graph** derived from road and path areas, with full path refinement only at both ends.
- **Lateral offsets** are applied only at materialization, so people do not walk single file down identical lines.
- No crowd avoidance runs in the abstract layer. Bodies in the physical layer handle local avoidance.

### 5.8 Dynamic changes

Locked gates, collapsed bridges, flooded fords, snowed-in passes, quarantine cordons, fences and rail lines are **area-flag toggles** on the sim mesh and the physical mesh, not geometry rebuilds.

- Each region has a **nav version stamp**. Paths crossing affected tiles are flagged stale.
- Affected agents re-plan at their next event, or immediately if currently on a stale segment.
- Barriers also change ecology (migration routes, fenced fields), so toggles emit events that other modules can react to.

### 5.9 Weather and light effects on movement

Mud, snow and storms change the speed multipliers of area types. Darkness slows off-road movement unless the agent carries light. Fear (a mood effect) can increase running and shorten routes. All of these are formulas from data.

### 5.10 Spatial index

- A **uniform grid** with cell size equal to the longest sight radius (default 40 m).
- **Stationary** agents (and animals) are inserted when they stop and removed when they start moving.
- **Movers** are not stored per cell. They are queried each encounter tick by sweeping their path segments (see section 7).
- The same grid indexes world objects (bodies, items on the ground, notices) for perception.

### 5.11 World size assumption

The design assumes one continuous world with a whole-world sim mesh in memory. If the world is much larger than expected, the sim mesh becomes a tile cache with prefetch along active paths. This is listed in the open decisions (section 21).

---

## 6. People

### 6.1 The agent record

| Group | Fields |
|---|---|
| Identity | name id, age, sex, appearance seed, archetype id, species (human) |
| Social | household, home site, relationship row, memory handle, reputation per settlement (derived) |
| Movement | path handle, path start time, mover class, speed modifiers |
| Activity | current activity id, target handle, start time, expected end time, token, reason code |
| Needs | hunger, rest, company, purpose (each a lazy value with bands) |
| Body | health, injuries (side table), illness flags, warmth (lazy) |
| Skills | level and experience per skill |
| Personality | 8 axes (0 to 100): courage, greed, piety, sociability, temper, diligence, superstition, honesty; plus up to 3 trait tags |
| Mood | baseline, active thoughts list, current band |
| Work | position handle, employer, wage state |
| Belongings | pocket container, worn container, owned-things list |
| Control | decision strategy id (`needs`, `routine`, `player`, custom), state (alive, dead, away, dormant) |

### 6.2 Needs (v1 content)

Needs are **data**. These four ship first. More can be added without code.

| Need | Rises with | Relieved by | Critical consequences |
|---|---|---|---|
| **Hunger** (food and drink) | Time awake (about 20 game hours from fed to critical *(tunable)*), slower asleep | Eating meals, drinking, from household stock, shops, forage, charity | Work speed drops, mood thoughts, then health loss (malnutrition), then death. Theft and poaching enter the fallback ladder. |
| **Rest** | Time awake (about 16 to 18 hours to critical), faster with exertion | Sleep (bed quality changes recovery rate), resting | Errors and accidents at work, irritability, collapse and forced sleep |
| **Company** | Time alone (about 3 days to critical), scaled by sociability | Chats, family time, pub, church, shared work | Loneliness thoughts, lower mood, withdrawal, willingness to leave |
| **Purpose** (income security and standing) | Unemployment, debt, insufficient wages, disgrace | Working, being paid, clearing debt, respect events | Resentment, crime, quitting, emigration, breakdowns |

**Bands.** Each need has three bands (fine, pressing, critical) with thresholds from data. The kernel schedules a threshold-crossing event, so nothing is polled.

**Pressure curves.** Urgency is a formula per need and band. The default is gentle in "fine", steep in "pressing", and dominating in "critical", so a critical need pre-empts any non-urgent activity.

**Warmth and health are body state, not needs.** Warmth changes with weather, clothing insulation and heat sources. Cold exposure causes hypothermia effects and drives seeking shelter and fuel. This keeps the need count low while still making winter dangerous.

### 6.3 Body state

- **Health** is a lazy value that heals when fed, rested and warm.
- **Injuries** are records in a side table with location, severity, bleeding rate and infection risk. They affect speed, skill and mood.
- **Illness flags** are hooks for a future disease module.
- **Fatigue and pain** apply modifiers to work speed and decision quality.
- Death occurs when health reaches zero or by a `Kill` command from a hazard. The core decides death. The physical layer may report "health depleted" but the core processes it.

### 6.4 Mood and thoughts

Mood is how the simulation expresses dread, grief, comfort and the gothic tone, without adding more needs.

- A **thought** is a data row: trigger event type, mood value, duration, stack limit, and personality modifiers.
- **Mood = baseline + sum of active thoughts.** Thoughts expire through the scheduler.
- Mood bands: content, strained, breaking. Mood modifies work speed, social willingness and decision weights.

| Example thought | Value | Duration | Modifiers |
|---|---|---|---|
| Saw a body | Negative | 3 days | Scaled by courage |
| Friend died | Strongly negative | 30 days | Scaled by affinity |
| Heard the howling at night | Negative (dread) | 1 day | Scaled by superstition |
| Walked in fog | Mildly negative | While in fog | Scaled by courage |
| Good meal | Positive | 1 day | None |
| Slept in own bed | Positive | 1 day | None |
| Attended service | Positive | 2 days | Scaled by piety |
| In debt | Negative | While indebted | Scaled by greed |

**Breakdowns** (5 types in v1): drunken binge, withdraws to home and refuses work, panic flight, violent outburst, paranoid accusation of a neighbour. Each is an activity with its own consequences and events. Which one occurs depends on personality, not a dice roll over a flat table.

### 6.5 Skills and traits

- **Skills** (about 16 in v1): farming, husbandry, hunting, cooking, brewing, smithing, carpentry, weaving and tailoring, mining, machine operation, trade, medicine, clerical, speech, melee, firearms, stealth.
- Levels run 0 to 20. Experience accrues per work span, scaled by task difficulty relative to level. Unused skills decay slowly.
- **Traits** (up to 3 tags per agent) adjust learning rates, weights and thresholds: quick learner, steady hands, gossip, drunkard, pious, brave, coward, miser. Traits are data rows listing the formulas they modify.

### 6.6 Decision-making

One function chooses an agent's next activity at any distance. It is a **scored selection with a fallback ladder**, not a planner.

**Triggers** (the function runs only on these):
- the activity ended, failed or was interrupted;
- a need changed band;
- a thought or breakdown changed mood band;
- the agent woke or arrived;
- new information arrived (a notice seen, a rumour heard, an expectation failed);
- an obligation deadline is approaching;
- an idle safety timer, jittered, every 2 to 4 game hours.

A **minimum decision interval** (default 5 game minutes) prevents thrashing.

**Pipeline:**

1. **Candidates.** The affordance index is queried for activities relevant to pressing needs and obligations. Candidates are limited to **what the agent knows**: home, workplace, known shops, kin, sites visited or heard about, within a known-region radius. The set is capped (default 24).
2. **Preconditions** are checked: item in stock, site open and not full, skill met, permitted by owner and law, reachable, time window valid.
3. **Score** each candidate:

   > score = Σ over needs (pressure × relief) + obligation + trait and preference weight + opportunity − cost (travel time, price, risk, effort)

   Every term is a formula from data with shipped defaults. The agent's own price memory and place knowledge feed cost.
4. **Select.** The highest score wins, with seeded noise scaled by impulsiveness. The current activity gets a commitment bonus.
5. **Fallback ladder** if nothing scores above a floor or preconditions fail. Each archetype has an ordered ladder of "desperate measures", gated by personality and desperation. Food example: eat from home stock, buy, ask family, borrow, work extra, forage or hunt, beg, steal, leave the settlement, starve. Honesty, piety and courage determine where in the ladder someone stops.
6. **Commit.** A `StartActivity` command is issued. The scheduler sets the completion or failure event, and movement begins if the target is elsewhere. The chosen activity and its **reason code** are logged for the why-trace.

**Obligations** are first-class pressure sources: a shift starting at a site, household duties, church on Sunday, a court summons, a debt due. Pressure grows as the deadline nears, and missing it fires consequences.

### 6.7 The activity catalog

An activity is a data row.

| Field | Meaning |
|---|---|
| id, tags | What it is and what it offers (eat, sleep, work, socialise, worship, steal...) |
| location rule | Site tag, specific target, or anywhere |
| preconditions | Queries and formulas |
| duration | Formula |
| need effects | Rates during, deltas at the end |
| stock effects | Transfers consumed and produced |
| risk profile | Hazard table: injury, accident, encounter modifiers |
| executors | Abstract resolver id and physical executor id |
| interrupt policy | What may interrupt it, with reaction rule |
| noise and visibility | Used for witnessing and hearing |
| events | Event types emitted on start, success, failure |

**v1 catalog (about 35 rows, grows through data):** sleep, rest, eat at home, eat out, buy, sell, work shift, commute, socialise, drink, worship, attend funeral, forage, hunt, trap, fish, farm tasks (plough, sow, tend, harvest), cook, brew, haul or deliver, seek work, apply, collect rent, pay wages, search for missing person, report, investigate, bury, steal, beg, flee, fight, recover, visit doctor, tend animals, milk, shear, loiter, patrol.

### 6.8 Scripted agents without a second system

The `strategy` field selects who makes decisions:

- `needs`: the default described above.
- `routine`: a fixed timeline of activities with targets. Needs, mood and death still exist. A critical need or high-priority interrupt triggers the archetype's reaction rule, then the routine resumes.
- `player`: decisions come from input.
- Custom strategies, for example a planner for a detective, can be registered as modules.

Scripted story characters and simulated population share activities, executors, events and saves. That keeps the game shippable at every milestone and lets a story NPC be switched to `needs` later.

### 6.9 The player as an agent

The player is an agent with `strategy = player`. Their actions use the same commands and produce the same events as anyone else's, so stealing bread or killing a bartender is a normal event with witnesses and consequences. Player survival is a mode setting: off, light (hunger, rest and warmth modifiers) or full (all needs). The default is light. This is an open decision (section 21).

### 6.10 Relationships

Per pair of agents who have met, the relationship row stores familiarity (0 to 100), affinity (−100 to 100), trust (0 to 100), kin or role flags (kin, employer, rival), and last interaction time.

- Dense matrix up to about 2,000 agents *(estimate)*, sparse beyond that.
- Updated by encounters, shared activities, gifts, theft, violence, and events such as a death.
- **Household** is a separate record: members, head, home, shared containers and a ledger of income and spending.

### 6.11 Life events

Aging, health decline, death, birth, marriage (minimal in v1), coming of age, apprenticeship, retirement. Children cannot hold full positions. Births are needed to keep populations from silently shrinking, so a basic birth rule is in v1, driven by household conditions and not a hidden spawner. Inheritance runs through the death cascade (section 11).

---

## 7. Encounters, perception and knowledge

Agents meet by being near each other. Meeting is detected cheaply, filtered hard, and resolved through a small, closed outcome table.

### 7.1 Detection radii

| Radius | Default | Used for |
|---|---|---|
| Contact | about 4 m | Talking, trading, bumping, grabbing |
| Sight | 10 to 40 m, by agent or species | Witnessing, noticing, stalking, fleeing |
| Hearing | By noise event loudness | Screams, gunshots, fights, bells |
| Smell | Animals only, species data | Carcasses, prey, humans |

**Sight is a formula, not a ray cast.** It is the base radius modified by light level, weather, fog, terrain density (forest shrinks it, open field extends it), stealth, and the target's motion. Fog and night therefore reduce encounters through the same mechanism, which supports the gothic atmosphere with no extra system.

### 7.2 Detection tick

Each encounter tick (default 15 game seconds; 60 during fast-forward) the detector:

1. Takes every **mover**. Roughly 15 to 25% of agents are moving at any time *(estimate)*, so a few hundred at most.
2. Gets candidate neighbours from the spatial grid.
3. Runs a **swept closest-approach test** between the mover's path segment over the tick and each neighbour's segment or position. This is why ticks can be long without missing crossings. Segments are split at path vertices.
4. Skips indoor agents. People inside the same site interact through the site's co-presence rules (the pub's "company" offer), not geometry.
5. Applies the **interaction filter**: only pairs where some rule could fire proceed (hostile possible, social possible, witness possible, trade possible).
6. Applies **per-pair cooldowns** (default 30 game minutes) and a **cap on events per tick**.

### 7.3 Outcome table

The table is closed but extensible by modules. Every outcome is data plus a rule.

| Outcome | Trigger | Effect |
|---|---|---|
| Greet or ignore | Passing, both idle or walking | Small familiarity gain if acquainted |
| Chat | Acquaintances, both free to stop | Company relief, fact exchange, possible invitation (pub, home) |
| Witness | One sees a crime, body, creature or event | A fact enters memory, reaction by archetype and personality |
| Hostile | Predator or bandit vs. target in sight | Stalk, chase, fight or flee, through the resolver or physical combat |
| Trade, beg or help | Need-driven | Transfers, aid for the injured, charity |
| Pursue | Law agent vs. wanted person | Chase and arrest attempt |
| Graze or ignore | Animals passing without threat | Nothing, or group merging |
| Flee | Prey sees a threat | Group flight, wariness update |
| Stalk or ambush | Predator in range of prey | Approach with reduced detection |
| Scavenge | Scavenger reaches a carcass | Compete, eat |
| Raid | Animal reaches crops, livestock or stores | Damage, theft of food, defender response |

### 7.4 Noise

Loud activities emit a **noise event** with a radius and a duration. Agents inside the radius receive a hearing candidate. Reaction rules decide to investigate, hide, flee or ignore. Walls can attenuate noise through site footprints. Noise is how an off-screen scream becomes a rumour.

### 7.5 Promotion

If the detector predicts that an abstract agent will perceive a physical agent soon (using lookahead equal to sight distance divided by speed), the abstract agent is **promoted** to a physical body first. The materialization radius is always larger than the longest sight range, so abstract and physical agents should not perceive each other at all in normal operation. Promotion handles the edge cases.

### 7.6 Memory

Each agent holds a bounded set of **facts** (default capacity 64, tunable).

| Field | Meaning |
|---|---|
| fact | Reference to an event or a statement type (died, stole, seen at, price of, hired) |
| subject, place | Who or what, and where |
| time of fact, time learned | When it happened, when the agent learned it |
| source | Direct observation, or the agent who told them |
| confidence | 0 to 100 |
| hops | How many retellings away |
| salience | How memorable it is |

- **Direct observation** gives confidence 90 to 100. **Hearsay** decays with each hop and with distrust of the source.
- **Exchange.** At a chat each side shares a few facts chosen by salience, personality (gossips share more), and relevance to the listener. The receiver updates confidence. Conflicts resolve by confidence.
- **Errors are possible.** A retold fact can lose or alter details with a probability that grows with hops, so rumours can become wrong.
- **Forgetting.** When full, the lowest (salience × recency) fact is evicted.

### 7.7 Place knowledge

An agent knows sites they have visited or heard about. For each known shop and good they remember the **last seen price and availability**, bounded to a few shops per good. This memory drives shopping choices (section 9), and it can be wrong, which is the source of many small, believable economic behaviours.

### 7.8 News boards and gazettes

Public and institutional facts live on **notice boards** at settlements: vacancies, crimes reported, deaths, auctions, curfews, church announcements. Each posting has a visibility rule and a delay. Agents read a board when they visit its site. In the town, a **gazette** carries significant events as individual newspaper items with content derived from events. A newspaper that reports last week's murder is also a legible, physical object for the player.

### 7.9 Expectations

An **expectation** is a record: holder (agent, household, site), subject, condition (be at X, return by T, open at hour H), deadline, and an event type to fire if unmet. When the deadline passes without the condition being satisfied, the kernel emits an `Absence` event, and reaction rules do the rest.

| Example | Holder | Result when unmet |
|---|---|---|
| Hunter due home by dusk | Household | Worry thoughts, asks neighbours, reports to constable, search task |
| Worker due at shift start | Employer | Finds a replacement, docks pay, looks for the worker |
| Shop due open at 08:00 | Regular customers | Reroute to another shop, price knowledge updates |
| Patrol due to check in | Constable | Alarm, search |

Expectations are why the world notices off-screen events **by their absence**, which is cheaper and more natural than global omniscience. They are also the detection mechanism for upkeep rules (section 12).

### 7.10 Witnessing

A crime or violent event creates **witness candidates**: agents within sight radius at that moment, including those inside doors or windows by site rules. Each witness receives a fact. Their reaction depends on trust in the law, courage, relationship to the parties, and fear. Possible reactions: intervene, report, flee, ignore, or blackmail (for dishonest agents).

---

## 8. Items, containers and production

### 8.1 Items

Every handled unit is an **individual record**: a loaf, a coat, a lamp, a sack of grain.

| Part | Fields |
|---|---|
| Hot | location (kind + handle), quality band, flags, expiry bucket |
| Cold | type, created time, producer, batch id, input list (provenance), history |
| Ownership | owner handle (agent, household, institution) |

Item **types** are data: category, tags (food, tool, fuel, material, clothing, weapon, medicine, book), base value, weight, volume, decay profile, wear profile, uses.

### 8.2 Lots

Bulk commodities (grain, flour, coal, lamp oil, water, coin) are **lots**: one record with an exact quantity. A lot is a unit that people actually handle (a sack, a scoop, a purse), so it is an individual with identity and history, not a compression layer.

- Lots split and merge only through transfers. Merging requires compatible quality.
- Provenance stores a bounded list of source batches with shares.
- Coin is a lot in a purse in v1. Individual coins are possible if the design later needs change-making, but that is a gameplay choice and not a performance one.

### 8.3 Containers

A **container** has a type, owner, location (site, world, carried), capacity (weight, volume, slots), conditions (temperature class, dampness), lock state, and access rules (owner, household, staff, public). Types: pocket, pack, pantry, cellar, shelf, crate, barrel, cashbox, purse, wagon bed, corpse, carcass.

### 8.4 Transfers

**Transfers are the only way items change container.** A transfer states item or lot, quantity, from, to, reason and cause event.

Reasons: take, give, buy, sell, steal, produce, consume, spoil, drop, loot, trade, tax, inherit, destroy.

The **ledger** keeps recent transfers at full detail and compacts older ones. The **conservation audit** (section 18) sums transfers against sources and sinks.

### 8.5 Spoilage and wear

- Each item has an **expiry bucket** computed from its decay profile and its container's conditions, placed on a time wheel. Cost is incurred only when something expires or moves to a container with different conditions.
- On expiry an item changes state (stale, rotten, inedible) or is destroyed to the spoilage sink.
- **Tools and clothing wear** lazily by use time. Breakage is an event.

### 8.6 Stations and recipes

A **station** is a record with type, site, owner, state, operator slots, wear, fuel or power container references, and a current process.

| Station type field | Meaning |
|---|---|
| throughput multiplier | Output speed relative to hand work |
| operator slots | Minimum and maximum workers |
| required skill and tool | For operators |
| fuel or power use | Rate and item types |
| accident hazard | Chance and severity by skill and fatigue |
| emissions | Noise, pollution to fields |
| allowed recipes | List |

A **recipe** is data: input item or lot types and quantities, quality constraints, output types, quantities and quality function, base time per unit, skill, tool, station type, byproducts and emissions, hazards.

### 8.7 The process engine

A **process** is a running recipe at a station. Its rate is:

> rate = base × station multiplier × skill factor × mood and fatigue factor × (operators present ÷ operators required)

Instead of ticking, the process runs in **work spans** that end at the earliest of:

- inputs exhausted (from rate and stock);
- output container full;
- shift end;
- a worker's need goes critical, or an interrupt;
- tool or station wear threshold, or fuel or power runs out;
- the **span cap** (default 60 game minutes);
- a conflicting change in the site's state.

A **minimum span** (default 5 to 10 game minutes) prevents event spam when interrupts are frequent.

**At each span boundary the engine:**

1. Creates the units produced during the span in bulk: count = floor(rate × elapsed), with per-unit completion timestamps derived from the rate.
2. Consumes inputs through transfers.
3. Updates wear and fuel.
4. Grants skill experience.
5. Applies hazard rolls (accidents produce injury events).
6. Records the span: worker, station, start, end, output stack, and **the constraint that ended it**.
7. Recomputes rates and schedules the next span.

A factory producing 10,000 units in a shift costs about as many events as a workshop producing 100, if the same constraints bind. Output attribution stays individual because every span names its worker and station.

### 8.8 Local dependency graph

A site's workers, stations and containers form a **small dependency graph**. When something in it changes (a worker leaves, stock arrives), only that graph is recomputed, and only through the versioned tokens. Cross-site flows happen only through **carriers**: agents moving items between containers. There are no direct links between sites.

### 8.9 Quality and learning

Quality is a function of skill, tool quality, material quality and mood, mapped to bands (poor, common, fine, excellent) in v1. Experience accrues per span, scaled by difficulty relative to current level, and faster with a master present.

### 8.10 Scaling from craft to industry

Scale comes from composition, not new rules. Each row adds data, and sometimes a role.

| Level | What exists | What emerges |
|---|---|---|
| Craftsperson | 1 agent, 1 bench, own tools | Output follows skill and tool quality. Illness or death halts it. |
| Workshop | 3 to 6 agents, several benches, shared stock, apprentices | Division of labour, learning by doing, a master's knowledge matters |
| Factory | Powered stations with high multipliers, fuel and maintenance needs, shifts, foreman, clerk, carters | Output many times a handworker's, but dependent on coal, raw material, labour and transport, and dangerous |
| Industry | Several factories, rail, a town built around the mill | Company-town dependency, strikes, closures that reshape a region |

A power loom producing 20 times a hand weaver's output is the "bigger number of products", and it is also why a mill closing leaves a village with sixty unemployed weavers.

### 8.11 Farming and resources

- **Crops** are individual plant records on plots: type, growth stage (lazy), health, moisture draw. Planting and harvesting are work spans. Growth is scheduled by stage boundaries, and weather, soil and blight modify rates.
- **Trees, orchards and medicinal plants** are individual plant records.
- **Ore, clay and stone deposits** are node records with remaining quantity.
- **Wells** and rivers are sites and field links with water quality hooks (a disease module can use them later).

### 8.12 Emissions

Stations write to fields: noise, smoke, and **pollution** in river cells. A river is a chain of cells with downstream links, and pollution flows downstream by a lazy rule. Fish, villagers and animals downstream react through their own rules. This is how an upstream mill ends up affecting a village, with no scripted link.

### 8.13 Construction

Whether agents can build new sites and stations at runtime is an open decision (section 21). The design keeps hooks for both. Without runtime construction, buildings are authored and owners expand by activating inactive stations and adding shifts.

---

## 9. Economy and labour

There is no economy module that computes prices, wages or employment. There are individuals with rules, and everything else is a read-only view over them (R1).

### 9.1 Ownership and property

Everything that matters has an **owner**: an agent, a household or an institution. Ownership controls access. **Theft is a transfer without consent**, which fires events and witness candidates. Ownership changes only through transfers, inheritance, sales, seizure or abandonment.

### 9.2 Households

A **household** is a record: members, head, home site, shared containers (pantry, cashbox), and a ledger of income and spending.

- Wages are paid to individuals. By rule, members contribute a share to the household purse, which the head or a designated member spends.
- Shopping lists derive from a **household stock policy**: for each essential good, a target number of days of stock by household size. Falling below target triggers a shopping activity.
- Children are dependents. Elderly or ill members reduce earning capacity, which feeds Purpose pressure.
- The household ledger is a permitted aggregate because it is attributable and is what the head actually decides from.

### 9.3 Shops and posted prices

A shop is a site with stock containers, a shopkeeper and a price list. **There is no global price function.** The local price is whatever local shopkeepers post.

Each shopkeeper adjusts a posted price at most once per day, by a bounded, damped rule:

> new price = clamp(old × (1 + stock pressure + sales velocity pressure), floor, ceiling)

- **Floor** = cost basis × (1 + minimum margin). **Ceiling** = base value × maximum multiplier.
- **Stock pressure** rises when stock is below target and falls when above.
- **Velocity pressure** compares recent sales against expected.
- Maximum change per day is 5% (tunable). Traits shift the parameters: a greedy shopkeeper pushes harder, a fair one dampens.
- Shopkeepers also see competitor prices when they visit or hear of them, which adds weak competition.

### 9.4 Customer choice

A customer considers shops they **know** and scores each by remembered price, distance, queue, availability and trust. Prices are refreshed on visit. Occasionally an agent explores an unknown shop. Distance matters because walking takes real time, so a bakery across town loses customers to a nearer one without any extra rule.

### 9.5 Purchases

A purchase is a transfer of goods and a transfer of coin, with an event. Trips batch several purchases. A shop out of stock fires a `Stockout` event that customers can remember (and be wrong about later).

### 9.6 Supply chains

Two channels carry goods between businesses and shops:

- **Markets.** Market days at market sites let producers sell directly to customers and shops.
- **Standing orders.** A record between supplier and buyer: good, quantity, interval, price rule. Carters deliver. Payment is on delivery or on credit.

Carters are agents with carts. Their cargo is real stock, they can be delayed, robbed or killed, and a missed delivery has real downstream effects.

### 9.7 Credit and debt

A **debt** is a record: debtor, creditor, principal, interest rule, due date. Shops may extend credit based on trust. Unpaid debts reduce reputation and trust, lead to seizure of goods, court, and in the worst case the workhouse. Moneylenders are an archetype. Debt feeds Purpose pressure, so it drives behaviour.

### 9.8 Wages

- A position has a wage amount and interval. The employer pays by **transfer from the employer's cashbox** at each interval.
- **Piece-rate** positions (farm and factory) pay per unit, taken from the work spans attributed to the worker.
- If the cashbox is empty, pay is delayed. A delayed-pay event gives workers thoughts, raises quit probability, and can trigger **collective action**: a strike emerges when many workers at one employer share the grievance thought and a worker with the organiser trait acts.

### 9.9 Filling positions

**Default: the upkeep rule.** When a holder is found dead or misses work for a grace period (about 2 days), the employer fills the position from candidates who are unemployed or in a less important position and who fit the skill requirement. Candidates are scored, may refuse, and take time to arrive and settle in. The full rule, importance ordering, fallbacks and chain behaviour are in section 12. It is simple, testable, and covers most cases.

**Optional rich pipeline (strategy swap, not in v1).** A site or archetype can replace the default with the fuller model in the rest of this subsection, where vacancies spread through notices and gossip and individuals choose whether to apply. It is kept for sites where hiring itself should be a story (a guild, a great house).

In the rich variant, the world posts vacancies, and individuals choose.

**Vacancy** is a record: site, role, requirements, wage, hours, posted time, channels.

**Channels:** a notice at the site, the settlement board, gossip (employers tell contacts), and institutional broadcast. A person only applies to vacancies they know about.

**Seekers.** Agents with Purpose pressure, unemployment or a better prospect run `seek work`: visiting boards and employers.

**Applicant evaluation** (agent side):

> utility = wage gain + skill match + commute cost + status + relationship with employer + class acceptability − risk

An agent applies if utility exceeds their threshold.

**Employer evaluation:** skill fit, reputation, relationships, reliability history, cost, and trait biases (nepotism, prejudice by class). The employer hires the best applicant above their threshold, or waits.

**Vacancy ageing.** The longer a vacancy stays open, the lower the standards and the higher the wage the employer will offer. Fallbacks, in order: take an apprentice, owner covers the duty at reduced hours, advertise in town (which can bring an immigrant), close.

**Vacancy chains** emerge naturally: a hire vacates their old position, which posts its own vacancy. No cap on chain depth is needed because each link costs real events, and the chain ends at an unemployed person or an immigrant.

### 9.10 Succession and inheritance

- A position or ownership has a **successor rule**: designated heir, spouse, partner, apprentice, deputy, or the institution.
- When an owner dies, the death cascade transfers ownership of sites, stations, containers and debts by those rules. If no heir exists, the estate passes to the parish or a bank.
- A **deputy** can cover shifts while a vacancy is open.
- Ownership transfer, household head and dependents run as upkeep rules (section 12.5).

### 9.11 Apprenticeship

A master takes an apprentice into a low-wage position. The apprentice's experience gain is multiplied while the master is present. Passing a skill threshold and a minimum time promotes the apprentice to journeyman, then master. Ladders are data.

### 9.12 Public roles

Constable, doctor, priest, gravedigger, postmaster, teacher, gamekeeper and bailiff are positions **owned by institutions**. They use the same upkeep rule, with the institution as actor, institutional criteria and an **appointment authority** (the parish vestry for the gravedigger, the estate for the gamekeeper, the council for the constable, a patron or bishop for the priest) and an appointment delay.

### 9.13 Owners, investment and contraction

Owners decide from their **business ledger** (a permitted aggregate: sales, costs, cash, backlog).

- **Expansion.** Backlog above threshold, cash above reserve and labour available leads to extra shifts, hiring, or activating an idle station. If runtime construction is enabled, it can mean buying a new station.
- **Contraction.** Losses over several periods lead to cutting shifts, then laying off by owner criteria (seniority, skill, favour), then closure.
- Closure is an event with many reactions: unemployment, Purpose pressure, migration, debts, sale of assets.

### 9.14 Outside-world valves

The railway station connects the local economy to an outside world with **bounded, logged** flows.

| Valve | Behaviour | Bounds |
|---|---|---|
| Outside traders | Import and export specific goods at outside price lists | Daily goods and coin caps per trader |
| Immigration | Strangers arrive, generated with full history and needs | Slow, capped per settlement per season, conditional on attractiveness (wages, housing, safety, reputation). Can be disabled per settlement or scenario. |
| Emigration | Agents with sustained high Purpose pressure leave | By rule, logged |
| Taxes, rent, tithe | Transfers from households and businesses to institutions and the outside | By rule |
| Relief and wages from outside | Pensions, remittances | Bounded |

These are the inflation, deflation and famine controls designers can tune. They protect the untouched world and are deliberately too weak to undo large, lasting changes (section 12.9). All appear as named sources and sinks in the conservation audit.

### 9.15 Derived views

Examples: unemployment rate, price index, food-days of stock per settlement, production per site, profit per business, wealth distribution. They are declared with their dependencies, cached, invalidated on change, and **read-only by construction**.

### 9.16 Oscillation control

Individual pricing and wage rules can swing. Controls: floors, ceilings, damping, minimum change intervals, and a soak test that tracks price and wage variance before any new pricing behaviour is added.

---

## 10. Ecology

Animals are a second agent class on the same kernel, using the same scheduler, movement, encounters, items and events, with a smaller record. The ecosystem is the **food web of recorded transfers**: forage → herbivores → predators → carcasses → scavengers and people. Populations emerge from individual births and deaths, so anything that changes those changes the ecosystem.

RimWorld is the reference for scale and feel: animals as pawns with a few needs, plant-limited herds, risk-weighted predation, wildness and taming, seasons. Details here are inspired by that approach and are not copied from any implementation.

### 10.1 Climate and weather

- **Seasons** from the calendar.
- **Temperature** per region: seasonal baseline plus daily curve plus noise.
- **Weather state machine** per region: clear, overcast, rain, fog, snow, storm, with seasonal transition tables.
- **Light** from time of day and weather.
- Effects flow through formulas: plant growth, sight radius, warmth, travel speed, spoilage.

### 10.2 Environmental fields

Per cell (about 32 to 64 m): fertility, moisture, forage biomass by type (grass, browse, grain stubble), cover density, water presence, pollution, temperature. Forage regrows lazily: capacity from fertility × moisture × season, with a regrowth rate. Fields are the medium, not decision-makers.

### 10.3 Plants

Crops, trees, orchards and medicinal plants are **individual records** because the economy needs them individually. Wild grass and brush are field values, not plants.

### 10.4 Species data

A species is a data row. A new species needs **no new code**.

| Group | Fields |
|---|---|
| Body | size, health, speed per terrain class, stamina, damage, armour |
| Diet | forage types or prey classes, nutrition values, hunger rate |
| Life | lifespan, maturity age, gestation, litter size, breeding season, juvenile mortality |
| Behaviour | class (herbivore, omnivore, predator, scavenger, domestic, vermin, bird, fish, unique), activity pattern (day, night, dusk), group type and size range, home range size, flee distance, wariness base, danger rating |
| Senses | sight, hearing and smell radii |
| Habitat | preferred terrain and cover, water need, seasonal range preferences |
| Products | meat, hide, milk, wool, eggs, bones |
| Taming | wildness, trainable skills |

"Who hunts whom" is derived from diet, size and danger. It is not authored per pair.

### 10.5 The animal record

Handle, species, sex, age, health, hunger (lazy), energy (lazy), position and path, group handle, home range (centre and radius), activity, owner (domestic), wariness, flags (pregnant, tamed, injured), parents (for lineage and inheritance of traits if used later). **Every animal is an individual from the start.**

### 10.6 Groups

A **group** (herd, flock, pack) is a coordination structure. It has a leader whose decisions the group follows (target cell, water, rest, flight). Members copy the leader's path with offsets, so one path query serves all. Members are full individuals with their own records, and a member **leaves group control** the moment something affects it: shot, injured, tamed, separated, captured, or a mother with a young one lagging. Groups split and merge by rule. A group can go dormant as a whole.

### 10.7 Behaviour

A fixed, ordered priority list from species data. No planner.

> flee > fight if cornered > drink > eat > rest and sleep > breed and rear young > roam within home range

Thresholds and activity patterns come from the species row.

### 10.8 Herbivores

- Choose a cell by forage, safety, distance and group preference, then graze.
- Grazing consumes the cell's forage through the lazy field mechanism. **Per-cell consumption** is the sum of grazers present, and a cell-depletion event wakes grazers to re-decide when forage hits a threshold.
- Overgrazing lowers forage, causing weight loss, fewer births, then starvation or dispersal.
- They drink at water sites, rest, ruminate.

### 10.9 Predators

1. **Hungry** (hunger past threshold).
2. **Search** within territory, biased toward remembered prey density, tracks, scent and time-of-day patterns.
3. **Detect** through the encounter system (sight, smell).
4. **Choose target** by value:

   > nutrition × catchability ÷ (risk + distance), boosted for wounded, young, isolated and stragglers

5. **Stalk** (reduced detection), **chase** (speed and stamina against the prey's), **strike** (damage resolution through the abstract resolver or physical combat).
6. A **failed hunt costs energy**, which is what limits kill rates.
7. A kill becomes a **carcass container** with meat items scaled by body mass. The predator eats until sated, then rests.

**Pack hunting** adds coordination bonuses and lowers failure rates. **Territory** is a home range, which expands under hunger and contracts under satiety. Boundaries are contested with rival packs.

### 10.10 Scavengers and carcasses

A carcass emits a **freshness-scaled odour event** with radius and intensity. Scavengers (crows, foxes, rats, dogs, wolves) path toward it and compete for the meat items. A carcass progresses fresh → decaying → bones. Items inside spoil according to container conditions. **Human bodies are carcass-like containers**, so a murdered hunter draws crows and foxes, which creates clues.

### 10.11 Breeding, growth and mortality

- **Eligibility:** mature, healthy, in condition, in season, a mate within range.
- **Gestation** is a lazy timer. At birth, offspring records are created (count from litter size and maternal condition) with the mother.
- Juveniles follow their mother, grow lazily, and become independent at maturity. They have higher mortality.
- **Mortality causes** (all logged): starvation, predation, injury, cold, old age, hunting, trapping, poison, drowning, fire, rail strikes. Disease is deferred.

### 10.12 Dispersal and the edge valve

Juveniles at independence disperse to low-density neighbouring ranges, which recolonizes areas after a local die-out. Beyond the world edge there is a **bounded, slow, logged valve** per species with a **recolonization policy** in species data: `fast` (a few individuals per season where habitat is suitable and vacant), `slow` (years), or `never`. A bad winter that leaves survivors recovers by breeding. If people or the player drive a species to zero in an area, it stays gone unless its policy allows arrivals, and then only slowly. **Unique monsters never respawn.** Valve usage is a soak-test metric.

### 10.13 Wariness and seasonal ranges

- **Wariness** (0 to 100) is learned per group: it rises under hunting pressure and decays slowly. It scales flee distance and cover preference. Hunted herds become shy.
- **Seasonal ranges.** Species data defines winter and summer preferences, giving migration with almost no extra machinery.

### 10.14 Domestic animals

Owned animals have a handle to their owner and are fed fodder or graze owned pasture.

| Animal | Role | Systemic links |
|---|---|---|
| Cattle, goats | Milk, meat | Milking work spans, fodder demand, predator raids |
| Sheep | Wool, meat | Shearing, shepherds, dogs, predator raids |
| Hens | Eggs | Coop containers, foxes |
| Horses | Draught and riding | Fodder and rest limit carter capacity, which ties trade to farming |
| Dogs | Herding, guarding, hunting | Reduce raid success, detect intruders |
| Cats | Vermin control | Rats in granaries reduce stock |

**Vermin** (rats, mice) are animals in storage sites. They consume and spoil stock, which gives a granary a reason to have a cat.

**Taming and training** are repeated interaction work spans against the animal's wildness. They are added after the wild ecosystem is stable.

### 10.15 Unique monsters

A unique creature (a werewolf, for example) is an agent with species data, a **lair** site, a territory, an appetite (hunger like a predator) and an activity pattern tied to moon phase and time of day. Its prey preference is data. When normal prey is scarce, appetite pushes it toward livestock and people. It never respawns, and its presence produces fear thoughts, rumours and behavioural changes through normal channels.

A **hidden-identity variant** is supported through strategy swap: a villager agent whose flagged curse changes the controlling strategy and body at night. This is a content option, not a kernel feature.

### 10.16 Ecological LOD

Far from people, animals use longer path segments and coarser encounter ticks. The rules are identical. Predation that happens unobserved still counts. Near people, timing is finer. Beyond a cap on physical bodies, extra animals are shown as cheap instanced visuals with no AI body.

### 10.17 How people and the player affect the ecosystem

| Action | Mechanism | Visible result |
|---|---|---|
| Overhunting or poaching | Fewer adults, fewer births, wary herds | Fewer sightings, hunters travel farther, game prices rise |
| Culling predators | Predator count drops | Herbivore boom, overgrazing, crop raids, then starvation crash |
| Clearing forest, building farms | Cell cover and forage change | Displaced animals, raiders near fields |
| Mill effluent | Pollution in river cells | Fish die-off, sick downstream villages |
| Fences, walls, rail lines | Navmesh barriers | Cut migration routes, herds blocked or killed on the line |
| Leaving bodies or offal | Carcass containers | Scavengers drawn to the settlement, and clues |
| Livestock and pasture | Owned animals graze cells or eat fodder | Shepherds, raids, fodder demand, overgrazed commons |
| Traps and poison | Station-like world objects | Indiscriminate kills (dogs, children, protected species) and legal consequences |
| Fire | Field reset | Flight, then a flush of regrowth |

Game law ties this to crime: poaching is an offence, and gamekeepers are a real job.

### 10.18 Visible signs

Tracks and scat as decaying world objects. Carrion birds circling carcasses. Dogs and herds reacting before the player sees a predator. Silence when one is close. Hunters report real data. The news board carries sightings. Market stalls show game meat availability.

### 10.19 Ecology-only soak test

A world with climate, fields, plants and animals, and no people. Run 50 simulated years across 20 seeds with pass criteria (section 18). This lets the ecology be tuned in isolation before it is coupled to people.

### 10.20 Risks

- **Boom-bust cycles.** Predator-prey systems oscillate by nature. Counter with cover refuges, predator satiation, territorial spacing and learned wariness.
- **Extinction cascades.** Caught by the soak test, handled by the valve.
- **Mesh drift.** Animal area costs must match on both meshes.
- **Scope creep.** Genetics, disease and individual animal personalities wait until the base ecology is stable.

---

## 11. Consequences, law and institutions

### 11.1 The event model

| Field | Meaning |
|---|---|
| id, type, time | Identity, event type from the registry, game time |
| place | Site, cell or position |
| actors, subjects | Handles with roles (killer, victim, witness, owner) |
| cause id | The event or command that led to this one, stamped by the kernel |
| rule id | The rule that emitted it |
| payload | A small typed record |
| visibility | Private, local, public |
| significance | Used by history compaction |

Reactions are **rule tables subscribed to event types** (section 16). They never poll. Because the kernel stamps causes and rule ids automatically, any event can be traced upward to its origin.

### 11.2 Reaction catalog (v1)

| Event | Typical reactions |
|---|---|
| Death | Death cascade (11.3) |
| Birth | New household member, thoughts, registry updates |
| Injury | Mood, work capacity, doctor visit, bleeding schedule |
| Theft | Witness facts, owner thought, complaint, trust drop |
| Assault, Murder | Witnesses, fear thoughts, constable task, retaliation intent |
| BodyFound | Facts for finder, report, investigation, funeral tasks |
| Absence | Expectation reactions (section 7.9) |
| GapDetected | Grace timer, consequences for the site, upkeep repair (section 12) |
| Hired, Fired, Quit | Household income, thoughts, relationship updates |
| Closure, Bankruptcy | Layoffs, debts, asset sales, migration |
| Stockout, PriceSpike | Customer rerouting, hoarding, thoughts |
| Predator sighting | Fear, curfew, hunting parties, livestock protection |
| Crop failure, Blight | Food scarcity, price pressure, relief demand |
| Fire, Accident | Injury, property loss, insurance-like institutional response |
| Funeral | Mood effects, social contact, costs |

### 11.3 The death cascade

When an agent dies:

1. State is set to dead. The **body** becomes a world object and a container holding the agent's belongings, at the place of death.
2. A `Death` event is emitted with cause and actors.
3. **Household:** grief thoughts, new head if needed, loss of income, Purpose pressure.
4. **Ownership and debts:** inheritance rules transfer owned sites, stations, containers and animals. Debts pass to heirs or lapse by rule.
5. **Position:** the position is marked holder-gone and site consequences begin at once. The upkeep rule detects the gap after its grace period and repairs it (section 12).
6. **Relationships:** friends and kin receive thoughts scaled by affinity. Rivals may gain relief.
7. **Expectations:** those involving the deceased are cancelled, and new ones are created where relevant (funeral expected).
8. **Knowledge:** witnesses receive facts. If nobody saw the death, nobody knows until absence or a found body reveals it.
9. **Tasks:** funeral tasks appear for the priest and gravedigger. Cost is paid by the household or the parish.
10. **Environment:** the body attracts scavengers and decays. If the death was unnatural and the body is found, a constable investigation begins.

### 11.4 Crime and law

**Offences in v1:** theft, assault, murder, poaching. Trespass can be added as data.

**Law per jurisdiction** (settlement or estate) is data: which acts are offences, who enforces them (constable, gamekeeper, bailiff), and penalties (fine, stocks, gaol, workhouse, transportation, hanging).

**Flow:**

1. An offence event fires and creates witness candidates.
2. Witnesses decide whether to report, based on trust in the law, relationship to the parties, and fear.
3. A **complaint** record is created at the constable.
4. **Investigation** tasks: examine scene (world objects such as footprints, blood and dropped items), interview witnesses (fact exchange), follow tracks, check suspects' belongings.
5. **Suspect ranking** uses facts weighted by confidence and evidence items by provenance. Errors are possible, because facts can be wrong.
6. **Arrest attempt** (pursue), then a **hearing** before a magistrate agent on set days, then **penalty**.
7. Consequences cascade: job loss, family hardship, reputation changes, revenge.

The player uses the same pipeline as a suspect, witness, or amateur detective, which makes investigations real gameplay.

### 11.5 Reputation

Opinions live in **individual relationship rows**. A settlement-level reputation for an agent is a **derived view** over the opinions of its residents, weighted by contact. It is cached and invalidated as underlying opinions change.

### 11.6 Institutions

Institutions are records with a ledger, office-holder agents, containers, and rules. They are funded by real transfers and constrained by real budgets.

| Institution | Role | Notes |
|---|---|---|
| Parish | Church, priest, sexton, gravedigger, relief fund, poor box | Provides bread and coal to qualifying households. This is the **famine floor**. |
| Estate | Landlord, bailiff, gamekeeper, rents, game law | Can evict, set rents, restrict hunting |
| Council | Constable, market inspector, guardians of the poor | Runs the workhouse, markets, law |
| Workhouse | Beds, meals, labour tasks | Takes the destitute, carries stigma thoughts, a last resort |
| Court | Magistrate, hearings | Fines, gaol, orders |
| Bank | Savings, loans | Optional in v1 |
| Railway | Valve to the outside world | Section 9.14 |
| Companies | Mill, mine, brewery owners | Employers with ledgers |

Rates, rents, tithes and taxes are **transfers** from households and businesses by rule, carried out by collector activities.

### 11.7 Hooks for later systems

**Disease:** contagion through encounter contacts, shared sites and water sources. Illness flags and thoughts already exist on agents. **Fire:** adjacency between site footprints. Both are deferred but designed for (Appendix B).

### 11.8 The why-trace

Every command records the rule that issued it, every event records its cause, and every activity records its decision reason. The why-trace walks these links upward from any entity or state, such as "why is the pub closed?", "why is this man hungry?", "why is bread expensive?" It is both the main debugging tool and a basis for investigation gameplay.

---

## 12. Upkeep: how the world repairs itself

Most of what makes a world feel alive after something breaks is simple: the bar loses its barman, so the bar gets a new barman. Scripting each such case by hand does not scale, and simulating the full chain of notices, gossip and applications for every case is expensive to build and tune. This section replaces both with **one generic mechanism**.

> **Individuals decide what to do with their day. Upkeep rules decide who fills structural holes.**

### 12.1 The idea

Each thing that must exist for the world to keep working declares a **healthy condition**: a position has a holder who attends, a shop has stock, a household has a head, a body gets buried. When the condition fails and stays failed for a **grace period**, an **upkeep rule** picks a repair from a small toolkit.

- During the gap, the world suffers through ordinary systems. The pub closes early, regulars reroute, the cashbox shrinks. Nothing is scripted per case.
- The repair has a real cost: someone leaves a lesser job, coin is spent, time passes, a person has to walk there.
- Both the gap and the repair are normal events with cause ids, so the why-trace explains them.

This is a deliberate return to "the world assigns jobs, but not blindly" from the original brief: the world assigns, and individuals still have to be suitable, available and willing.

**Repair reallocates what the world already has. It never creates people, skills, money or buildings out of nothing.** That is what lets the player's changes last (R14, section 12.9).

### 12.2 Anatomy of an upkeep rule

An upkeep rule is a **data row** run by one generic engine.

| Part | Meaning |
|---|---|
| Subject | What is watched: position, ownership, household, stock level, office, body, group |
| Healthy condition | A query or formula that is true when all is well |
| Detection | How the world finds out: an expectation fires `Absence`, a threshold-crossing event, a derived view crossing a line. The responsible actor must actually learn of it (R3). |
| Grace period | How long to wait before acting. Data, scaled by the subject's importance. |
| Actor | Who performs the repair: owner, employer, institution, household head. A fallback chain of actors if the first is gone. |
| Candidates | A read-only query over individuals, with hard filters (alive, adult, able to reach, qualified, not in a more important role) |
| Score | A formula: skill fit, proximity, unemployed bonus, relationship with the actor, cost, class and trait fit |
| Acceptance | The candidate's own check. They may refuse, and the reason is logged. |
| Repair | One or more commands: assign position, transfer ownership, place an order, appoint, move in |
| Handover | Delay for notice and travel, plus a temporary efficiency penalty while the new person settles in |
| Fallbacks | Ordered: relax requirements, raise the offer, take an apprentice, recruit outsider through a valve, give up with consequences |
| Limits | Cooldown per actor, depth cap, per-period cap on outside recruitment |
| Events | `GapDetected`, `RepairStarted`, `RepairCompleted`, `RepairFailed` |

### 12.3 The default rule: fill a position

This is the rule for the bartender, and it covers every other job.

| Step | Default behaviour |
|---|---|
| Healthy | The position has a holder who is alive and attends |
| Detection | Holder confirmed dead, or missed shifts. The employer learns through the absence expectation or a found body. |
| Grace | Confirmed death: 1 day (a mourning period). Unexplained absence: 2 days. Both scaled by importance (critical roles shorter, minor roles up to about 5 days). |
| Actor | Employer or owner. If absent, the deputy, then the institution that governs this kind of site. |
| Candidates | Alive adults who can reach the site within a day on foot, meet the position's minimum skill, and are **unemployed or in a less important position** |
| Score | Skill fit, unemployed bonus, proximity, relationship with the employer, wage gain, trait and class fit. Weights are data. |
| Acceptance | Candidate's Purpose pressure, current wage, class acceptability, relationship. Refusals are logged with reasons. |
| Handover | Starts after notice and the walk, with a short learning penalty |
| Chain | The vacated position opens its own gap with its own grace period |

**Termination.** Each repair draws from a **strictly less important** position, so a chain can never loop and is at most as long as the number of importance tiers (about 10). It ends at an unemployed person or at an outsider.

**Fallbacks, in order (strength depends on the resilience profile, 12.9):**

1. If no local candidate qualifies after the grace period: raise the wage offer **within the owner's means**, and relax the minimum skill by one level for **non-critical** roles only, accepting lower quality. Critical and licensed roles never relax.
2. Take an apprentice, **if a master exists**. Learning takes months.
3. The owner covers the duty at reduced hours, if able.
4. Ask outside. Arrivals are slow (weeks), capped, conditional on how attractive the settlement is, and bring average skills. They are never available on demand for critical skills.
5. If the site still cannot be staffed and keeps losing money for the closure-patience period: `Closure`, with real and lasting consequences.

### 12.4 Importance

Importance orders candidates ("take from below"), sets grace periods, and decides how hard the world tries before giving up.

> importance = base value (data, 0 to 10) × context multipliers

Context multipliers come from read-only derived views: the **sole provider** of a staple in its settlement (the only baker), the number of households depending on the site, institutional offices (constable, priest). Importance is recomputed when its inputs change. It is a number you can show in the inspector and tune.

### 12.5 The v1 catalog

One mechanism, about fifteen data rows.

| # | Subject | Healthy when | Actor | Default repair | Fallback |
|---|---|---|---|---|---|
| 1 | Position | Holder attends | Employer | Fill (12.3) | Relax (non-critical only), slow outside arrival, closure |
| 2 | Ownership | Site, stations and animals have a living owner | Estate officer or parish | Heir order: spouse, adult child, partner, deputy | Institution holds, then sells |
| 3 | Household head | A capable adult heads the household | Household or parish | Eldest capable adult | Relatives take dependents, parish, workhouse |
| 4 | Dependents | Every child and invalid has a carer | Parish and relatives | Placement with kin | Orphanage, workhouse, apprenticeship at age |
| 5 | Dwelling | Every household has a home | Landlord or estate | Vacant dwelling by rent and class | Lodging house, workhouse, street |
| 6 | Shop stock | Staples at or above target | Shopkeeper | Restock from a known supplier (price memory, distance) | Carter to town, outside trader, closure |
| 7 | Workshop inputs | Recipe inputs available | Owner | Order from a supplier | Switch recipe, idle, lay off |
| 8 | Public office | Office filled | Appointment authority | Appoint (rule 1 with institutional criteria and delay) | Visiting circuit from a neighbouring settlement if the parish can pay, otherwise service lost |
| 9 | Burial | Body buried within days | Parish or relatives | Gravedigger task and priest | Pauper burial, neglect with mood and decay consequences |
| 10 | Missing person | Resolved | Constable | Search task with volunteers by relationship | Presumed dead after 7 to 14 days, which triggers rule 2 |
| 11 | Food security | Settlement food-days at or above floor (derived view) | Parish and council | Relief, council grain purchase, rationing | Outside trader, famine consequences |
| 12 | Infrastructure | Well, bridge, gate and roofs usable | Owner or institution | Repair task to a qualified craftsperson, materials ordered | Order materials, accept ruin |
| 13 | Raids | Raid rate below threshold | Estate and farmers | Hire hunters, gamekeeper task with bounty | Accept losses |
| 14 | Group leader | A group has a leader | The group | Oldest or strongest successor | Split or dissolve |
| 15 | Minimum roles | A settlement has its key roles | Council | Ask for outside arrivals (slow, capped, conditional, 12.9) | Settlement decline |

Ecology (births, deaths, predation, dispersal) is **not** an upkeep rule. Only *human responses* to ecological events and *group leadership* use it.

### 12.6 What stays emergent

| Decided by upkeep rules (structure) | Decided by individuals (life) |
|---|---|
| Who fills a vacant position, ownership, household head | What an agent does with their day, which shop they use |
| Who is appointed to an office | Needs, mood, thoughts |
| Whether a shop restocks and from whom | Prices posted by shopkeepers, wages paid, production |
| That a body gets buried and a missing person gets searched for | Crime, gossip, relationships, violence |
| That a settlement does not silently lose key roles | Animal populations, predation, ecology |

The test for a new case: if the answer is "something must be restored so the world keeps functioning", it is an upkeep rule. If it is a choice with personality in it, it belongs to the decision function.

### 12.7 Keeping it from feeling blind or mechanical

- **The world suffers first.** The grace period is a visible gap, not instant replacement.
- **A named actor with a stated reason.** "Mrs Pike hired Martha because the barman position was vacant for two days and Martha had the best score." The rule id and score breakdown are in the why-trace.
- **Candidates can refuse,** and their refusals have reasons.
- **Handover is real.** The new person walks there, takes a day or more to start, and works worse at first.
- **The repair costs something.** A wage, a recruiting fee, and the gap left in the lesser job they came from.
- **It is visible.** `GapDetected` can spawn a notice object at the site ("Barman wanted") and gives dialogue something true to say. This needs no gossip simulation.

### 12.8 Guard rails

- **Termination:** strict importance ordering plus a depth cap.
- **Honest failure.** Repairs can fail, and failure has consequences (closure, lost service, decline). Without this the world would be unkillable. A gap with no candidate stays open for as long as it takes.
- **Metrics:** gaps opened per day, repair latency, chain depth, fallback and valve usage, failure rate. In an untouched world a handful of repairs per day is normal and a flood signals a defect upstream. In a shocked world, long-lived unrepaired gaps are expected and are checked for explanation, not for recovery (section 18.3).
- **Caps:** cooldown per actor, per-period limit on outside recruitment per settlement.
- **Determinism:** candidates are ranked by (score, id).
- **Upkeep inspector:** open gaps with timers, candidates considered and rejected with reasons.
- **Killing has lasting cost.** The same rules apply to everyone. Killing key people costs the world real time, skill and money, and outside recruitment is slow and capped. Stacking damage until a settlement dies is allowed and intended. Guard against crashes and invalid states, not against decline.
- **Rule discipline:** a new case is a new data row. A case needing custom logic uses a strategy swap (16.7). A lint warns above about 20 rows.

### 12.9 Weak by design: the player's changes stay changed

The player must be able to change the world for good, for worse or for better. Kill the only doctor, burn the mill, cull the wolves, or bring a doctor from town and fund a new bridge, and the world adapts around the change instead of undoing it. Upkeep is therefore built to **reallocate what the world has and never create what it lacks** (R14).

| # | Permanence rule | Effect |
|---|---|---|
| P1 | **Candidates come only from existing agents.** | If nobody local has the skill, the gap persists. |
| P2 | **Skills are not fungible.** | Critical and licensed roles (doctor, priest, smith) never relax their minimum skill. Other roles may relax by one level at a quality penalty. Skills grow only by training, over months to years. |
| P3 | **Repairs cost resources.** | Recruiting fees, wage differences, materials, relief funds. An actor without the budget defers or fails. Poor places repair less. |
| P4 | **Outside valves are slow, capped and conditional.** | Arrivals depend on attractiveness (wages, housing, safety, dread, reputation), take weeks, bring average skills rather than specialists on demand, and can be disabled per settlement or scenario. |
| P5 | **Closure and ruin are sticky.** | A closed site reopens only when an owner with capital and a candidate choose to. Buildings decay and are not rebuilt automatically. |
| P6 | **The irreplaceable stays gone.** | Authored and unique NPCs, unique monsters, destroyed unique items and buildings, locally extinct species (by policy). A role can be refilled, but the individual's relationships, knowledge and quest hooks are lost. |
| P7 | **Memory is long.** | Reputation, grudges, grief, debts and graves persist for years. Thoughts fade, but standing does not snap back, and a bad name lowers candidates' willingness. |
| P8 | **Decline loops reinforce.** | Fear leads to emigration, fewer customers, closures, fewer jobs and more emigration. Recovery needs investment from an owner with capital, or from the player. |
| P9 | **Local is fast, distant is slow.** | Reallocation inside a settlement takes days. Anything needing outside help takes seasons, and may never come. |

**The untouched world must still hold together.** Weak healing must not make the baseline decay. Normal turnover (aging, illness, ordinary departures) is covered by sustained demographic processes: births, apprenticeship, succession by kin, and a low steady arrival rate. These are part of the baseline balance, not upkeep. Upkeep only handles the leftover gaps. The soak tests therefore treat an untouched world (must stay stable) and a shocked world (must stay valid and explicable, not recovered) differently (section 18.3).

**The world has more than one stable state.** A thriving village and a dying village can both persist. The player can tip a settlement either way, and the new state is as durable as the old one.

**Resilience profile.** One tunable profile sets how strong upkeep is. The values are starting points to tune with soak tests.

| Parameter | Weak (default) | Normal | Strong |
|---|---|---|---|
| Grace period multiplier | 1.5× | 1× | 0.5× |
| Candidate search radius | Same settlement and neighbours within a day's walk | Two days' walk | Whole region |
| Skill relaxation | Non-critical roles, one level, quality penalty | One level, all but licensed roles | Two levels |
| Repair budget (share of an actor's reserves they will spend) | Low | Medium | High |
| Outside arrivals | At most one per settlement per season, never for critical skills | One per month | One per week |
| Closure patience (days of losses) | Short | Medium | Long |
| Reputation memory | Years | Months | Weeks |
| Species recolonization | Per species policy, mostly slow | Faster | Fast |

### 12.10 Relationship to the rest of the design

- **Detection** uses expectations (7.9) and derived views (9.15).
- **Candidate queries** are read-only queries (16.5), and **repairs are commands** (16.4) issued by the actor. R1, R2, R3 and R7 still hold.
- **Richer behaviour** can replace the default repair for a site or archetype by strategy swap (16.7), for example the full vacancy-and-application pipeline kept in 9.9 as an option.
- **Build impact:** milestone J1 becomes U1 (the upkeep engine plus v1 rows). The total schedule is unchanged, but scope is larger and risk lower.
- **Permanence:** healing strength comes from the resilience profile and the permanence rules (12.9).

---

## 13. Fidelity levels and the engine bridge

### 13.1 Two levels

| Level | Who | What runs |
|---|---|---|
| **Physical** | Agents and animals within the relevance radius of the player, up to a body budget | Engine bodies, local steering, animation, real combat, perception |
| **Abstract** | Everyone else | Analytic movement on the sim mesh, abstract resolvers |

The decision function, needs, mood, memory, transfers, events and economy run **identically** in both. The body budget (default 150 to 300 *(estimate, set by measurement)*) is tuned per target platform. Candidates are ranked by distance, visibility, relationship to the player, and quest involvement. Agents beyond the budget stay abstract even if close.

### 13.2 Who owns what (R10)

| Core owns (persistent) | Physical layer owns (transient) |
|---|---|
| Needs, mood, health, injuries | Pose, animation state |
| Inventory, ownership | Micro-position, collision |
| Knowledge, relationships | Hit detection details |
| Activity intent and reason | Local steering |
| Death | Reports "health depleted"; core decides death |

Damage is applied to the core immediately through events. The physical layer never holds state the core cannot recover.

### 13.3 Materialization

1. **Trigger:** an agent enters the spawn radius (always larger than the longest sight range plus a margin), and the body budget allows.
2. **Placement:** project the agent's current sim position to the physical mesh. If the spot is visible to the player, delay, or hide the spawn with fog, a door, or an occluder. If the agent is inside a site and the site is loaded, spawn at its slot.
3. **Create** the body, attach it to the core record, and mark the agent physical.
4. **Assign** the current activity to the matching physical executor, resuming where the analytic path says the agent would be.
5. **While physical:** the body reports position at about 1 Hz so abstract agents can detect it. Progress comes from the body, not the analytic table. The core keeps processing needs, decisions and events.

### 13.4 Dematerialization

When an agent leaves the despawn radius (larger than the spawn radius, for hysteresis) and is out of view: write back position (projected onto the sim path), health, inventory and orientation, then continue the activity abstractly with the remaining duration. If the body is mid-action (in combat, for example), despawn is deferred until it is safe.

### 13.5 Failure handling

- **Blocked body:** no progress for several seconds produces a `Blocked` event. The core re-paths. After repeated failures the core advances the body along its path or respawns it, logged as an exception.
- **Executor timeout:** an executor that does not finish within twice its expected duration is cancelled, and the activity is resolved abstractly.
- **Stale nav version:** bodies re-plan when the nav version of their region changes.

### 13.6 Executor contract

An executor is anything that makes an activity physically happen. The contract is small:

- **Start** with activity, target and parameters.
- Report **progress**, **complete**, **fail** (with reason), or accept an **interrupt** (with reason).
- Provide a **duration estimate** to the core.

The bridge ships default executors: WalkTo, UseSlot (sit, sleep, stand at bar, work position), Wait, Interact (take, give, talk), Combat handoff, Flee, Graze, Hunt. They are implemented with whatever AI technology the engine uses (behaviour trees, state machines, hand-written logic). Gameplay programmers add executors only for genuinely new physical behaviour.

### 13.7 Abstract resolvers

Activities with risk or contested outcomes (hunt, fight, steal, travel hazards) have **abstract resolvers**: a formula or small function from the same stats the physical layer uses (health, damage, skill, speed). A **calibration harness** runs the physical version headless thousands of times, compares outcome distributions (win rate, injury rate, duration) and flags divergences beyond a tolerance (default ±10% relative). An off-screen hunt must neither be safer nor deadlier than an on-screen one.

### 13.8 Bridge messages

**Core to host (output queue):** SpawnBody, DespawnBody, AssignActivity, WorldObject changes (bodies, items, notices, tracks), SetEnvironment (time, weather, fog), NavVersionChanged, BarkRequest (with context).

**Host to core (input queue):** PositionReport, Died, Wounded, ItemMoved, WitnessReport, FoundBody, ActivityCompleted, ActivityFailed, Interrupted, Blocked, PlayerCommand, DebugCommand.

### 13.9 Dialogue and barks

The core exposes an **agent context query**: needs, mood, recent events, opinions, known facts, current activity. Dialogue and bark systems read it so NPCs say things that are true about the world. This is a main way the player perceives the simulation.

### 13.10 Time acceleration

When the player sleeps, waits or travels far, physical bodies are suspended, and the sim steps with larger ticks (encounter ticks at 60 game seconds). When the player stops, bodies are rematerialized by normal rules. Headless fast-forward uses the same path.

---

## 14. How the gothic theme emerges

Theme comes from systems, not scripts. Each row is a mechanism that already exists, tuned for tone.

| Theme | Mechanism | What the player sees |
|---|---|---|
| Poverty and debt | Wages, rents, credit, debts, Purpose pressure | Threadbare households, sold belongings, men drinking away wages, creditors at doors |
| Scarcity and winter | Seasons, forage, fuel, warmth, relief limits | Empty stalls, fuel theft, hungry children, visible hardship in cold months |
| Industrial hazard | Station accident hazard, pollution fields | Maimed workers, dead rivers, smoke over the mill town |
| Fog and darkness | Sight radius formula, light, fear thoughts | Fewer encounters, nervous behaviour, danger you cannot see |
| Faith and superstition | Piety and superstition traits, rumour mutation | Charms on doors, accusations, people avoiding certain places |
| Dread | Thought family and mood bands | A village that changes behaviour after a death or a howl |
| Death as ordinary | Death cascade, funerals, grief | Funerals, orphans, graves, empty chairs |
| Vice | Needs, leisure, addictive goods | Drink, laudanum, regulars who stop coming |
| Class | Positions, access rules, respectability, appointment authority | Who can enter where, who is believed |
| Isolation | Travel time, weather, nav closures | Cut-off villages in snow, long walks, unreliable carts |
| Decay | Building condition (lazy wear) funded by owners | Ruined estates, shuttered mills, collapsing cottages |
| The unnatural | Unique monsters on predator rules, hidden-identity content | A threat that behaves like a living thing in the ecosystem |

**Tone control.** A small set of tunables (relief generosity, wage floors, mortality, monster appetite, fog frequency) sets how bleak the world is, so tone can be adjusted without touching rules.

---

## 15. World generation and start scenarios

### 15.1 Pipeline

1. **Authored geography.** Terrain, roads, rivers, forests, and **sites** (with doors, slots, fields, forage zones) placed in the world editor.
2. **Bake.** Sim and physical navigation meshes, field grids, site tables, spawn zones. Validation fails the build on errors.
3. **Population generation** from positions, not from scatter.
4. **Stock and ownership initialization**, consistent with conservation.
5. **Ecology seeding.**
6. **History pre-roll** with the real simulation, headless.
7. **Scenario injection** of the player.

### 15.2 Population from needs

- Every site declares positions. Generation creates **households around them**: family clusters with plausible ages and sexes, kin links, home sites, and skills consistent with roles plus noise.
- Personality axes come from archetype distributions. Archetypes are data: farmhand, publican, mill hand, constable, gentry, vagrant, and so on.
- A village needs a baker, smith, priest, publican, farmers, labourers, a few widows and orphans. The town adds mill workers, clerks, shopkeepers, doctor, magistrate, police, gentry, and a visible underclass.
- **Authored NPCs** (about 20 to 30: priest, doctor, constable, publican, estate owner, mill owner, and so on) override generated slots. They are hand-written with authored relationships and secrets.

### 15.3 Stock, money and ownership

Pantries are seeded with plausible stock by household size and class. Coin is distributed with a skewed distribution by class. Initial stock and coin are recorded as an **initial endowment** source in the conservation audit. Sites are assigned owners, and businesses get working stock, cashboxes and standing orders.

### 15.4 Ecology seeding

Animals are placed by habitat and density, with groups, home ranges and ages. The pre-roll lets populations settle.

### 15.5 History pre-roll

The pre-roll runs 3 to 5 simulated years with the real rules, abstract executors only. It produces:

- relationships, grudges and friendships;
- debts, savings, bankruptcies;
- graves, orphans, widows, vacancies filled and unfilled;
- an ecosystem that has already had its winters.

It doubles as a **stability test**: a seed that fails the soak criteria is rejected and regenerated. Authored NPCs are protected during pre-roll (reduced hazards, constrained life events) but not after. The result is cached as a **starting snapshot** so players do not pay the cost. Target pre-roll time on a development machine is under 5 minutes *(estimate)*, which needs the kernel to sustain roughly 1M events per second headless.

### 15.6 Scenario bundles

A start scenario is **data**, not code. It describes the player's initial conditions inside the generated world.

| Field | Meaning |
|---|---|
| id, name, description | Presentation |
| start site and time | Position, date, season |
| character | Age, skills, traits, appearance options |
| belongings and coin | Initial containers and purse |
| household and home | Or none |
| position | A job, or none |
| debts and obligations | Records created at start |
| reputation seeds | Per settlement |
| ties | 3 to 6 pre-wired relationships with named NPCs |
| secrets and flags | Wanted, cursed, disgraced, hunted. The world gets matching facts and records (a warrant, a rumour in specific NPC memories) |
| survival mode | Off, light, full |

Scenarios may adjust small tunables (a hard winter, a recent outbreak) but may not rewrite the world. The world the scenario starts in is the same living world as every other scenario.

### 15.7 Candidate scenarios

| Scenario | Start | Early pressures |
|---|---|---|
| Vagrant at the station | No home, no job, a few coins | Hunger, shelter, work, the workhouse |
| Village apprentice | Home, master, wage | Obligations, ambition, a master with secrets |
| Discharged soldier | Injury, debt, skills | Medical costs, employment prejudice, a veteran's pension dispute |
| Parish doctor | Position, patients | Disease hooks, poverty, class tension |
| Curate or priest | Position, congregation | Faith and doubt, relief decisions, rumours |
| Poacher | Skills, a warrant | Estate gamekeeper, hunger, ecological effects |
| Heir to a ruined estate | Property, debts | Rents, tenants, decay, relatives |
| Undertaker's assistant | Position, dead | Death patterns, secrets, scavenger tasks |
| Stranger with a curse | Flags, rumours | Fear, exile, the hidden monster |
| Mill hand | Wage, housing | Industrial hazard, strikes, company town |

### 15.8 Entry

A player who starts as an outsider **arrives through the same path as immigrants** (the station). A resident starts at home. Both are handled by the same records and rules.

### 15.9 Seeds

World seed plus scenario plus tunable profile reproduces the start exactly, which supports testing and bug reports.

---

## 16. Programmer experience

This is the main design goal of the kernel. It determines whether the system stays workable after the first year.

### 16.1 The customization ladder

A programmer should use the cheapest rung that works.

| Rung | Mechanism | Example |
|---|---|---|
| 1. Data | A row in a data file | A new job, good, species, thought, activity, scenario |
| 2. Data plus formula | Sandboxed expression in the data file, compiled at load | Pressure curves, price rules, risk, scoring terms |
| 3. Rule | A function reacting to an event, returning commands | "When a constable finds a body, open an investigation" |
| 4. Strategy swap | Replace a pluggable strategy for one archetype | A planner for a detective, a custom hiring rule for a guild |
| 5. Module | New record types, events, systems | Disease, fire, a new institution |

Most daily work happens on rungs 1 to 3. **If a feature needs a kernel change, the kernel is missing an extension point, and that is treated as a kernel bug.**

### 16.2 Modules

A module declares in one place: record types, events it emits and consumes, rules, activities, tunables, derived views, console commands, and dependencies on other modules.

- **Explicit registration** in a known order. No static-initializer tricks.
- The kernel collects declarations, orders them, and validates at startup: unknown ids, dangling references, duplicates, dependency cycles, conflicting rule orderings, and data files against **schemas generated from the registered types**.
- Errors give file, line, field, and a did-you-mean suggestion.
- A **startup dump** lists every module, rule, event binding and execution order.
- Budgets (number of needs, goods, activities and so on) are **lint warnings**, not hard limits.

### 16.3 Data files and formulas

- Human-readable text with comments. Ids are readable names, interned at load.
- **Hot reload** with validation. A failed reload leaves the previous data in place and reports.
- **Formulas** are a small expression language: arithmetic, comparisons, clamp, lerp, curve lookup, and read-only access to fields of the context (agent, site, item, species). They compile to bytecode, have no side effects, and carry a complexity limit. A formula test harness evaluates them against fixture contexts.

### 16.4 Rules and commands

A rule:

- declares its **trigger** (event type, or a keyed periodic schedule);
- takes a **read-only view** of the world;
- declares what it **reads and writes**, which lets the kernel order rules, detect conflicts, explain failures, and parallelize later;
- **returns commands**. It never mutates state directly (R7).

Commands are validated and applied at a safe point in a deterministic order. Core commands include: create or destroy record, set field (through typed setters), transfer, start or cancel activity, schedule or cancel event, emit event, post vacancy, create expectation. Validation errors name the rule that issued the command.

Benefits: no re-entrancy bugs, trivially testable rules, automatic cause stamping.

### 16.5 Queries and derived views

**Queries** are composable and read-only: spatial (within radius, in site), relational (household members, employees of a site), filters (by need band, skill, state), counts and folds.

**Derived views** are named aggregates with declared dependencies. They are cached, invalidated by writes to their dependencies, and cannot be written. Adding a new one is a few lines. Costly queries produce profiler warnings.

### 16.6 Activities and executors

An activity is a data row (6.7). Default executors cover most needs. When a new physical behaviour is genuinely needed, a gameplay programmer writes an executor against the small contract (13.6). The abstract resolver is a formula in the data or a small function, and the calibration harness compares it with the physical one.

### 16.7 Strategies

A **decision strategy** takes an agent view and returns an activity choice and a reason string. Shipped: `needs`, `routine`, `player`. Custom strategies register as modules and can be assigned per archetype or per agent, with no effect on others.

### 16.8 Tooling

Tooling ships with the kernel, not after.

| Tool | Purpose |
|---|---|
| **Why-trace** | Select anything and see the chain of events and rules behind its state |
| **Inspector** | Live panels per agent, animal, site, item, station, household: needs, activity, inventory, memory, relationships |
| **Event stream** | Filterable live log with **conditional breakpoints** ("pause when agent X's hunger becomes critical") |
| **Entity timeline** | Every event that touched an entity |
| **Rule profiler** | Per-rule time, events, commands, with budget warnings |
| **Upkeep inspector** | Open gaps with timers, candidates considered and rejected with reasons, repair history |
| **Overlays** | Navigation, paths, encounter radii, territories, forage, pollution, ownership, expectations |
| **Snapshot diff** | Compare two times or two runs |
| **Conservation audit** | Names the transfer that broke the invariant |
| **Console** | Commands registered by modules like everything else |
| **Replay and time travel** | Load a report, jump to a few ticks before a failure, step forward |
| **Soak dashboard** | Charts and thresholds from headless runs |
| **Auto-generated reference** | Modules, events, rules, schemas, tunables, activity catalog, data columns |

### 16.9 Determinism and replay

The sim is deterministic for a given build, seed and input log (R9). A bug report is a seed, a tick and an input file. Periodic in-memory checkpoints let the debugger rewind. This changes reproducing an emergent bug from "hope it happens again" to "load the report", which is the single biggest productivity lever in the design.

### 16.10 Testing

- **Headless by default.** The sim is a library. Tests start in milliseconds.
- **Fixture builders** create a tiny world in a few lines: five agents, one bakery, two doors.
- **Scenario tests** read as stories: given this village, when the baker dies, then within three days someone else bakes or the price has risen. Assertions query events and causes directly.
- **Golden runs:** state hashes at fixed ticks catch unintended changes.
- **Property and fuzz tests:** conservation holds under random events, positions never imply impossible speeds, saves round-trip.
- **Soak tests** run in CI with pass/fail thresholds and trend charts (section 18).
- **Performance tests** with synthetic populations at 10 times the target.

### 16.11 Tunables

Every constant is a **registered tunable**: name, range, description, default, owning module. Tunables are live-editable, stored as **profiles**, and comparable across soak runs. An experiment is a profile plus its result charts. The tuning loop is: change, soak, compare.

### 16.12 Saves and versions

Saves are versioned. Modules register migrations. A new field gets a default in old saves. Data files are versioned and validated against schemas.

### 16.13 Documentation

Reference docs are **generated from registrations** and cannot go stale. Hand-written recipes cover common tasks: add a good, add a species, add a job, add a rule, add an executor, add a module, add a soak metric.

### 16.14 Definition of done for a module

A module is not finished until it has: a schema, defaults, scenario tests, soak metrics, an inspector panel, why-trace support, generated docs, and a place where the player can see its effect.

### 16.15 Worked example: laudanum addiction

| Piece | Content |
|---|---|
| Data | A good (laudanum), an activity (take a dose), a need (craving) with bands, a thought (withdrawal) |
| Formulas | Craving growth by dose history. Dose effect on dread and rest. |
| One rule | When craving reaches critical and no stock exists, raise the weight of theft and the pharmacist's door in the decision score |
| Tests | One scenario (an addict with no money turns to crime within N days). One soak metric (addiction prevalence stays bounded). |

No kernel change, no engine change. The inspector, why-trace and docs work for it immediately. That is the bar for every feature.

### 16.16 Assumed technology

The design assumes **C++** in line with the host engine, with the kernel as a standalone static library, no exceptions or RTTI in hot paths, and no third-party dependencies in the kernel. The design itself is language-agnostic.

### 16.17 Designer access

Whether designers need to author rules (not just data and formulas) is an open decision (section 21). If yes, a small sandboxed scripting layer is added at rung 3 with the same read-only view, command return, and hot reload.

---

## 17. Performance and memory

Neither CPU nor RAM is the main risk. The simulation is event-driven and memory is plentiful. The risks are complexity, tuning and debuggability, which sections 16 and 18 address. This section sets targets and measurement gates.

### 17.1 Event cost model *(estimates, to be measured)*

| Source (per game day) | Events |
|---|---|
| Agent activities and decisions (500 to 2,000 agents) | 5k to 20k |
| Work spans | 3k to 8k |
| Transfers and transactions (batched per trip) | 5k to 15k |
| Encounter outcomes (after filtering) | 3k to 10k |
| Animals (all individual, mostly dormant, group-batched) | 10k to 40k |
| Housekeeping (expiry, thought expiry, threshold crossings) | 10k to 20k |
| **Total** | **about 50k to 100k** |

At a typical time scale (one game day in 30 to 60 real minutes) that is a few dozen events per real second. During fast-forward it is roughly 10k to 20k per second.

**Kernel throughput target:** at least **1M events per second** headless on one thread. At that rate a 5-year pre-roll of about 140M events takes roughly 2 to 3 minutes. This target is a gate in the build plan.

### 17.2 Frame budget

The sim gets a per-frame millisecond budget (default 1 to 2 ms, tunable per platform). If it falls behind:

1. Far-from-player agents drop to coarser encounter ticks.
2. Path service requests are deferred.
3. **No events are skipped.** Lag becomes latency, never lost causality.

### 17.3 Threading stages

| Stage | Description | When |
|---|---|---|
| S1 | Single-threaded, time-sliced inside the host frame. Path queries optionally on workers where threads exist. | Default from day one |
| S2 | Sim on its own thread with lock-free queues in and out, host reads immutable snapshots | If profiling shows the host frame is the bottleneck |
| S3 | Separate process or server, same queues over a network transport | Only for multiplayer or large worlds |

Moving between stages is a deployment change because the boundary is already a message interface. Platforms without threads stay on S1.

### 17.4 Memory budget *(estimates, depends on world size)*

| Kind | Example count | Size each | Total |
|---|---|---|---|
| Agents (rich memory, ties) | 500 to 2,000 | 1 to 2 KB | 1 to 4 MB |
| Relationship matrix (dense) | 500 to 2,000 agents | 4 B per pair | 1 to 16 MB |
| Items (handled units) | 1 to 3 M | 24 to 32 B | 24 to 96 MB |
| Animals (individuals) | 100 to 500 k | 64 to 96 B | 6 to 48 MB |
| Plants (crops, trees) | 1 to 2 M | about 16 B | 16 to 32 MB |
| Containers, sites, stations | about 50 k | 64 to 128 B | 3 to 6 MB |
| Fields | per cell | a few values | a few MB |
| Spatial index, path caches, nav meshes | n/a | n/a | 20 to 80 MB |
| History (30 days full detail) | 50 to 100 k events/day | about 32 B | about 70 MB |

A large world lands around **150 to 350 MB** with history the biggest single item. The counts scale to a platform budget without changing rules.

### 17.5 Saves

- Chunked, columnar, binary, compressed, with dirty tracking.
- Taken at a tick boundary using copy-on-write chunks, then written on a background thread.
- Expected compressed size tens of MB *(estimate)*.
- Autosave never blocks a frame. Full saves complete in the background within a few seconds.

### 17.6 Performance gates

Gates live in the build plan. Examples: synthetic populations at **10× target counts** at K0 and N1; events per second regression tests in CI (fail on more than 10% regression); a per-rule CPU budget checked by the profiler; memory counters per table reported in the inspector.

### 17.7 Mobile and constrained targets

If a constrained target is chosen (open decision), counts scale to its memory budget, the body budget drops, encounter ticks lengthen, and the single-thread stage S1 is required. Rules and data do not change. The dormancy and lazy-value mechanisms matter more there than any storage trick, because sustained CPU and heat are the limit, not RAM.

---

## 18. Stability and anti-collapse measures

A world built from many interacting individuals can fail in ways no single rule explains. The defences are designed in, not patched later.

### 18.1 Conservation audit

Every day (and on demand), the audit checks, per good and for coin and for population:

> sum over containers + carriers in transit + consumed ledger = initial endowment + produced + imported − exported − destroyed

A drift reports **which transfer broke it** (from the ledger, by cause chain). This catches duplication and loss bugs immediately, which is crucial in a system with millions of individual items.

### 18.2 Designed stabilizers

| Stabilizer | Prevents | How |
|---|---|---|
| Parish relief and workhouse | Total famine in an untouched world | Rules-based minimum support, funded by rates, bounded by its funds. It can run dry. |
| Births and baseline arrivals | Silent depopulation of an untouched world | Household-driven births and a low steady arrival rate, bounded and logged. Too slow to undo large losses. |
| Railway trade valve | Deflation, inflation, shortage spirals | Bounded import and export with caps |
| Ecology edge valve | Species loss from natural bad years | Slow, bounded, per-species policy (fast, slow, never). Does not undo deliberate extermination. |
| Price and wage damping | Oscillation | Floors, ceilings, change limits, minimum intervals |
| Cascade depth cap | Runaway event chains | Defer overflow to the next tick, log |
| Per-pair cooldowns and per-tick caps | Encounter spam | Section 7 |
| Protected authored NPCs in pre-roll | Pre-roll killing key characters | Reduced hazard until world start |

Every valve appears as a named source or sink in the audit, with its flow tracked as a soak metric. Stabilizers protect the untouched world from natural shocks. They are deliberately too weak to undo large, lasting changes (section 12.9).

### 18.3 Soak tests

Headless runs with pass/fail thresholds. The values below are **initial thresholds** to be tuned.

| Test | Setup | Pass criteria |
|---|---|---|
| Ecology only | Climate, fields, plants, animals, no people. 50 years × 20 seeds. | No extinctions (unique monsters excluded). Max-to-min population ratio over cycles below 10. Predator-to-prey ratio within band. Valve births below 10% of total. Forage not depleted across most suitable cells for more than a year. |
| Village | 40 agents, one village, 5 years | Population at least 80% of start. Starvation deaths under 2% of population per year. Median vacancy duration under 10 days. No negative stocks. Prices within 0.5× to 3× base. Audit passes daily. |
| Full world | 500+ agents, 20 years, no player | No settlement below 50% of starting population. Unemployment under 25% sustained. Valve flows below 10% of totals. Event rate within 3× its mean. |
| Disruptions | Remove baker, miller, smith, publican or doctor. Drought year. Blight. Predator outbreak. Mill closure. | Every outcome is **valid and explicable**, not necessarily recovered. Where a qualified local candidate exists, reallocation completes within the grace period plus handover. Where none exists, the gap persists and the why-trace shows why. Deaths and starvation stay within what local stocks and relief funds explain. |
| Shock persistence | Remove every person with a given skill, burn a mill, exterminate a local species | The loss persists for the whole run unless a recorded recovery path exists (training, a valve arrival, an owner's investment). Nothing is recreated without a named source. |
| Capability audit | Any run | Counts of agents by skill level change only through training, aging, births, deaths and valve arrivals, each logged. |
| Player proxy | Bots taking random player-like actions (steal, kill, trade, hunt) for a year | Invariants hold, audit passes, no crashes, no impossible states |
| Determinism | Same seed and inputs, repeated runs | Identical state hashes |
| Save round trip | Save, load, continue vs. uninterrupted run | Identical hashes |
| Teleport | Full world run | Zero impossible position changes |
| Performance | Fixed scenario | Events per second not more than 10% below baseline |
| Upkeep health | Full world run and disruptions | Gaps per settlement per month within band. Median repair latency under twice the grace period. Fallback to outside valves under 10% of repairs. No gap open longer than 30 days unless closure is the intended outcome. |

Reports include: population by settlement, births and deaths by cause, unemployment, vacancy durations, gaps opened and repair latency, price index, food-days of stock, starvation deaths, wealth distribution, animal populations, valve flows, event rates, and per-rule CPU.

### 18.4 Complexity budgets as lints

| Item | v1 starting count | Action when exceeded |
|---|---|---|
| Needs | 4 | Lint warning, written reason |
| Goods | about 12 to 30 | Lint warning |
| Activity types | about 35 | Lint warning |
| Roles | about 30 | Lint warning |
| Event types | about 40 | Lint warning |
| Rules per event type | 8 | Lint warning |

These protect scope and testability and are not technical limits.

### 18.5 Process rules

1. **One system at a time.** The next system starts only when the previous one passes its soak test.
2. **Visibility test.** A system must have a number it reads, a number it writes, and a place where the player can see the effect. Otherwise it is cut.
3. **Always shippable.** Mixed scripted and simulated agents from the first milestone.
4. **Tooling is never cut.** The inspector, why-trace, audit, replay and soak tests are cut last, after everything else.
5. **Freeze content while tuning.** Tuning runs use a fixed content snapshot.
6. **Calibrate resolvers** whenever stats change.

### 18.6 Risk register

| Risk | Likelihood | Impact | Mitigation |
|---|---|---|---|
| Death spirals (famine, depopulation) | High | High | Stabilizers, soak tests, disruption tests |
| Heuristic oscillation (prices, wages) | High | Medium | Damping, floors and ceilings, variance metrics |
| Process-network scheduling bugs | Medium | High | Versioned tokens, local recompute, audit |
| Mesh mismatch between sim and physical | Medium | High | Bake validation, parity tests, logged fallbacks |
| Calibration drift between resolvers | Medium | Medium | Calibration harness in CI |
| Boom-bust animal cycles | High | Medium | Refuges, satiation, territories, wariness, ecology soak |
| Visible pop-in | Medium | Medium | Radius rule, fog, door and occluder spawning |
| Unreproducible bugs | High | High | Determinism, replay, checkpoints |
| Tuning swamp | High | High | Tunable profiles, experiments, soak charts, one system at a time |
| Scope creep | High | High | Budget lints, deferred list, process rules |
| NPCs feel samey | Medium | Medium | Authored key NPCs, archetype variety, personality weights, barks from state |
| Player cannot perceive the sim | Medium | High | Visibility test, signs, dialogue from state, gazette |
| Self-healing hides upstream defects | Medium | High | Repair metrics are soak thresholds, upkeep inspector, honest failure outcomes |
| Weak healing makes the untouched world decay | Medium | High | Baseline demographic processes, untouched-world soak must stay stable, resilience profile |
| Estimates wrong | High | Medium | Early measurement gates, re-plan after K1 and N1 |

---

## 19. Build plan

These estimates **supersede earlier, smaller-scope figures.** They are rough guesses for one programmer who knows the host engine, and should be re-planned after K1 and N1 when real measurements exist. Milestone U1 replaces the earlier J1: the rich hiring pipeline became optional and its time goes into the broader upkeep catalog, so the total is unchanged.

### 19.1 Milestones

| ID | Scope | Depends on | Done when (gate) | Size |
|---|---|---|---|---|
| **K0** | Kernel: tables, generational handles, scheduler with versioned tokens, time wheels, wave processing, command buffer, event bus with cause stamping, module registry with validation, lazy values, fields, binary snapshots, seeds, bridge contract with the return path, transfer ledger | none | Toy module registers, runs and saves. Synthetic 10× load meets the 1M events/s target. Round-trip save hash matches. A killed agent stays dead across save and load. | 4 to 5 wks |
| **K1** | Tooling and tests: headless runner, fixture builder, scenario test harness, why-trace, inspector, event stream with breakpoints, timelines, rule profiler, conservation audit, replay and time travel, soak runner and dashboard, tunables and profiles, generated docs, hot reload | K0 | A new programmer adds a data-only feature and a tested rule in an afternoon | 3 to 4 wks |
| **N1** | Sim mesh and physical mesh bake with validation, analytic movement, sites, doors and slots, path service and cache, departures and deadlines, groups, spatial grid, bridge: spawn, despawn, executors, materialization, blocked handling, teleport detector | K0 | 40 agents commute for 30 simulated days with zero impossible speeds and no stuck agents. Bodies spawn and despawn without visible pops. | 4 to 5 wks |
| **P1** | Items, lots, containers, transfers, spoilage and wear, stations, recipes, process engine with work spans and constraint log, farming plots, emissions | K0 | The same rules run a craftsperson, a workshop and a powered mill. Output scales with stations. Conservation holds for 20 simulated years. | 4 to 5 wks |
| **E1** | Needs, mood and thoughts, decision function with fallback ladder, activity catalog v1, households, shops with posted prices, customer choice, wages, credit | N1, P1 | Village survives 5 years. Remove the baker: prices, queues and hunger respond, and starvation follows if nothing else changes. | 4 to 5 wks |
| **C1** | Encounter detection and outcomes, noise, bounded memory, gossip, expectations and absence, place and price knowledge | N1, K1 | Rumours spread through a village, mutate and fade. An off-screen absence is noticed by the right people. | 2 to 3 wks |
| **U1** | Upkeep engine and v1 catalog (section 12): importance view, fill-position rule, ownership and household succession, dependents, dwelling, shop restock, public offices, burial, missing persons, food security, minimum roles; permanence rules, resilience profile and capability audit; slow immigration valve; apprenticeship; owner investment and contraction | E1, C1 | Kill the publican: the pub reduces hours and a qualified local replacement starts after the grace period and handover. Kill the only doctor: the loss persists and the why-trace explains it. Repair metrics stay in band in an untouched full-world soak. | 3 wks |
| **A1** | Climate and weather, fields, plants in the wild, species data, herbivores, groups, breeding and mortality, ecological LOD | K0, N1 | Ecology-only world is stable for 50 years across 20 seeds | 3 to 4 wks |
| **A2** | Predators, carcasses, scavengers, territories, hunger-driven risk, dispersal, edge valve | A1, C1 | Predator-prey cycles stay bounded, local extinctions recolonize, no species dies out | 3 to 4 wks |
| **A3** | Human-animal interaction: hunting, trapping, butchery, game law, raids, tracks and signs | A2, E1, C1 | Culling wolves produces the full cascade. Overhunting shifts prices and routes. | 3 to 4 wks |
| **T1** | Town and several villages, trade runs, standing orders, markets, outside traders at the station, migration, strikes, closures | E1, U1, N1 | A bad harvest or an ambushed cart in one village visibly shifts the others. A mill closing reshapes its region. | 3 to 4 wks |
| **A4** | Livestock, pasture, vermin, draught horses in trade, taming, unique monsters | A2, P1, T1 | Horse fodder limits carter capacity. The monster's hunts follow prey scarcity. | 3 to 4 wks |
| **G1** | Crime and law pipeline, institutions, news boards and gazette, barks from state, visible signs, funerals | C1, E1 | A player who never fights still notices the world changing. A theft goes through witness, complaint, investigation and consequence. | 3 wks |
| **W1** | Population generation, authored NPCs, scenario bundles, history pre-roll with starting snapshots, scale-up to 500+ | all | Every scenario starts in a living world that passes the full soak suite | 5 to 7 wks |

### 19.2 Totals

| Milestone span | Weeks | Calendar |
|---|---|---|
| K0 to E1 (**first playable proof**: a village where people walk to work, make things, sell them, eat, and starve when the chain breaks) | 19 to 24 | about 4.5 to 5.5 months |
| Everything | 47 to 60 | about 11 to 14 months |

With a small team, A1 to A2 can run in parallel with P1 to E1 (they need only the kernel, movement and the ledger), and tooling can run in parallel with N1. Do not assume linear speedup, because integration and tuning dominate the later steps.

### 19.3 Cut order if the schedule slips

1. Taming and training detail (A4).
2. Traps and poison (A3).
3. Strike mechanics and the gazette.
4. Depth of the relationship matrix and gossip mutation.
5. Pre-roll length.
6. Ecology beyond A1 and A2 (wild fauna only).

**Never cut:** the conservation audit, the why-trace, replay, the soak tests, the teleport detector, bake validation.

### 19.4 Review points

After K1, N1 and E1, stop and review: measured event throughput, memory per record, decision cost per agent, and whether the first proof is fun to watch. Adjust counts, budgets and the rest of the plan with real numbers.

---

## 20. Worked examples

These traces show the mechanisms interlocking. None of the outcomes is scripted.

### 20.1 The bartender is killed

Harold works as bartender at The Crooked Lamp, owned by Mrs Pike, a widow. A poacher kills him in an alley at night.

1. **Death.** The kill is validated, Harold's state becomes dead, and his body becomes a container in the alley. Nobody saw it, so nobody knows.
2. **Absence.** Harold was due at 17:00. Mrs Pike's expectation fires an `Absence` event. She opens the pub herself at reduced hours and sends the potboy to his house. His wife's expectation (home by midnight) fires too: worry thoughts, asking neighbours, a report to the constable, and the missing-person upkeep rule starts a search.
3. **Gap opens.** The position is marked "holder gone" and the upkeep rule starts its grace timer: 2 days for an unexplained absence, or 1 day (a mourning period) once the death is confirmed.
4. **The world suffers first.** The pub closes early, regulars' company and leisure needs reroute by their price and distance memory, Mrs Pike's cashbox income falls, and stock ages. `GapDetected` posts a "Barman wanted" notice at the door for the player to see.
5. **Discovery.** A woman walking her dog finds the body at dawn. `BodyFound` gives her a fact, she reports it, the constable investigates (footprints, blood, dropped items), and the burial upkeep rule assigns a gravedigger task.
6. **Repair.** When the grace period ends, Mrs Pike acts. The candidate query returns alive adults within a day's walk with skill of at least 3 who are unemployed or in a less important job. The potboy (skill 4), Harold's widow (skill 5, unemployed, needs income) and a farmhand (skill 3, long commute) qualify. The ex-soldier (skill 2) is filtered out. Scoring (skill fit, unemployed bonus, proximity, relationship, wage gain) ranks the widow first. Her own acceptance check passes because her Purpose pressure is high. She walks to the pub and starts after a notice period, working worse at first.
7. **Chain.** If the potboy had been chosen, his position would open its own gap with its own grace period, and the next repair would draw from a still less important job. The chain ends at an unemployed person or at an outsider arriving by train.
8. **If nobody qualifies.** After the grace period the wage offer rises within Mrs Pike's means and the minimum skill relaxes by one level (non-critical role). An apprentice is considered only if a master exists, otherwise Mrs Pike keeps covering. An outsider may arrive after weeks if the settlement is attractive enough. If the pub keeps losing money for the closure-patience period, it closes with lasting consequences.
9. **Recovery.** Hours return to normal. Regulars drift back as their remembered availability updates, with lag. The widow's household income stabilizes.
10. **If the player did it.** The same events fire, with witnesses by sight radius, the same investigation, and the same repair. The player is a suspect in the same pipeline.

The why-trace answers "why is the pub short-staffed?" down to the death event, and "why is Martha serving drinks?" down to the upkeep rule and her score breakdown.

### 20.2 The hunter and the werewolf

Tom Marsh is a hunter whose household pantry is nearly empty. A unique monster has its lair in Blackwood and its normal prey (deer) has thinned over two bad winters.

1. **Need.** Tom's hunger is pressing. His fallback ladder has "buy" (no money), "ask family" (declined), then "hunt". His courage and the memory of game in Blackwood make hunting the top candidate.
2. **Departure.** The decision function computes a deadline (dusk), subtracts the estimated walk, adds jitter. Tom leaves home and **walks** across fields on the sim mesh. His wife's expectation (home by dusk) is created.
3. **Hunt.** Reaching Blackwood, Tom's hunt activity runs. The monster's hunger has crossed its threshold, so it is searching its territory with a bias toward prey tracks and scent. Tom's path and the monster's path are swept each tick.
4. **Encounter.** The detector finds a closest approach inside sight radius (fog shortens it). The outcome table gives `Hostile`: the monster stalks. If the player is far away, the **abstract resolver** runs using both stat sets. If the player is nearby, the bodies are physical and combat is real. Either way the same events result.
5. **Outcomes:**
   - **Tom dies.** His body and gear are placed at a specific forest site. The monster eats. Carcass odour attracts crows and foxes. Nobody knows yet.
   - **Tom survives wounded.** He walks home injured, loses health slowly, needs a doctor, cannot work for days, may have witnessed the creature (a fact with confidence 100).
   - **Tom kills it.** A unique monster is gone forever. The village gains a hero, hunters take the forest again, and a pelt and bounty change local wealth.
6. **If Tom dies.** At dusk the expectation fires. His wife worries, asks neighbours, and a search party task appears. The player can join, or stumble on the body. **Clues** are world objects (tracks, blood, claw marks) that decay. Rumours spread with confidence and mutation. Village dread rises, hunters stay home, game prices rise. Tom's household loses its income, and the widow's own hunger ladder begins, ending at parish relief. If Tom held a position, the upkeep rule fills it after its grace period.
7. **Ecosystem.** The monster's appetite is sated for several days. As prey remains scarce, hunger returns sooner and range expands toward farms and livestock. Culling wolves in response removes a competing predator and shifts prey dynamics.

### 20.3 The mill closes

The mill employs sixty weavers. Imported cotton becomes expensive after the railway trader raises its outside price.

1. **Ledger.** The owner's ledger shows losses for three periods in a row.
2. **Contraction.** The owner cuts a shift, then lays off by seniority and favour, then closes. A `Closure` event fires.
3. **Unemployment.** Sixty households lose income. Purpose pressure rises. Their fallback ladders begin: savings, credit at shops, pawning belongings, the parish.
4. **Shops.** Customers spend less. Shops see falling sales velocity, lower posted prices, run down stock, extend credit until their own cashboxes run dry. Some close.
5. **Debts and tension.** Unpaid debts reduce trust. Evictions by landlords. Shared grievance thoughts. Organizers emerge. Theft and poaching rise.
6. **Ecology.** More poaching in estate woods pressures game. The gamekeeper's reports reach the estate. Penalties rise.
7. **Institutions.** Parish relief funds drain. The workhouse fills. Rates rise, taxing the remaining households.
8. **Migration.** Agents with sustained high Purpose pressure leave through the station. Emigration is logged. Some die in the winter.
9. **Pollution stops.** The river downstream slowly recovers. Fish return, which gives a new food source for the survivors.
10. **Opportunity.** A new owner may buy the idle mill if cash and credit allow, restart some stations, and hire at lower wages. Or the buildings decay.

The player sees empty stalls, closed shops, men idle in the street, a swollen workhouse, funerals, a quiet river, and a village that talks about nothing else. Every one of those is a read of real individual state.

### 20.4 The player removes the doctor

The village of Ashby has one doctor, Dr Whitlow, a critical, licensed position. The player kills him.

1. **Cascade.** Death event, body, household grief, and a missed surgery that fires `Absence` for the patients waiting.
2. **Gap opens.** The doctor is the sole provider of medicine in Ashby, so importance is high and the grace period is short. The parish vestry is the actor.
3. **No candidates.** Nobody local meets the licensed minimum. The midwife (skill 3) and the chemist's assistant (skill 2) are filtered out, because licensed roles never relax (P2).
4. **Fallbacks.** The parish asks the town for a visiting circuit. Its fund is small and strained, so the town doctor comes once a month, if at all. Outside recruitment for critical skills is off by default (P4).
5. **Lasting consequences through ordinary systems.** Injuries heal worse, fevers and childbirth kill more, dread and grief thoughts spread, and households who can afford it move to town (P8). The midwife and the chemist's assistant gain informal importance and treat people at lower quality. Laudanum sells more.
6. **Years later.** Nobody trains as a doctor, because no master exists. The village keeps shrinking, shops close, and the estate lowers rents to keep tenants.
7. **The player can change it back.** Escort a doctor from town, fund the parish circuit, send a villager to apprentice under a town doctor, or pay for a dispensary. Each is a normal action, and the world remembers it. If nobody acts, the village keeps its new shape.

The why-trace answers "why is there no doctor in Ashby?" down to the death event, the empty candidate query, and the capped circuit.

---

## 21. Open decisions

| # | Decision | Options | Default | Needed by |
|---|---|---|---|---|
| 1 | World size and streaming | One continuous streamed world, or areas joined by loading transitions | One continuous world, whole-world sim mesh resident. If larger than expected, sim mesh tile cache. | Before N1 |
| 2 | Platform targets | Desktop only, or also web, mobile | Desktop first. Other targets scale counts and require single-thread stage S1. | Before K0 |
| 3 | Runtime construction | Agents can build sites and stations, or buildings are authored | Authored buildings. Owners expand by activating stations and adding shifts. | Before P1 |
| 4 | Designer scripting | Designers write rules, or only data and formulas | Data and formulas only. Revisit after U1. | Before K1 |
| 5 | Player survival | Off, light, full | Light | Before E1 |
| 6 | Individual coins | Coin lots, or individual coins | Coin lots | Before P1 |
| 7 | Multiplayer | None, or later server | None. Architecture permits stage S3. | Not now |
| 8 | Host engine integration | Confirm main-thread spawn, entity system limits, AI technology for executors | Assumptions in section 3 | Before N1 |
| 9 | Disease in v1 | Include, or defer | Defer. Hooks exist. | Before E1 |
| 10 | Pacing director | None, or a parameter-nudging director | None | After A3 |
| 11 | Determinism scope | Per build and platform, or cross-platform | Per build and platform | Before K0 |
| 12 | History retention | Days of full detail | 30 game days | Before K0 |
| 13 | Body budget per platform | Number of physical bodies | Measure at N1 | At N1 |
| 14 | Language and build | C++ kernel library | C++ | Before K0 |
| 15 | Hidden-identity monster variant | Content option | Add after A4 if desired | After A4 |
| 16 | Resilience preset | Weak, Normal, Strong | Weak | Before U1 |
| 17 | May outside recruitment ever supply critical skills (doctor, priest)? | Yes (slowly), or never | Never by default. Only slow training, or the player. | Before U1 |

---

## Appendix A: Decisions log

| Decision | Alternative considered | Reason |
|---|---|---|
| Individuals are the only writers, aggregates are derived views | Pooled household and workplace stocks, daily passes, central price and labour models | Required by the design brief. Also removes a class of divergence bugs between aggregate and individual state. |
| One record per handled item and per animal, bulk goods as lots | Stacks, per-cell animal counters, herds that individuate on contact | Memory is plentiful. Individual records give provenance and ecology without a second model. |
| Continuous position for every agent on navmeshes | Place graph with teleport to schedule | Required for encounters, ambushes, trade routes and a believable world |
| Two fidelity levels (physical, abstract) | Three levels with a local-abstract middle tier | A middle tier would be a second NPC system to maintain |
| One decision function for any distance | Separate behaviours for off-screen | Guarantees the same rules everywhere |
| Scored selection with fallback ladder | Full planner (GOAP/HTN) | Planners are costly to tune and debug. Job routines handle multi-step chains. |
| Employer-driven hiring with vacancy knowledge | Central labour assignment pass | Keeps decisions individual, produces vacancy chains and gossip effects |
| Posted prices by shopkeepers with damping | Global supply and demand price function | No global function can exist if individuals are the only writers |
| Strategy swap for scripted characters | Separate scripted NPC system | One system, always shippable, story NPCs can be promoted to simulated |
| Single-threaded budgeted core with message boundary | Separate sim thread from the start | Fewer concurrency bugs, works on platforms without threads, cost is low. Boundary allows threading later. |
| Per-settlement news board plus bounded per-agent memory | Fully omniscient world, or full belief models per agent | Legible, cheap, supports gossip and error |
| Expectations and absence detection | Global notification of off-screen events | Makes the world notice by absence, which is natural and cheap |
| Mood and thoughts for dread, grief and comfort | More needs (sanity, faith) | Fewer needs, richer tone, data-driven |
| Ecosystem from individual food-web transfers | Population equations | Anything that changes births or deaths changes the ecosystem, including player actions |
| Edge valve for wildlife and bounded outside valves for the economy | Closed world | Prevents permanent collapse while staying logged and tunable |
| Tooling, tests and replay as first-class milestones | Add tooling later | Without them emergent bugs cannot be reproduced |
| Budgets as lints | Hard limits | Limits protect scope, not memory or CPU |
| No dependence on the host engine's entity system for sim data | Sim records as engine entities | Host entities are heavy, main-thread bound, and unneeded off-screen |
| Upkeep rules with grace periods and importance ordering | Full vacancy, gossip and application hiring pipeline as the default | Simple, testable, always terminates, covers most consequences. The richer pipeline stays an optional strategy. |
| Upkeep limited to structural continuity | Applying it to ecology and daily behaviour | Keeps individuals as the source of life and ecology as emergent |
| Weak, reallocation-only healing | Strong self-healing with outside recruitment on demand | The player must be able to change the world permanently, for worse or for better |

---

## Appendix B: Deferred features

Each can be added later on the same kernel through the module system.

| Feature | Hook already present |
|---|---|
| Disease and epidemics | Illness flags, thoughts, contact encounters, shared sites, water quality, doctor role |
| Fire | Site footprint adjacency, fire hazard in stations, field reset |
| Banking | Debt records, institutions, ledgers |
| Genetics for animals and people | Parent handles on records |
| Deep marriage and family dynamics | Household record, relationship flags, life events |
| Individual coins and change-making | Lots with exact quantities |
| Runtime construction | Site and station templates, ownership, builder roles |
| Pacing director | Event bus, tunables |
| Multiplayer and S3 deployment | Message boundary, deterministic core |
| Animal personalities and training depth | Species data, wildness, interaction work spans |
| Newspapers as content beyond events | Gazette items, news boards |
| Designer scripting layer | Rule interface, read-only view, command return |
| Hidden-identity monsters | Strategy swap, species data |
| Rich hiring pipeline (notices, gossip, applications) | Strategy swap on the upkeep rule, section 9.9 |

---

## Appendix C: Glossary

| Term | Meaning |
|---|---|
| **Abstract executor** | Code that resolves an activity analytically without a physical body |
| **Activity** | A data-defined action an agent can perform, with preconditions, duration, effects and executors |
| **Affordance** | What a site or object offers (sleep, food, work), indexed by tag |
| **Agent** | A person, animal or unique creature with decisions and needs |
| **Body** | An engine object representing an agent near the player |
| **Bridge** | The narrow interface between the core and the host engine |
| **Cascade depth cap** | Limit on chained events within one timestamp |
| **Command** | A validated request to change state, returned by rules |
| **Conservation audit** | Daily check that goods, coin and population only change through recorded transfers and named sources and sinks |
| **Constraint log** | Record of what ended each work span (inputs, output full, shift, wear) |
| **Core** | The simulation library with no engine dependency |
| **Decision strategy** | The pluggable function that chooses an agent's next activity |
| **Derived view** | A cached, read-only aggregate computed from individual records |
| **Event** | A typed, time-stamped record of something that happened, with a cause id |
| **Executor** | Code that makes an activity physically happen |
| **Expectation** | A record that fires an Absence event if a condition is unmet by a deadline |
| **Fact** | A bounded memory entry with source and confidence |
| **Field** | A grid of environmental values (forage, fertility, pollution) stored lazily |
| **Gap** | A detected failure of a healthy condition (empty position, missing head, low stock) awaiting repair |
| **Grace period** | Time the world waits, and visibly suffers, before an upkeep rule acts |
| **Group** | A coordination structure for herds, packs, families and caravans |
| **Handle** | A typed 32-bit generational reference to a record |
| **Handover** | Notice, travel and settling-in time between a repair decision and the repair taking effect |
| **Hot and cold fields** | Frequently scanned vs. rarely used parts of a record, stored separately |
| **Importance** | A tunable value for positions, derived from base value and context, used to order candidates and set grace periods |
| **Kernel** | The game-agnostic part of the core: storage, scheduling, navigation, saves, tooling |
| **Lazy value** | A value stored as (value, timestamp, rate), computed on demand |
| **Lot** | A single record for a bulk quantity (sack of grain, purse of coin) |
| **Materialization** | Creating a physical body for an abstract agent |
| **Module** | A unit that registers record types, events, rules, activities and tunables |
| **Physical layer** | The engine side: bodies, animation, local steering, real combat |
| **Position (job)** | A role at a site held by an agent, with wage, hours and requirements |
| **Promotion** | Making an abstract agent physical early because it is about to perceive a physical one |
| **Resilience profile** | The tunable set that decides how strongly upkeep rules heal the world (weak, normal, strong) |
| **Rule** | A function reacting to an event, returning commands |
| **Site** | A place that matters: house, pub, field, well, market, station |
| **Soak test** | A long headless run with pass/fail thresholds |
| **Station** | A workplace record with throughput, operators, wear and recipes |
| **Thought** | A data-defined mood effect with duration and stacking |
| **Time wheel** | Bucketed expiry list that costs work only when items actually expire |
| **Token** | A versioned handle to a scheduled event, used for safe cancellation |
| **Transfer** | The only way items and coin change container |
| **Upkeep rule** | A data-defined rule that detects a gap and repairs it through an actor, candidates, acceptance and fallbacks |
| **Valve** | A bounded, logged source or sink at the world's edge |
| **Vacancy chain** | Sequence of positions opened when one hire vacates another |
| **Why-trace** | Walk from any state back through causes and rules |
| **Work span** | A bounded interval of production between scheduling events |
