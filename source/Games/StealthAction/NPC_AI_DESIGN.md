# NPC AI — Design and Rewrite Plan (StealthAction only)

Status: design, 2026-11-23. Replaces the NPC parts of `COMBAT_AI_DESIGN.md` (that document is superseded
once milestone M3 lands). The code lives in StealthAction only: it is not copied to ShooterGame (user
decision, M1).

The game: a first-person immersive sim, "Cyberpunk-Dishonored about vampires". The player is a vampire
with powers (possession, stealing a likeness, teleport, feeding, mesmerize, mist form) and can build
toward stealth, blades, guns or powers. Every one of those is a full way to play, and they mix freely:
start quiet, go loud, break contact, vanish, strike again.

---

## 0. Pillars

1. **Readable, consistent, exploitable.** Same situation → same timing and same reaction, every time.
   Randomness is cosmetic only (which bark line, which clip variant). The player learns the rules and
   uses them (Splinter Cell Blacklist, Arkane, Hitman).
2. **No psychic knowledge.** An NPC knows only what it perceived or was told. Being told needs a real
   message (a shout, a report, a radio call, an alarm), so killing the messenger works.
3. **Loud play is a full playstyle.** Fights are fun with any build. Every enemy type has a strength,
   a weakness and a tell. The squad, not one NPC's cleverness, makes the pressure.
4. **Stealth and combat flow both ways.** A fight can start anywhere and stay local; breaking line of
   sight and moving away (powers make it easy) sends NPCs into chase, then search, so stealth is always
   one good escape away.
5. **The vampire is frightening.** Powers, feeding and fast kills break enemy nerve; ordinary people
   don't understand what they saw, hunters do.
6. **Elegant, not clever.** A few concepts with one job each (§4). No second copies of state, no hidden
   timers in files the code can't see, no system that fights another.

---

## 1. Decisions (from the design Q&A, 2026-11-23)

| Topic | Decision |
|---|---|
| Games | StealthAction only; ShooterGame keeps its own NPC code and is not synced |
| Depth | New brain (perception, knowledge, alert, investigation, combat, BT wiring); keep the body parts that work (controller, animator, ragdoll/stand-up, gore, weapon meshes, task points) |
| Brain driver | C++ skills chosen by priority; the engine behaviour tree is used only by a `Scripted` skill for authored sequences |
| Combat layer | Full replace of `Entities/Npc/Combat/*` (coordinator, tactics, cover finder, approach map, lanes) |
| Engine | Upgrade the engine `AiPerception` (shared with other games); further engine changes only where listed (§24) |
| Feedback | Detection meter over visible NPCs **and** screen-edge indicators for off-screen ones |
| Darkness | Affects detection a little (distance, stance and motion matter more) |
| Calming down | Per event: minor scares return to normal, bodies / fights / powers leave the area permanently wary |
| Bodies | Plan dead and unconscious bodies, waking up, moved and hidden bodies now (player verbs come later) |
| Likeness | A stolen face is that specific person: strangers accept it, people who know them notice odd behaviour, seeing the real one blows it |
| Possession | The host is treated normally unless it acts out of character; per-NPC setup; witnessing a power use blows that cover for the witness (and whoever they tell) |
| Teleport | Unseen in motion: only where you land matters |
| Witnessed powers | Possession, likeness theft, feeding, mesmerize, mist form |
| Vampire lore | Depends on faction: hunters / church know; commoners and ordinary guards are confused and scared, then learn from rumours |
| Civilians | Scream (fixed small radius), flee, report to a guard; silence the witness before it arrives and nothing is reported |
| Factions | Relation matrix (asymmetric, changeable at runtime) + player reputation per faction |
| Animations | Existing clips only |
| Scale | ~60 NPCs alive per level, 10+ in one fight |
| Co-op | Designed for several players from the start |
| Old saves | May break |
| Areas | Public / restricted (warn, escort, then attack) / hostile zones; a likeness of someone allowed there grants access |
| NPC types | VIP + bodyguards, vampire hunters, officers / leaders, dogs |
| Melee feel | Dishonored / Dark Messiah: one attacker at a time (two at most), slow clear wind-ups, attacks commit, parry gives a stagger, kick breaks a turtling player, the others circle |
| Routines | Keep task points and schedules; the brain pauses and resumes them through one clean interface |
| Voice | Plan a full tiered bark list (event names; recordings later) |
| Enemy toughness | Cyberpunk-like: mooks die fast, elites and heavies take effort (armour, weak points); builds change it a lot |
| Feeding | Both: stealth feeding is quiet and full; combat feeding on a staggered enemy is fast, partial and loud |
| Vanishing | Powers make it easy: break line of sight and move away → chase → search |
| Ambush | Instant takedown from behind on any NPC that does not see you, even mid-fight (so NPCs must cover each other's backs, §17.6) |
| Escalation | Alarms pull designer-placed reserve squads and hunters; finite; silencing the runner prevents it |
| Faction style | Division 2 traits per faction (aggression, organisation, training, tech) shape how the same archetypes fight |
| NPC explosives | Yes, with long readable tells |
| Accuracy | Two accurate shooters per player at a time, the others pressure with near misses; accuracy falls while the player moves fast; never accurate from off-screen |
| Infighting | Only factions hostile to each other fight; NPC friendly fire within a faction does nothing |
| Counters | Hunter gear, back-covering, supernatural enemies, adapting to a repeated trick within one fight |
| Evasion | Trained factions dodge (sidestep, slide); everyone has the pain ladder |
| Difficulty | Scales perception, aggression, lethality and chase length |
| Fight spread | Local by senses and messages, plus area alarms (alarm area knows roughly where, not exactly) |
| Morale | Low morale: some flee (often to raise an alarm); cornered and hopeless ones surrender; hunters and fanatics never |
| NPC guns | Any firearm, held with the pistol clips until proper clips exist |

Earlier user rules that still hold: every gunman who sees its target shoots (only token holders aim to
hit); NPCs never stand in view watching; sprinting between positions is fine; weapon type may only add
aggression. Think through moment-to-moment body behaviour (posture, pace, crouch, facing), not only
squad plans; flanking must follow real routes through doorways.

---

## 2. Why rewrite: what is wrong today

### 2.1 The "problem wrong facing" save

At game time 88.0 s in `aitest`, the player stands at (9.9, 6.8). Every living guard believes the
player is at (31.4, 18.8), where the player was about 20 s earlier, with a line-of-sight timestamp of
*now*.

Cause:

1. Dead NPCs keep their combat brain registered. `CombatCoordinator::UpdateBeliefs`
   ([CombatCoordinator.cpp:1163](Entities/Npc/Combat/CombatCoordinator.cpp:1163)) groups every
   brain with `IsEngaged()` ([:1171](Entities/Npc/Combat/CombatCoordinator.cpp:1171)). A dead NPC's brain stops updating, so
   its target view, and `target_sees` (true when it died), are frozen.
2. The first "seer" ([:1207](Entities/Npc/Combat/CombatCoordinator.cpp:1207)) is a dead guard. The squad belief stays `Visible`, and
   every frame `ShareWithSquad(..., view.position, true)` ([:1234](Entities/Npc/Combat/CombatCoordinator.cpp:1234)) sends the frozen
   position to every living member as a fresh line-of-sight sighting.
3. Living guards face that position (or the bend of their path toward it) while the strafe tactic
   picks spots around the player's *real* position (`FindAttackLocation` reads the target entity
   directly). The vision cone (±60°, from the animated head bone,
   [NpcBase.cpp:1119](Entities/Npc/NpcBase.cpp:1119)) never contains the player.

### 2.2 The structural problems behind it

| Problem | Where |
|---|---|
| Four copies of "where is the target": `knownTargets`, the `target_*` primary copies, the squad `TargetBelief`, the behaviour-tree blackboard. Each has its own lifetime and its own cheats (1 s true-position tracking, [NpcBase.cpp:1774](Entities/Npc/NpcBase.cpp:1774)) | NpcBase, CombatCoordinator, general.bt |
| State is ~20 booleans (`follow`, `underArrest`, `attack`, `sees`, `seesAndDetected`, `report_to_guard`, `found_guard`, `DoingTask`, ...) plus comparisons on the order of the `Crime` and `InvestigationReason` enums | NpcBase.h |
| Decisions are split between C++ and a 87 KB tree whose wait nodes are hidden timers ("wait 5 s, then lose the target") the combat code can't see | general.bt, `UpdateBT` ([:1029](Entities/Npc/NpcBase.cpp:1029)) |
| Facing has many writers: locomotion accumulation, `lockAtTarget`, investigation orientation, root motion, a `length < 0.5 → 0.3` hack ([:854](Entities/Npc/NpcBase.cpp:854)); nobody decides what the NPC looks at | `NpcBase::AsyncUpdate` |
| Vision is one binary cone with one ray, from the head bone's animated axis; detection speed ignores distance, light and stance | engine `Observer`, `UpdateObserver` |
| NPCs write each other directly (knowledge sharing, target-lost spreading, investigations started on other NPCs) | `ShareTargetKnowlageWithFinal`, `StopTargetFollowFinal`, `ApplyPointDamage` |
| Search walks random points in a circle for 10 s | `SearchInvestigationBase` |
| Melee: the wind-up is a frozen pause, the NPC turns toward the player through the whole swing (`IsRotationAllowedDuringRootMotion` returns true while the attack runs, [NpcGuardMelee.cpp:33](Entities/Npc/NpcGuardMelee.cpp:33)), the hit test is one 1 m sphere trace straight ahead ([:281](Entities/Npc/NpcGuardMelee.cpp:281)), a new spot every 0.28 s ([:128](Entities/Npc/NpcGuardMelee.cpp:128)) makes it jitter, blocking is a dice roll with instant reaction ([:420](Entities/Npc/NpcGuardMelee.cpp:420)) | NpcGuardMelee |
| Civilians: run to the nearest guard or flee; no scream, no cower, no staged reaction to a body | BT civilian branch |
| The combat coordinator is stepped from every NPC's `Update` ([:579](Entities/Npc/NpcBase.cpp:579)) | NpcBase |

---

## 3. What other games do, and what we take

