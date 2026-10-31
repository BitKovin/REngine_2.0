# Bone Cloth Rig: Blender add-on design

Status: **design approved (decisions in §14); Phases 1 and 2 built, see §15 for what exists and what differs.**
Add-on home: `Tools/BlenderAddons/bone_cloth/` (Blender extension, `blender_manifest.toml`). Target and minimum: Blender 5.1
(slotted actions, bone collections); no compatibility code for older versions.
Engine side it feeds: `source/Engine/Cloth/BoneCloth.{h,cpp}`, `Cloth/BoneClothData.h`, `SkeletalMesh::PasteAnimationPoseWithCloth`,
`AnimationPose::MakeAdditive / ApplyAdditiveInPlace`, `World/WindField`. Engine contract version: commit `bdacf806f` ("cloth simulation",
2026-10-22).

---

## 0. In one page

"Bone Cloth" is a Blender add-on that takes a character armature and a cloth mesh (long coat, jacket, skirt, robe, cape, single strand)
and makes everything `BoneCloth` needs: the strip bones, the weights, the rest shape, the two wind poses, the leg colliders and the
`.skmm` settings. A guided sidebar takes someone who has never rigged through ten short steps. Each step shows a status and has one main
button:

| # | Step | What the beginner does | What the add-on does |
|---|------|------------------------|----------------------|
| 1 | Character | Picks the armature and the cloth mesh (or just selects them) and a preset ("Long coat") | Finds hips, chest, legs, arms and the facing; shows them with checkmarks |
| 2 | Mesh | Presses **Fix all** | Bakes odd transforms, cleans modifiers and groups, gives an unskinned mesh body weights, keeps a hidden copy of the original |
| 3 | Shape | Moves a few sliders (front opening, panels, rows, top / waist / hem lines) while watching ghost bones in the viewport, then **Build bones** | Generates the root and strip bones, named, rolled, joined and mirrored |
| 4 | Weights | Picks "where the coat leaves the body" (lines plus a curve, plus a belt if there is one), then **Build weights** | Computes every weight, blends panels, keeps sleeves on the arms, limits to 4 influences, smooths |
| 5 | Rest shape | Ticks "Keep clear of the legs", adds a little flare, or poses by hand, then **Apply** | Bends bones and mesh into the new rest shape (shape keys included), with a snapshot for reset |
| 6 | Wind poses | Sets "Open: 24 cm at the hem" and "Volume: 11 cm", previews with sliders | Solves the bone rotations and writes the two-frame clips the engine adds on |
| 7 | Collisions | Nothing (auto), or adds one | Finds thigh/calf capsules and fits their radii exactly as the engine will |
| 8 | Motion | Picks a feel ("Heavy wool", "Light silk") or tunes plain-language sliders | Fills `BoneClothData` |
| 9 | Preview | Presses Play and picks a test motion (run, spin, stop-and-go, wind) | Runs a Python port of `BoneCloth::Step` in the viewport, so the swing looks as it will in game |
| 10 | Check & export | Fixes what the check lists (each item has a **Fix** button), presses **Export** | Runs the engine's rules, exports the `.glb`, merges the cloth entry into `.skmm` |

**Auto Setup** runs steps 1–8 with the preset's defaults. That gives a working coat in one click, and any step can be tuned and
re-run afterwards. Every stage is non-destructive: the add-on regenerates from its settings and from the hidden original mesh, so
changing "rows: 3 → 4" is one slider move and **Update**.

---

## 1. Goals, non-goals, principles

### Goals
- **G1 Beginner first.** Someone who knows Blender's viewport but not rigging finishes a coat in about 10 minutes, with no
  constraints, drivers, weight painting or JSON.
- **G2 Any cloth.** Long coat (open front, two anchors), short jacket (open or closed), skirt (closed ring), robe, cape (back only),
  and single strands (tail, scarf, belt flap).
- **G3 What you see is what the engine does.** Strips found by the engine's rules, weights limited to 4 exactly as exported, colliders
  fitted with the engine's formula, layer weights combined the engine's way, and a preview swing that ports the engine's algorithm.
- **G4 Non-destructive and repeatable.** Settings drive generation; the original mesh is kept; **Update** re-runs only what is stale.
- **G5 Escape hatches for experts.** Every generated item (bones, weights, rest shape, wind poses) can be edited by hand. The add-on
  notices hand edits and asks before it overwrites them.
- **G6 Adopt existing rigs** (`player_coat.blend`) without losing anything.

### Non-goals
- No engine changes (section 14 lists engine limits only as notes).
- No Blender cloth simulation; the only simulation is the engine port used for preview.
- No branching strips (the engine follows one child per bone).
- No dual-quaternion skinning (the engine skins linearly).
- No live link to a running game.
- Not a body auto-rigger: the character already has a skeleton.

### Principles
- **P1 One source of truth.** The bones, weights and actions in the `.blend` are what gets exported. Add-on settings regenerate them
  and never shadow them, so a hand-edited bone is the bone.
- **P2 Mirror the engine in one place.** `engine_contract.py` holds every constant and rule copied from `BoneCloth.cpp`, with the engine
  commit it matches. Nothing else re-derives engine behaviour.
- **P3 Beginner words, engine names in tooltips.** The panel says "Hem stiffness"; its tooltip ends with `followTip`.
- **P4 Safe operations.** Each operator is one undo step. Destructive stages (mesh fix, rest shape) keep a snapshot, and **Start over**
  always works.
- **P5 Touch only what it made.** Generated bones, groups, constraints and actions are tagged. The add-on edits user data only when the
  user asks (adopt, mesh fixes, removing keys).

---

## 2. Engine contract (what the rig must satisfy)

Each row: an engine rule (from the source), what it means for the rig, and how the add-on handles it. `engine_contract.py`
implements every rule marked ⚙ as a pure function (numpy, no `bpy`) and unit-tests it.

| # | Engine rule | Meaning for the rig | Add-on response |
|---|-------------|---------------------|-----------------|
| C1 | Settings live in `<model>.glb.skmm` → `cloth: [BoneClothData…]`; `cloth_reload` re-reads them in game (`SkeletalMesh::RefreshCloth`) | Settings-only changes need no model export | Export writes `.skmm` by merging; "Settings only" export button |
| C2 ⚙ | Strips: every child of a root bone with ≥1 vertex weighted ≥ **0.05** (`SkinnedWeight`) starts a chain (`BuildStrips`) | A strip bone without such a vertex is invisible to the engine | Weights guarantee each generated bone gets them; check `STRIP-EMPTY` |
| C3 ⚙ | A chain continues through the child with the **most** such vertices; a bone with none ends it, and its children are dropped | Linear chains only; helper bones must carry no weight | Generator makes linear chains; checks `STRIP-BRANCH`, `STRIP-TRUNCATED` |
| C4 ⚙ | **Join**: chain A continues into chain B when B's top head, seen in A's last bone's frame, has `y > 0` and sideways ≤ **0.05·y** (`JoinTolerance`). Heads only | Two-anchor coats (chest part plus skirt part) depend on exact alignment | Generator puts the lower top head exactly on the upper bone's tail; check shows joins and near-misses (5–20 %) |
| C5 ⚙ | A joint's swing direction is toward the **next bone's head** (`tipLocal`), not the Blender tail | Disconnected tails are cosmetic only | Generator connects bones; info `TAIL-NOT-NEXT-HEAD` on adopted rigs |
| C6 ⚙ | Strip end: along the last bone's +Y (the head-to-head axis of the bone above, in the last bone's frame), as far as vertices weighted ≥ **0.25** (`TipWeight`) to the last bone reach, unless `tipLength > 0`. Under 1 cm it falls back to the previous segment's length | Hem vertices must carry ≥ 0.25 on the last bone, or the strip ends short | Weights put full cloth weight on the last bone at the hem; panel shows the engine's computed tip; check `TIP-SHORT` |
| C7 | `follow` and `maxAngle` interpolate root→tip by joint **index** `k/(count-1)` | Bone count changes the distribution, not the overall feel | Motion panel shows per-joint values for the active strip |
| C8 ⚙ | Strip side: top head minus the mean of all strip tops, along `left = up × forward`; within **±0.02 m** (`SideTolerance`) it is the middle (0), else +1 left / −1 right | Windward opening uses the side; back panels 2.3 cm off-centre count as sides | Panel shows L/R/M per strip; check `SIDE-AMBIGUOUS` within 3 cm of the tolerance |
| C9 ⚙ | Collider: capsule from `bone` head to `to` head. A radius of 0 is fitted: the median distance of vertices whose **dominant** bone lies on the way from `to`'s parent up to `bone`, or is a childless bone hanging off one (twist bones). Start radius from t ≤ 0.3, end from t ≥ 0.7; fallback is the median of all, then **0.06 m**. Every mesh in the model counts | Auto radius needs `to` to descend from `bone`; cloth vertices still dominated by body bones inflate torso capsules | Fitted with the same function and drawn; check `COLLIDER-NOT-DESCENDANT`; manual radius suggested for torso colliders |
| C10 ⚙ | Wind layer = clip by name, first and last frame; per-bone Local difference `last·first⁻¹` (rotation; translation and scale differences too). A strip uses a layer if any of its bones turns ≥ **0.5°** | Two-frame action, frame 1 = rest; unkeyed strips ignore the layer | Pose solver writes exactly that; per-column multiplier 0 → not keyed → strip excluded |
| C11 | `ApplyAdditiveInPlace` clamps the weight to **0..1**; open weight = smoothed wind ramp + `spinOpen`·spin; volume weight = `volume` + wind ramp + `spinVolume`·spin | The clip is the most the coat ever opens or flares | UI says "full open" and "full volume", not "per m/s" |
| C12 | Every frame, roots and strip bones take their **rest** transforms (animation keys on them are ignored); then the volume layer, then the open layer, applied on each run's top (its subtree) | The rest pose is the coat's shape; keys on cloth bones in locomotion clips are dead data | Rest shape is a first-class step; check offers to strip cloth keys from other actions |
| C13 | Swing (`BoneCloth::Swing`): tails ease toward the rest tail with `1−exp(−follow·dt)`, carried by `(1−trail)` of the body's movement, limited by `maxAngle`, pushed out of colliders (+`radius`), limited again; `dt` ≤ 1/15 s; snap on reset or a jump > `teleportDistance` | — | Preview simulator ports it line for line |
| C14 | Spin bones = all ancestors of the roots; spin rate = fastest yaw about world up | Anchoring decides which body turns count | Shown in the Motion panel ("Spin is read from: pelvis, spine_01…03") |
| C15 | ≤ **4** influences per vertex (`MAX_BONE_INFLUENCE`); ≤ **256** skin joints (`MAX_SKINNED_BONES`), and every exported bone is a joint, helpers included | Weights are trimmed to 4; helper bones use palette slots | The add-on limits to 4 itself, so the exporter trims nothing; check counts joints |
| C16 | Skeleton = every glTF node, under one synthetic `RootNode`; model space = glTF scene space (metres, Y-up), including the armature object's transform. Bone and object names share one namespace | An object named like a cloth bone breaks lookup | Check `NAME-CLASH` |
| C17 | `forward` is model-space facing, default +Z, which is Blender −Y after the exporter's Y-up conversion | A character facing +Y in Blender needs forward (0,0,−1) | Auto-detected from the feet; written to `.skmm` |
| C18 | Units: lengths m, speeds m/s, angles degrees, `followRoot/Tip` 1/s | — | All add-on lengths are world metres (Blender shows scene units) |
| C19 ⚙ | **Wind and turn clips** (`windFront/Left/Back/Right`, `turnLeft/Right`): each sampled at **17** even times; a sample is the Local change from the clip's first frame. Clip position = `ramp(windStart, windFull, |apparent wind|)` (horizontal, smoothed by `layerSmoothing`), or the signed turn rate's ramp (`spinStart..spinFull`, settling over `spinRelease`). The two wind sides nearest the wind's direction share by angle; wind and turn share by position; rotations are summed short-way and normalised | Frame 1 must be the rest shape; the clip's length sets the steps; both sides of a mirrored clip swap | `clip_shares`, `sample_pair`; `poses.blend_clips` previews exactly this; the clips replace `openClip` / `volumeClip` in the `.skmm` |
| C20 ⚙ | **Surface colliders** (`surface: true`): the joints' tips ignore them; instead each joint's **probes** (up to **6** vertices it carries ≥ **0.5** of, farthest-first) are kept out, the joint turned about its head (up to **3** times a frame, no padding, past `maxAngle`). A probe resting inside a surface capsule is ignored by it | The cloth, not the bone, must clear the body: bones may run inside it | `pick_probes`, `probe_ignore`; Torso button; auto radii end 1 cm inside the resting cloth (5th percentile) and are always written |
| C21 | Collider `offset` (model space): both ends shifted at rest, each then moving with its bone | Side capsules for an oval torso | Written in engine axes when non-zero |

---

## 3. What the current coat taught us

### 3.1 State of `sourceAssets/SlasherGame/player_coat.blend` (inspected 2026-10-29)

