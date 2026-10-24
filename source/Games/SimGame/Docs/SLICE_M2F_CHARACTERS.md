# Slice contract: M2f character representation (one body for NPCs and the player; the player's viewmodel waits for items)

Status: **implemented** (2026-10-24), owner's request and answers in section 9. Self tests pass (11,274 checks), `-sim_bridge_test` and `-sim_transition_test` pass,
pictures looked at (section 10). Deviations from the first draft are listed in section 10.
Independent of M2e (the level transition) except that both touch `SimPawn`; do M2e first (it is small), then this.

## 1. Why

Today `SimNpc` is a body, a physics controller, a navigator and a fader in one class, and `SimPawn` is a camera on a capsule with no body at
all. The player will need a body (shadow, third person, cinematic cameras, mirrors later, a visible character in dialogue) and the NPCs
already have one. Writing the body twice is what ShooterGame did (`Player` owns `bodyMesh` and the first-person hacks, `PlayerRepresentation`
renders remote players); this slice writes it once.

## 2. The three pieces

```
driver (decides where the character is)        representation (how it looks)

SimNpc  (CharacterController + PathFollow) --\
                                              >--> CharacterState --> CharacterBody      (third person; every character)
SimPawn (CharacterController + input)      --/                   \-> CharacterViewmodel  (first person; the player only; built with the first item)
```

**`CharacterState`** (`Characters/CharacterState.h`, plain struct, no engine entities): the one value that says everything the look needs.
Drivers fill it; representations only read it. Nothing in it points back at a driver (ShooterGame's `PlayerState::FromPlayerPtr(Player*)` ties the
state to the Player class; here each driver fills the struct itself).

| field | meaning |
|---|---|
| `feet` | level-local feet position (smoothed, what the camera or the eye sees) |
| `bodyYaw` | where the body faces (degrees) |
| `look` | pitch and yaw the head and spine aim at (the pawn's camera rotation; an NPC's heading) |
| `velocity` | world velocity (the animator derives direction relative to the body) |
| `stance` | Standing, Crouching |
| `onGround` | for jump / fall poses later |
| `model` | which character model (`CharacterModels` table, section 4) |
| `variant` | picks the tint of the untextured citizen model, as today |
| `alpha` | 1 normally; the fade value while appearing or disappearing |
| `visibility` | Shown, Warming (spawned, not drawn; section 6), FirstPersonOwner (section 4) |

Later fields (held item model paths, hand IK targets, death pose) are added to this struct, never passed beside it.

**`CharacterBody`** (`Characters/CharacterBody.{h,cpp}`, an `Entity`, class name `sim_character_body`, `SaveGame = false`): the third-person
look of a character. Owns the skeletal mesh(es), the locomotion animator and the paint. `Apply(const CharacterState&)` once per frame by the
driver; it positions the mesh, runs the animator and pastes the pose. It has no physics, no navigation, no sim knowledge, so it can be
spawned for a mirror, a cutscene or a test with a hand-made state. Drivers hold it through an `EntityHandle`, not a raw pointer.

**`CharacterViewmodel`** (an `Entity`, `sim_character_viewmodel`, player only): the first-person arms, a skeletal mesh with `IsViewmodel = true`
(the engine draws it with its own projection, `Camera::ViewmodelFOV`) at the camera, hidden in third person, reading the same `CharacterState`.
**Not built in this slice** (owner, section 9: "no arms yet"): there is no item to hold and no arms clip worth showing, and an entity with nothing
to draw would be dead code. The seam is in place and costs nothing: the representation split, `CharacterState`, the pawn's `ViewMode`, and the
first-person body already hiding its own arms and neck. It is built with the first held item (M4), reusing this struct.

**Drivers** keep what they do now: `SimNpc` = controller + path following + goal handling (fade and paint leave it); `SimPawn` = controller + input
+ camera (it gains a view mode, section 5). Each creates its own `CharacterBody` and fills a `CharacterState`
each frame.

