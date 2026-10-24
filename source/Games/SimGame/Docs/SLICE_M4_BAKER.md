# Slice contract: M4 the baker (a village you can play in)

Status: **approved, in progress** (2026-10-24; step 1 started). **Revised after the layout choice (13.6 to 13.9): one big open level with three villages, each a prefab; where section 3 or 10 differ, the revision paragraph at the top of section 3 wins.** The owner's answers to the first questions are in section 13 and are folded in below; this is ready for approval. Sources: design document 6.2 (needs), 6.6 (decision), 6.7 (activities),
8 (items), 9.3 to 9.9 (shops, wages, filling positions), 11.3 (death cascade), 12.3 (fill a position); addendum 8, 9, 14 (M4), 17.4; SIM_CONTRACT sections 9 to 13.

This is the design's own first proof ("remove the baker: prices, queues and hunger respond"), built **thin, and played in first person**, so that the sim stops being a tech
demo. It merges the addendum's M3 leftovers (calendar, ledger, conservation audit, why-trace on real rules) and M4 (needs, activities, items, shops, upkeep) with the
first player verbs and a real village map. Everything not needed to make that loop playable is out (section 2).

## 1. The loop the slice has to make true (acceptance story)

A player starts in the village at dawn and can do this without a console:

1. Walk down the street. Residents leave their houses, walk to work, buy bread, go home; the bakery has a queue at the counter at 8 and none at 11.
2. Read the clock (HUD) and see the day pass. Wait in place or sleep to fast-forward; the world keeps living, and speed returns to normal when something that matters happens.
3. Enter the bakery. Its shelf shows the loaves the sim says are there. Press **F** at the counter: "Buy a loaf (2d)". The coin leaves the purse, the loaf is in the pockets, the baker's till has the coin.
4. Eat the loaf from the inventory; hunger (if the player has it, 13.1) falls.
5. Take a loaf from the shelf without paying. If the baker or anyone else sees it, the shop refuses service for three days; if nobody saw, nothing happens and the bread is the player's. (A why-trace explains either.)
6. Kill the baker (debug command in this slice; real combat later). Her body lies where she died with her belongings. The shop does not open next morning, the queue goes to the miller's door, prices at the miller rise, hunger rises across the village over days.
7. After the grace period a qualified unemployed resident (the widow) walks in, takes over the oven (with a short learning penalty), and the bakery opens again. The player can ask the console `sim_why` for each step. Nothing was scripted.

Each step maps to a sub-slice (section 12); the whole is the gate for the milestone.

## 2. Scope

**In:** one village level (map), calendar and clock, hunger and rest, items / lots / containers / transfers / ledger, a conservation audit, activities as step lists with a scored
decision function and a food fallback ladder, positions and shifts, a three-link chain (grain valve, mill, bakery), a shop with a posted price, wages from named valves, a minimal
death cascade (body container, vacancy, fill-position upkeep), first player verbs (interact prompt, buy, take, eat, wait/sleep, inventory, HUD), minimal witnessing, tools and scenario tests.

