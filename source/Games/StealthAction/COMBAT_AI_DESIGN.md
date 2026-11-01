# StealthAction — NPC Combat AI Design

Combat behaviour for every fighting NPC derived from `NpcBase` (guards, bandits, melee guards, hybrids).
The goal is enemies that feel smart to fight against: they use cover, pin and flank, punish openings,
react to the player's weapon and stance, and talk to each other — while every smart move is readable
and beatable, so winning feeds the player's power fantasy.

This document is the contract for the combat AI code. Code that disagrees with it is a bug in one of
the two; fix whichever is wrong in the same change.

---

## 0. Requirements

| # | Requirement | Section |
|---|---|---|
| R1 | Hybrid (gun + melee) is the most common archetype; melee-only and ranged-only are supported too | §7 |
| R2 | Combat logic split into a base class and child classes, in the same way as `InvestigationBase` and its subclasses | §3 |
| R3 | Smart in **visible** ways: every behaviour has a tell (stance, animation, voice line, sound) and a counter | §2 |
| R4 | A target can be the player **or another NPC** (faction / hostile tags) | §4 |
| R5 | Cover is found at runtime from the level geometry; optional `cover_point` map hints improve it | §10 |
| R6 | Crouch and crouch-walk are part of combat movement | §11 |
| R7 | Only the existing animation clips are used (no new clips) | §11.2 |
| R8 | Fights with 10+ NPCs stay within the frame budget | §16 |
| R9 | A target that ducks behind cover does not drop combat into a search: the squad keeps fighting the spot it believes the target is hiding in | §5 |
| R10 | Suppressive fire at the target's last known position | §6.5 |
| R11 | Grenades and molotovs to flush a target out of cover | §12 |
| R12 | Voice lines and gestures/SFX, chosen by situation | §14 |
| R13 | Everything survives save/load | §3.5 |
| R14 | Game code only; any engine change is listed in §17 | §17 |

---

## 1. Current state

| Area | Today | Problem |
|---|---|---|
| Ranged combat | `NpcBase::UpdateTargetAttack` fires a 5-damage bullet every ~0.5 s with perfect lead; `FindAttackLocation` picks a random strafe spot every 1.5–3.5 s | Keeps distance and shoots; no cover, no squad play, unfair aim |
| Attack spot validity | `CheckAttackLocation` ([NpcBase.cpp:1813](Entities/Npc/NpcBase.cpp:1813)) returns `true` whether the LOS check passes or not | Ranged NPCs choose spots without line of sight |
| Melee | `NpcGuardMelee` state machine (pressure, wind-up, bait, block, punish, guard break) | Good duel logic; no group play, approaches straight through gunfire |
| Hybrid | `NpcGuardHybrid` switches weapon only by distance (< 4 m melee, > 10 m ranged) | Ignores what the target is doing |
| Movement | Locomotion plays only `idle`/`walk`/`run`; speed hard-set to 5/2 in `UpdateTargetFollow`; facing via `lockAtTarget` in `AsyncUpdate` | Feet slide when strafing while facing the target |
| Losing sight | `lastSeenPosition` keeps tracking the real position for 1 s ([NpcBase.cpp:1244](Entities/Npc/NpcBase.cpp:1244)); BT branch "move to get LOS" runs to `target_lastSeenPosition` (1 m radius), waits 5 s, then `TargetLostInvestigation` drops combat into a 10 s search | Everyone runs into the target's cover; ducking for a few seconds ends the fight; instant re-detection makes it flip-flop |
| Target loss spreading | `StopTargetFollowFinal` cancels follow on allies within 5 m | One NPC losing the target cancels the others |
| NPC targets | Melee damage only hits entities tagged `player` ([NpcGuardMelee.cpp:271](Entities/Npc/NpcGuardMelee.cpp:271), `:297`); `WarnAboutAttack` only comes from the player's twin swords | NPC-vs-NPC melee does nothing |
| Squad | None; every NPC decides alone | No coordination, no fairness cap on damage |

Unused assets that this design relies on: `base.glb` has `walk_l/r/b`, `run_l/r/b`, `crouch_idle`,
`crouch_f/b/l/r`, `slide`, `stun`, `sword_*`. `CharacterController` has `Crouch()` / `UnCrouch()` /
`isCrouched`.

---

## 2. Design principles

1. **Readable intent.** An NPC announces what it is about to do (voice line, gesture, stance change,
   aim-up pose) a moment before doing it. Overhearing "Going left!" and catching the flanker is the
   core power-fantasy loop.
2. **Every tactic has a counter.** Holding an angle → peek from the other side. Suppression → move
   while they're reloading. Flank → intercept. Grenade → relocate. Rush → parry/shoot the runner.
3. **Fair to the player, not to NPCs.** Attack tokens, deliberate first misses and off-screen
   leniency only apply when the target is the player (§9). NPC-vs-NPC fights use plain rules.
4. **Pressure at the right moments.** Enemies punish openings (reloading, being stunned, being busy
   with another enemy) and back off from strength (shaken morale, getting shot).
5. **No psychic knowledge.** After losing sight, NPCs only know what they saw, heard or were told by
   squadmates (§5). Sneaking away from cover while they hold the old spot must work.
6. **Commitment.** Tactics have a minimum commit time and switching hysteresis, so NPCs don't dither.
7. **Reuse the engine and game helpers** (`MathHelper`, `RandomHelper`, `Delay`, `Physics::*Trace`,
   `NavigationSystem`, `AiPerceptionSystem`); no duplicate utilities.

---

## 3. Architecture

### 3.1 Brain / body split

The NPC entity is the **body**: animation, damage application, weapon meshes, movement execution and
the physical verbs. The combat **brain** decides what to do and drives the body through those verbs.

```
NpcBase (body)                          CombatBehaviorBase (brain, owned by NpcBase)
  MoveOrder  <───── writes ───────────    tactic selection, target view, contact, morale
  FireShot(aimPoint, kind)  <── calls ──  current CombatTacticBase
  PerformMeleeAttack / StartBlock / Kick    │
  SetStance / SetWeaponMode / Throw         └─ requests → CombatCoordinator (tokens, slots, roles,
  PlayPhrace                                    cover reservations, voice lines, squad belief)
```

`NpcBase` gets `std::shared_ptr<CombatBehaviorBase> combatBehavior`, created by a virtual
`CreateCombatBehavior()` on the entity class (the counterpart of `CreateInvestigationFromReason`).
Civilians return `nullptr` and keep their flee/report behaviour.

| Entity | Behaviour |
|---|---|
| `NpcGuardBase` ("npc_guard"), `NpcBanditBase` ("npc_bandit") | `RangedCombatBehavior` |
| `NpcGuardMelee` ("npc_guard_melee") | `MeleeCombatBehavior` |
| `NpcGuardHybrid` ("npc_guard_hybrid") | `HybridCombatBehavior` |

### 3.2 Class layout

```
CombatBehaviorBase              lifecycle, target view refresh, contact tracking, tactic selection,
                                accuracy/suppression/morale state, serialization, debug UI
  RangedCombatBehavior          ranged tactic set + ranged tunables
  MeleeCombatBehavior           melee tactic set + melee tunables
  HybridCombatBehavior          both sets + weapon-mode choice (§7.3)

CombatTacticBase                Score(ctx), Start(), Update(dt), Stop(), IsDone(),
                                CanInterrupt(), MinCommitTime(), Serialize/Deserialize
  TacticStrafe                  current FindAttackLocation logic (fallback when no cover)
  TacticMeleeDuel               current NpcGuardMelee state machine
  TacticTakeCover, TacticCoverFight, TacticSuppressLastKnown, TacticHoldAngle, TacticFlank, TacticKite,
  TacticAdvance, TacticRush, TacticWaitForSlot, TacticBackOff, TacticThrowFlush,
  TacticFallBack, TacticCheckLastKnown, TacticEvade
```

Archetypes do not inherit from each other. Each behaviour **lists** the tactics it can use, with a
weight per tactic. Hybrid is the union of both sets, so there is no diamond inheritance.

Tunables are virtual getters on the behaviour, like `SearchInvestigationBase::SearchDuration()`, so
a child class (or a future elite variant) overrides only the numbers it changes.

### 3.3 Tactic selection

Every brain tick (§16):

1. Refresh the target view (§4) and contact (§5).
2. Check **forced interrupts** in priority order: stunned / ragdoll → nothing; live grenade or fire
   zone nearby → `TacticEvade`; hit flinch (§13.2); target within 1.5 m of a ranged-mode NPC and
   attacking → `TacticBackOff`.
