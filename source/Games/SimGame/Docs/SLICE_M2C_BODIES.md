# Slice contract: M2c agent bodies (materialization)

Status: **implemented** (2026-10-23). Written from the owner's direction and checked against the code; section 9 lists what was
decided on the way. Sources: addendum 3.3 (spawning is a handshake), 6.2 (bodies force tier 0), 7.2 (materialization by distance);
SLICE_M2B_BRIDGE.md (Snapshot, messages).

## 1. Owner's direction (2026-10-23)

- "Server dictates target position, but the spawned NPC uses its own navigation logic and has its own movement code. A spawned NPC
  has more complex movement and state code. If following a road, the sim dictates the next waypoint."
- Look: a **mix of both models** (`base.glb` and `guard.glb`), picked by the sim at the first proposal and saved with the agent.
- Free to use GameData content: models `models/npc/base.glb`, `guard.glb` and their embedded `idle` / `walk` / `run` animations.

## 2. Model

```
sim (thread)                                   game (thread)
  bodies.update  ---- BodySpawnProposed --->   SimAgents: hidden from the camera? -> SimNpc::SpawnAt
                 <--- BodyAccepted ----------
                 ---- BodyGoal (next corner) ->  SimNpc: own navigation (PathFollowQuery) to the goal, own state
                 <--- BodyReport (5 Hz) ------   feet position of the body
  ApplyBodyPosition: walker progress := nearest point on its route to the body (forward only, at most run speed)
                 ---- BodyDespawnProposed --->   hidden, or after 2 s: destroy
                 <--- BodyReleased ------------
```

While a body is **Active** it is the mover: the walker is not integrated (`movement.IsBodyDriven`), its progress follows the body's
reports, arrival is decided by where the body stands (within `body_arrive_radius` of the end). Everything the sim does with a walker
without a body (ghost integration, tiers, portals) is unchanged, and it resumes the moment the body goes away.

## 3. Sim side (`Sim/Bodies/SimBodies.*`, `Sim/Movement/SimMovement.*`)

| Record | Notes |
|---|---|
| `Embodied` (component, in movement) | state Proposed / Active / Releasing, `since`, `lastReport`, which path and leg was last sent as goal, `stopSent` |
| `Appearance {model}`, `BodyRetry {until}` (components) | model 0 base.glb, 1 guard.glb from the `bodies.look` stream; retry cool-down after a refusal or a lost body |
| output events | `BodySpawnProposed {entity, local, heading, speedScale, model}`, `BodyDespawnProposed {entity}`, `BodyGoal {entity, local, next, hasNext, kind Move/Stop, gait, final, speedScale}` |
| input events | `BodyAccepted {entity, local}`, `BodyRefused`, `BodyReleased {entity, local}`, `BodyReport {entity, local}`, `BodiesCleared` (all map onto one command, `ApplyBodyInput`, so they are logged and replay) |
| tunables | `bodies.spawn_radius` 120 m, `despawn_radius` 140 m, `max_bodies` 32, `propose_period` 5 ticks, `proposal_timeout` 30, `report_timeout` 30, `retry_cooldown` 50, `release_timeout` 60; `movement.body_off_path` 3 m, `body_arrive_radius` 0.75 m |

- **Proposals** every `propose_period` ticks: release first (player in another level, farther than `despawn_radius`, no player, or the walker
  has reached a portal), then spawn, nearest first (ties by entity index), only walkers of the player's level inside `spawn_radius`, never one
  on a portal leg, within `max_bodies`. Proposed bodies that are not answered within `proposal_timeout` lapse (cool-down); Active bodies
  silent for `report_timeout` are given up with a warning and the walker walks on by itself; a Releasing body not released within
  `release_timeout` is taken back.
- **Goals** every tick for Active bodies: the end corner of the leg the walker is on, plus the corner after it (`next`) so the body does not
  stop at every corner; sent when the path, the leg or the gait changes. A walker that is idle or waiting for a path gets a Stop goal.
- **ApplyBodyPosition** (the only way a report moves a walker): nearest point on the next three legs, forward only (`rebase` on acceptance may
  move back a little: the body appeared where the walker was a moment ago), never faster than the fastest gait x `speedScale` (the detector's
  rule; a faster report advances by what could be walked). A body farther than `body_off_path` from the route makes the sim plan again from
  where the body stands (at most every 2 s). At the end of the leg before a portal the walker steps onto the portal leg and the sim takes it
  across; the body is released because the walker is at a portal. At the end of the route: `Arrived`.