## 3. One animator for all

`SimNpcAnimator` becomes `CharacterAnimator`, used by every `CharacterBody`: driven by the state only (speed and direction relative to the body, stance).
Clips come from the model; what a model lacks it falls back from. `base.glb` has the full set (walk / run in four directions, crouch idle / walk
in four directions, idle, and more); `guard.glb` has idle, walk, run only. So: forward speed picks idle / walk / run; if the model has the
directional and crouch clips they are used (the player needs them), otherwise the forward clip plays whatever the direction. Written once as
a table of clip names per model in the `CharacterModels` entry, not as `if (model == ...)`.

## 4. First person: the visible body, and the full-body shadow

`base.glb`, `guard.glb`, `player_body.glb` and `player_coat.glb` share one UE4-style skeleton (`neck_01`, `upperarm_l/r`, `spine_03`, ... verified in the
files), so any clip plays on any of these meshes. The model table therefore has **two sources per entry**: the mesh and where its locomotion clips
come from. The player is `player_coat.glb` (it has only `rest`, `run`, `jump`, `inAir`, `dash` and the two additive coat clips) with
locomotion from `base.glb` (idle, walk and run in four directions, crouch set); citizens are `base.glb` / `guard.glb` playing their own clips.

In first person the player must see their own torso, legs and feet without arms, neck or head getting into the camera, and still cast the full
body shadow. The engine has `OnlyShadows` (draw command that only goes into the shadow pass) and `CastHiddenShadows`, but the pose is one pose
per mesh, and collapsing a bone also collapses it in the shadow. So a first-person-owned `CharacterBody` uses **two meshes sharing one pose
computation**:

| mesh | pose | drawn in |
|---|---|---|
| `visible` | the full pose with the model's `firstPersonHidden` bones scaled to zero (neck, both upper arms; and `spine_03` while looking down, as ShooterGame does so the chest stays out of the view; decided per model in the table) | the main passes, no shadow (`CastShadows = false`) |
| `shadow` | the full pose, untouched | shadow pass only (`OnlyShadows = true`) |

**Cloth** (the coat; engine `Cloth/BoneCloth`, settings in `player_coat.glb.skmm`, wind from `World/WindField`, unchanged and not redesigned: see the owner's
earlier decision of simple trailing bones): `CharacterBody` pastes through `SkeletalMesh::PasteAnimationPoseWithCloth(pose, deltaTime)` when
`HasCloth()` (as `CombatCharacter` does), after setting position and rotation for the frame, and `ResetCloth()` after a teleport or a level
change. The coat is simulated **once**: in first person the `shadow` mesh (full pose) runs the cloth, and the `visible` mesh receives the shadow mesh's
resulting pose with the hidden bones scaled to zero. To be confirmed in code before building on it: that `GetAnimationPose()` of a mesh returns the
post-cloth pose; if it does not, the visible mesh runs the cloth and the shadow mesh copies its pose before the bone edit. Either way one simulation. The
two assets (`player_coat.glb` and its `.skmm`) are copied from SlasherGame's `GameData/models/player/` into SimGame's.

The bone list is data in the `CharacterModels` table (one entry per model: path, directional clips, `firstPersonHidden`, tint rule), not code
sprinkled through a player class. NPC bodies create only the `visible` mesh (shadow on), so NPCs pay nothing for this. Switching the
pawn's view mode flips one flag on the body (`FirstPersonOwner`): the `shadow` mesh exists only while it is set, the `visible` mesh takes the
normal path otherwise. The body stands a little behind the camera in first person (ShooterGame: 0.2 m back along the facing) so the torso does not
clip into the lens; looked at in screenshots before it counts as done.

## 5. Third person for the player