| Item | Value |
|------|-------|
| Armature | object `root`, scale 0.01 (armature space is centimetres, world is metres), 134 bones, UE-mannequin names |
| Cloth mesh | `coat.001`, 1,074 verts. Object rotated 90° X relative to the armature, world scale 0.01; modifier `Armature.001`. An older `coat` (5,792 verts) is hidden, skinned only to body bones, and not exported (`use_visible`) |
| Roots | `cloth_root2` under `spine_03` (chest, head 1.42 m); `cloth_root` under `pelvis` (waist, 1.10 m) |
| Panels | 6: front/side/back × l/r at about ±51°, ±110°, ±173° (0° = front, + = character's left); open front gap about ±40° |
| Chains | `coat_<col>_00_<s>` (chest → waist, 20–32 cm; side panels start lower, below the armpit) joined to `_01.._03` (about 20, 20, 18 cm), hem bones end at 0.52 m; mesh bottom 0.507 m |
| Cloth share by height | 0 above 1.30 m; 0.11–0.37 on the chest; a **belt band** at 1.10–1.15 m pinned to the body (0); 0.4 at 1.05 m; 1.0 below 0.95 m. 79 vertices blend across two panels |
| Legacy control rig | 43 `MCH-*` bones, 3 controls (`coat_open_ctrl`, `coat_wind_ctrl`, `coat_flutter_ctrl`), 62 drivers with hand-derived expressions; constraints: Waist Follow (damped track 0.85), Coat Controls (copy rotation, before, offset), Follow Upper (copy location to the upper tail), Hang (damped track 0.7), knee solvers |
| Wind clips | `coat_open` (open_ctrl Y 0→10) and `coat_volume` (wind_ctrl Y 0→25, open_ctrl Y 0→−6), baked from the driver rig at export by force sampling |
| Export | Scene `glTF2ExportSettings`: GLB, ACTIONS, 4 influences, Y-up, force sampling, deform bones **off** (all 134 bones are skin joints, 46 of them helpers the engine never reads), `use_visible` |
| Settings | `player_coat.glb.skmm`, duplicated in SlasherGame and SimGame |

The two-anchor layout matters to the engine: the upper bone's rest follows the chest root, and the skirt bone's rest is kept on the
upper bone *as the two sets of rest transforms relate in this frame's body pose* (`aboveNow·inverse(aboveRest)·restModel`, where
`restModel` of the skirt bone hangs from the pelvis). So the skirt stays attached to the chest part's end but turns with the hips. The
add-on keeps this layout as the coat default and explains it in one line ("chest part rides the chest, skirt turns with the hips").

### 3.2 Pain points → features

| What was hand-made last time | Why it was hard | Add-on feature |
|------------------------------|-----------------|----------------|
| Placing 24 bones on the coat surface, mirrored, with matching rolls | Manual edit-mode work; easy to get joins and rolls inconsistent | §8.3 generator from guides: ray-cast placement, exact joins, symmetric rolls |
| Weights: chest blend, belt, panel blending, sleeves kept on arms | Painting by hand, then normalising, then limiting to 4 | §8.4 profile + pins + exclusions + panel/joint blending, limit and smooth built in |
| Bake the odd mesh transform (90° X, 0.01 scale) | Easy to break shape keys or normals | §8.2 one-click "Bake transform into mesh" |
| Slightly opening the rest shape | Apply-pose-as-rest breaks the mesh unless the armature modifier is applied with care | §8.5 own linear-blend skinning bake (handles shape keys), snapshot and reset |
| Open and volume target poses: 3 controls, 3 solver bones, 62 drivers | Expressions derived by hand and fragile | §8.6 direct pose solver from "cm at the hem" and per-column amounts; hand-edit mode |
| Capsule radii guessed by reading the game log | Engine fits at load and only logs the result | §8.7 same fit in Blender, drawn |
| Knowing whether it works before export | Export, run the game, `cloth_debug`, repeat | §8.9 preview port of the engine; §8.10 engine-rule checks |
| Writing `.skmm` by hand in two game folders | JSON by hand, copies drift | §8.11 merge writer, several export targets |

---

## 4. Vocabulary (panel words ↔ engine words)

| Panel word | Meaning | Engine / Blender name |
|------------|---------|-----------------------|
| **Panel** | One column of bones from where the cloth leaves the body to the hem | strip |
| **Anchor** | The body bone a panel hangs from (chest or hips) | parent of a root bone |
| **Root** | Small generated bone under an anchor that the panels hang from | `roots[]` |
| **Upper part / skirt** | The chest-to-waist bone and the bones below the waist, joined into one panel | joined chains |
| **Cloth amount** | Per vertex, 0..1: how much the panel bones move it instead of the body | the cloth share of its weights |
| **Belt / pin** | A band that stays on the body | cloth amount forced to 0 |
| **Rest shape** | How the coat hangs when nothing moves; the swing returns to it | armature rest pose of the cloth bones |
| **Wind poses** | Open (front panels swing out) and Volume (everything flares) | `openClip`, `volumeClip` |
| **Collider** | Capsule the bone ends cannot enter | `colliders[]` |
| **Padding** | Extra distance from colliders | `radius` |
| **Top / hem stiffness** | How quickly a bone catches up with where it would hang | `followRoot` / `followTip` |
| **Lag behind movement** | Share of the character's movement the cloth trails | `trail` |
| **Swing limit** | Most a bone turns from its rest direction | `maxAngleRoot` / `maxAngleTip` |
| **Column** | A panel's position around the body, as an angle (0° front, +90° character's left) | — |

---

## 5. Coordinate systems and units

| Space | Up | Units | Use |
|-------|----|-------|-----|
| Blender world | +Z | metres (scene unit scale must be 1) | All user-facing lengths, preview simulation, wind |
| Armature space | +Z | armature object's scale (cm for `player_coat`) | Bone and vertex math (after mesh prep, mesh space = armature space) |
| Bone local | +Y along bone | armature units | Pose bases, roll |
| glTF / engine model space | +Y | metres | `.skmm` vectors (`forward`) |

- **Body frame** (computed in step 1, stored): origin = hips anchor head; `up` = normalised (chest anchor head − hips anchor head),
  blended 50/50 with armature +Z so a leaning rest pose does not tilt everything; `F` = facing (horizontal, from the feet: the sum of
  `ball − foot` per side, projected; fallback −Y); `L = up × F`. Column angle θ: `atan2(dot(d, L), dot(d, F))` for `d` = point minus
  axis, so 0° = front, +90° = character's left, ±180° = back. Height `h = dot(p − origin, up)` plus the origin's world height, shown as
  height above the floor.
- **To engine**: `p_engine = C · p_world`, with `C` mapping (x, y, z) → (x, z, −y). `forward_engine = C · F`.
- **Lengths**: properties use `subtype='DISTANCE'` in world metres, so Blender shows them in scene units ("24 cm"). They are converted
  to armature units with `s = armature.matrix_world` scale, which must be uniform (check `ARMATURE-SCALE-NONUNIFORM`).
- **Angles**: degrees in the UI (`subtype='ANGLE'` shows degrees and stores radians), degrees in `.skmm`.

---

## 6. Architecture

### 6.1 Pipeline and staleness

```
 Original mesh (hidden copy)        Character (landmarks, body frame)
          │                                   │
          │                Shape guides ──────┴──► Bones (rest A)          [or adopted bones]
          │                                          │
          └───────────► Weights  (source geometry + bones A + profile)
                              │
                         Rest shape (parametric + hand rotations)
                              │  ── LBS bake ──► result mesh, bones rest B
                              │
                 ┌────────────┼──────────────────┐
             Wind poses   Colliders (fit)     Motion
           (rest B → 2-frame actions)            │
                 └────────────┼──────────────────┘
                           Check ──► Export (.glb + .skmm)
```

- Each stage stores the hash of its inputs (its settings plus upstream output hashes) after it runs. The panel compares and marks a
  stage **stale** (⟳) when an upstream stage or its own settings changed. **Update all** re-runs stale stages in order; each step panel
  also has its own **Update**.
- **Hand edits.** After a stage runs, the add-on records a fingerprint of what it produced: bone heads/tails/rolls, a weight checksum per
  cloth bone, action key values. If the fingerprint no longer matches, the stage is **hand-edited** (✎). Re-running it then asks: "You
  changed the weights by hand since they were built. Rebuild will replace those edits. [Rebuild] [Keep mine and mark up to date]".
- Live preview of settings (overlay ghosts, pose preview) never writes data; only stage buttons write.

### 6.2 Original mesh copy

The first time a cloth mesh is processed, the add-on duplicates it as `<name>.bc_source`:
- linked to a collection `Bone Cloth Sources` that is **excluded from the view layer** (never exported, never visible);
- has no modifiers; custom property `bone_cloth_source_of = <object name>`;
- holds the original positions, shape keys and the **original body weights**.

The visible object stays the one that is exported; its data is regenerated from the copy (weights) and then deformed (rest shape).
Two extra actions:
- **Start over**: restores the visible mesh from the copy and removes generated bones, groups, constraints and actions.
- **Use my edited mesh as the source**: when the user has edited the visible mesh, for example fixed a modelling issue. It un-bakes the
  rest shape by applying, per vertex, the inverse of its blended skin matrix, then replaces the copy (same check as §8.5). A vertex-count
  mismatch between the copy and the visible mesh triggers a banner offering this.

### 6.3 Tagging generated content

| Item | Tag |
|------|-----|
| Bones | in bone collections `Cloth` (strip bones) and `Cloth Roots`; edit-bone custom property `bone_cloth = "<setup id>"` |
| Constraints | named `BC Follow` (only on joined skirt-top bones, see §8.3.5) |
| Vertex groups | cloth-bone groups; helper group `BC amount` (paintable cloth amount); optional `BC exclude`, `BC pin` |
| Actions | custom property `bone_cloth = {"setup": id, "layer": "open"|"volume", "mode": "generated"|"hand", "hash": …}`, fake user on |
| Source copies | §6.2 |

### 6.4 Data model

Stored on the **armature object**: `Object.bone_cloth` (`BoneClothArmature`). All references are by name (strings) or
`PointerProperty(type=Object)`, never cached Python objects, so undo stays safe.

```
BoneClothArmature
  setups: Collection[BoneClothSetup]
  active_setup: int
  overlay: OverlaySettings                 # which ghosts and helpers are drawn

BoneClothSetup
  id: str (uuid, hidden)       name: str ("coat")        prefix: str ("coat")   # bone names <prefix>_<column>_<NN>_<l|r>
  preset: enum {LONG_COAT, JACKET, SKIRT, ROBE, CAPE, STRAND, CUSTOM}
  cloth_objects: Collection[ObjectRef]     # meshes the panels move (outer shell, lining, buttons...)
  character: CharacterSettings
  guides: GuideSettings
  weights: WeightSettings
  rest: RestSettings
  poses: PoseSettings
  colliders: Collection[ColliderSettings]
  motion: MotionSettings                   # = BoneClothData scalar fields
  export: ExportSettings
  state: StageState × 9                    # per stage: input hash, output fingerprint, last run time, message

CharacterSettings
  hips, chest: str (bone)                  # anchors; chest = bone whose children include both clavicles / upper arms
  thigh_l/r, calf_l/r, foot_l/r, ball_l/r: str
  arm_roots_l/r: str                       # clavicle; its descendants are "arm"
  exclude_bones: str list                  # default arms + neck + head chains (cloth amount 0 where they dominate)
  rig_family: enum (detected) {UE, MIXAMO, RIGIFY, METARIG, GENERIC}
  facing: enum {AUTO, NEG_Y, POS_Y, NEG_X, POS_X}; facing_vector (read-only, world)
  body_frame: origin, up, forward (read-only, cached)

GuideSettings                                       # ring layouts; STRAND has its own block
  layout: enum {OPEN_FRONT, CLOSED_RING, BACK_ONLY, FRONT_BACK, STRAND}
  anchors: enum {CHEST_AND_HIPS, HIPS, CHEST}       # coat / skirt+jacket / cape
  opening_half_angle: float (deg, OPEN_FRONT; auto-detected)
  back_vent: bool (hard split at 180°)
  columns_per_side: int 1..6;  center_back: bool;  center_front: bool (CLOSED_RING)
  column_spread: float 0..1                         # 0 = columns toward the opening, 1 = toward the back
  columns: Collection[ColumnGuide]                  # filled from the above; editable
  top_line: float (world height, auto = chest anchor head);  top_auto: bool
  waist_line: float (auto = height of the hips anchor's first spine child); waist_auto: bool
  hem_line: float (auto = lowest cloth vertex + 1 cm); hem_auto: bool
  upper_part: bool                                  # make the chest-to-waist bone (CHEST_AND_HIPS)
  rows: int 1..8                                    # bones below the waist
  row_spacing: enum {EVEN, DENSER_AT_HEM, DENSER_AT_TOP}; row_bias: float
  placement: enum {OUTER_SURFACE, MIDDLE, INNER_SURFACE}; surface_offset: float (m)
  path: enum {FOLLOW_SURFACE, STRAIGHT}
  root_names: upper, lower (defaults "<prefix>_root_upper", "<prefix>_root_lower"; adopt keeps existing)

ColumnGuide
  name: str ("front", "side", "back", "c1".., "back" center)   side: enum {L, R, C}
  angle: float (deg)       top_offset: float (m)     enabled: bool
  open_mult: float 0..1.5  volume_mult: float 0..1.5     rest_flare_mult: float

WeightSettings
  profile_preset: enum {LOOSE_BELOW_WAIST, LOOSE_BELOW_CHEST, BELTED_COAT, JACKET, SKIRT, CAPE, CUSTOM}
  blend_start: float (world height; auto = top_line − 5 cm)   blend_full: float (auto per preset)
  curve: node name in hidden node group ".BoneCloth" (Float Curve)      # "how the blend goes"
  pins: Collection[Pin {name, height, half_width, softness, strength}]   # belts
  exclude_arms: bool (true)  exclude_threshold: float (0.4)
  use_painted_amount: bool   # use group "BC amount" as painted, instead of the profile
  use_exclude_group / use_pin_group: bool
  side_mode: enum {POSITION, POSITION_WITH_OVERLAP_FIX}
  panel_sharpness: float 0..1 (0.5)     joint_softness: float 0..0.5 (0.35)
  rigid_parts: bool (true)   rigid_max_size: float (m, 0.04)
  smooth_iterations: int (4)  smooth_strength: float (0.5)
  clean_below: float (0.01)

RestSettings
  flare: float (m at hem, 0)   open: float (m at hem, 0)   bend: float 0..1
  clear_legs: bool (true)   clearance: float (m, 0.01)
  hand_rotations: Collection[{bone, quat}]          # manual rest edits, re-applied on rebuild when bone names match
  snapshot: Collection[{bone, head, tail, roll}]    # bones as generated (rest A)

PoseSettings
  open:   PoseLayer
  volume: PoseLayer
  preview_open, preview_volume: float 0..1 (UI only, not saved into data)
PoseLayer
  enabled: bool   action: PointerProperty(Action)   clip_name: str ("<prefix>_open")
  mode: enum {GENERATED, HAND}
  amount: float (m at the hem)     direction: float 0..1 (open: 0 outward … 1 sideways; volume: lift 0..1)
  bend: float 0..1 (0 = even arc, 1 = mostly near the hem)   upper_share: float 0..1 (how much the chest part takes part)

ColliderSettings
  bone, to: str   auto_radius: bool  radius: float   auto_end: bool  end_radius: float   enabled: bool
  fitted_radius, fitted_end: float (read-only cache)   origin: enum {AUTO_LEG, USER}

MotionSettings   (every BoneClothData scalar; same names, same defaults as BoneClothData.h)
  followRoot, followTip, trail, maxAngleRoot, maxAngleTip, radius, teleportDistance, tipLength,
  volume, openWindStart, openWindFull, windwardBias, volumeWindStart, volumeWindFull, layerSmoothing,
  spinOpen, spinVolume, spinStart, spinFull, spinRelease
  feel_weight, feel_floaty: float 0..1     # beginner macros (§8.8); last macro values, to show the sliders
  forward_override: bool, forward: vec3

ExportSettings
  targets: Collection[{glb_path: str (file path), write_glb: bool, write_skmm: bool}]
  deform_bones_only: bool                  # §14 Q2
  last_export: {time, glb_hash, skmm_hash, roots_written}
```

