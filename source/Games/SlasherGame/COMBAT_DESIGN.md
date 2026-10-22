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
| R16 | Full air combat (DMC-like): the ground moves are the air moves (legs in the air pose); landing hits in the air earns a hover (small gravity) and settles the victim for the next strike - air juggles, with limits | §5.8 |

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
| `Entity` subclass (`Update` / `AsyncUpdate` / `LateUpdate`, `LoadAssets`, `Start`, `Destroy`) | `CombatCharacter` (every fighting body), `PlayerController`, `Projectile`, player start. All per-entity logic lives here, in plain readable methods; the body systems (§2.2) call the bodies' frame methods |
| Member objects owned by the entity (like the old `Player` owned `CharacterController`, `SkeletalMesh`, `PlayerBodyAnimator`) | `CharacterController movement`, `SkeletalMesh* mesh`, `CharacterAnimator` (engine `Animator` subclass), `MoveRunner`, `WeaponSet` |
| ECS components (`REGISTER_COMPONENT` + `ECS_COMPONENT` on the entity class) | Combat data other entities and rules need to read/write on *any* entity: team, poise, weak points, status effects, combat state, attack tokens. Accessed as `character->GetPoise()` or `entity->GetComponent<PoiseComponent>()` |
| `Entity::Health` / `Entity::MaxHealth` | Health (not duplicated in a component) |
| ECS systems (`TICK_COMPONENT`, `DECLARE_SYSTEM`) | Pure data ticking: poise regeneration, status effect expiry, weak point regrow, hit-stop. Parallel scheduling: `CombatBodyUpdateSystem` runs every body's combat frame, `CombatBodyPoseSystem` every body's pose (animation, movement, hitboxes) and `ProjectileUpdateSystem` every projectile's sweep across the thread pool (`ctx.ParallelFor`) |
| ECS events (`EcsScheduler::Emit` + `EVENT_HANDLER`) | `HitRequestEvent` (the single damage funnel, one handler) and feedback notifications (`HitLandedEvent`, `DeathEvent`, …) |
| `EntityHandle` | Every reference kept across frames: targets, attackers, token holders, possession links |
| `AiPerceptionSystem` (`Observer`) | AI hearing (`EmitSoundAt`); sight is the AI controller's own (§8) |
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
Level::AsyncUpdate            (bodies have no AsyncUpdate work)
PostAsyncUpdate ECS  (worker) CombatBodyPoseSystem, every started body - CombatCharacter::UpdatePose: animator
                              (clips slaved to the move clock, pose LOD), root motion, CharacterController::Update,
                              pose paste, hitboxes, weapon posing. After the hits resolved: a hit-stop that began
                              this frame already holds the pose