3. Otherwise score every tactic in the set: `score = tactic.Score(ctx) × archetypeWeight`.
   `Score` returns 0 when the tactic is not applicable.
4. The current tactic gets a commitment bonus (+0.15) until `MinCommitTime()` passes. Switch only if
   the best tactic beats the current one by more than the hysteresis (0.1), or the current tactic is
   done or interruptible and done.

The selection context (`CombatContext`) is a plain struct: target view, contact, own state, the
coordinator snapshot for this NPC's squad, and the cover query results. Tactics read it; they never
look up other NPCs directly.

### 3.4 Frame order and threading

The engine frame is: physics → `AiPerceptionSystem::Update` → `Level::Update` (main thread) →
`Level::AsyncUpdate` (**thread pool**, all NPCs in parallel) → `Level::LateUpdate` (main thread).

| Phase | What runs |
|---|---|
| `Update` (main) | First `NpcBase::Update` of the frame calls `CombatCoordinator::Step()` (guarded by frame number): collect last frame's requests, resolve tokens / slots / roles / reservations / voice lines, merge squad belief, publish a read-only snapshot per squad |
| `AsyncUpdate` (parallel) | Brain tick inside `NpcBase::AsyncUpdate`, at the place `UpdateTargetAttack` always ran (after `UpdateBT`); `UpdateTargetAttack` now runs the brain. Reads own state, the snapshot and target entities (read-only); writes only its own NPC and its request outbox |
| `LateUpdate` (main) | Effects on **other** entities: melee damage to NPC targets, `WarnAboutAttack` to NPC defenders, near-miss suppression from bullets, grenade/molotov spawning |

Rules:
- A brain never writes another NPC's data in `AsyncUpdate`. Cross-NPC effects are queued and applied
  in `LateUpdate`, like the existing `shareKnowlageWith` / `pendingTargetLostSpreads` pattern.
- The coordinator is a static class with main-thread-only mutation; the snapshot is immutable while
  `AsyncUpdate` runs. Requests go into a per-NPC outbox (no locks on the hot path).
- Physics traces from `AsyncUpdate` are fine (the current `FindAttackLocation` already does them).

### 3.5 Save / load

- `NpcBase::Serialize` stores the behaviour like it stores `currentInvestigation`: high-level brain
  state (weapon mode, morale, suppression, magazine, accuracy ramp, current tactic id). On load the
  current tactic restarts with `Start()`.
- Contact fields (§5.1) are added to `TargetInfo::Serialize/Deserialize`.
- The coordinator and the cover cache are runtime-only. Tokens and slots are re-requested on the
  first frame after loading.

---

## 4. Targets: `CombatTargetView`

The brain never assumes its target is the player. Each tick it fills a `CombatTargetView` for its
primary target and for the other known hostile targets it ranks (§4.2).

| Field | Player | NPC target |
|---|---|---|
| `position`, `velocity` | `controller.GetSmoothPosition()`, `controller.GetVelocity()` | `Position`, `controller->GetVelocity()` |
| `eyePosition` | camera height from the controller | `Position + 0.6 m` (NPC positions are body centres) |
| `isMeleeArmed` | `currentWeapon->IsMelee()` | brain weapon mode is Melee |
| `isReloading`, `magazineEmpty` | `WeaponFirearm::reloading`, `Data.magazineAmmo == 0` | brain reload state (§6.12) |
| `isBlocking`, `isParrying` | `currentWeapon` / `currentOffhandWeapon` `Blocking` / `Parrying` | brain `isCurrentlyBlocking` |
| `isCrouched`, `isInCover` | `controller.isCrouched`; in cover = cover cache says the spot protects from me | stance; brain's current cover spot |
| `isAimingAtMe` | camera forward vs direction to me within ~6° (scaled by distance) | its aim target is me |
| `isFastMoving` | dashing / sliding / mantling / speed > run | speed > 4 |
| `lastFireTime` | from weapon fire events (`EmitSoundAt` from `WeaponFirearm`) | brain's last shot time |
| `isStunned` | n/a | `isStunned()` |
| `health01` | `Health / MaxHealth` | same |

### 4.1 NPC-vs-NPC support

- Melee damage (`DoNormalAttackDamage`, `DoBreakAttackDamage`) hits any entity that is hostile to the
  attacker (`IsHostile` / faction), not only `player`. Damage to NPCs is applied in `LateUpdate`.
- An NPC melee swing calls `WarnAboutAttack` on an NPC defender, so NPCs block each other. Today only
  the player's twin swords do it.
- Bullets already filter by `OwnerTag`; near-miss suppression (§13.1) works on NPCs too.

### 4.2 Threat ranking

`threat = danger × proximity × attention`, where:
- `danger`: has a gun and line of sight to me > melee-armed close > melee-armed far; ×1.5 if it shot
  me in the last 3 s;
- `proximity`: 1 at ≤ 5 m, falling to 0.3 at 30 m;
- `attention`: ×1.3 if it is aiming at or attacking me, ×1.2 if it is the player.

The primary target switches only when another threat beats it by 30% (`CombatThreat::SwitchFactor`; the 1 s dwell is not implemented), so a guard fighting a
bandit turns to the player when the player starts shooting at him, without flip-flopping.

---

## 5. Contact: losing sight behind cover

### 5.1 Contact levels

Each NPC still sees for itself (`TargetInfo::sees`). What the **squad believes** about a target is
held by the coordinator per squad (§8.4) and mirrored into each member's `TargetInfo`:

```
enum class TargetContact { None, Visible, Concealed, Lost };

TargetInfo += contact, lastVisiblePosition, lastVisibleVelocity, lastVisibleTime,
              estimatePosition, estimateRadius, concealedUntil, peekPoints[3], hideSpotType
```

| Level | Meaning | Squad behaviour |
|---|---|---|
| **Visible** | at least one squad member sees the target | normal tactics |
| **Concealed** | recently lost, no evidence it left: "he's behind that crate" | combat continues against the estimate (§5.3) |
| **Lost** | evidence the target left, or concealment timed out | `TacticCheckLastKnown` (§6.13), then the existing search |
| **None** | handed off to `TargetSeenInvestigation` | investigation / patrol, unchanged |

### 5.2 Entering Concealed

When the last squad member loses sight:
- `lastVisiblePosition` / `lastVisibleVelocity` = where the target actually was when last seen.
  This is separate from `lastSeenPosition`, which keeps its current meaning for investigations.
- While engaged, the "keeps tracking the real position" window shrinks from 1.0 s to
  `LostSightTrackTime()` = 0.4 s (the "saw him duck" moment). Outside combat it stays 1.0 s.
- `estimatePosition` = last visible position + velocity × 0.4 s, projected onto the navmesh.
  `estimateRadius` starts at 1 m.
- The hide spot is classified with the cover cache (§10.2) for the direction from the squad to the
  target: **low** (peek point above it at head height), **corner** (peek points at both edges, found
  with lateral traces), or **none** (the target ran around a corner or out of view: peek point at
  the corner it vanished behind).
- `concealedUntil` = now + `ConcealedDuration()` (6–10 s, random per squad).
- Voice line: "He's behind cover!" / "He ducked!" (§14).

### 5.3 Concealed behaviour

By time since concealment:

| Time | Squad response |
|---|---|
| 0–1.5 s | Everyone holds: ranged NPCs stay in or move to cover and aim at peek points (`TacticHoldAngle`); one suppressor fires at the hide spot (`TacticSuppressLastKnown`) |
| 1.5 s+ | Coordinator assigns one flanker per squad (`TacticFlank`): moves to a spot with LOS to the estimate, at least 60° away from the squad's main direction, not straight to the estimate |
| 1.5 s+, melee/hybrid within 10 m | `TacticAdvance` cover-to-cover toward the hide spot while ranged allies pin; a melee NPC may wait beside the corner to catch the target coming out |
| 4 s+, same spot | `TacticThrowFlush` becomes available (§12) |
| any | Holders are spread over the peek points so their aim is readable; when the target peeks, a holder fires after a reaction delay (0.3–0.5 s) with higher-than-normal accuracy |

NPCs that lost sight because of their **own** movement (they walked behind a pillar) side-step to
regain LOS on the estimate instead of running at it.

### 5.4 Evidence

Refreshes contact (back to Visible, or resets `concealedUntil`):
- any squad member sees the target;
- the target fires from cover (muzzle sound via `heardSounds` within 3 m of the estimate);
- the target damages a squad member.

Moves to **Lost**:
- a squad member has had LOS to the estimate at both crouch and stand height for 0.5 s and the
  target isn't there → "He's not here!";
- a sound from the target more than 5 m away from the estimate (the squad turns to the new position
  and re-enters Concealed there);