Curves ("how the blend goes", "bend") live in one hidden node group `.BoneCloth` (`ShaderNodeTree`), one `ShaderNodeFloatCurve` per
use, named `<setup id>.<use>`. They are drawn with `layout.template_curve_mapping(node, "mapping")` and evaluated with
`mapping.evaluate(mapping.curves[3], x)` after `mapping.initialize()`. This is the standard way to give add-on settings a curve widget.

### 6.5 Modules

```
bone_cloth/
  blender_manifest.toml       id "bone_cloth", version, blender_version_min "5.1.0", permissions: files (export)
  __init__.py                 register / unregister, handlers
  engine_contract.py          constants + pure rules mirrored from BoneCloth.cpp (C2–C10, C13 maths) — no bpy
  sim.py                      preview simulator (port of BoneCloth Build/Step) on top of engine_contract
  frames.py                   body frame, world <-> armature <-> engine conversions, units
  landmarks.py                rig family detection, anchor/leg/arm finding, facing
  mesh_prep.py                checks and fixes, source copy, body-weight transfer
  guides.py                   defaults from mesh measurements, opening detection, column/row layout, ray-cast placement
  bones.py                    bone generation, names, rolls, roots, joins, BC Follow, adopt existing
  weights.py                  cloth amount, around/along blending, merge, limit, smooth, rigid parts, mirror (numpy)
  rest_shape.py               parametric rest rotations, clear-legs solver, LBS bake (shape keys), snapshot/reset
  poses.py                    pose solver, slotted-action writer, hand-edit capture, preview composition
  colliders.py                auto legs, fit (via engine_contract), checks
  doctor.py                   all checks, fixes registry
  export.py                   glTF call, .skmm merge writer, post-export verification
  overlay.py                  GPU draw handlers (guides, ghosts, strips, capsules, weights colors, debug lines)
  gizmos.py                   (phase 3) drag handles for lines, columns, pose amounts
  presets/*.json              built-in presets (§9); user presets in the extension's user directory
  props.py, ui.py, ops_*.py   property groups, panels, operators
  tests/                      engine_contract unit tests (plain Python) + headless Blender tests
```

### 6.6 Blender API notes for the implementer
- **Actions (slotted, 5.1)**: `act = bpy.data.actions.new(name)`; `slot = act.slots.new(id_type='OBJECT', name=arm.name)`;
  `strip = act.layers.new("Layer").strips.new(type='KEYFRAME')`; `cb = strip.channelbag(slot, ensure=True)`;
  `cb.fcurves.new('pose.bones["x"].rotation_quaternion', index=i)`; LINEAR keys at frames 1 and 2; `act.use_frame_range = True`,
  `frame_start=1`, `frame_end=2`; `use_fake_user=True`. Do not touch the legacy `action.fcurves`.
- **Edit bones** need Edit mode: `with bpy.context.temp_override(active_object=arm, …): bpy.ops.object.mode_set(mode='EDIT')`.
  Restore the user's mode afterwards. Use `EditBone.align_roll(vector)` for roll and `EditBone.matrix = M` to set from a pose matrix.
- **Weights in bulk**: read with `bmesh` deform layer (`bm.verts.layers.deform.verify()`), compute in numpy, write back the same way;
  `VertexGroup.add` per vertex is too slow above about 5 k vertices.
- **Ray casts**: `mathutils.bvhtree.BVHTree.FromPolygons` on the **source** mesh's undeformed coordinates (armature space), not on
  the evaluated mesh, which is posed.
- **Overlays**: `SpaceView3D.draw_handler_add(..., 'WINDOW', 'POST_VIEW')`, `gpu.shader.from_builtin('POLYLINE_UNIFORM_COLOR')`,
  `batch_for_shader`. Never use a color attribute for previews: the exporter exports the active color attribute
  (`export_vertex_color='ACTIVE'`).
- **Handlers**: `@persistent` `frame_change_post` for the preview simulator; `depsgraph_update_post` is not used (feedback loops).
- **Non-bone vertex groups** (`BC amount`) are ignored by the glTF exporter's skinning, so they are safe to keep.
- **Timers** (`bpy.app.timers`) debounce slider-driven overlay recomputes (0.15 s).

---

## 7. UI overview

Sidebar (N) tab **Bone Cloth**, in Object, Pose, Edit and Weight Paint modes. It follows the active object: an armature, or a mesh
skinned to one.

```
┌ Bone Cloth ───────────────────────────────────────┐
│ Setup [ coat        ▾][+][−]    Preset: Long coat  │
│ ● Ready to export · 2 warnings                     │
│ [  Auto Setup  ]   [ Update all (2 stale) ]        │
│ Next: Wind poses look hand-edited, check them  [Go]│
├────────────────────────────────────────────────────┤
│ ✓ 1  Character                                   ▸ │
│ ✓ 2  Mesh                                        ▸ │
│ ✓ 3  Shape & bones                               ▸ │
│ ⟳ 4  Weights        (shape changed)              ▸ │
│ ✓ 5  Rest shape                                  ▸ │
│ ✎ 6  Wind poses     (open: hand-edited)          ▸ │
│ ✓ 7  Collisions                                  ▸ │
│ ✓ 8  Motion                                      ▸ │
│   9  Preview                                     ▸ │
│ ! 10 Check & export (2 warnings)                 ▸ │
└────────────────────────────────────────────────────┘
```

- Status icons: ✓ done and up to date · ⟳ stale (Update) · ✎ hand-edited · ! warnings · ✗ blocking error · blank = not run.
- Each step is a sub-panel. Open it to get its settings, its main button, and a **?** that pops up a three-sentence explanation of what
  the step is for, in plain words.
- The **Next** line is the assistant: the first not-done or stale step, or the first check item, phrased as an instruction, with a
  **Go** button that opens that panel and frames the relevant bones in the viewport.
- Viewport header toggle **Bone Cloth overlays**, with a popover per overlay: guides, ghost bones, strip colors, capsules, wind pose
  ghosts, problem vertices, preview debug lines.
- Every slider's tooltip: what it does in one sentence, what to try if something looks wrong, then the engine field name.
- Error messages say what to do: "Panel `coat_side_02_l` has no vertices on it, so the game will cut the panel there. Press
  **Build weights**, or move the bone onto the coat."

---

## 8. Steps in detail

### 8.1 Step 1: Character

**Create setup** (shown when the active armature has none): a popup asking "What are you setting up?" with large preset buttons (Long
coat · Jacket · Skirt · Robe · Cape · Strand · Custom) and "Which mesh is the cloth?" (a dropdown of meshes skinned to or parented to the
armature, preselected from the selection or by name: coat, jacket, skirt, cape, cloak, robe, dress, tail, scarf). OK runs detection.

**Detection** (`landmarks.py`):
1. **Rig family** by name patterns: UE (`pelvis`, `spine_0N`, `thigh_l`, `calf_l`, `foot_l`, `ball_l`, `clavicle_l`), Mixamo
   (`mixamorig:Hips`, `Spine2`, `LeftUpLeg`, `LeftLeg`, `LeftFoot`, `LeftToeBase`), Rigify (`DEF-spine.00N`, `DEF-thigh.L`, `DEF-shin.L`,
   `DEF-foot.L`, `DEF-toe.L`), Blender metarig (`spine`, `thigh.L`, `shin.L`), generic.
2. **Structural fallback**, which wins when names are ambiguous:
   - hips = the bone whose children contain a left/right pair that each lead into a chain of 3 or more going down;
   - chest = the highest spine bone whose children contain a left/right pair leading sideways (clavicles or upper arms);
   - thigh/calf/foot = along each leg chain, skipping twist bones (name contains "twist", or a childless bone sharing its parent with a
     longer chain);
   - ball = the foot's child.
3. **Facing**: horizontal sum of `ball − foot` per side; fallback −Y. An arrow is drawn on the floor, with a **Flip** button.
4. **Body frame** (§5) cached.

Panel: a table of landmarks with ✓ or ✗ and a `prop_search` dropdown for each. Missing ones are red, with "Pick the bone the hips/legs
start at". The cloth objects list has +/− (outer shell, lining, buttons can be separate objects).

### 8.2 Step 2: Mesh preparation

A checklist; every row has a **Fix** button, and **Fix all** runs the safe ones. "Safe" means geometry looks the same afterwards.

| Check | Fix | Safe? |
|-------|-----|-------|
| Cloth mesh is not skinned (no armature modifier or no weights) | **Give body weights**: transfer from a body mesh (Data Transfer, nearest face interpolated, all groups by name) or, if there is none, Blender's automatic heat weights. Transfer is recommended because heat weights fail on clothing | asks which body mesh |
| Mesh transform ≠ armature's (here: 90° X, scale ×100) | **Bake transform into mesh**: `data.transform(armature_world⁻¹ · mesh_world)`, then set parent to the armature with identity local and parent inverse. Shape keys and custom normals follow (`Mesh.transform` handles both). Negative determinant → flip normals | yes |
| Armature modifier missing, duplicated, pointing elsewhere, not first, or using vertex-group / multi-modifier options | Make one, first, pointing at the setup armature | yes |
| **Preserve Volume** on | Turn off: the engine skins linearly, so Blender would show a different deformation | yes |
| Vertices with > 4 influences | Done in Build weights (counted here) | — |
| Unnormalized weights / vertices without weights | Normalize; give weightless vertices the weights of the nearest weighted vertex (graph distance) | yes |
| Empty groups, or groups for bones that don't exist | Remove | yes |
| Loose parts (buttons, pockets, buckles) | Info: "3 small parts will move rigidly with the cloth" (§8.4.6) | — |
| Mirror symmetry (≥ 95 % of vertices have a mirror within 1 mm) | Info only; enables Mirror tools | — |
| Hidden or excluded meshes skinned to this armature | Info: "`coat` is hidden and won't be exported" | — |
| Other meshes have weights on cloth bones (left over from an older rig) | Lists them; **Remove cloth weights** | asks |
| Scene unit scale ≠ 1, armature scale non-uniform | Explains; no automatic fix (an armature scale fix would rescale every action) | — |

After **Fix all**, the add-on makes the source copy (§6.2) if it does not exist.

### 8.3 Step 3: Shape and bones

#### 8.3.1 What the beginner sees