| Idea | Source | Used in |
|---|---|---|
| Vision zones instead of one cone; a detection timer scaled by distance, light and NPC state; consistent timing, varied barks | Splinter Cell Blacklist (Walsh, Game AI Pro 2 ch. 28) | §5, §6 |
| Keyhole vision (angle shrinks with distance); timer runs up while seen, down while not; one visibility point: chest while sneaking, head in combat | The Last of Us (McIntosh, Game AI Pro 2 ch. 34) | §5 |
| A shared, timestamped last-known-position record; no cheating; the "5 m snuck away" check | The Last of Us | §6, §12 |
| Search map: cells reachable but not visible, spreading over time, cleared by what NPCs see | The Last of Us | §11.4 |
| Chase by flooding an influence map from the last known position with a barrier behind it, then a slow search; chase length per difficulty | Dishonored 2 (Couvidou, Game AI Pro Online 2021) | §11.3 |
| Area/choke graph: NPCs without line of sight watch doors and windows; sound distance along the area path | Blacklist (TEAS) | §5.4, §11.5 |
| Tiered barks: specific the first times, generic later | Blacklist | §18.2 |
| Off-screen NPCs hear the player's minor sounds at half range | Blacklist | §5.4 |
| Disappearing-NPC trick: in earshot 10 s, then silent 5 s → investigate | Blacklist | §13.3 |
| Alert ladder with thresholds and per-level decay; weapons drawn never drop back to unaware; team relations matrix, asymmetric, runtime changes | The Dark Mod wiki | §6.4, §15 |
| Knowledge services, situations with roles, behaviour zones for crowds (look / avoid / alert / scared / prone, strongest wins), panic flows to exits, VIP escort / evacuate / escort-search | Hitman (AI and Games) | §10, §14 |
| Role requests to a coordinator (Flanker, Approacher, Investigator, Opportunistic Shooter); post selectors (criteria multiplied) | The Last of Us | §10, §11.2 |
| Attack tokens per attack type, stealable; accuracy weighted by player movement; pain ladder to a finisher; no new attacks during a finisher | DOOM 2016 (AI and Games) | §17 |
| Faction traits (aggression, organisation, training, tech) over shared archetypes; reactions to being shot; readable wind-ups | The Division 2 (AI and Games) | §17.2 |
| Longer melee tells, tracking stops at a tagged point, attack start conditions | The Last of Us Part II (Chou, GDC 2021) | §17.8 |
| One attack every 2–3 s across the group, near/far groups, no attacks from off-screen, waiting enemies circle | Vossen, melee enemy design | §17.8 |
| Enemies counter the player's powers with their own | Cyberpunk 2077 2.0 | §16.6, §17.6 |
| Stolen face: victim unconscious, moving the victim breaks it, guards alerted end it, animals see through | Dishonored: Death of the Outsider (Semblance) | §16 |
| Enforcers who see through disguises; trespass → escort; suspicious actions | Hitman | §15.3, §16 |
| Sight radius / lose-sight radius hysteresis, stimulus age, team affiliation | Unreal AI Perception docs | §5 |
| Factions with enemy / neutral / friendly; coward behaviour | Emerald AI docs | §15 |
| Patrol / suspicious / alert / search ladders with a fill-and-drain meter, stepwise cool-down | Fab "Stealth AI Kit", StraySpark UE5 articles | §6 |
| Inconsistent rules ruin a multi-playstyle game | Deus Ex: Human Revolution post-mortem | Pillar 1 |

---

## 4. Architecture

```
            ┌───────────────── engine AiPerception (upgraded, §5) ─────────────────┐
            │  Observer: zones, gaze, results with strength  ·  sounds with loudness │
            └───────────────────────────────┬──────────────────────────────────────┘
                                            │ sight / hearing / smell / aura results
   ┌────────────────────────── one NPC (worker thread) ───────────────────────────┐
   │  Mind      memory records · awareness meters · alert · fear · concerns (§6)  │
   │    │                                                                          │
   │  Skills    each skill: Want(ctx) → priority; the best one runs (§8)           │
   │    │                                                                          │
   │  Body      one BodyIntent per frame: move, gait, stance, gaze, weapon, action │
   │            → CharacterController, animator, observer eye/gaze (§9)            │
   └──────────────┬───────────────────────────────────────────────────▲───────────┘
                  │ messages, role requests (queued)                   │ messages, roles granted
   ┌──────────────▼──────────── NpcDirector (main thread, once a frame) ┴──────────┐
   │  delivers messages (§7) · situations + roles (§10) · search maps, chase floods │
   │  area wariness, alarms, reinforcements · faction table + reputation (§15)      │
   └────────────────────────────────────────────────────────────────────────────────┘
   World data: AreaZone, AlarmPoint, SafeRoom, ReserveSquad, task points · NavGrid (§11)
```

### 4.1 The rules that keep it simple

1. **One source of truth for knowledge**: an NPC's `NpcMemory`. Situations read members' memories;
   they never keep a copy. Nothing else stores "where the target is".