- `concealedUntil` passes, or `estimateRadius` exceeds 6 m. The radius grows by
  0.3 × the target's crouch speed per second without evidence.

Player footsteps do not emit perception sounds today, so sneaking away quietly works by design.
Making running audible is an open item (§19).

### 5.5 What changes in existing code

- `UpdateObserver` detection decay: while a target is engaged (`follow && attack`) and its contact
  is Visible or Concealed, detection does not decay and `StopTargetFollow` is not called.
- `StopTargetFollowFinal` ally cancellation: for NPCs with a combat brain, losing the target goes
  through the squad belief instead. The current spreading stays for arrest/follow without combat.
- Behaviour tree: see §15.

---

### 5.7 Combat awareness and sharing (as implemented)
- **Sharing:** every frame somebody in the squad sees the target, all engaged members get that position as their
  last seen / last line-of-sight position (`NpcBase::ReceiveSquadSighting`), whether or not they can see each
  other. Searches and the behaviour tree work from what the squad knows.
- **Hearing the target:** an NPC that hears its target (a shot; the player's gun reaches every observer within
  its `npcNotifyRadius`, 20 m) without seeing it turns to the sound at once for 1 s, whatever its tactic faces.
  The sound, or a shot from it passing within 1.5 m of a member, while nobody sees it (whatever the contact level)
  moves the squad's estimate to it, restarts the hiding timers and the suppression,
  and puts a lost squad back to hunting it there. Every member (and any NPC that hears it itself and is after that
  target, squad or not) moves its **last line-of-sight position** there when the sound is more than 2 m from it
  (`belief.heardUpdateDistance`), so aiming, facing and moving work from where the target really is. A holder the
  target has now left out of fighting range (85% of its attack range) moves up to a spot within that range (60 to
  100% of it) that sees the new position, or heads that way if there is none; holders in range stay put.
- **It moved: go and find it.** When the target turns up (heard, or tracked by ear) more than 5 m from where the
  squad had it, or is heard after the squad had lost it, for 6 s the holders don't hold the old angles: each moves to
  a spot that sees the new position (from 4 m out to its fighting range, up to 40 m of path), or heads straight for
  it when there is none. Suppressors suppress the new spot; flankers are re-planned against it.
- **Tracking by ear:** while the target hides, once a second the squad checks whether it can hear it: within 8 m of
  any member, or moving faster than 3 m/s within 25 m. If so the estimate moves to it (off by up to 1.5 m), the
  hiding timers restart, and the members get the new estimate as last seen. Walking round a wall doesn't shake a
  squad in a fight; sneaking off slowly and far enough does.
- **Watching the corner:** an NPC with a wall between it and the believed position faces and aims at the farthest
  point of its path there that it can see (where the target would come into view), not through the wall; if that
  point is more than 100 degrees off the target's direction it faces the target's direction. Without sight of the
  target the aim pitch is held within 20 degrees, and points within 1.5 m are aimed at level. A searching NPC that was in
  the fight carries the gun at the ready.
- **Cover:** an NPC only changes cover on its timer once it hasn't seen the target for 1 s (no hopping while it
  could be shooting), and moves up at most every 6 s.
- **The main force doesn't close in** (user: "keeps pressure, doesn't push"): strafing NPCs fight from the range
  they are at (sidesteps at most 0.5 m closer, boxed in they back off) and only close in when the target is beyond
  85% of their attack range; no stepping in or moving up when the target reloads (`strafe.pressInOnReload`,
  `cover.advanceOnReload`); a retreating target is only followed by the NPCs it has left out of range; an NPC that
  lost its line of sight moves to a spot at most 2 m closer, or sidesteps, after 1.2 s; holders move for a line at
  most 1 m closer; no pusher when nobody flanks. Only flankers (and the lone close-quarters case) move in.

### 5.6 As implemented (M4)

- **Belief:** `TargetBelief` (contact level, last visible position and velocity, estimate and its radius, peek
  points, timers, roles) lives in `CombatCoordinator`, one per target, rebuilt every frame from the engaged
  brains. There is one squad per target: NPCs far apart share it (the cluster-by-distance squads of §8.1 are not
  built). The context carries the belief, the believed threat position and the NPC's role.
- **Visible to Concealed** after NPCs that stand out in the open to shoot (or fight in melee) have looked for 0.5 s
  without seeing the target, or after nobody has seen it for 4 s. An NPC hiding by choice between peeks doesn't count:
  otherwise the last NPC of a squad, hiding for a moment, would declare the player lost. The estimate is the last visible position
  plus 0.4 s of its velocity; the radius starts at 1 m and grows 0.5 m/s; concealment lasts 6 to 10 s. Peek points
  are over the estimate and 1.2 m either side of it (the hide spot is not classified as low or corner cover).
- **Evidence:** a shot heard from the target moves the estimate to the sound and restarts the timers. The target
  turns **Lost** when the timer runs out, the radius passes 6 m, or (no sooner than 3 s into concealment) a member
  within 15 m that faces the spot (60 degrees either side) has a clear look at it, standing and crouched body height,
  for 1 s while nobody sees the target.
- **Roles while Concealed** come from the squad plan (§8.6): flankers go round, up to 3 NPCs with a line of fire
  on the spot suppress it for 5 to 8 s, the rest push; after the suppression one NPC holds the angle
  (`TacticHoldAngle`: crouched, aimed at a peek point; when the target shows up there it shoots at once with the
  first-shot warning miss skipped and 1.25x accuracy for 2 s) and everybody else pushes. Pushers
  (`TacticRegainSight`) go to spots 4 to 10 m from the estimate, 10 to 70 degrees round, seeing it and reachable;
  within 7 m of it they creep, and a spot that doesn't show the target after 1.5 s is traded for another. Melee NPCs close to 3.5 m of the estimate, pause 0.8 to
  1.6 s and push in on it (`TacticAdvance`); a hybrid within 8 m of the estimate draws its sword. Nobody uses the
  target's real position: cover, aim and facing use the estimate.
- **Visible, but not to this NPC:** a gun NPC that hasn't seen the target for 0.8 s while a squadmate does, and has
  no cover spot to go to, moves to a reachable spot that sees the target (`TacticRegainSight`, reacquire mode: 5 m to
  the desired range + 3 m, within 25 m of walking). A cover spot whose peeks don't see the target for 1.2 s is
  abandoned for another.
- **Lost:** the nearest NPC crouch-walks to the estimate, looks around for 1.5 s and ends the fight; the second
  nearest covers it for up to 3 s; the rest end the fight after 1 s. Ending the fight (`NpcBase::TargetLost`) always
  starts the `TargetSeen` search now: it used to go through `TryStartInvestigation`, which refuses while
  `target_attack` is set, so an NPC that lost its target used to stand where it was. A 9 s timeout and a 12 s
  backstop in the coordinator end the phase regardless.
- **Contact for the rest of the game:** `NpcBase::IsTargetInContact` (behaviour tree) is true while the squad has a
  belief other than None, so NPCs stay in the attack branch while the target hides. Detection decay is suspended
  for a target the brain holds contact with (`HoldsCombatContactWith`). The ally cancellation in
  `StopTargetFollowFinal` is untouched: the brain's own `TargetLost` does not spread.