```
Layout     [ Open front ▾]   Anchors [ Chest + hips ▾]
Panels per side   [ 3 ]      ☐ Centre back panel   ☐ Back vent
Front opening     [ 80° ]  (detected)
Spread            [ ——●—— ]
Rows below waist  [ 3 ]      ☑ Upper part (chest → waist)
Top line   [ 1.40 m ] ☑ auto      Waist [ 1.08 m ] ☑ auto     Hem [ 0.52 m ] ☑ auto
Place bones [ on outer surface ▾]  offset [ 0 cm ]   Path [ follow surface ▾]
Row spacing [ even ▾]
┌ Columns ─────────────────────────────────────────┐
│ name    side angle  top     open  volume          │
│ front   L/R  51°    −2 cm   100%  100%            │
│ side    L/R  110°   −10 cm   40%  100%            │
│ back    L/R  173°   +2 cm     0%   80%            │
└──────────────────────────────────────────────────┘
[ Build bones ]      after building: [Edit bones by hand] [Snap to surface] [Even out] [Symmetrize]
```

While the panel is open, the overlay draws: the top/waist/hem lines as rings fitted to the mesh cross-section at that height, the
opening wedge (shaded), column rays (blue left, red right, green centre), and ghost bones (polylines with joint dots) recomputed live
(debounced) from the sliders. **Build bones** commits.

#### 8.3.2 Measuring the mesh (defaults)

All on the source mesh in armature space, using the body frame:
- **Hem** = lowest cloth vertex height + 1 cm.
- **Opening** (OPEN_FRONT): take vertices between the waist and the hem, histogram their θ in 5° bins, and find the largest empty run
  containing 0°. Half-angle = half its width plus 5°. If no empty run contains 0°, the front is closed → suggest CLOSED_RING (jacket
  zipped, skirt, robe).
- **Back-only** detection (cape): if the cloth covers only |θ| > 90° → suggest BACK_ONLY.
- **Top line** = chest anchor head height (cape: clavicle head). **Waist** = head of the hips anchor's first spine child (`spine_01`:
  1.076 m → shown as 1.08), or the centre of a detected belt band (§8.4.3).
- **Per-column top**: from the top line, step down 1 cm at a time along the column's ray. The top is the first height where the ray
  hits the cloth and the hit vertex is not arm-dominated (original weights). This starts side panels below the armpit automatically
  (`player_coat`: side tops 1.30 m, 10 cm below the 1.40 m top line; front 1.38 m, back 1.42 m).
- `player_coat`'s skirt rows start at 1.10 m rather than the auto 1.08 m: its bones were placed on the belt. With a belt pin
  detected (§8.4.3), the waist line auto-snaps to the pin's lower edge.

#### 8.3.3 Column angles
- OPEN_FRONT, n per side: the left side's free arc is `[opening_half, 180°]` (or `[opening_half, 180° − vent_half]` with a vent).
  Columns sit at `θ_i = lerp(arc_start + m, arc_end − m', (i + spread)/(n − 1 + 2·spread))` with small margins so the first panel sits
  just beside the opening and the last near the back centre. Right side is the mirror (−θ). Centre back adds θ = 180° (side C).
  The `player_coat` result for n = 3 is about 51°, 110°, 173°.
- CLOSED_RING: n per side plus optional centre front (0°) and centre back (180°), evenly around 360°.
- BACK_ONLY: n per side over `[180° − spread_arc, 180°]`, plus optional centre.
- Columns can be edited in the table afterwards. Editing one side mirrors to the other while **Symmetric** is on (default).

#### 8.3.4 Bone positions
For each column and each target height (top, waist, then `rows` heights between waist and hem by `row_spacing`):
1. Ray from the body axis at that height, direction `cos θ·F + sin θ·L`, against a BVH of the source mesh.
2. Hits along the ray: **outer** = last hit, **inner** = first, **middle** = their average (cloth thickness, linings). Add
   `surface_offset` along the ray.
3. No hit (gap, or the opening edge): use the nearest cloth vertex within ±10° of θ at that height; if none, interpolate from the rows
   above and below; if still none, the column is reported "doesn't reach this height" and that bone is skipped. A column must keep at
   least one bone below the waist.
4. **Path = STRAIGHT**: the waist-to-hem points are replaced by a straight line from the waist point to the hem point (keeps
   flare-shaped coats from getting wavy bones).
5. **Hem point** = the ray hit at the hem height, then moved down to the lowest cloth vertex within the column's ±(half spacing)
   sector. The engine tip is measured from the vertices anyway (C6); this only sets the last Blender tail.

Rows by `row_spacing`: EVEN = equal arc length between the waist point and the hem point; DENSER_AT_HEM / TOP = arc-length parameter
`t^(1±bias)`.

#### 8.3.5 Bones, names, hierarchy
- **Roots**: `<prefix>_root_upper` (child of the chest anchor; head on the body axis at the top line, 10 cm long pointing down, deform
  **on**, no weights) and `<prefix>_root_lower` (child of the hips anchor, at the waist line). HIPS-only and CHEST-only layouts make one
  root.
- **Strip bones**: `<prefix>_<column>_<NN>_<l|r>` (centre columns: no suffix). `00` is the upper part (only with CHEST_AND_HIPS and
  `upper_part`); `01..` are the rows below the waist. Head = point k, tail = point k+1 (connected), last tail = hem point. Deform on,
  rotation mode QUATERNION, `use_inherit_rotation` on, `inherit_scale` FULL.
- **Parents**: `00` → upper root; `01` → lower root (not to `00`, so the skirt turns with the hips, §3.1); `02..` → previous bone,
  connected. Joined chains: `01`'s head is exactly `00`'s tail, so in `00`'s frame it lies on +Y with zero sideways offset (C4).
- **Roll**: `align_roll(outward)` with outward = the column's radial direction, so local Z points away from the body and local X runs
  along the coat. Right-side rolls come out mirrored, which keeps Blender's pose mirror and Symmetrize working.
- **BC Follow**: a `COPY_LOCATION` constraint on every joined `01` bone, targeting `00` with head/tail = 1 (world space). In Blender this
  keeps the skirt attached to the chest part when the spine bends in an animation or a hand pose. The engine ignores it: animation on
  strip bones is not used (C12), and the swing takes `01`'s head from `00`'s tail anyway.
- **Bone collections**: `Cloth`, `Cloth Roots`. A per-setup collection is added only when there are several setups.

#### 8.3.6 STRAND layout (tail, scarf, flap)
The panel asks for **Anchor bone** and **Path**: either two picked vertices (start and end; the path follows the mesh's centre line
between them, found by slicing the mesh perpendicular to the start→end line every `length/(rows·4)` and taking the slice centroids),
or the active edge loop sequence. Several strands = several setups on the same anchor, or a "Strands: N" option that splits the mesh
by loose parts or by angle around the anchor.

#### 8.3.7 Hand tools after building
- **Edit bones by hand**: Edit mode with only `Cloth` and `Cloth Roots` visible and X-axis mirror on. On exit, the add-on re-derives
  everything from the bones (P1); guides show "bones hand-edited".
- **Snap to surface**: re-projects selected bones' joints along their column ray, keeping heights.
- **Even out**: equal lengths along each selected chain, keeping the end points.
- **Symmetrize**: Blender's armature symmetrize restricted to cloth bones (works with `_l/_r`).
- **Adopt existing rig** (phase 2): pick root bones (or read them from an existing `.skmm` entry). The add-on discovers chains with the
  engine's rules (C2–C4), maps them to columns by angle and names (`front/side/back`), fills Guides as "adopted" (read-only summary),
  and imports Motion, Colliders and clip names from the `.skmm`.

### 8.4 Step 4: Weights

The weights are built from the **bones as they are** (generated or adopted), plus a cloth-amount profile. So the same tool works on
hand-placed bones.

#### 8.4.1 What the beginner sees
```
Where does the coat leave the body?   [ Belted coat ▾ ]
  Blend starts at [ 1.30 m ]   Fully loose at [ 0.95 m ]
  How it blends   [ curve widget: ease in/out ]
  Belts / pins    + Belt   1.12 m  ± 2.5 cm  soft 1 cm   [×]
☑ Keep sleeves on the arms        ☐ Use my painted cloth amount
Panels blend   soft ——●—— sharp    Joints blend  [ 0.35 ]
☑ Small parts move rigidly (buttons) up to [ 4 cm ]
Smoothing [ 4 ] × [ 0.5 ]
[ Show cloth amount ] [ Paint cloth amount ] [ Build weights ]
[ Show panel colors ] [ Smooth selected ] [ Mirror L → R ] [ Assign selected to panel ▾ ]
```

**Show cloth amount** writes the computed amount into the `BC amount` group and switches to Weight Paint with that group active (red =
moves with the panels, blue = stays on the body). **Paint cloth amount** does the same and ticks *Use my painted cloth amount*, so the
beginner can paint where the coat should stay or float with ordinary brushes. **Build weights** writes the final weights.

#### 8.4.2 Cloth amount `c(v)` ∈ [0, 1]
For each vertex at height `h` (body frame), from the source mesh:
1. **Profile**: `c = curve(clamp((blend_start − h)/(blend_start − blend_full), 0, 1))`. Most presets use a smoothstep. BELTED_COAT
   uses blend 1.32 m → 0.95 m with curve points (0, 0) (0.54, 0.40) (0.68, 0.45) (1, 1): the chest rises to about 0.4 at the belt,
   holds just below it, then reaches 1.0 at 0.95 m. With the belt pinned, that is close to the shares measured on `player_coat`
   (by 5 cm band: 0.11, 0.24, 0.37 on the chest; 0 on the belt; 0.40 at 1.05–1.10 m; 1.0 below 0.95 m).
2. **Pins (belts)**: for each pin, `p = strength·(1 − smoothstep(half_width, half_width + softness, |h − pin.height|))`; then
   `c ← c·(1 − p)`.
3. **Exclusions**: `a = Σ` original weights on `exclude_bones` (arms, neck, head chains);
   `c ← c·(1 − smoothstep(0.5·exclude_threshold, exclude_threshold, a))`. Sleeves stay on the arms even where they hang below the blend
   line.
4. **Groups**: `BC exclude` multiplies by `(1 − w)`; `BC pin` multiplies by `(1 − w)`.
5. **Painted**: with *Use my painted cloth amount*, steps 1–4 are skipped and `c = BC amount`.

#### 8.4.3 Belt detection (preset BELTED_COAT and "Detect belt")
A belt is a band of vertices whose original weights are almost all on one torso bone and where vertex density per cm of height is
above 2× the median, within ±10 cm of the waist line. It is offered as a pin and never applied silently.

#### 8.4.4 Around (between panels): `U_s(v)`
For ring layouts, each strip `s` has an angle at every height, `θ_s(h)`: interpolated along its joint polyline (heads plus the engine
tip; above the top → top angle, below the tip → tip angle). So slanted panels (front panels flaring outward) blend correctly.

1. **Side** σ(v) ∈ {L, R, C}:
   - POSITION: L if θ > 0, R if θ < 0.
   - POSITION_WITH_OVERLAP_FIX (default for OPEN_FRONT): vertices in the front sector `|θ| < opening_half + 20°` take the side of their
     connected component *within that sector* when that component touches only one side's non-front region. This handles overlapping or
     double-breasted flaps whose left flap crosses to x < 0. Components touching both sides (a closed front) fall back to position.