2. **No psychic reads**: code may read a target's real position only inside perception, or when the
   NPC currently perceives it (this frame's sight result). Everything else uses memory records.
3. **NPCs never write each other.** Everything that crosses NPCs (telling, warning, damage reactions,
   role requests) is a message queued from the worker thread and delivered by `NpcDirector` on the main
   thread.
4. **Off means off.** Dead, unconscious, mesmerized, possessed or disoriented NPCs don't perceive,
   don't send messages and hold no roles (their situation memberships end at once).
5. **The Body has one writer per frame**: the running skill. Body reflexes (flinch, stagger,
   knockdown, hazard step) may override it.
6. **All timings are data** (`GameData/ai/*.json`), the same every time; difficulty scales them.

### 4.2 Frame order

1. Engine: `AiPerceptionSystem::Update` (parallel, observers time-sliced).
2. `NpcBase::AsyncUpdate` (parallel): read perception results → Mind → skill selection and update →
   BodyIntent → controller, animation, observer eye/gaze for the next perception pass. Outgoing messages
   and role requests go into the NPC's outbox.
3. `NpcDirector::Step` (main thread, once a frame): deliver messages, update situations, roles, search
   maps (time-sliced), alarms, area wariness.
4. `NpcBase::LateUpdate` (main thread): physics-rebuilding changes (crouch), damage application.

`NpcDirector` is an `Entity` placed by the game at level load (like `NpcSimulationManager`), with
`SaveGame = true`, so situations, area state and reputation save with the level.

### 4.3 Classes (game code, `Entities/Npc/`, plain members, engine idioms)

| Class | Job |
|---|---|
| `NpcBase : Entity` | Owns the parts below; damage, death, ragdoll, save/load. Map class names stay (`npc_guard`, `npc_civilian`, `npc_guard_melee`, `npc_guard_hybrid`, `npc_bandit`) and pick a profile |
| `NpcProfile` (data) | Role, archetype, faction, voice, traits, identity setup, tuning ids (§19) |
| `NpcMind` | Memory, awareness meters, alert, fear, concerns, interpretation rules |
| `NpcSkill` + skills | Behaviour (§8) |
| `NpcBody` | BodyIntent → controller, locomotion, weapon posture, gaze, actions, doors, melee moves |
| `NpcDirector` | Message delivery, situations, roles, area state, alarms, reinforcements, faction table |
| `NpcSituation` + kinds | Shared plan around one subject (§10) |
| `SearchMap`, `ChaseFlood`, `PostQuery` | Spatial reasoning on the engine `NavGrid` (§11) |
| `FactionTable` | Relations, traits, lore, reputation (§15) |
| `AreaZone`, `AlarmPoint`, `SafeRoom`, `ReserveSquad` | Map entities (§15, §17.11) |

---

## 5. Senses (engine `AiPerception` upgrade)

### 5.1 Vision zones

Each `Observer` gets a `VisionProfile` (data per profile, scaled by alert, §6.4):

| Zone | Shape | Default (calm) | Strength factor |
|---|---|---|---|
| Focus | keyhole: half-angle 30° at range, widening to 60° inside 5 m | 25 m (40 m in combat) | 1.0 |
| Peripheral | half-angle 100°, short | 8 m | 0.35 |
| Near | sphere, 360° | 1.5 m, only a moving or noisy target | 0.6 |

Lose-sight hysteresis: a target already being watched stays in its zone 10% beyond the edge.

The observer eye is the head bone *position* with the **gaze direction from the Body** (§9.2), not
the head bone's animated axis. What the NPC visibly looks at is exactly what it sees.

### 5.2 Visibility of a target

`ObservationTarget` gets sample points (chest, head) set by its owner each frame, and a
`visibility` scale set by its owner (stance, mist form, power effects).

- Rays: to the chest point while the observer is not in combat; to the chest, then the head, in combat
  (The Last of Us: favours the sneaking player, keeps combat sharp). The first unblocked ray counts.
- Light: the engine samples the BSP light volume at the target's chest (throttled, 4 Hz per target) and
  stores `lightLevel` 0..1. It enters detection "a little" (§6.2).
- Result per (observer, target) per update: `SightResult { target, strength 0..1, zone, distance,
  point }`. `strength` = zone factor × distance factor × visibility × light factor.
  `visibleTargets` stays (targets with strength > 0) for existing users (SlasherGame).

### 5.3 Special senses (results of the same shape)

| Sense | Who | Rule |
|---|---|---|
| Smell | dogs | 10 m, ignores light, likeness, possession and mist; blocked by walls; follows a scent trail of the player's real path for the last 20 s (smell is honest, not a cheat) |
| Aura | hunters | 6 m, through likeness and possession (reveals the true identity), not through walls |
| Touch | everyone | bumping a body, being grabbed: instant, strength 1 |

### 5.4 Hearing

`AiPerceptionSystem::EmitSound(SoundEvent { position, loudness (m of radius), type, causerId,
apparentId })`.

- Propagation (decided: plain, no portal graph): straight from the source to the listener. Every solid
  world wall the segment crosses adds a penalty to the distance (6 m each, at most 3 counted; data), so a
  wall muffles a sound instead of stopping it. Heard if distance + penalties ≤ loudness; the loudness at
  the listener is `1 − effective distance / loudness`.
- Delivered: `HeardSound { perceivedPosition, loudnessAtListener 0..1, type, causerId, apparentId }`.
  The perceived position is exact for close or loud sounds and blurred by up to 25% of the distance for
  faint ones.
- Fairness: for player-caused minor sounds (footsteps, landing, door, thrown object) a listener that is
  off every player's screen and farther than 6 m hears at half radius (Blacklist).

Sound sources (types and radii are data): walk 0 / run 8 m / sprint 12 m / landing 6 m / crouch 0;
door 6 m; thrown object impact 10 m; melee clash 15 m; suppressed shot 10 m; gunshot 40 m; explosion
60 m; body falling 6 m; scream 15 m (also a message, §7); feeding (stealth) 4 m, (combat) 15 m;
breaking glass 20 m.

### 5.5 Scheduling

Observers update every 4th frame (as now), meters integrate with the real elapsed time. Candidate
targets are filtered by range and BSP cluster visibility before any ray. Budget: about 200 rays a
frame for 60 NPCs (§21).

---

## 6. Mind

### 6.1 Memory records (the single source of truth)

```
Record {
    subject      entity id (person, body, object) or event id
    kind         Person | Body | Event | Identity | Hazard
    position, velocity
    time         when it was true (kept as-is when told on: never re-stamped)
    source       Saw | Heard | Told | Felt (damage)
    certainty    0..1 (seen 1, heard by loudness, told 0.8 of the teller's)
    detail       concern type, apparent identity, body state and cause, culprit ...
}
```

One record per (subject, kind); the newest `time` wins. Person records about hostiles stay until the
subject is confirmed dead or the situation ends; event records expire (30–120 s, data).

This is what replaces `knownTargets`, the `target_*` copies, `TargetBelief` and the blackboard.

As built (M1): a meter holds only the strength of the awareness (the position of its last perception lives for one update
and is not saved), and a concern holds no position: it points at its subject's Person record
(`NpcMind::ConcernPosition`). Hearing an alarm sound and being hit write that record, so a concern never exists without one.

### 6.2 Awareness meters

One meter per (observer, subject), 0..1. Per sense update:

```
fill/s = 0.6 × strength × motion × stateMul × acuity × suspiciousness
```

| Factor | Values (initial tuning) |
|---|---|
| strength | from §5.2: zone (focus 1, peripheral 0.35, near 0.6) × distance (1.0 up to 6 m, falling linearly to 0.2 at the zone range) × visibility × light (0.6 dark .. 1.0 lit) |
| motion | still 0.7, walk 1.0, run 1.6, crouch-walk 0.6, crouched still 0.4 |
| stateMul | Calm 1.0, Wary 1.5, Alarmed 2.0, Combat 6.0 |
| acuity | profile (default 1.0; dogs, hunters, officers higher) |
| suspiciousness | §6.3: 0 for someone who looks legitimate, up to 1 for a known hostile |

Thresholds: **0.35 Notice** (glance, "Hm?"), **0.7 Suspect** (investigate or confront), **1.0
Identify** (react to what it is). Decay: hold 1.5 s after the last stimulus, then −0.2/s (−0.1/s when
Alarmed or in Combat). Sounds add directly: +loudness × 0.5 × suspiciousness of the sound type.

Examples (calm guard, s = 1, focus zone unless stated): lit, walking at 5 m → 1.7 s to Identify;
at 15 m → 2.7 s; crouch-walking in the dark at 10 m → 5.6 s; walking in the peripheral zone at 5 m →
4.8 s; in combat at 15 m → 0.35 s.

### 6.3 Suspiciousness (identity and context)

What the observer thinks it is looking at decides how fast the meter fills:

| Observed | s |
|---|---|
| A known hostile (hostile faction or reputation, or a compromised identity) | 1.0 |
| A stranger in a hostile zone | 1.0 |
| A stranger in a restricted zone | 0.8 |
| A weapon drawn where that is not normal for that identity | 0.6 |
| A legitimate identity acting out of character (§16.3) | 0.2–0.6 (×2 for acquaintances) |
| A body, blood, a dropped weapon | 1.0 (identified as "body", not as a hostile) |
| A legitimate identity behaving normally | 0 (glanced at, never suspected) |

### 6.4 Alert (per NPC) and area wariness

| Alert | Means | Senses | Body |
|---|---|---|---|
| Calm | nothing wrong | ×1.0 | routine, weapon holstered |
| Wary | something happened here | ×1.5 | routine with look-arounds, weapon held, pairs patrol closer |
| Alarmed | an intruder or threat is around, location unknown | ×2.0 | investigate, search, chase |
| Combat | a located hostile (fresh record) | ×6.0 | fight |

Escalation is instant (a concern sets it). Cool-down is stepwise: Combat → Alarmed after 8 s without a
fresh record and the chase is over; Alarmed → Wary when the search ends; Wary → Calm after 60 s,
only down to the **area floor**.

The area floor (per `AreaZone` wariness area, else a 30 m circle around the event) is set by events and
never lowered by time: noise, curiosity, a trespass warning → no floor; a body found, a fight, a power
witnessed → Wary for the rest of the level; an alarm raised → Alarmed for 3 min, then Wary.

### 6.5 Fear

One meter 0..1 for everyone; roles set the thresholds. Inputs (data): ally killed nearby +0.15
(+0.3 if fed upon), a power witnessed +0.2 (ignorant) / +0.05 (lore-aware), gunfire near +0.1,
heavy damage taken +0.2, officer killed +0.25 to the squad, outnumbered +0.1/s; officer nearby −0.1/s,
safety −0.05/s.

| Role | Uneasy | Scared | Panic / break |
|---|---|---|---|
| Civilian | 0.3 | 0.6 | 1.0 |
| Guard | — | 0.6 (shaken: prefers cover, worse aim) | 0.85 (flee or surrender, §17.10) |
| Hunter, fanatic, supernatural | — | — | never |

### 6.6 Concerns (interpretation rules)

A small C++ table turns an identified percept plus context into a concern. Concerns drive skills and
situations; the table is the only place where "what does this mean" lives.

| Identified | Context | Concern |
|---|---|---|
| Hostile person | any | Threat → Combat |
| Stranger | restricted zone, Calm / Wary | Trespass → Confront |
| Stranger | restricted zone, Alarmed / Combat | Threat |
| Drawn weapon | public, guard observer | Brandish → Confront |
| Drawn weapon | civilian observer | Fear +0.4 |
| Dead body | not in Combat | BodyFound → Alarmed, area floor Wary |
| Dead body | in Combat | noted only (context, Blacklist) |
| Drained body | lore-aware faction | VampireSign → Alarmed, request hunters |
| Unconscious body | guard | WakeUp |
| Noise (minor) | Calm / Wary | Curious → Investigate |
| Gunshot, explosion, scream | any | Alarm → search the source |
| Power use | ignorant | Unnatural → fear, report, guards treat as an assault |
| Power use | lore-aware or rumoured | Vampire → Combat, call hunters |
| Possession / likeness theft | any | Identity compromised (§16.4) + power-use concern |
| Ally missing | §13.3 | Missing → Investigate |
| Changed object (door, light) | on the NPC's routine path | Curious |
| Damage taken | any | Threat (attacker if seen, else Alarm at the shot's origin) |

---

## 7. Messages (how NPCs tell each other)

| Kind | Reach | Who can send | Cost |
|---|---|---|---|
| Shout | 15 m sound (muffled by walls like §5.4; louder voices further) | anyone | instant; a bark plays |
| Report | face to face, ≤ 2.5 m | anyone | 2 s talking, both stand |
| Radio / whistle | own squad / area | officers, hunters, organised factions | 1 s, a bark |
| Alarm | whole alarm area | anyone at an `AlarmPoint` (2 s to ring) | area Alarmed, reserves dispatched |

Payload: memory records (with their original time and certainty × 0.8), concerns, situation
invitations. The receiver merges records by the newest-time rule and reacts through its own Mind. A
receiver never gets more than the sender knew. Dead, unconscious, mesmerized, possessed or disoriented
NPCs can't send (this alone removes the bug in §2.1).

---

## 8. Skills

```
class NpcSkill {
    virtual const char* Id() const;
    virtual float Want(const NpcContext& ctx) const;   // 0 = not now; else priority
    virtual void Start(NpcContext&), Update(NpcContext&, float dt), Stop(NpcContext&);
    virtual float MinCommit() const;                    // seconds kept unless a higher band wants
    virtual nlohmann::json Serialize() const; virtual void Deserialize(const nlohmann::json&);
};
```

The highest `Want` runs. A running skill keeps a +0.1 bonus inside its `MinCommit`. Priority bands keep
the order obvious:

| Band | Skills |
|---|---|
| 90 survival | Flee, Surrender, AvoidHazard (fire, a lit fuse), Cower |
| 70 combat | Fight (archetype style, §17) |
| 60 duty | RaiseAlarm, Report, Evacuate (VIP) |
| 50 alarmed | Chase, Search, TendBody |
| 40 curious | Investigate |
| 30 social | Confront (warn, escort, detain), Greet |
| 20 escort | Bodyguard |
| 10 routine | Routine (task points and schedules) |
| data | Scripted (runs a behaviour tree asset; priority set by the designer) |

Roles decide which skills an NPC has (civilians: Flee, Cower, Report, Investigate-light, Routine;
guards: everything but Cower; dogs: Investigate, Search, Chase, Fight, Routine).

**Attention is not a skill.** Glancing at something noticed is done by the Body's gaze (§9.2) under
whatever skill runs, so a working guard can glance at a noise without dropping his job.

**Routine** wraps the existing task points: `Pause()` when another skill takes over, `Resume()` when
Routine wins again. The task point's lock-position and animation are released on pause. No
`DoingTask` flags, no blackboard.

**Not skills, NPC states**: Possessed (the player drives the Body; Mind off), Mesmerized (stands,
Mind off), Disoriented (after possession, 4 s stun), Unconscious, Dead. Each ends situation memberships.

---

## 9. Body

### 9.1 BodyIntent

```
BodyIntent {
    destination, acceptRadius, gait (stand | walk | run | sprint | crouch-walk)
    stance (stand | crouch), facing (path | point | gaze)
    gaze { point, priority, glanceAllowed }
    weapon (holstered | held | ready | aimed | lowered)
    action (clip request), lockRotation (melee commit)
}
```

Written once per frame by the running skill, then applied: path following (existing
`PathFollowQuery`), controller velocity, locomotion clip choice (existing `PickLocomotionAnimation`),
weapon layer, spine aim, doors on the path (existing `UpdateDoorUpdate`).

### 9.2 Gaze (the facing fix)

What the NPC looks at is chosen, not a side effect:

1. the attack target in sight (fighting);
2. a noticed stimulus: a glance of 0.8–1.5 s if the skill allows glances;
3. the skill's look point: a search frontier cell (§11.4), a watched door, an investigation point;
4. the path ahead;
5. the routine's orientation.

Head and spine turn up to ±70° from the body (existing `spineRotation` plus head); beyond that the body
turns (240°/s at most, 120°/s while walking). The perception eye uses this gaze direction, so the
player can read exactly where an NPC can see by looking at its head.

### 9.3 Clips (existing only)

| Behaviour | Clips |
|---|---|
| Idle, walk, run, strafe, backpedal | idle, walk, run, walk_l/r/b, run_l/r/b |
| Crouch, crouch-walk | crouch_idle, crouch_f/b/l/r |
| Dodge (trained factions) | slide |
| Wary / searching idle | caution_idle (`animations/npc/caution_idle.glb`) |
| Look around, check a corner | idle or caution_idle + procedural head / spine sweep |
| Examine a body, wake someone | crouch_idle (+ stand_up on the woken NPC) |
| Cower, plead, hands up, surrender | scared (+ crouch_idle for cowering) |
| Wounded on the ground | inPain |
| Gun: hold, ready, aim tell, fire | pistol_hold, pistol_idle, pistol_aim, pistol_fire (also for rifles and shotguns for now) |
| Melee guard, walk, attacks, kick, block | sword_idle, sword_walk, sword_attack_main, sword_attack_break, sword_kick, sword_block |
| Flinch, falter, stagger, disoriented, parried | stun (cut short, full, or looped) |
| Knockdown, stand up | ragdoll + standUp |
| Sit, smoke (routines) | sit_down, sitting_idle, stand_up, smoking |

Melee wind-ups use the attack clips played slowly up to a commit marker (§17.8). Commit markers are
added as events to the `.skmm` metadata, not new clips.

---

## 10. Situations and roles (group play)

A **situation** is a shared plan around one subject: a noise to check, an intruder to find, a target to
fight, a VIP to protect. The first NPC that meets the concern opens it; others join when they learn of
the subject (their own senses or a message). A situation holds planning data (roles, the search map,
the chase flood, claimed posts) and **no knowledge**: it reads its members' memories (the freshest
record among them).

| Situation | Opened by | Roles (counts are data) |
|---|---|---|
| Investigate | Curious concern | Investigator 1, Backup 0–1 (Wary: 1); the rest stay on routine and glance |
| Search | target lost, body found, alarm | Searcher n, Watcher (chokes) 1–2, Checker 1, Leader (officer) |
| Combat | Threat concern, per hostile target (each player separately) | MeleeAttack 1 (2 on hard), AccurateShot 2, Throw 1 / 12 s, Flank 1, Approach 1, Counter (hunter) 1, AlarmRun 1, pairs for back-covering |
| Escort | a VIP's bodyguards | slots in formation, outward gaze sectors |
| Evacuate | threat near a VIP | Lead, Rear, flanks |
| Panic | civilians in one area | none (shared exits and flee flows) |

Roles are **requested** (`RequestRole(role)` → granted or denied, released on stop). Exclusive roles
(Flank, Checker, AlarmRun) go to the best candidate the situation picks each second (shortest valid
route), as in The Last of Us. Tokens can be stolen: a holder that can't use its token (no line of sight
for 1 s) loses it to a waiting member, so nobody idles in view.