- **Melee slots:** 2 per player target (3 when the player has a gun: he can't block them), 2 s minimum hold,
  closest bidder first; none against NPC targets. A melee
  NPC without one waits on a ring 5 m from the target at its own fixed angle (`TacticWaitForSlot`).
- **Rush:** a melee NPC (hybrids no longer rush, §7.3) charges with a 0.8 m sideways
  offset at 5.5 m/s while the target is reloading or stunned and 3 to 12 m away. Closer than 3 m the duel takes over.
- **Not built yet:** waiting beside the corner, the full hybrid weapon-choice scoring
  of §7.3 (only the two triggers above), squad clustering, voice lines (M5).

## 6. Tactics

Each entry: when it scores, what it does, the tell, and the counter. Numbers are starting values.

### 6.1 TacticStrafe (M0)
The current `NpcBase::FindAttackLocation` ring/flank scoring, with the LOS bug fixed. Fallback for
ranged mode when no cover is found or the target is too close for cover to matter.
Until cover exists (M3) it also ducks: standing at a firing position, it crouches when a gun-armed target aims at it
and stands to move or when the target looks away (1.5 s minimum per stance, because a stance change rebuilds the
physics body).
*Tell:* visible movement. *Counter:* shoot.

### 6.2 TacticMeleeDuel (M0)
The current `NpcGuardMelee` state machine moved into a tactic unchanged (pressure, wind-up, bait,
block, punish, guard break). Scores high when the NPC holds a melee slot (§8.2) and is in melee range.

### 6.3 TacticTakeCover
Scores when exposed (no cover from the current threats), under fire or suppressed, or without an
attack token. Requests a cover spot (§10), reserves it, runs there (crouch-walks the last 2 m),
crouches on arrival. *Tell:* runs and slides into cover (`slide` when the last stretch is > 3 m).
*Counter:* shoot it on the way, or flank the cover.

### 6.4 TacticCoverFight
In cover against a visible target. Low cover: crouch (hidden) ↔ stand and fire a burst. Corner cover:
step out sideways and back. One tactic with or without an attack token, so a token changing hands never
interrupts a peek; the token only sets the aim (§9) and the peek length (holders peek longer). The squad
takes turns: while fewer than 2 squadmates are out shooting, a hidden NPC comes out after 0.3 s, and when
nobody is out it comes out even through moderate suppression.
*Tell:* stands/steps out and raises the gun (aim-up pose) before the first shot. *Counter:* aim at
the cover edge and shoot first.

### 6.5 TacticSuppressLastKnown
Fire at the estimate of a hiding target (§5.6). Ignores tokens and the damage budget; the bullets chip
the cover and keep the target down while somebody flanks.
*Tell:* sustained fire hitting the cover. *Counter:* move while they reload (§6.12).

### 6.6 TacticHoldAngle
Crouched (or in cover) aiming at an assigned peek point; fires after the reaction delay when the
target appears there. *Tell:* visible aim at a specific edge. *Counter:* peek at another edge, or
leave.

### 6.7 TacticFlank
Assigned by the coordinator (one flanker per squad). Picks a spot with LOS to the target or its
estimate, ≥ 60° from the squad's main direction, avoiding the target's aim cone; moves crouched when
within the target's view. *Tell:* "Going left!" plus a short point gesture. *Counter:* intercept.

### 6.8 TacticAdvance
Moves cover to cover toward the target. In pairs it alternates (one moves, one covers). Used by melee
approaching a gunner and by the Lost check. *Tell:* "Moving up!". *Counter:* shoot the mover.

### 6.9 TacticRush
Melee or hybrid charge on an opening: target reloading, stunned, busy with another enemy (another NPC
holds a melee slot on it and the target faces away), or target melee-armed and close. Runs with a
slight lateral offset, not a straight line. *Tell:* "He's reloading!", weapon draw. *Counter:*
parry, shoot the runner, back off.

### 6.10 TacticWaitForSlot
Melee NPC without a melee slot: circles on a staging ring (4–6 m), faces the target, feints a step
in, spreads around the target with the others. Takes a slot as soon as one frees up.
*Tell:* visible circling. *Counter:* thin them out one by one.

### 6.11 TacticBackOff
Ranged-mode NPC with the target within 1.5 m: `sword_kick` as a shove (small damage, knockback), then
retreats toward cover or range. Hybrids usually switch to melee instead (§7.3). *Tell:* shove wind-up.
*Counter:* parry/block the shove, follow through.

### 6.12 Reload (body state, not a tactic)
NPCs get a magazine (`MagazineSize()`, e.g. 8 for pistols). When empty: ~1.5 s with the weapon lowered
(`pistol_hold` pose), a click/reload sound, and a voice line ("Reloading!"), preferably in cover. This
is an opening for the player. Suppression bursts count against the magazine.

Implemented in `RangedFireControl` (owned by the brain, so it survives tactic switches and hybrid mode changes):
8 rounds, 1.5 s reload, `SetWeaponLowered` drops the aim pose while reloading, and the magazine refills silently
4 s after combat ends. The `combat_reloading` event is a placeholder until it is recorded (a missing event plays nothing).

### 6.13 TacticCheckLastKnown (Lost)
The two closest squad members: one covers from a spot with LOS toward the estimate, the other
crouch-walks to the estimate, then to the nearest corner around it. When done the brain calls
`TargetLost()` for the squad, which starts the existing `TargetSeenInvestigation` search. Others
hold, then join the search. *Tell:* "Where'd he go?", "Check the corners". *Counter:* ambush the
checker; take down the coverer from behind.

### 6.14 TacticThrowFlush
See §12.

### 6.15 TacticFallBack
Morale broken (§13.3): retreats to cover farther from the target, blind-fires over low cover,
calls for help. *Tell:* "Fall back!", panicked fire. *Counter:* chase, or let them go.

### 6.16 TacticEvade
Forced: live grenade or fire zone within its radius + 1 m, or (ranged NPC in the open) the player
aims at it from > 8 m with a cooldown of 8 s. Slides (`slide`) or runs perpendicular to the threat,
toward cover if possible. *Tell:* the dive itself.

---

## 7. Archetypes

| | Ranged | Melee | Hybrid |
|---|---|---|---|
| Main tactics | TakeCover, CoverFight, SuppressLastKnown, HoldAngle, Flank, Kite, ThrowFlush | Advance, Rush, WaitForSlot, MeleeDuel | All of both |
| Preferred range | 8–18 m | 0–2 m | by weapon mode |
| Approach to a gunner | n/a | cover to cover, or zigzag in the open when aimed at | switches to gun at range |

### 7.1 Ranged
Lives in cover: peek-shoots with a token, suppresses without one, holds angles while the target is
concealed, flanks when assigned. Backs off from close threats with a shove. Throws a grenade/molotov
when the target camps.

### 7.2 Melee
Never runs straight through gunfire: advances cover to cover while the target aims at it, rushes on
openings, waits on the staging ring when no slot is free, surrounds with other melee NPCs, then duels
with the existing logic. When a squadmate gets parried, others hesitate for a moment (0.5–1 s before
taking the next slot).

### 7.3 Hybrid weapon choice
Scored every tick, with a 3 s minimum in each mode and a switch cost (§11.2):

| Favours melee | Favours ranged |
|---|---|
| Target within 4 m | Target farther than 10 m |
| Target melee-armed and charging a gunner ally ("bodyguard": intercept) | Target in cover far away / concealed far away |
| Target reloading or stunned within 12 m | Parried twice in this fight → back off and shoot |
| Concealed target within 8 m (clear the corner with the sword) | No melee slot free and the target is not attacking it |
| Melee slot free on the target | Allies already hold both melee slots |

`DisableRangedWeapon` (disarm) still locks the hybrid to melee.

**As implemented now (user rule: melee only very close, and only when safe):** the gun is the default. Against a
target with a gun the sword comes out within 3 m only if the target is reloading, stunned or not aiming at this
NPC, and stays out while the target is within 4.5 m. Against a target without a gun it comes out within 2.5 m
(stays to 4.5 m). There is no charging with the sword from range any more (no rush on reload, no corner clearing,
no breach with the sword, no intercept). The hybrid's own distance rule (melee under 4 m, back to the gun only past
10 m) is overridden, because it turned every brief sword moment into a permanent melee runner.

---

### 7.4 As implemented

What the target holds decides the tactics, read from two weapon flags:
`Weapon::IsRanged()` (true for every `WeaponFirearm`) and `Weapon::CanBlockOrParry()` (sword, twin swords,
cane). For an NPC target, "has a gun" means its brain is not in melee mode, and "can block" means it is.

- **Melee vs a target that can't block or parry** (a gun, empty hands): runs straight at the target, attacks after
  0.15 s in reach, 0.35 to 0.6 s recovery, never baits or retreats.
- **Melee vs a target that can** (a blade, a melee NPC): runs straight in at full speed until 3.5 m
  (`EngageDistance`), then the cautious duel (flanking footwork at 3 m/s, probing, baiting, retreats grown by blocks
  and parries). The old approach walked the whole 8 m attack range at 2 m/s and crept in 0.8 m steps.
- **Gunners in the open vs a target with a gun** (`TacticStrafe`, when no cover is found): further than the desired
  range + 2 m they walk in firing, each to its own side (up to 4 m off the line) so the squad spreads; beyond the
  attack range they run. In range they sidestep 3 m every 1.6 to 2.8 s, alternating sides, keeping a line of sight
  and at least 5 m from the target. Walking stays under the 2.2 m/s fire limit, so they shoot while moving.
- **Gunners vs a target without a gun** (`TacticKite`, beats strafing and cover): no cover, since there is nothing
  to hide from. Out of reach they sidestep every 1.8 to 3.2 s (alternating sides, 9 to 14 m, keeping a line of
  sight; `CombatMoves::PickSidestep`), shooting, back off at 2 m/s (still shooting) when it gets within 7 m, turn and run at 5 m/s inside 3.5 m, and kick it away (`sword_kick`, 5 damage,
  7 m/s knockback on the player, blocked or parried kicks do nothing, 3 s cooldown) inside 2.2 m. With no way back
  (every retreat direction blocked) they stand and shoot. Tokens and the damage budget still apply, so a
  sword-wielding player faces steady fire but never more than the budget.
