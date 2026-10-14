# SlasherGame — Combat Design

Third-person slasher-shooter on REngine 2. Melee is **timing based** (no directional inputs), movement is
**dash / double jump / short glide**, firearms (RMB) break enemy **weak points**, melee (LMB) chips **poise**,
a broken poise opens a **finisher** that grants a temporary damage boost. Allies and **possession** (the player
takes over an enemy body, which keeps its own moves) are first-class features. Melee weapons are **rig models**
that carry the attack animations: the body plays its attacks from the weapon model.

This document is the contract for the combat code. Code that disagrees with it is a bug in one of the two;
fix whichever is wrong in the same change.

---

## 0. Requirements

| # | Requirement | Section |
|---|---|---|
| R1 | Production-quality and stable from the start; no "good enough for v1" | all |
| R2 | Built from the engine's own building blocks: `Entity` subclasses own their logic (readable OOP), ECS components are data blocks queried by any entity, systems/events handle cross-entity rules. No replacement framework on top of the engine | §2 |
| R3 | Performant, easy to use, flexible | §2.4, §11 |
| R4 | Movesets in `.json`, ready for a future visual editor (round-trip serialization) | §5.2, §10 |
| R5 | Per attack: animation, speed, blend-in + start time depending on whether the previous attack was light or heavy, damage time(s), next-attack window, alternative (heavy) point, `heavy` flag | §5.2 |
| R6 | One light opener; spam / quick press → light; late press or **hold** → heavy (starts at the alternative point when held); an extra-long hold (0.6 s, during any move) → launcher instead, so a heavy that could still become it starts on release; free mixing | §5.4 |
| R7 | File access only through `FileSystemEngine::` | §10 |
| R8 | Combat data loaded only in `LoadAssets`; dev-only explicit reload compiled out of `DISTRIBUTION` | §10.3 |
| R9 | Single-player, net-ready: every cross-entity combat change goes through one funnel | §6.1, §12 |
| R10 | Allies and possession are real features | §4.4 |
| R11 | Enemy decisions in C++ brains behind an interface a Behaviour Tree can implement later | §8 |
| R12 | Game code only; the engine changes only where it cannot support a requirement, and those changes are listed in §13.1 | §13 |
| R13 | The body takes its attack animations from the weapon model (full character rig + weapon bones, no character mesh) | §9.3 |
| R14 | Recovery: when the combo window closes unused, control returns instantly; the attack plays out (weapon back to idle) until the player moves, then hands over to locomotion smoothly | §5.5 |
| R15 | Hundreds of enemies on screen: per-body work runs in parallel through the engine ECS, far bodies animate at a reduced rate | §2.2, §2.4 |
| R16 | Full air combat (DMC-like): the ground moves are the air moves (legs in the air pose); fighters attacking or attacked in the air have reduced gravity and hang together | §5.8 |

---

## 1. What changed from the previous design

| Previous design | Problem | Now |
|---|---|---|
| Separate combat code for NPCs (`MoveDef` + systems) and the player (`WeaponLogic`, `ComboPlayer`, `PlayerAction`) | Possession needs player input to drive *any* body; every feature written twice | One body class (`CombatCharacter`) and one move runner + moveset JSON for hero, enemies, allies, possessed bodies. Only the controller differs |
| Combo timings in code (`LinearComboLogic`, `makeLogic` lambdas) | Tuning requires recompiling; no editor possible | All timings in moveset JSON; code provides behaviours, data provides numbers |
| Static `Blocks`/`Cancels` bitmask per action | Can't express "dash may cancel this attack only after its hit frames" | Time-windowed per-move cancel windows in data |
| Many plug-in class families (`WeaponLogic`, `WeaponPoseLayer`, `ControllerRef`, `ControlLink`, …) | Abstraction without flexibility where it matters (numbers) | Engine building blocks + one behaviour registry; flexibility comes from JSON |
| Frontal-cone melee hits | Hits that visually miss and vice versa | Swept blade traces against hitboxes |
| Untyped `OnPointDamage(string bone…)` as the combat entry point | Lossy (no poise / weak-point / reaction data) | Typed `HitRequestEvent` funnel; `OnPointDamage` kept as the adapter for engine-originated damage |

---

## 2. Engine building blocks and how combat uses them

### 2.1 Mapping

| Engine block | Used for |
|---|---|
| `Entity` subclass (`Update` / `AsyncUpdate` / `LateUpdate`, `LoadAssets`, `Start`, `Destroy`) | `CombatCharacter` (every fighting body), `PlayerController`, `Projectile`, player start. All per-entity logic lives here, in plain readable methods |
| Member objects owned by the entity (like the old `Player` owned `CharacterController`, `SkeletalMesh`, `PlayerBodyAnimator`) | `CharacterController movement`, `SkeletalMesh* mesh`, `CharacterAnimator` (engine `Animator` subclass), `MoveRunner`, `WeaponSet` |
| ECS components (`REGISTER_COMPONENT` + `ECS_COMPONENT` on the entity class) | Combat data other entities and rules need to read/write on *any* entity: team, poise, weak points, status effects, combat state, attack tokens. Accessed as `character->GetPoise()` or `entity->GetComponent<PoiseComponent>()` |
| `Entity::Health` / `Entity::MaxHealth` | Health (not duplicated in a component) |
| ECS systems (`TICK_COMPONENT`, `DECLARE_SYSTEM`) | Pure data ticking: poise regeneration, status effect expiry, weak point regrow, hit-stop. Parallel scheduling: `CombatBodyUpdateSystem` runs every body's frame and `ProjectileUpdateSystem` every projectile's sweep across the thread pool (`ctx.ParallelFor`) |
| ECS events (`EcsScheduler::Emit` + `EVENT_HANDLER`) | `HitRequestEvent` (the single damage funnel, one handler) and feedback notifications (`HitLandedEvent`, `DeathEvent`, …) |
| `EntityHandle` | Every reference kept across frames: targets, attackers, token holders, possession links |
| `AiPerceptionSystem` (`Observer` / `ObservationTarget`) | AI sight |
| `PathFollowQuery` + `CharacterController` | AI navigation |
| `Spawner`, map entities, `REGISTER_ENTITY` | Placing / spawning enemies (`className = "enemy_grunt"`) |
| NLOHMANN structs (`NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT`, like `SkeletalMeshMetaData`) | Moveset / character / weapon / effect data with round-trip serialization |
| `CONSOLE_FUNC`, `UpdateDebugUI` (ImGui), `DebugDraw` | Dev tools |

### 2.2 Threading (engine frame order)

```
PreUpdate ECS        (main)   hit-stop / poise / status / weak-point / token systems
Level::Update        (main)   PlayerController: possession, camera look input (bodies have no Update work)
PostUpdate ECS                CombatBodyUpdateSystem (Characters/CombatBodySystems.cpp), every started body:
                     (worker)   1. BeginCombatFrame  own position / grounded / input edges            ParallelFor
                     (worker)   2. UpdateCombat      controller input (AI brains) -> MoveRunner (moves,  ParallelFor
                                                     combos, blade sweeps) -> facing -> aim
                     (main)     3. FlushCombatFrame  queued hits, cues, shots, cross-body actions,     body order
                                                     corpse removal, dev data reload
                              ProjectileUpdateSystem: sweeps (ParallelFor), then impacts (main, stable order)
                              then the tick group's event flush: EVENT_HANDLER(ResolveHitRequest)
Level::AsyncUpdate   (worker) CombatCharacter::AsyncUpdate: animator (clips slaved to the move clock, rate by
                              distance), root motion, CharacterController::Update, pose paste, weapon posing
Level::LateUpdate    (main)   PlayerController: camera, HUD; weapon trail entities
```

**Parallel update rules** (`Combat/CombatFrame.h`). During steps 1–2 a body reads other bodies (their step-1
position / grounded state, their components) and writes only itself. Everything else is queued on the body and runs
in step 3 on the main thread:

| Work | Path |
|---|---|
| Hit requests (blade sweeps) | `CombatCharacter::QueueHit` → emitted in body order, so resolution never depends on which thread swept |
| Move cues (sounds, particles, camera shake, trail on/off) | `PlayCue` queues while parallel |
| Shots (projectile entities, muzzle flash, fire sound, AI noise) | `WeaponSet::Fire` changes ammo / timers at once, `FlushShots` creates the rest |
| Trail entities | `SetTrail` records the request; `WeaponSet::LateUpdate` creates / stops the entity |
| Cross-body actions (finisher pairing / release, ragdoll, time scale) | `CombatCharacter::RunOnMainThread` (runs at once when already on the main thread) |
| Attack tokens on the target | `AttackTokensComponent` is locked (one mutex for all tables) |
| AI observers (perception lists) | created in `OnPossess` (main thread), only moved during the update |

Hits from all bodies are resolved together in one pass after the system, in body order (sorted ECS ids). Two
characters hitting each other on the same frame both land (a trade). When two attackers start a finisher on the same
victim in one frame, the first in body order pairs and the other finisher is interrupted.

### 2.3 Lifetime rules

* Cross-frame references are `EntityHandle`s, resolved when used; a resolved entity that is `Destroyed` is
  treated as gone.
* Combat data (`std::shared_ptr<const ...Data>`) is obtained in `LoadAssets` and shared by all entities of
  a type.

### 2.4 Performance rules

* No per-frame heap allocations in steady state (queued side effects are per event, not per frame).
* No string lookups in per-frame paths: move ids, slots and tags are resolved to indices / bitmasks when data
  loads; clip names to `hashed_string`.
* Body logic (AI, moves, sweeps, aim traces) runs in parallel (§2.2); pose evaluation and character movement run
  in `AsyncUpdate` (parallel across entities). Physics queries (Jolt narrow-phase) are safe from both.
* **Animation LOD** (`CharacterData::animationLod`): bodies farther than `halfRateDistance` from the camera evaluate
  their pose every 2nd frame, beyond `quarterRateDistance` every 4th. Clips are sampled at the move clock, so a late
  evaluation shows exactly where the move is; root motion becomes a velocity over the evaluated span; movement,
  hitboxes and weapon placement still update every frame. The controlled body is always full rate.
* Clips whose model has every body bone blend without the completion copy (§9.1).

---

## 3. The body: `CombatCharacter`

Every fighting body — hero, enemy, ally, possessed enemy — is a `CombatCharacter` configured by a character
JSON. Concrete types are thin classes that name their data file:

```cpp
class EnemyGrunt : public CombatCharacter
{
public:
    EnemyGrunt() : CombatCharacter("GameData/combat/characters/grunt.character.json") {}
};
REGISTER_ENTITY(EnemyGrunt, "enemy_grunt")
```

A generic `combat_character` entity reads the data path from the map key `def`.

| Part | Kind | Responsibility |
|---|---|---|
| `movement` | `CharacterController` member | capsule, gravity, collision |
| `mesh` | `SkeletalMesh*` (Drawables) | body, hitboxes from `.skmm` |
| `animator` | `CharacterAnimator : Animator` member | locomotion blend space, air poses, action layer slaved to the move clock, recovery layer, upper-body overlay, pose-snapshot transitions, root-motion extraction, weapon drive poses |
| `moves` | `MoveRunner` member | runs moves from the moveset: windows, combo resolution, recovery, cancels, cues, behaviours |
| `weapons` | `WeaponSet` member | weapon meshes (Bone props or Rig models, §9.3), hand / holster mesh swaps, blade and muzzle points + sweeps, trails, firearm state and holster timer, projectile spawning |
| `input` | `CharacterInput` member | what the current controller wants this frame |
| `controller` | `Controller*` | who drives the body (PlayerController or the body's own AIController) |
| `CombatBodyComponent` | ECS component | marks the body for `CombatBodyUpdateSystem` (§2.2) |
| `TeamComponent` | ECS component | team, original team (possession / allies), perception tag |
| `CombatStateComponent` | ECS component | dead, invulnerable, god mode, in finisher, hit-stop curve, victim shake, active move tags, last attacker |
| `PoiseComponent` | ECS component | poise meter, regen, armor resist from moves, broken timer (finishable) |
| `WeakPointsComponent` | ECS component | weak point health / broken / regrow per definition entry |
| `StatusEffectsComponent` | ECS component | active effects and aggregated modifiers |
| `AttackTokensComponent` | ECS component | how many attackers may engage this entity at once |
| `Entity::Health` | engine field | health |

Movement helpers (walk/run acceleration, air control, jump counts, glide, dash displacement, knockback, root
motion → velocity, target magnet on top of root motion, air stall, eased turning) are methods of `CombatCharacter`
operating on `movement`. Turning is rate-limited (`turnRate`) and eases into the target direction
(`turnSharpness`); nothing snaps the facing except the dash and the finisher alignment. Control is DMC-style
crisp: the hero reaches run speed and stops within ~0.1 s and turns 180° in ~0.2 s - weight comes from the strikes
(hit-stop, victim reactions, feedback, §6.3 / §9.4), never from sluggish locomotion.

Per-entity hit-stop: `CombatStateComponent` holds the hit-stop curve (duration, elapsed, scale); the character's
local delta time (`Time::DeltaTimeF * dilation`) drives its move clock, animation, movement and hit recoil, so
hit-stop stops exactly the two fighters without touching the global time scale (§6.3).

---

## 4. Control (`Control/`)

### 4.1 Controller ↔ body

```cpp
class Controller
{
public:
    virtual void UpdateInput(CombatCharacter& body, CharacterInput& input, float deltaTime) = 0;
    virtual void OnPossess(CombatCharacter& body) {}
    virtual void OnUnpossess(CombatCharacter& body) {}
};
```

`CombatCharacter::UpdateCombat` asks its controller for input first, so input is always fresh (the player's look
input was taken in `Level::Update`, before the body system). `CharacterInput` = move axis (world space), aim direction, focus target and **virtual buttons**
(`Attack`, `Secondary`, `Dash`, `Jump`, `Finisher`, `Interact`) with press/release times. AI may also call
`MoveRunner::TryStartMove(moveIndex)` directly. Both paths obey the same move rules, so a possessed enemy plays
by its own moveset and an AI-driven hero obeys the same rules as when the player drives it.

### 4.2 PlayerController (`Entity`, "player_controller")

Spawned by the player start; persists while the player hops between bodies. Maps `InputAction`s to virtual
buttons (camera-relative movement), runs the third-person camera in `LateUpdate`, owns the HUD, and performs
possession. The camera (`player.json` `camera`) orbits with mouse look and sits behind and above the body
looking down (`defaultPitch`), following a focus point on the body (`focusHeight` above the capsule center) with some
lag (`followSharpness`). Framing is in camera space: the view center sits `frameHeight` above the focus point along
the camera's own up axis (and `shoulderOffset` to its right), so the body keeps the same place on screen - below the
crosshair - at every pitch; looking down it never covers the crosshair and the shot composition does not change.
Looking up (down to `minPitch`) the boom shortens toward `lookUpDistance` and the framing offset with it (same
angle). Collision is a sphere trace (focus point → framed point → boom). Aim input is the camera ray (the
crosshair). On top of the framing the camera has an impact response (`camera.impact`, `CameraImpact`, §9.4): a
kick and a rumble within comfort limits, in unscaled time; shakes queued through the engine (`Camera::AddCameraShake`) are applied
after it. The HUD target marker shows the running attack's target, else the body the next attack would pick
(`CombatCharacter::PredictAttackTarget`), else what the camera looks at.

### 4.3 AIController

Owned by the body it was created for (`std::unique_ptr`), suspended while the body is possessed (keeps its
memory). Owns the brain, the `Observer`, path following and token requests.

### 4.4 Teams, allies, possession

* `Team { Neutral, Player, Enemy }`; friendly fire is off unless a hit says otherwise.
* **Ally** = a `CombatCharacter` on `Team::Player` whose AIController runs `AllyBrain`.
* **Possession** (`PlayerController::Possess(target)`): validated by the target's `possession.allowed`, alive,
  not in a finisher. The target's AIController is suspended and the target joins `Team::Player`; the hero gets
  an AIController with the hero data's `possession.idleBrain` (default defensive ally) so it is never a frozen
  dummy. **Release** returns control to the hero; the released body becomes an **ally** or **reverts** to its
  original team/brain. If the possessed body dies, control returns to the hero automatically.

---

## 5. Moves and movesets (`Actions/`, `Data/`)

### 5.1 Concepts

* **Move** — anything a body does with an animation: attack, dash, jump, glide, fire, hit reaction, finisher,
  death. Everything is a move, so everything obeys the same windows and rules.
* **Behaviour** — the C++ class giving a move its mechanics (`Attack`, `Dash`, `Jump`, `Glide`, `Fire`,
  `Reaction`, `Finisher`, `FinisherVictim`, `Charge`, `Death`), chosen by name in JSON, with a typed `params`
  block parsed and validated when the moveset loads. New mechanic = new behaviour class + one registration.
* **Channel** — `Primary` (one at a time, owns the body) or `Overlay` (upper body, concurrent with locomotion or
  with a primary move that allows it — firing while running).

All times are in **clip seconds** (unscaled). `speed` (and the attacker's `attackSpeed` status modifier, and its
hit-stop) scales the move clock, so windows stay on the animation: at `speed` 1.5 every window comes 1.5× sooner in
real time and retiming a move never needs its windows edited. Durations in plain seconds do not scale: `entry`
blend-ins, `blendOut`, recovery blends, `cooldown`, `inputBuffer`, and behaviour durations such as the dash's.
A move without a `clip` runs on its `endTime` alone (logic-only moves); `loop` + `loopStart` repeat
`[loopStart, endTime]` while the behaviour sustains (charges, glides, held fire).

### 5.2 Moveset file

```jsonc
{
  "version": 1,
  "name": "hero_scythe",
  "animations": ["GameData/models/weapons/scythe.glb"],  // clip sources, preloaded in LoadAssets; attack
                                                         // clips come from the Rig weapon model (§9.3)
  "inputBuffer": 0.4,                                  // s an Attack press stays valid (combos queue up)
  "actionBuffer": 0.15,                                // s a Dash / Jump / Secondary / Finisher press stays valid
  "holdCarriesOver": true,                             // the hold that started an attack counts as held for its link
  "holdDelay": 0.1,                                    // s Attack held from rest before the hold opener starts (§5.4)
  "holdDelayInMove": 0.6,                              // s Attack held during a move before the hold opener starts
  "blendOut": 0.2,
  "combos":    { "Attack": "light_1" },                 // optional "AirAttack": a different opener in the air (§5.8)
  "holdOpeners": { "Attack": "launch" },               // §5.4: Attack held -> the launcher
  "actions":   { "Dash": "dash", "AirDash": "air_dash", "Jump": "jump", "AirJump": "double_jump",
                 "Glide": "glide", "Secondary": "fire", "Finisher": "finisher", "Draw": "draw" },
  "reactions": { "Flinch": "react_flinch", "Stagger": "react_stagger", "Knockdown": "react_knockdown",
                 "Launch": "react_launch", "PoiseBreak": "react_broken", "FinisherVictim": "finisher_victim",
                 "Death": "death" },
  "moves": {
    "light_1": {
      "behavior": "Attack",
      "clip": "scythe_light_1",
      "animation": "GameData/models/weapons/scythe.glb", // optional when "animations" has one entry
      "speed": 1.15,
      "heavy": false,                                  // R5
      "tags": ["Attack", "Melee"],
      "channel": "Primary", "bodyMask": "Full", "overlay": "Block", "startState": "Ground",
      // bodyMask: Full | NoLegs (+ "legWeight") | UpperBody | Bones (+ "maskBones": ["clavicle_r"]) - §9.1
      "entry": {                                       // R5: blend + start time by previous attack
        "fresh":      { "blendIn": 0.08, "startTime": 0.00 },
        "afterLight": { "blendIn": 0.10, "startTime": 0.06 },
        "afterHeavy": { "blendIn": 0.16, "startTime": 0.12 },
        "afterHold":  { "blendIn": 0.10, "startTime": 0.08 }   // a hold move released early handed over to this one
      },
      "endTime": 0.95, "blendOut": 0.2, "cooldown": 0,
      "hits": [ { "start": 0.30, "end": 0.42, "weapon": "Main", "damage": 12, "poiseDamage": 5,
                  "reaction": "Flinch", "knockback": 3.5, "hitStop": 0.07, "shake": 0.04 } ],  // R5: damage time
      "combo": { "windowStart": 0.45, "alternativeAt": 0.65, "windowEnd": 0.90,     // R5: window + alt point
                 "next": "light_2", "alternative": "heavy_2" },
      "recovery": { "lowerBodyBlend": 0.15 },          // R14: from combo.windowEnd unless "start" is set
      "cancels": [ { "start": 0.45, "into": ["Dash", "Jump"] },
                   { "start": 0.65, "into": ["Locomotion"] } ],                    // §5.6: the stick ends it from here
      "rootMotion": { "enabled": true, "scale": 1.0 },
      "tracking": { "turnRate": 600, "magnetRange": 3.0, "idealDistance": 1.5, "maxMagnetSpeed": 7 },   // §5.7
      "armor": [ { "start": 0.25, "end": 0.42, "poiseResist": 0.5 } ],
      "invulnerable": [],
      "cues": [ { "time": 0.30, "sound": "event:/Weapons/knife/knife_attack" },
                { "time": 0.30, "trail": 1 }, { "time": 0.44, "trail": 0 } ],
      "params": {}
    }
  }
}
```

Every field except `clip` (or `endTime` for clip-less moves) has a default. Loading re-serializes the parsed data
and warns about any source key that did not survive the round trip (typos never fail silently); references (move
ids, slot names, tags, behaviours, window order) are validated with the full JSON path in the message.

`variants` lists more clips of the same source sharing the move's timing; a behaviour picks one by clip index
(0 = `clip`, n = `variants[n - 1]`) — Fire uses it for akimbo recoil (§5.7):

```jsonc
"fire": { "behavior": "Fire", "clip": "fire_r", "variants": ["fire_l"], "channel": "Overlay", "bodyMask": "UpperBody",
          "params": { "weapon": "Secondary", "restartOnShot": true } }
```

Move fields not shown above: `airLegs` (Full moves in the air blend the legs to the air pose, default on, §5.8),
`legWeight`, `variants`, `loop` / `loopStart`, `hold` (§5.4).

Hit window fields: `start`, `end`, `weapon` (blade of that slot) **or** `bone` + `radius` (body-bone sphere, kicks),
`direct` (paired moves: hits the move's target without a sweep), `source`, `damage`, `poiseDamage`,
`weakPointDamage` (default = damage), `reaction`, `knockback` (m/s push; the victim's reaction move decides over how
long), `launch`, `hitStop` (s), `hitStopScale` (time scale while stopped, default 0.03), `shake` (m the victim's mesh
shakes), `camera` (view kick when the player lands it, impact units; default 0 - light hits never move the view),
`impactScale` (× the victim's hit effect), `air` (what the hit does in the air - `lift`: at least this upward speed
for an airborne victim and an airborne attacker landing it, `stall`: seconds of air combat for the victim, §5.8),
`hitSound`, `hitParticle` (override the victim's `hitEffect`).

Tracking fields (§5.7): `searchRange` (target search radius, all around the body), `until` (turn toward the target
until; < 0 = the first strike),
`startTurnRate` for the first `startTurnTime` (the attack lines up fast), then `turnRate`, `turnSharpness` (easing of
the attack's turns), `magnetRange`, `idealDistance`, `magnetUntil` (< 0 = the end of the last strike),
`maxMagnetSpeed`, `advance` (m/s forward while input is held and there is no target).
Cue fields: `time`, `sound`, `particle` (at `bone`), `trail` (1 on / 0 off for `trailWeapon`), `cameraShake`.

### 5.3 Hero combo graph

"The same combo in a light and a heavy variation" = step *n* of either variation links to step *n+1* of both:

```
            next        next        next
light_1 ───────► light_2 ───────► light_3 ───────► (end)
       ╲ alt        ▲   ╲ alt        ▲
        ▼   next    │    ▼   next    │
       heavy_2 ─────┘   heavy_3 ─────┘
          └────alt────► heavy_3
```

`light_1` is the only opener. Branches, loops and finisher enders are just other `next`/`alternative` targets.
The shipped hero opens with `start` (the wind-up and first strike; `light_1` begins in its hold pose); holding Attack
starts the launcher `launch` instead - from rest after 0.1 s, during any move (a combo included) after 0.6 s (§5.4).

### 5.4 Combo resolution (R6) — `ComboResolver`

For an attack with a `combo` link (`ws`, `a`, `we` in its clip time):

```
clip time:  0 ─────────────── ws ═══════════ a ═══════════ we ──── endTime
                buffering        LIGHT zone      HEAVY zone     closed
```

| Input | Result |
|---|---|
| Press **and release** during this attack before `ws` (spam) | **light** starts exactly at `ws` |
| Press and release inside `[ws, a)` | **light** starts on release |
| Button **held** when the clip reaches `a` (pressed in the window, during the attack, or - with `holdCarriesOver` - the hold that started this attack) | **heavy** starts at `a` - on release where the launcher can start (below) |
| Press inside `[a, we]` | **heavy** starts on the press - on release where the launcher can start |
| Held for `holdDelayInMove` (0.6 s, from the press; not before `ws`) where the launcher can start | the **hold opener** (launcher) starts instead of the heavy, committed |
| Nothing by `we` | link closes; the attack goes into **recovery** (§5.5) and a later press starts the opener |

* **Heavy or launch.** Where the launcher can start (on the ground, off cooldown) a heavy press might still be
  becoming the 0.6 s hold, so the heavy waits for the button: released before 0.6 s - the heavy, on release (a burst
  held across `a` still reaches its heavy); held 0.6 s - the launcher, never both. While such a press is held the
  link stays open past `we` (no recovery), so a burst near the end of the window released after it still gets its
  heavy. Where the launcher cannot start (in the air) there is nothing to wait for and the heavy starts at once.
  Lights are unaffected: they start on release anyway.
* **Keep the light zone short** (the hero's: 0.08 clip s, ~5 frames). Light presses come early - taps made before `ws`
  are buffered and start the light exactly at `ws` - so the zone only has to hold a tap made as the window opens.
  Heavy presses come slightly before the moment the player aims for, so the heavy zone starts soon after `ws`; a long
  light zone turned those presses into lights and made heavies hard to reach.
* If the link has only one branch (only `next`, or only `alternative`, or the other branch cannot start in the
  current state), any qualifying input takes the existing branch. If neither can start, presses go to the opener of
  the current state.
* Taps released longer than `inputBuffer` before the window opens are stale and ignored; the press that started the
  current attack is never a *new* press (with `holdCarriesOver` it only counts as held for the heavy).
* The next attack uses `entry.afterHeavy` if the current attack is `heavy`, else `entry.afterLight`; attacks
  started outside a combo - and the launcher after a long hold - use `entry.fresh`.
* The resolver is pure logic (no engine calls) and covered by the self tests (§11). AI brains press follow-ups as
  taps: a light at `ws`, a heavy at `a`.

**Hold opener (launcher).** A combo slot may name a hold opener (`holdOpeners`). An Attack press that no combo window
owns then waits instead of starting the opener at once (`DecideOpener`, pure, self-tested). How long it must be held
depends on where it was pressed:

| Press | Released earlier (a tap) | Held long enough |
|---|---|---|
| From rest - no move running, or a recovery playing out: `holdDelay` (0.1 s) | the combo **opener** starts on release | the **hold opener** starts with its wind-up. Released during the wind-up (clip time < `hold.until`) it hands over to `hold.release` - the usual attack - entered with that move's `entry.afterHold`; held through `hold.until` it commits (the launch strike) |
| During a move - a dash, an attack past its combo window: `holdDelayInMove` (0.6 s) | the combo **opener** starts on release (once the move lets an attack in) | the **hold opener** starts **committed**: the hold already showed the intent, its release no longer hands over |

* Both paths commit the launch at about the same hold: from rest 0.1 s + the wind-up (0.26 clip s at speed 0.7 =
  0.37 s), during a move 0.6 s. From rest the short delay keeps the opener responsive (a longer tap still gets it
  through the wind-up hand-over); during a move there is no wind-up hand-over, so the delay itself is long.
* A press that cannot start anything yet (a dash before its Attack cancel) stays pending and decides when it can: a
  hold already long enough starts the hold opener at once. Where a press was made decides its rule, not where the
  body is when it is judged: a press made during a dash and held past the dash's end still needs 0.6 s (judged from
  rest it would pop the launcher's wind-up the moment the dash ended).
* A reaction or death (`StartForcedMove`) spends the held press: holding through a stagger fires nothing afterwards.
* A ground launcher cannot start in the air: there an unowned press starts the air opener at once, and combo heavies
  start at once (the table above).
* `hold` (`until`, `release`) is a move field: any Primary move can require its button held through a wind-up.
  Validation warns when a hit window opens before `until` (an early release would cut a landed strike). AI bodies
  that start a hold move without holding Attack hand over at once.

### 5.5 Recovery (R14)

The tail of an attack after its combo window is not gameplay; it only brings the weapon back to idle.

```
clip time:  0 ── hits ── ws ═══ combo window ═══ we ┊ recovery ─────────── endTime
                                                     ┊ the move has ended: locomotion, dash, jump, attacks
                                                     ┊ the clip keeps playing: whole body until the body moves
```

* `recoveryStart` = `recovery.start` if set, else `combo.windowEnd` for moves with a combo link; moves without
  either (dash, reactions, finishers by default) play to `endTime`. `recovery.enabled: false` turns it off.
* When the primary move reaches `recoveryStart` and the combo did not fire, `MoveRunner` ends it with
  `MoveEndReason::Recovered` and keeps a `RecoverySlot` (move, clip time, serial). The body is free at once:
  locomotion and facing follow input, any move may start (an attack press starts the opener).
* The recovery clock keeps advancing (`speed` × attack speed) and fires the move's remaining `cues` (sheathe
  sounds…); hit windows and behaviours no longer run. Any new primary move clears it; overlay moves (firing) play
  over it.
* Animation (§9.1): the animator keeps sampling the *same* clip instance — no restart, no transition. Until the
  body moves the clip plays on the **whole body** and its root motion still moves the capsule, so a long exit plays
  exactly as authored and ends in the idle stance. When the body moves (movement input, airborne, death) the body
  calls `MoveRunner::ReleaseRecovery()` (latched in the `RecoverySlot`): root motion stops, the legs hand over to
  locomotion over `recovery.lowerBodyBlend` (default 0.2 s) and the upper body over `recovery.upperBodyBlend`
  (default 0.35 s; < 0 keeps it until the clip ends). Whatever still plays when the clip ends fades over the move's
  `blendOut`. Rig weapon bones follow the upper-body weight, so the weapon returns to its hold pose (§9.3).
* Validation: recovery needs a clip and the `Primary` channel; `recovery.start` may not precede
  `combo.windowEnd`.

### 5.6 Cancels, overlays, forced moves

* Move **X** may start while move **M** runs iff M has a `cancels` window active now whose `into` intersects X's
  `tags`, or M has reached `endTime`. Combo links are resolved before cancel checks.
* `Locomotion` in `into` is movement input: inside that window a primary move ends as soon as the stick is pushed
  (≥ 0.3) - unless Attack is held or an Attack press is waiting (the player is still feeding the combo). A move with a
  clip hands over like a released recovery (§5.5: the clip keeps playing while legs, then upper body blend to
  locomotion); without one it ends. Attacks open it after their light zone closes (`combo.alternativeAt` or later;
  the hero's ~0.1 s later, so a heavy pressed a little late while moving still lands), enders right after their
  strike: mashing with the stick held steers the combo, a delay (heavy) combo is done standing still (DMC rule), and
  a lone attack frees the body ~0.25 s after its strike instead of at the end of the combo window. Validation warns about a `Locomotion` window inside the light zone, and rejects `Locomotion` as a move tag.
* Buffers: an Attack press waits `inputBuffer` (combos and cancels into attacks queue up); every other action waits
  `actionBuffer` - a dash pressed long before its cancel window opens is not wanted any more when it would fire.
* `Overlay` moves start only when the primary move has `"overlay": "Allow"` (or none runs); starting a primary
  move with `"overlay": "Block"` interrupts the overlay. A blocking primary also suspends aiming (§9.1): the dash
  faces its direction and the aim offset fades out; jumping and gliding allow overlays and keep aiming.
* Forced moves (hit reactions, finisher victim, death) ignore cancel windows; armor decides whether a reaction
  plays at all.

### 5.7 Behaviours

| Behaviour | Mechanics | `params` |
|---|---|---|
| `Attack` | hit windows → blade sweeps / bone spheres (run by `MoveRunner` + `WeaponSet`); **target assist**: the controller's focus, else the **attack target** (`CombatCharacter::FindAttackTarget`): every hostile within `tracking.searchRange` **all around the body** and in sight, scored by its angle from the move input direction (the facing without input) plus `targeting.distanceWeight` × distance / range, the previous attack's target favoured by `targeting.keepBonus` - pointing the stick at a body picks it, a near tie keeps the combo's target; the body turns toward it (or the input direction) until `tracking.until` - never snaps: fast for the first `startTurnTime` (`startTurnRate`), then at `turnRate`; the **magnet** adds speed toward it on top of the clip's root motion until `idealDistance`, arriving by the next strike, until `magnetUntil` (stationary clips still close in), and never lets anything carry the body closer; without a target `tracking.advance` steps forward while input is held. **Air** (§5.8): run in the air the attack keeps the body in air combat, follows the target's vertical motion, lifts the attacker with every hit on an airborne victim, goes on on the ground after landing; `plunge` dives in the air and holds its strike until landing; `launchSelf`: the attacker leaps too - at clip `time`, or `onHit` when one of the move's hits actually launches its target (armor downgrading the launch, or a kill, leaves it on the ground) - optionally only while Attack is held | `locomotion`, `air` (`matchTargetHeight`), `plunge` (`start`, `speed`, `hold`), `launchSelf` (`time`, `onHit`, `verticalSpeed`, `requireHeld`) |
| `Dash` | displacement along input (or facing), gravity suspended, i-frames from `invulnerable`, stops at walls, air charges. EaseOut slows linearly to `exitSpeed` × run speed, so the body runs out of the dash; after the displacement input steers the body (`tailLocomotion`). Jump cancels it from the first frame (data) | `distance`, `duration`, `curve`, `airCharges`, `exitSpeed`, `tailLocomotion`, `faceDirection` |
| `Jump` | vertical velocity; ground or air (air jumps refill on landing); ends on landing; horizontal speed carries into the jump up to `carrySpeed` × run speed (a dash-jump leaves at run speed) | `velocity`, `maxAirJumps`, `airControl`, `carrySpeed` |
| `Glide` | started by the move runner when the body falls with Jump held for `holdDelay` (no second press); reduced gravity, capped fall speed, limited time per airtime; ends on release; sustains at `endTime` | `maxDuration`, `gravityScale`, `maxFallSpeed`, `airControl`, `holdDelay` |
| `Fire` | overlay; fires on start and at the weapon's fire rate while held (or per press), projectiles from the next muzzle toward the aim point, strafe-to-aim facing, locomotion at `moveSpeed`; a weapon drawn by the move fires on the next frame (muzzle in the hands). With `restartOnShot` every shot restarts the clip (`MoveRunner::RestartClip`) as clip n for the weapon's n-th muzzle | `weapon`, `button`, `automatic`, `aimTurnRate`, `restartOnShot`, `moveSpeed` |
| `Draw` | puts `weapon` in the hands when it starts (the clip begins with the grab: the other weapon is put away and this one shown at once) and plays the draw clip, usually an UpperBody overlay. The body starts the `Draw` action by itself once another weapon's `holsterDelay` ran out (§9.3) | `weapon` |
| `Reaction` | knockback / launch handed over by the hit funnel; optionally held while poise is broken | `knockbackScale`, `knockbackTime`, `launchScale`, `holdWhileBroken` |
| `Finisher` / `FinisherVictim` | paired: victim aligned in front of the attacker, both `inFinisher` (pairing and release on the main thread, §2.2); damage from `direct` hit windows; reward status effect on completion; slow motion | `distance`, `reward`, `slowMotionDuration`, `slowMotionScale` |
| `Charge` | windup, then rush with limited homing until hit / wall / timeout; optional self-stagger on wall crash | `speed`, `chargeStart`, `maxDuration`, `homingRate`, `stopOnHit`, `crashStagger` |
| `Death` | death clip held on its last frame, optional ragdoll | `ragdoll`, `ragdollDelay` |

### 5.8 Air combat (R16)

* **One combo for ground and air.** The ground moves are the air moves: `start` → `light_*` / `heavy_*` have
  `startState: Any` and the moveset needs no `AirAttack` opener (`GetOpenerSlot`: without one, a press in the air
  starts the `Attack` opener). Jumps, double jumps, glides and air dashes cancel into them; a combo carries on through
  take-off and landing - landing mid-attack the move simply continues on the ground. In the air a Full-body move
  blends its legs toward the air pose by `legWeight` (faded in with take-off - `airLegs`, on by default; jumps and
  dashes, whose clips pose their own air legs, turn it off), root motion is scaled by `airCombat.rootMotionScale`
  (ground lunges would carry the body off) and knockback taken by `airCombat.knockbackScale` (the attacker stays in
  reach).
* **Air combat** (`CharacterData::airCombat`, `CombatCharacter::EnterAirCombat`): a body is in air combat while
  airborne and fighting - running an attack (and for `hold` after it), or for a hit's `air.stall` after being hit in the
  air or launched. Rising, it keeps the normal arc (`riseGravityScale`): a jump or a launch reaches its top. Falling,
  gravity is scaled by `fallGravityScale` and a faster fall eases to `maxFallSpeed` (no snap, gravity never pushes past
  it): fighters hang at the top of the arc and sink slowly together. Landing ends it. Outside air combat (a combo
  over, no hit for a while) the body falls normally.
* **Together**: an attack in the air follows its target's vertical motion - the target's vertical speed plus a pull
  toward its height (`air.matchTargetHeight`): attacker and launched target rise, hang and sink as one; against a
  grounded target the attacker comes down onto it. A hit on an airborne victim lifts it to at least `hit.air.lift`, and
  an airborne attacker landing it gets the same lift, so a combo bobs the pair up a little with every hit.
* **Plunge**: an attack's air version may dive (`plunge`: from `start` at `speed`, the clip held at `hold` - strike pose,
  hit window open - until it lands, `MoveBehavior::HoldsClock`); landing, the clip and the move go on on the ground.
  The hero's `heavy_1` - the pause after the opener - plunges in the air.
* **Launcher**: the hero's `launch` (clip `launch`, played at 0.7), the `Attack` hold opener (§5.4): wind-up until
  0.26 (from rest, released earlier, the usual attack `start` takes over; after a 0.6 s hold during another move it is
  committed from the start), a rising slash 0.27 - 0.37 with a `Launch` hit
  (`launch` 10.5 m/s: ~2.2 m up), 1.2 m of root motion forward, armor through the wind-up. When the hit launches its
  target the hero goes up with it (`launchSelf.onHit`, 9.6 m/s = 10.5 × √(21 / 25): the hero's gravity is 21, a
  grunt's 25, so both peak at the same height); both rise after the shared hit-stop. Recovery starts at 0.4: airborne,
  it releases at once - the legs blend to the air pose while the arms finish the follow-through (the clip itself
  stands on the ground). A whiff, or a target whose armor turns the launch into a stagger, leaves the hero on the
  ground; a launcher that breaks the target's poise still launches it (it flies in its broken pose and stays
  finishable). Launches survive their hit-stop: the movement's blocked-rise check (`CombatCharacter::ResolveRise`)
  measures the physics' vertical speed in the dilation it was applied with - judged in this frame's dilation, every
  launch was cut to a hop where the hit-stop eased out.

---

## 6. Combat rules (`Combat/`)

### 6.1 The funnel

Anything that changes another entity's combat state is a `HitRequestEvent` emitted on the main thread.
`EVENT_HANDLER(ResolveHitRequest, HitRequestEvent)` is the only handler and the only place combat state of a
victim changes. Entities without combat components (engine props, destructible brushes) receive the result
through their normal `OnPointDamage`. Engine-originated damage arriving at a `CombatCharacter` through
`OnPointDamage` is converted into a `HitRequestEvent`.

### 6.2 `HitRequestEvent`

attacker / victim (`EntityHandle`), source (`Melee`, `Firearm`, `Finisher`, `Environment`, `Explosion`), damage,
poise damage, weak-point damage, requested reaction, knockback, launch, hit-stop, `heavy`, world point / normal /
direction, hitbox name, flags (`IgnoreArmor`, `IgnoreInvulnerable`, `FriendlyFire`).

### 6.3 Resolution order

1. Victim alive and not invulnerable; attitude hostile unless `FriendlyFire`.
2. **Weak point** by hitbox → weak-point damage if the source is accepted; on break: `BreakPoise`, `Kill`,
   `DamageBonus`, regrow timer.
3. **Damage** = base × attacker `damageDealt` × victim `damageTaken` modifiers × `brokenDamageTaken` while
   broken × intact weak-point multiplier × broken weak points' `bonusDamageTaken`.
4. **Poise** damage × attacker / victim poise modifiers × source scale (`Firearm`: the victim's
   `poise.firearmScale`; a weak-point `BreakPoise` = instant break) × (1 − armor resist). Poise ≤ 0 →
   **Broken** (finishable).
5. **Health** (`Entity::Health`); ≤ 0 → death.
6. **Reaction**: death > poise break (the `PoiseBreak` reaction move; the hit staggers - a `Knockdown` / `Launch` hit
   still knocks down / launches, `PoiseBreakReaction`) > requested reaction (`heavy` upgrades `Flinch` → `Stagger`
   against unarmored targets; armor downgrades it, hyper armor removes it; a broken body only reacts to `Knockdown` /
   `Launch`). Started through `CombatCharacter::OnHitResolved`.
7. Hit-stop on attacker and victim (DMC3 reference): both stop at the contact frame (`hitStopScale`, default 0.03 -
   a near-freeze), hold, and ease back to normal speed over the last 30 % of `hitStop`. The onset is instant - an
   eased onset lets the blade slide through the body and the contact reads soft; the eased release keeps it from
   popping. Durations carry the weight: light ~0.05-0.07 s (3-4 frames), enders ~0.1 s, heavies 0.12-0.16 s. A hit
   during a hit-stop stops the body again for the longer of the two at the deeper scale. The victim's mesh shakes
   along the hit direction (`shake`, at least 0.12 s), starting displaced, so the contact frame already shows the
   blow; its upper body recoils (§9.1). A victim in the air gets its air stall (§5.8). Knockback, `HitLandedEvent`
   for feedback (§9.4).

### 6.4 Poise, broken, finisher

* Poise regenerates after `regenDelay` without poise damage. Armor windows multiply poise damage by
  `(1 - poiseResist)`; `1` = hyper armor.
* **Broken** lasts `breakDuration`: only forced moves play, the body is finishable and takes
  `brokenDamageTaken`× damage; poise refills when it ends.
* `Finisher` near a finishable enemy (range / angle from the attacker's data) starts the paired moves; on
  completion the attacker gets the reward status effect (e.g. `finisher_boost`: +50 % damage for 8 s).

### 6.5 Weak points

Per character: name, hitbox bones, health, accepted sources (default `Firearm`), multiplier while intact, break
effects, regrow time (0 = permanent). "Guns break weak points, swords chip poise" is per-enemy data.

---

## 7. Hit detection

* **Melee**: weapons define a blade segment (base → tip, radius, samples) in the space of their socket bone
  (`blade.bone`, required for Rig weapons, §9.3) or of the weapon model; bone-sphere hits (kicks) use a body bone
  + radius. `AsyncUpdate` stores the socket transform after the pose is final; the next `Update` sphere-sweeps
  each sample from its previous to its current position (`Physics::MultiSphereTrace`, overlap on the first frame)
  against `HitBox | MainBody`, ignoring the attacker and friendlies, while a hit window is open. A per-window
  hit-once list prevents double hits.
* **Firearms**: shots leave from the muzzle socket (`firearm.muzzles`); `Projectile` entities carry a
  `ProjectileComponent`; `ProjectileUpdateSystem` sweeps them all in parallel (`Physics::MultiSphereTrace`), then
  resolves impacts on the main thread in a stable order. They pass through friendlies and corpses and emit hit
  requests; tracer = engine particle system.

---

## 8. AI (`AI/`)

* `AIBrain::Think(AIContext&)` runs inside the body's parallel update (worker threads). The context offers
  perception results, current target, distances, navigation, token requests and commands (`TryStartMove`, `MoveTo`,
  `Strafe`, `SetFocus`). Brains never touch health, meshes or other entities directly - which is what keeps them
  thread-safe (tokens are locked, §2.2).
* Brains: `MeleeBrain`, `RangedBrain`, `AllyBrain`; registered by name, selected by `ai.brain`.
* Attack tokens live on the **target** (`AttackTokensComponent`), so enemies take turns instead of swarming.
* Sight uses the engine `AiPerceptionSystem`: every body registers an `ObservationTarget` tagged with its team;
  AI bodies own an `Observer`.
* BT-ready: a `BehaviorTreeBrain : AIBrain` can map the context to a blackboard later; nothing else changes.

---

## 9. Animation and feedback

### 9.1 Body composition (`CharacterAnimator : Animator`)

Evaluated in the body's `AsyncUpdate` from an `AnimatorInput` built from the move runner's state, in this order:

1. **Locomotion** — idle + 4-direction run blended by velocity relative to facing, phase-synced, playback rate
   from speed; fall / glide poses in the air. The idle is the **stance idle** of the weapon in the hands when it has
   one. A body with **no** directional clips strafes procedurally (`locomotion.strafe`): while its facing is not driven
   by the movement (aiming, firing, AI facing a target) the legs (`legsBone`) run the forward clip turned toward the
   move direction, at most `maxTwist`, following direction changes at `turnRate`; moving backward (past 90° ± 20°
   hysteresis) they run a second instance of the clip played backward, turned away from it, cross-fading over
   `switchTime`; the `counterBones` turn the torso back to the facing. Some directional clips present: missing
   directions play the forward run. On the ground the torso leans into acceleration (`locomotion.lean`: forward
   when speeding up, inward when turning, smoothed).
2. **Stance hold** — the weapon's `stance.hold` clip on each `stance.holdBones` subtree (the scythe idle on the
   right arm, the pistol aim on the upper body): how the weapon is carried during locomotion. Move clips pose the
   arms themselves, so the hold stays under every move.
3. **Full-body primary** — the move clip sampled at the move clock replaces 1–2. `NoLegs` moves, and `Full` moves
   in the air (`airLegs`, faded by the air weight with take-off and landing), then blend the `locomotion.legBones`
   subtrees (thighs down) toward the locomotion / air pose by the move's `legWeight`, local to the clip's hips: the
   hips swing with the attack, the legs mostly keep the air pose.
4. **Recovery** (§5.5) — the ended attack's clip on the same `Animation` instance. Full-body weight fades
   1 → 0 over `lowerBodyBlend` (legs to locomotion), then the upper body (`LayeredLerp` from
   `locomotion.upperBodyBone`) holds it until the clip ends and fades over `blendOut`.
5. **Upper-body / bone-masked primary**, then the **overlay** (firing, drawing) faded in / out by its `blendIn` /
   `blendOut`; the overlay is on top because it only runs with primaries that allow it. `UpperBody` layers from
   `locomotion.upperBodyBone` (mesh space); `Bones` only on the move's `maskBones` subtrees (local space) - the draw
   shows on the right arm alone, like the scythe's stance hold.
6. **Aim** — a weapon whose stance lists `aimBones` (the pistols: `spine_01..03`) is an aiming weapon: while it is
   in the hands the body faces the aim direction (strafes, turning at `aimTurnRate`) and those bones turn the hold
   pose toward the aim point (the controller's: crosshair for the player, target for AI): yaw about up, then pitch,
   clamped to `aimMaxYaw` / `aimMaxPitch`, spread evenly over the bones, weighted by the stance. The hold pose aims
   along the body's facing; shots leave from the muzzles toward the same point. When another overlay clip takes
   over a visible one (each shot restarting its recoil clip), it blends from the previous pose over its `blendIn`.
   Aiming applies only while no primary move blocks overlays (§5.6); the aim offset fades in / out over 0.15 s.
7. **Grip** — the weapon's `stance.handBones` (the hands holding it) take the grip of the clip posing the weapon in
   local space: its overlay clip, else its move / recovery clip (clips from the weapon's own model), else its hold
   clip, weighted by how much each shows. Blends and mesh-space layers move the arms, never the grip, so the weapon
   stays tight in the hand.
8. **Hit recoil** (`hitRecoil`) — every hit bends the upper body away from the blow at once by
   `knockback × degreesPerKnockback` (≤ `maxAngle`; armored hits half), spread over the `bones` chain by weight
   (default spine_01..03 + neck_01); a damped spring (`frequency`, `damping`) in the body's local time brings it back,
   so the bend holds through the hit-stop and sways back after it. It is directional - reaction clips only ever
   recoil backward - and works under armor and on bodies without reaction clips (the hero). Applied on the final
   pose, outside transitions: a reaction that starts mid-recoil does not freeze it into its blend.

Stances cross-fade over `stance.blend` when the weapon in the hands changes (idles and holds are weight-
normalized, so a swap never dips into the base pose); stance clips loop on the body's idle clock, so standing
still the hold matches the stance idle. Clips of models lacking some body bones (the pistols model has no coat
bones) leave those bones to the pose underneath (each layer is completed from the base before blending —
`Lerp`'s fast path would drop them, `LayeredLerp` would collapse them to identity); clips whose model has every
body bone skip that copy. Layers rooted at
`locomotion.upperBodyBone` blend its rotation in mesh space (the torso keeps the layer's facing whatever the
legs do); limb layers blend in local space. Partial layers never change the `root` bone (the engine's blends take it
from the layer; it belongs to the legs). Clips are `Animation` instances per clip file (`animations` of the
moveset, the weapons' stance sources), loaded in `LoadAssets`.

### 9.2 Transitions and root motion

* Pose snapshots: on a primary start (`entry.blendIn`), end (`blendOut`) or interruption the last output pose is
  frozen and blended into the new composition. Recovery is not a transition — the clip continues.
* Root motion: `root` bone delta pulled from the primary clip, scaled (and magnetized), rotated to world and
  converted to velocity for `CharacterController` (collision preserved); `root` translation is stripped from the
  final pose. Recovery never moves the capsule.

### 9.3 Weapon models (R13)

`WeaponMeshData.attach` selects how a weapon mesh is posed:

| Mode | Model | Posing |
|---|---|---|
| `Bone` | a rigid prop | attached to a body bone (`hand.bone` / `holster.bone`) with `offset` / `rotation` / `scale` |
| `Rig` | the full character rig + weapon bones, no character mesh | drawn with the body's transform; body bones copied from the body's final pose; weapon-only bones posed as below |

A melee weapon is a `Rig` model and **carries the attack clips**: the moveset's `animations` list the weapon
model, so the body plays its attacks from it (its body bones match the body skeleton; the body ignores the
weapon bones). Validation warns when a Rig weapon model is not an animation source of the character's moveset.

Weapon bones of a Rig mesh (bones the body skeleton does not have), every frame in `WeaponSet::AsyncUpdate`:

* **Rest**: the attachment's `clip` sampled at `clipTime` when loading (`hand` while in the hands, `holster`
  otherwise; bind pose without a clip).
* **Drive**: while in the hands, blended toward the clip poses driving the body — the primary move clip (weight
  1) or the recovery clip (its upper-body weight), then the overlay clip (its weight). Clips from a model without
  a given weapon bone leave it at rest.
* **Transitions**: when another clip takes over, the weapon bones cross-fade from their last output over the
  same time the body uses (`entry.blendIn` / `blendOut`); hands ↔ holster snaps.
* **Follow bone**: with `hand.bone` / `holster.bone` set, weapon root bones keep the transform *relative to that
  body bone* that the clip has, so the weapon stays in the hand through every body blend (lower-body recovery,
  snapshots, upper-body layers) whatever bone the rig parents it to. Empty = parent-local posing.
* **Points**: the blade (`blade.base` → `blade.tip`) and every muzzle are a weapon-model bone plus an `offset` in
  its space, evaluated on the final weapon pose (a bone is required for Rig weapons). The scythe sweeps between its
  `attack_start` and `attack_end` bones; shots cycle through every muzzle of every mesh (`muzzle_r`, `muzzle_l`).
* **Hand / holster copies**: `hand.hide` / `holster.hide` list meshes of the model hidden in that state
  (`StaticMesh::MeshHideList`, name prefix) — the scythe model carries an in-hand (`active`) and a back (`holser`)
  copy. A weapon without a holster copy uses `visibleWhenHolstered: false`.
* **Built-in weapons**: a weapon file without meshes is part of the character model (an enemy's modelled axe or
  gun); its points use the body's bones.
* **Holster timer**: a weapon other than Main with `holsterDelay` > 0 goes back once that many seconds passed since
  it entered the hands or last fired — the body plays the moveset's `Draw` action (instant swap without one).
  Attacks swap to their hit windows' weapon instantly.

### 9.4 Feedback

Feedback handlers on `HitLandedEvent` / `PoiseBrokenEvent` / `FinisherEvent` (`CombatFeedback.cpp`); move `cues`
drive move-specific sounds / trails / particles; the finisher's slow motion is `Time::AddTimeScaleEffect`. Rules
never depend on feedback. The style reference is DMC3: semi-realistic, light hits sell themselves through the stop,
the victim and blood - no flashes, no camera on every swing.

| Layer | Source | What it does |
|---|---|---|
| Hit-stop + victim shake | hit window (`hitStop`, `shake`) | §6.3 |
| Hit recoil | victim's `hitRecoil`, hit's `knockback` | §9.1 step 8 |
| Pushback | hit's `knockback`, victim's reaction move (`knockbackTime`) | a light hit moves the victim ~0.35 m, a heavy ~1 m; reactions start at the clip's recoil (`entry.startTime`), not at a lead-in |
| Hit effect | victim's `hitEffect` (`particle`, `sound`, `scale`) × the hit's `impactScale`; a hit's `hitParticle` / `hitSound` override it | spawned at the contact spraying along `HitRequestEvent::spray` (the blade's sweep; projectiles: back out of the entry wound). `hit_flesh` spawns `scale` blood pieces that drip and leave decals |
| Camera kick | hit's `camera` (impact units) for hits the player's body lands; `feedback.takenKick` (+ the hit's `camera`) for melee hits it takes, `takenFirearmKick` per bullet | a jolt along the blow as seen on screen, biased down (`kickVerticalBias`), on a damped spring (`kickFrequency` 6 Hz, `kickDamping` 0.6: one small overshoot, settled in ~0.25 s). Mostly a camera slide (`kickOffset`), a little turn (`kickAngle`), no roll. 0 on light hits |
| Camera rumble | `player.json` `feedback` | trauma from landed heavy hits (`camera × dealtShake`), poise breaks, finishers, melee hits taken (`takenShake`), kills (`killShake`, 0 by default), cue `cameraShake`. The strongest trauma wins (no summing); amplitude = trauma², linear decay (`shakeDecay`: full trauma lasts 0.45 s); vertical-dominant (`shakeHorizontal` 0.5), 8 Hz, no roll. Bullets never rumble |

**Camera comfort rules** (`Control/CameraImpact`). A camera that blurs or disorients is a bug, not a tuning choice:

* **Frequency ≤ 12 Hz** for every wave, harmonics included. At 60 fps a 12 Hz wave has 5 frames per cycle; faster
  motion is a frame-to-frame jump the eye smears into blur / double images. The rumble is built from two sines at and
  below `shakeFrequency` per axis (the first version had components at 33 and 57 Hz - above what 60 fps can show).
* **Bounded view speed.** What disorients is the whole image swinging (vection), not how far it moves: a kick turns the
  view at most 60°/s (its start, `KickPeakSpeed`), a full rumble at most 40°/s (`ShakePeakSpeed`). Stacked kicks
  (multi-hits, bursts of bullets) are clamped to the speed of one maximal kick, every frame.
* **No roll.** A tilting horizon is the strongest vestibular conflict.
* **Translation and the vertical axis carry the impact.** A slide jolts the camera against a steady world; a turn
  swings the world itself. A vertical jolt reads as weight.
* **Nothing piles up.** Kicks are speed-clamped, rumble takes the strongest trauma, bullets only nudge - combos and
  automatic fire never build a lasting shake.
* **Rare.** Light hits never move the view; rumble is for finishers, poise breaks and heavy hits taken.
* **The player decides.** Settings → Gameplay → Camera → **Camera Shake** (0–1, default 1;
  `GameSettings::Gameplay.CameraShake`, saved in `SaveData/GameSettings.cfg` `[Gameplay]`) scales the whole response -
  kicks, rumble and shakes queued through the engine (`Camera::AddCameraShake`). 0 keeps the camera perfectly steady.
  `PlayerCamera` reads it every frame, so it applies while the slider moves.

`player.json` validation enforces it: frequencies above 12 Hz and a kick damping below 0.4 (ringing) are errors, a
kick or rumble faster than its speed limit is a warning (shipped data must load without warnings, §11).

The kick and the hit recoil share `DampedSpring` (`Combat/Spring.h`): an exact closed-form step (frame-rate
independent), kicks normalized so the response peaks at the requested size about a quarter period later.

---

## 10. Data (`Data/`, `GameData/combat/`)

```
GameData/combat/movesets/*.moveset.json
GameData/combat/characters/*.character.json   stats, mesh, locomotion clips, movement, poise, weak points,
                                              weapons, AI, possession, finisher, targeting, hit effect / recoil
GameData/combat/weapons/*.weapon.json         meshes (Bone / Rig), blade, firearm & projectile
GameData/combat/effects.json                  status effects
GameData/combat/player.json                   hero entity, camera (+ impact response), feedback, possession rules
```

### 10.1 Character file (abridged)

```jsonc
{
  "version": 1, "name": "grunt", "team": "Enemy",
  "mesh": { "model": "GameData/models/npc/base.glb", "scale": 1.0 },
  "locomotion": { "animation": "GameData/models/npc/base.glb", "idle": "idle",   // "sources": { "idle": "<file>" } per clip
                  "runForward": "run", "runBackward": "run_b", "runLeft": "run_l", "runRight": "run_r",
                  "runClipSpeed": 5.5, "legBones": ["thigh_l", "thigh_r"],
                  "strafe": { "legsBone": "pelvis", "counterBones": ["spine_01", "spine_02", "spine_03"], "maxTwist": 75 },
                  "lean": { "bone": "spine_01", "maxForward": 7, "maxSide": 9 } },
  "animationLod": { "halfRateDistance": 22, "quarterRateDistance": 45 },
  "moveset": "GameData/combat/movesets/grunt.moveset.json",
  "health": 130,
  "poise": { "max": 40, "regenDelay": 2.0, "regenRate": 15, "breakDuration": 3.0, "brokenDamageTaken": 1.5,
             "firearmScale": 0.3 },
  "movement": { "walkSpeed": 2.5, "runSpeed": 5.5, "acceleration": 30, "deceleration": 40, "airControl": 0.3,
                "gravity": 25, "turnRate": 540, "turnSharpness": 12, "radius": 0.4, "height": 1.8, "quality": "NpcGeneric" },
  "weakPoints": [ { "name": "head", "hitboxes": ["head"], "health": 30, "sources": ["Firearm"],
                    "multiplier": 2.0, "onBreak": ["BreakPoise"], "regrow": 0 } ],
  "weapons": { "Main": "GameData/combat/weapons/grunt_blade.weapon.json" },
  "ai": { "brain": "Melee", "sightRadius": 25, "fov": 140, "preferredMinRange": 1.5, "preferredMaxRange": 3.0,
          "attacks": [ { "move": "light_1", "minRange": 0, "maxRange": 2.2, "weight": 3, "cooldown": 1.5,
                         "token": "Melee" } ] },
  "possession": { "allowed": true },
  "finisher": { "range": 2.5, "angle": 60, "reward": "finisher_boost" },
  "attackTokens": { "melee": 2, "ranged": 2, "heavy": 1 },
  "targeting": { "distanceWeight": 0.5, "keepBonus": 0.2, "inputThreshold": 0.3 },   // §5.7
  "airCombat": { "riseGravityScale": 1.0, "fallGravityScale": 0.25, "maxFallSpeed": 2.0, "fallEase": 10,
                 "hold": 0.3, "rootMotionScale": 0.5, "knockbackScale": 0.5 },                  // §5.8
  "hitEffect": { "particle": "hit_flesh", "sound": "", "scale": 3 },                  // §9.4
  "hitRecoil": { "bones": [ { "bone": "spine_01", "weight": 0.3 }, { "bone": "spine_02", "weight": 0.3 },
                            { "bone": "spine_03", "weight": 0.25 }, { "bone": "neck_01", "weight": 0.15 } ],
                 "degreesPerKnockback": 3, "maxAngle": 22, "frequency": 3.5, "damping": 0.55 },   // §9.1
  "corpseLifetime": 15
}
```

### 10.2 Weapon file

```jsonc
{
  "version": 1, "name": "scythe", "kind": "Melee",
  "meshes": [ {
    "model": "GameData/models/player/weapons/scythe/scythe.glb",  // full rig + weapon bones, carries the attack clips
    "attach": "Rig",
    "hand":    { "clip": "idle", "hide": ["holser"] },   // weapon-bone rest pose in the hands; the back copy hidden
    "holster": { "hide": ["active"] },                   // on the back: the in-hand copy hidden
    "visibleWhenHolstered": true
  } ],
  "stance": { "hold": "idle", "holdBones": ["clavicle_r"], "handBones": ["hand_r"], "blend": 0.2 },   // §9.1 (pistols add "aimBones")
  "blade": { "base": { "bone": "attack_start" }, "tip": { "bone": "attack_end" }, "radius": 0.3, "samples": 5 },
  "trailParticle": "meleeTrail"
}
```

`hand.bone` / `holster.bone` are optional follow bones (§9.3); the shipped rigs parent their weapon bones to the
hands, so they are not needed. A firearm lists its muzzles and may set `holsterDelay`:

```jsonc
"holsterDelay": 1.5,
"firearm": { "fireRate": 6, "muzzles": [ { "bone": "muzzle_r" }, { "bone": "muzzle_l" } ], "projectile": { ... } }
```

A `Bone` mesh (rigid prop) instead names the body bone and its `offset` / `rotation` / `scale`
(`"hand": { "bone": "hand_r", "offset": { "x": 0, "y": 0.02, "z": 0.05 }, "rotation": { "x": 0, "y": 90, "z": 0 } }`;
vectors are `{ "x", "y", "z" }` objects, the engine's glm JSON format). Firearms add `firearm`
(`fireRate`, `magazine`, `reloadTime`, `spread`, `pellets`, `muzzles`, sounds, `projectile`).
### 10.3 `CombatData`

Thread-safe cache keyed by path (`LoadAssets` can run on worker threads). Loads through
`FileSystemEngine::ReadFile`, parses with the engine's NLOHMANN structs, validates and resolves references to
indices / bitmasks / `hashed_string`, and returns `std::shared_ptr<const T>` shared by all entities of a type.

### 10.4 Loading rule and dev reload

* Loads happen in `LoadAssets` (entity `LoadAssets`, `PreloadEntityType`, level load). Runtime code only uses
  pointers obtained there. A cache miss outside `LoadAssets` is logged as an error in dev builds.
* Dev-only (`#ifndef DISTRIBUTION`): `combat_reload` console command / debug-panel button re-parses every loaded
  file. Bodies adopt the new data at their next move boundary; invalid edits keep the old data and log errors.
* The debug panel tunes a moveset live (Apply) and saves it (Apply + Save). The save edits the hand-authored file in
  place (`Data/JsonTextPatch`, `PatchMoveSetText`): only the values edited in the panel change in the text - `_note`
  fields, key order, one-line objects, tabs and how every other number is spelled stay byte for byte. Edited floats
  are written as the shortest text that reads back as the float (0.7, not 0.699999988079071); a key the file did
  not have is inserted after its object's last member in that object's style; resized arrays and new members are
  written without default fields. The edits are taken relative to the data the panel started from (or last saved),
  so changes made to the file meanwhile survive. The save loads the patched text back and writes only if it means
  exactly the file's data plus the edits; a file that does not load is never overwritten.

---

## 11. Debugging and verification

* ImGui combat panel (dev builds): the selected body's health / poise / hit-stop / air combat, AI state, and the move
  timeline (`Debug/MoveTimeline`) of its primary and overlay moves: a ruler in clip seconds zoomed to where the move's
  windows are, one labelled lane per kind - Hits, Combo (LIGHT / HEAVY zone with the follow-up each starts), one
  Cancel lane per window (what it lets in), Armor, Invulnerable - the hold commit (yellow), recovery start (purple) and
  now (white); a "Now" line saying in words what the move does at this moment; a (?) legend; hovering lists
  everything at the time under the cursor, hovering a lane name explains the lane. Below it the running recovery and
  the combo input.
* Moveset editor (`Debug/MoveSetEditor`, same panel) edits a copy of the selected body's moveset - every field: the
  moveset's buffers, slot tables (combos, hold openers, actions, reactions) and animation files; for the selected move
  everything its JSON holds, in sections (General, Entry, Hits, Combo link, Hold, Cancels, Recovery, Root motion,
  Tracking, Armor, Invulnerable, Cues, Behaviour params with all their defaults, Other for fields added later). It
  works on the serialized form, so new data fields appear without editor code. Enums and move references are
  dropdowns, tags toggles, lists have add / remove; dimmed = default, right-click resets, every field explains itself
  on hover. Moves can be added, duplicated and deleted. The copy is validated after every edit (errors and warnings
  listed; Apply is disabled while there are errors); Apply makes it live at the bodies' next move boundary, Apply +
  Save also writes it into the file (§10.4), Revert drops the edits. Behaviour params are stored as written plus what
  differs from the behaviour's defaults (`MoveParamsToStore`) - the file never fills up with defaults.
* Debug draw: blade sweeps and hits, navigation path.
* Console: `combat_reload`, `combat_selftest`, `god`, `possess`, `release`,
  `spawn_combatant <type> [distance] [angle]` (around the controlled body, angle from the view direction, + = right).
* `-combat_selftest` launch argument (dev builds): runs the self tests, writes `SaveData/combat_selftest.log`, quits.
* `combat_selftest` (dev builds) runs pure-logic suites in the real engine: combo resolver timing (§5.4),
  cancel windows, the hit-stop curve and shake, the feedback spring (peak, frame-rate independence), attack target
  scoring, moveset / weapon / character parsing / validation / round trip (masks, tracking defaults, air data,
  Locomotion cancels, hit feedback fields, strafe / lean / LOD, hit recoil, targeting), the debug save (in-place
  edits, structure changes, edits made meanwhile, and a one-value save of every shipped moveset staying a local edit),
  damage and poise formulas — and loads every
  shipped file (`GameData/combat/characters/*.character.json` with their movesets and weapons, `effects.json`,
  `player.json`), failing on any error or warning.

---

## 12. Net-readiness (R9)

No replication now. Seams: every cross-entity change is a `HitRequestEvent` with `EntityHandle`s (network
stable); a body's action state is fully described by `(moveIndex, moveTime)` plus the recovery
`(moveIndex, recoveryTime)`; the move clock is authoritative and animation is a pure function of it.

---

## 13. Engine facts relied on

| Fact | Used for |
|---|---|
| Frame order PreUpdate ECS → `Level::Update` → PostUpdate ECS (+ event flush) → `AsyncUpdate` (parallel) → PostAsyncUpdate ECS → `LateUpdate` → PostLateUpdate ECS | §2.2 |
| Events emitted from one thread are flushed FIFO at the end of the next tick group | hit funnel ordering |
| `LoadAssets` may run on a worker thread / throwaway preload instance (`PreloadEntityTypeAsync`) | thread-safe `CombatData`, side-effect-free `LoadAssets` |
| `SkeletalMesh::SetAnimationTime / Update / PullRootMotion` (bone `"root"`), `PlayAnimation` | slaved clips, root motion |
| `AnimationPose` = bone-local matrices by name; `GetModelSpaceTransform` (missing bones = identity); `PasteAnimationPose` (missing bones = bind); a freshly loaded mesh reports its bind pose | Rig weapon posing (body bones + weapon bones), sockets |
| A `SkeletalMesh`'s pose snapshot refreshes only when the clip time changes (`dirtyPose`, protected) | `WeaponMesh::SampleClipPose` marks it dirty when sampling rest poses |
| `AnimationPose::Lerp` (root from B; above 0.995 returns B as-is, dropping bones only in A), `LayeredLerp` (root from B, traverses the given root's topology, bones missing in B count as identity, `UseWorldSpaceRotation` = blend space of the start bone) | layers, snapshots, recovery (locomotion passed as B keeps the root), Rig weapon blends (both poses carry every weapon bone); upper-body layers restore the root |
| `MathHelper` (`RotateVector`, `FindLookAtRotation`, `Transform` / `DecomposeMatrix`, `NormalizeAngle`) | facing, root motion to world, weapon attach matrices |
| `Delay` (deadline on `Time::GameTime`) | move / AI attack cooldowns, AI reaction and target-loss timers, held AI buttons, respawn, corpse removal |
| ECS systems may split their loop with `ctx.ParallelFor` (`ParallelSafe`) on the level thread pool; `EcsScheduler::Emit` is thread-safe (event types registered at startup); `EcsWorld` mutations are immediate and not thread-safe | `CombatBodyUpdateSystem`, `ProjectileUpdateSystem` |
| Jolt narrow-phase queries (`Physics::LineTrace` / `SphereTrace` / `MultiSphereTrace`) are safe from worker threads (body read locks) | sweeps, aim and AI traces in the parallel update |
| `SoundPlayer::PlayOneshot`, `GlobalParticleSystem::SpawnParticleAt`, `Level::AddEntity`, `Camera::AddCameraShake`, `Time::AddTimeScaleEffect`, `AiPerceptionSystem` lists are main-thread only | the queued side effects of §2.2 |
| `Physics::MultiSphereTrace / MultiSphereOverlap` return `entity` + `hitboxName`; hitboxes are `BodyType::HitBox` from `.skmm` | sweeps, weak points |
| `CharacterController` moves by velocity during `Physics::Simulate` | root motion → velocity |
| `REGISTER_ENTITY` token-pastes the class name | entity classes at global scope |
| `StaticMesh::MeshHideList` hides model meshes by name prefix (copied in `FinalizeFrameData`, after `AsyncUpdate`) | hand / holster weapon copies |
| The engine does not handle `info_player_start`; maps load as compiled `.bsp` | `SlasherPlayerStart` registers it and spawns the controller + hero |
| A glTF model's animated non-joint nodes (`root`) also take skin-palette slots | rig size budget below |

### 13.1 Engine changes

| Change | Why |
|---|---|
| `MAX_SKINNED_BONES` 128 → 256 (`skinned_model.hpp`) | the player rigs (cloth bones) use 135–138 palette slots; skinning writes past a 128-slot palette (heap corruption) |
| The palette reaches shaders as `finalBonesMatrices0` (slots 0–127) and `finalBonesMatrices1` (128–255); `SKINNED_BONES_PER_UNIFORM`, `Shader::SetUniform(name, const mat4*, count)`, `SkeletalMeshDrawCommand::ApplyAdditionalShaderParams` (still sets the legacy `finalBonesMatrices` for shaders of other games) | bgfx shader binaries store a uniform array's size in 8 bits (256 wraps to 0) and its per-draw opcode in 10 bits of registers |
| Game shaders: `GameData/shaders/source/bones.sh` (`GetBoneMatrix`) used by `vs_default`, `vs_shadowvolcap`, `vs_shadowvoledge`; compiled with `GameData/shaders/ShaderCompilationAutomation.exe` | the split palette |
| `AnimationPose::LayeredLerp`: `UseWorldSpaceRotation` is no longer scaled by `progress` (it only chooses the blend space) | partial mesh-space blends mixed in the local-space result: a fading upper-body layer swung the torso toward the legs' pelvis twist (aim drifting after firing). Callers passing fractional values (ShooterGame) now get exactly the space they ask for |

---

## 14. Recipes

* **New enemy**: `characters/<name>.character.json` + `movesets/<name>.moveset.json` + a three-line
  `CombatCharacter` subclass with `REGISTER_ENTITY` (or a `combat_character` map entity with `def`).
* **New move**: add it to a moveset; reference it from a combo link, `actions`, `reactions` or AI `attacks`.
* **New mechanic**: subclass `MoveBehavior`, parse its params, `REGISTER_MOVE_BEHAVIOR`.
* **New melee weapon**: export the rig + weapon meshes / bones with the attack, hold and holster clips as one
  model; a `Rig` weapon file pointing at it; list the model in the moveset's `animations`.
* **New AI**: implement `AIBrain`, `REGISTER_AI_BRAIN`, set `ai.brain`.
* **New combat data on entities**: a struct + `REGISTER_COMPONENT`, `ECS_COMPONENT` on the entity class; a
  `TICK_COMPONENT` system if it needs per-frame data updates.
* **New status effect**: add it to `effects.json`; grant it from a finisher reward or code.

---

## 15. File map

```
SlasherGame/
  COMBAT_DESIGN.md
  GameStart.cpp                boot, input actions, pause, startup map
  Combat/                      CombatTypes, CombatComponents (+ systems), CombatEvents, CombatRules
                               (hit funnel, target scoring), CombatFeedback, CombatFrame (parallel update
                               rules), Spring (damped spring for recoil / camera kick)
  Data/                        MoveSetData, CharacterData, WeaponData, StatusEffectData, PlayerConfigData,
                               CombatData (cache), DataLoadReport, JsonEnum, JsonTextPatch (in-place edits of
                               authored JSON for the debug save)
  Actions/                     MoveRunner, ComboResolver, MoveBehavior (+ registry), Behaviors/* (Attack, Dash,
                               Jump, Glide, Fire, Draw, Reaction, Death, Finisher, Charge)
  Characters/                  CombatCharacter (+ movement), CombatBodySystems (parallel body frame),
                               CharacterAnimator, WeaponSet, WeaponMesh, CharacterInput, Characters.cpp
                               (registered hero / enemy types)
  Control/                     Controller, PlayerController, PlayerCamera, CameraImpact (kick / rumble within
                               comfort limits), PlayerStart
  AI/                          AIBrain (+ registry), AIController, brains
  Weapons/                     Projectile (+ ProjectileUpdateSystem)
  Effects/                     particle systems (blood, melee trail, bullet trail)
  UI/                          CombatHud, Pause/*, LoadingScreen/*
  Settings/                    video / audio / input / gameplay (camera shake) settings + rebinding
  Debug/                       CombatDebug (panel, console commands), MoveTimeline (the panel's move timeline),
                               MoveSetEditor (every field of a moveset), SelfTests
  GameData/combat/             movesets, characters, weapons, effects.json, player.json
  GameData/shaders/source/     bones.sh (split bone palette, §13.1)
  SlasherGame.vcxproj          Visual Studio project (REngine.sln); CMake globs the sources itself
```

---

## 16. Decisions log

| Decision | Why |
|---|---|
| Engine building blocks only: `Entity` logic, ECS components for shared combat data, systems/events for cross-entity rules, `EntityHandle` references | R2; no second framework to learn or maintain |
| One body class + one move runner for player, enemies, allies, possessed bodies | Possession and allies require it; every feature written once |
| Everything is a move; behaviours in C++, numbers in JSON | Feel tuning without recompiling; editor-ready |
| Time-windowed cancels per move | Slasher feel depends on *when* a cancel is allowed |
| `HitRequestEvent` with a single handler as the damage funnel | Order independence, trades, one place for authority checks |
| Move clock authoritative, animation slaved | Windows always match visuals; hit-stop; net-ready |
| Blade sweeps against hitboxes on last frame's poses | Accurate, deterministic, no worker-thread physics races |
| Heavy starts at the alternative point when held - on release where the launcher could still take the hold | "Later = heavy" whether the press is delayed or held (bursts across the alternative point are how the player reaches heavies); waiting for the release only where the 0.6 s launcher hold is possible keeps a long hold from firing a heavy and then the launcher. Deciding light / heavy by the press time alone (tried) made held bursts light and the heavies unreachable |
| Soft targeting (stick-direction scoring all around the body + magnetism), no hard lock-on yet | Fits "melee faces input"; lock-on can be added to PlayerController later |
| Per-entity hit-stop via local time dilation | Global time scale would freeze unrelated fights |
| Hero driven by an idle brain while the player possesses another body | Never a frozen dummy; enables mimic fights |
| Recovery ends the move and continues the clip on the upper body | Instant control after a finished combo without cutting the weapon's return to idle |
| Melee weapons are Rig models that carry the attack clips; weapon bones posed from the driving clip, kept relative to a follow bone | Weapon motion authored with the attack; stays in the hand through every body blend |
| Weapon stance (idle override + hold layer) lives in the weapon file and only layers over locomotion | How a weapon is carried belongs to the weapon; move clips already pose the arms (dash, jump hold the scythe themselves) |
| Shot recoil clips are move `variants` chosen per muzzle, not a weapon field | Animation stays in the moveset (editor); the weapon only defines geometry |
| Weapons modelled into a character are weapon files without meshes | Existing enemy models keep their axes / guns; same hit and shot code |
| Body frames run in an ECS system with `ParallelFor`; side effects queued and flushed in body order | Hundreds of enemies (R15) without giving up deterministic hit resolution; entity classes keep owning their logic |
| Animation rate by camera distance, sampled at the move clock | The pose is the cost per body; a late evaluation is still exact |
| Target magnet adds to root motion instead of replacing it | Clips keep their authored motion; stationary attacks still close in; one rule keeps bodies out of each other |
| Hit-stop stops instantly at the contact and only eases the release | An eased onset let the blade slide through and read soft; lag came from long stops, so lights stay at 3-4 frames (DMC3) |
| Weight from strikes, not from locomotion | Halving acceleration / turn rate to add "weight" made the body feel uncontrollable; DMC-style crisp movement, impact from hit-stop, recoil, pushback, blood and camera |
| Melee target chosen all around the body by angle to the stick (facing without input) + distance | The player points at the body to hit; a cone dropped bodies beside or behind the hero |
| Launcher on Attack held (`holdOpeners`, `hold`): from rest after 0.1 s with a cancellable wind-up, during any move after 0.6 s committed; tap = opener on release | One attack button; the hold is the one input that cannot be pressed by accident while mashing. From rest the wind-up hand-over keeps the opener responsive; inside a move a mashed press must never launch, so the delay is long and the launch commits at once |
| Movement input ends attacks from their `Locomotion` cancel window (after the light zone) | Control returns ~0.25 s after a strike instead of at the end of the combo window; mashing with the stick held still steers; delay combos stand still (DMC rule) |
| Short light zones (0.08 clip s): the heavy zone starts soon after the window opens | Lights are buffered into the window start, so a short zone loses none; heavy presses arrive slightly early and a long light zone (0.15 - 0.2 s) turned them into lights |
| Separate `actionBuffer` for dash / jump / fire | A long buffer fired dashes the player no longer wanted |
| Hit effects belong to the victim (`hitEffect`), hits only scale / override them | Blood vs sparks vs sand is what the body is made of |
| Camera impact on a spring + trauma shake, light hits 0 | Directional jolts on heavy strikes; a shake on every swing numbs it (DMC3 keeps light hits still) |
| Debug save patches the authored text instead of re-serializing | A full dump (every default, no notes, 0.699999988079071) turned a one-value tweak into a 78 KB rewrite of hero.moveset.json |
| Camera impact within hard comfort limits (≤ 12 Hz, bounded view speed, no roll, slide over turn, strongest wins) | The first tuning (18 Hz noise with 33 / 57 Hz harmonics, 2.5° rumble, roll, summed trauma) blurred the image and disoriented; comfort is enforced by code and validation, not left to tuning |
| Air attacks reuse ground clips with `NoLegs` (hips follow the clip, legs mostly air pose) | No separate air clips needed; an upper-body layer left the hips static |
| The ground moves are the air moves; the air look and physics come from the body (`airLegs`, `airCombat`) | Separate air moves duplicated tuning and could drift apart (an air opener without the ground opener's wind-up snapped out faster); combos now flow through take-off and landing |
| Air combat keeps the rising arc and only hangs the fall | Stopping the rise when an attack starts killed jumps and launches; hanging at the top of the arc reads as natural DMC air combat |
| The grip of the weapon's own clips overrides the hands in local space | Blended arms never loosen the hand around the weapon |
| Procedural strafe from the forward run when no directional clips exist | Strafing while aiming without new assets; real clips replace it automatically |
| Camera framing offset in camera space around a focus point on the body | A world-up pivot offset collapses into depth when looking down (the body covers the crosshair); a camera-space offset keeps the composition at every pitch |

### Open items (assets / design input)

* Hero: hit reaction, death and finisher clips (reactions are logic-only, the finisher reuses `combo1_heavy_2`),
  a double-jump flip, strafe / backward runs (procedural strafe until then), a hitbox `.skmm` for player_coat.
* Dedicated air clips if the reused ground clips are not enough; a launcher whose clip leaps (the hero rises on the
  standing clip, its legs handed to the air pose by the recovery).
* Combat sounds (hit, swing, sheathe events) - cue / hit fields and the victims' `hitEffect.sound` exist. Sound is
  the biggest missing part of impact.
* A slash-spark / hit-flash particle (DMC3 style) for the contact frame - needs a texture; `hitEffect` / `hitParticle`
  take it once it exists.
* Enemies use the ShooterGame models (humanAxe, humanGun, zombie, npc base) until dedicated assets exist.
* Final tuning numbers (all in JSON).
* Gameplay trigger for possession (API + debug command exist).
* Ammo economy (magazine + reload supported; `magazine: 0` = infinite).