`SimPawn` has a `ViewMode` (`FirstPerson`, `ThirdPerson`). Fallout style: a key toggles it (V; section 9), the console has `sim_view first|third`,
and launch `-sim_view third` for screenshots. In third person the camera sits behind and above the pawn on a boom, pulled in by a sphere trace
so it does not enter walls (`Physics::SphereTrace`, as ShooterGame's `UpdateThirdPersonCamera`; written as one function with named tunables
`distance`, `up`, `shoulder`, `radius`), the (later) viewmodel is hidden and the body's `FirstPersonOwner` is cleared (the full body draws, with no shadow
mesh). Movement stays camera-relative as now; the body faces the way it moves/aims by `bodyYaw` (turn-in-place is not in this slice).

For cinematic cameras the seam is `ViewMode` plus a camera override: anything (a dialogue, a scripted shot) can set the pawn to third person
and take over `Camera::position/rotation` for a time; the body does not care. Only the two rigs above exist in this slice; the director is
a later one.

## 6. The transition distance: bodies that exist but are not shown

Today bodies are made within `spawn_radius` (120 m) and, if the spot is in sight, fade in; out of sight they appear at once. A body that
appears 8 m away and starts from a standstill does not look natural. New rule: **bodies are made farther out than they are shown.**

| distance from the player | what a body is |
|---|---|
| `warm_radius` (spawn) to `visible_radius` | **Warming**: a real body (physics, path following, reports, the animator clock) that is not drawn at all and casts no shadow. By the time it reaches the visible radius it is walking at pace with a settled controller and a running animation. |
| inside `visible_radius` | **Shown**. A body that arrives from the ring is shown at once if warmed up (it is walking, nothing pops) |
| beyond `visible_radius + 5 m` hysteresis (walked away) | back to Warming |
| beyond `despawn_radius` (existing rule) | released, as today |

A body counts as warmed up after `WarmupSeconds` (1.5 s of life with a goal). Rules for the cases that are not "walks in from the ring" stay
as they are (loading-screen window: at once; made inside the visible radius, out of sight: at once; in sight: fade over 1 s) because the player can
load into or teleport next to a crowd and that must not wait for a warm-up. The fade stays as the fallback; it just stops being the normal case.

Hidden bodies cost a controller update and an animator clock tick each, no draw and no skinning (the pose is not pasted). `sim_stats` gets
`warming` and `shown` counts, and each change is logged like the others (`sim body: <id> becomes visible (walking in from the ring, N m)`).

Where the radii live: `spawn_radius` / `despawn_radius` are existing sim tunables (`BodiesModule`); `visible_radius` is a presentation
setting of the game side (`SimAgents`), tunable at runtime from the console (`sim_body_radius`). Defaults in section 9.

## 7. What moves