- **Hybrids vs a target without a gun:** the gun stays out (forced) and the hybrid kites; the sword comes out only
  inside 2.5 m, or when the target is within 4 m of a gunner ally and the hybrid within 10 m of it (it steps in to
  protect the gunner). Against a target with a gun the reload/stun rush and the clear-the-corner rule stay.
- **Threat ranking** treats a target without a gun as the melee danger (0.8 close, 0.4 far).
- Animation events now reach `NpcBase::HandleAnimationEvent` for every NPC (they used to pile up unread on ranged
  ones); melee NPCs override it for their attacks.

## 8. Coordinator

`CombatCoordinator` (static, game code). All resolution happens in `Step()` on the main thread (§3.4).

### 8.1 Squads
NPCs engaged on the same target, of the same or friendly faction, are grouped by proximity
(cluster radius 15 m, at most 5 per squad). Squads are rebuilt each step from cheap data (positions,
targets); a squad keeps its id while at least one member stays, so its belief survives regrouping.

### 8.2 Tokens and slots (per target)

| Resource | Player target | NPC target |
|---|---|---|
| Accurate-fire tokens | 2 (whole fight, all squads) | no limit |
| Melee slots | 2 | 2 |
| Flanker | 1 per squad | 1 per squad |
| Throwables | 1 in flight per target; cooldown 15–20 s per squad, 8 s per target | same |

Tokens go to the highest bidders: bid = tactic score × (1 + time without a token × 0.2). A token is
held for at least 2 s and is dropped when the holder loses LOS, is stunned or takes cover.

### 8.3 Damage budget (player only)
The coordinator tracks damage dealt to the player in the last second. Above `PlayerDamageBudget()`
(e.g. 18/s), further shots that second are forced misses. This keeps 10+ NPC fights from shredding
the player in one bad moment.

### 8.4 Squad belief
Per squad and target: contact level, estimate, peek points, concealment timer (§5). Merged each step
from members' sightings and requests, then mirrored into each member's `TargetInfo`. Squads far apart
can disagree, which is natural.

### 8.5 Roles and reservations
- Cover spot reservations (a spot is held by one NPC; released when it leaves or dies).
- Peek point assignment for holders.
- Flanker and Lost-check pair assignment (the squad plan, §8.6).
- Voice line arbitration (§14).
- Danger zones: live grenades and molotov fires, read by the cover finder and pathing scores.

### 8.6 Squad plan (as implemented)
`CombatCoordinator::PlanSquad`, per target, 4 times a second and at once when the contact level changes. It
writes a `SquadOrder` (role, flank side, squad axis) into each gun NPC's brain; the tactics read it from the
context. Melee NPCs get no role (melee slots coordinate them) except in Lost. The **squad axis** is the average
direction from the target to the gun NPCs: the line the squad attacks along.

| Contact | Roles |
|---|---|
| Visible | everybody is the **base of fire** (cover fight, strafe, kite: everyone who sees the target shoots); 1 s into a fight, and 2 s after each flank, **flankers** go round: 1 with 2 or 3 gunners, 2 with 4 or 5, 3 with 6+. Through lanes (§8.7) when the approach map has any well round the front; otherwise the open-ground pincer below, which is skipped against a target without a gun |
| Concealed | flankers keep going, or are picked after 0.5 s (same counts); the **main force keeps the pressure on from where it is and doesn't walk onto the target**: up to 3 NPCs with a line of fire on the spot **suppress** it for 5 to 8 s (low-accuracy shots into the cover), everybody else **holds** the ways out (the front entry, or a point over / beside its cover), crouched where it stands; a holder with no line on its point (a wall in between) walks up to 12 m to a spot with one. Only the flankers move in; with nobody flanking, 1 NPC (one without a line of fire) **pushes**. Stacked flankers and pushers breach together (§8.7) |
| Lost | nearest member **checks**, coming in through the cheapest lane other than the front one; the second **watches** from the front lane's stack spot; the rest search (§5.6) |

- **How many go round:** at most half the gun NPCs (at least one from two up, at most 3): the main force keeps
  the pressure on.
- **Picking flankers:** gun NPCs 4 to 25 m from the target, not reloading or busy; the attack token holders keep
  shooting from the front unless nobody else can go. An NPC already round a side swings further round it
  (score = angle round that side - 0.03 per metre); a second flanker takes the other side.
- **The open-ground flank** (`TacticRegainSight`, flank mode, score 1.2: above cover and strafing): a spot 65 to 115 degrees
  round from the squad axis on the ordered side, at 6 m to the desired range + 2 m (5 to 12 m from a hiding
  target), seeing the target, reachable within 40 m. Of up to 6 reachable candidates it takes the best route: path
  length plus 1.5 extra metres for every metre the target can see, and no route closer than 3.5 m to the target.
  The flanker sprints there (no shooting at a sprint) with cover dropped. The flank is over when it gets there,
  when it sees the target from at least 50 degrees round and within the desired range + 4 m, after 3 failed
  searches, or after 15 s; it then rejoins the base of fire from where it stands.
- **Advance on reload:** the moment the target starts reloading, every gun NPC that has sat in cover for 0.8 s
  moves to cover at least 3 m closer (within 10 m), keeping its spot if there is none; strafing NPCs step in.

### 8.7 Lanes, stacking and the breach (as implemented)
Inside buildings "65 degrees round" is usually behind a wall; what matters is which **way in** a route uses.

- **Approach map** (`ApproachMap`, per target, main thread): rebuilt every 1.5 s or when the target / estimate moves
  3 m. `NavigationSystem::BuildReachTree` (engine) runs Dijkstra over the navmesh from the target (35 m circle, 512
  polygons) and returns every polygon's path cost and next step toward the target. One trace per polygon (path cost
  ≤ 30 m) from the target's eye to 1.4 m over its centre says what the target can see.
- **Lanes:** wherever a hidden polygon's next step is in sight, its shared edge is a **crossing**: the doorway or
  corner where a route comes into view. Crossings are grouped (direction within 25 degrees or 3 m apart), nearest
  first; each group is a lane with its `entry` (nearest crossing), `direction`, and a **stack spot** up to 2 m back
  inside the hidden polygon. A lane with fewer than 3 hidden polygons behind it (shadow of a crate) is dropped.
- **Front lane:** the lane most base-of-fire gunners are in (a hidden NPC: its route's lane; one in the target's
  sight: the lane facing it). Cover searches score spots in the line of fire out past the front entry 1.5 lower,
  so the squad waits **beside** a doorway, not in front of it.
- **Lane flankers:** lanes at least 60 degrees round from the front, best first (degrees round - 0.5 per metre to
  the stack); one flanker per lane, two may share the only one. Of the 3 gun NPCs nearest the stack spot (token
  holders last) the one with the cheapest route goes (path + 1.5 per metre the target can see, never within 2.5 m
  of it, at most 90 cost). It sprints with the gun ready, creeps the last 3 m crouched, and waits **stacked**
  (crouched, gun on the entry). NPCs sharing a lane stack 1 m apart further back. A lane order is dropped after
  25 s, or when the target moves so that the lane has become the front.
