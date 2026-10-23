# Slice contract: M2a abstract movement

Status: **approved and implemented** (2026-10-23). The code is in `Sim/Movement/`; SIM_CONTRACT section 8 is the reference for
what exists. Where the code differs from the proposal below, section 11 says how.
Follows addendum section 17.4 (short contract per slice, tests first). Sources: addendum 6.2, 6.3, 7.1; design 5.4, 5.7.
The addendum wins over design 5.3 (analytic lookup): walkers are **integrated** along their path every step, per tier.

## 1. Goal and done-when

Agents walk the world mesh continuously, in every level, at the cost of a polyline step. Done when the self tests show:

- 40 walkers commute across all four test levels (6 portals) for 30 minutes of world time with **zero teleport-detector
  violations**, identical state hashes on a second run, identical results after save and load in the middle of the walk;
- arrival time of every walker equals `path length / speed` (+ portal delays) within one tick, at T0, T1 and T2 steps and
  under fast-forward steps;
- tier changes follow the player across a level change.

Out of scope (later slices): bodies and materialization, the abstract-versus-physical calibration, the bridge entity and
the thread/PVS handshake (SIM_CONTRACT step 4), encounters, groups, doors, mover classes other than a person.

## 2. Files

```
Sim/Movement/
  SimSpeedTable.h        shared speed table: mover class x gait x area -> m/s (header only, no sim types; the physical
                         layer includes it later, so both sides read the same numbers)
  SimMovement.h/.cpp     MovementModule(WorldNav&): components, events, commands, tunables, systems, invariant
  SimWalk.h/.cpp         pure path walking: Advance(path state, seconds) and PositionAt; no World, tested alone
Tests/SimMovementTests.cpp
```

## 3. Data

**Components** (saved, hashed, dumped):

| Component | Fields |
|---|---|
| `Position` | `glm::vec3 nav` (metres, nav space). Written only through `movement::Teleport` / the integrator (section 7) |
| `Walker` | `state` (Idle, WaitingForPath, Walking, Crossing); `gait`; `speedScale` (per agent, 1.0 default); `destination` (nav); `requestedAt` (tick); `corners`, `cornerLevels` (the path, nav space); `nextCorner`; `lastIntegrated` (tick); `crossingUntil` (tick); `from`/`to`/`ticks` of the last integration (for the detector) |
| `PlayerAnchor` | `level` (index in the world file), `nav` position. One entity; created by the first `PlayerMoved` input. The player's level is told by the game, not derived |

No agent stores a level id (addendum 4.5): a walker's level is `cornerLevels` of its current leg; the player's comes from the
game.

**Events**

| Event | Flags | When |
|---|---|---|
| `PlayerMoved {level, local position}` | Input | the game reports the player (bridge, every 0.1 s or per metre moved) |
| `PathAssigned {walker, length, corners, portals}` | | a path request was solved and the walker starts |
| `PathFailed {walker, reason}` | | invalid destination, destination unreachable (a partial path is a failure, nobody walks to the "closest point"), stale walker |
| `PortalCrossed {walker, fromLevel, toLevel}` | | the walker took an off-mesh link; also a `PositionException` |
| `Arrived {walker, at}` | Significant | the walker reached the destination; `at` is the exact tick (matters when a long step covers it) |
| `PositionException {entity, from, to, reason}` | Significant | the only way a position changes other than by path progress: `placed`, `portal`, (later `respawn`, supernatural) |

**Commands** (rules emit them; apply functions check preconditions and `Reject` otherwise)

| Command | Effect |
|---|---|
| `RequestWalk {walker, destination (nav), gait}` | walker -> WaitingForPath (replaces a running walk from its current position) |
| `StopWalk {walker}` | stands still at its extrapolated position, path dropped, no `Arrived` |
| `SetGait {walker, gait}` | takes effect from the next integration; arrival math stays exact because speed is read per integration |
| `SetPlayerLocus {level, nav}` | applies `PlayerMoved` (a rule maps the input to this command) |

`movement::Place(ctx, entity, position)` is the setup helper for scenario placement: creates `Position` + `Walker` and logs a
`PositionException(placed)`.

**Tunables** (all live-tunable): `tier0_radius` 50 m, `tier0_step` 1 tick, `tier1_step` 2 ticks, `tier2_step` 10 ticks,
`portal_delay` 1.0 s, `paths_per_tick` 16, `speed_scale_spread` 0.1 (per-agent variation set at placement).

**Speed table** (m/s, person): walk 1.4, run 4.0 (the owner has since raised them to **2.4 and 8.0** in `SimSpeedTable.h`; the tests read the table), area multiplier 1.0 for the single ground area the engine bakes today
(the table is keyed by area so roads and mud can be added without touching the integrator).

## 4. Step order (kernel systems, registration order)

1. `movement.paths`: take up to `paths_per_tick` walkers in WaitingForPath, oldest `requestedAt` first, ties in storage
   order; project start and destination; run them through **`PathService`** (request, `Update` with an unbounded budget,
   `TakeResults`: results come back in request order); apply: `PathAssigned` or `PathFailed`. Walkers left over wait for
   the next tick. The queue *is* the set of waiting walkers, so a save in the middle loses nothing.