**Out (not in this slice, each later or never):** company and purpose needs, mood and thoughts, memory and gossip, law, institutions and courts, crime beyond the shop ban, ecology,
trains, blood moon, population generation (residents are authored), health and injury detail (health is one value), combat (only `sim_kill`), real dialogue (a name label and a short bark
line), weather, seasons, farming simulation (fields are a site where people work for a valve wage), a second shop or competition (the miller's flour is the only price besides bread).

## 3. The village map (levels are authored; I build with the map editor)

**Revision (owner, 13.6 to 13.9).** The world of this slice is **one big flat open level**, `vale.map.json` / `vale.bsp` (about 560 x 460 m, forest belts between the villages, dirt roads joining their gates, a tall tree
wall and invisible clip at its edge; one `sim_portal` pair to `sim_test_road.bsp` at a road end), the new layout root and startup map. It holds **three villages of about 100 x 100 m, each its own prefab**
(`prefabs/village_a.map.json` ribbon street, `village_b.map.json` green and ring, `village_c.map.json` crossroads with a church on a terrace; the home village is **C**), placed as instances so that editing one village is
local to its file. **Interiors are never separate levels**: every house, the bakery, the mill and the tavern are **nested prefabs** (`prefabs/buildings/*.map.json`) with their interior in the same level, and props are
prefabs again (`prefabs/props/*.map.json`). Sim entities inside prefabs take their ids from the **resolved `targetname`** (the editor joins the instance names with `|`, e.g. `village_c|bakery|counter`), and
`site` keys are editor **references** (type `target` in `entities.ent`), so a prefab used twice never produces a duplicate id and ids stay stable while instance names stay stable. `sim_spawn` and `sim_portal`
stay in the root map (their names must not be prefixed). **All three villages are populated** (about 16 residents in 8 households each, about 48 in all, authored data; each village has its own baker, miller, grain valve and
price: no trade between villages in this slice). The player wakes in a **room at the tavern of village C**.

- **Level** `village.bsp` (+ `.map.json`), the new layout root and startup map of `world.manifest`; one `sim_portal` pair to `sim_test_road.bsp` (a, road, b, c stay as "elsewhere" and as
  transition and nav tests). About **100 x 100 m** (owner): the whole village is within the despawn radius from its middle, and the ring (45 / 65 / 80 m) only matters at its edges and
  in the field; the radii stay as they are, a bigger world will use them fully. **Real human scale** (door 2.1 x 1.0 m, storey 2.8 m, street 5 m): the first-person game, not the
  Slasher-scale (1.3 to 1.5 x) of the level-blockout skill.
- **Sites** (`sim_site` brush, `sim_door` points, `sim_slot` points, `sim_container` points, keys in `entities.ent`): bakery (shop front with counter slot, oven slot, bake room, flour store
  container, till container, display shelf containers), mill (grain bin, flour bin, millstone slot), six to eight houses (bed slot, table slot, pantry container, purse container), a
  tavern and a church as closed backdrop (no interior; `sim_site` with `playable=0`), a well / green, the field (outskirts site, work slots), the village edge `sim_portal`.
- **Interiors only where the loop needs them**: bakery, mill, two houses the player can walk into at once, the rest closed shells (lesson L6 of the map editor project: a level is a path
  through a setting, not a pile of rooms). A sleeping resident is not shown: bodies are not proposed for agents inside a house the player is not in (section 10).
- **Pipeline**: I write `.map.json` and the prefabs through the editor (Core + Shell at `F:\Git\mapeditor`, its `level-blockout` skill and building generators, real-scale parameters), take pictures,
  you judge taste; 2 or 3 layout variants first, then only what you ask changed (editor lessons L1 to L8). Export `.bsp` with `bsp_export_tool`, then `-sim_bake_nav` (the bake validates
  every portal, spawn and site door; a door or slot off the mesh fails the bake naming the entity). Props are the **level editor's prefabs** (owner, 13.5): a prefab is an ordinary map file of brushes and entities (editor design 5.4), so shelves, counters, the oven, crates, barrels, benches, beds,
  tables, lamp posts, fences and the like are `.map.json` files in `GameData/prefabs/` (the editor's `prefabRoot`; the folder does not exist yet), built with the editor's brush tools and shape helpers
  and placed as instances. They are baked with the level like any geometry. I author them, you judge them from pictures. What this gives and what it does not: prefabs are brush geometry (boxes,
  convex hulls, cylinders and cones through the kit's `hull` / `cone` helpers), so a **loaf, a sack, a purse are blocky and low-detail by construction**; that is a visual ceiling of this slice, accepted
  with the choice, and a model-based replacement later does not change any contract here (the sim only knows item types and container slots).
- **Authoring keys** (extend `entities.ent` where missing): `sim_site` {siteId, kind, displayName, capacity, `owner` (a resident id), `playable`}, `sim_door` {site, door, lock},
  `sim_slot` {site, slotType (counter, oven, bed, table, stool, millstone, field), facing}, `sim_container` {containerId, site, containerType (shelf, pantry, till, bin, purse), access
  (public, household, staff, owner), capacity, `visible` (shows its contents through `sim_item_prop` entities of the same container)}, `sim_item_prop` brush entity {container, slot, itemType} (shown while the container's slot holds that type, section 5). Stable ids are mandatory (addendum 9.2); the bake and the loader reject duplicates.

## 4. Calendar and clock (`Sim/Calendar`)

- Scale (owner, settled): **0.5 game minutes per world second**, a game hour is 2 minutes, a day 48 minutes. `Calendar` derives day, hour, minute from the tick (no stored time of day).
  Start: day 1, 05:30, a Monday. Sunrise 06:00 and sunset 18:00 are constants of the slice (daylight flag for rules; no seasons).
- Events: `HourStruck {hour}` (EventSignificant), `DayStarted`; activities and shifts are scheduled in game time through helpers (`Calendar::TickOf(day, hour, minute)`), never by tick arithmetic
  in rules.
- **Time controls for the player** (the first use of fast-forward and wake events): `Wait {hours}` and `Sleep` (until morning, or 8 game hours) set `Host::SetSpeed` (a step of 50 ticks, 12
  game minutes per step at x10 or more); the speed returns to 1 and `Woke` is sent on a wake event: the player is addressed, an agent next to the player starts a fight or shouts, a
  need of the player goes critical, a body appears in sight. List in `calendar.wake_events` (addendum O3). Not available while an agent is hostile in sight.
- **Visuals of time (owner's decision, 13.2): baked light styles and a blended sky.** The editor and the engine already bake and play up to 8 light-style layers per surface (`light_*`
  entities with a `style` key, lightmap layers and light-volume probes per style, `GetStyleColor(style)` multiplies each layer every frame). What is missing is who decides a style's value: today it is a
  built-in pattern table that loops at 10 Hz. Two small, additive engine changes (both listed in SIM_CONTRACT section 1 when done):
  - **E1: a style provider.** `CQuake3BSP::GetStyleColor` first asks a game-installed function `bool(uint8_t style, glm::vec3& colour)`; when it answers, that colour (RGB, may exceed 1) is the
    style's value, otherwise the built-in pattern. SimGame installs one that maps the **game hour** to the colours of its styles, with smooth curves so styles blend (one function, tunables in data).
    Characters follow for free: their light-volume probes sum the same style layers. No change to the baker, the BSP format or the renderer.
  - **E2: a sky that follows time.** The map disables the automatic sky (`_sky_auto` off); the game draws a **sky sphere** (a model loaded from GameData) with a custom shader that samples two or three
    sky cubemaps (day, dusk, night) and blends them by the same hour curves, with the sun / moon disc and stars in the night cubemap. Shader compiled with the repo's
    `ShaderCompilationAutomation.exe` (not a copy of it). How the engine draws the map's sky today is read first, when this step starts.
  - **The village is baked with these styles** (light entities carry the id; the editor's own style analysis picks the layers per face): style 1 **sun** (a `light_directional` plus `light_sky` for
    the day sky fill), style 2 **dusk glow** (a low, warm, orange directional from the west and a warm sky fill), style 3 **moon** (a cold, dim directional and sky fill), style 4 **street and window lamps**
    (point lights at the lamp posts and in the lit windows: on from dusk to dawn, with a window flicker later). Base style 0 holds only what never changes (small constant ambient, interior fill).
    That is four styles of the 8 slots a surface may carry (base + up to 7), so a face can take all of them.
  - **Hours -> values** (default curves, data in `GameData/sim/daylight.json`): sun rises 05:30 to 07:00 and sets 17:00 to 18:30; dusk glow peaks at 18:00 and 06:00 (shorter); moon is up from 20:00 to 04:30
    with a slow fade either side; lamps switch on 17:30 to 18:15 and off 05:45 to 06:30. Each style's value is the product of its curve and a colour (so dusk is orange, noon is white).
  - **Not the sim's business**: the provider reads the calendar's hour as the game side sees it (`Calendar` snapshot in the client); the sim's own `Daylight` flag (sunrise / sunset constants) is for rules.
    Console `sim_time <hour>` sets the clock for pictures and `-sim_hour <h>` launches at that hour.

## 5. Items, containers, transfers, ledger (`Sim/Items`)

- **Types** are data (`GameData/sim/items.json`): `bread_loaf` (food, 0.9 nutrition, value 2, stale after 2 game days), `flour` (lot, sack), `grain` (lot), `coin` (lot, in pence), and a
  `pocket_watch`-style trinket is *not* included. Quality bands: fresh / stale.
- **Records**: `Container {type, owner, site, level, access, capacity, locked}` (a component on a sim entity), `Item {type, quality, expiry}` and `Lot {type, quantity}` as entities whose
  `Location` points at a container. Owners are agent, household or site. A container with `map id` is **map-backed** (placed by `sim_container`, loaded at start from the world file the bake
  extends with a container list); container-less items do not exist.
- **Transfers are the only way an item changes container** (`Transfer {what, quantity, from, to, reason, cause}` command; reasons take, give, buy, sell, steal, produce, consume, spoil, drop,
  loot, inherit, destroy). The ledger keeps recent transfers at full detail (ring) and compacts older ones to per-day per-reason totals.
- **Conservation audit** (never cut): `ConservationAudit(world)` sums every lot and item by type, and compares against the running totals of named **sources and sinks**: `valve.farm_grain`
  (source of grain, sink of coin), `valve.estate_wages` (source of coin for work at the field), `consume` (sink), `spoil` (sink), `produce` (source, consumes inputs by recipe), `destroy`.
  Run after every step of the soak and as a kernel invariant in tests; a mismatch fails and names the type and the last transfers.
- **Game side, item props are prefab brush entities that are shown and hidden** (no runtime model, no runtime spawning): a prefab such as `prefabs/shelf_bread.map.json` carries the shelf **and** one
  `sim_item_prop` brush entity per place a thing can lie (a loaf-shaped hull on each shelf position), each with `container` (the `sim_container` id), `slot` (0..n) and `itemType`
  (`bread_loaf`). The game asks the sim for the contents of the loaded level (`ContainerContents` output event on load and on change, `SimContainerView` bridge code) and **shows the prop of slot k when the
  container's k-th slot holds that type, hides it otherwise** (hidden = not drawn, no collision, not interactable). A container therefore shows at most as many items as it has props: its `capacity` is the
  prop count, and the bake / the loader reject a container whose capacity exceeds its props when `visible = 1`. Bulk lots (flour in the store, coin in a till) have a fill-level prop set (empty, third, full) by
  the same mechanism. To verify when step 3 starts, before it is relied on: that a brush entity's drawables can be hidden and its collision disabled at runtime in the engine (SimPortal already reads its
  `BSPModelRef` drawables); if not, the fallback is one tiny engine addition (a `SetVisible` on brush entities), listed in SIM_CONTRACT section 1. Dropped items go to a ground-pile container with a
  position; picked up the same way. No optimistic client changes in this slice (latency is measured in the in-game test, expected under 0.3 s, 6.1).

## 6. Needs (`Sim/Needs`, data `GameData/sim/needs.json`)

- **Hunger** and **rest**, as in the design 6.2 (rises ~20 game hours from fed to critical awake, slower asleep; rest ~16 hours). Each is a `LazyValue`; band crossings (fine, pressing,
  critical) are scheduled events `NeedBand {agent, need, band}` (nothing polls). Pressure curves per band from data.
- **Consequences in this slice**: critical hunger lowers work speed (a work shift produces less) and, after a grace of `needs.starve_after` (default 36 game hours), health falls linearly to
  zero (**one** health value; `Death {cause: starvation}` when zero). Rest: critical rest forces sleep at home (or where the agent stands, if away) for 4 hours.
- **The player has them too, "light" (owner, 13.1), with Kingdom Come: Deliverance as the reference.** Same needs and curves as a resident, with KCD's shape: food has stages (**fed**, **peckish**,
  **hungry**, **starving**), over-eating on a full stomach has a cost (a stage above fed, "full", where more food gives a small penalty), sleep and rest have stages (**rested**, **tired**,
  **exhausted**), sleeping on a bed rests faster than on the ground, food spoils (the loaf goes stale in 2 game days, the price of a stale one is lower, eating it gives less). Each stage is a row of
  data (`GameData/sim/needs.json`: threshold, effects, text for the HUD). **Light** means the effects that this game has systems for: in this slice hungry and exhausted lower the run speed and
  the HUD shows the stage, starving and exhausted add a screen tint; there is **no starvation death** for the player (the player stays at the starving stage). KCD's stamina, health regeneration
  and skill effects are rows that stay empty until those systems exist, so the table is already the right shape. Residents run the same stage table with real consequences (starvation, 36 game hours).

## 7. Activities and the decision function (`Sim/Activities`, data `GameData/sim/activities.json`)

- **An activity is a list of primitive steps**: `Travel(target)`, `Enter(site)`, `UseSlot(kind, duration)`, `Transfer(spec)`, `Wait(condition)`, `Exit(site)`. Executors exist per
  primitive: abstract (sim-only, resolves in one event at the computed time) and physical (the body walks to the slot, the animation plays; section 10). A body appearing mid-activity
  resumes the current step (addendum 8). Interrupts cancel at step boundaries.
- **v1 catalog** (rows of data, about ten): `sleep` (home bed slot, night), `eat_at_home` (pantry has food), `buy_bread` (travel to the bakery, queue at the counter slot, buy, leave), `work_shift`
  (travel to the position's site, stay at the slot for the shift), `bake` (oven slot, recipe), `mill` (millstone slot, recipe), `field_work` (field slot, wage from the valve), `serve` (counter slot while
  the shop is open: this is the baker's activity during opening hours), `idle_at_home`, `loiter_on_green`. The existing `routine` strategy (npcs.json) stays for the authored routine NPCs.
- **Decision** (`strategy = needs`): triggers exactly as design 6.6 (activity ended, need changed band, woke, arrived, shift approaching, jittered idle timer), minimum decision interval 5 game
  minutes, candidate set capped at 24 from **what the agent knows** (in this slice: the home, the workplace and every site of the village), preconditions (stock, open, not full, reachable),
  score = sum of pressure x relief + obligation + preference - cost (travel time, price), seeded noise, commitment bonus. The chosen activity and its **reason code** are logged for the why-trace.
- **Food fallback ladder** (design 6.6, cut to what exists): eat at home from the household pantry, buy bread (if the shop is open and has stock and the agent has coin), buy flour and bake at
  home is *not* in the slice, borrow from kin (a transfer from another household's pantry if the agent is a relative and it has a surplus), starve. `steal` is in the ladder data but gated off
  for every resident (honesty 1.0) so the theft test can switch it on.
- **Obligations**: a shift is an obligation with a start time; pressure grows as it nears and missing it fires `ShiftMissed` (used by upkeep).

## 8. Work, production, the shop, upkeep (`Sim/Work`, `Sim/Economy`)

- **Positions** (data `GameData/sim/positions.json`): `{ id, site, role, importance, minSkill, hours, wage | owner, slot }`: baker (owner-operated, importance high), miller (owner), baker's helper
  (wage, hours, vacancy at start, filled by upkeep), field hands (wage from `valve.estate_wages`). A resident holds at most one position; skills are a small table per resident
  (`baking`, `milling`, `labour` 0 to 1) from the resident data.
- **Recipes** (data `GameData/sim/recipes.json`): `mill: grain 10 -> flour 8, 1 game hour at the millstone slot`; `bake: flour 4 -> bread_loaf 12, 2 game hours at the oven slot`. Output scales with
  the holder's skill band (a learning penalty after a handover: the first week at 70 percent). Production is a **work span** (start, duration, produces at the end), not a polled loop.
- **Grain valve**: the farm delivers `valve.farm_grain` sacks to the mill's grain bin at 06:00 every day up to `economy.valve_grain_per_day`; the miller pays for it in coin out of his purse (a named
  sink). A delivery missed (a miller dead) is lost, not queued.
- **Shop**: the bakery is a site with a display container, a till, a counter slot, opening hours (08:00 to 17:00, six days; closed Sunday), and a **posted price** with the design's damped rule (9.3):
  at most 5 percent per day, floor cost basis x 1.15, ceiling base value x 2, stock pressure and sales-velocity pressure. `Stockout` (display empty while open) is an event residents can
  remember (their price memory is a bread price only). The miller sells flour to the baker by the same rule; both are transfers with an event (9.5).
- **Upkeep: fill a position** (design 12.3, exactly): holder confirmed dead -> grace 1 game day (important role); a missed shift 2 days; the employer (the owner, else the next in the position's list) picks
  among adults who can reach the site, meet the minimum skill and are **unemployed or in a strictly less important position**; scored, can refuse, with a walk and handover. Fallbacks in order:
  raise the offer within means, take an apprentice (none in the slice), owner covers (none), ask outside (not in the slice), `Closure`. The default candidate in the shipped cast is the widow.
- **Death cascade (minimal, design 11.3)**: state dead, the body is a container with the belongings at the place of death, `Death {cause, actor}` event, household head moves up, the position is marked
  holder-gone, anyone who sees the body (an agent that perceives it) generates a `BodyFound` event; the rest (grief, inheritance beyond "the household keeps it", funerals, scavengers) is out.

## 9. Player verbs and interface (game side, `Sim/Bridge/SimPlayer*`, UI in RmlUi)

- **The player in the sim**: an agent with `strategy = player` (design 6.9) created at the first player report, holding `pocket` and `purse` containers (start: 6 pence, no items); the pawn's
  existing report keeps its place. Verbs are **commands the same as any agent's** (`Transfer`, `StartActivity`), sent as input events and answered by an output `ActionResult {requestId, ok, reason}`.
- **Interaction** (`interact`, F, already registered): the pawn looks along the view for the nearest interactable (container prop, counter slot, door, body) within 2 m with a line of sight, shows a
  prompt at the screen centre from a **context card** the sim answers (`Context {target}` -> text, verbs, price, whether it is theft), holding F or pressing a second key selects the alternative. The
  game side is a thin `IInteractive`-style interface (ShooterGame's `InteractionSystem/IInteractive.h` is the shape; written here, SimGame does not link ShooterGame).
- **Verbs**: `Buy` (counter, display item, price; refuses with a reason: closed, out of stock, banned, no coin), `Take` (from a container the player has access to; **a take from a container the player
  has no access to is a `steal` transfer** and the prompt says so), `Eat` (from pockets), `Drop`, `Wait` / `Sleep` (section 4), `Open door` (engine `func_door`, level-owned). No dialogue: looking at a resident shows a
  name label and, on F, a one-line bark from state ("Bakery's shut today.", chosen from a short table by the resident's activity).
- **HUD** (`GameData/ui/hud.rml`): clock and date, a hunger and rest bar (survival mode), purse, a pockets list (type, count), the interaction prompt, a small log line ("Bought a loaf for 2d").
- **Witnessing (minimal)**: when a `steal` transfer happens, every awake agent that is in the same site, or within 12 m outside with a clear line (a sim query against the physical layer: bodies are
  the observers, so a stranger far away who has no body does not see), emits `Witnessed {thief}`. The owner of the container (or the shop's current holder) applies the **shop ban** (`service_refused_until
  = now + 3 game days`) and the agent walks over and says so. Nothing else follows (no law, no guard reaction). Unwitnessed: no event, and `sim_why` can show the theft to a developer only.
- **`sim_kill <id>`** (console, development): the `Kill` command with the player as actor, for the loop in section 1; real attack verbs come with combat.

## 10. Bodies in sites (changes to `SimAgents` / `CharacterBody` drivers)

- An agent inside a site is counted by occupancy, but when the player is in the **same level within the visible radius and the site is public or the player is in it**, the agent gets a body at its
  slot position (`sim_slot` facing) and plays the slot's pose (`counter`, `oven`, `stool` = `sitting_idle`, `table`, `field` = idle). Sleeping or private-site agents get no body unless the player is inside
  that site. The ring, warm-up and fade rules of M2f apply unchanged.
- Doors: a body walks through a `sim_door` point (open or closed `func_door` is not simulated; agents pass doors like portals inside the level, one second each way, no door animation in the slice).
- The sim keeps its walk-in budget (32 bodies) and the ring (45 / 65 / 80 m); the village's ~16 residents are well under it, so in practice every resident that is outside has a body once the player is within 65 m.

## 11. Tools and tests (written first, addendum 17.4)

- **Text tools** for this slice (`Server::RunTextCommand` additions, also console `sim_text`): `why <event>` (exists), `who <agent>` (needs, activity, reason, position, purse, pantry), `site <id>`
  (occupancy, stock, price, holder), `ledger [n]`, `audit`, `prices` (series of the last 20 days), `day` (the village's day as a timeline: who did what), `kill <agent>` (as the player).
- **Scenario tests** (headless, in `-sim_selftest`, on the shipped village world file and data): *an ordinary day* (the baker bakes at 06:00, the shop opens at 08:00, at least five distinct buyers
  buy bread, nobody is hungry by evening); *a thirty-day soak* (no agent starves, price stays in its band, coin and goods audit clean, no violation, replay identical, save and load mid-day identical); *the baker
  dies* (shop closed next day, `Stockout` memory, price at the miller rises within three days, the widow takes over after the grace period and the bakery opens again within five days, why-trace names each
  step); *the miller dies* (no flour: the bakery stops after its stock, the village hungers, the upkeep fills the mill or the chain closes after the patience period); *theft* (a witnessed steal sets the
  ban; unwitnessed does not).
- **In game** (`-sim_village_test`, headless like the others, report `SaveData/sim_village.log`): the pawn walks into the bakery with the sim paused at 08:05, buys a loaf (coin and loaf move, latency recorded),
  eats it, takes a loaf unseen (nobody inside), waits to evening at x20 with a wake on a test event, kills the baker with `Kill`, fast-forwards three days and reads the state through the text tools.
- **Looked at** with pictures: the village from three viewpoints, the bakery interior with the counter and shelf, a queue at 08:15, an empty shop, the replacement baker at the oven. Not judged by me: feel and
  taste (you), the layout and proportions (you, from my pictures and your own walk).

## 12. Build order (each step is playable or verifiable on its own; steps 1, 2 and 2b are independent of each other, except that 2b's village pictures need 1)

| Step | Content | Gate |
|---|---|---|
| 1 Map | Village layout variants, then the chosen one with sites, doors, slots, containers, props, bake; the four light styles authored in the editor | Two or three pictures to choose from; the bake passes; NPCs of the old test data can walk the street. **Status 2026-10-24: layout chosen (all three, home C); prefab set, three villages and the vale built, exported, baked, nav bake passes; owner review of the pictures pending; the four light styles are not authored yet (step 2b); the old test NPCs are bound to the test levels, not walking the vale** |
| 2 Time | Calendar, `HourStruck`, clock HUD, Wait / Sleep with wake events, the `day` tool | Waiting a day at speed passes in seconds, wakes on a test event; replay and save identical |
| 2b Light of the day | E1 style provider, E2 sky sphere and shader, the daylight curves, `sim_time` / `-sim_hour`; first on a test box map with the four styles, then on the village | Pictures at 05:00, 06:30, 12:00, 18:00, 19:00, 23:00 show a believable day without a pop at any hour; the characters' light follows; lamps come on at dusk |
| 3 Items | Types, containers (map-backed), transfers, ledger, audit, container views and props, `Take` / `Drop` / `Eat`, inventory HUD | The player picks up a prop loaf and eats it; the audit stays clean over a 30-day toy run |
| 4 Needs and activities | Needs, step lists, decision function, `who`, data files; residents (strategy needs) at home, sleeping, eating | A day of residents with nobody hungry and no teleport; the why-trace explains any choice |
| 5 Work | Positions, recipes, valves, mill, bakery, shop price rule, `serve`, queue, `Buy`, interaction prompts | Ordinary-day scenario passes; the player buys bread; the soak passes |
| 6 Consequences | Death cascade, `sim_kill`, vacancy and fill-position, theft and witnessing, the shop ban | The three consequence scenarios pass; section 1 step 6 and 7 happen on screen |
| 7 Review | Measured tick cost per tier, memory per record, whether the village is fun to watch (design 19.4 review point) | You play it; we adjust counts and rates from numbers before M5 |

## 13. Decisions (owner, 2026-10-24)

1. **Player survival**: *light*, with Kingdom Come: Deliverance's survival model as the reference (section 6): staged hunger and rest, a cost for over-eating, spoilage, bed versus ground; this slice implements
   the effects the game has systems for (run speed, HUD stage, tint) and no starvation death for the player.
2. **Time of day**: baked light **styles** (2 to 4 variants, here four: sun, dusk, moon, lamps) blended through a game-installed **style provider** (E1), and a **sky** by disabling the automatic sky and drawing
   a sky sphere with a shader that blends cubemaps (E2) (section 4). Both are engine changes and are approved by this decision; each is additive and listed in SIM_CONTRACT section 1 when made.
3. **Theft**: a witnessed theft gets the three-day shop ban and nothing else (no guards, no law).
4. **Village**: about 100 x 100 m (smaller than my first proposal), real human scale, Victorian-gothic hamlet, the new layout root and startup map.
5. **Props**: the level editor's **prefabs** (brush geometry, section 3 and 5), not Blender models; item props are prefab brush entities shown and hidden from the sim's container contents (section 5).

6. **Villages (2026-10-24)**: all three layout variants are built, on one big open map with forests between them, **each village a prefab** to localise editing. Home village **C** (crossroads, church on a terrace).
7. **Population**: all three villages populated now (about 48 residents), sites and loops in each.
8. **Interiors**: not separate levels; houses and buildings are prefabs placed in the one level.
9. **Player start**: a room at the tavern of village C (no household, no pantry: food comes from the shop).
Still open and not blocking until step 2b: **the sky art**. Three cubemaps are needed (day, dusk, night with stars). Options: derive them from the panorama already in GameData (`env/skybox1_cube.png`,
a stormy red sky) by grading, generate them procedurally in the shader (gradient + clouds noise + stars), or you supply panoramas. I will propose the procedural one first (it needs no assets and blends
exactly), and you judge it from pictures.

Smaller defaults I will use unless you object: money in pence (loaf 2d, a day's wage 8d), residents about 16 in 8 households (existing names keep: Edith Marsh the baker, Gideon Wray the miller, Mrs. Aldous the
widow, Tom Pell, Sergeant Pike, Warden Harlan, Old Barnaby, Hob Tanner), one vacancy at start (the baker's helper), shop closed on Sunday, `sim_kill` as the only attack.