Level::LateUpdate    (main)   PlayerController: camera, HUD; weapon trail entities
```

The ECS scheduler runs `ctx.ParallelFor` on the level's async-update pool (`EngineMain::Init` sets it with
`LevelThreadPoolAdapter`, §13.1). The body systems hand the bodies out one by one to one job per pool thread
(`ForEachBodyParallel`): a body's cost varies a lot (the controlled body and near bodies evaluate their pose every
frame), and the pool's `ParallelFor` cuts a range into equal fixed chunks.

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
| AI sight | reads this frame's bodies (`CombatFrame::GetBodies`) and traces the world; writes only its own controller |
| AI observers (hearing) | created in `OnPossess` (main thread), only moved during the update |

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
* Body logic (AI, moves, sweeps, aim traces) and pose evaluation / character movement run in parallel ECS systems
  (§2.2). Physics queries (Jolt narrow-phase) are safe from both.
* AI sight costs per body, not per pair: only hostile bodies are looked at, nearest first, a bounded number of traces,
  every 4th frame (§8).
* Animation composes in place (`CharacterAnimator::ComposePose`): every blend writes into poses the animator keeps
  between evaluations (`AnimationPose::LerpInPlace` / `LayeredLerpInPlace`, a completion that keeps its bone set), and
  the body reads the result by reference - a steady composition does not allocate.
* Measured with `-perf_crowd` (§11) on a 24-thread desktop, 300 bodies: 72 → 139 fps from the game-side work, then
  141 → 260–284 fps (frame 7.2 → 3.6–3.9 ms) with the engine fixes of §13.1 / §13.2. The main thread's render
  (~3.1–3.5 ms) now sets the frame.
* **Pose LOD** (`CharacterData::animationLod`, `CombatCharacter::UpdatePoseDetail`): bodies farther than
  `halfRateDistance` from the camera evaluate their pose every 2nd frame, beyond `quarterRateDistance` - or outside
  the camera's view (the last rendered frame's culling) - every 4th. Clips are sampled at the move clock, so a late
  evaluation shows exactly where the move is; root motion becomes a velocity over the evaluated span; movement,
  hitboxes and weapon placement still update every frame. The controlled body and attacking bodies (their blades
  sweep along the pose) are always full rate. Bodies at the 4th-frame rate also follow the ground with the
  controller's cheaper line traces (`NpcLowQuality`): at 300 bodies that halved the movement cost.
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
| `weapons` | `WeaponSet` member | weapon meshes (Bone props or Rig models, §9.3), hand / holster mesh swaps, reach extension, blade and muzzle points + sweeps, trails, firearm state and holster timer, projectile spawning |
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
(`Attack`, `Secondary`, `Dash`, `Jump`, `Finisher`, `Interact`, `SwitchMain`, `SwitchSecondary`) with press/release times. AI may also call
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

Weapon switching is DMC-style cycling: `switch_melee` (X, gamepad d-pad left) → `SwitchMain` cycles the Main
(melee) weapons, `switch_firearm` (C, d-pad right) → `SwitchSecondary` the firearms (§9.3). The HUD shows the
equipped weapons above the ammo.

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
                 "Glide": "glide", "Secondary": "fire", "Finisher": "finisher", "Draw": "draw", "AirDrop": "drop" },
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
`legWeight`, `variants`, `loop` / `loopStart`, `hold` (§5.4), `reach` (below), `additive` (below).

**Per-weapon moves** (`weaponMoves`, keyed by `WeaponData::name`): while a weapon is equipped (§9.3) its entries
replace the moveset's own `combos` / `holdOpeners` / `actions` - the body looks every slot up through
`MoveRunner::GetTables()` (`MoveSetData::BuildTables`). Combos and hold openers belong to the Main weapon; actions to
the slot that owns them (`ActionWeaponSlot`): `Secondary`, `AirSecondary`, `Reload` to the firearm, the rest (`Draw`,
`AirDrop`, `Finisher`, `Sheathe`) to the melee weapon. Movement actions stay in the moveset's own table. A weapon's
table cannot remove an entry, so weapon-specific moves never go in the moveset's own tables (the sword has no draw:
it must not inherit the scythe's). AI attacks name a `slot` ("Attack", "Secondary") instead of a move to use the
equipped weapon's. Character validation warns about tables of weapons the character does not have and entries the
weapon's slot does not own.

```jsonc
"weaponMoves": {
  "scythe":  { "combos": { "Attack": "start" }, "holdOpeners": { "Attack": "launch" },
               "actions": { "Finisher": "finisher", "Draw": "draw", "AirDrop": "drop" } },
  "sword":   { "combos": { "Attack": "sword_start" }, "holdOpeners": { "Attack": "sword_launch" },
               "actions": { "Finisher": "sword_finisher", "AirDrop": "sword_drop", "Sheathe": "sword_sheathe",
                           "MovingSheathe": "sword_sheathe_moving" } },
  "musket":  { "actions": { "Secondary": "musket_fire" } },
  "rifle":   { "actions": { "Secondary": "rifle_fire", "Reload": "rifle_reload" } }
}
```

**Additive layer** (`additive`, Overlay moves only): the clip's difference from its own pose at `referenceTime` is
added to the `bones` subtrees (`space` Local or Mesh, the root bone keeps the pose's) before the `bodyMask` layer
replaces its own bones with the clip. The musket's shot: `bodyMask` Bones `clavicle_l` (the arm takes the clip),
`additive` `["root"]` Local (the kick moves spine and coat a little).

Cue field `sheathe`: 1 puts the Main weapon into its sheath (the frame the blade is in), 0 draws it (§9.3).

`layerSpace` (`Default` / `Local` / `Mesh`): the space the move's partial layers blend in - its UpperBody / Bones mask
and the upper body of its recovery hand-over. Default: mesh at `locomotion.upperBodyBone` (the torso keeps the
clip's facing), local at limbs. Every sword move is `Local`: the sword's attack stance is side-on (54° off the
facing, 105° off the idle) and a mesh-space hand-over twisted the chest that far off the running hips. The musket's
shot is `Mesh` at `clavicle_l`: the arm keeps the clip's aim whatever the run does with the chest.
`layerMeshShare` (0..1, unset = by `layerSpace`) sets anything between: the share of the layers' start-bone rotation
blended in mesh space (0 Local, 1 Mesh). The sword's sheathes are 0.5 - half kept facing, half on the running hips.

`grips` (`start`, `end`, `hands`, `weapon`): windows where the clip holds its weapon with more hands than the
weapon's own (the sword's two-handed heavies and drop: the left hand on the grip); the parts those hands carry
(`WeaponMeshData::carried`, the scabbard) hide. They run on through the recovery.

`suspendsAim`: an Overlay that poses the weapon itself (the rifle's spinning reload) pauses the aim sweep; the body
still faces the aim direction (and strafes) through it.

A `Full` Overlay replaces the whole body - its root included (the reload spins on it) - and hands its legs to the
pose underneath by `legWeight` as the body runs (`airLegs`: and as it flies); standing it keeps the clip's footwork.

Reach (`ReachData`): the weapon in `weapon` (default Main) gets `length` m longer during the move — fully from
`start` to `end`, growing over `blendIn` (default 0.12 s) before and shrinking back over `blendOut` (0.2 s) after;
`start` / `end` < 0 (default) take the first / last blade hit window of that weapon. The weapon's extend bone does it
(§9.3), so the mesh, the blade sweep and the trail all reach further. The hero: light attacks (`start`, `light_1..3`)
0.2 m, heavy attacks (`heavy_1..3`, `launch`) 0.4 m. Validation: a reach needs `start` and `end` or a blade hit window to time
it by; the character load warns when the weapon of a move's reach has no extend bone.

```jsonc
"reach": { "length": 0.6 }                                      // over the blade hits
"reach": { "length": 0.8, "start": 0.3, "end": 0.6, "blendIn": 0.15, "blendOut": 0.25, "weapon": "Main" }
```

Hit window fields: `start`, `end`, `weapon` (blade of that slot) **or** `bone` + `radius` (body-bone sphere, kicks),
`direct` (paired moves: hits the move's target without a sweep), `source`, `damage`, `poiseDamage`,
`weakPointDamage` (default = damage), `reaction`, `knockback` (m/s push; the victim's reaction move decides over how
long), `launch`, `hitStop` (s), `hitStopScale` (time scale while stopped, default 0.03), `shake` (m the victim's mesh
shakes), `camera` (view kick when the player lands it, impact units; default 0 - light hits never move the view),
`impactScale` (× the victim's hit effect), `air` (what the hit does in the air - `lift`: at least this upward speed
for two airborne fighters, `stall`: seconds the victim hovers; both < 0 = the character's `airCombat` default, `stall: 0` = no hover, §5.8),
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
* The short `holdDelay` from rest is only for a hold opener whose wind-up hands back to the usual attack (a `hold`
  rule): a tap that lasts a little longer still becomes the opener. A hold opener without one - the air drop, which
  commits at once - always needs `holdDelayInMove` (`HoldOpenerDelay`, pure, self-tested): a 0.15 s tap in the air is
  the usual attack, never the drop.
* A press made in the air that is still undecided when the body lands (or the reverse) never starts the other state's
  hold opener: held, it waits; released, it is the usual attack. A hold meant for the drop does not pop the ground
  launcher on landing.
* The hold that started a hold opener is spent on it: it does not carry into that move's combo link
  (`holdCarriesOver` is for the press that started a usual attack). Holding on through the drop's landing is not a
  heavy, nor another launcher.
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
  over it - an aiming weapon's hold goes over its upper body while it aims (§9.1), so a gun without a fire clip (the
  rifle) still shows its pose while the legs blend out of the attack.
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
* `Overlay` moves start only when the primary move has `"overlay": "Allow"` (or none runs), or when a cancel window
  of the blocking primary lets their tags in - the overlay then ends it (firing cuts the standing sheathe short);
  starting a primary move with `"overlay": "Block"` interrupts the overlay. A blocking primary also suspends aiming (§9.1): the dash
  faces its direction and the aim sweep fades out; jumping and gliding allow overlays and keep aiming.
* Forced moves (hit reactions, finisher victim, death) ignore cancel windows; armor decides whether a reaction
  plays at all.

### 5.7 Behaviours

| Behaviour | Mechanics | `params` |
|---|---|---|
| `Attack` | hit windows → blade sweeps / bone spheres (run by `MoveRunner` + `WeaponSet`); **target assist**: the controller's focus, else the **attack target** (`CombatCharacter::FindAttackTarget`): every hostile within `tracking.searchRange` **all around the body** and in sight, scored by its angle from the move input direction (the facing without input) plus `targeting.distanceWeight` × distance / range, the previous attack's target favoured by `targeting.keepBonus` - pointing the stick at a body picks it, a near tie keeps the combo's target; the body turns toward it (or the input direction) until `tracking.until` - never snaps: fast for the first `startTurnTime` (`startTurnRate`), then at `turnRate`; the **magnet** adds speed toward it on top of the clip's root motion until `idealDistance`, arriving by the next strike, until `magnetUntil` (stationary clips still close in), and never lets anything carry the body closer; without a target `tracking.advance` steps forward while input is held. **Air** (§5.8): the target is one within `targeting.airHeightRange` above / below (an enemy standing far below is not chased); a melee hit landed in the air starts the attacker's hover and settles an airborne victim once (swinging at air earns nothing); before the hover the attack chases the target's height; it goes on on the ground after landing; `plunge` hops by `rise` as it starts in the air, dives - steered onto the target below by the magnet, committed (`MoveBehavior::Commits`) - and holds its strike until landing; `needsTarget`: in the air the move only starts with something to launch, an airborne hostile within its magnet range; `launchSelf`: the attacker leaps too - at clip `time`, or `onHit` when one of the move's hits actually launches its target (armor downgrading the launch, or a kill, leaves it on the ground) - optionally only while Attack is held | `locomotion`, `air` (`matchTargetHeight`, `needsTarget`), `plunge` (`start`, `speed`, `hold`, `rise`), `launchSelf` (`time`, `onHit`, `verticalSpeed`, `requireHeld`) |
| `Dash` | displacement along input (or facing), gravity suspended, i-frames from `invulnerable`, stops at walls, air charges. EaseOut slows linearly to `exitSpeed` × run speed, so the body runs out of the dash; after the displacement input steers the body (`tailLocomotion`). Jump cancels it from the first frame (data) | `distance`, `duration`, `curve`, `airCharges`, `exitSpeed`, `tailLocomotion`, `faceDirection` |
| `Jump` | vertical velocity; ground or air (air jumps refill on landing); ends on landing; horizontal speed carries into the jump up to `carrySpeed` × run speed (a dash-jump leaves at run speed) | `velocity`, `maxAirJumps`, `airControl`, `carrySpeed` |
| `Glide` | started by the move runner when the body falls with Jump held for `holdDelay` (no second press); reduced gravity, capped fall speed, limited time per airtime; ends on release; sustains at `endTime` | `maxDuration`, `gravityScale`, `maxFallSpeed`, `airControl`, `holdDelay` |
| `Fire` | overlay; fires on start and at the weapon's fire rate while held (or per press), projectiles from the next muzzle toward the aim point, strafe-to-aim facing, locomotion at `moveSpeed`; a weapon drawn by the move fires on the next frame (muzzle in the hands). With `restartOnShot` every shot restarts the clip (`MoveRunner::RestartClip`) as clip n for the weapon's n-th muzzle. Without `sustain` the move is one shot: it starts only when the weapon is ready and plays its clip once (the musket) | `weapon`, `button`, `automatic`, `sustain`, `aimTurnRate`, `restartOnShot`, `moveSpeed` |
| `Draw` | puts `weapon` in the hands when it starts (the clip begins with the grab: the other weapon is put away and this one shown at once) and plays the draw clip, usually an UpperBody overlay. The body starts the `Draw` action of its Main weapon by itself once the Secondary one's `holsterDelay` ran out (§9.3) | `weapon` |
| `Reload` | the reload clip of `weapon` on the overlay channel (the rifle's: Full - it spins the body); the weapon's own timer (`reloadTime`) refills the magazine, the move starts it and keeps the weapon in the hands; the body moves at `moveSpeed`. The body starts the `Reload` action by itself while its firearm reloads, cutting the fire move short; a trigger still held afterwards fires again | `weapon`, `moveSpeed` |
| `Sheathe` | a sheathable Main weapon goes back into its sheath; its cue's `sheathe` switches the meshes on the frame the blade is in. The body starts it by itself once a drawn sheathable weapon has nothing left to do (§9.3): `Sheathe` (standing, full body) or `MovingSheathe` (upper body); interrupted, it plays again later | - |
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
* **Hover** (`CharacterData::airCombat`; the rules are `Combat/AirJuggle.h`, engine-free and unit-tested). A body
  that juggles in the air has (almost) no gravity: falling it has `fallGravityScale` of it and sinks no faster than
  `maxFallSpeed` (a faster fall brakes to that, `fallEase`); rising it keeps the normal arc (`riseGravityScale`) -
  a jump or a launch reaches its top. Every character has its own numbers: an enemy's are near zero (0.02, 0.15 m/s),
  the hero's a small gravity (0.25, 1.5 m/s): a tight combo (a hit every 0.3 s) holds her height, a sloppier one lets
  her sink about 0.8 m in 2 s. **Hits earn the hover, not swings:**
  * **attacker** - a melee hit landed while the attacker is airborne starts its hover; the attack that landed it
    carries it (it hovers while that attack runs) and `hold` (0.5 s) after it. An air swing that hits nothing earns
    nothing and falls normally. When a launcher also launches its attacker on a successful hit (`launchSelf.onHit`),
    the attacker receives the same hit-specific `air.stall` hover duration as the victim (1.2 s for the ground
    launcher); if the hit has no explicit stall, each body uses the victim-hold default from the victim's character data.
    A time-triggered leap by itself still follows a normal jump arc and earns no hover. (The air launcher's hit also
    grants this shared stall: it is landed in the air.)
  * **victim** - the player controls a juggled enemy by landing hits: it hovers while the attack that hit it in the
    air runs and the **attacker's** `victimHold` after it (the hero's 2.5 s), so a slow wind-up always has something
    to hit. Every hit restarts the timer, nothing else ends it. A hit's `air.stall` may ask for more, never less (the
    launcher's 1.2 s gives way to the longer hold); `stall: 0` asks for none - the drops slam the victim down.
  * **bullets** - a firearm hit stops an airborne enemy's vertical motion (it keeps its height) and restarts a
    the shooter's `firearmHold` (0.2 s) hover: sustained fire holds an enemy where it is, and when the fire stops gravity fades in
    over `gravityRamp` and it falls. (The pistols' reaction is `None`; the air hit still counts. An enemy's bullets
    do not stall the player.)
  When a hover ends gravity **fades in** over `gravityRamp` (0.4 s) instead of snapping on.
* **No infinite air time - the player's.** A body's hang clock starts with its first hover of an airtime and runs
  until it lands (not while the body is on the ground; local, hit-stop dilated time). `maxHangTime` later the hover's
  gravity and sink grow back over `hangFade` (2 s) to the normal ones, however often the body is hit or attacks. Only
  the hero has a limit (`airCombat.maxHangTime`); an enemy's is 0 - none of its own: it falls when her juggle ends
  (she comes down, misses, or stops attacking for longer than her `victimHold`), never for a reason of its own.
  Landing resets the clock, the hover and the launcher's strain.
* **The settle: positioning comes from a landed hit, nothing else.** A melee hit between two airborne fighters
  moves the *victim* once (`AirJuggle::PlanPull`, `CombatCharacter::SetAirPull`): within `pullTime` (0.18 s) to the
  attacker's height plus the hit's rise, and - if it is further away than the attack's `idealDistance` but within
  `pullRange` (1 m) of it - to the ideal distance. The destination is **fixed at the hit**: the victim never follows
  the attacker afterwards, so dashing away leaves it where it was settled, and a victim out of reach is not pulled at
  all. A victim closer than the ideal distance is not pushed away; one more than `pullMaxHeight` off in height only
  rises with the hit. Launching hits, kills, grounded victims, plunges and player-controlled victims are never
  pulled. No other system moves an enemy toward the player.
* **A slight rise.** The same hit lifts both fighters to at least `hit.air.lift` (default `hitLift` 2.5 m/s, about
  0.15 m under normal rise gravity); the victim's rise is part of its settle destination.
* **The attacker's height.** Before the hover an air attack chases its target's height (the target's vertical speed
  plus a pull, `air.matchTargetHeight`): it reaches an enemy that hangs above it, rises with a launched one, comes
  down onto a grounded one. Hovering, the attacker keeps its own height - the small gravity is its own, it does not
  copy the enemy's (that is why it can fall before the enemies it juggles); the hits level the enemy with it.
* **Plunge**: an attack's air version may dive (`plunge`: a hop of `rise` m/s as it starts in the air - the weapon goes
  up - then from `start` at `speed`, the clip held at `hold` - strike pose, hit window open - until it lands,
  `MoveBehavior::HoldsClock`); landing, the clip and the move go on on the ground. A plunge move keeps its own height
  (no target height chase); diving, the magnet steers it onto the target below (within `magnetRange`), and the move is
  **committed** (`MoveBehavior::Commits`): no combo link resolves, no cancel window opens and no recovery begins until
  it lands - presses made meanwhile are judged then. Its links and cancels belong after `hold` (self-tested for the
  hero's drop). The hero's only plunge is the drop.
* **Launcher**: the hero's `launch` (clip `launch`, played at 0.7), the `Attack` hold opener (§5.4): wind-up until
  0.26 (from rest, released earlier, the usual attack `start` takes over; after a 0.6 s hold during another move it is
  committed from the start), a rising slash 0.27 - 0.37 with a `Launch` hit
  (`launch` 10.5 m/s: ~2.2 m up), 1.2 m of root motion forward, armor through the wind-up. When the hit launches its
  target the hero goes up with it (`launchSelf.onHit`, 9.6 m/s = 10.5 × √(21 / 25): the hero's gravity is 21, a
  grunt's 25, so both peak at the same height); both rise after the shared hit-stop and both receive the hit's
  `air.stall` hover (1.2 s). This lets the hero hang with the launched target before starting the air combo. Recovery
  starts at 0.4: airborne,
  it releases at once - the legs blend to the air pose while the arms finish the follow-through (the clip itself
  stands on the ground). A whiff, or a target whose armor turns the launch into a stagger, leaves the hero on the
  ground; a launcher that breaks the target's poise still launches it (it flies in its broken pose and stays
  finishable). Launches survive their hit-stop: the movement's blocked-rise check (`CombatCharacter::ResolveRise`)
  measures the physics' vertical speed in the dilation it was applied with - judged in this frame's dilation, every
  launch was cut to a hop where the hit-stop eased out.
* **Launcher in the air, and the drop.** The launcher (`startState: Any`) works in the air, **much weaker**: its launch
  speed, the hero's own leap and the victim's hang scale by `launchScale` (0.5) for the first one of an airtime,
  times `launchDecay` (0.6) for every launcher that connected before (`MovementResources::airLaunchStrain`, counted
  when a launching hit connects - a whiff or a released wind-up is free), never below `launchMinScale` (0.15): 0.50,
  0.30, 0.18, 0.15. Every melee hit of the combo landed between two airborne fighters heals `launchRecover` (0.25) of
  the strain, so combos between launchers keep it strong (3 hits between: 0.50, 0.44, 0.39...). In the air it needs
  something to launch (`air.needsTarget`): an **airborne** hostile within its `magnetRange` and
  `targeting.airHeightRange` (the controller's focus must qualify too). An enemy standing below is nothing to launch:
  **without one, a fresh hold falls back to the drop attack** (`actions.AirDrop`, the hero's `drop`). This fallback
  applies only when `StartOpener` selects the hold opener. While a combo is resolving, AirDrop is not a combo branch:
  without a launch target, the usual light/heavy follow-up rules still apply.
  Because a hold now means something in the air, the first attack of an airtime waits for its release or the hold
  delay, like on the ground: `holdDelay` (0.1 s) when the launcher can start (its wind-up hands back to `start`),
  `holdDelayInMove` (0.4 s) for the drop, which commits at once - only a deliberate hold drops, a tap never does.
* **The drop** (`drop`, the `combo1_heavy_2` clip at 1.45): hops 4 m/s as the weapon goes up, dives at 18 m/s from
  0.24 steering onto the target below (the drop's target search ignores `airHeightRange`), holds the strike (hit
  window open) from 0.66 until it lands; its hit `air.stall: 0` slams an airborne victim down. Landed, it goes on like
  `heavy_2`: its combo link (`light_3` / `heavy_3` from the hold pose, 0.75 - 1.28) and cancels (0.79), all after
  the hold. The hold that started it is spent - holding on through the landing starts nothing.

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
   blow; its upper body recoils (§9.1). A victim in the air gets its hover and lift (§5.8). Knockback, `HitLandedEvent`
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
  + radius. `UpdatePose` stores the socket transform after the pose is final; the next combat frame sphere-sweeps
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
* Sight (`AIController::UpdateSight`): the hostile bodies of this frame (`CombatFrame::GetBodies`) inside
  `ai.sightRadius` and the `ai.fov` cone - the nearest 6 plus the current target - each through the BSP cluster
  visibility and one trace against the opaque world; every 4th frame per body (spread over the bodies), the result
  holding in between. Other bodies do not block sight (a crowd behind its front row still sees the hero). Hearing
  stays with the engine `AiPerceptionSystem`: AI bodies own an `Observer` for `EmitSoundAt` (its own sight is off,
  and bodies register no `ObservationTarget`): the engine observer traced every target against every observer
  (at 300 bodies 37 ms of CPU per frame, 3.8 ms of the game thread).
* BT-ready: a `BehaviorTreeBrain : AIBrain` can map the context to a blackboard later; nothing else changes.

---

## 9. Animation and feedback

### 9.1 Body composition (`CharacterAnimator : Animator`)

Evaluated in the body's `UpdatePose` (`CombatBodyPoseSystem`) from an `AnimatorInput` built from the move runner's
state, in this order:

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
   right arm, the pistols' aim on the upper body): how the weapon is carried during locomotion. An aiming weapon
   holds the middle frame of its sweep (step 7); `idleTime` / `holdTime` hold one frame of a clip instead of looping
   it. `holdSpace` chooses the space the hold's start bones blend in (default: mesh at the upper-body bone, local at
   limbs; the musket's arm is Mesh). Move clips pose the arms themselves, so the hold stays under every move - except
   Movement moves (jump, glide, dash: their clips carry no weapon), over which the stances with `moveHoldBones` (the
   firearms: their hands) lay the hold again on the hands only, in local space: a gun stays in the hands while the
   body - arms included - dashes, jumps or glides the way its clip does. While a weapon aims, its whole hold goes
   back over a move's clip (a jump or glide allows aiming, §5.6) or an attack playing out in recovery, faded in with
   the aim, unless the clip is that weapon's own: the sweep and yaw (step 7) aim the hold pose, so the weapon points at
   the aim point in the air and while the legs blend out of an attack (the rifle has no fire clip to cover it). A firearm that leaves the melee weapon in the hands (the
   musket in the left hand, §9.3) is a **layered** stance: its hold goes on top of the others with its own weight
   instead of cross-fading with them. A sheathable weapon has two stances: `stance` while drawn and `sheathedStance`
   while sheathed (the sword's: its idle and the plain locomotion run), cross-fading when it changes state.
3. **Full-body primary** — the move clip sampled at the move clock replaces 1–2. `NoLegs` moves, and `Full` moves
   in the air (`airLegs`, faded by the air weight with take-off and landing), then blend the `locomotion.legBones`
   subtrees (thighs down) toward the locomotion / air pose by the move's `legWeight`, local to the clip's hips: the
   hips swing with the attack, the legs mostly keep the air pose. The clip keeps its `root` through that hand-over.
4. **Recovery** (§5.5) — the ended attack's clip on the same `Animation` instance. Full-body weight fades
   1 → 0 over `lowerBodyBlend` (legs to locomotion), then the upper body (`LayeredLerp` from
   `locomotion.upperBodyBone`) holds it until the clip ends and fades over `blendOut`.
5. **Upper-body / bone-masked primary**, then the **overlay** (firing, drawing) faded in / out by its `blendIn` /
   `blendOut`; the overlay is on top because it only runs with primaries that allow it. An overlay with an `additive`
   layer (§5.2) first adds its clip's difference from the clip's reference frame to the additive bones (the musket's
   kick through spine and coat), then lays its mask. `UpperBody` layers from
   `locomotion.upperBodyBone` (mesh space); `Bones` only on the move's `maskBones` subtrees (local space) - the draw
   shows on the right arm alone, like the scythe's stance hold. The overlay keeps its mask until it has faded out
   (after the move ended the input has no overlay; fading on the default mask moved the whole upper body - the spine
   twitch at the end of the draw). An overlay replacing a visible one (a draw after firing) cross-fades over the new
   clip's `blendIn`: the outgoing one (last pose, difference, weight, shape) and the incoming one are layered over the
   same pose and the two results blend, so bones only one of them covers fade instead of snapping. When another
   overlay clip takes over a visible one (each shot restarting its recoil clip), it blends from the previous pose.
   A `Full` overlay owns the whole body, its `root` included - the rifle reload's spin plays standing, running and in
   the air (handing its legs over used to hand the root over too: the engine's partial blends take it from their layer).
6. **Grip** — the weapon's `stance.handBones` (the hands holding it) take the grip of the clip posing the weapon in
   local space: its overlay clip, else its move / recovery clip (clips from the weapon's own model), else its hold
   clip, weighted by how much each shows. Blends and mesh-space layers move the arms, never the grip, so the weapon
   stays tight in the hand. Layered stances grip their own hands on top.
7. **Aim sweep** — a weapon whose stance has a `sweep` (every firearm of the hero) is an aiming weapon: while it is
   in the hands the body faces the aim direction (strafes, turning at `aimTurnRate`) and the sweep aims the pose at
   the aim point (the controller's: crosshair for the player, target for AI). The sweep clip aims the weapon from
   `startPitch` on its first frame to `endPitch` on its last, level on its middle frame - the hold pose (step 2);
   each half maps pitch to time linearly. The clip at the aim pitch, as its difference from the middle frame
   (`AnimationPose::MakeAdditive`), is added on the `sweep.bones` subtrees (spine_01: the upper body and the coat) in
   mesh space - after the weapon's own clips (recoil, reload, the musket's shot) and the grip, weighted by the stance
   and the aim fade. It replaced a procedural spine rotation that turned the coat with the spine: the sweep turns
   spine, arms and coat the way the clip does, and bones the clip lacks (the pistols' model has no coat) keep their
   mesh-space rotation. Then the **yaw**: the body turns toward the aim at `aimTurnRate`, and until it gets there
   `sweep.yawBones` (spine_01..03, an even share each, after the pitch) turn the rest about up, at most `sweep.maxYaw`
   (60°) - the weapon is on the aim at once, as with the procedural aim the sweep replaced. Shots leave from the
   muzzles toward the same aim point. Aiming applies only while no primary move blocks overlays (§5.6); the aim fades
   in / out over 0.15 s.
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
* **Extension** (`extend`): moves with a `reach` (§5.2) move the weapon's `extend.bone` out along `extend.axis` (its
  own space) by the reach's length — on the drive target, so a move taking over blends the length to its own over
  the transition above, like the clip's weapon bones. Meters are converted with the bone's bind-pose axis in model
  space and the body's transform. The part of the mesh skinned to the bone and the points on it go along: the scythe
  model skins its head to `attack_end` and the bottom of the handle to `attack_start`, so moving `attack_end` along
  its Y lengthens the shaft, the blade sweep and the trail. Only a weapon bone of a Rig model can extend; in the
  hands only.
* **Hand / holster copies**: `hand.hide` / `holster.hide` list meshes of the model hidden in that state
  (`StaticMesh::MeshHideList`, name prefix) — the scythe model carries an in-hand (`active`) and a back (`holser`)
  copy. A weapon without a holster copy uses `visibleWhenHolstered: false`.
* **Built-in weapons**: a weapon file without meshes is part of the character model (an enemy's modelled axe or
  gun); its points use the body's bones.
* **Rig models without weapon bones**: a model skinned to body bones only (the musket and rifle to a hand, the sword
  and its scabbard to both) is drawn with the body pose as it is; its points are a body bone of the model (`hand_l`)
  plus an offset in that bone's space (model units).
* **Weapon lists and switching**: a character lists several weapons per slot (`weapons.Main` / `weapons.Secondary`
  arrays, the first equipped); `SwitchMain` / `SwitchSecondary` presses cycle them (`CombatCharacter::EquipWeapon`).
  The outgoing weapon's moves stop with it (its overlay actions; a Main weapon's recovery), the move tables follow the
  new one (§5.2). A Main switch waits while a melee attack runs (its combo belongs to the old weapon). A firearm that
  was out is replaced in the hands by the new one (unless that one only comes out to fire). Weapons that are not
  equipped show their holster state.
* **Hands and presentation**: each weapon holds `hands` (the scythe and sword: Right; the musket: Left; pistols and
  rifle: both). The equipped Main weapon is in the hands unless the Secondary weapon is out and takes every hand any
  of its in-hand states needs: then it is holstered and the firearm's stance replaces its stance. A firearm leaving
  one of them free is layered beside it (§9.1): the scythe stays in the right hand while the musket fires from the
  left.
* **Sheath** (`sheathedHands`, mesh attachment `sheathed`, `sheathedStance`): a sheathable melee weapon (the sword,
  held sheathed in the left hand) has a third state. It rests sheathed (the sword idle; running is the plain run); an
  attack of the Main weapon draws it as it starts (the mesh switches to the drawn blade at once, the attack's blend
  covers the turn into the side-on attack stance). Once it has nothing left to do - no primary move, no overlay - the
  body sheathes it: standing, right after an attack played out to its ready stance (its recovery ended unreleased),
  the `Sheathe` action - the full-body clip steps from the attack stance into the idle and ends on it; while moving,
  in the air, or when something else drew it, `MovingSheathe` - the clip on the upper body in local space over the
  legs. A standing sheathe hands over like a recovery when the stick is pushed (legs to the run, the upper body keeps
  the clip to its end) and is cut by an attack, dash, jump or shot. The cue `sheathe: 1` puts the blade in on the
  frame it is seated; cut before, the weapon stays drawn and sheathes again later. A firearm coming out moves the
  weapon to the state whose hands it leaves free (`WeaponSet::FitMainToHands`): a left-hand firearm draws it, a
  right-hand one sheathes it, one taking both hides it (it comes back sheathed).
* **Carried parts** (`carried`: a hand and mesh prefixes): parts of the model held by a hand the weapon does not hold
  (the sword's scabbard, `active_optional`, in the left hand) hide while that hand holds something else - the other
  slot's weapon, or this weapon's grip in a two-handed move window (`MoveData::grips`).
* **Holster timer**: a Secondary weapon with `holsterDelay` > 0 goes back once that many seconds passed since it came
  out or last fired, when nothing on the overlay channel runs — beside the Main weapon with the fade below, otherwise
  the Main weapon's `Draw` action plays (instant swap without one). A firearm with `holsterAfterFire` (the musket, no
  idle in the hands) goes away once no fire move ran for its `holsterDelay` (0.35 s: it stays aimed after the
  shot). Put away beside the Main weapon, its mesh stays in the hand while its stance fades over `stance.blend` - the
  arm lowers it, then it goes. Attacks swap to their hit windows' weapon instantly.

### 9.4 Feedback

Feedback handlers on `HitLandedEvent` / `PoiseBrokenEvent` / `FinisherEvent` (`CombatFeedback.cpp`); move `cues`
drive move-specific sounds / trails / particles; the finisher's slow motion is `Time::AddTimeScaleEffect`. Rules
never depend on feedback. The style reference is DMC3: semi-realistic, light hits sell themselves through the stop,
the victim and blood - no flashes, no camera on every swing.

Weapon trail (`Effects/MeleeTrail.h`, the engine `TrailEmitter`): `WeaponSet::PlaceTrail` puts the system on the outer
part of the blade every frame (`SetTrailTransform` from `trailStart` along the blade to its tip), and every sample of
the ribbon is that span - centred on it, turned along it, as long as it (so it follows the reach, §9.3). The newest
sample follows the blade, older ones fade over 0.18 s. The look is the texture's (`textures/particles/melee_trail.png`,
unlit, alpha-blended at 0.6): u runs from the oldest sample to the blade, v from the tip edge to the base - a faint
pale-red edge at the tip over a dark crimson body that fades toward the base and the old end.

Bullet tracer (`Effects/BulletTrail.cpp`, "bullet_trail", the projectile's `tracer`): a camera-facing ribbon from where
the shot was 0.12 s ago to where it is, 8 cm wide at the shot narrowing to 3 cm, warm orange and additive - light
against the dark levels, fading out over its life once the shot ends.

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

### 9.5 Coat cloth

The coat is bone cloth (engine `Cloth/BoneCloth.h`), spring bones the way early-2000s action games did coats: six
panels (front, side and back, left and right), each one chain from the chest to the hem - the upper panel bone under
`cloth_root2` continues into the three skirt bones under `cloth_root` it ends at. Each bone's tip trails a point - where the bone would hang on the bone above as
that one hangs now - easing toward it each frame, quick at the chest and slow at the hem, so the panel bends smoothly
with no hard point. The coat trails a share of the character's movement (`trail`) and all of its turning; no speed is
kept, so starts and stops never jitter or overshoot. The coat's keys in the locomotion clips (and on its roots) are
not used: the panels hang from the body in their rest shape. Settings: `GameData/models/player/player_coat.glb.skmm`
(`cloth`), re-read in game with `cloth_reload`; `cloth_debug` draws the chains and capsules.

* **Order.** `CombatCharacter::UpdatePose` composes the pose, moves the body, sets the mesh transform and only then
  pastes (`PasteAnimationPoseWithCloth`) - the coat lags this frame's movement. A body with cloth pastes every frame
  (the last composed pose between evaluations); hitstop's local delta time 0 holds the coat; `Teleport` resets it.
* **Legs.** Capsules on thighs and calves, fitted to the pants mesh; the bone tips are pushed out of them.
* **Wind.** `wind <x> <y> <z> [gust] [Hz]` sets the world wind; it pushes the tips. Two additive coat clips of
  `player_coat.glb` turn the rest shape the panels swing back to: `coat_open` (front panels swing out, ~24 cm at the
  hem; headwind, windward side more) and `coat_volume` (every panel flares ~11 cm at the hem; `volume` always, more
  with wind speed). Both are two-frame actions in `player_coat.blend` (rest, then `coat_open_ctrl` Y 10 /
  `coat_wind_ctrl` Y 25 with `coat_open_ctrl` Y -6 cancelling its opening); until the model is re-exported with them
  the layers are off (a warning at load).
* **Spin.** A fast turn - a quick turn, a spinning attack - opens the coat as if the air filled it: the yaw rate of the
  fastest of the bones the coat roots hang from and their ancestors (root, pelvis, spine_01-03), each turned by the
  entity and the animation together - so attacks that whip the pelvis or the spine open it too, even when the spine
  counters the pelvis - adds `spinOpen` / `spinVolume` to the two layers, from `spinStart` to `spinFull` deg/s. It
  rises with `layerSmoothing` and settles over `spinRelease`, so the coat stays open a beat after the turn. Both
  weights 0 turn it off.

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
  "weapons": { "Main": ["GameData/combat/weapons/grunt_blade.weapon.json"] },      // per slot, the first equipped (§9.3)
  "ai": { "brain": "Melee", "sightRadius": 25, "fov": 140, "preferredMinRange": 1.5, "preferredMaxRange": 3.0,
          "attacks": [ { "move": "light_1", "minRange": 0, "maxRange": 2.2, "weight": 3, "cooldown": 1.5,
                         "token": "Melee" } ] },      // or "slot": "Attack" - the equipped weapon's move (§5.2)
  "possession": { "allowed": true },
  "finisher": { "range": 2.5, "angle": 60, "reward": "finisher_boost" },
  "attackTokens": { "melee": 2, "ranged": 2, "heavy": 1 },
  "targeting": { "distanceWeight": 0.5, "keepBonus": 0.2, "inputThreshold": 0.3, "airHeightRange": 2.0 },   // §5.7, §5.8
  "airCombat": { "riseGravityScale": 1.0, "fallGravityScale": 0.02, "maxFallSpeed": 0.15, "fallEase": 20,
                 "hold": 0.5, "victimHold": 1.8, "firearmHold": 0.2, "gravityRamp": 0.4,
                 "maxHangTime": 0, "hangFade": 2.0, "hitLift": 2.5, "rootMotionScale": 0.5, "knockbackScale": 0.5,
                 "pullTime": 0.18, "pullRange": 1.0, "pullMaxHeight": 1.0, "pullMaxSpeed": 12,
                 "launchScale": 0.5, "launchDecay": 0.6, "launchRecover": 0.25, "launchMinScale": 0.15 },   // §5.8 (defaults = an enemy)
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
  "hands": ["Right"],                                    // §9.3; a sheathable weapon adds "sheathedHands" + mesh "sheathed"
  "stance": { "idle": "idle", "hold": "idle", "holdBones": ["clavicle_r"], "handBones": ["hand_r"], "blend": 0.2 },   // §9.1
  "blade": { "base": { "bone": "attack_start" }, "tip": { "bone": "attack_end" }, "radius": 0.3, "samples": 5 },
  "extend": { "bone": "attack_end", "axis": { "x": 0, "y": 1, "z": 0 } },   // §9.3: moves' reach lengthens the shaft
  "trailParticle": "meleeTrail",
  "trailStart": 0.2                                     // the trail spans the blade from 20 % to the tip (§9.4)
}
```