A situation ends when its subject is resolved or it has no members; its members' alerts then cool down
by §6.4.

---

## 11. Spatial reasoning

### 11.1 NavGrid (engine, new)

A sparse set of cells (1.25 m) sampled from the navmesh at level load, with neighbour links (including
up / down across stairs and drops), cell lookup by position, and a cell's floor point. It is the one
canvas for posts, chase floods and search maps (Dishonored 2's DSM idea). Built in the background at
load; cached next to the `.bsp.nav` file.

### 11.2 Posts and post selectors

A post query takes NavGrid cells around an NPC (≤ 20 candidates) and scores them with a **selector**:
a list of criteria, each 0..1, multiplied (The Last of Us). Criteria (data-composed):

`DistanceBand(min, max)`, `CoverFrom(threat)` (2 rays from the threat's eye to crouch and stand
heights), `SightTo(threat)`, `PathNotTowardThreat`, `NotBehindThreat`, `NotClaimed`,
`NotInAllyLineOfFire`, `Exposed` (DOOM "exposed cover" for aggressive factions), `NearChoke`,
`OnScreenFor(player)`, `Reachable`.

Selectors used: Cover, Shoot, Flank, Watch, Circle (melee ring), Retreat, Safety (civilians), Throw,
OpenPost (where to stand to investigate a point), Surround.

Rays are time-sliced (≤ 160 a frame across all queries); results are kept 0.5 s.

### 11.3 Chase (Dishonored 2 flood)

When a fighting NPC loses sight of its target and has no fresher record, the Combat situation runs a
chase flood: seed heat (H = 20) at the last known cell, a barrier at the neighbours behind the last
known velocity, then propagate heat and barrier step by step; stop when no cell heats, more than
MH = 25 cells heat in one step, or MS steps are reached (difficulty: 20 / 30 / 40). The chase
destination is the warm cells' weighted centroid (snapped to a warm cell). Runs over 3 frames.

Chasers run there. A new high-priority stimulus re-runs it (at most 3 times per chase). On arrival
without contact: a disappointment bark and a look-around (2 s), then Search.

Corridors make long chases; open rooms stop early and go to search. Players learn to lure chasers
past traps, which is intended.

### 11.4 Search map (The Last of Us)

The Search situation keeps a probability per NavGrid cell, seeded at the last known position
(certainty-weighted), spreading to neighbours at the player's walk speed. Every frame, cells inside
members' vision zones with a clear ray are cleared (≤ 64 rays a frame per situation, round-robin).
Searchers claim the reachable cell with the best probability / path-length ratio that nobody else
claimed; watchers hold cells next to chokes with high outflow; the checker walks the original spot.

Search gaze: the frontier cell (visible boundary next to unvisited probability) with the most mass,
which is where a hidden player most likely is. This replaces "look at the bend of the path".

The search ends when total probability falls below 5% of the start, or after the difficulty's search
time (60 / 90 / 120 s). Its members then cool down; area floors stay.

### 11.5 Watching without line of sight

A Holder or Watcher that can't see the target faces the frontier cell or the choke (door, window) the
target must pass to reach it. With several NPCs, chokes are shared out (one watcher per choke).

---

## 12. Investigation and search: moment to moment

- **A noise, calm guard.** Glance at once (gaze, 0.3 s reaction). If the meter crosses Suspect or
  the noise was a "curious" type: one Investigator is granted; the others stay on routine and glance.
  The investigator barks ("What was that?"), walks (weapon held, `caution_idle` pauses) to an open
  post 2–4 m from the point with a view of it, looks around 3 s (head sweep), barks ("Nothing.") and
  resumes routine. Wary: two go, one covers.
- **Spotted at the edge, then gone.** Meter at Suspect, then decays. The NPC walks to where it saw
  something (Investigate), gun ready, asking "Who's there?". If it identifies you on the way → Combat.
- **Lost in a fight (the save case).** The last LOS record is the target's position at the moment it
  was lost (no tracking after that). Shooters suppress near that spot 2–4 s (near misses), Holders
  watch the frontier and chokes, the chase flood runs; if the player moved more than 5 m since that
  record according to *evidence* (a sound, a sighting), the Checker goes at once. Otherwise the
  Checker goes after 6–10 s. Then chase, then search. Nothing reads the player's real position.
- **The player vanished (teleport or mist) mid-fight.** A "vanish" event: barks ("Where'd he go?!"),
  pairs turn back to back (§17.6), the search map is seeded with every cell within the player's
  teleport range that the squad can't see; gaze sweeps those cells.
- **Giving up.** End-of-search bark ("He's gone."), the searchers walk back to routine, the area keeps
  its floor (Wary if it was a fight).

---

## 13. Bodies and evidence

### 13.1 Bodies

Bodies are `ObservationTarget`s with a body record: state (dead / unconscious / drained), cause
(blade / gun / explosion / fire / drained / unknown), found-by. A found body does not trigger again.

Guard finds a body: Identify (suspiciousness 1) → shout "Body here!" (message) → TendBody: walk to it,
`crouch_idle` 2 s, read the cause → Search situation around the body (radius grows with how fresh the
kill is) and the area floor becomes Wary. A drained body for a lore-aware or rumoured faction:
VampireSign, request hunters.

Unconscious body: the guard wakes it (`crouch_idle` 3 s, the victim plays `standUp`). The woken NPC is
Alarmed, joins the search, and keeps what it saw before it went down (the apparent identity of its
attacker, if it saw one).

Moved and hidden bodies: a body only triggers when seen. Hidden spots (`HidingSpot` containers, dark
corners by light level) hide it. Seeing someone carry a body is a crime (Murder-level concern).

### 13.2 Civilians and bodies

Civilian sees a dead body: scream (message, concern BodyFound) → flee 10 m from it → Report to the
nearest guard it knows of. Unconscious body: no scream, Report. Drained body: as dead, +Unnatural fear.

### 13.3 Missing allies and changed objects

- Disappearing NPC (Blacklist): if NPC A had NPC B in earshot for 10 s and then hears and sees nothing
  of B for 5 s while staying in the area → Investigate B's last position ("Mike? You there?").
  Cooldown 60 s.
- A partner missing at a chat task point → the waiting partner investigates.
- Changed objects: a door or light whose state differs from what an NPC last saw on its routine path
  creates a Curious event with a 30 s lifetime; the first NPC to see it claims it.

---

## 14. Civilians, VIPs and bodyguards

### 14.1 Fixed fear rules

| Trigger | Fear |
|---|---|
| A stranger with a weapon drawn in view | +0.4 once, +0.1/s |
| A gun aimed at this civilian (player aim cone, < 10 m) | Scared at once: hands up, backs off |
| A gunshot within 25 m | +0.6 |
| Violence or a power seen | +1.0 (Panic) |
| A body seen | +0.8, and BodyFound |

Reactions: **Uneasy** (glance, step away, mutter), **Scared** (back away facing the threat, hands up if
aimed at), **Panic** (one scream, a 15 m shout; flee along the best safety post or exit flow). **Cower**
instead of fleeing if the threat is within 4 m and no flee path leads away from it; stays until the
threat is 8 m away for 5 s. Same inputs always give the same reaction.

### 14.2 Witnesses

A civilian who identified a crime, a culprit or a body becomes a **witness** (a HUD icon shows it).
After reaching safety (or at once if a guard is closer than the threat), it runs to the nearest guard
it knows of (seen in the last 2 min, or a guard post), reports (2 s), and the guard gets its records:
the place, the culprit's apparent identity, what happened. Silencing the witness on the way means
nothing is reported. Calm again (fear < 0.2 for 30 s): back to routine, avoiding the area.

Staff (servants, barmen) also Confront trespassers in staff areas mildly ("You can't be back here") and
report if ignored; guests only fear and report (Hitman).

### 14.3 VIP and bodyguards

Escort situation: bodyguards keep formation slots (1.5–3 m), each with an outward gaze sector (they
cover each other's backs). Threat: Evacuate to a designer `SafeRoom` along a route that avoids the
threat; the VIP follows. VIP missing (no sight for 10 s): Escort-search: the nearest bodyguard checks
the last position, then they fan out (Hitman). VIP dead: alarm + search; VIP unconscious: evacuate the
body to the safe room if possible, else guard it.

---

## 15. Factions, reputation, zones, crimes

### 15.1 Faction table (`GameData/ai/factions.json`)

- Relations: friend / neutral / hostile, asymmetric, defaults plus per-NPC overrides in the map,
  changeable at runtime by script events (saved by `NpcDirector`).
- Combat traits per faction (§17.2), lore awareness (ignorant / knows), voice set.
- Only hostile factions fight each other. NPC fire and blades never damage non-hostile NPCs; shooters
  hold fire when a non-hostile is in the line.

### 15.2 Reputation (per player)

Standing per faction, −100..100: ≤ −50 hostile, ≥ 50 friendly, else the faction's default for
strangers. Witnessed crimes attributed to the player's *own* identity change it (assault −30, murder
−60, feeding −80, trespass −5; helping a faction's member +10, data). Crimes done in a likeness or a
possessed body are attributed to that identity instead (§16.5).

### 15.3 Zones (`AreaZone` brush entity)

| Type | Strangers | With access (faction, role, identity list) |
|---|---|---|
| Public | fine | fine |
| Restricted | Trespass: Confront | fine unless acting out of character |
| Hostile | Threat: Combat | fine unless acting out of character |

Confront flow: "Hey! You can't be here." (meter shown) → 5 s to leave, the guard follows at 3 m →
second warning at 2 s left → leaving resolves it, staying or coming back within 30 s escalates to
Combat. An escort walks toward the zone exit, expecting the player to follow. Zones also define the
wariness and alarm areas.

### 15.4 Crimes

Trespass, Brandish, Assault, Murder, Feeding, PowerUse, BodyCarry, Theft (later). Each maps to a
concern and a reputation change; witnesses carry them in their records.

---

## 16. The vampire: powers, cover and lore

### 16.1 Identity

Every person has a true identity (who controls it) and an apparent identity (who it looks like):
the player normally (true = apparent = self), in a likeness (apparent = the victim), possessing a
host (the host's body: true = the player, apparent = the host).

Per-NPC identity setup in the map: `role`, `access` (zones), `acquaintances` (groups who know this
NPC: squad, household, colleagues), `possessable`, `likenessAllowed`, `inCharacter` overrides (may
this NPC draw a weapon, run, enter a zone...).

### 16.2 What an observer judges

1. Is this apparent identity compromised in my memory? → hostile (s = 1).
2. Do I know them (acquaintance)? → they notice odd behaviour twice as fast and greet them.
3. Are they allowed here (zone access for that identity)?
4. Are they acting in character?

### 16.3 Out-of-character signals (suspiciousness while seen)

Running in a calm area 0.2; crouching 0.3; climbing or mantling 0.4; a weapon drawn without cause
0.5; somewhere their schedule doesn't put them (acquaintances only) 0.3; not answering a greeting
(acquaintances greet; walking past without stopping) 0.3; carrying a body: crime. Several add up
(capped at 0.8). At Identify an acquaintance asks first ("Bob? You alright?"); continuing → Compromised.

### 16.4 What blows a cover (for that observer, then for whoever it tells)

- Seeing the possession start (the host convulses, `stun` 0.5 s) or end (the player emerges).
- Seeing the likeness taken.
- Seeing the real person (awake or as a body) while knowing the copy is around (doppelgänger).
- A hunter's aura or a dog's smell within range.
- Seeing that identity commit a crime or use a power.
- Being told.

The cover then fails only for those who know. The HUD shows which NPCs see through you (§18.1).

### 16.5 Powers, one by one

| Power | Rule |
|---|---|
| Possession | Enter: a visible power use at the host. During: the host keeps its identity and access; others treat it normally unless it acts out of character. Its crimes are attributed to the host: witnesses treat that body as the intruder while possessed. Exit: the player emerges (visible); the host is Disoriented 4 s, then has "lost time": Wary, and reports it if left somewhere odd or hurt. Witnesses later *detain* the host (Confront → walked to an officer), they don't kill it |
| Likeness | Taken from an unconscious, unseen victim (Death of the Outsider). The victim's body is evidence (doppelgänger rule); moving it breaks the likeness (player-side rule). Ends when the player is identified as hostile |
| Feeding | From behind, unseen: quiet (4 m), full, leaves a drained body. In combat on a staggered enemy: fast, partial, loud (15 m), a power use for witnesses, +0.3 fear to allies who see it |
| Mesmerize | The target's Mind is off (stands, staring) up to 10 s or until hurt; afterwards it remembers nothing. Seeing a dazed person near the player: odd (0.4); lore-aware: power use |
| Mist form | Player visibility ×0.15, untouchable. The mist itself is a Curious stimulus for the ignorant; lore-aware and hunters identify it within 8 m; a flare cancels it |
| Teleport | Unseen in motion; the landing spot is perceived normally. Vanishing from an NPC's sight mid-fight is a "vanish" event (§12) |

### 16.6 Lore and rumours

| Level | Who | A witnessed power means |
|---|---|---|
| Ignorant | commoners, ordinary guards | Unnatural: fear, report; guards treat it as an assault and search |
| Rumoured | an area / faction after 2 unnatural reports reached a guard or officer | Vampire: alarm, request hunters; acquaintances greet-test each other (odd behaviour ×1.5) |
| Knows | hunters, church | Vampire: immediate Combat, counters used |

Rumours spread as told records (type Supernatural) and persist for the level.

Hunters: aura sense 6 m; flare (10 s of full light in 8 m: reveals mist, a teleport landing inside it is
seen at once); holy water (feeding or possessing them fails and staggers the player); crucifix ward (3 m:
no possession, feeding or mesmerize nearby). Dogs: smell (§5.3), barks are shouts to their handler, a
fast bite.

---

## 17. Combat

### 17.1 What makes loud play fun here

- Every archetype is strong against one style, weak against another, and has a visible tell.
- Enemies react to what the player does *now* (distance, weapon in hand, blocking, powers, hiding,
  standing on an unreachable spot), not to a build label.
- The player feels powerful: catching a gunman up close, staggering and feeding, breaking a squad's
  nerve. The fight stays threatening through numbers, roles and counters, not bullet sponges.
- Stealth tools work inside a fight (vanish, backstab, mist), and the squad has readable answers
  (back-covering, flares), so the mix is a skill, not an exploit.

### 17.2 Faction traits (Division 2)

| Trait | Low | High |
|---|---|---|
| Aggression | holds range, short bursts, relocates rarely | closes distance, circles, long bursts, rushes |
| Organisation | ignores allies, no callouts | ranks, callouts, radio, flank calls, back-covering pairs |
| Training | slow wind-ups, poor aim, no cover fire, no dodge | fast wind-ups, good aim, cover fire, flanks, dodges |
| Tech | blades, clubs, pistols | rifles, explosives, hunter gear |

Examples: Bandits (aggression high, organisation low, training low, tech low), City Watch (medium,
high, medium, medium), Hunters' Order (medium, high, high, high + holy gear), house guards (low, high,
medium, medium), ghouls (very high, none, none, none), rival vampires (high, medium, high, powers).

### 17.3 Archetypes

| Archetype | Wants | Strong against | Weak against | Tell |
|---|---|---|---|---|
| Watchman (sword + pistol, most common) | 4–10 m, swaps to sword under 4 m | mixed play | a focused build | weapon swap |
| Gunman (any firearm) | 8–20 m, posts by traits | blades at range | being caught (< 4 m: backs off, shoves, slow to re-aim), flanks, reloads | aim-up, reload bark |
| Swordsman / bruiser | 0–3 m | other blades (blocks, kicks) | guns, powers | guard stance, wind-ups |
| Thrower | 10–20 m, behind others | a player hiding or camping | rushes | lit fuse, bark, long wind-up |
| Hunter | 6–15 m | powers (aura, flare, holy water, ward) | blades and guns from ambush | flare, ward glow |
| Officer | behind the line | — (makes the squad better: radio, roles, morale) | being killed first | voice, gestures |
| Dog | 0–2 m | likeness, mist, darkness (smell) | guns, height | barking |
| Ghoul (supernatural) | 0 m, rushes | slow builds | blades, fire | screech |
| Rival vampire (supernatural) | any; teleports to flank posts, mist, lunge | everyone | hunters' gear works on them too | mist puff, sound |

Toughness (Cyberpunk-like): mooks die in 1–3 blade hits or 2–4 pistol shots; elites have armour plates
(shoot or break them off) and weak points tied to their gear (a thrower's satchel, a hunter's flare
pouch); heavies and supernatural enemies take real effort. Numbers are data per archetype and tier.

### 17.4 Roles and tokens (per player target)

MeleeAttack 1 (2 on hard), AccurateShot 2, Throw 1 per 12 s, Flank 1, Approach 1, Counter 1 (hunter),
AlarmRun 1. Every gunman that sees its target shoots; only AccurateShot holders aim to hit, the rest
fire near misses (they pressure, they suppress, they make noise).

### 17.5 Fairness

- Aim tell: the first shot after acquiring waits for the aim-up (`pistol_aim` held 0.5–0.9 s by
  training).
- Hit chance for token holders: base (training) × the player's movement (standing 1.0, walking 0.8,
  sprinting or sliding 0.45, mid-teleport 0) × visibility.
- Never accurate from off-screen: a shooter outside the player's view (±60°) misses on purpose; a melee
  attack never *starts* outside the player's view (±70°), the attacker steps into view first, or barks
  audibly.
- A damage-per-second cap per player (difficulty).
- No new attack starts against a player who is mid-finisher (combat feeding).

### 17.6 Back-covering and adapting (the answer to "instant from behind")

An instant takedown works on any NPC that does not see the player (meter below Notice and the player
behind it). In Alarmed and Combat:

- NPCs within 6 m pair up; each pair member glances behind its partner's back every 2–4 s (head and
  spine turn, readable: the player times the moment).