2. **Neighbours**: among the strips of side σ (plus centre strips, plus the other side's back strip when the back is not vented), take
   the two whose `θ_s(h)` bracket θ. Toward the opening, or beyond the outermost strip: 100 % to the outermost strip. Across the back
   centre (no vent): wrap through 180°.
3. **Blend**: `t = (θ − θ_a)/(θ_b − θ_a)`; sharpened by `k = panel_sharpness`: `t' = smoothstep(0, 1, clamp((t − ½)/(1 − k) + ½, 0, 1))`;
   `U_a = 1 − t'`, `U_b = t'`. k = 0 gives a soft linear blend, k → 1 gives hard panel edges.
4. CLOSED_RING wraps 360° with no opening. STRAND: `U = 1` for the nearest strand, or a distance-ratio blend between the two nearest
   when strands are close.

Because `U` and `V` depend on angle and height only, the inner and outer shells, the lining and the thickness all get identical weights.

#### 8.4.5 Along (between a panel's bones): `V_{s,k}(v)`
- Parameter `σ ∈ [0, n]` in bone units: find the segment whose head heights bracket `h` (the last segment runs to the engine tip),
  `σ = k + t`. Above the top → 0; below the tip → n. (STRAND: closest point on the polyline in 3D instead of height.)
- Joint softness `b ∈ [0, 0.5]`: `β_j(σ) = smoothstep(0, 1, (σ − (j − b))/(2b))` for joints `j = 1..n−1` (b = 0 → a step).
  `V_0 = 1 − β_1`, `V_k = β_k − β_{k+1}`, `V_{n−1} = β_{n−1}`. These sum to 1. The last bone reaches 1.0 below `n − 1 + b`, so the hem
  satisfies C6 (≥ 0.25), and every bone gets vertices ≥ 0.05 (C2) unless no vertex lies at its height.

#### 8.4.6 Merge, rigid parts, limit, smooth
For every vertex:
1. `c = 0` → keep the original body weights exactly, minus any weights on cloth bones.
2. `W_cloth[bone(s, k)] = Σ U_s · V_{s,k}`; `W_body` = original weights (cloth bones removed), normalised.
3. `W = (1 − c)·W_body + c·W_cloth`.
4. **Rigid parts** (loose parts smaller than `rigid_max_size`): every vertex of the part takes the final weights of the main-mesh
   vertex nearest to the part's centroid. Buttons don't bend.
5. **Smooth** `smooth_iterations` times on vertices with `c > 0` or a neighbour with `c > 0`:
   `W_v ← (1 − λ)W_v + λ·mean(W_neighbours)`, skipping edges whose ends have different side labels in the front sector (no smoothing
   across the opening) and keeping `c = 0` vertices fixed.
6. **Limit to 4**: keep the 4 largest weights. If the vertex has `c ≥ 0.5`, prefer cloth bones: always keep the two largest cloth
   weights. Drop weights below `clean_below`, then renormalise. This is exactly what gets exported, because the exporter's own 4-limit
   then has nothing to trim.
7. Write all groups via bmesh; remove cloth-bone weights the vertex no longer has.

After building, a summary line: "1,074 vertices · 412 move with the panels · 79 blend between panels · 0 over 4 influences · every
panel bone carries vertices (min 6 at ≥ 0.05) · tips: front 0.53 m …".

#### 8.4.7 Inspection and hand tools
- **Show panel colors**: overlay colouring each vertex by the blend of its panels' colours (GPU, nothing written). Joint bands are
  shown as darker stripes.
- **Show problems**: red points for vertices that are over 4 influences, unnormalised or weightless, and for bones whose vertices are
  under the 0.05 / 0.25 thresholds.
- **Assign selected to panel**: in Edit/Weight mode, selected vertices take 100 % of a chosen panel (keeping their `V` split along it).
- **Smooth selected** (same smoother, selection only), **Mirror L → R** (nearest mirrored vertex within 1 mm or topology mirror, with
  `_l ↔ _r` name flip), **Transfer from another coat** (phase 3: Data Transfer from a reference mesh's cloth groups, then remap bone
  names).
- Hand painting in Weight Paint is always allowed; the stage is then marked ✎ (§6.1).

### 8.5 Step 5: Rest shape ("default transforms")

The engine swings every panel back to the rest shape (C12), so the rest pose is the coat's look at rest. Typical adjustments are a
little flare so the hem clears the legs, a slightly open front, or straighter panels.

#### 8.5.1 What the beginner sees
```
How should the coat hang when nothing moves?
  Flare at hem       [ 2 cm ]     Open front at rest [ 0 cm ]    Bend [ ——●—— ]
  ☑ Keep clear of the legs   margin [ 1 cm ]         (front_l: +3.1 cm, side_l: +0 cm …)
[ Preview ]  [ Edit by hand ]  [ Apply ]  [ Reset to original ]
```
**Preview** poses the bones (not applied) so the mesh shows the result. **Edit by hand** enters Pose mode on cloth bones with X-mirror.
**Apply** bakes. **Reset to original** restores the generated rest A from the snapshot and the mesh from the source.

#### 8.5.2 Parametric rest rotations
The same solver as the wind poses (§8.6.3), with direction = radial (flare) or sideways-away-from-the-opening (open) and amount = the
target hem displacement times the column's `rest_flare_mult`. Hand rotations (`hand_rotations`, captured from **Edit by hand**) are
applied on top, per bone, as local rotations.

#### 8.5.3 Keep clear of the legs
For each strip, with the leg colliders at rest (§8.7) and `need = r(t) + motion.radius + clearance`: if any joint tail or the engine tip
is closer than `need` to a leg capsule axis, rotate the whole strip outward (radial axis, distributed by `bend`) by the smallest angle
that clears it, found by bisection over 0–30°. The per-strip result is reported. Reason: a tip that rests inside a capsule is pushed
out by the engine every frame, so the coat sits differently from the authored shape and the mesh clips the legs in Blender.

#### 8.5.4 Apply (LBS bake)
1. Pose all strip bones with the rest rotations; `BC Follow` moves the joined `01` heads.
2. For each object with weights on any strip bone (cloth objects, plus any other mesh found in §8.2), compute per vertex
   `M_v = Σ_i w_i · (P_i · R_i⁻¹)` in armature space (P = posed `pb.matrix`, R = `bone.matrix_local`; non-cloth bones contribute the
   identity) and set `p' = M_v · p`, for the base mesh **and every shape key** (keys are deformed with the same `M_v`, so their relative
   offsets survive). Custom normals are stored relative to the surface and need nothing.
3. Edit mode: for each strip bone `EditBone.matrix = P_i` (head, roll, direction), keeping each bone's length; `01` heads land on `00`
   tails.
4. Clear the cloth bones' pose bases.
5. Downstream stages become stale: wind poses (GENERATED → regenerated by Update; HAND → offer "keep their shapes in world space": new
   basis = `newRestLocal⁻¹ · oldRestLocal · oldBasis`), colliders (unchanged), check.
6. Other actions with keys on cloth bones: info "their meaning shifted; the game ignores them" plus **Remove cloth keys from other
   actions**.

Rebuild order (Update all): bones from guides (rest A) → weights on the source → rest rotations (parametric + hand) → bake. Hand rest
rotations survive rebuilds as long as the bone names exist; dropped ones are listed.

### 8.6 Step 6: Wind poses (drive poses)

#### 8.6.1 What they are, in the panel's words
- **Open**: the shape the coat takes in a full headwind, or a fast spin: front panels swing out to the sides like doors. The windward
  side opens more in game.
- **Volume**: the shape at full wind speed or a fast spin: every panel flares out. Part of it can be always on (`volume`).
- The game blends from the rest shape toward each pose (never beyond it) and swings around the result.

#### 8.6.2 What the beginner sees
```
Open      ☑   clip [ coat_open   ]   mode: generated
  Hem moves      [ 24 cm ]
  Direction      outward ——●—— sideways
  Bend           even arc ——●—— mostly at the hem
  Chest part     [ 50 % ]       (columns: front 100 %, side 40 %, back 0 %  → edit in Shape)
Volume    ☑   clip [ coat_volume ]   mode: generated
  Hem flares     [ 11 cm ]   Lift [ 0 ]   Bend ——●——   Chest part [ 30 % ]
Preview   Open [——●——]  Volume [——●——]      Wind from [ dial ]  speed [ 8 m/s ]  ☐ use wind
[ Update poses ]  [ Edit open by hand ]  [ Edit volume by hand ]  [ Mirror L → R ]  [ Back to generated ]
```
Ghost hems for both poses (cyan and magenta) are drawn while the panel is open.

#### 8.6.3 Pose solver (generated mode)
For each strip with column multiplier `m > 0`:
```
θ        = strip top angle;  side = ±1 (or 0)
R(θ)     = cosθ·F + sinθ·L                     # radial out
T_away   = side·(−sinθ·F + cosθ·L)             # along the ring, away from the front opening
D        = normalize( open:   lerp(R, T_away, direction)
                      volume: R + direction·up )            # 'lift'
S        = normalize(hem − top)                 # panel direction at rest
axis     = normalize(S × D)                     # turns S toward D
shares_k = w_k / Σw,  w_k = ((k+1)/n)^(3·bend), upper part's w multiplied by upper_share
target   = amount · m                           (world m → armature units)
α        = bisection on [0, 85°]: |hem(α) − hem| = target, where
           FK: R_k = Rot(axis, α·Σ_{j≤k} shares_j);  head'_{k+1} = head'_k + R_k·(head_{k+1} − head_k);  hem' via the last R
M_k      = desired armature-space matrix: R_k applied to the rest matrix of bone k, translated to head'_k
basis_k  = (P_parent · R_parent⁻¹ · R_k_rest)⁻¹ · M_k   # with the skeleton parent (roots stay at rest), so 01's basis
                                                       # carries 00's turn, as the engine needs (§3.1)
```
- Keys: frame 1 identity, frame 2 `basis_k` rotation (quaternion), on strip bones only. Strips with `m = 0` get no keys, so the
  engine's 0.5° test (C10) leaves them out of that layer.
- An unreachable target (hem distance more than the panel can swing at 85°) is clamped and reported.
- Check: the posed hem against the leg capsules at rest → warning "the open pose pushes the front hem into the left thigh by 2 cm".

#### 8.6.4 Hand edit
**Edit open by hand** shows the pose at full weight in Pose mode with only cloth bones selectable and X-mirror on. A banner reads
"Editing OPEN pose — [Save] [Cancel]". **Save** captures each strip bone's basis rotation into frame 2 and switches the mode to HAND; the
amount and direction sliders turn into a **Scale** slider (slerps every bone's rotation, 0–150 %). **Back to generated** discards hand
edits after confirming. **Capture from current pose** turns any evaluated pose (for example a legacy driver rig at frame 2) into a HAND
pose, which is how `player_coat`'s existing clips are kept (§10).

#### 8.6.5 Preview composition (matches the engine)
The engine applies the volume layer, then the open layer, as local turns: `local ← turn_open · turn_vol · local`, with `turn =`
slerp(identity, delta, w) and w clamped to 0..1. In Blender basis terms the previewed basis per strip bone is
`slerp(I, open_basis, w_open) · slerp(I, vol_basis, w_vol)`. With *use wind* on, `w_open` per strip comes from the engine's formula
(headwind + `windwardBias`·windward side, ramped from `openWindStart` to `openWindFull`) and `w_vol = volume + ramp(volumeWindStart,
volumeWindFull, speed)`. The preview writes temporary pose bases and restores them when the slider panel closes. It never keys.

#### 8.6.6 Clips
Names default to `<prefix>_open` / `<prefix>_volume` (`coat_open` / `coat_volume` for adopted coats). They are written to
`openClip` / `volumeClip`. Disabling a layer writes an empty string and leaves the action alone.

### 8.7 Step 7: Collisions

- **Auto legs** (the default for coats, jackets, skirts, robes): `thigh → calf` and `calf → foot` per side, from the landmarks. Capes
  add a torso capsule `spine_01 → neck_01` with a **manual** radius (auto-fit would count the cape's own chest vertices, C9).
- **Fit**: `engine_contract.fit_collider()` on the vertices of every mesh that will be exported, at rest, using each vertex's dominant
  bone after the 4-limit. Same "on the way" and leaf rules, same t ≤ 0.3 / t ≥ 0.7 split, medians, fallbacks. Shown as "auto
  (0.089 m)" next to each end; tick off *auto* to type a radius.
- **Draw**: capsule wireframe in orange (the colour `cloth_debug` uses) at the radius, dashed at radius + padding (what the bone tips
  actually keep away from).
- **Add collider**: two bone pickers. A warning appears when `to` is not a descendant of `bone` and auto is on (the engine falls back to
  0.06 m).
- **Padding** (`radius`) lives here, not in Motion: beginners look for it next to the capsules.

### 8.8 Step 8: Motion settings

#### 8.8.1 Beginner view
```
Feel       [ Heavy wool ▾ ]   (Leather · Heavy wool · Cotton · Light silk · Custom)
  Fabric weight   heavy ——●—— light
  Response        snappy ——●—— floaty
Wind opens the coat from [ 1 m/s ] fully at [ 13 m/s ]
Wind fills the coat from [ 1 m/s ] fully at [ 15 m/s ]   Always-on volume [ 50 % ]
Spinning opens [ 80 % ]  fills [ 60 % ]  from [ 180 °/s ] to [ 720 °/s ]   settles in [ 0.5 s ]
▸ Advanced (every BoneClothData field, exact engine names)
```
- **Fabric weight** w (0 heavy … 1 light) drives `followTip = lerp(10, 3, w)`, `maxAngleTip = lerp(35, 75, w)`,
  `trail = lerp(0.3, 0.7, w)`.
- **Response** r (0 snappy … 1 floaty) drives `followRoot = lerp(90, 40, r)` and `layerSmoothing = lerp(0.1, 0.4, r)`. Both macro
  mappings are starting points to tune against the preview.
- Moving a macro writes the fields; editing a field directly sets Feel to *Custom*.
- Under Advanced, a small table shows per joint of the active strip: follow (1/s) and max angle (°), interpolated by index (C7), and the
  engine tip length (C6). `forward` is shown read-only (from Character facing) unless overridden.
- "Spin is read from: pelvis, spine_01, spine_02, spine_03" (C14).

### 8.9 Step 9: Preview (engine port in the viewport)

#### 8.9.1 What the beginner sees
```
[ ▶ Play with cloth ]  [ ■ Stop ]  [ Reset ]
Test motion [ Run 5 m/s ▾ ]   (Animation only · Walk 1.5 m/s · Run 5 m/s · Run in a circle · Spin 720°/s ·
                               Stop and go · Strafe · Teleport test)
Animation   [ run ▾ ]  (actions of the armature)
Wind  ☐ on   from [ dial ]  [ 6 m/s ]  gusts [ 30 % ] at [ 0.4 Hz ]
☑ Draw engine debug lines (yellow strips, orange capsules)
```

#### 8.9.2 How it works
- `sim.BoneClothSim.build(setup)` runs `engine_contract` on the current Blender data. Strips, joins, tips, sides, colliders and layer
  usage come from the bones, the limited weights and the two actions' frame-1/frame-2 bases, exactly as `BoneCloth::Build` would
  derive them from the exported model.
- `step(dt)` ports `UpdateWindWeights`, `SpinRate`, `RestShape`, `ComputeModelSpace`, `Swing` and `WriteBones`:
  - It works in **Blender world space** with world up = +Z. The algorithm's only axis-dependent pieces are the spin rate (yaw about
    up), the wind and the model-space `forward`/`left`, which are converted with the body frame. Lengths are world metres, so
    `radius`, `teleportDistance` and `tipLength` apply unchanged.
  - Body bones come from Blender's evaluated pose after the frame change (`pb.matrix` of non-cloth bones). Roots and strip bones are
    overridden to rest, as the engine does.
  - `world` = armature `matrix_world` × the test-motion transform (a generated translation and yaw over time; "Animation only" uses the
    object's own animation, root motion included).
  - Wind: `WindField::Sample` ported (base velocity, gust phase travelling with the wind, two sines 0.65/0.35 with factor 2.37 and
    phase 1.3).
  - `dt = Δframe / fps`, clamped to 1/15 s; a backward jump or a jump of more than 2 frames resets (snap), like `Reset()`.
- Output: strip bone matrices → pose bases, parents first. They are written in `frame_change_post`, so Blender's armature modifier
  deforms the mesh with the engine's result. Nothing is keyed. **Stop** restores the bases captured at **Play**.
- Debug lines are drawn like `cloth_debug`: head→tail yellow, capsules orange.
- Parity: `engine_contract.ENGINE_COMMIT = "bdacf806f"`. The panel shows "Preview matches engine cloth of 2026-10-22". When the engine
  file changes, the parity tests (§11) are re-run and the constant updated.

Optional (phase 3): **Bake preview to action** for renders and cutscenes in Blender only (game clips must not carry it; the engine
ignores strip keys anyway).

### 8.10 Step 10a: Check (the doctor)

Runs automatically before export, and on demand. Each item has a severity, a message in plain words, the bones or vertices it concerns
(**Show** frames them), and a **Fix** when one exists. Errors block export; warnings don't.

| ID | Sev. | Condition | Fix |
|----|------|-----------|-----|
| ROOT-MISSING | E | A root named in the setup is not a bone | Rebuild bones |
| ROOT-NONDEFORM | E | Root or strip bone is not deform while *deform bones only* export is on | Set deform |
| NAME-CLASH | E | A cloth bone name equals another bone's or an object's name (C16) | Rename with suffix |
| NO-STRIPS | E | No child of the roots carries vertices ≥ 0.05 (C2) | Build weights |
| STRIP-EMPTY | E | A panel bone carries no vertex ≥ 0.05, so the engine truncates the strip there (C3) | Build weights / remove bone |
| STRIP-BRANCH | W | A strip bone has two children with vertices; the engine follows the one with more (C3) | Show |
| JOIN-NEAR-MISS | W | An upper chain's end points at a lower top with a sideways offset of 5–20 % of the distance: it will *not* join (C4) | Snap lower top onto upper tail |
| JOIN-UNINTENDED | W | Two chains join that the setup didn't generate as joined | Show |
| BONE-ZERO | E | Bone shorter than 1 mm | Rebuild |
| TIP-SHORT | W | Last bone has no vertex ≥ 0.25, so the engine tip falls back to the previous segment (C6) | Build weights |
| SIDE-AMBIGUOUS | W | A strip intended as L/R has its top within 3 cm of the centre, near the 2 cm tolerance (C8) | Show |
| INFLUENCES | W | Vertices with > 4 influences; the exporter would drop the smallest (C15) | Limit to 4 |
| UNNORMALIZED / WEIGHTLESS | W | — | Normalize / fill from neighbours |
| PRESERVE-VOLUME | W | Armature modifier preserve volume on (engine is linear) | Turn off |
| MESH-TRANSFORM | W | Cloth mesh transform ≠ armature's | Bake transform |
| JOINTS-256 | E | Exported bones > 256 (C15) | Deform bones only / remove helpers |
| CLIP-MISSING | W | `openClip`/`volumeClip` names an action that doesn't exist or won't export | Update poses |
| CLIP-FLAT | W | No strip bone turns ≥ 0.5° in a clip, so that layer is off in game (C10) | Update poses |
| CLIP-NOT-TWO-FRAMES | I | Clip has keys between its first and last frame (the engine reads only the ends) | — |
| CLIP-OTHER-BONES | I | Clip changes non-cloth bones (ignored by the cloth; harmless) | Remove those keys |
| COLLIDER-BONE | E | Collider names a missing bone | Pick |
| COLLIDER-NOT-DESCENDANT | W | Auto radius with `to` not below `bone`, so the engine uses 0.06 m (C9) | Set radius |
| COLLIDER-ODD-RADIUS | W | Fitted radius < 2 cm or > 30 cm | Set radius |
| REST-IN-LEGS | W | Rest tips inside capsule + padding | Keep clear of the legs |
| POSE-IN-LEGS | I | Wind pose hem inside a capsule at rest | Show |
| FACING | W | Front columns are not on the `forward` side of the body | Flip facing |
| CLOTH-KEYS | I | Other actions key cloth bones or roots (ignored in game, C12) | Remove cloth keys |
| LEGACY-RIG | I | Constraints/drivers on cloth bones that aren't `BC Follow` (Blender-only effect) | Show / remove legacy rig (§10) |
| HIDDEN-CLOTH | E | A cloth object won't be exported (hidden, excluded, not in the export set) | Unhide |
| UNITS | W | Scene unit scale ≠ 1, or non-uniform armature scale | Explain |
| PARAMS | W | `openWindStart ≥ openWindFull` (a step), `followTip > followRoot`, `maxAngle > 120°`, `spinStart ≥ spinFull` … | Show the field |
| STALE | W | A stage is stale (§6.1) | Update all |
| GLB-STALE | I | Bones, weights or clips changed since the last `.glb` export (settings-only export won't carry them) | Export model |

### 8.11 Step 10b: Export

#### 8.11.1 Panel
```
Targets
  ☑ glb ☑ skmm   source/Games/SlasherGame/GameData/models/player/player_coat.glb   [📁][×]
  ☐ glb ☑ skmm   source/Games/SimGame/GameData/models/player/player_coat.glb        [📁][×]
  [+ Add target]
☐ Export deform bones only   (134 → 88 bones for this file)
Will export: root, coat.001, body, body.001, arms   (visible meshes skinned to root)
[ Export model + settings ]   [ Export settings only (.skmm) ]
Last export: 2026-10-29 21:04, model up to date
In game: cloth_reload re-reads settings · cloth_debug 1 shows strips and capsules · model changes need the model reloaded
```

#### 8.11.2 glTF
`bpy.ops.export_scene.gltf(filepath=…, **settings)`, where `settings` = the scene's stored `glTF2ExportSettings` (the user's habits are
kept) with these forced: `export_format='GLB'`, `export_animations=True`, `export_animation_mode='ACTIONS'`, `export_skins=True`,
`export_influence_nb=4`, `export_all_influences=False`, `export_yup=True`, `export_rest_position_armature=True`,
`export_force_sampling=True`, `export_def_bones=deform_bones_only`. Preview state (sim, pose preview) is suspended and restored around
the call; Object mode is entered and restored.

#### 8.11.3 `.skmm` merge
- Read `<glb>.skmm` if present (nlohmann JSON, strict). Keep every other key (`animations`, `hitboxes`, unknown future keys) and every
  other cloth entry untouched.
- Find this setup's entry: same root set as written last time (`last_export.roots_written`, so renaming roots still replaces), else the
  same root set as now, else append.
- Write the entry in `BoneClothData` field order with the existing file's layout (4-space indent, blank lines between the groups:
  roots/forward/tip · motion · colliders · wind · spin). Floats are printed with up to 4 decimals and no exponent, `forward` as
  `{ "x": …, "y": …, "z": … }`, and colliders' `radius/endRadius` as 0.0 when auto. Write via a temp file, then rename.
- **Settings only** writes just the `.skmm` (for every target with skmm ticked). This is the fast tuning loop with `cloth_reload`.

#### 8.11.4 Post-export verification (phase 2)
Read the written `.glb` back (JSON chunk plus the accessors for `JOINTS_0`, `WEIGHTS_0`, `POSITION`, inverse binds, node transforms,
animations) with numpy and run `engine_contract` on it. Compare strips, joins, tips, sides, collider fits and layer usage with what the
panel showed before export. Any difference is reported as an error ("exporter changed X"). This catches exporter surprises such as
deform-bones-only re-parenting, trimmed channels, or a hidden mesh.

---

## 9. Presets (starting values)

Values come from `player_coat` where it applies; the others are first guesses to tune with the preview.

| Field | Long coat | Jacket | Skirt | Robe | Cape | Strand |
|-------|-----------|--------|-------|------|------|--------|
| layout / anchors | OPEN_FRONT / chest+hips | detected open/closed / chest+hips | CLOSED_RING / hips | CLOSED_RING or OPEN / chest+hips | BACK_ONLY / chest | STRAND / picked |
| panels per side (+centre) | 3 | 3 | 3 (+front +back) | 3 (+back) | 2 (+back) | 1 |
| rows below waist / upper part | 3 / yes | 2 / yes | 3 / no | 4 / yes | 4 (from shoulders) / — | 5 |
| opening half-angle | detected (≈40°) | detected | — | detected | front half | — |
| blend start → full | top−0.10 → waist−0.15 m, smooth | waist+0.05 → hem+0.10 | waist → waist−0.12 | as coat | shoulders → shoulders−0.15 | start → start+20 % |
| belt pin | detect (offered) | — | waistband 2 cm | detect | — | — |
| followRoot / followTip | 60 / 5 | 80 / 12 | 50 / 6 | 55 / 4 | 40 / 4 | 50 / 6 |
| trail | 0.5 | 0.3 | 0.4 | 0.5 | 0.6 | 0.4 |
| maxAngleRoot / Tip | 8 / 50 | 5 / 30 | 8 / 55 | 8 / 45 | 10 / 80 | 10 / 70 |
| radius (padding) | 0.05 | 0.03 | 0.04 | 0.05 | 0.04 | 0.02 |
| colliders | thighs, calves | thighs | thighs, calves | thighs, calves | spine (manual 0.16), thighs | none |
| open: hem / direction / columns | 24 cm / 0.6 / front 100 % side 40 % back 0 % | 8 cm / 0.6 / front 100 % | off | 15 cm / 0.6 / front | 30 cm / outward (billow) / all | off |
| volume: hem / columns | 11 cm / all 100 %, back 80 % | 4 cm / all | 8 cm / all | 10 cm / all | 15 cm / all | off |
| volume (always on) | 0.5 | 0.3 | 0.4 | 0.5 | 0.5 | 0 |
| open wind start/full | 1 / 13 | 2 / 12 | — | 1 / 13 | 1 / 10 | — |
| volume wind start/full | 1 / 15 | 2 / 14 | 1 / 12 | 1 / 15 | 1 / 12 | — |
| windwardBias / layerSmoothing | 0.5 / 0.2 | 0.5 / 0.2 | 0.5 / 0.2 | 0.5 / 0.25 | 0.5 / 0.3 | — / 0.2 |
| spinOpen / spinVolume | 0.8 / 0.6 | 0.5 / 0.4 | 0 / 0.7 | 0.7 / 0.6 | 0.6 / 0.8 | 0 / 0 |
| spinStart / Full / Release | 180 / 720 / 0.5 | 180 / 720 / 0.4 | 180 / 720 / 0.5 | 180 / 720 / 0.6 | 120 / 600 / 0.7 | — |
| rest: clear legs / flare | on / 0 | on / 0 | on / 1 cm | on / 0 | — / 0 | — |

Presets are JSON files (every setting group except object references). **Save as preset** writes one to the user preset folder;
**Apply preset** overwrites settings (not data) and marks stages stale.

---

## 10. Migrating `player_coat.blend`

Phase 2 feature, done once on a copy first.
1. **Adopt**: roots `cloth_root2`, `cloth_root` (read from `player_coat.glb.skmm`). The engine rules find 6 chains, 4 joints each, joined
   `00 → 01`. Columns are named from the bone names. Motion, colliders and clip names are imported from the `.skmm`.
2. **Capture the wind poses**: for `coat_open` and `coat_volume`, evaluate frame 2 with the legacy driver rig active and save each strip
   bone's evaluated basis as a HAND pose (§8.6.4). Shapes are kept to within float precision.
3. **Check**: expect LEGACY-RIG (constraints/drivers), CLIP-OTHER-BONES (the clips key all 134 bones), CLOTH-KEYS (locomotion clips
   key coat bones), HIDDEN-CLOTH info for the old `coat` (not a cloth object; info only), MESH-TRANSFORM (90° X / 0.01).
4. **Remove legacy rig** (optional, asks): deletes `MCH-*` coat bones, the three `*_ctrl` bones and their drivers and constraints,
   except the two `Follow Upper` copy-location constraints, which become `BC Follow`. Waist Follow and Hang are dropped: the engine
   doesn't use them (C12) and the preview shows the real behaviour. Exported joints go from 134 to 88.
5. **Weights**: keep (adopted, marked ✎), or rebuild with the BELTED_COAT profile (§8.4.2, close to the current distribution;
   compare with **Show cloth amount** before replacing).
6. **Export** to both game folders; compare the new `.skmm` with the old (should differ only in formatting) and the strips with
   `cloth_debug 1` in game.

---

## 11. Testing and parity

| Test | How | Pass when |
|------|-----|-----------|
| T1 contract unit tests | Plain Python + numpy, `tests/test_engine_contract.py`: synthetic skeletons and vertex sets for C2–C10 (empty bone, branch, join exact / 4 % / 6 %, tip fallback, side tolerance, collider leaves and fallback, 0.5° layer threshold) | Every case matches a hand-computed expectation taken from the C++ |
| T2 golden coat | Run `engine_contract` on `player_coat` data | 6 strips × 4 joints, joins `00→01`, sides L/R, collider radii equal to the engine's log lines `BoneCloth (cloth_root2): collider thigh_l-calf_l fitted to radius …` within 1 mm |
| T3 headless build | `blender -b tests/data/mannequin_coat.blend --python tests/run_build.py` for every preset | Weights sum to 1, ≤ 4 influences, every panel bone ≥ 1 vertex ≥ 0.05, last bones ≥ 0.25 at hem, joins exact, clips 2 frames, check has no errors |
| T4 round trip | Export, then re-read the `.glb` (§8.11.4) and the `.skmm` | Same strips, tips, colliders; `.skmm` keeps foreign keys byte-for-byte |
| T5 LBS bake | Apply a rest shape on a mesh with 2 shape keys; compare with Blender's armature modifier result on the base | Positions within 1e-5 of the armature's span; shape key deltas preserved |
| T6 preview parity | Same scripted motion (run 5 m/s, stop, 720°/s spin, 8 m/s headwind) in a test level with `cloth_debug` and in the Blender preview, side-by-side capture at fixed frames | Tip positions match visually (numeric harness later if needed) |
| T7 migration | §10 on a copy of `player_coat.blend` | Clip deltas per bone within 0.1°, identical strip set |
| T8 beginner run | Someone who has not rigged before sets up a downloaded jacket with only the panel | Under 15 min, no forum search; every question they ask becomes a tooltip or a check |

---

## 12. Phases

| Phase | Scope | Result |
|-------|-------|--------|
| **1: Coat & jacket end to end** | Data model, Character, Mesh prep (with body-weight transfer), Shape & bones for OPEN_FRONT / CLOSED_RING (sliders + overlay), Weights (profile, pins, exclusions, panel/joint blend, rigid parts, smooth, limit, panel colours), Rest shape (parametric, clear legs, hand edit, LBS bake, reset), Wind poses (generated, hand edit, preview composition), Colliders (auto + fit + draw), Motion (presets, macros, advanced), Check, Export (glb + skmm merge, several targets), Auto Setup, Update all | A beginner rigs a new coat or jacket and ships it |
| **2: Confidence** | Preview simulator with test motions and wind, adopt existing rigs + `player_coat` migration, post-export verification, side overlap fix, SKIRT / ROBE / CAPE / STRAND layouts, wind-direction pose preview | What you see is what the game does; old assets join the workflow |
| **3: Comfort** | Viewport gizmos (drag lines, column angles, hem amounts), user presets, weight transfer from a reference coat, bake preview to action, several setups per armature polish | Faster iteration for regular users |

---

## 13. Risks

| Risk | Mitigation |
|------|------------|
| Blender API churn (slotted actions, exporter options in later 5.x) | One adapter module for actions and export; min 5.1; T3 in CI-able headless Blender |
| Weighting heuristics fail on odd meshes (heavy overlaps, capes folded over shoulders) | Painted cloth amount, assign-to-panel, hand painting all respected; the check catches what the engine would break on |
| Preview drifts from the engine when `BoneCloth.cpp` changes | `ENGINE_COMMIT` constant, T1/T6 re-run on engine change; check warns when `BoneCloth.cpp` (if found under the repo) is newer than the contract |
| Exporter alters the skeleton (deform-bones-only re-parenting) or trims channels | Post-export verification (§8.11.4); deform-bones-only off by default |
| Beginners edit the visible mesh after setup | Vertex-count banner, "Use my edited mesh as the source" |
| Large meshes (> 50 k vertices) make live overlays slow | Overlays sample ≤ 5 k vertices; builds are numpy; progress in the status bar |

---

## 14. Open questions and engine notes

Decisions (answered 2026-10-29):
1. **Location**: `Tools/BlenderAddons/bone_cloth/`, engine-wide. Yes.
2. **Deform bones only on export**: an **optional** checkbox in the Export panel, **off by default**. The panel shows the bone count
   it would save (134 → 88 on `player_coat`); post-export verification (§8.11.4) covers the case where it is on.
3. **`player_coat.blend`**: **will be migrated** onto the add-on once Phase 2 (adopt) exists, following §10. Done on a copy first,
   and the old `.skmm` is kept for comparison.
4. **Blender version**: **5.1+ only**. No code paths for older versions (slotted actions and 5.1 API used directly).
5. **Preview parity**: a **hand-maintained Python port** of `BoneCloth` is accepted. No engine-side test harness is planned;
   `ENGINE_COMMIT` and the T1/T6 tests (§11) are the guard against drift.

---

## 15. Implementation status (Phase 1 built, 2026-10-29)

Built and tested (`README.md` has the commands): every module of §6.5 except `gizmos.py` and `sim.py`, plus `pipeline.py` (the steps
as plain functions), `stages.py` (done / out of date), `model_data.py` and a `tests/` folder.

| Area | State |
|------|-------|
| Engine contract (§2) | C2–C10, C13-adjacent maths, influence limit: `engine_contract.py`, 9 unit tests pass |
| Auto Setup, Update all, Start over | Done. On the old body-weighted `coat` mesh of `player_coat.blend` (old rig removed): 6 panels × 4 joined bones, belt found at 1.17 m, weights, rest shape, both clips, export |
| Verified independently | `tests/verify_glb.py` reads the exported `.glb` like the engine loader (all nodes, skins, weights, animations) and applies the engine rules: 6 strips, joins, sides L/R, tips 0.25–0.27 m, collider radii 11/8 cm and 7/7 cm, open clip used by 4 strips, volume clip by 6 |
| Rest-shape bake | Matches Blender's own posed result to 2e-5 (units: cm) with shape keys; reset restores the source exactly |
| UI | All panels draw against real property and operator names (mock-layout test). Not yet looked at in a live Blender window |
| Overlay | Rings for the lines, ghost bones of the planned layout (recomputed by a timer after a setting changes), leg capsules. Untested headless (needs a viewport) |

Differences from the design above (all deliberate; the first two found by testing):

1. **Source copy matrices.** Source copies live in an excluded collection and are never evaluated, so their world matrix is computed
   from the parent chain (`mesh_prep.world_matrix`); `model_data` skips them.
2. **Moving connected bones.** Bone lengths are read before any bone is moved (a parent drags the heads of its connected children).
   Without this the baked mesh was off by up to 0.5 cm.
3. **Clearing the legs (§8.5.3)** tests only the lower half of each panel and adds at most 10 cm of flare: the upper joints near the
   hips can never clear a thigh capsule, and the game's own pushing handles them.
4. **Columns** are stored for the left side (and centre); the right side is always the mirror. No `spread` slider: the angles come
   from the opening and the panel count (§8.3.3 with fixed margins).
5. **Weights**: sides by position only (no overlap fix for double-breasted flaps); no `BC exclude` / `BC pin` groups (the paintable
   `BC amount` group and belts cover it); no assign-to-panel, smooth-selected or mirror buttons; no panel-colour overlay (the
   cloth-amount group shows in Weight Paint; `tests/render_view.py` renders panel colours for development).
6. **Rest and wind poses**: no stored hand rotations on rebuild (§8.5.2), no edit banner or scale slider; **Save my pose as Open /
   Volume** captures the current pose, **Update poses** returns to generated.
7. **Check (§8.10)** has the rules that matter most for export; `SIDE-AMBIGUOUS`, `FACING`, `POSE-IN-LEGS`, `STRIP-BRANCH`,
   `CLIP-NOT-TWO-FRAMES` and `CLIP-OTHER-BONES` are not implemented. Stage "hand edited" (✎) marks are not shown.
8. **Not built yet (later phases)**: BACK_ONLY and STRAND layouts, capes; adopting an existing rig and migrating `player_coat.blend`
   (§10); the preview simulator and test motions (§8.9); post-export verification inside Blender (the script exists as
   `tests/verify_glb.py`); user presets; viewport gizmos; "Use my edited mesh as the source".

### 15.1 Short jackets: "Follow the spine" anchors (added after the first test on a real jacket)

A short jacket needs panels that run from under the arms to the hem and still follow a strongly bent spine. The engine allows it
(a strip may be joined from chains hanging from different bones; each joint's rest direction includes the relative bend between
its anchors), so the add-on has a third anchor mode, **Follow the spine**, used by the Jacket preset:

- The panel is cut at the spine joints (the heads of the spine bones). Each piece hangs from a root bone under the spine bone that
  physically carries it: the one with the largest head at or below the piece. The hem part below the pelvis hangs from the pelvis.
  Pieces from different owners are joined exactly (the lower head sits on the upper tail, plus a `BC Follow` constraint in Blender).
- **Bones toward the spine** (`bone_inset`, 0.75 in the preset): placed on the cloth surface, the bones sit 10-15 cm in front of
  the spine's joints, neighbouring pieces then disagree about where their joint is when the spine bends, and the chain drags the hem
  (18 cm after a 60 degree bend on the test jacket). So the torso part of each panel is moved toward the spine's joint line, fading
  back to the surface by the pelvis so the leg capsules still work. The top point keeps 3 cm of lateral offset because the engine
  reads a strip's left or right from it. Result on the test jacket after a 60 degree bend, against the mesh deformed with its
  original body weights: mean 1.8 cm, worst 8 cm (surface-placed bones: mean 6 cm, worst 19 cm).
- The top line starts about at the armpits (chest joint + 8 cm); the weight profile makes everything from there down move with the
  panels, so the whole jacket sways, and the open pose opens it in the middle (front panels).
- Auto Setup also drops a chest part that has no cloth under it, and uses fewer panels per side when the mesh is too sparse for
  that many (a missing-vertices panel would be cut by the game).
- `tests/test_jacket.py` runs this on a saved jacket scene.

### 15.2 Phase 2 (built after the first jacket)

| Piece | State |
|-------|-------|
| Swing preview (`sim.py`, step 9) | A port of `BoneCloth::Step` (wind and spin weights, rest shape with the layers, swing, colliders) running on the armature, driven by the timeline. Test motions: stand, walk, run, circle, spin, stop and go, strafe, teleport; wind speed, direction, gusts; engine debug lines. Tested (`test_sim.py`): rests exactly when nothing happens, trails a running body by 11-19 cm, a 12 m/s headwind opens every strip, a 720 deg/s spin gives full spin weight, a teleport settles |
| Adopt an existing rig (`adopt.py`) | Strips found by the engine's rules from the root bones; settings, colliders and clip names from the `.skmm`; the original `player_coat` rig is recognised as 6 strips of 4 joined bones. Never rebuilt or deleted (Start over only forgets it) |
| Post-export check (`glb_check.py`) | Every export is read back like the loader (all nodes, skins, weights, animations) and compared with the panel's plan: strips, sides and collider radii |
| Cape (`BACK_ONLY` + `CHEST` anchor) | Panels over the back, one root at the shoulders; tested on a synthetic cape (5 panels, blown back 26 cm in a headwind) |
| Strand | One chain along the mesh from a picked bone, joints at slice centres along its principal axis; weights run along it; tested on a synthetic ponytail |
| Hand-made poses | Saved with their slide, mirrored R/L, never overwritten by Update, recognised when keys are added by hand |

Things found while building it, worth knowing:

- The preset's always-on `volume` (0.3 for the Jacket) means the rest shape in the game is never the plain rest: the preview
  at "stand still" shows it, as the game does.
- On a teleport the engine computes the body's velocity from the jump, so the wind layers open for a frame or two (a gale of
  120 m/s) before the weights settle. The port reproduces this; the strips themselves snap and do not stretch.
- Bone selection in Blender 5 lives on pose bones (`PoseBone.select`), not on `Bone`.

Still open: migrating `player_coat.blend` itself (the adopt step works; removing the legacy driver rig is not automated),
viewport gizmos, user presets, side-overlap fix for double-breasted flaps.

Engine behaviours the add-on works around (notes only, no changes proposed):
- No gravity: rest directions are kept on the anchor, so a pelvis tilted in an animation tilts the skirt. The rest shape and the
  two-anchor layout are the tools for this.
- One chain per root child, at most two wind layers.
- Joined strips are detected by geometry only (C4), so the generator must align them exactly and the check must show near-misses.
- Collider auto-fit counts every mesh's vertices by dominant bone, including cloth still dominated by body bones; torso capsules need
  manual radii.

### 15.3 Wind and turn clips, torso surface collision (2026-10-30)

The ShooterGame jacket's bones run 2-4 cm off the spine, inside the body, and its hand-made Open pose twists them about their
own axes: the panel swings around the spine like a door. That is the look wanted, so the bones stay; but tip collision
could never keep that cloth out of the torso (the tips are inside any torso capsule and a twist does not move them), and
Open stacked on Volume compounded the twists (112 degrees on `coat_front_03`). Two changes, engine and add-on together:

- **Clips instead of layers (C19).** Poses are authored, physics is the safety net: a clip per wind side and per turn
  direction, played as far as the wind or turn has grown and blended, never stacked. A headwind clip can open the jacket on
  frame 2 and blow it back on frame 3. Right clips are the left ones mirrored with both sides swapped
  (`poses.swapped_pose`).
- **Surface colliders and probes (C20, C21).** The torso capsules keep a few of each joint's vertices out by turning the
  joint about its head. On the jacket a single spine capsule (11.5 / 13.5 cm) fits: its cloth rests 11-17 cm from the spine
  all round, so no side capsules. In the preview a 15-degree turn clip took the probes 1.6 cm into it; with the probes,
  0.07 cm.
- The check poses every clip frame and the 45-degree blends in Blender (so the BC Follow joins act as the game's) and
  reports new depth into any collider and edge stretch.

Not covered: skinning still stretches the seam between a front panel twisted around the body and the back panel
(`POSE-STRETCH` reports it, 1.5x on the jacket's Open pose); that is for the pose, not the collision. Engine changes:
`Cloth/BoneCloth.{h,cpp}`, `BoneClothData.h`, `AnimationPose::BoneTRS / DecomposeBone / ComposeBone` made public.

### 15.4 Generated clips (clipgen.py)

All six clips come from one reference Open pose: the hand-made one, or a hinge (front panels swing up to 55 degrees
round the body, reached 60 % down the panel; back panels flare 35 degrees). Each bone's world turn is taken apart by
swing-twist about the body's up into a yaw (round the body), a flare out (about the panel's tangent) and a roll (about
its radial), with a per-panel profile (how far down the panel each bone has its share). Every frame scales these:
`frame_params` per clip and side (front opening, back flare, extra hem lift, a push - every panel moved the same way,
the air's - or a spin - every panel turned round the body, the cloth trailing a turn). Opening beyond the Open pose
spreads up the panel by `profile ** 0.3`, so the chest pieces peel back with it instead of the panel creasing at the
waist; the windward front panel takes no push (it would cross the opening). Frame 2 of the headwind reproduces the
reference exactly.

Judged on rendered contact sheets (`tests/render_clips.py`: five views per pose, cloth coloured by panel, top view cut
at chest height), not on numbers. What the sheets showed and changed: a side wind made as a spin sent the back panels
upwind (now a push); the windward front panel swept over the opening (now pressed only); the blown-back front creased
at the waist (now the chest follows). Still visible: a crease or small tear at the front/back seam by the hip when the
front panel is thrown far round - skinning, reported by POSE-STRETCH.

Later, on large renders and the live viewport (the user's reports): hem lift pushed straight out from the body sent the
front of a headwind forward (now it goes the way the air goes, after the panels' swing); the back stacking its flare on
the lift gave an M (now front and back lift alike); swinging the front panels far past the Open pose crossed the side
seams and tore them from the back panels, and the reference's roll, applied about axes the panel had swung away from,
folded each panel (now the reference shape is built first and carried round as one piece, Blown back defaults to
1.0 - a front panel never swings round further than the Open pose, past which it crosses the side seam - and the lift
spreads down every panel by length alike so panels lift together at their seams).

Wind and turn combine in layers (C19 revised): the wind's clips (two sides by angle) at full weight, the turn's clip
on top. While the body spins, the wind's side fades by the spin weight - it sweeps round faster than cloth can follow,
and in the preview it made the jacket flap open and shut twice a turn - and its smoothed speed counts as headwind
instead, plus spinOpen x windFull at a full spin (`spin_air`): spinning in wind opens and flares the jacket steadily.

Wind from behind follows the user's hand-made clip (subtle): the front panels close 0.15 of the Open pose's opening
(their slide too) with 0.34 of its roll against it, the front hems tilt out 0.42 of Hem lift, the back stays; frame 2 is
half. Generated against the hand-made one: 0.6 cm apart on average at frame 3, 2.1 cm at the hem tips.

Weights: the mesh's halves are not built alike (130 / 128 vertices), and the weights came out uneven at the side seams
(right side 1-7 cm behind the side still 43-84% on its front panel, the left 24%), so turning left opened the right side
50% more than the mirrored turn opened the left. Mirror weights (Weights step) gives each vertex on one side the
weights at its mirror image on the other side's surface, interpolated across the face it lands on, left and right bones
swapped.

Physics: Swing and Collision (0..1) scale the trailing and the colliders' push; both 0 = the clips alone. Windowed
trailing (motionWindow > 0): each tip trails the average speed of the spot it hangs from over the window (lagRoot..lagTip
seconds of it), so a walk's sway cancels - pelvis sliding 3 cm at 2 Hz: hem tips 0.74 cm classic, 0.11 cm with 0.5 s -
while a run still trails. motionWindow 0 keeps the classic trailing (Slasher's coat).
The windowed trail is split into the body's turn (movement around the axis through the roots, beyond the body's own) and
the rest, by direction: physicsFront/Back/Sides/Vertical and physicsSpin scale each. It never points toward that axis:
running backward used to push the back panel (bones inside the body, only 2-3 probes) into the torso; now the back hem
tips stay 13.9 cm out (14.1 standing) with a 0.1 s trail.

Legs on the probes (with surface colliders, i.e. bones inside the body): crouch-walking sideways the thighs came up
through the jacket's front ~9.5 cm - the animated pose puts it there, and the leg push on tails (swing-limited to
2-16 degrees) and bone lines (inside the body) can't see the cloth surface. Now the leg capsules push the probes and each
joint's tail: a probe inside a leg or in its shadow (between leg and body, seen from the body's axis) goes to the leg's
outer surface (out from the leg's axis, mirrored outward), past the hip end not at all (pelvis = surface colliders).
Each joint turns for the probes of its own and all joints below it (posed relative to it), by 1/(joints from it to the
probe's own) of each push, so the strip bends over the thigh along its length; a joint alone can't (its head is often
inside the thigh). Probes may stay as deep as they rest; ones resting in a leg's shadow are ignored by it. 3 iterations
didn't converge (one probe each, ~20 per strip): 8 (LEG_ITERATIONS). tests/test_crouch.py: deepest 9.4 -> 7.0 cm
(crouch_l), 9.6 -> 7.7 (crouch_r); renders show the front and sides lying over the thighs. What's left is between
probes (12 probes per joint did not change it).

Front wind, last frame (new jacket 'jaket', no hand-made Open pose, so the generator's own hinge): the hems of the front
panels curled in at the sides (cloth bunched at x=0.30 m) where the artist's edit fans them out (0.42 m, 6 cm higher).
Fitting the artist's frame 3 against the generator (tail positions, 12 bones): the missing part is an outward tilt of
the lower panel about the panel's own outward axis AS IT STANDS ONCE OPENED (the rest axis, `-tangent`, tucks the hem in
or forward: 7 cm worse), growing from 20% down the panel to the hem, about as large as the hem lift (Frame.flare = lift;
FLARE_FROM). Mean tail error 6.9 -> 1.7 cm (max 13.7 -> 3.3). Only windFront's last frame has it; the others unchanged.
The flare axis follows the panel's whole swing (the Open yaw plus the blown-round part): with Blown back 3 it kept the
1x axis and curled the hems in. And a front panel is now never blown further round than MAX_AZIMUTH (110 degrees, 90 =
straight to the side): at Blown back 3 the panels swung 165 degrees round and folded up behind the back panel. Changes
nothing at Blown back 1 on the player coat (all six clips, tails within 0.0 cm).

Skin folds at the hem corners ('two strange folds'), front wind last frame: not the weights. The front panels swing
96 degrees round the body, the side panels beside them 30: the cloth between, 69 degrees of arc at rest, has to squeeze
into about 3, and linear skinning folds it (tests/skin_metric.py: 18 squeezed triangles / stretched edges in that pose).
Tried and dropped: other weight settings (smoothing, sharper edges, softer joints: all worse or no better) and a
pose-aware refit of the vertex weights (projected gradient on the edge lengths over all clip poses): the numbers
improved, the renders did not (more jagged) - the cloth cannot disappear, only fold somewhere else. Fixed in the pose:
a front wind carries the side panels round with the front ones (Frame.carry, SIDE_CARRY = 0.5 of the front's swing,
strongest straight to the side, none at the back): 18 -> 5 in that pose, the hem now flows out as one flare. Side
wind (windLeft frame 2: 8) and the rest are unchanged.

Spinning (preview Spin, spinOpen 1): the layers stack - the front wind (a spin's headwind) blows the front panels round
to MAX_AZIMUTH, then the turn clip opens the far front panel again (0.8 of the Open pose) and turns everything 16 degrees:
the front hems ended 138 degrees round the body, behind the side and back cloth (10 of 81 hem vertices under it, 216
triangles flipped over their rest normal). tests/test_spin.py measures it over a 720 deg/s spin. Changed: the turn opens
the far front panel 0.3 (TURN_OPEN: the spin brings the wind anyway), the side carry is the front's whole swing
(SIDE_CARRY 1.0) and MAX_AZIMUTH is 100 degrees: 2 of 81 under, 127 flipped, hems 123 degrees round. Only the front
panels' far side matters; other clips and the player coat's static tests are unchanged.

Back fold ('jaket wind front isn't wide and has this fold in the middle of back'): the generator's own hinge gave the back
strips (theta +-172 degrees, Opens 0.8) a yaw of +-44 degrees in the front panels' sense - toward each other. Left and right
back halves crossed the centre line (rest x -5 -> +1.6 cm, 0.4 -> -5 cm: the cloth flipped over a 12 cm band: the V),
in every clip that uses the reference, since the first version. Panels beyond BACK_FROM (135 degrees) no longer yaw in the
hinge; they flare and lift. Back hem across the seam: x unchanged, y +6..10 smoothly. Front wind's last frame at Blown back 3:
squeezed triangles / stretched edges 18 / 10 -> 1 / 3.
My earlier side carry (all non-front panels, 60 degree bell) also moved the side hems inward (0.33 -> 0.12 m) and the back
across the centre, narrowing the pose the artist's hand frame had wide. Now it only follows the front panels' swing PAST the
Open pose (zero at Blown back 1, so the defaults reproduce the hand frame to 0.0 cm for the sides and 0.3 for the back,
tests/fit_hand.py), side panels only (CARRY_WIDTH 30 degrees round straight to the side).

'It misses proper lift. It's just tips. Should be whole spine': two causes. (1) The upper strip bones hang from their own
spine bones; the BC Follow constraints (COPY_LOCATION) pin each to the previous tail so a strip behaves as the engine's chain.
The preview mutes them while it runs and un-mutes them on stop - a reload or a save while it ran left them muted (and a
preview started on muted ones never un-muted them), so every pose showed loose hinges: only the short hem bones swung
(back +7 cm, hem 'curling'). sim.unmute_orphans (on start, on enabling the add-on, after loading a file) puts them back.
Every scene copy saved while a preview ran was muted: measure on a copy with them on (tests/skin_metric, test_spin,
fit_hand all load the scene as it is). (2) With the chain on, the generator's lift (1.6 x the hem lift, growing toward the
hem) is what the artist wants: swung out by hand, the back bones' world turns were 9.3, 17.4, 29.1, 38.9, 40.8, 42.4 degrees,
which the generator produces to 0.1 degrees (tails 0.0 cm). A uniform tilt of the whole back (tried: BACK_SPINE 0.35-0.6,
'the hem curls up' read off the muted pose) was wrong and is gone.

Preview speed (Blender 5.1, 36 cloth bones): 33 ms a step, 70% in the thigh push called once per probe and leg. leg_push_many
does all probes against all legs in one call (checked against leg_push in test_engine_contract): 12 ms, 14 ms a frame with
Blender's own update. tests/profile_sim.py times and profiles it.

Only colliders the cloth covers (collider `covers`, set on the auto leg capsules) do this. Arm capsules first got it
too (every non-surface collider) and the jacket exploded: the arms hang outside the jacket, so the cloth between arm
and body was thrown across them. Other colliders keep the plain tip push. With bones inside the body that push does
little and can harm: arm capsules (hand to lowerarm) put the jacket 2.4 cm into the torso in a 25 m/s side wind
(test_wind_clips fails on a scene with them). Tried the arms' match - the same solver keeping the cloth on an arm's
inner side - and dropped it: the jacket was flung up over the shoulders (15 cm into the arms; 8.6 cm when only cloth
blown fully past an arm was brought back) in the game's idle, no wind. The rest allowance comes from the bind pose, where
the arms are out (A-pose); hanging, the forearm capsules sit in the jacket's sides, so keeping the cloth under them
fights the torso. With the tip push the jacket stays within 0.5 cm of the arms in a 25 m/s side wind and a 180 deg/s turn.

Turns follow the user's hand-made turn (moderate): the whole jacket turns about 16 degrees with the body, the trailing
front panel opens 0.8 of the Open pose and flares out, the back flares 0.4; frame 2 is half of frame 3. A hand-made left
clip is kept and its right clip mirrored from it. Still open on the jacket: in the Open pose the front panels swing
through the hips (the torso capsule is narrower than the hips), and the side-seam hem corners fold - skinning between a
panel turned 55 degrees and one at 0 (weights cannot reach the arc; smoothing off, hard seams, a denser mesh did not
help, fitting weights to the poses helped a little).