`hand.bone` / `holster.bone` are optional follow bones (§9.3); the shipped rigs parent their weapon bones to the
hands, so they are not needed. A firearm lists its muzzles, its hands and its aim sweep, and may set `holsterDelay`
(or `holsterAfterFire`):

```jsonc
"hands": ["Right", "Left"],
"stance": { "holdBones": ["spine_01"], "handBones": ["hand_r", "hand_l"],
            "sweep": { "clip": "sweep", "bones": ["spine_01"], "space": "Mesh", "startPitch": -90, "endPitch": 90 } },
"holsterDelay": 1.5,
"firearm": { "fireRate": 6, "muzzles": [ { "bone": "muzzle_r" }, { "bone": "muzzle_l" } ], "projectile": { ... } }
```

Shipped hero weapons: `scythe`, `sword` (Main); `pistols`, `musket` (left hand, one shot, `holsterAfterFire`),
`rifle` (automatic, 30 rounds, reload clip; no fire clip yet - its fire move is clip-less) (Secondary).

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
  `spawn_combatant <type> [distance] [angle]` (around the controlled body, angle from the view direction, + = right),
  `perf_crowd` (below).
* `-combat_selftest` launch argument (dev builds): runs the self tests, writes `SaveData/combat_selftest.log`, quits.
* Profiling (dev builds): `perf_crowd <count> [spacing m] [types] [ally share]` spawns rings of bodies around the
  controlled body (default mix grunt, grunt, gunner, brute); an ally share recruits that part of them like a released
  possessed body, so the crowd fights. `-perf_crowd <same arguments>` runs it once the startup map is in, with god mode
  on the controlled body: a repeatable load. Tracy zones mark the combat frame (`Combat: ...`), the body parts
  (`Body: controller / moves / weapons / facing + aim / animation / animator evaluate / paste pose / movement /
  hitboxes / weapon pose`), `AI: perception / think / path`, `Combat: resolve hit`, and a `Combat bodies` plot; a
  build with `TRACY_ENABLE` + `TRACY_ON_DEMAND` (the VS project's configurations define them) is captured with
  `Tools/tracy/tracy-capture.exe` and read with `tracy-csvexport.exe` or the profiler.
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
| `EcsScheduler` runs `ctx.ParallelFor` on the pool given to `SetThreadPool` (`EngineMain::Init`: `LevelThreadPoolAdapter`, forwarding to `Level::Current->asyncUpdateThreadPool`), whose `ParallelFor` cuts the range into one equal chunk per thread and waits for the whole pool | §2.2 (`ForEachBodyParallel`) |
| `StaticMesh::IsCameraVisible` returns the last rendered frame's culling result (a cached flag) | pose LOD |
| Events emitted from one thread are flushed FIFO at the end of the next tick group | hit funnel ordering |
| `LoadAssets` may run on a worker thread / throwaway preload instance (`PreloadEntityTypeAsync`) | thread-safe `CombatData`, side-effect-free `LoadAssets` |
| `SkeletalMesh::SetAnimationTime / Update / PullRootMotion` (bone `"root"`), `PlayAnimation` | slaved clips, root motion |
| `AnimationPose` = bone-local matrices by name; `GetModelSpaceTransform` (missing bones = identity); `PasteAnimationPose` (missing bones = bind); a freshly loaded mesh reports its bind pose | Rig weapon posing (body bones + weapon bones), sockets |
| A `SkeletalMesh`'s pose snapshot refreshes only when the clip time changes (`dirtyPose`, protected) | `WeaponMesh::SampleClipPose` marks it dirty when sampling rest poses |
| `AnimationPose::Lerp` (root from B; above 0.995 returns B as-is, dropping bones only in A), `LayeredLerp` (root from B, traverses the given root's topology, bones missing in B count as identity, `UseWorldSpaceRotation` = blend space of the start bone); `LerpInPlace` / `LayeredLerpInPlace` give the same results written into the first pose | layers, snapshots, recovery (locomotion passed as B keeps the root), Rig weapon blends (both poses carry every weapon bone); upper-body layers restore the root |
| `MathHelper` (`RotateVector`, `FindLookAtRotation`, `Transform` / `DecomposeMatrix`, `NormalizeAngle`) | facing, root motion to world, weapon attach matrices |
| `Delay` (deadline on `Time::GameTime`) | move / AI attack cooldowns, AI reaction and target-loss timers, held AI buttons, respawn, corpse removal |
| ECS systems may split their loop with `ctx.ParallelFor` (`ParallelSafe`) on the level thread pool; `EcsScheduler::Emit` is thread-safe (event types registered at startup); `EcsWorld` mutations are immediate and not thread-safe | `CombatBodyUpdateSystem`, `ProjectileUpdateSystem` |
| Jolt narrow-phase queries (`Physics::LineTrace` / `SphereTrace` / `MultiSphereTrace`) are safe from worker threads (body read locks) | sweeps, aim and AI traces in the parallel update |
| `SoundPlayer::PlayOneshot`, `GlobalParticleSystem::SpawnParticleAt`, `Level::AddEntity`, `Camera::AddCameraShake`, `Time::AddTimeScaleEffect`, `AiPerceptionSystem` lists are main-thread only | the queued side effects of §2.2 |
| `CQuake3BSP::FindClusterAtPosition` / `IsClusterVisible` are read-only lookups | AI sight from worker threads |
| `Physics::MultiSphereTrace / MultiSphereOverlap` return `entity` + `hitboxName`; hitboxes are `BodyType::HitBox` from `.skmm` | sweeps, weak points |
| `CharacterController` moves by velocity during `Physics::Simulate` | root motion → velocity |
| `REGISTER_ENTITY` token-pastes the class name | entity classes at global scope |
| `StaticMesh::MeshHideList` hides model meshes by name prefix (copied in `FinalizeFrameData`, after `AsyncUpdate`) | hand / holster weapon copies |
| Sensor contacts: `Physics::gAdds` / `gRemovals` get one enter when an entity pair's first contact starts and one exit when its last one ends (sleeping bodies keep theirs) | - (behaviour of the engine fix in §13.1; triggers of other games) |
| The engine does not handle `info_player_start`; maps load as compiled `.bsp` | `SlasherPlayerStart` registers it and spawns the controller + hero |
| A glTF model's animated non-joint nodes (`root`) also take skin-palette slots | rig size budget below |

### 13.1 Engine changes

| Change | Why |
|---|---|
| `MAX_SKINNED_BONES` 128 → 256 (`skinned_model.hpp`) | the player rigs (cloth bones) use 135–138 palette slots; skinning writes past a 128-slot palette (heap corruption) |
| The palette reaches shaders as `finalBonesMatrices0` (slots 0–127) and `finalBonesMatrices1` (128–255); `SKINNED_BONES_PER_UNIFORM`, `Shader::SetUniform(name, const mat4*, count)`, `SkeletalMeshDrawCommand::ApplyAdditionalShaderParams` (still sets the legacy `finalBonesMatrices` for shaders of other games) | bgfx shader binaries store a uniform array's size in 8 bits (256 wraps to 0) and its per-draw opcode in 10 bits of registers |
| Game shaders: `GameData/shaders/source/bones.sh` (`GetBoneMatrix`) used by `vs_default`, `vs_shadowvolcap`, `vs_shadowvoledge`; compiled with `GameData/shaders/ShaderCompilationAutomation.exe` | the split palette |
| `AnimationPose::MakeAdditive` / `ApplyAdditiveInPlace` (`AdditiveSpace` Local / Mesh, like Unreal's additive animations) | aim sweeps and additive move layers (§9.1): a clip's difference from a reference pose added on a subtree. Mesh space turns bones in model space, so the aim holds whatever the pose underneath does with the parents and the coat stays where the clip keeps it; nothing else in the engine could add a pose |
| Bone cloth: `Cloth/BoneCloth.h` (spring-bone chains under root bones, joined across branches: each tip eases toward where the bone would hang (no springs), trailing part of the body's movement, with leg capsules and additive wind layers), settings `BoneClothData` in `SkeletalMeshMetaData::cloth` (`.skmm`); `SkeletalMesh::PasteAnimationPoseWithCloth` / `ResetCloth` / `ReloadClothData`; console `cloth_debug`, `cloth_reload`. `SkeletalMesh` constructor and destructor moved to the .cpp | the coat (§9.5): the baked coat keys could not react to what the body does; reusable for any game's coats, skirts, capes, tails |
| `World/WindField` (one world wind with gusts; console `wind`) | the coat's wind; the engine had no wind |
| `AnimationPose::LayeredLerp`: `UseWorldSpaceRotation` is no longer scaled by `progress` (it only chooses the blend space) | partial mesh-space blends mixed in the local-space result: a fading upper-body layer swung the torso toward the legs' pelvis twist (aim drifting after firing). Callers passing fractional values (ShooterGame) now get exactly the space they ask for |

Performance fixes (from profiling `-perf_crowd 300`, 2026-10-15; §13.2 has what was found and measured). Each keeps the
existing API and behaviour unless noted; all are standard C++ (no compiler- or library-specific behaviour).

| Change | Why |
|---|---|
| `EngineMain::Init` sets the ECS thread pool (`EcsScheduler::SetThreadPool(LevelThreadPoolAdapter)`); `GameStart` no longer does | every `ctx.ParallelFor` of every game ran serially |
| `Layers::TRACE_ONLY` mapped to its own broadphase layer, paired with nothing by the simulation; hitboxes (`CreateHitBoxBody`) are created in it, `StartRagdoll` / `StopRagdoll` move them to `MOVING` and back (`Physics::SetObjectLayer`) | hitboxes made a rejected narrow-phase pair with every overlapping capsule each step |
| Contact listener: sensor contacts recorded per thread during the step (no lock), paired up after it by Jolt's sub-shape keys (`afterSimulation`). **Behaviour:** an entity pair gets one enter when its first contact starts and one exit when its last one ends - before, the contact set was emptied twice per step, so enters fired every step and exits never | `physicsMainLock` taken per contact inside the multithreaded step; the enter / exit bug (triggers of ShooterGame / StealthAction now get exits) |
| Ignore pairs (`AddIgnorePair` / `RemoveIgnorePair`) lock their set | written from the parallel character updates (data race) |
| Traces take their ignore lists by const reference (filters view them with `std::span`); sphere / cylinder query shapes cached per thread | a vector copy and a shape allocation per trace |
| `CharacterController`: ground checks skip character bodies through the mask (no per-hit scan of every controller), its ignore list built once; stacking candidates from a broadphase box query (`Physics::CollectBodiesInBox`), controllers found through `BodyData::characterController` | allocations per trace, O(N²) scans; a ground cast hitting a neighbour lost the ground |
| `Level`: per-entity zone names only while a profiler is connected, phase lists in reused member buffers, `GetDrawMeshes()` by const reference; `FinalizeFrame` caches each draw command's sort key (`IDrawCommand::SortKey`) before sorting | ~1300 allocations per frame; the sort re-resolved the shader per comparison (0.22 ms on the main thread) |
| `StaticMesh::FinalizeFrameData` copies the hide list only when it changed | a set rebuilt per visible mesh per frame |
| `AnimationPose::LerpInPlace` / `LayeredLerpInPlace` (the value forms call them); `GetModelSpaceTransform` / `ApplyFABRIK` use the shared skeleton topology (looked up without a lock for a thread's last skeleton) instead of copying the whole cached skeleton per call; `PasteAnimationPose(const AnimationPose&)` | hundreds of allocations per evaluated body |
| `SkeletalMesh::UpdateHitboxes`: per-hitbox binding (bone index, inverse offset, constraint, velocities) rebuilt when the hitboxes or the model change; `GetBoneMatrix(const string&)` | string copies and name-keyed lookups per hitbox per frame |
| `RandomHelper`: per-thread `std::mt19937` | `rand()` is not thread-safe (and on MSVC every worker started from the same seed); `RandomInt` returned 0 |

### 13.2 Engine performance notes

Found profiling `-perf_crowd 300` (2026-10-15). Fixed unless marked otherwise (the fixes are listed in §13.1).

| Engine | Cost at 300 bodies | Status |
|---|---|---|
| Hitbox bodies are kinematic, `LinearCast`, in `Layers::MOVING`: every active capsule / sensor overlapping them (their own body's included) is a narrow-phase pair rejected by `OnContactValidate` every step (`dynamicCollisionGroupOrMask` disables Jolt's pair caching). `Layers::TRACE_ONLY` existed for this but was never mapped to a broadphase layer | 2.6 of the 3.7 ms physics step | fixed: physics step 3.6 → 0.9 ms |
| `AnimationPose` is an `unordered_map<hashed_string, mat4>`; `Lerp` / `LayeredLerp` return new maps; `GetResultPose` and `PasteAnimationPose(AnimationPose)` copy; `GetModelSpaceTransform` copied the whole cached skeleton (names, name map, children) under a shared lock per call | ~73 µs per evaluated body (11 ms CPU per frame), hundreds of heap allocations per body per evaluation from 14 threads | fixed: in-place blends, const-ref paste, shared topology; the game composes in place (animation 12.4 → 7.4–8.3 ms CPU, two runs). Not changed: the map itself (flat bone-index arrays would save the remaining lookups but change a type every game uses) |
| `SkeletalMesh::UpdateHitboxes`: per hitbox a `std::string` copy of the name, name-keyed lookups / inserts, a matrix inverse | 19 µs per body (5.8 ms CPU) | fixed: 16 µs per body (4.9 ms CPU) - the rest is Jolt's locked `SetPositionAndRotation` per hitbox (~1.3 µs each, 12 per body); batching it needs Jolt's broadphase interface, which the engine does not expose |
| `CharacterController::CheckGroundAt` builds ignore vectors per trace and scans every controller (`s_allControllers`) per hit; a ground cast hitting a neighbour lost the ground; stacking scans every controller twice | movement 28 µs per body (8.4 ms CPU), O(N²) | fixed: 18 µs per body (5.4 ms CPU), mostly the ground cast against the map |
| `ThreadPool`: one mutex-protected `std::queue<std::function>` for all jobs; `ParallelFor` = one equal chunk per thread and waits for the whole pool; idle workers fall back to 1–2 ms timed waits; the physics, async-update and main pools oversubscribe the usable threads | - | not changed: the bodies' phase runs at 84 % thread efficiency with `ForEachBodyParallel`, and waits are woken by notifications (the main thread's "Wait GameUpdate Jobs" time was `FinishFrame` work, now traced) - no measurable gain left for a scheduler rewrite |
| `Level::Update / LateUpdate / AsyncUpdate / PreFinalize` built a `std::string` zone name per entity per frame (also without Tracy), copied `LevelObjects` per phase; `GetDrawMeshes()` returned a copy | ~1300 allocations per frame | fixed |
| `Level::FinalizeFrame` sort called the virtual `GetSortKey()` (shader lookup) per comparison; `FinalizeFrameData` rebuilt each visible mesh's hide-list set | 0.44 ms on the main thread | fixed: 0.21 ms |
| `EngineMain::Init` never called `EcsScheduler::SetThreadPool` | every `ctx.ParallelFor` serial | fixed |
| `AiPerceptionSystem` / `Observer::UpdateVisibility`: every target against every observer, no tag (team) filter, the observer's BSP cluster recomputed per target | 37 ms CPU per frame when used | not changed: the game does not use its sight (0.04 ms per frame) |
| The contact listener took the global `physicsMainLock` for every sensor contact inside the multithreaded step (and fired enters every step, exits never) | contention in the step | fixed |
| `RandomHelper` used `rand()` (per-thread state on MSVC, every worker starting from the same seed; not thread-safe elsewhere) | - | fixed |

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
                               Jump, Glide, Fire, Draw, Reload, Sheathe, Reaction, Death, Finisher, Charge)
  Characters/                  CombatCharacter (+ movement), CombatBodySystems (parallel body frame and pose),
                               CharacterAnimator, WeaponSet, WeaponMesh, CharacterInput, Characters.cpp
                               (registered hero / enemy types)
  Control/                     Controller, PlayerController, PlayerCamera, CameraImpact (kick / rumble within
                               comfort limits), PlayerStart
  AI/                          AIBrain (+ registry), AIController, brains
  Weapons/                     Projectile (+ ProjectileUpdateSystem)
  Effects/                     particle systems (blood, melee trail, bullet trail)
  UI/                          CombatHud, Pause/*, LoadingScreen/*
  Settings/                    video / audio / input / gameplay (camera shake) settings + rebinding
  Debug/                       CombatDebug (panel, console commands incl. perf_crowd), MoveTimeline (the panel's move timeline),
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
| Coat: old-school trailing bones, chest to hem - each tip eases toward where it would hang, no springs; the locomotion clips' coat keys unused; wind as additive layers on the rest shape | What early-2000s action games did. A cloth-style simulation (surfaces, one-sided collision, blends with the animation) cost more and still showed seams |
| Target magnet adds to root motion instead of replacing it | Clips keep their authored motion; stationary attacks still close in; one rule keeps bodies out of each other |
| Hit-stop stops instantly at the contact and only eases the release | An eased onset let the blade slide through and read soft; lag came from long stops, so lights stay at 3-4 frames (DMC3) |
| Weight from strikes, not from locomotion | Halving acceleration / turn rate to add "weight" made the body feel uncontrollable; DMC-style crisp movement, impact from hit-stop, recoil, pushback, blood and camera |
| Melee target chosen all around the body by angle to the stick (facing without input) + distance | The player points at the body to hit; a cone dropped bodies beside or behind the hero |
| Launcher on Attack held (`holdOpeners`, `hold`): from rest after 0.1 s with a cancellable wind-up, during any move after 0.6 s committed; tap = opener on release | One attack button; the hold is the one input that cannot be pressed by accident while mashing. From rest the wind-up hand-over keeps the opener responsive; inside a move a mashed press must never launch, so the delay is long and the launch commits at once |
| Movement input ends attacks from their `Locomotion` cancel window (after the light zone) | Control returns ~0.25 s after a strike instead of at the end of the combo window; mashing with the stick held still steers; delay combos stand still (DMC rule) |
| Short light zones (0.08 clip s): the heavy zone starts soon after the window opens | Lights are buffered into the window start, so a short zone loses none; heavy presses arrive slightly early and a long light zone (0.15 - 0.2 s) turned them into lights |
| Body pose / movement in an ECS system (`CombatBodyPoseSystem`) instead of `AsyncUpdate`, bodies handed out dynamically | One place schedules all per-body work; runs after the hits resolved; dynamic hand-out instead of the pool's fixed chunks |
| AI sight in the game (hostile bodies, nearest first, bounded traces), the engine observer only for hearing | The engine observer is O(observers × targets) with traces and no team filter: 3.8 ms of the game thread at 300 bodies |
| Pose LOD also by camera visibility, attacking bodies always full rate; far / unseen bodies on line-trace movement | Crowds stand within the half-rate distance; blades sweep along the pose, so attackers keep it exact |
| Separate `actionBuffer` for dash / jump / fire | A long buffer fired dashes the player no longer wanted |
| Hit effects belong to the victim (`hitEffect`), hits only scale / override them | Blood vs sparks vs sand is what the body is made of |
| Camera impact on a spring + trauma shake, light hits 0 | Directional jolts on heavy strikes; a shake on every swing numbs it (DMC3 keeps light hits still) |
| Debug save patches the authored text instead of re-serializing | A full dump (every default, no notes, 0.699999988079071) turned a one-value tweak into a 78 KB rewrite of hero.moveset.json |
| Camera impact within hard comfort limits (≤ 12 Hz, bounded view speed, no roll, slide over turn, strongest wins) | The first tuning (18 Hz noise with 33 / 57 Hz harmonics, 2.5° rumble, roll, summed trauma) blurred the image and disoriented; comfort is enforced by code and validation, not left to tuning |
| Air attacks reuse ground clips with `NoLegs` (hips follow the clip, legs mostly air pose) | No separate air clips needed; an upper-body layer left the hips static |
| The ground moves are the air moves; the air look and physics come from the body (`airLegs`, `airCombat`) | Separate air moves duplicated tuning and could drift apart (an air opener without the ground opener's wind-up snapped out faster); combos now flow through take-off and landing |
| The drop needs the long hold (`holdDelayInMove`) even from rest, is committed while it dives, and spends its hold; the air launcher needs an airborne target in reach | The drop has no wind-up hand-over, so the 0.1 s from-rest delay turned slightly long taps into drops (the hover gone); a link or cancel open mid-dive fired a heavy or the launcher in the air or on landing; a grounded enemy below counted as something to launch, so the drop never started above one |
| Air combat keeps the rising arc and only hovers the fall | Stopping the rise when an attack starts killed jumps and launches; hanging at the top of the arc reads as natural DMC air combat |
| The hover is earned by landed hits, not by being in the air attacking; a successful self-launch shares its hit's stall | Swinging at air must not earn what a hit earns; a launcher that sends both bodies up should let the attacker hang with the victim |
| A hit settles the victim once, to a destination fixed at the hit; nothing follows the attacker | A tether that follows (even briefly, even only while hovering) drags enemies after a dashing player and feels like a leash; DMC juggles keep the enemy in reach through the hit itself |
| The attacker keeps its own height while hovering; only the victim is levelled | The attacker copying the enemy's vertical motion (the old behaviour) made the player float for as long as the enemy did - she could never fall first |
| Only the player's air time is limited (gravity grows back instead of switching on); a juggled enemy hangs while the attacker's hits keep coming (the attacker's `victimHold`, carried through the attack that hit it) | No infinite juggles, and the player comes down first; an enemy that fell for a reason of its own (its own hang clock, a short stall, a slow wind-up) felt uncontrollable - the player's hits alone decide |
| The grip of the weapon's own clips overrides the hands in local space | Blended arms never loosen the hand around the weapon |
| Procedural strafe from the forward run when no directional clips exist | Strafing while aiming without new assets; real clips replace it automatically |
| Camera framing offset in camera space around a focus point on the body | A world-up pivot offset collapses into depth when looking down (the body covers the crosshair); a camera-space offset keeps the composition at every pitch |
| Weapon reach is a procedural move of a weapon bone (`reach` per move, `extend` per weapon), applied before the weapon-bone transitions | One set of clips serves every reach; the blade sweep and trail follow the bone, and a move change blends the length like any weapon bone instead of snapping |
| Aiming is an authored sweep clip added in mesh space (its middle frame is the hold), not a procedural spine rotation | The spine rotation turned the coat with the spine; the sweep moves spine, arms and coat as authored, and mesh space keeps the aim right over any base pose |
| Additive poses in the engine (`AnimationPose`), next to the blends | A generic pose operation every game can use, not a SlasherGame helper |
| Per-weapon move tables in the character's moveset (`weaponMoves`), one moveset per body | Switching weapons swaps the opener, launcher, drop, draw, fire and reload without a second moveset to load or keep in sync; combo links stay moves of one file (editor) |
| Hand occupancy decides where the melee weapon goes when a firearm comes out | One rule covers holstering (pistols, rifle), staying in the hand (musket in the left hand) and sheathing (a right-hand gun beside the sword) |
| Sword attacks draw at once from the sheath; the sheathe plays after every combo | The user's choice: responsive openers; the sheathed idle is the sword's rest state |
| The standing sheathe is a full-body move started exactly when an attack has played out to its ready stance; otherwise an upper-body one in local space | The clip turns the hips 105° and steps the feet from the attack stance into the idle: played over idle legs (the first version) it twisted the body, and fading the attack into the idle first spun it |
| Sword layers in local space (`layerSpace`) | The sword's stance is side-on; a mesh-space hand-over held the chest 54° off the running hips |
| The rifle's reload is a Full overlay whose legs hand over while moving | Its clip spins the whole body on the root; on the upper body alone the torso spun over still legs |
| Firearm holds over movement moves on the hands only (`moveHoldBones`) | Jump / glide / dash clips carry no gun, so the hands must hold it; the whole upper-body hold (spine_01) stiffened the dash's torso upright, and the arms belong to the move too |
| The aim turns the yaw between facing and aim too (`sweep.yawBones`, `maxYaw`) | The sweep alone only pitches: the weapon trailed the body's turn toward the crosshair |
| Full-body clips (moves, Full overlays) keep their `root` while their legs hand over | The rifle reload's spin vanished in the air and while running: the legs' hand-over took the root from the pose underneath |
| The musket's arm in mesh space, kept out a moment after the shot and lowered before it hides | The local arm swung with the run's chest sway and the strafe twist; hiding at the clip's end popped the gun out of an aimed arm |

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
* Rifle fire clip (rifle_fire is clip-less: the aimed sweep pose holds while firing); the sword's `draw` clip is not
  used (attacks draw from the sheath at once); combat sounds for the musket and rifle.