2. `movement.integrate`: for every walking or crossing walker that is due (section 5), advance it by the time since its
   `lastIntegrated`, emit `PortalCrossed` / `Arrived`.

Rules react to `Arrived` and `PathFailed` as usual; this slice only tests them with a toy rule.

## 5. Tiers

Per walker, evaluated each step: **T0** if within `tier0_radius` (horizontal) of the player in the player's level; **T1** if
in the player's level; **T2** otherwise (other levels, or no player yet). A walker is due when `IsStaggerDue(entity, now, dt, step)`
holds, so load spreads over ticks by entity index; it then integrates the whole time since `lastIntegrated`. A larger
kernel step (fast-forward) simply makes every walker due. Bodies (later) force T0.
`movement::PositionAt(world, entity, tick)` extrapolates along the path without mutating anything (addendum 6.2).

## 6. Integration

Walk the corner list: consume `speed x time` metres per leg (3D length; speed = table(class, gait, area) x `speedScale`);
leg ends at a level change = portal: reach the link start, wait `portal_delay`, then `Teleport(portal)` to the link end
(the delay and the leg's remaining time are consumed from the same time budget, so a step spanning several corners and
portals is exact). Reaching the last corner: `Arrived{at = lastIntegrated + time used}`, state Idle. `NavPath` already carries
the level of every corner.

## 7. No-teleport rule and detector

All position writes go through one function, `movement::MovePosition(ctx, entity, to, cause)`: cause *path* is checked
against the fastest allowed speed (run x (1 + spread) x elapsed + tolerance), cause *exception* emits `PositionException`.
Invariant `movement.no_teleport` (runs after every step in tests and soaks): fails if a walker's `Position` differs from the
end of its recorded last integration (somebody wrote it behind the chokepoint's back), if any recorded move exceeded the
speed limit, or if a position is not on the mesh (projection fails by more than 0.5 m).

## 8. Tests (headless, in `-sim_selftest`)

- `SimWalk` alone: straight/curved paths, many small steps = one big step (to 1e-4 m), exact arrival tick, speed change mid-path.
- Tier selection: radius edge, level change of player, walker crossing a portal changes tier, stagger due once per period.
- Commute test (the done-when list above), hash every tick on two runs, save/load mid-walk and continue identically.
- Fast-forward: the same scenario stepped in blocks of 1, 10 and 600 ticks arrives at the same ticks (±1) in the same order.
- Detector: an injected foreign write and an over-speed move are caught; `Place` and portal crossings are not violations.
- Path requests: cap per tick respected, oldest first, unreachable destination -> `PathFailed`, stale walker -> rejected.
- Cost note (like the toy world throughput): µs per walker per integration at 400 walkers.

## 9. Decisions I took that you may want to change

1. **Requests are solved to completion inside the tick, capped by `paths_per_tick`**, instead of slicing one search across
   ticks (addendum 6.3). Measured: the longest test route (3 portals, 74 polygons) takes **55 µs** one-shot. Slicing across
   ticks would need the half-finished search in the save, or a replay-on-load trick, to stay byte-identical after load.
   `PathService` stays in use (request order, dedicated query) and its iteration budget stays available if a real world ever
   needs it. Say so if you want true slicing now.
2. Partial paths fail instead of walking to the closest reachable point.
3. Speeds (1.4 / 4.0 m/s here, 2.4 / 8.0 m/s now) and the tier numbers above are placeholders until the calibration slice measures a real body.
4. The player is a separate `PlayerAnchor` (told by the game), not a Walker.

## 10. Settled by the owner

- Calendar scale (2026-10-23): **0.5 game minutes per world second** (1 game minute = 2 s, 1 game hour = 2 min, a day = 48
  min). Movement stays in real world seconds. The conversion lives in M4's time module, not in this slice.

## 11. As built (differences from the text above)

- `Walker.state` has three values (Idle, WaitingForPath, Walking); "crossing" is `Walker.progress.crossing`, so the state is not
  stored twice. The level of a walker is `cornerLevels` of its current leg; the player's comes from `PlayerAnchor`.
- The position chokepoint is `MovementModule::AdvanceWalker` (and `Place`), not a separate `MovePosition` function: it
  integrates, writes `Position` and records `lastTo` / `lastWalked` / `lastTicks`. Portal crossings are logged as
  `PortalCrossed` + `PositionException(portal)` and are not counted as walking.
- Waiting requests are ordered by `requestedAt`, ties by entity index (not storage order: entt iterates newest first).
- `PathAssigned.length` and `Walker.walkLength` are walking metres only; the straight line between a portal's two ends
  (levels sit far apart in nav space) is never part of a length or a time.
- The speed table has one ground area today. `NavPath` does not say which area a leg lies in, so every leg uses
  `GroundArea`; the multiplier table is ready for roads and mud once the path reports areas.
- `Walker.lastIntegrated` is also set when a walk is requested, so a request replaces a running walk from its extrapolated position.