| from | to |
|---|---|
| `SimNpc` mesh, paint (`ApplyPaint`), fade (`FadeIn/Out/UpdateFade`), animator | `CharacterBody` (the fade becomes `CharacterState.alpha`; `SimNpc::FadeIn/FadeOut` stay as the driver's calls and set it) |
| `SimNpcAnimator` | `CharacterAnimator` |
| `SimNpc::ModelPath` | `CharacterModels` table |
| `SimAgents` appear / disappear rules | unchanged; gain the Warming state of section 6 |
| nothing in the sim | no sim change at all: bodies, reports, goals, saves are as they are |

SIM_CONTRACT sections 9 and 10 are updated by the implementation (pawn and bodies).

## 8. Tests

Pure (sim self tests): `CharacterModels` entries are valid (clip names exist as strings, hidden bones non-empty for every model with a
first-person flag); the visibility decision as a pure function `DecideBodyVisibility(distance, warmedUp, wasShown, ...)`: every row of the
table in section 6 including hysteresis, the load window, and "made inside the radius".
Against the real files: every model in the table loads and has the bones in `firstPersonHidden` and the clips it names (the check reads the
glb, as the nav tests read the fixtures).
In game (`-sim_bridge_test` extended): bodies of NPCs in the ring are counted warming and not drawn (`mesh->WasRended` false), enter the visible
radius and become shown without a fade; the pawn has a body; view mode switches both ways and the flags follow (shadow mesh present exactly while
first-person-owned); the coat of the pawn's body moves when the pawn turns.
Looked at (screenshots with `-sim_view` and `-sim_look`): the pawn in third person from behind (walking and standing), the pawn in first
person looking down (legs and torso visible, no arms or neck), and a shot of the shadow on the ground in first person
showing a full body (head and arms included); an NPC walking in from the ring at 2 s intervals.
Not verifiable headless and left to the owner: how the transition distance feels in motion, the third-person camera feel.

## 9. Decisions (owner, 2026-10-24)

1. **Radii**: `visible_radius` 45 m, spawn 65 m, despawn 80 m (the warm ring is 20 m wide). In the 64 m test levels part of each level is
   therefore Warming, which tests the ring. Replaces the 120 / 140 of SIM_CONTRACT section 10.
2. **Player body**: `models/player/player_coat.glb` with the cloth simulation applied; its locomotion animations come from other models on the
   shared UE4 skeleton (section 4: `base.glb`). Citizens keep `base.glb` / `guard.glb`.
3. **Viewmodel arms**: none yet (section 2).
4. **Third-person toggle key**: V.

## 10. As built, and where it differs from the draft

Files: `Characters/CharacterState.h`, `CharacterModels.{h,cpp}`, `CharacterAnimator.{h,cpp}`, `CharacterBody.{h,cpp}`, `BodyVisibility.h` (pure); `SimNpc` and `SimPawn` are
drivers; `SimAgents` owns the ring; assets `GameData/models/player/player_coat.glb` + `.skmm` copied from SlasherGame (byte-identical). Launch `-sim_view first|third`,
`-sim_pitch <deg>` (positive looks down), console `sim_view`, `sim_body_radius`, key V (action `view`, in the settings list). Sim radii are now 65 / 80 (`BodiesModule`).

Differences from the draft:
- **Light sampling.** The engine lights a mesh from the light volume at its *entity's* position. A body whose entity stands on the floor came out black (the pawn's body
  did, citizens did not); `CharacterBody` puts its entity at the body's middle (feet + 0.9 m), as the old `SimNpc` capsule centre did.
- **`PreloadAssets()`** is called on every mesh, as SlasherGame does for its coat (without it the coat's embedded textures did not apply).
- **No look-down chest bone.** The draft hid `spine_03` while looking down, as ShooterGame does. Looking down by 60 and 80 degrees showed the coat front and no chest in the way, so
  the field was dropped rather than kept on a guess. (The engine's pitch is positive when looking down; ShooterGame hides it when `x < 0`, which is looking up here.)
- **Shadow mesh** is made on the first first-person frame and switched with `Visible` (not created and destroyed); the visible mesh casts shadows only when it is the whole body.
- **Third-person boom** has `distance` 2.4, `up` 0.3 and sphere `radius` 0.3 as constants in `SimPawn.cpp`; no `shoulder`. The body always faces the camera's yaw (as ShooterGame):
  strafing and backing up play their own clips; turn-in-place is not in this slice.
- **Animator**: idle blended with a forward and a side clip by the movement relative to the body, one gait phase shared by both moving clips, run/walk by speed with a hysteresis
  (3.2 / 2.8 m/s). It replaces the citizens' idle / walk / run switch, so a start from standing blends in.
- **Test hook**: the bridge test runs with a visible radius of 15 m so the ring is used in the 64 m levels (30 bodies made warming, 2 came out of it in the run).

Checked (headless pictures, `-sim_view` / `-sim_pitch`): third person from behind (hat, coat, lit), first person looking down at 35, 60 and 80 degrees (no neck, no arms, the coat
front and its edges at 80, and the floor shadow showing a full body with head and raised arm), a citizen and a warden walking (animated, lit), 2 transitions of bodies through the ring
in the log. Not checked: the cloth moving in motion (still pictures only), the feel of the third-person camera, the ring in a big level, the foot sliding of the run clip at 8 m/s (the
calibration slice), a front view of the player (no camera for it yet).