- **Pushers** (target hidden) take the front lane in turns: only the one nearest the stack moves; the others hold
  crouched with the gun on the entry until it is stacked. A pusher keeps the lane it was given while a lane with an
  entry within 3 m of it exists. The front lane is sticky: another lane must have 2 more base-of-fire gunners to
  replace it (pushers and flankers don't vote), so NPCs walking up a lane can't flip it to another doorway.
- **Close quarters:** a pusher within 8 m of the hiding target gets no lane. It works round the corner the target
  is behind ("slicing the pie"): every second, or on reaching its spot, a 1.5 m crouched sidestep on the same side
  to a spot that sees the believed position (never closer than 3 m), or a step straight in when there is none;
  gun on the spot throughout.
- **Breach:** when everybody with a lane is stacked, or the first one has waited 1 s (3 s while the target hides),
  or the target reloads or is stunned with someone stacked, or 8 s after the first lane order, or at once when
  nobody is left in the base of fire: every stacked NPC steps into its doorway (1 m past the entry), firing, and the
  base of fire is all out shooting for 3 s (cover fight ignores hiding timers and suppression). A breacher is done
  when it sees the target within 3.5 m of the entry or after 4 s.
- **Stacking:** a flanker that stalls within 3 m of its stack spot for 0.5 s (as close as the navmesh lets it)
  counts as stacked.
- **Found:** a flanker hit by its target (it knows where the shot came from: that becomes its last line-of-sight
  position) drops the lane and fights it directly (the post-flank press).
- **Bypassed:** a flanker waiting at a stack whose entry no longer leads to the target (more than 75 degrees off,
  or the target nearer than the entry: it moved, or was heard elsewhere) ends the flank without the 8 s rest, so the
  plan sends it round another way at once if there is one.
- **Spotted:** a flanker that sees the target on the way or at the stack has lost the surprise and turns
  aggressive. From at least 50 degrees round it fights right there; otherwise, within 10 m of its entry, it goes in
  at once, and that starts the squad's breach (assault fire) immediately.
- **After a flank** the NPC is locked into attacking for 5 s (`TacticPress`, above every other gun tactic): no
  cover and no new orders; while it sees the target it takes 2 m sidesteps to spots that still see it (never closer
  than 4 m), and when it loses the line it goes straight for the believed position (to 4 m) until the target shows.
  For 15 s after, cover is allowed only at its distance to the target or closer, never back out the way it came.
  It isn't sent on another flank for 8 s.

### 8.8 Settling, holding and pursuit (as implemented)
- **Settling:** a new fight, the base of fire takes its positions first. The squad counts as settled 1.5 s in once
  half the base of fire is in cover, or after 5 s whatever happens.
- **Holding or running:** the target is anchored where it stands; moving more than 4 m re-anchors it. It is
  **holding** once it has stayed within 4 m for 2 s; a re-anchor that took it 1 m+ further from the squad's centre
  is a **retreat**.
- **Flankers** (target visible) only go once the squad is settled **and** the target is holding. A retreating
  target is pursued instead: every NPC in cover moves up to cover closer to it (the reload advance), flankers still
  on the way are called back, and no new ones go until it holds again.
- **Patience:** while flankers are out and the breach hasn't been called, the base of fire hides more: cover fight
  hidden 1.2 to 2.2 s, out 1.0 to 1.8 s, and a hidden NPC only comes out after 1 s when nobody is shooting. The
  breach brings everyone out at once.

- **Debug:** with an NPC's debug window open the target's lanes are drawn (yellow: target to entry, cyan: entry to
  stack); the window shows lane phase, breach, assault, posture, speed and stance.

---

## 9. Accuracy and fairness

**The firing rule:** every gun NPC that can see its target shoots, whatever tactic it is running
(`CombatBehaviorBase::FireIfTargetVisible`, after the tactic's update). Tactics decide where the NPC stands
and whether it hides; an NPC is only ever silent because it can't see the target (hidden in cover, behind a
wall, reloading, mid-kick) or because it is sprinting between covers (too fast to shoot). It never stands
in view watching.

Every shot rolls a hit chance:

```
hitChance = BaseAccuracy()
          × ramp(timeInContinuousLOS)       // 0.2 → 1.0 over AccuracyRampTime() (≈2 s)
          × (shooterMoving ? 0.6 : 1)
          × (1 - suppression)               // §13.1
          × (targetFastMoving ? 0.5 : 1)    // dash, slide, mantle
          × (targetOnlyHeadVisible ? 0.7 : 1)
          × distanceFalloff(distance)
```

- **Hit:** aim at a body point with small spread and velocity lead (the current lead code).
- **Miss:** aim 0.4–1.2 m off the target, biased to land visibly near it (for the player: in front of
  or beside the camera, never through cover the player is behind), so misses read as near misses.
- Getting hit halves the shooter's ramp.

Player-only rules:
- Only token holders use the formula; others are capped at suppression accuracy (~0.1).
- The first shot after (re)acquiring the player is always a miss.
- Shooters outside the player's camera view: ×0.3 and a doubled ramp time.
- Damage budget (§8.3).
- Aim-up pose (`pistol_aim`) for at least 0.3 s before the first shot of a burst.

Implemented terms (M2): base accuracy, line-of-sight ramp (0.2 → 1 over 2 s; getting hit halves the progress; a
long loss of sight restarts it), shooter moving, shooter outside the camera frustum (also ramps 2× slower), distance
falloff (full to 10 m, 0.6 at 25 m). Not yet: suppression, target speed, cover exposure (M3). Attack tokens
(2 per player target, 1 s minimum hold, bids grow while waiting; an NPC bids only while it is exposed to shoot, or for 1.5 s after, and a holder that went back into hiding bids at half) and the damage budget (leaky bucket, 18 per second,
drawn when a shot is decided to hit) live in `CombatCoordinator`. Tokenless shooters are capped at 0.1.

NPC targets: formula only, with a lower `BaseAccuracy()` for NPC-vs-NPC so those fights last long
enough to look interesting.

---

## 10. Cover

### 10.1 Query
`CoverFinder::FindCover(npc, threats, params) → CoverSpot { position, type (Low / Corner),
cornerSide, peekPosition }`.

Candidates: 24–32 samples on rings around the NPC (2 m to `CoverSearchRadius()`, ~12 m), projected
onto the navmesh (`NavigationSystem::ProjectPointToNavMesh`), plus `cover_point` hints in range.

Score per candidate:
- protection against each known threat, weighted by threat (§4.2); rejected if exposed to a second
  threat that has LOS to it;
- distance from the NPC (prefer short moves);
- distance to the target within the archetype's preferred band;
- angle spread from squadmates (no clumping, natural flanking);
- not reserved, not in a danger zone;
- hint priority bonus.

The top 3 candidates get a path check (`FindSimplePath`, rejected if the path is longer than 2× the
straight distance), and the winner gets one real trace from the threat's eye position at crouch and
stand height, against everything (doors and movers included).

### 10.2 Cover cache
Threat-independent and lazy, so it scales to 10+ NPCs:
- 1 m grid cells on the navmesh, filled on demand, kept for the level.
- Per cell, 16 direction sectors: a short trace (1.5 m) outward at crouch head height (0.9 m) and
  stand head height (1.6 m). Blocked low / clear high → **low cover** for that sector. Blocked at
  both, with a lateral step of 0.8 m that clears standing height → **corner cover** with a side.
- Only `BodyType::World` is cached; the final trace (§10.1) catches dynamic geometry.
- A query only needs the sector toward each threat, then the single real verification trace.
- Same cache classifies the target's hide spot for peek points (§5.2).

### 10.3 `cover_point` hint entity
Registered like `nav_point` (`REGISTER_ENTITY(CoverPoint, "cover_point")`), needs an entry in the
editor's entity definitions.

| Property | Meaning |
|---|---|
| `angle` | direction the cover protects from |
| `type` | `low` / `corner` / `auto` (classify with the cache) |
| `priority` | score bonus (0–1) |

Hints pre-fill their cell and join the candidate pool. They are still verified against the actual
threat, so a misplaced hint can't produce bad behaviour.

---

### 10.4 As implemented (M3)

- **Files:** `CoverMap` (cache), `CoverFinder` (query), `CoverPlanner` (per-NPC assignment, owned by the brain),
  `CoverPoint` (hint entity), reservations in `CombatCoordinator` (mutex protected; the one other exception to
  "main thread only", next to the damage budget).
- **Cache probes** use a 0.2 m sphere trace 1.5 m out at crouch (0.9 m) and standing (1.6 m) head height, per
  sector, on first use. A query has a budget of 48 trace calls (24 probes); sectors it can't afford are skipped.
- **Candidates:** 32 ring samples (3 to 12 m) plus hints, grounded with one trace and snapped to the 1 m cell
  centre. Scored on move distance, distance band to the threat (5 m to 85% of the attack range, preferring the
  desired range), Low-cover bonus, ally spread, hint priority and a little randomness. The best 4 are verified:
  protective trace from the threat's eye, a way to shoot (stand over Low cover, step out sideways 1.8 m for Full cover),
  a path (reached, at most 2.2x the straight distance + 2 m) and a free reservation.
- **Only the primary target is considered**, not every known threat, and the verification trace hits world
  geometry only (not doors or movers).
- **Planner:** searches every 0.8 to 1.4 s while it has no spot (1.2 s after a failed search), re-checks the
  spot against the target every 0.5 s and drops it, searching at once, when the target has flanked it. No cover
  is used when the target is closer than 5 m, when a swordsman is within 10 m, in melee mode, or while stunned or
  busy with a task.
- **Tactics:** `TakeCover` (run, last 2 m crouch-walking for Low cover) and `TacticCoverFight`: hidden 0.5 to
  1.0 s, exposed 1.8 to 3.2 s, for every NPC (the token only sets accuracy). Reloading, a flinch or suppression
  above 0.6 keep or send the NPC back into hiding, except that with no squadmate out shooting it comes out after
  0.3 s and through suppression below 1.0.
  Standing on the step out to a corner peek still counts as in cover. After 5 to 9 s in one spot the NPC moves to
  different cover (not within 2.5 m of the old spot). Strafing stays as the fallback (weight 0.5) when no cover is
  found. No slide into cover yet.
- **What counts as cover:** level geometry and physics props (`World | MainBody`: crates, tables, doors), for the
  cache probes and every verification trace. Characters don't count.
- **Suppression:** a bullet passing within 1.5 m of an NPC of another faction adds 0.35 (once per bullet and
  NPC; checked in `Bullet::LateUpdate`), a hit adds 0.5, decay 0.5 per second, accuracy x (1 - suppression).
  It also ducks strafing NPCs above 0.6. The player's bullets suppress NPCs too.
- **Flinch:** ranged brains only (hybrids and melee NPCs would apply the `stun` clip's root motion). A hit of
  15+ damage, or a 25% chance for smaller ones, plays `stun` for 0.35 s and delays the next shot; 1 s cooldown.
- **Behaviour tree:** the "move for ranged attack" condition now reads `target_inContact` (sees the target, or the
  brain holds cover against an unseen one for up to 8 s) instead of `target_sees`, and its walk to
  `attackPosition` accepts 0.35 m instead of 1.0 m so the NPC really stands in its cover spot. This is the
  stand-in for the squad contact levels of §5 (M4).
- **Editor:** `cover_point` (type auto/low/corner, priority) is in the TrenchBroom `entities.ent`.

## 11. Movement, stance and animation

### 11.1 Move order
When a brain is active, `NpcBase::AsyncUpdate` reads a `CombatMoveOrder` instead of the hard-coded
speed and `lockAtTarget` rules:

```
struct CombatMoveOrder { bool active; float speed;
                         NpcStance stance;         // Standing, Crouched
                         NpcFacing facing;         // Movement, Target, Point
                         vec3 lookAt; };
```

Where to go stays in `attackPosition`: the behaviour tree walks there (§15), so the order carries no
position. The order is reset at the start of each frame (`UpdateTargetFollow`) and the brain writes a new one
while it has a target; without an active order the legacy rules apply (investigation speeds, patrol facing).
Stance changes rebuild the physics body, so `NpcBase::LateUpdate` (main thread) applies them with
`controller->Crouch()` / `UnCrouch()` (stand-up waits for `CanStandUp`); they are skipped while stunned or in ragdoll.

| Movement | Speed |
|---|---|
| Crouch-walk | 2.2 |
| Walk / strafe while aiming | 2.8 (fires up to 3.3) |
| Run | 5.0 |
| Rush | 5.5 |

### 11.2 Clip mapping (existing clips only)

| Need | Clip |
|---|---|
| Directional movement | Locomotion picks by local velocity sector (±10° hysteresis): `walk`/`walk_l`/`walk_r`/`walk_b`, `run`/`run_l`/`run_r`/`run_b`; crouched: `crouch_idle`, `crouch_f/l/r/b`. Upper-body `pistol_*` layer stays on top (`LayeredLerp` from `spine_01`), so crouched aiming works |
| Melee locomotion | `sword_idle`, `sword_walk` |
| Low-cover peek | crouch → stand (stance change) |
| Corner peek | strafe step out and back |
| Slide into cover / evade | `slide` |
| Shove | `sword_kick` (`attack_kick` event) |
| Hit flinch | `stun` played briefly and cut with a quick blend-out |
| Grenade / molotov throw | `sword_attack_main` on the upper body only, release on its `attack_sword_normal` event |
| Point gesture | short `pistol_aim` toward the gestured direction (spine rotation) |
| Reload | weapon lowered (`pistol_hold`) for the reload time |
| Weapon switch (hybrid) | no clip: both weapons hidden for 0.5–0.7 s (the opening), then the new weapon appears |

---

### 11.3 Body rules (as implemented)
`CombatBehaviorBase::ApplyBodyRules` runs after the tactic every tick and decides how the body carries out what
the tactic chose, so no tactic can get it wrong:

- **Posture** (`CombatMoveOrder::posture`, applied in `NpcBase::UpdateAnimations` over the legacy arrest flags):
  aimed (`pistol_aim`) by default, ready (`pistol_idle`) while sprinting somewhere, lowered (`pistol_hold`) only
  while reloading. The gun is never down mid-fight otherwise.
- **Pace:** a gun NPC that can't shoot right now (no line of sight, reloading) and has more than 2 m to go runs,
  facing its path. Moves a tactic marks `careful` (crouching into low cover, the last stretch to a stack spot,
  creeping on a hidden target, a breach, kneeling) are left alone.
- **Stance:** pinned (suppression ≥ 0.6) out of cover and not going anywhere: crouched.
- **Facing and aim** use the **believed target position**: the target while this NPC sees it, the squad's last
  line-of-sight position while a squadmate does, the estimate while it hides, the NPC's own last line-of-sight
  position otherwise (`TargetInfo::lastLOSPosition`, written only on actual sight and passed on with shared
  knowledge; `lastSeenPosition` keeps following the target for 1 s and is for searching). Shots still need sight.
- **Bounding approach** (`CombatMoves::BoundingApproach`, strafe beyond the desired range + 6 m, kite beyond 17 m):
  sprint 1.5 to 2.5 s, stop 0.8 to 1.2 s and shoot, kneeling on half the stops; each NPC aims for its own side
  (±4 m, ±3 m kiting) so the squad fans out. Without a line of sight it simply sprints.
- **Low cover** fights hide 0.8 to 1.5 s and come up 1.5 to 2.5 s, so ducking reads as ducking.
- Sidesteps correct the range by at most one step, so a far NPC never asks for an impossible straight jump.

## 12. Grenades and molotovs

**When:** the target has been Concealed in the same spot for 4 s+, or Visible-but-in-cover for 6 s+,
and flanking hasn't worked; target 6–20 m away and ≥ 5 m from the thrower; no squadmate within the
blast radius of the landing point; coordinator cooldowns allow it (§8.2).

**Throw:** voice line ("Grenade out!" / "Burn him out!"), 0.6 s upper-body wind-up, release on the
event. The ballistic arc is checked with 3–4 trace segments before committing.

**Grenade:** game-side projectile, simple ballistics with bounce (line trace, damping), 2.0–2.5 s
fuse, then spawns the existing `Explosion` (its `Radius` / damage fields). Registers a danger zone
while live.

**Molotov:** shatters on impact and creates a fire zone with the existing `EnemyFire` /
`FireParticleSystem`: damage per second, ~6 s lifetime. Cover spots inside become invalid, which
flushes the target. Registered as a danger zone.

**Reactions:** NPCs (allies and NPC targets) inside a danger zone take `TacticEvade`. Which faction
carries what (e.g. guards grenades, bandits molotovs) is per-behaviour data.

---

## 13. Reactions and morale

### 13.1 Suppression
Bullets from a hostile that pass within 1.5 m of an NPC add +0.35 suppression (checked in
`Bullet::LateUpdate` against nearby observers; segment-to-point distance). Hits add +0.5.
Decays 0.5/s. Above 0.6 the NPC ducks: crouches in cover, or takes cover. It also lowers accuracy (§9).
The player's gun is respected.

### 13.2 Hit flinch
Damage above `FlinchDamage()` (or 25% of other hits) plays the short `stun` flinch and interrupts the
current tactic. It can cancel a melee wind-up ("I interrupted his swing"). 1 s cooldown so the NPC
can't be stun-locked by one gun.

### 13.3 Morale (0..1 per NPC)

| Event | Change |
|---|---|
| Squadmate died within 15 m | −0.15 (−0.25 if it saw it) |
| Squadmate killed by a takedown, headshot or melee finisher | extra −0.1 |
| Own health < 35% | −0.2 |
| Squad lost half its members | −0.2 |
| Hit the target / target below 30% health | +0.1 |
| Reinforcements joined the squad | +0.2 |
| Not under fire | +0.02/s |

- **< 0.45 shaken:** prefers cover and holding, rarely rushes, accuracy ×0.8.
- **< 0.2 broken:** `TacticFallBack`, blind fire, "Fall back!" / "I need help!".
- **Last man broken:** runs to the closest guard (`FindClosestGuard`) and reports, which starts the
  existing guard investigation flow. No surrender (no clip).

---

## 14. Voice lines and gestures

The coordinator lets one NPC per squad speak at a time (2–4 s apart, on top of `globalPhraceDelay`),
with a global cap of 2 simultaneous speakers in large fights.

**Speech** when the line gives the player useful information and the player is within ~30 m
(NPC-vs-NPC fights: rarer). **Gesture/SFX** when speech would be unrealistic (far away, mid-sprint)
or the speech budget is used up.

New events under `vo_base_event_path` (placeholders until recorded):

| Event | Line | Trigger |
|---|---|---|
| `combat_spotted` | "There he is!" | Concealed/Lost → Visible |
| `combat_target_reloading` | "He's reloading!" | target reload seen |
| `combat_reloading` | "Reloading!" | own reload |
| `combat_flank` | "Going left/right!" | flanker assigned |
| `combat_suppress` | "Keep him down!" | suppression starts |
| `combat_moving_up` | "Moving up!" | advance |
| `combat_target_in_cover` | "He's behind cover!" | Concealed starts |
| `combat_not_here` | "He's not here!" | flanker finds the spot empty |
| `combat_where` | "Where'd he go?" | Lost |
| `combat_grenade_out` | "Grenade out!" / "Burn him out!" | throw |
| `combat_grenade_warn` | "Grenade!" | sees a live grenade not from its squad |
| `combat_man_down` | "Man down!" | squadmate died |
| `combat_fall_back` | "Fall back!" | morale broken |
| `combat_need_help` | "I need help!" | last man |

Existing `target_found`, `target_lost`, `arrest_final` stay.

---

## 15. NpcBase and behaviour tree integration

**Blackboard:** add `target_inContact` = contact is Visible or Concealed, or a Lost check is running.

**`general.bt` guard combat branch** (edit in the BT editor):
- "move for ranged attack": condition `target_sees` → `target_inContact`; children become
  `Task Not Doing` → `Move To Position(attackPosition, 0.5 m)`. The brain owns range handling, so the
  inner "not in range → move to `target_lastSeenPosition`" node goes away.
  **As implemented:** that inner node is kept for NPCs without a brain; its condition is
  `attack_skipApproach` = in attack range, or the brain steers (`NpcBase::CombatBrainSteers`: engaged,
  attacking, in contact). Without it every NPC beyond attack range walked straight at the target.
- "move to get LOS" stays as a fallback for NPCs without a brain. NPCs with a brain don't reach it:
  `target_inContact` stays true until the brain itself calls `TargetLost()` (§6.13).

**`NpcBase` changes:**
- `combatBehavior` member, `CreateCombatBehavior()`, tick in `AsyncUpdate`, `Serialize` / `Deserialize`.
- `UpdateTargetAttack` delegates to the brain when one exists; `NpcGuardMelee` / `NpcGuardHybrid`
  keep only body verbs (attack, block, kick, stun, weapon meshes, animation events).
- Movement reads `CombatMoveOrder` (§11.1); `UpdateTargetFollow`'s hard-coded combat speed and the
  combat part of `lockAtTarget` are skipped when a brain is active.
- `FireShot(aimPoint, kind)` replaces the inline bullet spawn (accuracy §9, magazine §6.12).
- Detection decay and target-loss spreading gates (§5.5).

---

## 16. Performance (10+ NPCs)

- **Think rate:** decisions (tactic scoring, threat ranking) every 0.1–0.2 s for NPCs near the camera,
  up to 0.5 s far away or off-screen, staggered by `observer->id` like `UpdateDoorUpdate`. Execution
  (movement, firing) runs every frame.
- **Cover:** ≤ 8 cache-cell fills per NPC per tick; queries mostly hit the cache; path checks only for
  the top 3.
- **Coordinator step:** O(engaged NPCs) per frame, no physics.
- **Near-miss checks:** one segment-vs-nearby-observers test per bullet per frame.
- Profile with Tracy in a 10+ NPC fight at M3 and M4; zones per brain phase
  (`ZoneScopedN("Combat Think")`, `"Combat Cover"`, …).

---

## 17. Engine facts relied on / engine changes

Relied on: `CharacterController::Crouch/UnCrouch/isCrouched/CanStandUp`,
`NavigationSystem::ProjectPointToNavMesh/FindSimplePath/HasLineOfSight`, `Physics::LineTrace/
SphereTrace/CylinderTrace` from worker threads, `AiPerceptionSystem::GetObserversInRadius/EmitSoundAt`,
`Animator` / `AnimationPose::LayeredLerp`, frame order in §3.4.

Engine changes: **none planned.** Any that turn out to be necessary get listed here with the reason.

---

## 18. Milestones

| # | Content | Done when |
|---|---|---|
| M0 | Brain framework (§3), `CombatTargetView`, coordinator stub, save/load, debug UI; `TacticStrafe` and `TacticMeleeDuel` wrap the existing routines (`NpcBase::RunRangedAttack`, `NpcGuardMelee::RunMeleeAttack`); the logic moves into the tactics when M1/M3/M4 rework it | Plays the same as before |
| M1 | Move order, stance, directional and crouch locomotion, LOS bug fix, NPC-vs-NPC melee damage and warnings | Strafing/crouch-walking look right; guards and bandits fight in melee |
| M2 | Threat ranking, tokens, accuracy model and fairness rules, NPC reload | 4+ enemies feel fair; reload is a visible opening |
| M3 | Cover cache + finder + `cover_point`, TakeCover, cover fight, suppression and flinch | Enemies use the level; perf OK with 10+ |
| M4 | Squad belief and contact levels (§5), HoldAngle, Flank, Advance, Rush, WaitForSlot, CheckLastKnown, BT edit | Ducking behind cover keeps the fight going; sneaking away works |
| M5 | Voice lines and gestures, grenade and molotov | Player can hear the plan; camping gets flushed |
| M6 | Morale, FallBack, Evade, BackOff, tuning per archetype | — |

**In-game checklist** (each milestone adds its rows): duck behind low cover for 8 s; sneak away while
concealed; reload in the open with a hybrid at 10 m; rush a gunner with a hybrid nearby; fight 10+
guards; guard vs bandit squad fight; save/load mid-fight in every contact level.

**Debug:** `UpdateDebugUI` shows the current tactic and the top 5 scores, contact, morale,
suppression, token, accuracy; `DebugDraw` shows attack position, chosen cover, peek points, estimate
radius and flank path. A coordinator window lists squads, tokens and beliefs.

---

## 19. Open items

- FMOD events for the voice lines in §14 (placeholders until then).
- Grenade and molotov models and sounds (placeholder mesh until then).
- Whether running should emit a perception sound so NPCs can hear a target leaving cover (§5.4).
- Whether `stun` has root motion that would push the NPC during a short flinch.
- Which factions carry grenades vs molotovs.
- All numbers in this document are starting values for tuning in game.

## 20. Small fixes to make along the way

- `NpcGuardHybrid::UpdateWeaponMeshVisibility` reads `weaponMesh->Visible` before its null check.
- `NpcBase::UpdateTargetAttack` / `UpdateTargetFollow` use the target entity without null checks.
- `FindAttackLocation`'s neighbour list includes the NPC's own observer and civilians.
- `NpcBanditBase` constructor body calls `NpcBase();`, which only builds a temporary.

## 21. Decisions log

| Date | Decision |
|---|---|
| 2026-10-31 | Cover found at runtime, with optional `cover_point` hints |
| 2026-10-31 | Voice lines and gestures/SFX, chosen by situation |
| 2026-10-31 | Existing animation clips only |
| 2026-10-31 | Encounters up to 10+ NPCs |
| 2026-10-31 | Grenades and molotovs included |
| 2026-10-31 | Targets can be other NPCs; fairness rules only protect the player |
| 2026-10-31 | Suppressive fire at the last known position |
| 2026-10-31 | Losing sight behind cover → Concealed, decided per squad; search only after evidence or timeout |

## 22. Tuning (`CombatTuning`)

Every gameplay number of the combat AI lives in `Combat/CombatTuning.h`, grouped by system (movement, shot,
shove, fire, brain, hybrid, tokens, belief, squad, cover, coverFight, approach, bounding, strafe, kite,
regainSight, press, search, melee). The code reads them with `CombatTuning::Get()`; a difficulty setting changes
them with `CombatTuning::Set(values)` (start from `CombatTuning::Get()` or a default-constructed
`CombatTuningValues`) and `CombatTuning::Reset()` restores the defaults.

- Call `Set` between frames (menu, level load): the brains read the values on worker threads.
- The tactic weights (`brain.strafeWeight`, `brain.suppressLastKnownWeight`) are read when an NPC is created.
- Not in it: algorithm internals (cover search rings and sample counts, trace heights, threat ranking factors in
  `CombatThreat.cpp`, the melee body's own state machine timings in `NpcGuardMelee`).