- **Tier**: an embodied walker is tier 0 in the snapshot (`AgentView.tier`, `AgentView.body`).
- **Loads**: a load drops every `Embodied` (the game has no bodies after it); the game side clears its bodies on every level change and sends
  `BodiesCleared`. Bodies the sim does not know about are told to go (`BodyDespawnProposed` in answer to a stray report).

## 4. Game side (`Sim/Bridge/SimAgents.*`, `SimNpc.*`)

- **Answering a proposal** (`SimAgents`, owner's rule: bodies fade in and out, also in sight and close by): every proposal is accepted. It waits for the first snapshot at
  least as new as the proposal's tick (snapshots trail the sim by one or two ticks; an older one shows an agent that just took a portal still on the far side). If the agent is
  then in another level than the player's there is no body (`left the level before its body appeared`). The body appears where the agent is then. It appears **at once**
  right after a level load (3 s) and when the spot is *hidden*: outside a 70 degree half cone of the camera, behind level geometry (`Physics::LineTrace` against the world),
  or in a BSP cluster the camera's cluster cannot see. Otherwise it **fades in** over `SimNpc::FadeSeconds` (1 s). A despawn goes the same way: at once out of sight, a 1 s
  fade out in sight (the body keeps walking and reporting while it fades). Why it goes is only logged (portal, range, level, no player).
- **Reports**: every body reports its feet position every 0.2 s.
- **`SimNpc`** (entity "sim_agent_body", not saved with the level): `CharacterController` (NPC quality, 0.45 m x 1.8 m), `SkeletalMesh` of the
  appearance's model (`base.glb` has no textures: a white-textured figure with a per-person tint, kept on purpose; `guard.glb` is textured), a fade through the tint alpha
  (below 1 the engine draws the mesh as a blended Transparent surface, at 1 as the alpha-tested Masked one), `SimNpcAnimator` (idle / walk / run from the model's own clips by ground speed; poses only advance while the mesh is drawn).
  Movement is its own: `PathFollowQuery` on the level's nav mesh to the goal (the engine's `FindSimplePath`), speed from the **shared speed table**
  x the agent's `speedScale`, acceleration and turning, a Blocked state when it wants to move and does not (asks for the path again). On reaching a
  non-final goal it walks on to `next`; at a final goal or a Stop it stands. This is where doors, other bodies and reactions will grow.

## 5. Tests (`-sim_selftest`, `SimBodiesTests.cpp`, `SimBridgeTests.cpp`) and the in-game run

A scripted stand-in for the game (`FakeGame`) answers proposals, walks bodies in straight lines at a chosen fraction of the agent's speed and
reports, exactly the messages the bridge sends. Checked: budget, nearest first, same-level only, hysteresis; a body at the agent's speed arrives
with the ghost within 1.5 s and a body at half speed makes the agent arrive when the body does; goals, a Stop at the end; a body 6 m off its route
makes the sim plan again; no answer, refusals with cool-down, silent bodies; release when the player changes level, a game that never releases; a
route over a portal with a body; a stray report; a save with bodies out loads without them and the agents walk on; replay of a run with body
reports as inputs reproduces every checkpoint. In game (`-headless -sim_bridge_test`): 9 bodies appear near the pawn out of sight, walk, and the
sim's agents stay within 3 m of them; after a save and load the bodies come back.

## 6. Not here

Doors and dynamic obstacles, bodies avoiding each other, perception and reactions, dialogue, death and ragdoll, hitboxes, clothing variety
beyond the two models, calibration of the body's walk against the speed table (the body uses the table, but nobody has compared times with a
rendered animation yet), and the level transition (a body at a portal is released; the other level spawns its own).

## 7. Engine facts relied on

`Level::BspData.FindClusterAtPosition` / `IsClusterVisible` (cluster visibility), `Physics::LineTrace`, `Camera::position` / `Forward()`,
`CharacterController` (NPC generic quality), `PathFollowQuery` + `NavigationSystem::FindSimplePath`, `SkeletalMesh` / `Animator` with the clips
embedded in `base.glb` and `guard.glb`, `EntityHandle` for references that survive a level change.

## 8. Settled by the owner

Body motion: the sim dictates targets, the body navigates by itself. Look: mix of both models.

## 9. Decisions taken without asking

The sim is the authority for arrival only through the body's reports (the walker follows the body, not the other way round); spawn/despawn radii 120 / 140 m (they were
45 / 55 m; the test levels are 64 m across and have no fog, so everything inside the level must have a body) and budget 32 bodies; a lost body hands the agent back to the sim;
the fade lasts 1 s.

## 10. Settled later by the owner (2026-10-23)

Citizens stay tinted mannequins (`base.glb`). Bodies appear and disappear by fading the model's alpha, in sight and close by too, replacing the first rule (no visible spawns
closer than 15 m, a 2 s wait, refusals).