- An NPC that sees its partner taken down is instantly in Combat with the player (no takedown on it).
- Adapting within one fight: the Combat situation counts the player's tricks (vanish + backstab,
  shooting from an unreachable spot, camping a choke, kiting with a blade). After the second use of
  the same trick: vanish + backstab → pairs tighten into back-to-back groups, hunters throw flares;
  unreachable → throwers flush, others fall back to cover the exits; camping → a flank plus a throw;
  kiting → gunmen stop chasing and hold posts. Each response is barked ("Back to back!", "He's on the
  roof again!") so the player learns why.

### 17.7 Ranged combat, moment to moment

- Acquire: aim-up tell, then bursts (length by traits), cooldowns between bursts.
- Posts: the Shoot selector (Cover for defensive factions, Exposed for aggressive ones). Reposition
  when exposed too long, flanked (the player's angle to the cover's normal > 70°), hit, or token lost;
  sprint between posts (weapon at the chest).
- Reload: weapon lowered, bark ("Reloading!"), a real window.
- Player closes in (< 4 m): gunmen back off (walk_b) and shove under 1.5 m (sword_kick); watchmen draw
  swords; organised squads call a swordsman ("He's on me!"). Catching a gunman pays off.
- Player hides in cover: holders watch, near-miss suppression 2–4 s, a flanker takes a route outside
  the combat vector (The Last of Us cost shape on the NavGrid), a thrower flushes after 4 s of camping.
- Player on an unreachable spot (no navmesh path): return fire from cover, cover the exits, flush with
  explosives, fall back if losses mount (Blacklist).
- Explosives: 1 s wind-up with a lit fuse and a bark, aimed at the player's cover or spot, never at
  point blank or near allies; molotov fire patches become hazards everyone avoids (path cost and
  AvoidHazard).

### 17.8 Melee, moment to moment (fixes §2.2)

Attack data (per attack, per archetype):

| Field | Meaning |
|---|---|
| clip | sword_attack_main (slash), sword_attack_break (heavy, breaks blocks), sword_kick (guard break, push) |
| windupScale | the clip plays at this speed (0.4–0.6) up to the commit marker: a slow, readable wind-up with the spine pulled back |
| commit | a `.skmm` event marker, ~0.18 s before the hit event: from here the NPC no longer turns (`lockRotation`) |
| active | hit event ± 0.08 s: capsule sweeps between the blade's previous and current positions each frame (weapon bone), not one ray |
| damage, poise damage | data |
| recovery | 0.6–0.9 s after the swing, open to punishment |
| parryable | yes for slash and heavy; the kick can't be parried, only dodged |

Rules:

- Turn taking: MeleeAttack tokens; at least 1.2 s between attack starts across the group; never two
  wind-ups in their commit phase at once.
- Waiting attackers circle on a ring at 2.5–3.5 m (walk_l/r, sword_walk), one slot per NPC by angle.
  Re-slot only when the slot is blocked or the player moved more than 1.5 m (no jitter). Trained
  NPCs feint (step in, step back).
- Facing: during the wind-up the NPC turns toward the player at up to 180°/s; after commit it is
  locked; root motion carries the swing.
- Blocking is a visible stance (sword_block), not a dice roll: a swordsman blocks frontal attacks
  (±60°) while not winding up or recovering; training sets how often it chooses to stand guarded. A
  block breaks after 3 blocked hits or one heavy hit → falter.
- Parried → stagger (feedable). A turtling player gets kicked (after 1.5 s of blocking).
- Pain ladder (poise per archetype, regenerates after 2 s): flinch (short `stun`, never cancels a
  committed swing), falter (cancels the current action, 0.6 s), stagger (1.5–2 s, feedable),
  knockdown (ragdoll stun).

### 17.9 Dodging (trained factions)

When the player aims at one of them for 0.4 s within 15 m, or a lunge or power wind-up is visible, a
trained NPC may sidestep or slide (cooldown 4 s, never during its own commit). The dodge has its own
small tell (a crouch dip), so the player can bait it.

### 17.10 Morale: break, flee, surrender

At the break threshold (fear 0.85, §6.5): if a flee path exists, run (often to an `AlarmPoint` or the
reserves: the runner is a target worth catching); if cornered or the player is within 4 m aiming at
it, surrender (hands up, `scared`, weapon dropped). A surrendered NPC stays out of the fight; the player
can spare (walk away: it flees and reports later), knock out or feed. Hunters, fanatics and
supernatural enemies never break. Killing the officer: −0.25 to its squad's fear, and the situation
stops granting exclusive roles for 10 s (the squad fights as individuals).

### 17.11 Escalation

An alarm (bell `AlarmPoint`, an officer's radio, a runner reaching reserves) dispatches the area's
designer-placed `ReserveSquad`s and, for VampireSign concerns, hunters. Reserves are real, placed
NPCs: finite, and they arrive by walking (or running) from where they are. The alarm area becomes
Alarmed with a rough position (the alarm's report, ±10 m), not an exact one.

### 17.12 Supernatural enemies

They use the same senses, mind and skills, plus an ability set (data): teleport to a Flank post (mist
puff and sound as a tell), mist (untouchable 2 s), lunge (long-reach melee with a long wind-up).
Hunter gear works on them too.

---

## 18. Feedback

### 18.1 HUD (per local player)

- Over visible NPCs whose meter for this player is above 0: "?" fills white → yellow (Notice →
  Suspect), "!" red at Identify. Extra icons: searching for you, witness running to report, runner to
  an alarm, sees through your cover.
- Screen edge: arrows for off-screen NPCs within 30 m with a meter for you, the same colours; a red
  flash for an incoming melee attack or a thrown explosive from off-screen.

### 18.2 Barks

Three tiers (Blacklist): tier 1 specific (the first two uses per level), tier 2 generic, tier 3 very
generic. Arbitration: one bark per situation per 2 s, none overlapping within 10 m, the most important
wins. FMOD event names: `event:/Character/<Voice>/<bark>_t1|_t2|_t3`. Voices per profile (Guard1..n,
Officer, Hunter, Civilian male / female, Bandit).

| Bark | When | Tier 1 example |
|---|---|---|
| notice | meter crosses Notice | "Hm? Something moved." |
| suspect | crosses Suspect | "Who's there? Show yourself." |
| identify_intruder | Identify, hostile | "Intruder!" |
| identify_vampire | lore-aware, power seen | "Bloodsucker! Here!" |
| unnatural | ignorant, power seen | "God's teeth, what was that?!" |
| investigate_start / _clear | Investigate | "I'll check it." / "Must've been rats." |
| heard_noise / heard_shot | sound concern | "Footsteps?" / "Shots! Over there!" |
| target_lost / vanished | lost LOS / vanish event | "Lost him!" / "He just— vanished!" |
| chase / give_up | chase, end of chase | "After him!" / "He's gone. Spread out." |
| search_start / search_end | Search | "Check every corner." / "Nothing. Stay sharp." |
| trespass_warn1 / warn2 / escort | Confront | "You can't be here." / "Last warning." / "Out. Now." |
| brandish | Brandish | "Put that away!" |
| body_dead / body_drained / body_unconscious | TendBody | "Man down!" / "He's... drained." / "Wake up, you oaf." |
| wake_up | woken NPC | "Someone hit me—" |
| missing_ally | §13.3 | "Mike? You there?" |
| flank / cover_me / reloading / grenade | combat roles | "Going round!" / "Cover me!" / "Reloading!" / "Fire in the hole!" |
| back_to_back / watch_backs / flare | §17.6 | "Back to back!" / "Watch your backs!" / "Light it up!" |
| roof_again / fall_back | adapting | "He's on the roof again!" / "Fall back!" |
| man_down / morale_break / surrender | morale | "We lost Tom!" / "I'm not dying here!" / "Mercy! Mercy!" |
| alarm_raise / reinforce | escalation | "Ring the bell!" / "We're here, where is he?" |
| greet / odd_ally / doppelganger / possession_seen | cover | "Evening, Bob." / "Bob? You alright?" / "Wait— Bob's over there!" / "His eyes— that's not Bob!" |
| civilian_scream / report / plead / cower | civilians | (scream) / "Guard! Someone's been killed!" / "Please, don't!" / (whimper) |

Today's 8 events map in: target_found → identify_intruder, arrest → trespass_warn1, arrest_final →
trespass_warn2, shots_fired → heard_shot, weapon_surrender → brandish, dead_body → body_dead,
heard_sound → heard_noise, target_lost → target_lost.

---

## 19. Data files

| File | Holds |
|---|---|
| `GameData/ai/perception.json` | vision zones, rates, thresholds, decay, motion and light factors, sound radii |
| `GameData/ai/situations.json` | message radii and delays (shout, report, radio), investigate roles, chase flood and search map tuning |
| `GameData/ai/factions.json` | factions, relations, traits, lore, voices, reputation rules |
| `GameData/ai/roles/<role>.json` | archetype profile: skills, tuning, weapons, poise, toughness, melee attack table, abilities |
| `GameData/ai/difficulty.json` | multipliers: perception, aggression (tokens, cadence, flank and throw frequency), lethality, chase length and search time |
| `GameData/ai/barks.json` | bark ids → tiers → FMOD events |

Per-NPC map properties: `profile`, `faction`, `squad`, `acquaintances`, `possessable`,
`likenessAllowed`, `voice`, `reserve` (belongs to a reserve squad), `target` (default task, as now),
`script` (behaviour tree asset for the Scripted skill). All JSON uses NLOHMANN structs, loaded in
`LoadAssets`.

---

## 20. Save and load

- `NpcBase`: profile id, health, poise, fear, alert, memory records (times in game time), meters,
  current skill id + its state, body state (ragdoll / unconscious / stand-up), routine task state.
- `NpcDirector`: situations (members, roles, the search map's cells above 1%; chase floods are re-run),
  area floors, rumours, alarms, reputation per player, faction runtime overrides.
- Old saves don't load NPC state (decided).

---

## 21. Performance (60 NPCs) and co-op

| Part | Budget |
|---|---|
| Perception | each observer every 4th frame; range + cluster filter first; ≤ 2 rays per candidate; ~200 rays a frame |
| Mind + skills | full rate within 40 m of a player or when Alarmed / Combat; 5 Hz when Calm and far |
| Body | animation and hitboxes gated by visibility (as now) |
| Posts | ≤ 20 candidates, ≤ 160 rays a frame in total |
| Search map | ≤ 3000 cells per situation, ≤ 64 clearing rays a frame per situation |
| Chase flood | ≤ MS steps over 3 frames |

Tracy zones per layer. Co-op: subjects are entity ids; meters, tokens, roles, HUD, identity and
reputation are per player; nothing special-cases "the" player except the fairness rules, which apply
to every player.

---

## 22. Situations walkthrough (expected behaviour)

1. **Crouch-walking behind a patrolling guard in a dim corridor.** Near zone ignores a quiet target;
   focus is ahead. If the guard glances back (Wary look-around), the meter fills slowly (0.6 × 0.6 ×
   light 0.6): the "?" appears, the player has seconds to step out of view.
2. **Spotted at 20 m in the open.** "?" fills over ~3 s; the guard turns its head first (readable),
   then walks toward the spot, "Who's there?". Break line of sight before "!" and it investigates.
3. **Fight, the player ducks round a corner (the save case).** Suppression at the last LOS spot,
   watchers face the corner's frontier, a checker comes after 6–10 s, then chase and search. Gaze always
   points at the frontier, never at a stale squad position.
4. **Silent kill next to two chatting guards.** The chat task point is a group: the partner hears the
   body fall (6 m) → "Hey, did you hear that?"; killed out of earshot → the disappearing-NPC rule
   after 5 s.
5. **A civilian sees a body.** Scream (others within 15 m: Uneasy / Panic by distance), flee, run to the
   guard at the gate (witness icon). Kill or mesmerize it on the way → nothing reported.
6. **Likeness of a servant in the kitchen.** The cook (acquaintance) greets; walk past without stopping
   → "Bob? You alright?"; sprinting there → "Bob" is compromised for the cook, who shouts.
7. **A guard sees the player possess his partner.** "His eyes— that's not Bob!": the host is hostile for
   that guard and everyone he shouts to; others still accept "Bob".
8. **A blade build charges three gunmen.** Aim tells; while sprinting their hit chance is 0.45; the
   nearest backs off and shoves, a watchman draws its sword ("He's on me!"); a staggered gunman is fed
   upon, the others' fear jumps; one breaks and runs for the bell: catch him or face reserves.
9. **Sniping from an unreachable roof.** Return fire from cover, others cover the stairs, a thrower lands
   dynamite on the roof (lit fuse, bark); second time: "He's on the roof again!" and they fall back.
10. **Vanish and backstab twice.** First time works. Second time pairs turn back to back, a hunter fires a
    flare; the third attempt needs better timing (glance rhythm) or another tool.
11. **Co-op.** One player is fought in the courtyard, the other sneaks in the cellar: the cellar guard
    hears shots (Alarmed, searches toward the sound) but has no record of the second player, whose meter
    starts from 0.

---

## 23. Debugging and tests

- Debug panel (look at an NPC, as now): alert, fear, meters per subject, memory records (age, source,
  certainty), concerns, skill wants, situation and role, BodyIntent, gaze, last messages in / out.
- World overlay: vision zones from the gaze, the gaze ray, last-known-position ghosts (developer view),
  search map heat, chase flood, post scores.
- Headless scenario tests `-npc_selftest` on `aitest` (run when asked; the user checks feel in game):
  detection times at set distances (±10%); a dead NPC never sends a message or holds a role; lost
  target → chase → search → give up, gaze on the frontier; body found → shout → search; witness reaches
  a guard → report, witness killed → no report; trespass warn → escort → combat; melee: only token
  holders attack, no attack starts off-screen, no turning after commit; save / load mid-search;
  possession seen → host compromised for the witness and those told only; two players keep separate
  meters.

### 23.1 As built: `-ai_test` (2026-11-23, compiled and run)

The harness lives in `Debug/AiTests/` (`AiTest.h` is the overview). It replaces the flag name `-npc_selftest` above.

- **Run:** `Debug/AiTests/run_ai_tests.ps1 [-Filter sense,chase,squad] [-Runs N]`, or by hand
  `StealthAction.exe -headless -no_map -ai_test <filter> -ai_runs N` with the working directory `source/Games/StealthAction`.
  Results: `SaveData/ai_tests/summary.log`, `result.json`, `trace_<scenario>.csv` (a row per NPC, four a second).
- **How it works:** the engine got one hook (`EngineMain::FrameHook`, §24): the runner loads each scenario's map through the
  normal loader, then runs the scenario inline, stepping `GameUpdate` + `FinishFrame` with a fixed 1/30 s step (no
  rendering, so far faster than real time). The puppet is the real `Player`: it walks, runs and crouches along a
  navigation path (`NavigationSystem::FindSimplePath`) through the real character controller (the movement actions
  and the crouch the keys toggle), carries a real weapon (`Loadout`: `weapon_mpsd` is an illegal firearm, so
  `CombatTargetView::hasGun` and the `illegal_weapon` tag are real; `weapon_sword`; none), so the observation target,
  the footsteps and the combat AI's view of the target are the real ones. Only `TeleportTo`, `Park` and the gun's noise
  (`Shoot`: a gunshot sound and the violent-crime delay) are scripted. A move with no path, or with no progress for
  2 s, fails the scenario with a clear line. Scenarios read NPC state through `Probe` snapshots and fail through
  `Ctx::Check`.
- **Invariant, every step of every chase and squad scenario: no knowledge without perception.** While no NPC has seen
  the puppet or heard a sound it made for 1 s, the point where the squad believes the puppet is (last sighting while
  visible, the estimate while concealed or lost) must not move toward the puppet's true position by more than 0.3 m a
  step. The chase scenarios do not expect the fight to end at a fixed time: it has to end within the designed window
  after the last evidence of the puppet (`concealedMax` + `lostMaxTime` + search `timeout` + 5 s = 36 s); a puppet an
  NPC finds again says so in the log and the fight rightly goes on.
- **Built scenarios:** `sense_*` (on the new `npc`: the sight zones and their strength, sight by distance, posture and
  motion, and the whole path to identification, each held against `perception.json` and the Mind's formula within
  8 to 20%; footsteps and gunshots; the HUD value for both classes), `chase_*` (hide, leave the map and come back,
  fly straight up, run away, noise from far away), `squad_*` (flankers must not stall and must breach). Planned ones
  are in §25.
- **Baseline before the Phase A fixes (old harness, three runs):** `chase_escape_to_hidden_spot` and
  `chase_fly_straight_up` failed every run and `chase_run_away` one run in three. The causes, as verified: (1) the old
  guards saw with one 150 m cone (`observer->maxDistance`), so a player hovering 100 m up was fought for good; (2)
  `CombatCoordinator::TrackByEar` gave the squad the target's true position (+-1.5 m) whenever a member was within 8 m,
  or within 25 m while it moved faster than 3 m/s, with no sound and no wall in the way: a psychic read; (3) the guards
  followed the real position for 1.0 s after losing sight instead of the designed 0.4 s while engaged; (4) the old
  puppet moved by a teleport a frame along a straight line, so it stuck on walls in view of the guards, which made
  part of the escape failures test artifacts. Phase A fixed (1) by giving the old guards the vision profile of the
  new `npc` (`NpcVisionTuning::ApplyTo`), (2) by deleting `TrackByEar` (real footsteps, landings and shots already
  reach the squad as sounds), (3) with `belief.lostSightTrackTime`, and (4) with the navigation puppet. Result with
  the new harness, three runs of sense, chase and squad: all 45 pass.
- **What the chase results say:** a puppet that goes to ground is often found again: the squad's search walks into
  a line of sight on it from 20 to 30 m (the combat sight range is 40 m), and the fight then goes on for as long as it
  is seen. In the three runs the hidden-spot scenario ended its fight 30 s after the last sighting twice and went on
  once, and run-away went on all three times. That is reported as "found", not failed; whether the squad should find
  a crouched player at that range is a question of feel for the user.
- **Stealth and social tiers (M1 sign-off, M2 core):** `stealth_*` (routine post and patrol laps, the noise investigation
  against the profile timings, the alert cycle, save and load mid-investigation, W1, W2) and `social_*` (shout range and
  walls, dead and destroyed shouters, report and radio, investigate and search roles, lost target to chase to search to
  give up, the search gaze, the search leak invariant). Social scenarios run the runner's invariant on the `Npc` minds
  too: no Person record about the puppet may be newer than the last perception plus 0.3 s. Messages are queued through
  `TestMind().outbox` the way a mind queues them. A scenario that stops before its first step leaves the previous
  frame's render list pointing at removed NPCs and crashes the exe: the runner now writes `last_scenario.log` at once.
- **Writing scenarios:** do not teleport the puppet while NPCs still see it (one stale frame gives the squad the true
  position: not real play); walk it with `ctx.Travel(...)`. The guards only react to an armed, visible player (crime
  tags): the puppet carries the illegal gun by default and `Shoot()` makes it a violent criminal for 6 s. The player can
  run (hold dash), which makes footstep sounds. The harness must advance
  `EngineMain::frame` (observers update one in four frames) and must not set `SimulatingGameTicks` (`Player::Update`
  returns early).
- **Limits:** the engine's random generators are not seedable, so runs are not bit-identical: checks use windows and
  `-ai_runs` repeats a scenario. Scenarios find their places on the navigation grid (`World::FindClearSpot`,
  `FindHiddenSpot`) instead of hard-coded coordinates, and say so in the log when the map has no such place.

---

## 24. Engine changes

1. **AiPerception** (`source/Engine/AiPerception`): vision profiles and gaze input, target sample
   points and visibility scale, strength results, light level from BSP light volumes, special senses
   (smell, aura, touch) hooks, `EmitSound` with loudness, type and wall-muffled propagation, debug draw.
   `visibleTargets` and the hearing API stay compatible for SlasherGame.
2. **NavGrid** (`source/Engine/Navigation`): sparse navmesh cell grid, neighbours, lookup, cache file.
3. **EngineMain::FrameHook** (`source/Engine/EngineMain.h`): a game can take over a frame's game update
   (`std::function<bool()>`, called once per main-loop frame before `GameUpdate`; returning true skips the engine's
   own update and finish for that frame, rendering still runs), with `RunGameStep()` and `RunFinishFrame()` to step
   the simulation. Only the automated tests (`-ai_test`, §23.1) use it; nothing runs without it.

### 24.1 M0 as built (2026-11-23, not yet compiled or run: the user builds)

- `Observer`: `useVisionProfile` + `VisionProfile` (zones, hysteresis, light), `extraSenses` (smell, aura:
  radius, optional wall block), `traceHead`, `ownTarget`; results in `sightResults` (`SightResult`: sense,
  zone, strength, distance, point), `touches` (from `AiPerceptionSystem::Touch`). The gaze is `forward`, the
  eye is `position`. With the flag off the old cone runs unchanged (SlasherGame, old NPCs).
- `ObservationTarget`: `chestPoint` / `headPoint`, `visibility`, `moving`, `sampleLight` -> `lightLevel`
  (sampled at 4 Hz on the main thread from the BSP light volumes).
- `AiPerceptionSystem::EmitSound(SoundEvent)` (walls muffle, `soundSettings` is the data, `isOnPlayerScreen`
  is the game's hook for the fairness rule), `DrawDebug()`; `HeardSound` gained `perceivedPosition`,
  `loudnessAtListener`, `apparentId`.
- `NavGrid` (`Navigation/NavGrid.h`): `Build()` after the navmesh (called from `Level.cpp`), cache
  `<map>.navgrid`, `FindCell`, `CellsInRadius`, `DrawDebug()`. Toggles: ImGui "navigation" window ->
  "draw nav grid", "draw perception" (StealthAction; the toggles also exist in ShooterGame's engine-side overlay).
- Not done on purpose: closed doors do not muffle sound (doors are `MainBody`, not world); dynamic
  navmesh obstacles are not in the grid; cell links across drops (one-way) wait for M2's needs.

4. **AiPerception, M2 core:** `AiPerceptionSystem::MuffledDistance(from, to)` (distance plus the wall penalty; `EmitSound`
   uses it, and the director uses it for shouts) and a public `Observer::ZoneAt(point)` (the vision zone a point is in,
   from the same `ClassifyZone` helper `UpdateVisibilityProfiled` uses; the search map clears cells with it).

Animation playback speed per action clip already exists (`Animation::Update(timeScale)`); commit
markers are `.skmm` event data.

---

## 25. Rewrite plan

The new brain is built next to the old one under a new class (`Npc`) so the game stays playable; the
map class names (`npc_guard`, `npc_civilian`, ...) switch to the new class at M3. Each milestone is
complete for its scope (no placeholders). Nothing is copied to ShooterGame.

| M | Scope | Done when |
|---|---|---|
| **M0** engine | AiPerception upgrade, NavGrid, debug draw | SlasherGame builds and behaves as before; overlay shows zones and NavGrid on `aitest` |
| **M1** core | `Npc` with profile data, Mind (records, meters, alert, fear, concerns), Body (intent, gaze, locomotion, doors), skill framework, Routine (task points), Investigate, HUD meter + edge arrows, save/load, debug panel | A calm guard on `aitest` glances, investigates and returns with the §6.2 timings; the HUD matches its meter; save/load mid-investigation |
| **M2** social | Messages, `NpcDirector`, situations + roles, NavGrid posts, chase flood, search map, Search / Chase / TendBody skills, bodies (dead / unconscious / drained), missing ally, changed objects, area floors, alarms + `AlarmPoint`, barks (tiers, arbitration) | Body found → shout → search → give up; killing the shouter first keeps others calm; alarms reach only their area |
| **M3** combat + cutover | Combat situation, tokens, Shoot / Cover / Flank / Circle selectors, ranged and melee modules (§17.7–17.8), pain ladder, fairness rules, morale (flee, surrender), throwers, unreachable handling, back-covering and adapting, dodging, faction traits, difficulty; map classes switch to `Npc`; old NPC code leaves the build | The §22 combat cases (3, 8, 9, 10) play as written; no NPC faces a stale position; melee never homes after commit |
| **M4** civilians | Fear rules, Flee / Cower / Report / Surrender, witnesses, staff vs guests, VIP Escort / Evacuate / Escort-search, `SafeRoom` | §22 case 5; a VIP is evacuated and searched for |
| **M5** world | Faction table + reputation, `AreaZone` (confront, escort, hostile), crimes and attribution, hostile-faction infighting, reserves (`ReserveSquad`) | Trespass flow; bandits and watch fight each other; an alarm brings reserves |
| **M6** vampire | Identity and cover, acquaintances, out-of-character signals, compromise rules, possession / likeness / feeding / mesmerize / mist / vanish reactions, lore and rumours, hunters (aura, flare, holy water, ward), dogs (smell, scent trail), supernatural enemies | §22 cases 6, 7; rumours turn "unnatural" into "vampire" across an area |
| **M7** finish | Performance pass for 60 NPCs (Tracy), co-op checks, delete the old code | Frame budget held with 60 NPCs on a test map; the old NPC code is gone from StealthAction |

Player-side work the AI depends on (not in this plan): takedown / knockout, carrying bodies, feeding,
possession control of a host, likeness, mesmerize, mist form, aiming cone exposure for civilians. The AI
exposes the hooks (events and identity state) from M1 on.

### 25.3 M1 as built

Files: `Entities/Npc/Npc.h/.cpp` (`Npc : NpcBase`, map class `npc`, keys `profile` and `target`),
`Entities/Npc/Brain/` (`NpcData` tuning + profile loading, `NpcMemory`, `NpcMind`, `NpcBody`, `NpcSkill`
framework, `SkillRoutine`, `SkillInvestigate`, `NpcSounds`), `UI/Player/UiNpcAwareness` (HUD marker + edge
arrows, owned by `PlayerHud`). Data: `GameData/ai/perception.json` (vision, meter, alert, fear, `sounds`
table) and `GameData/ai/roles/<profile>.json` (`guard`, `civilian`). `entities.ent` has the `npc` point class.

How it runs: `NpcBase` gained virtual hooks (`UpdateBrain`, `ComputeFacingLock`, `UpdateWeaponPosture`,
`UpdateObserver`, `TryStartInvestigation`, `OnNearMiss`); old classes keep the legacy brain. `Npc` runs
Mind -> skill choice (Routine 10, Investigate 40 (Curious, only as investigator or backup) / 48 (Threat); an Alarm is the Search skills of M2, running skill +0.1
within its commit time) -> one `BodyIntent` -> `NpcBody`. Hits from other threads are queued under a mutex.
Routine drives task points through the existing `DoingTask` / `Task_TargetReached` path.

Player side: `Player::UpdateAiPresence` fills the observation target (sample points, light, motion) and
emits footstep (running only, every 1.8 m) and landing sounds. `WeaponFirearm::NotifyNpcs` now emits a
`Gunshot` sound (walls muffle it, also for old NPCs).

Save / load: key `npcBrain` (profile, mind with record ages, current skill id and its state); restore does
not call Start again.

Deferred on purpose: `caution_idle` clip (Wary and Investigate use `pistol_hold`), bodies and other NPCs as
perception subjects (M2, next phase), area floors (M2, next phase), Fight (M3: a seen hostile is closed on to
`holdDistance` and watched), factions and zones (M5), closed doors do not muffle sound.

Unverified in game (check first): head / spine gaze axes and signs (copied from the pistol aim code), HUD
screen y direction and `Camera::projection * Camera::view` order, arrow texture and "?" / "!" glyphs in the
HUD font, gunshot hearing of old NPCs behind walls.

### 25.4 M1 sign-off and M2 core as built

M1 signed off with the `stealth` tier. Fixes it needed: a heard sound of the curious kind now raises a Curious concern at
Notice (`meter.curiousSound`), before only Suspect did, so ordinary noises never started an investigation; and
`NpcBase::UpdateTask` no longer restores `DesiredTask` after a task point's update, which sent every patrol point
straight back to the first one (a patrol never advanced).

M2 core, files in `Entities/Npc/Director/` (`NpcDirector`, `NpcSituation`, `SituationSearch`), `Entities/Npc/Spatial/`
(`PostQuery`, `ChaseFlood`, `SearchMap`), `Brain/` (`NpcMessage`, `SkillChase`, `SkillSearch`), data
`GameData/ai/situations.json` and `roles/officer.json` (a guard with a radio, for the tests):
- **Director:** an Entity (`npc_director`, saved) made by the first NPC that starts. Its `LateUpdate` runs after all NPC
  updates on the main thread: collect outboxes (a dead or destroyed NPC sends nothing), deliver due messages (the sender
  must still act), step the situations, put `ended` notes in inboxes. NPC threads read situations lock free because
  nothing changes them during the async phase.
- **Messages:** shout (muffled distance <= 15 m, 0.4 s), report (straight <= 2.5 m, 2 s), radio (same faction within
  100 m, 1 s, profile with `radio: true` only). The receiver stores the record as Told with certainty x 0.8 and the
  original time; a "hostile" record raises an Alarm concern. A guard shouts when it crosses Identify (cooldown 6 s).
- **Situations** are identified by (kind, subject). Curious concerns map to Investigate (one investigator, one backup
  while somebody is Wary, the rest glance). Alarm concerns, and a Threat concern whose hostile is lost, map to Search:
  Contact (somebody sees it), Chase (flood from the last record, chasers run to the heat centroid), Search (checker at
  the original spot, searchers claim map cells at least 5 m apart, up to two watchers at chokes). The search map starts at
  the freshest record, spreads at walk speed and is cleared by what members' eyes reach; it ends under 5 % or at 90 s.
- **Roles live in `SituationMember.order`; a situation holds planning data only.** Where the subject is comes from the
  members' memories (`Freshest`). No situation, flood or map is ever fed the puppet's true position.
- **Not done yet (next phase):** bodies, alarms, barks, the radio and report senders (nothing queues them yet; only the
  tests do), Leader use, civilians flee or cower (M4: they lost the alarm walk because they have no search skill).

### 25.1 Deleted at the end (M3 build exclusion, M7 deletion)

`Entities/Npc/Combat/*` (coordinator, tactics, cover finder / map / planner, approach map, belief, fire
control), `Entities/Npc/Investigations/*`, `Ai/BehaviorTreeTasks/*` (NPC tasks), the NPC use of
`GameData/behaviourTrees/general.bt`, `NpcGuardMelee` / `NpcGuardHybrid` / `NpcGuardBase` /
`NpcCivilianBase` / `NpcBanditBase` classes (replaced by profiles), `Crime` / `TargetInfo` /
`CombatMoveOrder` types. `COMBAT_AI_DESIGN.md` is marked superseded.

### 25.2 Kept

`CharacterController` use, `NpcAnimatorBase` family (fed by BodyIntent), ragdoll / stand-up, gore,
weapon meshes, bullets, task points (`TaskPoint` and subclasses) and `NpcSimulationManager` schedules,
doors. `Entities/Enemy/*` (older enemy classes) is out of scope.

---

## 26. Open items and risks

- Two-handed guns on pistol clips look wrong until new clips exist (accepted).
- Dogs need a body (model and clips); the existing `npc_dog` (TestNpc) assets are the starting point.
- NavGrid build time on large maps: measure on `l1_town`; bake if needed.
- Walls muffle sound by a flat penalty (no portal routing): a loud sound next door is heard, a faint one is not; tune the penalty. Closed doors are not counted as walls yet.
- Tuning is the bulk of the work after M3: the debug panel and overlays exist from M1 for that reason.

---

## Sources

- Martin Walsh, "Modeling Perception and Awareness in Tom Clancy's Splinter Cell Blacklist", Game AI Pro 2 ch. 28 — http://www.gameaipro.com/
- Travis McIntosh, "Human Enemy AI in The Last of Us", Game AI Pro 2 ch. 34 — http://www.gameaipro.com/
- Laurent Couvidou, "Flooding the Influence Map for Chase in Dishonored 2", Game AI Pro Online 2021 — https://www.gameaipro.com/GameAIProOnlineEdition2021/GameAIProOnlineEdition2021_Chapter06_Flooding_the_Influence_Map_for_Chase_in_Dishonored_2.pdf
- Sadoulet & Couvidou, "Taking Back What's Ours: The AI of Dishonored 2", GDC 2017 — https://www.gdcvault.com/play/1024660/Taking-Back-What-s-Ours
- Tommy Thompson, "The AI of Hitman (2016)" — https://www.gamedeveloper.com/design/the-ai-of-hitman-2016-
- Tommy Thompson, "Endure and Survive: The AI of The Last of Us" — https://gamedeveloper.com/design/endure-and-survive-the-ai-of-the-last-of-us
- Tommy Thompson, "Designing the Enemy AI of The Division 2" — https://gamedeveloper.com/design/designing-the-enemy-ai-of-the-division-2
- Tommy Thompson, "Cyber Demons: The AI of DOOM (2016)" — https://www.gamedeveloper.com/design/cyber-demons-the-ai-of-doom-2016-
- Tommy Thompson, "Bringing Balance to Stealth AI in Splinter Cell: Blacklist" — https://gamedeveloper.com/design/bringing-balance-to-stealth-ai-in-splinter-cell-blacklist
- "How Naughty Dog defined melee attacks and behaviors in The Last of Us Part II" — https://www.gamedeveloper.com/design/how-naughty-dog-defined-melee-attacks-and-behaviors-in-i-the-last-of-us-part-ii-i-
- Bart Vossen, "Enemy design and enemy AI for melee combat systems" — https://gamedeveloper.com/design/enemy-design-and-enemy-ai-for-melee-combat-systems
- The Dark Mod wiki: AI stats, Visual scan, AI Relations, Sound Propagation — https://wiki.thedarkmod.com/index.php/AI_stats
- Unreal Engine, AI Perception — https://dev.epicgames.com/documentation/en-us/unreal-engine/ai-perception-in-unreal-engine
- Emerald AI wiki, Faction Relations and Coward AI — https://black-horizon-studios.gitbook.io/emerald-ai-wiki/emerald-components-required/detection-component/faction-relations
- Fab "Stealth AI Kit" — https://www.fab.com/listings/2a2b4da2-8bb7-4b6d-bc90-0866539d33aa
- StraySpark, stealth detection gauges and attack tokens in UE5 — https://www.strayspark.studio/blog/stealth-detection-gauge-ue5-perception-alertness-memory
- Dishonored: Death of the Outsider, Semblance — https://www.pcgamesn.com/dishonored-2/dishonored-death-of-the-outsider-semblance
- Cyberpunk 2077 2.0 changes — https://www.pcgamer.com/cyberpunk-2077-2-0-patch-breakdown/
- Dishonored alertness states (2012 interview) — https://gameranx.com/updates/id/7591/article/dishonored-offers-intelligent-ai-no-new-game-plus/
