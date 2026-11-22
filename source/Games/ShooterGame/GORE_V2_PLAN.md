# Gore V2 — layered flesh and procedural anatomy (implementation plan)

Target look: Left 4 Dead 2. A hit tears a ragged hole in the skin and you see what is inside: red meat, ribs, bone,
guts. Limbs come off with the bone sticking out of the stump. Nothing is modelled per character: every interior is
generated from the character's own mesh and skeleton when its gore profile is built.

This file replaces `GORE_DESIGN.md` §3-§4 and `GORE_IMPLEMENTATION.md` §10 (decals, wound slots, cavity meshes, the
ray-traced interior pass). Where they differ, **this file wins**. The rules of `GORE_IMPLEMENTATION.md` §0.1 and the
index-space table of §0.2 still apply; read those two sections first.

Do the tasks **in order**. Each task ends with a check. Do not start the next task before the check passes. If
something in this plan does not match the code you find, stop and ask; do not improvise.

The plan has two parts:

* **Part 1 (T1-T12): the base.** Layered flesh, generated anatomy, torn severing. Build it and pass the T10 picture
  checks first.
* **Part 2 (T13-T14): two features on top of the working base**, each a small, separately checked step:
  **T13** flat sliced cross-sections for blade cuts, **T14** flying meat chunks.
  Part 2 is **required**, not optional. It is separate only so that a mistake is easy to find: every Part 2 task
  says exactly which Part 1 code it changes.

Blood running down the skin over time is **not** wanted (Left 4 Dead 2 does not have it). Do not add it.

**Amendments (2026-11-14, user requests; they win over everything below):**

* **No normal map and no specular on gore parts.** The game is LDR (no HDR, no tone mapping): a bump or a highlight on a
  dark wet surface clips to white sparkle. Wherever this plan mentions `u_goreFleshNormal`, `fleshNormal`,
  `goreBumpTriplanar`, `goreWetShade`, wet / gloss / fresnel / specular of flesh, cap, shells, anatomy or chunks: do not
  implement it. Flesh is shaded by `goreShade` in `gore_flesh.sh` (stepped diffuse of the directional and point lights
  plus a warm red scatter, nothing else). The profile has no `fleshNormal` texture key any more. The skin layer
  (`fs_gore.sc`) keeps the stock mesh shader's own specular and adds no blood shine.
* **Protected bones while the enemy is alive.** A profile may list `"protectedWhileAlive": ["neck_01", "head"]` (profile bone
  names, mapped through `boneMap`; each bone and everything below it is protected). While the victim lives, a hit that does
  not kill it leaves no hole, no cut and no meat on a protected bone (a small blood splat and a burst only), and no cut may
  remove a part that holds a protected bone (`SeverableBone::holdsProtected`: a waist cut would take the head with it).
  The sever damage of such hits is still collected, so the killing blow can take the part off. The killing blow ignores
  the list (a lethal hit may still blow a head off). The list is read in `GoreProfile.cpp` right
  after the severable block, stored as `Profile::protectedWhileAlive` (one byte per skeleton bone); the checks are in
  `ProcessPendingHits` (before `TrySever`), in `TrySever` (candidate selection) and in `BlastSever`.
* **Wounds have an inside (the cavity).** A wound ellipsoid is a boolean subtraction from the body: where it lies inside the
  body it leaves a closing surface (`Gore/GoreCavity.*`), built on the worker thread, skinned with bone weights taken from the
  nearest body triangles, and removed where another wound or a cut has already taken the body away. The body volume it is
  clipped to is the `Volume` submeshes plus every closed non-Volume submesh (`TriGrid::Mode::Body`). A hole must never show the
  floor through a part that is bigger than the wound. See `GORE_IMPLEMENTATION.md` section 9.1 for the details and for what was
  and was not looked at.

---

## 0. Read this first

### 0.1 What is kept, what is deleted

| Kept unchanged | Changed | Deleted |
|---|---|---|
| Engine (`source/Engine`): **no changes at all** | `GoreTypes.h`, `GoreComponent.h`, `GoreRenderState.h` | `GoreWoundMesh.h/.cpp` (cavity meshes) |
| `GoreCutSolver.h/.cpp` | `GoreProfile.h/.cpp` | `fs_gore_cavity.sc` |
| `GoreSource.h/.cpp` (one new field) | `GoreDrawCommands.h/.cpp` | `fs_gore_interior.sc`, `fs_gore_interior_depth.sc` |
| `GoreEvents.h`, `GoreTypes.cpp` | `GoreSkeletalMesh.h/.cpp` | wound slots, decals, `freeformWounds` |
| `StealthGoreAdapter.h/.cpp` | `GoreSystem.h/.cpp`, `GoreFx.cpp` | |
| `GoreParticles.cpp` | `GoreGib.cpp`, `GoreDebug.cpp`, `GoreSettings.h/.cpp` | |
| shadow shaders `vs_shadowvol*_gore.sc`, `gore_shadow.sh`, `gore_flesh.sh`, `varying.def.sc`, `vs_gore.sc` | `gore_common.sh`, `fs_gore.sc`, `fs_gore_depth.sc`, `fs_gore_customId.sc` | |

New files: `Gore/GoreFlesh.h`, `Gore/GoreFlesh.cpp`, `shaders/source/gore/vs_gore_inner.sc`,
`fs_gore_inner.sc`, `fs_gore_inner_depth.sc`. Part 2 adds `Gore/GoreCap.h/.cpp` (T13) and `Gore/GoreChunks.h/.cpp`
(T14), and one line in `GameStart.cpp` (T14).

### 0.2 The idea in one page

**Layers.** A wounded body is drawn as nested surfaces, outside to inside:

| Layer index | What | Geometry | Texture |
|---|---|---|---|
| 0 | skin (the normal model) | model vertex + index buffer | model texture + blood |
| 1 | fat / torn meat | **shell 1**: the model moved inward by 30 % of its max inset | flesh, pale |
| 2 | muscle | **shell 2**: 60 % | flesh |
| 3 | deep meat | **shell 3**: 88 % | flesh, dark |
| 4 | **anatomy**: bones, ribs, spine, skull, guts | generated mesh, one per model | bone / guts |

A *shell* uses the model's own **index buffer** (so the bone-partitioned index ranges work) and a second **vertex
buffer** built once per model. Each shell vertex stores the inward direction and its *max inset* (metres). The vertex
shader moves it: `position + inward * maxInset * fraction`. Max inset is 40 % of the body thickness measured at that
vertex (capped at 7 cm), so thin parts never turn inside out and a thin sheet (max inset 0) gets no flesh.

**Wound = one ragged ellipsoid in bind space** (the existing `Wound` struct). Every layer discards the pixels inside
it, but each deeper layer uses a **smaller** ellipsoid:

```
scale(layer) = healScale * max(0, 1 - layer * taper)        // taper is per wound, 0.1 .. 0.5
```

* `taper = 0.5` (graze): skin hole full size, shell 1 half size, shells 2-3 and bones untouched. You see red meat.
* `taper = 0.2` (crater): all layers open, bones get a hole of 20 % size. You see ribs with a broken gap.
* `taper = 0.1` (destruction): layers 1.0 / 0.9 / 0.8 / 0.7 / 0.6. Everything is cut, the limb is in two pieces.

Hits that land on an existing wound make it bigger **and lower its taper**, so shooting the same spot digs deeper.

**Severing = a cut plane + a destruction wound at the same place.** The plane (existing `Cut`) only decides which half
of the cut bone belongs to the body and which to the gib. The destruction wound removes a ragged band on both sides
of the plane in every layer, so both ends are torn and tapered: skin ends first, then meat, and the bone sticks out
furthest. No cap geometry, no ray tracing, no `gl_FragDepth`.

**Why nothing shows through.** Shells and anatomy are drawn two-sided; a back face is shaded as dark wet meat. Through
a hole you therefore see either a deeper layer, a bone, or the dark inside of the far wall. You only see the world
when the far side has a wound too (a through-hole), which is correct.

**Cost.** Undamaged enemies: unchanged (default shaders, no extra draws). A wounded body draws, per wounded submesh,
3 extra shell draws limited to the index ranges of bones near a wound, plus one anatomy draw limited to the pieces
near a wound. No per-wound geometry, no worker thread, no per-wound GPU buffers.

### 0.3 Layer constants (use these names everywhere)

```cpp
// GoreTypes.h
constexpr int   kShellLayers = 3;
constexpr float kShellFraction[kShellLayers] = { 0.30f, 0.60f, 0.88f };   // share of a vertex's max inset
constexpr int   kAnatomyLayer = kShellLayers + 1;                          // = 4
constexpr float kMinTaper = 0.10f;
```

### 0.4 Build, shaders, test

* Build (incremental): `Build\ui_ninja.cmd ui_stealth` from a `cmd` prompt. After **adding or deleting a .cpp**, run
  `Build\ui_cfg_stealth.cmd` instead (CMake globs the sources; it reconfigures and builds).
* Also edit `StealthAction.vcxproj` and `StealthAction.vcxproj.filters` for every added / deleted file (task T9).
* Shaders: run `source/Games/StealthAction/GameData/shaders/ShaderCompilationAutomation.exe` from its own folder.
  Every shader must compile for every target. **Declare every `SAMPLER2D(name, slot)` in the `.sc` file itself**, never
  in an include: the engine reads sampler slots from the `.sc` text only.
* Run: exe `Build/StealthAction/windows/bin/StealthAction.exe`, **working directory**
  `source/Games/StealthAction`, always with `-headless` (never without: it would open a window on the user's desktop).
* Look test (already in `GoreDebug.cpp`): `-headless -gore_look npc_human_axe -gore_src weapon_pistol -gore_shots 5
  -gore_dist 1.1 -gore_file <name>` writes `SaveData/screenshots/<name>.png` and quits. Other switches: `-gore_aim
  <bone>`, `-gore_aim_dy`, `-gore_aim_dx`, `-gore_damage <n>`, `-gore_lethal`, `-gore_alive`, `-gore_yaw <deg>`,
  `-gore_wait <tick>`, `-gore_flip`. **Open every screenshot and look at it.** A build that compiles proves nothing here.
* Commit only when the user asks. Never push.

---

## T1. Types (`GoreTypes.h`, `GoreComponent.h`, `GoreRenderState.h`)

### T1.1 `GoreTypes.h`

1. Delete `kMaxDecals`, `struct CavityGeometry`, `struct CavityTicket`, `struct Decal`, and the include `<atomic>`.
   Keep `<memory>` (T13 needs it). `<VertexData.h>` may go (`GoreFlesh.h` includes it itself).
2. Add the constants of §0.3 after `kMaxSkeletonBones`.
3. In `struct Wound` delete the fields `cavityTicket`, `cavityFailed`, `cavity`, `slot`, `through`. Add:

```cpp
        float   taper = 0.5f;       // how fast the hole shrinks per layer: scale(layer) = heal * max(0, 1 - layer * taper)
        int16_t limitSkel = -1;     // >= 0: the wound only touches this skel bone and its subtree (sever wounds)
```

Everything else in the file stays (`Rng`, `HashSeed`, `WoundKind`, `Cut`, `Splat`, `Hit`, `SetHitBone`).

### T1.2 `GoreComponent.h`

Delete: the `decals` array and `decalCount`, `openSlots`, `slotDamage`, `slotDamageTime`. Keep everything else.

### T1.3 `GoreRenderState.h`

Delete the `decals` array and `decalCount`.

**Check:** nothing compiles yet (other files still use the removed fields). That is expected; go on.

---

## T2. Settings and sources

### T2.1 `GoreSettings.h/.cpp`

Remove `freeformWounds` (field, comment, the `j.value` line). Add:

```cpp
        bool  severAlive = false;   // false: limbs and heads only come off with the killing blow or on a corpse
```

and load it: `s.severAlive = j.value("severAlive", s.severAlive);`.

`GameData/gore/settings.json` becomes:

```json
{
  "maxGibs": 48,
  "maxGibSpawnsPerFrame": 6,
  "maxParticlesPerFrame": 96,
  "level": "full",
  "freshBand": 0.05,
  "severAlive": false
}
```

### T2.2 `GoreSource.h/.cpp`

Add to `struct Source`: `float taper = -1.0f;   // < 0: the default of the source type (GoreSystem)`. In
`GoreSource.cpp`, where the other fields are read from JSON, add `taper` with `j.value("taper", -1.0f)` in the same
style as `woundRadius`. Leave `holePower` and `decalRadius` in the struct (unused now, harmless) but stop documenting them.

`GameData/gore/sources.json` — replace the `"sources"` object (keep `"classMap"` exactly as it is):

```json
"sources": {
  "default":   { "type": "bullet",      "woundRadius": 0.026, "refDamage": 20,  "exit": false, "severPower": 0.5, "tags": ["ranged"] },
  "pistol":    { "type": "bullet",      "woundRadius": 0.030, "refDamage": 20,  "exit": true,  "severPower": 1.0, "tags": ["ranged"] },
  "revolver":  { "type": "heavyBullet", "woundRadius": 0.045, "refDamage": 45,  "exit": true,  "severPower": 2.5, "tags": ["ranged"] },
  "sawed":     { "type": "pellet",      "woundRadius": 0.022, "refDamage": 8,   "clusterRadius": 0.09, "severPower": 2.0, "tags": ["ranged"] },
  "sword":     { "type": "slash",       "woundRadius": 0.035, "gashLength": 0.20, "refDamage": 30, "severPower": 2.5, "tags": ["melee"] },
  "axe":       { "type": "slash",       "woundRadius": 0.045, "gashLength": 0.16, "refDamage": 30, "severPower": 2.5, "tags": ["melee"] },
  "cane":      { "type": "blunt",       "woundRadius": 0.0,   "refDamage": 30,  "severPower": 0,   "tags": ["melee"] },
  "crucifix":  { "type": "blunt",       "woundRadius": 0.0,   "refDamage": 30,  "severPower": 0,   "tags": ["melee", "holy"] },
  "explosion": { "type": "blast",       "woundRadius": 0.13,  "refDamage": 60,  "severPower": 1.5, "minScale": 0.6, "maxScale": 2.2, "tags": ["blast", "fire"] },
  "cannon":    { "type": "blast",       "woundRadius": 0.22,  "refDamage": 200, "severPower": 1.5, "blastRadius": 0.7, "minScale": 0.8, "maxScale": 1.3, "tags": ["blast", "ranged"] }
}
```

Wounds are deliberately larger than real bullet holes: at 5-15 m a 1 cm hole is one pixel.

---

## T3. Profile (`GoreProfile.h/.cpp`) and data files

### T3.1 `GoreProfile.h`

1. Delete `struct WoundSlot`, the forward declaration `struct TriangleGrid;`, `Profile::triGrid`, `Profile::slots`.
2. Add before `struct Profile`:

```cpp
    struct ModelFlesh;   // GoreFlesh.h

    // Where the generated skeleton and organs go. Bone names are skeleton names after "boneMap" was applied.
    struct AnatomyConfig
    {
        glm::vec3 forward = glm::vec3(0, 0, 1);   // bind space: the direction the character faces
        glm::vec3 up = glm::vec3(0, 1, 0);        // bind space
        std::vector<uint16_t> torso;              // skel: bones of the trunk (they get a spine at the back, no limb bone)
        std::vector<uint16_t> ribs;               // skel: trunk bones that get ribs and chest organs
        std::vector<uint16_t> guts;               // skel: trunk bones that get a gut mass
        int head = -1;                            // skel: gets a skull, -1 = none
        float minBoneLength = 0.06f;              // metres: shorter bone segments get no bone inside
    };
```

3. In `struct Profile` add:

```cpp
        Texture* guts = nullptr;
        AnatomyConfig anatomy;
        std::shared_ptr<ModelFlesh> flesh;        // shell vertices + anatomy mesh, built once per model (GoreFlesh.cpp)
```

(`boneT` already exists: it is the bone texture.)

### T3.2 `GoreProfile.cpp`

1. Replace `#include "GoreWoundMesh.h"` with `#include "GoreFlesh.h"`.
2. After `p.boneT = LoadTexture(textures, "bone");` add `p.guts = LoadTexture(textures, "guts");`.
3. Delete the whole block `// Authored wound slots.` (`if (j.contains("slots") ...) { ... }`).
4. After the severable block (before `// Vertex samples for the cut solver.`) add the anatomy parsing:

```cpp
        // Anatomy layout (all optional; missing bones are skipped with a warning).
        {
            const nlohmann::json a = j.value("anatomy", nlohmann::json::object());
            auto readVec3 = [](const nlohmann::json& v, glm::vec3& out)
            {
                if (v.is_array() && v.size() == 3) out = glm::vec3(v[0].get<float>(), v[1].get<float>(), v[2].get<float>());
            };
            auto boneOf = [&](const std::string& name) -> int
            {
                const uint16_t skel = model.skeleton.findBone(mapBone(name));
                if (skel == INVALID_BONE_IDX || model.skeleton.bones[skel].skinIdx < 0)
                {
                    Logger::Warning("Gore: '%s' anatomy bone '%s' is missing or has no skin index", modelPath.c_str(), name.c_str());
                    return -1;
                }
                return skel;
            };
            auto readList = [&](const char* key, std::vector<uint16_t>& out)
            {
                if (!a.contains(key) || !a[key].is_array()) return;
                for (const nlohmann::json& n : a[key])
                {
                    if (!n.is_string()) continue;
                    const int skel = boneOf(n.get<std::string>());
                    if (skel >= 0) out.push_back(static_cast<uint16_t>(skel));
                }
            };

            if (a.contains("forward")) readVec3(a["forward"], p.anatomy.forward);
            if (a.contains("up")) readVec3(a["up"], p.anatomy.up);
            if (glm::length(p.anatomy.forward) < 1.0e-4f) p.anatomy.forward = glm::vec3(0, 0, 1);
            if (glm::length(p.anatomy.up) < 1.0e-4f) p.anatomy.up = glm::vec3(0, 1, 0);
            p.anatomy.forward = glm::normalize(p.anatomy.forward);
            p.anatomy.up = glm::normalize(p.anatomy.up);
            readList("torso", p.anatomy.torso);
            readList("ribs", p.anatomy.ribs);
            readList("guts", p.anatomy.guts);
            if (a.contains("head") && a["head"].is_string()) p.anatomy.head = boneOf(a["head"].get<std::string>());
            p.anatomy.minBoneLength = a.value("minBoneLength", p.anatomy.minBoneLength);
        }
```

5. At the end replace `p.triGrid = BuildTriangleGrid(model, p);` with (it must come **after** `p.anyBounds`,
   `p.parent`, `p.subtreeEnd` are filled, because the builder reads them):

```cpp
        p.flesh = BuildModelFlesh(model, p);
```

6. `ClearProfiles()` stays as it is. It has no caller today. Do **not** destroy GPU buffers in it (a draw command may
   still hold the handles).

### T3.3 Data

`GameData/gore/humanoid.gore`: delete the whole `"slots"` array, add `"guts"` to `"textures"`, change `"fleshScale"`,
and add `"anatomy"`. The `"severable"` array stays exactly as it is. Result (severable shortened here):

```json
{
  "textures": {
    "flesh": "gore/flesh_d.png",
    "fleshNormal": "gore/flesh_n.png",
    "blood": "gore/blood_soak.png",
    "edgeNoise": "gore/edge_noise.png",
    "bone": "gore/bone_d.png",
    "guts": "lq_flesh/flesh_gut1.jpg"
  },
  "fleshScale": 5.0,
  "bloodColor": [0.35, 0.02, 0.02],
  "submeshes": {},
  "anatomy": {
    "forward": [0, 0, 1],
    "up": [0, 1, 0],
    "torso": ["pelvis", "spine_01", "spine_02", "spine_03"],
    "ribs": ["spine_02", "spine_03"],
    "guts": ["pelvis", "spine_01"],
    "head": "head",
    "minBoneLength": 0.06
  },
  "severable": [ ... unchanged ... ]
}
```

`models/enemies/knight/knight.glb.gore`: delete its `"slots"` array and extend `"boneMap"` so the anatomy names
resolve (the knight uses a Mixamo rig; `mapBone` is applied to anatomy names too):

```json
{
  "extends": "GameData/gore/humanoid.gore",
  "boneMap": {
    "upperarm_l": "mixamorig:LeftArm",   "upperarm_r": "mixamorig:RightArm",
    "lowerarm_l": "mixamorig:LeftForeArm", "lowerarm_r": "mixamorig:RightForeArm",
    "thigh_l": "mixamorig:LeftUpLeg",    "thigh_r": "mixamorig:RightUpLeg",
    "calf_l": "mixamorig:LeftLeg",       "calf_r": "mixamorig:RightLeg",
    "neck_01": "mixamorig:Head",
    "pelvis": "mixamorig:Hips",          "spine_01": "mixamorig:Spine",
    "spine_02": "mixamorig:Spine1",      "spine_03": "mixamorig:Spine2",
    "head": "mixamorig:Head"
  }
}
```

`npc/base.glb.gore`, `npc/guard.glb.gore`, `npc/mobster.glb.gore`: add the same `"guts"` texture line and the same
`"anatomy"` block as `humanoid.gore` (they do not use `extends`). Their bone names are the humanoid ones.

Known bone names (checked in the `.glb` files): humanoid = `pelvis spine_01 spine_02 spine_03 neck_01 head
clavicle_l/r upperarm_l/r lowerarm_l/r hand_l/r thigh_l/r calf_l/r foot_l/r ball_l/r weapon`; knight =
`mixamorig:Hips Spine Spine1 Spine2 Neck Head LeftShoulder LeftArm LeftForeArm LeftHand LeftUpLeg LeftLeg LeftFoot
LeftToeBase` (+ Right).

---

## T4. `GoreFlesh.h/.cpp` — shells and anatomy (new)

All of this runs **once per model**, on the game thread, inside `GetProfile`. Expect 5-20 ms on first use of a model.
GPU buffers are created later, on the main thread, by `EnsureGpu()` (called from `GoreSkeletalMesh::FinalizeFrameData`).

### T4.1 `GoreFlesh.h` (complete file)

```cpp
#pragma once

#include "GoreTypes.h"

#include <VertexData.h>
#include <skinned_model.hpp>
#include <bgfx/bgfx.h>
#include <glm.h>

#include <memory>
#include <vector>

namespace Gore
{
    struct Profile;

    // One generated part of the anatomy mesh (a bone, a rib ring, the skull, a gut mass): a range of its index buffer.
    struct FleshPiece
    {
        uint32_t first = 0, count = 0;            // indices
        glm::vec3 bbMin = glm::vec3(0), bbMax = glm::vec3(0);   // bind space
        uint16_t skel = 0;                        // the bone it is rigidly attached to
    };

    // Generated once per model and shared by every body of that model.
    struct ModelFlesh
    {
        // Shells: per submesh (same order and size as model->meshes[i].vertices; empty = this submesh has no flesh).
        // Vertex fields: Position = model position, Normal = INWARD direction (unit), Tangent = the model's real normal,
        // ShadowMapCoords.x = max inset in metres. Bone indices / weights / uv are copied. Drawn with the model's index buffer.
        std::vector<std::vector<VertexData>> shellVertices;

        // Anatomy: one mesh for the whole model. Each vertex follows exactly one bone (weight 1).
        // Color.r = material (0 bone, 1 guts), Color.g = brightness, ShadowMapCoords.x = 1.
        std::vector<VertexData> anatomyVertices;
        std::vector<uint32_t> anatomyIndices;
        std::vector<FleshPiece> pieces;

        // GPU side. Main thread only (EnsureGpu).
        std::vector<bgfx::VertexBufferHandle> shellVbh;      // parallel to shellVertices, invalid where empty
        bgfx::VertexBufferHandle anatomyVbh = BGFX_INVALID_HANDLE;
        bgfx::IndexBufferHandle  anatomyIbh = BGFX_INVALID_HANDLE;
        bool gpuReady = false;

        void EnsureGpu();   // creates the buffers on first call; cheap afterwards
    };

    // Game thread. Needs profile.submeshRoles, anyBounds, parent, subtreeEnd and anatomy to be filled.
    std::shared_ptr<ModelFlesh> BuildModelFlesh(const roj::SkinnedModel& model, const Profile& profile);
}
```

### T4.2 `GoreFlesh.cpp` (complete file)

Write it exactly like this. Notes on the parts that are easy to get wrong follow the code.

```cpp
#include "GoreFlesh.h"
#include "GoreProfile.h"

#include <Logger.hpp>

#include <algorithm>
#include <cmath>
#include <unordered_map>

namespace Gore
{
    namespace
    {
        constexpr float kPi = 3.14159265358979f;

        // ---- ray casts against the body (bind space) -----------------------------------------------------------------

        struct Tri { glm::vec3 a, b, c; };

        bool RayTri(const glm::vec3& o, const glm::vec3& d, const Tri& t, float& out)
        {
            const glm::vec3 e1 = t.b - t.a, e2 = t.c - t.a;
            const glm::vec3 p = glm::cross(d, e2);
            const float det = glm::dot(e1, p);
            if (std::abs(det) < 1.0e-10f) return false;
            const float inv = 1.0f / det;
            const glm::vec3 s = o - t.a;
            const float u = glm::dot(s, p) * inv;
            if (u < 0.0f || u > 1.0f) return false;
            const glm::vec3 q = glm::cross(s, e1);
            const float v = glm::dot(d, q) * inv;
            if (v < 0.0f || u + v > 1.0f) return false;
            out = glm::dot(e2, q) * inv;
            return out > 1.0e-4f;
        }

        // Uniform grid over the Volume submeshes. Not thread safe (the stamp array): game thread only.
        class TriGrid
        {
        public:
            void Build(const roj::SkinnedModel& model, const Profile& profile)
            {
                glm::vec3 lo(1.0e9f), hi(-1.0e9f);
                for (size_t m = 0; m < model.meshes.size(); ++m)
                {
                    const SubmeshRole role = m < profile.submeshRoles.size() ? profile.submeshRoles[m] : SubmeshRole::Volume;
                    if (role != SubmeshRole::Volume) continue;
                    const roj::SkinnedMesh& mesh = model.meshes[m];
                    for (size_t i = 0; i + 2 < mesh.indices.size(); i += 3)
                    {
                        Tri t{ mesh.vertices[mesh.indices[i]].Position, mesh.vertices[mesh.indices[i + 1]].Position,
                               mesh.vertices[mesh.indices[i + 2]].Position };
                        tris.push_back(t);
                        lo = glm::min(lo, glm::min(t.a, glm::min(t.b, t.c)));
                        hi = glm::max(hi, glm::max(t.a, glm::max(t.b, t.c)));
                    }
                }
                if (tris.empty()) return;

                origin = lo - glm::vec3(0.01f);
                const glm::vec3 size = hi - origin + glm::vec3(0.01f);
                for (int k = 0; k < 3; ++k)
                    dims[k] = glm::clamp(static_cast<int>(std::ceil(size[k] / cell)), 1, 128);

                const size_t cells = static_cast<size_t>(dims.x) * dims.y * dims.z;
                std::vector<uint32_t> counts(cells, 0);
                auto forCells = [&](const Tri& t, auto&& fn)
                {
                    const glm::ivec3 c0 = CellOf(glm::min(t.a, glm::min(t.b, t.c)));
                    const glm::ivec3 c1 = CellOf(glm::max(t.a, glm::max(t.b, t.c)));
                    for (int z = c0.z; z <= c1.z; ++z)
                        for (int y = c0.y; y <= c1.y; ++y)
                            for (int x = c0.x; x <= c1.x; ++x)
                                fn(Index(x, y, z));
                };
                for (const Tri& t : tris) forCells(t, [&](size_t c) { ++counts[c]; });

                cellStart.assign(cells + 1, 0);
                for (size_t c = 0; c < cells; ++c) cellStart[c + 1] = cellStart[c] + counts[c];
                cellTris.assign(cellStart[cells], 0);
                std::fill(counts.begin(), counts.end(), 0);
                for (uint32_t i = 0; i < tris.size(); ++i)
                    forCells(tris[i], [&](size_t c) { cellTris[cellStart[c] + counts[c]++] = i; });

                stamp.assign(tris.size(), 0);
            }

            bool Empty() const { return tris.empty(); }

            // Distance to the nearest face along the ray (any side), -1 when nothing is hit within tMax.
            float Cast(const glm::vec3& o, const glm::vec3& d, float tMax)
            {
                if (tris.empty()) return -1.0f;
                ++stampId;
                float best = -1.0f;
                long long last = -1;
                const float step = cell * 0.25f;
                for (float s = 0.0f; s <= tMax + step; s += step)
                {
                    if (best >= 0.0f && best <= s - cell) break;   // every cell up to the hit was visited
                    const glm::vec3 p = o + d * std::min(s, tMax);
                    const glm::vec3 f = (p - origin) / cell;
                    const int x = static_cast<int>(std::floor(f.x)), y = static_cast<int>(std::floor(f.y)), z = static_cast<int>(std::floor(f.z));
                    if (x < 0 || y < 0 || z < 0 || x >= dims.x || y >= dims.y || z >= dims.z) continue;
                    const long long c = static_cast<long long>(Index(x, y, z));
                    if (c == last) continue;
                    last = c;
                    for (uint32_t k = cellStart[c]; k < cellStart[c + 1]; ++k)
                    {
                        const uint32_t ti = cellTris[k];
                        if (stamp[ti] == stampId) continue;
                        stamp[ti] = stampId;
                        float t;
                        if (RayTri(o, d, tris[ti], t) && t <= tMax && (best < 0.0f || t < best)) best = t;
                    }
                }
                return best;
            }

        private:
            glm::ivec3 CellOf(const glm::vec3& p) const
            {
                const glm::vec3 f = (p - origin) / cell;
                return glm::ivec3(glm::clamp(static_cast<int>(std::floor(f.x)), 0, dims.x - 1),
                                  glm::clamp(static_cast<int>(std::floor(f.y)), 0, dims.y - 1),
                                  glm::clamp(static_cast<int>(std::floor(f.z)), 0, dims.z - 1));
            }
            size_t Index(int x, int y, int z) const { return (static_cast<size_t>(z) * dims.y + y) * dims.x + x; }

            std::vector<Tri> tris;
            glm::vec3 origin = glm::vec3(0);
            float cell = 0.06f;
            glm::ivec3 dims = glm::ivec3(1);
            std::vector<uint32_t> cellStart, cellTris, stamp;
            uint32_t stampId = 0;
        };

        // ---- shells ----------------------------------------------------------------------------------------------------

        uint64_t WeldKey(const glm::vec3& p)
        {
            // 0.5 mm cells; 21 bits per axis (+-524 m).
            const auto q = [](float v) { return static_cast<uint64_t>(static_cast<int64_t>(std::llround(v * 2000.0f)) & 0x1FFFFF); };
            return q(p.x) | (q(p.y) << 21) | (q(p.z) << 42);
        }

        void BuildShells(const roj::SkinnedModel& model, const Profile& profile, TriGrid& grid, ModelFlesh& out)
        {
            const size_t meshCount = model.meshes.size();
            out.shellVertices.assign(meshCount, {});

            auto isVolume = [&](size_t m)
            {
                const SubmeshRole role = m < profile.submeshRoles.size() ? profile.submeshRoles[m] : SubmeshRole::Volume;
                return role == SubmeshRole::Volume;
            };

            // 1. Weld by position: vertices split by uv seams or hard edges must move together, or the shell cracks open.
            std::unordered_map<uint64_t, uint32_t> weld;
            std::vector<glm::vec3> position, normalSum;
            std::vector<std::vector<uint32_t>> weldId(meshCount);
            for (size_t m = 0; m < meshCount; ++m)
            {
                if (!isVolume(m)) continue;
                const roj::SkinnedMesh& mesh = model.meshes[m];
                weldId[m].resize(mesh.vertices.size());
                for (size_t v = 0; v < mesh.vertices.size(); ++v)
                {
                    const VertexData& vd = mesh.vertices[v];
                    const auto it = weld.emplace(WeldKey(vd.Position), static_cast<uint32_t>(position.size()));
                    if (it.second) { position.push_back(vd.Position); normalSum.push_back(glm::vec3(0)); }
                    weldId[m][v] = it.first->second;
                    normalSum[it.first->second] += vd.Normal;
                }
            }

            // 2. Inward direction and max inset per welded point: 40 % of the body thickness behind it, at most 7 cm.
            //    A sheet thinner than 2 cm has no flesh (max inset 0: the shell shader discards it).
            const size_t n = position.size();
            std::vector<glm::vec3> inward(n, glm::vec3(0));
            std::vector<float> inset(n, 0.0f);
            for (size_t i = 0; i < n; ++i)
            {
                const float len = glm::length(normalSum[i]);
                if (len < 1.0e-5f) continue;
                inward[i] = -normalSum[i] / len;
                const float t = grid.Cast(position[i] + inward[i] * 0.004f, inward[i], 0.6f);
                const float thickness = t < 0.0f ? 0.3f : t + 0.004f;
                inset[i] = thickness < 0.02f ? 0.0f : std::min(0.4f * thickness, 0.07f);
            }

            // 3. No spikes: a point never goes much deeper than its shallowest neighbour (one pass, from a copy).
            {
                std::vector<float> neighbourMin(n, 1.0e9f);
                for (size_t m = 0; m < meshCount; ++m)
                {
                    if (!isVolume(m)) continue;
                    const roj::SkinnedMesh& mesh = model.meshes[m];
                    for (size_t i = 0; i + 2 < mesh.indices.size(); i += 3)
                    {
                        const uint32_t id[3] = { weldId[m][mesh.indices[i]], weldId[m][mesh.indices[i + 1]], weldId[m][mesh.indices[i + 2]] };
                        for (int a = 0; a < 3; ++a)
                            for (int b = 0; b < 3; ++b)
                                if (id[a] != id[b]) neighbourMin[id[a]] = std::min(neighbourMin[id[a]], inset[id[b]]);
                    }
                }
                for (size_t i = 0; i < n; ++i)
                    if (neighbourMin[i] < 1.0e8f) inset[i] = std::min(inset[i], neighbourMin[i] * 1.5f + 0.004f);
            }

            // 4. Shell vertices.
            for (size_t m = 0; m < meshCount; ++m)
            {
                if (!isVolume(m)) continue;
                const roj::SkinnedMesh& mesh = model.meshes[m];
                std::vector<VertexData>& dst = out.shellVertices[m];
                dst = mesh.vertices;
                for (size_t v = 0; v < dst.size(); ++v)
                {
                    const uint32_t id = weldId[m][v];
                    dst[v].Tangent = mesh.vertices[v].Normal;          // the real normal (for lighting)
                    dst[v].Normal = inward[id];                        // the direction the shell moves in
                    dst[v].ShadowMapCoords = glm::vec2(inset[id], 0.0f);
                }
            }
        }

        // ---- anatomy ---------------------------------------------------------------------------------------------------

        struct Builder
        {
            std::vector<VertexData>& v;
            std::vector<uint32_t>& ix;
            std::vector<FleshPiece>& pieces;
            uint32_t pieceFirstIndex = 0, pieceFirstVertex = 0;

            void Begin() { pieceFirstIndex = static_cast<uint32_t>(ix.size()); pieceFirstVertex = static_cast<uint32_t>(v.size()); }

            void End(uint16_t skel)
            {
                if (ix.size() == pieceFirstIndex) return;
                FleshPiece p;
                p.first = pieceFirstIndex;
                p.count = static_cast<uint32_t>(ix.size()) - pieceFirstIndex;
                p.skel = skel;
                p.bbMin = glm::vec3(1.0e9f);
                p.bbMax = glm::vec3(-1.0e9f);
                for (size_t i = pieceFirstVertex; i < v.size(); ++i)
                {
                    p.bbMin = glm::min(p.bbMin, v[i].Position);
                    p.bbMax = glm::max(p.bbMax, v[i].Position);
                }
                pieces.push_back(p);
            }

            uint32_t Vert(const glm::vec3& pos, const glm::vec3& normal, const glm::vec2& uv, int skin, float material, float shade)
            {
                VertexData d;
                d.Position = pos;
                d.Normal = normal;
                d.TextureCoordinate = uv;
                d.BlendIndices[0] = static_cast<float>(skin);
                d.BlendWeights = glm::vec4(1, 0, 0, 0);
                d.Color = glm::vec4(material, shade, 0.0f, 1.0f);
                d.ShadowMapCoords = glm::vec2(1.0f, 0.0f);
                v.push_back(d);
                return static_cast<uint32_t>(v.size() - 1);
            }

            // A tube along `points` with a radius per point. closed = the last point joins the first (a ring).
            // `side` is any vector not parallel to the tube (the rings are built around the tube with it).
            void Tube(const std::vector<glm::vec3>& points, const std::vector<float>& radii, int sides, bool closed,
                      const glm::vec3& side, int skin, float material, float shade, float uvPerMetre)
            {
                const int count = static_cast<int>(points.size());
                if (count < 2) return;
                const uint32_t base = static_cast<uint32_t>(v.size());
                float along = 0.0f;
                for (int i = 0; i < count; ++i)
                {
                    const glm::vec3& prev = points[closed ? (i + count - 1) % count : std::max(i - 1, 0)];
                    const glm::vec3& next = points[closed ? (i + 1) % count : std::min(i + 1, count - 1)];
                    glm::vec3 tangent = next - prev;
                    tangent = glm::length(tangent) > 1.0e-6f ? glm::normalize(tangent) : glm::vec3(0, 1, 0);
                    glm::vec3 x = glm::cross(side, tangent);
                    if (glm::length(x) < 1.0e-4f) x = glm::cross(glm::vec3(0.31f, 0.52f, 0.79f), tangent);
                    x = glm::normalize(x);
                    const glm::vec3 y = glm::cross(tangent, x);
                    if (i > 0) along += glm::distance(points[i], points[i - 1]);
                    for (int s = 0; s <= sides; ++s)
                    {
                        const float a = 2.0f * kPi * static_cast<float>(s) / sides;
                        const glm::vec3 nrm = x * std::cos(a) + y * std::sin(a);
                        Vert(points[i] + nrm * radii[i], nrm, glm::vec2(static_cast<float>(s) / sides, along * uvPerMetre), skin, material, shade);
                    }
                }
                const int ringStride = sides + 1;
                const int segments = closed ? count : count - 1;
                for (int i = 0; i < segments; ++i)
                {
                    const int j = (i + 1) % count;
                    for (int s = 0; s < sides; ++s)
                    {
                        const uint32_t a = base + i * ringStride + s, b = a + 1;
                        const uint32_t c = base + j * ringStride + s, d = c + 1;
                        ix.insert(ix.end(), { a, c, b, b, c, d });
                    }
                }
                if (!closed)   // end caps: a fan around each end point
                {
                    for (int end = 0; end < 2; ++end)
                    {
                        const int i = end == 0 ? 0 : count - 1;
                        const glm::vec3 dir = glm::normalize(end == 0 ? points[0] - points[1] : points[count - 1] - points[count - 2]);
                        const uint32_t centre = Vert(points[i] + dir * radii[i] * 0.4f, dir, glm::vec2(0.5f, 0.5f), skin, material, shade);
                        for (int s = 0; s < sides; ++s)
                        {
                            const uint32_t a = base + i * ringStride + s;
                            ix.insert(ix.end(), { centre, a, a + 1 });
                        }
                    }
                }
            }

            // A lumpy ellipsoid. axes: columns = the three axis directions (unit). lump: 0 = smooth, 0.2 = knobbly.
            void Blob(const glm::vec3& centre, const glm::mat3& axes, const glm::vec3& radii, float lump, float seed,
                      int segments, int rings, int skin, float material, float shade, float uvRepeat)
            {
                const uint32_t base = static_cast<uint32_t>(v.size());
                for (int r = 0; r <= rings; ++r)
                {
                    const float phi = kPi * static_cast<float>(r) / rings;
                    for (int s = 0; s <= segments; ++s)
                    {
                        const float theta = 2.0f * kPi * static_cast<float>(s) / segments;
                        const glm::vec3 unit(std::sin(phi) * std::cos(theta), std::cos(phi), std::sin(phi) * std::sin(theta));
                        // Periodic in theta (integer multiples), so the seam at s = 0 / segments closes.
                        const float bump = 1.0f + lump * (0.6f * std::sin(3.0f * theta + seed) * std::sin(2.0f * phi + seed * 1.7f)
                                                        + 0.4f * std::sin(5.0f * theta + seed * 2.3f) * std::sin(4.0f * phi + seed));
                        const glm::vec3 local = unit * radii * bump;
                        const glm::vec3 nrm = glm::normalize(axes * (unit / glm::max(radii, glm::vec3(1.0e-4f))));
                        Vert(centre + axes * local, nrm,
                             glm::vec2(static_cast<float>(s) / segments * uvRepeat, static_cast<float>(r) / rings * uvRepeat * 0.5f),
                             skin, material, shade);
                    }
                }
                const int stride = segments + 1;
                for (int r = 0; r < rings; ++r)
                    for (int s = 0; s < segments; ++s)
                    {
                        const uint32_t a = base + r * stride + s, b = a + 1, c = a + stride, d = c + 1;
                        ix.insert(ix.end(), { a, c, b, b, c, d });
                    }
            }
        };

        glm::vec3 JointBind(const roj::SkinnedModel& model, uint16_t skel)
        {
            return glm::vec3(glm::inverse(model.skeleton.bones[skel].invBind)[3]);
        }

        bool Contains(const std::vector<uint16_t>& list, uint16_t value)
        {
            return std::find(list.begin(), list.end(), value) != list.end();
        }

        // The child of b (a skin bone) that continues it best along `prefer` (unit). -1 if it has no skin-bone child.
        int ChildAlong(const roj::SkinnedModel& model, const Profile& profile, uint16_t b, const glm::vec3& prefer)
        {
            int best = -1;
            float bestDot = -2.0f;
            const glm::vec3 pb = JointBind(model, b);
            for (uint16_t c = b + 1; c < profile.subtreeEnd[b]; ++c)
            {
                if (profile.parent[c] != b || model.skeleton.bones[c].skinIdx < 0) continue;
                const glm::vec3 d = JointBind(model, c) - pb;
                const float len = glm::length(d);
                if (len < 1.0e-4f) continue;
                const float dt = glm::dot(d / len, prefer);
                if (dt > bestDot) { bestDot = dt; best = c; }
            }
            return best;
        }

        // Limb radius around a point: median of 6 rays square to `axis`. -1 when fewer than 4 rays hit within 0.3 m
        // (the point is not inside the body: a weapon bone, an IK bone).
        float RadiusAround(TriGrid& grid, const glm::vec3& point, const glm::vec3& axis)
        {
            glm::vec3 x = glm::cross(axis, std::abs(axis.y) < 0.9f ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0));
            x = glm::normalize(x);
            const glm::vec3 y = glm::cross(axis, x);
            float d[6];
            int hits = 0;
            for (int i = 0; i < 6; ++i)
            {
                const float a = 2.0f * kPi * static_cast<float>(i) / 6.0f;
                const float t = grid.Cast(point, x * std::cos(a) + y * std::sin(a), 0.3f);
                if (t > 0.0f) d[hits++] = t;
            }
            if (hits < 4) return -1.0f;
            std::sort(d, d + hits);
            return d[hits / 2];
        }

        void BuildAnatomy(const roj::SkinnedModel& model, const Profile& profile, TriGrid& grid, ModelFlesh& out)
        {
            const AnatomyConfig& cfg = profile.anatomy;
            const glm::vec3 up = cfg.up, forward = cfg.forward;
            const glm::vec3 right = glm::normalize(glm::cross(up, forward));
            const int boneCount = model.skeleton.boneCount;
            Builder B{ out.anatomyVertices, out.anatomyIndices, out.pieces };

            // 1. A bone inside every limb segment (parent joint -> child joint), sized by the limb around it.
            for (int b = 0; b < boneCount; ++b)
            {
                const int skin = model.skeleton.bones[b].skinIdx;
                if (skin < 0 || Contains(cfg.torso, static_cast<uint16_t>(b))) continue;
                const glm::vec3 pb = JointBind(model, static_cast<uint16_t>(b));
                for (uint16_t c = static_cast<uint16_t>(b + 1); c < profile.subtreeEnd[b]; ++c)
                {
                    if (profile.parent[c] != b || model.skeleton.bones[c].skinIdx < 0) continue;
                    const glm::vec3 pc = JointBind(model, c);
                    const float len = glm::distance(pb, pc);
                    if (len < cfg.minBoneLength) continue;
                    const glm::vec3 axis = (pc - pb) / len;
                    const float limb = RadiusAround(grid, (pb + pc) * 0.5f, axis);
                    if (limb < 0.0f || limb > 0.14f) continue;
                    const float r = glm::clamp(0.28f * limb, 0.006f, 0.03f);

                    static const float kT[7] = { 0.00f, 0.04f, 0.12f, 0.50f, 0.88f, 0.96f, 1.00f };
                    static const float kR[7] = { 1.20f, 1.70f, 1.00f, 0.88f, 1.00f, 1.70f, 1.20f };   // knobs at both ends
                    std::vector<glm::vec3> pts;
                    std::vector<float> radii;
                    for (int i = 0; i < 7; ++i) { pts.push_back(pb + axis * (len * kT[i])); radii.push_back(r * kR[i]); }

                    B.Begin();
                    B.Tube(pts, radii, 6, false, std::abs(axis.y) < 0.9f ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0), skin, 0.0f, 1.0f, 6.0f);
                    B.End(static_cast<uint16_t>(b));
                }
            }

            // 2. Trunk bones: a spine at the back of the body, made of vertebrae (alternating thick / thin).
            for (uint16_t b : cfg.torso)
            {
                const int skin = model.skeleton.bones[b].skinIdx;
                const int c = ChildAlong(model, profile, b, up);
                if (c < 0) continue;
                const glm::vec3 pb = JointBind(model, b), pc = JointBind(model, static_cast<uint16_t>(c));
                const float len = glm::distance(pb, pc);
                if (len < 0.02f) continue;
                const float back = grid.Cast((pb + pc) * 0.5f, -forward, 0.4f);
                const glm::vec3 offset = back > 0.0f ? -forward * (back * 0.6f) : glm::vec3(0);

                const int steps = std::max(2, static_cast<int>(len / 0.022f));
                std::vector<glm::vec3> pts;
                std::vector<float> radii;
                for (int i = 0; i <= steps; ++i)
                {
                    pts.push_back(glm::mix(pb, pc, static_cast<float>(i) / steps) + offset);
                    radii.push_back((i % 2 == 0) ? 0.024f : 0.013f);
                }
                B.Begin();
                B.Tube(pts, radii, 6, false, forward, skin, 0.0f, 0.95f, 8.0f);
                B.End(b);
            }

            // 3. Ribs: rings fitted to the chest cross-section, a breastbone at the front, dark organs inside.
            for (uint16_t b : cfg.ribs)
            {
                const int skin = model.skeleton.bones[b].skinIdx;
                const int c = ChildAlong(model, profile, b, up);
                if (c < 0) continue;
                const glm::vec3 pb = JointBind(model, b), pc = JointBind(model, static_cast<uint16_t>(c));
                const float len = glm::distance(pb, pc);
                if (len < 0.03f) continue;

                const int ringCount = std::max(1, static_cast<int>(len / 0.045f));
                constexpr int kDirs = 16;
                std::vector<glm::vec3> fronts;
                glm::vec3 meanRadii(0.0f);
                glm::vec3 meanCentre(0.0f);

                B.Begin();
                for (int r = 0; r < ringCount; ++r)
                {
                    const glm::vec3 centre = glm::mix(pb, pc, (static_cast<float>(r) + 0.5f) / ringCount);
                    float dist[kDirs];
                    float sorted[kDirs];
                    for (int i = 0; i < kDirs; ++i)
                    {
                        const float a = 2.0f * kPi * static_cast<float>(i) / kDirs;
                        const float t = grid.Cast(centre, right * std::cos(a) + forward * std::sin(a), 0.4f);
                        dist[i] = t > 0.0f ? t : 0.12f;
                        sorted[i] = dist[i];
                    }
                    std::sort(sorted, sorted + kDirs);
                    const float median = sorted[kDirs / 2];
                    for (int i = 0; i < kDirs; ++i) dist[i] = std::min(dist[i], median * 1.5f);   // a ray that ran into an arm

                    std::vector<glm::vec3> pts;
                    std::vector<float> radii;
                    for (int i = 0; i < kDirs; ++i)
                    {
                        const float a = 2.0f * kPi * static_cast<float>(i) / kDirs;
                        const float smooth = 0.5f * dist[i] + 0.25f * (dist[(i + 1) % kDirs] + dist[(i + kDirs - 1) % kDirs]);
                        const glm::vec3 dir = right * std::cos(a) + forward * std::sin(a);
                        // 80 % of the way to the skin; ribs slope down towards the front.
                        pts.push_back(centre + dir * (smooth * 0.80f) - up * (0.03f * std::max(0.0f, std::sin(a))));
                        radii.push_back(0.009f);
                    }
                    B.Tube(pts, radii, 4, true, up, skin, 0.0f, 1.0f, 6.0f);

                    fronts.push_back(pts[kDirs / 4]);   // a = 90 degrees = straight forward
                    meanRadii += glm::vec3(dist[0] + dist[kDirs / 2], 0.0f, dist[kDirs / 4] + dist[3 * kDirs / 4]) * 0.5f;
                    meanCentre += centre + forward * ((dist[kDirs / 4] - dist[3 * kDirs / 4]) * 0.5f);
                }
                if (fronts.size() >= 2)
                {
                    std::vector<float> radii(fronts.size(), 0.014f);
                    B.Tube(fronts, radii, 4, false, forward, skin, 0.0f, 1.0f, 6.0f);
                }
                B.End(b);

                // Chest organs (lungs / heart): one dark lumpy mass that fills the ribs.
                meanRadii /= static_cast<float>(ringCount);
                meanCentre /= static_cast<float>(ringCount);
                B.Begin();
                B.Blob(meanCentre, glm::mat3(right, up, forward), glm::vec3(meanRadii.x * 0.62f, len * 0.6f, meanRadii.z * 0.62f),
                       0.14f, static_cast<float>(b) * 1.3f, 10, 6, skin, 1.0f, 0.55f, 2.0f);
                B.End(b);
            }

            // 4. Guts: a knobbly mass in the belly.
            for (uint16_t b : cfg.guts)
            {
                const int skin = model.skeleton.bones[b].skinIdx;
                const int c = ChildAlong(model, profile, b, up);
                const glm::vec3 pb = JointBind(model, b);
                const glm::vec3 pc = c >= 0 ? JointBind(model, static_cast<uint16_t>(c)) : pb + up * 0.12f;
                const float len = std::max(glm::distance(pb, pc), 0.08f);
                glm::vec3 centre = (pb + pc) * 0.5f;

                auto reach = [&](const glm::vec3& d) { const float t = grid.Cast(centre, d, 0.4f); return t > 0.0f ? t : 0.12f; };
                const float rx = std::min(reach(right), reach(-right));
                const float front = reach(forward), backD = reach(-forward);
                centre += forward * ((front - backD) * 0.5f);

                B.Begin();
                B.Blob(centre, glm::mat3(right, up, forward), glm::vec3(rx * 0.62f, len * 0.70f, (front + backD) * 0.5f * 0.62f),
                       0.20f, static_cast<float>(b) * 2.1f + 0.7f, 10, 6, skin, 1.0f, 1.0f, 3.0f);
                B.End(b);
            }

            // 5. Skull: fitted into the head.
            if (cfg.head >= 0 && cfg.head < static_cast<int>(profile.anyBounds.size()) && profile.anyBounds[cfg.head].hasData)
            {
                const uint16_t h = static_cast<uint16_t>(cfg.head);
                const int skin = model.skeleton.bones[h].skinIdx;
                const roj::BoneBound& bb = profile.anyBounds[h];
                const glm::vec3 joint = JointBind(model, h);
                // The head bone also carries part of the neck: the skull sits in the upper part of its bounds.
                glm::vec3 centre = (bb.bbMin + bb.bbMax) * 0.5f;
                const float top = glm::dot(bb.bbMax - joint, glm::abs(up));       // rough height of the head above its joint
                centre = joint + up * (std::max(top, 0.1f) * 0.55f) + (centre - joint - up * glm::dot(centre - joint, up));

                auto reach = [&](const glm::vec3& d, float fallback) { const float t = grid.Cast(centre, d, 0.3f); return t > 0.0f ? t : fallback; };
                const float rx = 0.5f * (reach(right, 0.08f) + reach(-right, 0.08f));
                const float ry = reach(up, 0.1f);
                const float front = reach(forward, 0.1f), backD = reach(-forward, 0.1f);
                centre += forward * ((front - backD) * 0.5f);

                B.Begin();
                B.Blob(centre, glm::mat3(right, up, forward), glm::vec3(rx, ry, (front + backD) * 0.5f) * 0.80f,
                       0.03f, 0.4f, 12, 8, skin, 0.0f, 1.0f, 2.0f);
                B.End(h);
            }
        }
    }

    void ModelFlesh::EnsureGpu()
    {
        if (gpuReady) return;
        gpuReady = true;

        const bgfx::VertexLayout layout = VertexData::Declaration();
        shellVbh.assign(shellVertices.size(), BGFX_INVALID_HANDLE);
        for (size_t i = 0; i < shellVertices.size(); ++i)
        {
            if (shellVertices[i].empty()) continue;
            shellVbh[i] = bgfx::createVertexBuffer(
                bgfx::copy(shellVertices[i].data(), static_cast<uint32_t>(sizeof(VertexData) * shellVertices[i].size())), layout);
        }
        if (!anatomyVertices.empty() && !anatomyIndices.empty())
        {
            anatomyVbh = bgfx::createVertexBuffer(
                bgfx::copy(anatomyVertices.data(), static_cast<uint32_t>(sizeof(VertexData) * anatomyVertices.size())), layout);
            anatomyIbh = bgfx::createIndexBuffer(
                bgfx::copy(anatomyIndices.data(), static_cast<uint32_t>(sizeof(uint32_t) * anatomyIndices.size())), BGFX_BUFFER_INDEX32);
        }
    }

    std::shared_ptr<ModelFlesh> BuildModelFlesh(const roj::SkinnedModel& model, const Profile& profile)
    {
        auto flesh = std::make_shared<ModelFlesh>();

        TriGrid grid;
        grid.Build(model, profile);
        if (grid.Empty())
        {
            flesh->shellVertices.assign(model.meshes.size(), {});
            return flesh;   // no Volume submesh: wounds are holes only
        }

        BuildShells(model, profile, grid, *flesh);
        BuildAnatomy(model, profile, grid, *flesh);

        Logger::Log("Gore: flesh built: " + std::to_string(flesh->pieces.size()) + " anatomy pieces, "
                    + std::to_string(flesh->anatomyVertices.size()) + " anatomy vertices");
        return flesh;
    }
}
```

Things that are easy to get wrong here:

* **Skin index vs skel index.** `BlendIndices[0]` takes the **skin** index (`skeleton.bones[skel].skinIdx`).
  `FleshPiece::skel`, `cfg.torso`, `profile.parent`, `profile.subtreeEnd` are **skel** indices.
* The shell stores the **inward** direction in `Normal` and the real normal in `Tangent`. The shader depends on it.
* The weld is what keeps the shell closed. Do not compute the inward direction per vertex.
* `glm::mat3(right, up, forward)` takes the three vectors as **columns**; `axes * local` then maps local x to `right`.
* The index type is 32 bit for the anatomy (`BGFX_BUFFER_INDEX32`). Model index buffers are reused as they are.
* `std::llround` needs `<cmath>`.

**Check (after T5-T8 compile):** the log shows `Gore: flesh built: N anatomy pieces` with N around 30-40 for a human
(about 19 limb bones, 4 spine pieces, 2 rib sets, 2 chest organs, 2 gut masses, 1 skull). N = 0 or a missing
anatomy-bone warning means the bone names in the `.gore` file are wrong.

---

## T5. Shaders

### T5.1 `gore_common.sh` — replace the whole file

```glsl
#ifndef GORE_COMMON_SH
#define GORE_COMMON_SH

#define GORE_MAX_WOUNDS 8
#define GORE_MAX_CUTS 4
#define GORE_MAX_SPLATS 16

uniform vec4 u_goreCounts;        // x wounds, y cuts, z splats
uniform vec4 u_goreParams;        // x unused, y isGib, z fleshScale, w gore level (0 full, 1 reduced, 2 minimal)
uniform vec4 u_goreRangeMask;     // per index range: x wound bits, y cut bits, z splat bits
uniform vec4 u_goreLayer;         // x layer index (0 skin, 1..3 shells, 4 anatomy), y inset fraction, z 1 = anatomy
uniform vec4 u_goreBloodColor;
uniform vec4 u_goreCameraPos;
uniform vec4 u_goreWoundBind[24];
uniform vec4 u_goreWoundParams[8];   // x roughness, y rimWidth, z healScale, w seed
uniform vec4 u_goreWoundExtra[8];    // x taper
uniform vec4 u_goreCutBind[4];
uniform vec4 u_goreCutBindSphere[4];
uniform vec4 u_goreCutRegrowBind[4];
uniform vec4 u_goreCutParams[4];
uniform vec4 u_goreSplats[16];

bool goreBit(float mask, int i) { return mod(floor(mask / exp2(float(i))), 2.0) >= 1.0; }

// Edge noise in [-1, 1]. Lod 0: called inside loops/branches with discard.
float goreNoise(vec2 uv) { return texture2DLod(u_goreEdgeNoise, uv, 0.0).r * 2.0 - 1.0; }

vec3 goreWoundLocal(vec4 r0, vec4 r1, vec4 r2, vec3 p)
{
    vec4 p4 = vec4(p, 1.0);
    return vec3(dot(r0, p4), dot(r1, p4), dot(r2, p4));
}

void gorePlaneBasis(vec3 n, out vec3 t, out vec3 b)
{
    vec3 up = abs(n.y) < 0.99 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
    t = normalize(cross(n, up));
    b = cross(n, t);
}

// Is bind point p removed by cut i (body side)? Also returns the fresh-growth amount (0..1).
bool goreCutRemoves(int i, vec3 p, out float fresh)
{
    fresh = 0.0;
    vec4 pl = u_goreCutBind[i];
    vec4 sph = u_goreCutBindSphere[i];
    if (length(p - sph.xyz) > sph.w) return false;
    vec4 cp = u_goreCutParams[i];
    vec3 t; vec3 b; gorePlaneBasis(pl.xyz, t, b);
    float edge = goreNoise(vec2(dot(p, t), dot(p, b)) * 6.0 + cp.y) * cp.x;
    float d = dot(pl.xyz, p) - pl.w;
    if (d <= -edge) return false;                       // kept side
    vec4 rg = u_goreCutRegrowBind[i];
    if (rg.w > 0.0)
    {
        float g = rg.w - length(p - rg.xyz) + goreNoise(p.xy * 9.0 + p.z * 3.0 + cp.y) * 0.01;
        if (g > 0.0) { fresh = 1.0 - smoothstep(0.0, 0.05, g); return false; }   // already regrown
    }
    return true;
}

// Surface test in bind space for the layer in u_goreLayer.x. true = discard.
// rim: 0..1 near a hole edge of this layer. soak: blood soaked in (skin). fresh: regrowing.
bool goreSurface(vec3 p, out float rim, out float soak, out float fresh)
{
    rim = 0.0; soak = 0.0; fresh = 0.0;
    bool isGib = u_goreParams.y > 0.5;
    float layer = u_goreLayer.x;
    int nw = int(u_goreCounts.x + 0.5);
    int nc = int(u_goreCounts.y + 0.5);
    int ns = int(u_goreCounts.z + 0.5);

    for (int i = 0; i < GORE_MAX_CUTS; i++)
    {
        if (i >= nc) break;
        if (!goreBit(u_goreRangeMask.y, i)) { if (isGib) return true; continue; }
        float f;
        bool removedForBody = goreCutRemoves(i, p, f);
        fresh = max(fresh, f);
        if (isGib ? !removedForBody : removedForBody) return true;
    }
    for (int i = 0; i < GORE_MAX_WOUNDS; i++)
    {
        if (i >= nw) break;
        if (!goreBit(u_goreRangeMask.x, i)) continue;
        vec4 wp = u_goreWoundParams[i];
        float scale = wp.z * max(0.0, 1.0 - layer * u_goreWoundExtra[i].x);   // deeper layers: smaller hole
        if (scale < 0.02) continue;
        vec3 q = goreWoundLocal(u_goreWoundBind[i*3], u_goreWoundBind[i*3+1], u_goreWoundBind[i*3+2], p) / scale;
        float r = length(q);
        // Two octaves, a different pattern per layer, so the layer edges never line up.
        vec2 nuv = q.xy * 0.5 + 0.5 + wp.w + layer * 0.37;
        float edge = (goreNoise(nuv) * 0.7 + goreNoise(nuv * 2.7 + 0.31) * 0.3) * wp.x;
        if (r < 1.0 + edge) return true;
        rim = max(rim, (1.0 - smoothstep(1.0 + edge, 1.0 + edge + wp.y, r)) * min(wp.z * 2.0, 1.0));
    }
    for (int i = 0; i < GORE_MAX_SPLATS; i++)
    {
        if (i >= ns) break;
        if (!goreBit(u_goreRangeMask.z, i)) continue;
        vec4 s = u_goreSplats[i];
        float k = 1.0 - smoothstep(0.0, max(s.w, 0.0001), length(p - s.xyz));
        soak = max(soak, k * (0.6 + 0.4 * texture2DLod(u_goreBlood, p.xy * 7.0 + p.z * 2.0, 0.0).r));
    }
    return false;
}

#endif
```

Removed on purpose: decals, the ray/interval code, `goreDepth`, the world-space wound and cut uniforms.

### T5.2 `fs_gore.sc` (skin)

Two edits only:

1. Delete the decal lines: `float decalHole = ...; float decalRing = ...;`, the `if (u_goreCounts.w > 0.5)
   goreDecals(...)` line, and the two `color = mix(...)` lines that use `decalRing` and `decalHole`.
2. Replace the blood mix line so the rim is darker and reads as torn flesh:

```glsl
    float blood = u_goreParams.w > 1.5 ? 0.0 : max(goreRim, soak);
    color = mix(color, u_goreBloodColor.rgb * (0.55 + 0.45 * soak), blood);
    color = mix(color, u_goreBloodColor.rgb * 0.35, goreRim * goreRim * 0.8);   // the torn edge itself is almost black-red
```

`fs_gore_depth.sc` and `fs_gore_customId.sc` need no edit (they only call `goreSurface`). `vs_gore.sc` needs no edit.
The skin draw sets `u_goreLayer = (0,0,0,0)` (T6).

### T5.3 `vs_gore_inner.sc` (new: shells and anatomy)

```glsl
$input a_position, a_normal, a_texcoord0, a_texcoord2, a_indices, a_weight, a_color0, a_tangent
$output v_texcoord0, v_color0, v_normal, v_world, v_texcoord1, v_goreBind

#include <../bgfx_shader.sh>

uniform mat4 projection;
uniform mat4 view;
uniform mat4 world;
uniform vec4 viewmodelScaleFactor;
uniform vec4 isViewmodel;
uniform vec4 u_goreLayer;   // x layer, y inset fraction, z 1 = anatomy

#define MAX_BONES 128
uniform mat4 finalBonesMatrices[MAX_BONES];

mat4 GetBoneTransforms(vec4 indices, vec4 weights)
{
    float sum = weights.x + weights.y + weights.z + weights.w;
    if (sum < 0.01)
        return mat4(1.0, 0.0, 0.0, 0.0,  0.0, 1.0, 0.0, 0.0,  0.0, 0.0, 1.0, 0.0,  0.0, 0.0, 0.0, 1.0);
    return
        finalBonesMatrices[int(indices.x)] * (weights.x / sum) +
        finalBonesMatrices[int(indices.y)] * (weights.y / sum) +
        finalBonesMatrices[int(indices.z)] * (weights.z / sum) +
        finalBonesMatrices[int(indices.w)] * (weights.w / sum);
}

void main()
{
    bool anatomy = u_goreLayer.z > 0.5;

    // Shell: a_normal is the INWARD direction, a_tangent the real normal, a_texcoord2.x the max inset (metres).
    // Anatomy: a_normal is the real normal and nothing moves.
    vec3 bindPos = anatomy ? a_position : a_position + a_normal * (a_texcoord2.x * u_goreLayer.y);
    vec3 bindNormal = anatomy ? a_normal : a_tangent;

    mat4 boneTrans      = GetBoneTransforms(a_indices, a_weight);
    mat4 vertWorldTrans = mul(world, boneTrans);

    vec4 worldPos = mul(vertWorldTrans, vec4(bindPos, 1.0));
    v_world = worldPos;

    vec4 clipPos = mul(projection, mul(view, worldPos));
    if (isViewmodel.x > 0.5)
        clipPos.z *= 0.2 * viewmodelScaleFactor.x;
    gl_Position = clipPos;

    v_normal    = normalize(mul(vertWorldTrans, vec4(bindNormal, 0.0)).xyz);
    v_color0    = a_color0;                    // raw: anatomy keeps its material id in .r
    v_texcoord0 = a_texcoord0;
    v_texcoord1 = a_texcoord2;
    v_goreBind  = vec4(bindPos, anatomy ? 1.0 : a_texcoord2.x);   // w: max inset (shells) - 0 means "no flesh here"
}
```

The copy the vertex shader uses for the header (`$input` names, `mul`, `MAX_BONES 128`, uniform names) is
`vs_gore.sc`; if a compile error names an attribute, compare with that file.

### T5.4 `fs_gore_inner.sc` (new)

```glsl
$input v_texcoord0, v_color0, v_normal, v_world, v_texcoord1, v_goreBind

#include <../bgfx_shader.sh>

SAMPLER2D(u_texture,         0);
SAMPLER2D(u_textureEmissive, 1);

#include "../lighting_common.sh"
// Sampler slots are parsed from this file's own text by the engine (includes are not scanned), so they are declared here.
SAMPLER2D(u_goreFlesh,       2);
SAMPLER2D(u_goreFleshNormal, 3);
SAMPLER2D(u_goreEdgeNoise,   4);
SAMPLER2D(u_goreBlood,       5);
SAMPLER2D(u_goreBone,        6);
SAMPLER2D(u_goreGuts,        7);
#include "gore_common.sh"
#include "gore_flesh.sh"

void main()
{
    bool anatomy = u_goreLayer.z > 0.5;
    if (!anatomy && v_goreBind.w < 0.004) discard;          // a thin sheet has no flesh behind it

    float rim = 0.0; float soak = 0.0; float fresh = 0.0;
    bool anyBits = (u_goreRangeMask.x + u_goreRangeMask.y) > 0.0;
    if (anyBits && goreSurface(v_goreBind.xyz, rim, soak, fresh)) discard;

    vec3 P = v_world.xyz;
    vec3 viewDir = normalize(u_goreCameraPos.xyz - P);
    vec3 N = normalize(v_normal);
    float inside = 0.0;                                      // 1: we look at the inner side of this surface
    if (dot(N, viewDir) < 0.0) { N = -N; inside = 1.0; }

    vec3 bind = v_goreBind.xyz;
    vec3 albedo;
    float wet = 1.0;
    float gloss = 1.0;

    if (anatomy)
    {
        vec3 boneC = texture2DLod(u_goreBone, v_texcoord0, 0.0).rgb;
        vec3 gutC  = texture2DLod(u_goreGuts, v_texcoord0, 0.0).rgb;
        float isGuts = step(0.5, v_color0.r);
        // Bone: off-white with blood in the pores and a lot of blood near a break.
        float stain = texture2DLod(u_goreBlood, v_texcoord0 * 1.7 + bind.xy * 3.0, 0.0).r;
        vec3 bone = mix(boneC * vec3(1.0, 0.93, 0.82), u_goreBloodColor.rgb * 0.9, clamp(stain * 0.55 + rim * 0.8, 0.0, 0.95));
        albedo = mix(bone, gutC * 1.15, isGuts) * v_color0.g;
        wet = mix(0.55, 1.0, isGuts);
        gloss = mix(0.7, 1.0, isGuts);
    }
    else
    {
        float layer = u_goreLayer.x;
        albedo = goreTriplanar(bind + vec3(layer * 0.13, layer * 0.07, 0.0), N, u_goreParams.z);
        N = goreBumpTriplanar(N, bind, u_goreParams.z * 1.3, 0.7);
        // Layer 1: pale fat and torn skin. Layer 2: red muscle. Layer 3: dark.
        vec3 tint = layer < 1.5 ? vec3(1.15, 0.80, 0.70) : (layer < 2.5 ? vec3(1.0, 0.62, 0.58) : vec3(0.62, 0.36, 0.34));
        albedo *= tint;
        albedo = mix(albedo, u_goreBloodColor.rgb * 1.1, rim * 0.7);          // blood wells up at every torn edge
    }

    albedo *= mix(1.0, 0.30, inside);                        // the inside of a wall is in shadow
    albedo *= 0.75;                                          // raw flesh is darker than lit skin

    if (u_goreParams.w > 0.5) { float l = dot(albedo, vec3(0.3, 0.59, 0.11)); albedo = vec3(l, l, l) * 0.4; }   // reduced / minimal gore

    vec3 finalColor = goreWetShade(albedo, N, P, viewDir, wet, gloss * mix(1.0, 0.4, inside));
    gl_FragColor = ApplyFog(vec4(finalColor, 1.0), P);
}
```

`goreTriplanar`, `goreBumpTriplanar`, `goreWetShade` are in `gore_flesh.sh` (unchanged). `ApplyFog` and the light
uniforms come from `lighting_common.sh`. If the compiler reports that `gl_FragColor`-only shaders may not `discard`
after a texture read on some target, keep the order shown (all discards before the first non-Lod texture read; every
texture read here is `texture2DLod` on purpose).

### T5.5 `fs_gore_inner_depth.sc` (new)

```glsl
$input v_texcoord0, v_color0, v_normal, v_world, v_texcoord1, v_goreBind

#include <../bgfx_shader.sh>

// Sampler slots are parsed from this file's own text by the engine (includes are not scanned), so they are declared here.
SAMPLER2D(u_goreFlesh,       2);
SAMPLER2D(u_goreFleshNormal, 3);
SAMPLER2D(u_goreEdgeNoise,   4);
SAMPLER2D(u_goreBlood,       5);
#include "gore_common.sh"

void main()
{
    if (u_goreLayer.z < 0.5 && v_goreBind.w < 0.004) discard;

    float rim = 0.0; float soak = 0.0; float fresh = 0.0;
    bool anyBits = (u_goreRangeMask.x + u_goreRangeMask.y) > 0.0;
    if (anyBits && goreSurface(v_goreBind.xyz, rim, soak, fresh)) discard;

    #if(BGFX_SHADER_LANGUAGE_ESSL)
    gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
    #endif
}
```

### T5.6 Delete

`fs_gore_cavity.sc`, `fs_gore_interior.sc`, `fs_gore_interior_depth.sc`, and their compiled `.bin` files under
`GameData/shaders/compiled/*/*/gore/` (search for `fs_gore_cavity.bin`, `fs_gore_interior.bin`,
`fs_gore_interior_depth.bin`).

**Check:** `ShaderCompilationAutomation.exe` compiles every gore shader for every target with no error.

---

## T6. Draw commands (`GoreDrawCommands.h/.cpp`)

### T6.1 `GoreDrawCommands.h`

`struct GpuData` becomes:

```cpp
    struct GpuData
    {
        glm::vec4 counts = glm::vec4(0);       // x wounds, y cuts, z splats
        glm::vec4 params = glm::vec4(0);       // y isGib (0/1), z fleshScale, w gore level (0 full, 1 reduced, 2 minimal)
        glm::vec4 bloodColor = glm::vec4(0.35f, 0.02f, 0.02f, 1.0f);
        std::array<glm::vec4, kMaxWounds * 3> woundBind{};
        std::array<glm::vec4, kMaxWounds> woundParams{};   // x roughness, y rimWidth, z healScale, w seed
        std::array<glm::vec4, kMaxWounds> woundExtra{};    // x taper
        std::array<glm::vec4, kMaxCuts> cutBind{}, cutBindSphere{}, cutRegrowBind{}, cutParams{};
        std::array<glm::vec4, kMaxSplats> splats{};
        Texture* flesh = nullptr;
        Texture* fleshNormal = nullptr;
        Texture* edgeNoise = nullptr;
        Texture* blood = nullptr;
        Texture* bone = nullptr;
        Texture* guts = nullptr;
    };
```

In `class GoreDrawCommand`:

```cpp
        enum class Pass : uint8_t { Front, Shell, Anatomy };

        Pass pass = Pass::Front;
        bool goreActive = false;
        const GpuData* gore = nullptr;
        float layer = 0.0f;           // 0 skin, 1..kShellLayers shells, kAnatomyLayer anatomy
        float insetFraction = 0.0f;   // shells: share of the max inset
```

Delete the `shell` field. Everything else in the class stays.

### T6.2 `GoreDrawCommands.cpp`

```cpp
    namespace
    {
        const char* kVertex = "gore/vs_gore";
        const char* kInnerVertex = "gore/vs_gore_inner";
    }

    Shader* GoreDrawCommand::GetForwardShader() const
    {
        if (!goreActive) return SkeletalMeshDrawCommand::GetForwardShader();
        if (pass == Pass::Front) return ShaderManager::GetShaderProgram(kVertex, "gore/fs_gore");
        return ShaderManager::GetShaderProgram(kInnerVertex, "gore/fs_gore_inner");
    }

    Shader* GoreDrawCommand::GetDepthShader() const
    {
        if (!goreActive) return SkeletalMeshDrawCommand::GetDepthShader();
        if (pass == Pass::Front) return ShaderManager::GetShaderProgram(kVertex, "gore/fs_gore_depth");
        return ShaderManager::GetShaderProgram(kInnerVertex, "gore/fs_gore_inner_depth");
    }
```

`GetCustomIdShader` and `GetShadowVolumeShaders`: unchanged (they already test `pass == Pass::Front` / `pass !=
Pass::Front`).

`ApplyAdditionalShaderParams`: replace the body after the `if (!goreActive || gore == nullptr) return;` line with:

```cpp
        const GpuData& g = *gore;
        const int nw = static_cast<int>(g.counts.x + 0.5f);
        const int nc = static_cast<int>(g.counts.y + 0.5f);
        const int ns = static_cast<int>(g.counts.z + 0.5f);

        shader->SetUniform("u_goreCounts", g.counts);
        shader->SetUniform("u_goreParams", g.params);
        shader->SetUniform("u_goreBloodColor", g.bloodColor);
        shader->SetUniform("u_goreCameraPos", vec4(cameraPos, 1.0f));
        shader->SetUniform("u_goreAlphaTest", vec4(Material.NeedsAlphaDiscard() ? 1.0f : 0.0f));
        shader->SetUniform("u_goreLayer", vec4(layer, insetFraction, pass == Pass::Anatomy ? 1.0f : 0.0f, 0.0f));

        if (nw > 0)
        {
            shader->SetUniform("u_goreWoundBind", g.woundBind.data(), static_cast<size_t>(nw) * 3);
            shader->SetUniform("u_goreWoundParams", g.woundParams.data(), static_cast<size_t>(nw));
            shader->SetUniform("u_goreWoundExtra", g.woundExtra.data(), static_cast<size_t>(nw));
        }
        if (nc > 0)
        {
            const size_t c = static_cast<size_t>(nc);
            shader->SetUniform("u_goreCutBind", g.cutBind.data(), c);
            shader->SetUniform("u_goreCutBindSphere", g.cutBindSphere.data(), c);
            shader->SetUniform("u_goreCutRegrowBind", g.cutRegrowBind.data(), c);
            shader->SetUniform("u_goreCutParams", g.cutParams.data(), c);
        }
        if (ns > 0)
            shader->SetUniform("u_goreSplats", g.splats.data(), static_cast<size_t>(ns));

        shader->SetTexture(hashed_string("u_goreFlesh"), g.flesh);
        shader->SetTexture(hashed_string("u_goreFleshNormal"), g.fleshNormal);
        shader->SetTexture(hashed_string("u_goreEdgeNoise"), g.edgeNoise);
        shader->SetTexture(hashed_string("u_goreBlood"), g.blood);
        shader->SetTexture(hashed_string("u_goreBone"), g.bone);
        shader->SetTexture(hashed_string("u_goreGuts"), g.guts);
```

(`SetUniform` / `SetTexture` ignore names a program does not have, so the same code serves every pass.) Remove the
`u_goreViewProj` uniform line; keep `CaptureCamera` and the three `Draw*` overrides (only `cameraPos` is still used;
`viewProj` may stay as a member).

`CopyCommonFrom`: unchanged.

---

## T7. Mesh (`GoreSkeletalMesh.h/.cpp`)

### T7.1 Header

* `HasGore()`: `return goreState.version != 0 && (goreState.woundCount || goreState.cutCount || goreState.splatCount);`
* Delete: `CavityRef`, `pendingCavities`, `finalCavities`, `CavityGpu`, `cavityGpu`, `visibleCavityCommands`,
  `UpdateCavities()`, `interiorCommands`, `finalInteriorCommands`, and the destructor (declaration and definition:
  there is nothing left to destroy).
* `struct SubmeshRanges`: `std::vector<DrawIndexRange> front, shell, shadowCap, shadowEdge;`
* Add:

```cpp
        std::vector<DrawIndexRange> pendingAnatomyRanges;

        // kShellLayers commands per submesh: index = submesh * kShellLayers + layer. The final* copies are what the
        // renderer may still hold (same pattern the interior commands used).
        std::vector<std::shared_ptr<GoreDrawCommand>> shellCommands, finalShellCommands;
        std::shared_ptr<GoreDrawCommand> anatomyCommand, finalAnatomyCommand;
```

* Add `#include "GoreFlesh.h"` to the `.cpp`.

### T7.2 `OnAssetsReloaded`

```cpp
    void GoreSkeletalMesh::OnAssetsReloaded()
    {
        SkeletalMesh::OnAssetsReloaded();

        // New command objects; the final* lists are not touched here: the renderer may still hold the old ones.
        shellCommands.clear();
        for (size_t i = 0; i < GetSubMeshCount() * kShellLayers; ++i)
        {
            auto sc = std::make_shared<GoreDrawCommand>();
            sc->pass = GoreDrawCommand::Pass::Shell;
            shellCommands.push_back(sc);
        }
        anatomyCommand = std::make_shared<GoreDrawCommand>();
        anatomyCommand->pass = GoreDrawCommand::Pass::Anatomy;

        builtVersion = 0;
    }
```

### T7.3 `PreFinalize`

Delete the line `pendingCavities.clear();`. Nothing else changes.

### T7.4 `BuildRanges`

The mask of a range (which wounds / cuts / splats touch it) is now needed for bones **and** for anatomy pieces. Move
the existing per-bone mask code into one lambda and use it for both. Inside the `for (size_t m ...)` loop, after
`removed`, `shadowRanges`, `gibWithCut`, `gibFirst`, `gibLast` are set up, define:

```cpp
            // Mask of everything that touches a box that follows skel bone `b`. Used for bone ranges and anatomy pieces.
            auto maskFor = [&](size_t b, const glm::vec3& bbMin, const glm::vec3& bbMax) -> glm::vec4
            {
                float woundBits = 0.0f, cutBits = 0.0f, splatBits = 0.0f;

                for (int i = 0; i < goreState.woundCount; ++i)
                {
                    const Wound& w = goreState.wounds[i];
                    if (w.limitSkel >= 0)   // a sever wound only touches the cut bone and what hangs on it
                    {
                        const size_t first = static_cast<size_t>(w.limitSkel);
                        if (first >= profile.subtreeEnd.size() || b < first || b >= profile.subtreeEnd[first]) continue;
                    }
                    if (AabbOverlap(woundMin[i], woundMax[i], bbMin, bbMax))
                        woundBits += Bit(i);
                }

                for (int i = 0; i < goreState.splatCount; ++i)
                {
                    const Splat& s = goreState.splats[i];
                    if (SphereOverlapsAabb(s.center, s.radius, bbMin, bbMax))
                        splatBits += Bit(i);
                }

                for (int i = 0; i < goreState.cutCount; ++i)
                {
                    const Cut& c = goreState.cuts[i];
                    if (c.boneSkel >= profile.subtreeEnd.size() || b < c.boneSkel || b >= profile.subtreeEnd[c.boneSkel])
                        continue;
                    if (!goreState.isGib && b != c.boneSkel)
                        continue;
                    if (!SphereOverlapsAabb(c.center, c.influenceRadius, bbMin, bbMax))
                        continue;

                    float maxDist = -1.0e9f;
                    for (int corner = 0; corner < 8; ++corner)
                    {
                        const glm::vec3 p((corner & 1) ? bbMax.x : bbMin.x, (corner & 2) ? bbMax.y : bbMin.y, (corner & 4) ? bbMax.z : bbMin.z);
                        maxDist = std::max(maxDist, glm::dot(glm::vec3(c.plane), p) - c.plane.w);
                    }
                    if (goreState.isGib) maxDist = 1.0f;   // a gib keeps the removed side
                    if (maxDist > -0.01f)
                        cutBits += Bit(i);
                }
                return glm::vec4(woundBits, cutBits, splatBits, 0.0f);
            };
```

Then in the per-bone loop replace the old `float woundBits ... if (own.hasData) { ... }` block and `const glm::vec4
mask(...)` with:

```cpp
                const glm::vec4 mask = own.hasData ? maskFor(b, own.bbMin, own.bbMax) : glm::vec4(0.0f);
```

keep `append(ranges.front, mask);`, and replace the interior append with the shell append:

```cpp
                // Flesh shells: only where a wound or a cut can show them, only on Volume submeshes.
                if (role == SubmeshRole::Volume && (mask.x > 0.0f || mask.y > 0.0f))
                    append(ranges.shell, mask);
```

(`ranges.shell.clear();` goes next to the other `clear()` calls at the top of the loop; delete `ranges.interior`.)
The shadow part of the loop is unchanged.

**Fix the `removed` loop.** Today it starts at the cut bone itself, so the body drops the cut bone's whole range and
nothing is left to clip (that was the "removed at the joint" mode). The body must keep the cut bone's own range and
clip it with the plane; only the bones **below** it leave with the gib:

```cpp
                    for (size_t b = static_cast<size_t>(c.boneSkel) + 1; b < profile.subtreeEnd[c.boneSkel] && b < n; ++b)
                        removed[b] = 1;
```

(The gib side is already right: it draws `[gibFirst, gibLast)` including the cut bone, clipped the other way.)

**The `continue` for a submesh with role `Attachment` stays. Role `Shell` (cloth) keeps its front ranges (holes and
blood) but gets no flesh shells.**

After the `for (size_t m ...)` loop, build the anatomy ranges (once per `BuildRanges`, not per submesh):

```cpp
        pendingAnatomyRanges.clear();
        if (profile.flesh)
        {
            const bool gibWithCut = goreState.isGib && goreState.cutCount > 0 && goreState.cuts[0].boneSkel < profile.subtreeEnd.size();
            const uint16_t gibFirst = gibWithCut ? goreState.cuts[0].boneSkel : 0;
            const uint16_t gibLast = gibWithCut ? profile.subtreeEnd[gibFirst] : 0;

            for (const FleshPiece& piece : profile.flesh->pieces)
            {
                const size_t b = piece.skel;

                if (goreState.isGib)
                {
                    if (!gibWithCut || b < gibFirst || b >= gibLast) continue;   // not part of this gib
                }
                else
                {
                    bool gone = false;   // strictly below a cut bone: left with the gib
                    for (int i = 0; i < goreState.cutCount && !gone; ++i)
                    {
                        const uint16_t cb = goreState.cuts[i].boneSkel;
                        if (cb < profile.subtreeEnd.size() && b > cb && b < profile.subtreeEnd[cb]) gone = true;
                    }
                    if (gone) continue;
                }

                const glm::vec4 mask = maskForPiece(b, piece.bbMin, piece.bbMax);
                if (mask.x <= 0.0f && mask.y <= 0.0f) continue;   // nothing opens the body near it: it is hidden anyway

                DrawIndexRange r;
                r.first = piece.first;
                r.count = piece.count;
                r.param = mask;
                pendingAnatomyRanges.push_back(r);
            }
        }
```

`maskFor` was defined inside the submesh loop and captures `woundMin` / `woundMax`. To use it here, **hoist** the
wound AABB arrays (`woundMin`, `woundMax`) and the `maskFor` lambda to the top of `BuildRanges`, before the submesh
loop (they do not depend on the submesh), and call it `maskFor` in both places (the name `maskForPiece` above is the
same lambda). Note that the body-side removal test differs: a **bone range** is skipped when `removed[b]` (the whole
subtree including the cut bone's descendants; the cut bone itself is kept and clipped), an **anatomy piece** is skipped
when its bone is strictly below a cut bone. A piece on the cut bone itself is kept and clipped by the plane through
its cut bit.

### T7.5 `FillGpuData`

* `g.counts = glm::vec4(goreState.woundCount, goreState.cutCount, goreState.splatCount, 0.0f);`
* `g.params = glm::vec4(0.0f, goreState.isGib ? 1.0f : 0.0f, profile.fleshScale, static_cast<float>(GoreSettings::Get().level));`
* After `g.blood = profile.blood;` add `g.bone = profile.boneT; g.guts = profile.guts;`
* Wound loop: keep `Mb`, `StoreRows(g.woundBind, i, glm::inverse(Mb));`, the heal scale and `g.woundParams[i]`.
  Delete `M`, `s`, the `woundWorld` line. Replace the `woundExtra` line with
  `g.woundExtra[i] = glm::vec4(w.taper, 0.0f, 0.0f, 0.0f);`
* Delete the whole `pendingCavities` block.
* Cut loop: keep only what fills `g.cutBind[i]`, `g.cutBindSphere[i]`, `g.cutRegrowBind[i]` (with `regrowRadius`) and
  `g.cutParams[i]`. Delete `M`, `s`, `nWorld`, `pointWorld`, `centerWorld`, `cutWorld`, `cutWorldSphere`, `cutCenter`,
  `cutRegrowWorld`, `cutFrame`, `T`, `B`, `reach`. The lambda `bindToWorld` and the locals `World`, `palette` are then
  only needed by `BuildRenderPalette(palette)`: keep `palette`, delete the rest.
* Delete the decal loop.

### T7.6 `FinalizeFrameData`

Keep the first part (finalGpu copy, render palette, the per-submesh front command setup with `cmd.IndexRanges`,
shadow ranges, `RangeParamUniform`). Add `cmd.layer = 0.0f; cmd.insetFraction = 0.0f; cmd.pass =
GoreDrawCommand::Pass::Front;` to the front command setup. Delete `finalCavities = pendingCavities;`, the whole
interior-command block inside the loop, `finalInteriorCommands = interiorCommands;` and `UpdateCavities();`.

Before the per-submesh loop:

```cpp
        ModelFlesh* flesh = (profile && profile->flesh) ? profile->flesh.get() : nullptr;
        if (flesh && pendingActive) flesh->EnsureGpu();   // main thread: creates the shared buffers on first use
```

Inside the per-submesh loop, after the front command `cmd` is set up:

```cpp
            for (int k = 0; k < kShellLayers; ++k)
            {
                const size_t slot = i * kShellLayers + k;
                if (slot >= shellCommands.size()) break;
                GoreDrawCommand& sc = *shellCommands[slot];
                sc.CopyCommonFrom(cmd);

                const bool hasBuffer = flesh && i < flesh->shellVbh.size() && bgfx::isValid(flesh->shellVbh[i]);
                sc.goreActive = cmd.goreActive && hasBuffer && !pendingRanges[i].shell.empty();
                sc.pass = GoreDrawCommand::Pass::Shell;
                sc.layer = static_cast<float>(k + 1);
                sc.insetFraction = kShellFraction[k];

                if (sc.goreActive)
                {
                    sc.VertexBuffer = flesh->shellVbh[i];             // the model's index buffer stays (CopyCommonFrom)
                    sc.IndexRanges.assign(pendingRanges[i].shell.begin(), pendingRanges[i].shell.end());
                }
                else
                {
                    sc.IndexRanges.clear();
                }

                sc.RangeParamUniform = "u_goreRangeMask";
                sc.Cull = StaticMeshDrawCommand::CullFace::None;      // the inside of a wall is visible through a hole
                sc.TwoSided = true;
                sc.DepthPrePass = true;
                sc.Material.CustomId = 0;
                sc.OnlyShadows = false;
                sc.ReceiveDetailShadows = false;
                sc.ShadowCapVbh = BGFX_INVALID_HANDLE;
                sc.ShadowCapIbh = BGFX_INVALID_HANDLE;
                sc.ShadowCapIndexCount = 0;
                sc.ShadowEdgeVbh = BGFX_INVALID_HANDLE;
                sc.ShadowEdgeIbh = BGFX_INVALID_HANDLE;
                sc.ShadowEdgeIndexCount = 0;
            }
```

After the loop:

```cpp
        if (anatomyCommand)
        {
            GoreDrawCommand& ac = *anatomyCommand;
            const bool hasFront = count > 0;
            if (hasFront) ac.CopyCommonFrom(static_cast<GoreDrawCommand&>(GetDrawCommandAt(0)));

            ac.goreActive = hasFront && pendingActive && flesh && bgfx::isValid(flesh->anatomyVbh) && bgfx::isValid(flesh->anatomyIbh)
                         && !pendingAnatomyRanges.empty();
            ac.pass = GoreDrawCommand::Pass::Anatomy;
            ac.layer = static_cast<float>(kAnatomyLayer);
            ac.insetFraction = 0.0f;

            if (ac.goreActive)
            {
                ac.VertexBuffer = flesh->anatomyVbh;
                ac.IndexBuffer = flesh->anatomyIbh;
                ac.IndexRanges.assign(pendingAnatomyRanges.begin(), pendingAnatomyRanges.end());
            }
            else
            {
                ac.IndexRanges.clear();
            }

            ac.RangeParamUniform = "u_goreRangeMask";
            ac.Cull = StaticMeshDrawCommand::CullFace::None;
            ac.TwoSided = true;
            ac.DepthPrePass = true;
            ac.Hidden = false;                 // submesh 0 may be on the hide list; the anatomy is not a submesh
            ac.Material.CustomId = 0;
            ac.OnlyShadows = false;
            ac.ReceiveDetailShadows = false;
            ac.ShadowCapVbh = BGFX_INVALID_HANDLE;
            ac.ShadowCapIbh = BGFX_INVALID_HANDLE;
            ac.ShadowCapIndexCount = 0;
            ac.ShadowEdgeVbh = BGFX_INVALID_HANDLE;
            ac.ShadowEdgeIbh = BGFX_INVALID_HANDLE;
            ac.ShadowEdgeIndexCount = 0;
        }

        finalShellCommands = shellCommands;
        finalAnatomyCommand = anatomyCommand;
```

Rules that matter:

* A command with `goreActive == false` must **never** be collected (T7.7): an inactive shell would fall back to the
  default shader and draw the body a second time.
* A command that is collected must have a **non-empty** `IndexRanges` (an empty list means "draw the whole buffer").
  `goreActive` above already requires non-empty ranges.
* `CopyCommonFrom` copies `Material`, including the model's texture and **alpha mode**. A body whose texture has an
  alpha channel would make its flesh count as transparent (no depth write, wrong pass). So every shell command and the
  anatomy command also sets, after `CopyCommonFrom`:

```cpp
                sc.Material.SurfaceTypeOverride = SurfaceType::Opaque;   // flesh is always opaque, whatever the skin texture is
                sc.Material.Masked = false;
```

  (same two lines with `ac.` for the anatomy command; `SurfaceType` comes from `DrawCommands/SurfaceType.h`).

### T7.7 `CollectDrawCommands`

```cpp
    void GoreSkeletalMesh::CollectDrawCommands(std::vector<IDrawCommand*>& out)
    {
        SkeletalMesh::CollectDrawCommands(out);

        for (auto& sc : finalShellCommands)
            if (sc->goreActive && !sc->Hidden)
                out.push_back(sc.get());

        if (finalAnatomyCommand && finalAnatomyCommand->goreActive)
            out.push_back(finalAnatomyCommand.get());
    }
```

`BuildRenderPalette`, `SetGoreState`, `ClearGoreState`, `GetRenderPalette`: unchanged.

---

## T8. Gameplay side

### T8.1 `GoreSystem.cpp` — delete

* `#include "GoreWoundMesh.h"`.
* In `GrowWound`: the three lines `w.cavity.reset(); w.cavityTicket.reset(); w.cavityFailed = false;`.
* In `AddWound`: the three `merged.cavity...` lines.
* The whole section from the comment `// ---- authored mode: decals, profile wound slots, parts removed at their
  joints` down to the end of `ProcessAuthoredHit` (`kAuthoredDecaySeconds`, `kKillingBlowWeight`, `EffectiveDamage`,
  `kMaxDecalRadius`, `AddDecal`, `FindPart`, `InRemovedPart`, `RemovePart`, `OpenSlots`, `ProcessAuthoredHit`).
  **Before deleting, copy** `kAuthoredDecaySeconds`, `kKillingBlowWeight`, `EffectiveDamage` and `InRemovedPart` up
  next to `kSeverThreshold`: they are still used (below).
* In `ProcessPendingHits`: the `if (!GoreSettings::Get().freeformWounds) { ProcessAuthoredHit(...); continue; }` block
  and the loop `// Cavity geometry for every wound that has none yet`.
* In `PushRenderStateIfChanged`: replace the wound filter loop with `state.wounds = gore.wounds; state.woundCount =
  gore.woundCount;` and delete the two decal lines.
* In `GoreProcessHitsSystem` (bottom of the file): everything between `Gore::ProcessPendingHits(self, gore);` and
  `Gore::PushRenderStateIfChanged(self, gore);` (the `PollCavity` loop and `cavityArrived`).
* `kSeverThreshold`, `kSeverDecaySeconds` (replaced below).

### T8.2 `GoreSystem.cpp` — wound depth (taper)

Add near the top of the anonymous namespace:

```cpp
        // How deep a new wound of each source type goes (see GoreTypes.h kShellLayers): 0.5 = skin and a little meat,
        // 0.2 = down to the bone, 0.1 = through everything. A source may override it ("taper" in sources.json).
        float TaperOf(const Source& source, WoundKind kind)
        {
            if (source.taper >= 0.0f) return glm::clamp(source.taper, kMinTaper, 0.6f);
            switch (source.type)
            {
            case SourceType::Bullet:      return kind == WoundKind::Exit ? 0.34f : 0.45f;
            case SourceType::HeavyBullet: return kind == WoundKind::Exit ? 0.20f : 0.30f;
            case SourceType::Pellet:      return 0.22f;
            case SourceType::Slash:       return 0.34f;
            case SourceType::Blast:       return 0.14f;
            default:                      return 0.50f;
            }
        }
```

`MakeWound` gets one more parameter `float taper` (put it after `rim`) and sets `w.taper = taper;`. `MakeShotWound`
gets the same parameter and passes it on. Every call site passes `TaperOf(source, kind)` (for the pellet cluster
loop use `TaperOf(*source, WoundKind::Crater)`).

`GrowWound` — stacking digs deeper. Add as the last line:

```cpp
            w.taper = std::max(w.taper - 0.07f * weight, kMinTaper);
```

`AddWound`, union branch — add after `merged.reach = ...`:

```cpp
                merged.taper = std::min(merged.taper, other.taper);
```

`CanMerge` — add as the first line: `if (a.limitSkel >= 0 || b.limitSkel >= 0) return false;   // a sever wound stays as it is`

Overkill: in `ProcessPendingHits`, where the bullet / heavy radius `r` is computed and where the blast radius `r` is
computed, multiply by `(hit.lethal ? 1.5f : 1.0f)`; in the pellet cluster loop a cluster does not know `lethal`, so
add `bool lethal = false;` to `struct Cluster`, set `target->lethal |= hit.lethal;` when a pellet joins, and
multiply the crater `r` by `(c.lethal ? 1.4f : 1.0f)`.

### T8.3 `GoreSystem.cpp` — severing

**Rule:** a part comes off when the damage collected on it reaches the part's `detach` value (`SeverableBone::
detachDamage`, from the `.gore` file). Damage is `hit.damage * source.severPower`, the killing blow counts 2.5x, and
it fades over 20 s. Unless `GoreSettings::severAlive` is true, a part only comes off a corpse or with the killing blow.

Replace the body of `TrySever` from the line `const SeverableBone& sev = ...` to the end with:

```cpp
            const SeverableBone& sev = profile.severable[candidate];

            const double now = Time::GameTime;
            float& damage = gore.severDamage[candidate];
            double& at = gore.severDamageTime[candidate];
            damage = damage * static_cast<float>(std::exp(-(now - at) / kAuthoredDecaySeconds)) + EffectiveDamage(hit) * source.severPower;
            at = now;

            // A living enemy keeps its limbs and head (it still collects the damage, so the killing blow tears them off).
            if (hit.alive && !hit.lethal && !GoreSettings::Get().severAlive) return false;

            const bool instant = hit.lethal && sev.lethal && source.severPower > 0.5f && IsSeverType(source.type);
            if (damage < sev.detachDamage && !instant) return false;

            glm::vec3 normal = hit.direction;
            bool strict = false;
            if (source.type == SourceType::Slash)
            {
                strict = true;   // a blade only cuts when it goes across the limb
                if (glm::length(hit.sweepNormal) > 1.0e-4f)
                    normal = hit.sweepNormal;
                else
                    normal = glm::cross(hit.direction, std::abs(hit.direction.y) < 0.99f ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0));
            }
            else
            {
                normal = glm::vec3(0.0f);   // not a blade: straight across the bone (SolveCut falls back to the bone axis)
            }

            return MakeCut(self, mesh, gore, profile, source, hit, candidate, hit.point, normal, false, strict, world, palette, rng);
```

Also change the candidate search in `TrySever`: a hit on a hand must count for the forearm. After the existing
candidate search, if `candidate < 0`, walk up the parents:

```cpp
            if (candidate < 0)
            {
                uint16_t b = anchor.skel;
                while (b < profile.parent.size())
                {
                    if (b < profile.severableBySkel.size() && profile.severableBySkel[b] >= 0)
                    {
                        if (!SubtreeHoldsCut(gore, profile, b)) candidate = profile.severableBySkel[b];
                        break;
                    }
                    const uint16_t parent = profile.parent[b];
                    if (parent >= b) break;   // root (INVALID_BONE_IDX is larger than any index)
                    b = parent;
                }
            }
            if (candidate < 0) return false;
```

In `BlastSever` replace `if (hit.damage * source.severPower < kSeverThreshold) return;` with
`if (EffectiveDamage(hit) * source.severPower < 60.0f) return;`, add
`if (hit.alive && !hit.lethal && !GoreSettings::Get().severAlive) return;` right after it, and in its `MakeCut` call
pass `false` for `atJoint` (replace `!GoreSettings::Get().freeformWounds`).

**The sever wound.** In `MakeCut`, right after `gore.cuts[gore.cutCount++] = cut;` add:

```cpp
            // The plane only says which half is whose. This wound tears a ragged band out of every layer on both sides of
            // it: skin ends first, meat next, the bone sticks out furthest. The gib copies it (it is anchored on the cut bone).
            {
                Wound w;
                const glm::vec3 n = glm::normalize(glm::vec3(cut.plane));
                const float across = std::max(1.7f * cut.outlineRadius, 0.06f);
                w.center = cut.center;
                w.rotation = RotateZTo(n);
                w.radii = glm::vec3(across, across, 0.055f);
                w.anchorSkin = cut.boneSkin;
                w.anchorSkel = cut.boneSkel;
                w.limitSkel = static_cast<int16_t>(cut.boneSkel);
                w.kind = WoundKind::Crater;
                w.taper = kMinTaper;
                w.roughness = 0.45f;
                w.rimWidth = 0.6f;
                w.seed = rng.Next01();
                w.reach = 2.0f * MaxExtent(profile, cut.boneSkel);
                w.order = gore.orderCounter++;

                if (gore.woundCount < kMaxWounds)
                {
                    gore.wounds[gore.woundCount++] = w;
                }
                else
                {
                    int oldest = -1;   // replace the oldest wound that is not a sever wound
                    for (int i = 0; i < gore.woundCount; ++i)
                        if (gore.wounds[i].limitSkel < 0 && (oldest < 0 || gore.wounds[i].order < gore.wounds[oldest].order)) oldest = i;
                    if (oldest >= 0) gore.wounds[oldest] = w;
                }
            }
```

With `kMaxCuts = 4` and `kMaxWounds = 8` there is always a non-sever slot to replace.

In `ProcessPendingHits`, the sever call becomes (it already exists for the non-blast case; keep the `continue`):

```cpp
            if (!gore.isGib && InRemovedPart(gore, profile, anchor.skel)) continue;   // that part is gone
            if (source.type != SourceType::Blast && TrySever(self, mesh, gore, profile, source, hit, anchor, pBind, World, palette, rng))
                continue;
```

### T8.4 Blood bursts (`GoreFx.cpp`, `GoreSystem.h`)

Add to `GoreFx.cpp`, inside `namespace Gore`, after `FxNewFrame`:

```cpp
    // A burst of droplets from a fresh wound. batches: 1 = a puff, 6 = a fountain. Shares the per-frame particle budget.
    void SpawnBloodBurst(const glm::vec3& point, const glm::vec3& direction, int batches, float speed)
    {
        if (batches <= 0 || g_budget <= 0 || glm::length(direction) < 1.0e-4f) return;
        batches = std::min(batches, g_budget);
        g_budget -= batches;
        GlobalParticleSystem::SpawnParticleAt("gore_spurt", point, MathHelper::FindLookAtRotation(glm::vec3(0), glm::normalize(direction)),
                                              glm::vec3(static_cast<float>(batches), speed, 0.0f));
    }
```

Declare it in `GoreSystem.h` next to `FxNewFrame`:
`void SpawnBloodBurst(const glm::vec3& point, const glm::vec3& direction, int batches, float speed);` (add `#include
<glm.h>` there).

`"gore_spurt"` (in `GoreParticles.cpp`) takes `scale = (batches, speed multiplier, 0)` and a rotation that looks along
the jet; with `speed >= 0.5` the jet is tight. Its droplets leave `decal_blood` on world geometry by themselves.

Call it from `ProcessPendingHits`:

| Where | Call |
|---|---|
| Bullet / HeavyBullet, after the entry wound | `SpawnBloodBurst(hit.point, -hit.direction + glm::vec3(0, 0.4f, 0), heavy ? 2 : 1, 0.7f);` |
| Bullet / HeavyBullet, after the exit wound (inside the `HitboxExitPoint` branch) | `SpawnBloodBurst(exitWorld, hit.direction + glm::vec3(0, 0.25f, 0), heavy ? 5 : 3, 1.1f);` |
| Slash | `SpawnBloodBurst(hit.point, -hit.direction + glm::vec3(0, 0.6f, 0), 3, 0.9f);` |
| Blast | `SpawnBloodBurst(hit.point, glm::vec3(0, 1, 0), 6, 1.2f);` |
| Pellet cluster (in the cluster loop; world point = `glm::vec3(World * palette[c.skin] * glm::vec4(c.center, 1.0f))`, world dir = `glm::normalize(glm::vec3(World * palette[c.skin] * glm::vec4(dir, 0.0f)))`) | back: `SpawnBloodBurst(p, -d + glm::vec3(0, 0.4f, 0), 2, 0.8f);` forward: `SpawnBloodBurst(p + d * 0.2f, d + glm::vec3(0, 0.25f, 0), 4, 1.2f);` |
| `MakeCut`, after the gib request | `SpawnBloodBurst(point, hit.direction + glm::vec3(0, 0.5f, 0), 6, 1.2f);` |

`FxNewFrame()` is called once per frame from `GibPool::Tick()` (in `GameStart::Update`), before the ECS ticks of that
frame. That is already so; do not add a second call.

### T8.5 `GoreGib.cpp`

* Wound copy: `if (w.anchorSkel >= S && w.anchorSkel < end)` (delete `&& !w.cavityFailed`).
* Delete the decal copy loop.

Everything else stays. The sever wound is anchored on `S`, so the gib gets it and its end is torn like the stump.

### T8.6 `GoreDebug.cpp`

* Delete the log block that reads `g->openSlots`, `pr0.slots`, `g->decalCount`, `g->decals` (the first `if
  (npc->HasComponent<GoreComponent>())` block in the `fired && ticks == shotTick` branch); keep the line that logs
  wounds / splats / pending / version / cuts, and add a per-wound line:
  `ProbeLog("  wound " + std::to_string(i) + " center " + V3(w.center) + " radii " + V3(w.radii) + " taper " + std::to_string(w.taper));`
* Delete the whole `-gore_slot` block (`auto slotArg = ...` to the end of its `if`); `shotEye` stays as `eye`.
* After the profile log line add: `if (pr.flesh) ProbeLog("gore_look: flesh pieces " + std::to_string(pr.flesh->pieces.size()));`
  (include `GoreFlesh.h`).
* `CheckClosedMeshes`, `DrawWoundVolumes`, the ImGui window, `-gore_dump_bounds`: unchanged.

### T8.7 Other call sites

`StealthGoreAdapter.cpp`, `NpcBase.cpp`, `NpcHumanBase.cpp`, weapons, `GameStart.cpp`: **no change**.

---

## T9. Project files

`StealthAction.vcxproj`:
* replace `<ClCompile Include="Gore\GoreWoundMesh.cpp" />` with `<ClCompile Include="Gore\GoreFlesh.cpp" />`
* replace `<ClInclude Include="Gore\GoreWoundMesh.h" />` with `<ClInclude Include="Gore\GoreFlesh.h" />`

`StealthAction.vcxproj.filters`: the same two replacements (the entries have a `<Filter>Gore</Filter>` child; keep it).

Delete `Gore/GoreWoundMesh.h` and `Gore/GoreWoundMesh.cpp`. Then run `Build\ui_cfg_stealth.cmd` (reconfigure + build).

**Check:** the build finishes with no error and no new warning in `Gore/`. `grep` the `Gore` folder for `cavity`,
`decal`, `slot`, `interior`, `freeform`, `triGrid`: only comments explaining history may remain (better: none).

---

## T10. Visual checks (do all of them, look at every picture)

Run from `source/Games/StealthAction`. `X` = `..\..\..\Build\StealthAction\windows\bin\StealthAction.exe -headless`.

| # | Command | What the picture must show |
|---|---|---|
| 1 | `X -gore_look npc_human_axe -gore_shots 0 -gore_file v2_clean` | An untouched enemy. Identical to before this work. |
| 2 | `X -gore_look npc_human_axe -gore_src weapon_pistol -gore_shots 1 -gore_alive -gore_dist 1.1 -gore_file v2_pistol1` | One ragged hole about 6 cm wide in the clothes, **red meat** inside it (not black, not sky), a dark red rim. |
| 3 | `... -gore_shots 5 -gore_aim spine_02 -gore_alive -gore_file v2_pistol5` | The five hits merged into a deeper crater: meat ring, then **ribs** (pale bars) with dark organs behind. |
| 4 | `... -gore_src weapon_sawed -gore_shots 9 -gore_aim spine_02 -gore_alive -gore_file v2_shotgun` | A crater 15-20 cm wide: torn skin, a meat ring, broken ribs, a dark cavity. No floating triangles, no sky. |
| 5 | `... -gore_src weapon_revolver -gore_shots 1 -gore_aim upperarm_l -gore_lethal -gore_file v2_arm` | The arm is off. The stump is torn, tapered, with a **bone sticking out**. A gib lies or flies nearby. |
| 6 | same as 5 with `-gore_yaw 90` and with `-gore_yaw -90` | The stump from both sides: no see-through into the body, no flat plane. |
| 7 | `... -gore_src weapon_revolver -gore_shots 1 -gore_aim neck_01 -gore_aim_dy 0.12 -gore_lethal -gore_file v2_head` | Head off or blown open: neck stump with the spine, or an open skull with dark matter inside. |
| 8 | `... -gore_src weapon_cannon -gore_shots 1 -gore_aim spine_02 -gore_lethal -gore_file v2_cannon` | A huge hole through the torso, limbs torn off, spine visible. |
| 9 | 3, 4, 5 with `npc_knight`, `npc_zombie`, `npc_human_gun` | The same on the other models (the knight proves `boneMap`). |
| 10 | 3 with `-gore_yaw 180` (shoot from the front, look from behind) | The back is intact unless an exit wound went through. |

If a picture is wrong, use this table before changing anything else:

| Symptom | Cause |
|---|---|
| Hole shows sky / floor through an intact far side | Shell commands are not drawn: `goreActive` false (no buffer, empty ranges), or the submesh role is not `Volume`, or `EnsureGpu` not called. |
| The whole body turns red / is drawn twice | An inactive shell command was collected and used the default shader (T7.7), or a collected command has empty `IndexRanges`. |
| Meat pokes out through the skin far from wounds | The shell moved outward: `Normal` holds the outward normal instead of the inward direction, or the sign in `vs_gore_inner.sc` is flipped. |
| Meat z-fights with the skin on thin parts (ears, fingers, cloth) | The `v_goreBind.w < 0.004` discard is missing, or `ShadowMapCoords.x` was not written. |
| Shell has cracks along uv seams | The inward direction was computed per vertex instead of per welded position. |
| Bones / ribs are black or untextured | `u_goreBone` / `u_goreGuts` not declared in the `.sc` itself, or the `"guts"` texture path is wrong (check the log for a texture warning). |
| No bones at all | `flesh pieces 0` in the log: anatomy bone names wrong, or every `RadiusAround` failed (the body submesh has role `shell`, so the grid is empty). |
| Ribs stick out of the chest | `forward` / `up` in `"anatomy"` do not match the model's bind pose, or the 0.80 factor needs to go down for this model. |
| A wound on the thigh also eats the other leg | A sever wound without `limitSkel`, or `maskFor` ignores `limitSkel`. |
| Stump is a flat disc | The sever wound was not added (`MakeCut`), or it merged into another wound (`CanMerge` must refuse `limitSkel >= 0`). |
| Gib end is clean-cut while the stump is torn | The gib did not copy the sever wound (`anchorSkel` must be the cut bone). |
| Everything inside is pitch black | `u_goreCameraPos` not set for the inner passes (the `Draw*` overrides call `CaptureCamera` only when `goreActive`), or light uniforms missing: compare with how `fs_gore.sc` is fed. |
| Wound edges of all layers line up like onion rings | The `layer * 0.37` noise offset is missing in `goreSurface`. |

---

## T11. Tuning order (after everything above works)

Tune in this order, one thing at a time, with a screenshot before and after:

1. **Hole size per weapon** — `woundRadius` in `sources.json`. A pistol hit must be readable from 8 m.
2. **Depth per weapon** — `TaperOf` (or `"taper"` per source). Pistol: meat only. Three pistol hits in one spot: ribs.
3. **Edge raggedness** — `roughness` passed to `MakeShotWound` / `MakeWound` (0.3-0.5) and the two-octave mix in
   `goreSurface`.
4. **Layer look** — the three tints and the `0.75` / `0.30` factors in `fs_gore_inner.sc`; `fleshScale` in the `.gore`
   file (texture repeats per metre).
5. **Layer spacing** — `kShellFraction` and the `0.4f` / `0.07f` thickness factors in `BuildShells`.
6. **Bone sizes** — `0.28f * limb` (limb bones), rib `0.80f` and `0.009f`, skull `0.80f` in `BuildAnatomy`.
7. **Stump** — the sever wound's `across` factor (1.7), its length (0.055) and taper.
8. **Blood** — batches and speeds in T8.4, `maxParticlesPerFrame`.

Do not add features while tuning. Flat blade cross-sections and flying meat chunks are Part 2 (T13-T14) and **must**
be done. Things that are deliberately **not** in this plan: blood running down the skin (not wanted), blood pools
under corpses, regeneration rules (the `healDuration` / regrow fields still work in the shaders and can be driven later), save/load
of gore state.

---

# Part 2 — on top of the working base

Start Part 2 only when every T10 picture is right. Each task below lists the Part 1 code it changes; change nothing else.

---

## T13. Flat sliced cross-sections for blade cuts

**What.** A blade (source type `slash`) that takes a part off leaves a **flat** cut. On the cut face you see a solid
cross-section, like a slice of ham: a pale outer ring (skin and fat), red muscle rings, the bone as a pale disc and
dark marrow in its middle. Every other weapon keeps the torn stump of Part 1.

**How.** When the cut is made, the game thread slices the body with the cut plane once and builds a small flat mesh,
the **cap**:

* The plane is intersected with the triangles of the **skin**, of each of the three **shells** (at their moved
  positions) and of the **bone piece** of the cut bone. Every crossed triangle gives one line segment on the plane.
* Each segment becomes one triangle `(centre, a, b)`, where `centre` is the cut centre on the bone axis. All triangles
  of one layer together are a filled disc with exactly that layer's outline. This needs no ordered loop and survives
  small gaps in the mesh. (It assumes the cross-section is star-shaped around the bone axis; limbs and necks are.)
* The discs of the deeper layers lie 0.5 mm in front of each other, so they never z-fight: you see rings.
* The mesh is built twice in one buffer: once facing the removed side (drawn by the **body**) and once facing the
  other way (drawn by the **gib**). Body and gib share the same `CapGeometry` object.
* Cap vertices take the bone weights of the model vertex they came from, so the cap stays glued to the stump.

A clean cut gets **no sever wound** and its plane edge has **no noise**, so skin and flesh end exactly on the cap.

### T13.1 `GoreTypes.h`

Before `struct Cut` add `struct CapGeometry;   // GoreCap.h`. In `struct Cut` add:

```cpp
        bool clean = false;                         // a blade cut: flat, capped by `cap`; false: torn (sever wound)
        std::shared_ptr<const CapGeometry> cap;     // shared by the body and the gib; null for torn cuts
```

`Cut` is copied by value everywhere (component, render state, gib); the shared pointer makes that cheap and safe.

### T13.2 `GoreCap.h` (new, complete)

```cpp
#pragma once

#include "GoreTypes.h"

#include <VertexData.h>
#include <skinned_model.hpp>
#include <bgfx/bgfx.h>

#include <memory>
#include <vector>

namespace Gore
{
    struct Profile;

    // The flat face of a blade cut: filled discs (skin, three flesh layers, bone, marrow) on the cut plane, skinned like
    // the stump. Built once per cut; immutable afterwards; shared by the body and the gib.
    struct CapGeometry
    {
        std::vector<VertexData> vertices;
        std::vector<uint32_t> indices;
        uint32_t bodyFirst = 0, bodyCount = 0;   // index range the body draws (faces the removed side)
        uint32_t gibFirst = 0, gibCount = 0;     // index range the gib draws (faces the other way)

        // GPU side. Created on the main thread (EnsureGpu, from FinalizeFrameData), freed with the object.
        mutable bgfx::VertexBufferHandle vbh = BGFX_INVALID_HANDLE;
        mutable bgfx::IndexBufferHandle  ibh = BGFX_INVALID_HANDLE;
        mutable bool gpuReady = false;

        void EnsureGpu() const;
        ~CapGeometry();
    };

    // Game thread, a few thousand plane tests. Null when the plane does not cross the skin near the cut centre
    // (the caller then makes a torn cut instead).
    std::shared_ptr<const CapGeometry> BuildCap(const roj::SkinnedModel& model, const Profile& profile, const Cut& cut);
}
```

### T13.3 `GoreCap.cpp` (new, complete)

```cpp
#include "GoreCap.h"
#include "GoreFlesh.h"
#include "GoreProfile.h"

#include <cmath>

namespace Gore
{
    namespace
    {
        struct Crossing
        {
            glm::vec3 p = glm::vec3(0);
            const VertexData* weights = nullptr;   // the model vertex whose bone weights this point takes
        };

        struct Segment
        {
            Crossing a, b;
            int layer = 0;   // 0 skin, 1..3 shells, 4 bone
        };

        // Where the plane dot(n, p) = w crosses a triangle: two points on its edges. False when it does not cross.
        bool SliceTriangle(const glm::vec3* p, const VertexData* const* src, const glm::vec3& n, float w, Crossing* out)
        {
            const float d[3] = { glm::dot(n, p[0]) - w, glm::dot(n, p[1]) - w, glm::dot(n, p[2]) - w };
            int count = 0;
            for (int k = 0; k < 3 && count < 2; ++k)
            {
                const int j = (k + 1) % 3;
                if ((d[k] > 0.0f) == (d[j] > 0.0f)) continue;      // this edge stays on one side
                const float t = d[k] / (d[k] - d[j]);               // never 0 / 0: the two signs differ
                out[count].p = glm::mix(p[k], p[j], t);
                out[count].weights = t < 0.5f ? src[k] : src[j];
                ++count;
            }
            return count == 2;
        }

        // Look of each disc, outside to inside. tint multiplies the texture; material 0 = flesh texture, 1 = bone texture;
        // shrink scales the disc around its centre (the marrow is the bone outline at 55 %).
        struct Look { glm::vec3 tint; float material; float shrink; };
        const Look kLooks[6] =
        {
            { glm::vec3(1.25f, 0.95f, 0.80f), 0.0f, 1.00f },   // 0 skin outline: pale skin and fat ring
            { glm::vec3(1.05f, 0.60f, 0.55f), 0.0f, 1.00f },   // 1 shell 1: light muscle
            { glm::vec3(0.90f, 0.42f, 0.40f), 0.0f, 1.00f },   // 2 shell 2: muscle
            { glm::vec3(0.62f, 0.28f, 0.28f), 0.0f, 1.00f },   // 3 shell 3: deep muscle
            { glm::vec3(1.00f, 0.94f, 0.84f), 1.0f, 1.00f },   // 4 bone
            { glm::vec3(0.45f, 0.13f, 0.10f), 1.0f, 0.55f },   // 5 marrow
        };
        constexpr float kLift = 0.0005f;   // metres between two discs
    }

    void CapGeometry::EnsureGpu() const
    {
        if (gpuReady) return;
        gpuReady = true;
        if (vertices.empty() || indices.empty()) return;
        vbh = bgfx::createVertexBuffer(bgfx::copy(vertices.data(), static_cast<uint32_t>(sizeof(VertexData) * vertices.size())),
                                       VertexData::Declaration());
        ibh = bgfx::createIndexBuffer(bgfx::copy(indices.data(), static_cast<uint32_t>(sizeof(uint32_t) * indices.size())),
                                      BGFX_BUFFER_INDEX32);
    }

    CapGeometry::~CapGeometry()
    {
        if (bgfx::isValid(vbh)) bgfx::destroy(vbh);
        if (bgfx::isValid(ibh)) bgfx::destroy(ibh);
    }

    std::shared_ptr<const CapGeometry> BuildCap(const roj::SkinnedModel& model, const Profile& profile, const Cut& cut)
    {
        if (!profile.flesh) return nullptr;
        const ModelFlesh& flesh = *profile.flesh;

        const size_t boneCount = profile.subtreeEnd.size();
        const uint16_t S = cut.boneSkel;
        if (S >= boneCount || profile.parent.size() != boneCount) return nullptr;

        const glm::vec3 n = glm::normalize(glm::vec3(cut.plane));
        const float w = cut.plane.w;
        const glm::vec3 centre = cut.center;
        const float limit = 1.6f * cut.outlineRadius + 0.03f;   // ignore anything the plane crosses far from the cut

        // Triangles that can cross the plane near the cut: the cut bone, everything below it, and its parent
        // (vertices near the cut may be weighted to the parent bone).
        std::vector<uint16_t> bones;
        if (profile.parent[S] < boneCount) bones.push_back(profile.parent[S]);   // a root's parent is INVALID_BONE_IDX (large)
        for (uint16_t b = S; b < profile.subtreeEnd[S]; ++b) bones.push_back(b);

        std::vector<Segment> segments;
        auto add = [&](const glm::vec3* p, const VertexData* const* src, int layer)
        {
            Crossing c[2];
            if (!SliceTriangle(p, src, n, w, c)) return false;
            if (glm::distance(c[0].p, centre) > limit || glm::distance(c[1].p, centre) > limit) return false;
            if (glm::distance(c[0].p, c[1].p) < 1.0e-5f) return false;
            Segment s;
            s.a = c[0];
            s.b = c[1];
            s.layer = layer;
            segments.push_back(s);
            return true;
        };

        int skinSegments = 0;
        for (size_t m = 0; m < model.meshes.size(); ++m)
        {
            const SubmeshRole role = m < profile.submeshRoles.size() ? profile.submeshRoles[m] : SubmeshRole::Volume;
            if (role != SubmeshRole::Volume) continue;      // cloth is a thin sheet: clipped by the plane, no cross-section

            const roj::SkinnedMesh& mesh = model.meshes[m];
            if (mesh.boneIndexFirst.size() != boneCount + 1) continue;

            const std::vector<VertexData>* shell = nullptr;
            if (m < flesh.shellVertices.size() && flesh.shellVertices[m].size() == mesh.vertices.size())
                shell = &flesh.shellVertices[m];

            for (uint16_t b : bones)
            {
                const uint32_t end = mesh.boneIndexFirst[b + 1];
                for (uint32_t i = mesh.boneIndexFirst[b]; i + 2 < end; i += 3)
                {
                    const uint32_t id[3] = { mesh.indices[i], mesh.indices[i + 1], mesh.indices[i + 2] };
                    const VertexData* src[3] = { &mesh.vertices[id[0]], &mesh.vertices[id[1]], &mesh.vertices[id[2]] };

                    glm::vec3 p[3] = { src[0]->Position, src[1]->Position, src[2]->Position };
                    if (add(p, src, 0)) ++skinSegments;

                    if (shell == nullptr) continue;
                    const VertexData* sv[3] = { &(*shell)[id[0]], &(*shell)[id[1]], &(*shell)[id[2]] };
                    if (sv[0]->ShadowMapCoords.x < 0.004f || sv[1]->ShadowMapCoords.x < 0.004f || sv[2]->ShadowMapCoords.x < 0.004f)
                        continue;                               // no flesh under this triangle (same rule as the shell shader)

                    for (int k = 0; k < kShellLayers; ++k)
                    {
                        for (int v = 0; v < 3; ++v)             // the same move the shell vertex shader does
                            p[v] = sv[v]->Position + sv[v]->Normal * (sv[v]->ShadowMapCoords.x * kShellFraction[k]);
                        add(p, src, k + 1);
                    }
                }
            }
        }
        if (skinSegments < 3) return nullptr;

        // The bone inside the cut bone.
        glm::vec3 boneCentre = centre;
        {
            glm::vec3 sum(0.0f);
            int count = 0;
            for (const FleshPiece& piece : flesh.pieces)
            {
                if (piece.skel != S) continue;
                for (uint32_t i = piece.first; i + 2 < piece.first + piece.count; i += 3)
                {
                    const VertexData* src[3] = { &flesh.anatomyVertices[flesh.anatomyIndices[i]],
                                                 &flesh.anatomyVertices[flesh.anatomyIndices[i + 1]],
                                                 &flesh.anatomyVertices[flesh.anatomyIndices[i + 2]] };
                    const glm::vec3 p[3] = { src[0]->Position, src[1]->Position, src[2]->Position };
                    if (add(p, src, 4))
                    {
                        sum += segments.back().a.p + segments.back().b.p;
                        count += 2;
                    }
                }
            }
            if (count > 0) boneCentre = sum / static_cast<float>(count);   // the bone need not sit exactly on the cut centre
        }

        // Texture axes on the plane.
        const glm::vec3 upRef = std::abs(n.y) < 0.99f ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);
        const glm::vec3 T = glm::normalize(glm::cross(n, upRef));
        const glm::vec3 B = glm::cross(n, T);

        auto cap = std::make_shared<CapGeometry>();
        cap->vertices.reserve(segments.size() * 8);
        cap->indices.reserve(segments.size() * 8);

        auto vertex = [&](const glm::vec3& pos, const glm::vec3& normal, const Look& look, const VertexData* weights)
        {
            VertexData v;
            v.Position = pos;
            v.Normal = normal;
            const glm::vec3 r = pos - centre;
            v.TextureCoordinate = glm::vec2(glm::dot(r, T), glm::dot(r, B)) * 10.0f;   // 10 texture repeats per metre
            if (weights != nullptr)
            {
                for (int k = 0; k < 4; ++k) v.BlendIndices[k] = weights->BlendIndices[k];
                v.BlendWeights = weights->BlendWeights;
            }
            else
            {
                v.BlendIndices[0] = static_cast<float>(cut.boneSkin);
                v.BlendWeights = glm::vec4(1, 0, 0, 0);
            }
            v.Color = glm::vec4(look.tint, 1.0f);
            v.ShadowMapCoords = glm::vec2(1.0f, look.material);
            cap->indices.push_back(static_cast<uint32_t>(cap->vertices.size()));
            cap->vertices.push_back(v);
        };

        // sign +1: seen from the removed side (the body's cap). sign -1: seen from the kept side (the gib's cap).
        auto emit = [&](float sign)
        {
            const glm::vec3 normal = n * sign;
            for (const Segment& s : segments)
            {
                const int discs = s.layer == 4 ? 2 : 1;        // a bone segment makes the bone disc and the marrow disc
                for (int k = 0; k < discs; ++k)
                {
                    const int lookIndex = s.layer + k;
                    const Look& look = kLooks[lookIndex];
                    const glm::vec3 c = s.layer == 4 ? boneCentre : centre;
                    const glm::vec3 lift = normal * (kLift * static_cast<float>(lookIndex));   // deeper discs lie nearer the viewer
                    vertex(c + lift, normal, look, nullptr);
                    vertex(c + (s.a.p - c) * look.shrink + lift, normal, look, s.a.weights);
                    vertex(c + (s.b.p - c) * look.shrink + lift, normal, look, s.b.weights);
                }
            }
        };

        cap->bodyFirst = 0;
        emit(1.0f);
        cap->bodyCount = static_cast<uint32_t>(cap->indices.size());
        cap->gibFirst = cap->bodyCount;
        emit(-1.0f);
        cap->gibCount = static_cast<uint32_t>(cap->indices.size()) - cap->gibFirst;

        return cap;
    }
}
```

Why the signs are right: `Cut::plane` has its normal pointing to the **removed** side (`dot(n, p) - w > 0` is removed).
The body keeps the other side, so its cap is looked at from the removed side: normal `+n`, deeper discs lifted along
`+n`. The gib is the mirror case.

### T13.4 Shader and draw command

`gore_common.sh`: only the comment of `u_goreLayer` changes: `// x layer index, y inset fraction, z 1 = anatomy / cap / chunk (no inset), w 1 = cap`.

`fs_gore_inner.sc`: add `bool cap = u_goreLayer.w > 0.5;` under `bool anatomy = ...`, and turn the
`if (anatomy) { ... } else { ... }` into `if (cap) { ... } else if (anatomy) { ... } else { ... }` with this new first
branch (the other two branches stay as they are):

```glsl
    if (cap)
    {
        // The flat face of a blade cut. v_color0.rgb = the tint of this ring, v_texcoord1.y = 1 for bone and marrow.
        float isBone = step(0.5, v_texcoord1.y);
        vec3 fleshC = texture2DLod(u_goreFlesh, v_texcoord0, 0.0).rgb;
        vec3 boneC  = texture2DLod(u_goreBone, v_texcoord0 * 3.0, 0.0).rgb;
        albedo = mix(fleshC, boneC, isBone) * v_color0.rgb;
        float film = texture2DLod(u_goreBlood, v_texcoord0 * 1.7, 0.0).r;      // a thin film of blood over the slice
        albedo = mix(albedo, u_goreBloodColor.rgb * 1.3, clamp(0.12 + film * 0.4, 0.0, 0.6) * (1.0 - 0.6 * isBone));
        wet = 1.0;
        gloss = 1.0;
    }
```

A cap is drawn with range mask 0, so `anyBits` is false and `goreSurface` is never called for it (it lies exactly on
the plane and must not be clipped by its own cut). `vs_gore_inner.sc` and `fs_gore_inner_depth.sc` need no change:
a cap is drawn in "anatomy" mode (`u_goreLayer.z = 1`: no inset, `a_normal` is the real normal).

`GoreDrawCommands.h`: add to `class GoreDrawCommand`: `bool cap = false;   // the flat face of a blade cut (pass Anatomy)`.

`GoreDrawCommands.cpp`: the `u_goreLayer` line becomes

```cpp
        shader->SetUniform("u_goreLayer", vec4(layer, insetFraction, pass == Pass::Anatomy ? 1.0f : 0.0f, cap ? 1.0f : 0.0f));
```

### T13.5 `GoreSkeletalMesh`

Header (`#include "GoreCap.h"` in the `.cpp`, forward declare `struct CapGeometry;` is already in `GoreTypes.h`):

```cpp
        // One command per cut for the flat face of a blade cut. capHeld keeps the geometry (and its GPU buffers) alive
        // for as long as a command may still point at it.
        std::array<std::shared_ptr<GoreDrawCommand>, kMaxCuts> capCommands, finalCapCommands;
        std::array<std::shared_ptr<const CapGeometry>, kMaxCuts> capHeld;
```

`OnAssetsReloaded`, next to the shell commands:

```cpp
        for (auto& cc : capCommands)
        {
            cc = std::make_shared<GoreDrawCommand>();
            cc->pass = GoreDrawCommand::Pass::Anatomy;
            cc->cap = true;
        }
```

`FinalizeFrameData`, after the anatomy command block and before `finalShellCommands = shellCommands;`:

```cpp
        for (int i = 0; i < kMaxCuts; ++i)
        {
            if (!capCommands[i]) continue;
            GoreDrawCommand& cc = *capCommands[i];

            std::shared_ptr<const CapGeometry> geometry;
            if (pendingActive && count > 0 && i < goreState.cutCount)
                geometry = goreState.cuts[i].cap;
            if (geometry) geometry->EnsureGpu();   // main thread

            const uint32_t first = geometry ? (goreState.isGib ? geometry->gibFirst : geometry->bodyFirst) : 0;
            const uint32_t indexCount = geometry ? (goreState.isGib ? geometry->gibCount : geometry->bodyCount) : 0;

            cc.goreActive = geometry && bgfx::isValid(geometry->vbh) && bgfx::isValid(geometry->ibh) && indexCount > 0;
            if (cc.goreActive)
            {
                cc.CopyCommonFrom(static_cast<GoreDrawCommand&>(GetDrawCommandAt(0)));   // transform, light, palette
                cc.VertexBuffer = geometry->vbh;
                cc.IndexBuffer = geometry->ibh;
                DrawIndexRange range;
                range.first = first;
                range.count = indexCount;
                range.param = glm::vec4(0.0f);      // mask 0: a cap is never clipped
                cc.IndexRanges.assign(1, range);
            }
            else
            {
                cc.IndexRanges.clear();
            }

            cc.pass = GoreDrawCommand::Pass::Anatomy;
            cc.cap = true;
            cc.gore = &finalGpu;
            cc.layer = static_cast<float>(kAnatomyLayer);
            cc.insetFraction = 0.0f;
            cc.RangeParamUniform = "u_goreRangeMask";
            cc.Cull = StaticMeshDrawCommand::CullFace::None;
            cc.TwoSided = true;
            cc.DepthPrePass = true;
            cc.Hidden = false;
            cc.Material.SurfaceTypeOverride = SurfaceType::Opaque;
            cc.Material.Masked = false;
            cc.Material.CustomId = 0;
            cc.OnlyShadows = false;
            cc.ReceiveDetailShadows = false;
            cc.ShadowCapVbh = BGFX_INVALID_HANDLE;
            cc.ShadowCapIbh = BGFX_INVALID_HANDLE;
            cc.ShadowCapIndexCount = 0;
            cc.ShadowEdgeVbh = BGFX_INVALID_HANDLE;
            cc.ShadowEdgeIbh = BGFX_INVALID_HANDLE;
            cc.ShadowEdgeIndexCount = 0;

            capHeld[i] = geometry;   // the previous one (if any) is released here, after this frame's command was rewired
        }
        finalCapCommands = capCommands;
```

`CollectDrawCommands`, at the end:

```cpp
        for (auto& cc : finalCapCommands)
            if (cc && cc->goreActive)
                out.push_back(cc.get());
```

The cap uses the body's **render palette** (copied by `CopyCommonFrom`), in which the bones below a cut follow the cut
bone. Cap vertices that took weights of a child bone therefore stay on the plane.

### T13.6 `GoreSystem.cpp` — `MakeCut`

`MakeCut` already has the parameter `bool strictTilt`; it is true exactly for blade cuts (T8.3). Add `#include
"GoreCap.h"`. After `cut.phase = CutPhase::Severed;` add:

```cpp
            // A blade leaves a flat slice with a solid cross-section; everything else tears (the sever wound below).
            if (strictTilt)
            {
                cut.clean = true;
                cut.roughness = 0.0f;                    // skin and flesh end exactly on the plane
                cut.cap = BuildCap(*mesh->GetSkinnedModel(), profile, cut);
                if (!cut.cap)                            // the plane found no skin to slice: fall back to a torn cut
                {
                    cut.clean = false;
                    cut.roughness = result.cut.roughness;
                }
            }
```

and wrap the whole sever-wound block of T8.3 (`{ Wound w; ... }`) in `if (!cut.clean) { ... }`. The block comes after
`gore.cuts[gore.cutCount++] = cut;`, which must stay **after** the code above (the stored cut must carry `clean` and `cap`).

`GoreGib.cpp`: no change. `gore->cuts[0] = cut;` copies `clean` and `cap`; the gib draws the `gib` range.

### T13.7 Project files and check

Add `Gore\GoreCap.cpp` / `Gore\GoreCap.h` to `StealthAction.vcxproj` and `.filters` (same pattern as T9), run
`Build\ui_cfg_stealth.cmd`, recompile shaders.

| # | Command (prefix `X -gore_look npc_human_axe -gore_dist 1.1`) | The picture must show |
|---|---|---|
| 1 | `-gore_src weapon_sword -gore_shots 1 -gore_aim lowerarm_l -gore_damage 60 -gore_lethal -gore_file v2_slice` | The forearm is off with a **flat** cut. |
| 2 | same with `-gore_yaw 90` and `-gore_yaw -90` (find the angle that looks at the cut face) | Rings on the face: pale outer ring, red rings, a pale bone disc with a dark centre. No gaps, no flicker between rings. |
| 3 | same on `upperarm_l`, `thigh_l`, `neck_01` (`-gore_damage 100`) | The same on every part. The gib's end shows the same slice. |
| 4 | the T10 check 5 again (revolver, arm) | Still a **torn** stump with the bone sticking out (no cap). |

| Symptom | Cause |
|---|---|
| Cut face missing, you look into the hollow limb | `cut.cap` is null (log it): fewer than 3 skin segments. The plane is not where you think: print `cut.plane`, `cut.center`, `outlineRadius`. Or the cap command is not collected. |
| Face is there on the body but not on the gib (or the reverse) | Wrong index range (`gibFirst` / `bodyFirst`) or `goreState.isGib` not used. |
| Face is black from one side | Normal sign: the body range must have normal `+n`, the gib range `-n`. |
| Rings flicker | `kLift` missing or applied with the wrong sign (deeper discs must be nearer to the viewer). |
| Cap floats or swims when the limb moves | Cap vertices did not get bone weights (`weights` null for outline points), or the cap command lost `BoneMatrices` (`CopyCommonFrom`). |
| A huge disc reaching into the torso | The `limit` test is missing, or `outlineRadius` is wrong. |
| Ragged skin edge around a flat face | `cut.roughness` was not set to 0, or the sever wound was still added. |

---

## T14. Flying meat chunks

**What.** Big hits throw small pieces of meat and bone splinters out of the wound. They fly, bounce once or twice,
leave a blood mark where they first land, lie there for a while and vanish.

**How.** The particle system only draws billboards, so chunks are their own small system inside the gore module:

* **Meshes:** four tiny generated meshes (three meat lumps, one bone splinter), built once with the `Builder` of
  `GoreFlesh.cpp` and shared by every chunk.
* **Drawing:** every chunk is one `GoreDrawCommand` in anatomy mode (`gore/vs_gore_inner` + `gore/fs_gore_inner`, the
  same shaders and textures as the bones and guts). No new shader. The bone palette is one identity matrix.
* **Simulation:** no physics bodies. Each chunk is a point with a velocity: gravity, one `Physics::LineTrace` against
  the world per frame, bounce or come to rest. That is cheap enough for 48 chunks.
* **Owner:** one entity per level (`GoreChunkSystem`), created on first use from `GameStart::Update` (not from an ECS
  tick; entity creation inside a tick group trips the EcsGuard, the same reason `GibPool` exists).

### T14.1 Settings

`GoreSettings.h`: add `int maxChunks = 48;` and `int maxChunkSpawnsPerFrame = 16;`; load both in `GoreSettings.cpp`
like `maxGibs`; add `"maxChunks": 48, "maxChunkSpawnsPerFrame": 16` to `GameData/gore/settings.json`.

### T14.2 Chunk meshes (`GoreFlesh.h/.cpp`)

`GoreFlesh.h`, at the end of the namespace:

```cpp
    // Four small meshes around the origin, about 1 unit in radius (scaled per chunk): variants[0..2] meat lumps,
    // variants[3] a bone splinter. Every vertex follows skin bone 0 (the chunk system gives it an identity matrix).
    void BuildChunkMeshes(std::vector<VertexData>& vertices, std::vector<uint32_t>& indices, std::vector<FleshPiece>& variants);
```

`GoreFlesh.cpp`, after `BuildModelFlesh` (inside `namespace Gore`, outside the anonymous namespace; it uses `Builder`):

```cpp
    void BuildChunkMeshes(std::vector<VertexData>& vertices, std::vector<uint32_t>& indices, std::vector<FleshPiece>& variants)
    {
        Builder B{ vertices, indices, variants };
        const glm::mat3 I(1.0f);

        // Meat lumps: material 1 (guts texture), different shapes and brightness.
        B.Begin(); B.Blob(glm::vec3(0), I, glm::vec3(1.0f, 0.70f, 0.85f), 0.30f, 0.3f, 7, 5, 0, 1.0f, 1.0f, 1.0f); B.End(0);
        B.Begin(); B.Blob(glm::vec3(0), I, glm::vec3(1.3f, 0.50f, 0.70f), 0.35f, 1.9f, 7, 5, 0, 1.0f, 0.8f, 1.0f); B.End(0);
        B.Begin(); B.Blob(glm::vec3(0), I, glm::vec3(0.8f, 0.80f, 0.60f), 0.25f, 4.2f, 6, 4, 0, 1.0f, 1.1f, 1.0f); B.End(0);

        // A bone splinter: material 0 (bone texture), thick in the middle, pointed at one end.
        const std::vector<glm::vec3> points = { glm::vec3(-1.2f, 0.0f, 0.0f), glm::vec3(-0.4f, 0.1f, 0.0f),
                                                glm::vec3(0.5f, -0.05f, 0.0f), glm::vec3(1.3f, 0.1f, 0.0f) };
        const std::vector<float> radii = { 0.12f, 0.30f, 0.26f, 0.05f };
        B.Begin(); B.Tube(points, radii, 5, false, glm::vec3(0, 1, 0), 0, 0.0f, 1.0f, 1.0f); B.End(0);
    }
```

### T14.3 `GoreChunks.h` (new, complete)

```cpp
#pragma once

#include <glm.h>

namespace Gore
{
    // Queues `count` chunks flying from `point` roughly along `direction` (world space). speed in m/s, size = radius of
    // a typical chunk in metres. Safe to call from anywhere on the game thread; nothing is created until ChunkPool::Tick.
    void SpawnChunks(const glm::vec3& point, const glm::vec3& direction, int count, float speed, float size);

    namespace ChunkPool
    {
        void Tick();   // GameStart::Update, next to GibPool::Tick(): creates the system entity on first use, spawns the queue
    }
}
```

### T14.4 `GoreChunks.cpp` (new, complete)

```cpp
#include "GoreChunks.h"

#include "GoreDrawCommands.h"
#include "GoreFlesh.h"
#include "GoreSettings.h"

#include <AssetRegistry.h>
#include <BSP/Quake3Bsp.h>
#include <Entity.h>
#include <EntityHandle.h>
#include <IDrawMesh.h>
#include <Level.hpp>
#include <Particle/GlobalParticleSystem.hpp>
#include <Physics.h>
#include <Time.hpp>
#include <World/WorldOrientationManager.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <algorithm>
#include <array>
#include <cstdlib>
#include <vector>

namespace Gore
{
    namespace
    {
        constexpr int kMaxChunks = 64;             // size of the pool; GoreSettings::maxChunks may use less
        constexpr double kFlightSeconds = 8.0;     // a chunk that never lands is removed after this
        constexpr double kRestSeconds = 12.0;      // how long a landed chunk stays
        constexpr float kShrinkSeconds = 0.5f;     // it shrinks to nothing over the last half second

        float Rand01() { return static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX); }   // looks only, never gameplay
        glm::vec3 RandVec() { return glm::vec3(Rand01() - 0.5f, Rand01() - 0.5f, Rand01() - 0.5f) * 2.0f; }

        struct Chunk
        {
            bool active = false;
            bool resting = false;
            bool decalDone = false;
            uint8_t variant = 0;
            uint8_t bounces = 0;
            glm::vec3 position = glm::vec3(0);
            glm::vec3 velocity = glm::vec3(0);
            glm::vec3 spinAxis = glm::vec3(0, 1, 0);
            float spinSpeed = 0.0f;                 // radians per second
            glm::quat rotation = glm::quat(1, 0, 0, 0);
            float size = 0.03f;                     // metres
            double bornAt = 0.0;
            double dieAt = 0.0;
        };

        struct Request
        {
            glm::vec3 point, direction;
            int count;
            float speed, size;
        };

        // The shared meshes. CPU side is built once (game thread); GPU side on first draw (main thread).
        struct ChunkMeshes
        {
            std::vector<VertexData> vertices;
            std::vector<uint32_t> indices;
            std::vector<FleshPiece> variants;
            bgfx::VertexBufferHandle vbh = BGFX_INVALID_HANDLE;
            bgfx::IndexBufferHandle ibh = BGFX_INVALID_HANDLE;
            bool gpuReady = false;

            void EnsureGpu()
            {
                if (gpuReady || vertices.empty()) return;
                gpuReady = true;
                vbh = bgfx::createVertexBuffer(bgfx::copy(vertices.data(), static_cast<uint32_t>(sizeof(VertexData) * vertices.size())),
                                               VertexData::Declaration());
                ibh = bgfx::createIndexBuffer(bgfx::copy(indices.data(), static_cast<uint32_t>(sizeof(uint32_t) * indices.size())),
                                              BGFX_BUFFER_INDEX32);
            }
        };

        ChunkMeshes g_meshes;
        const std::vector<mat4> g_identityPalette(1, mat4(1.0f));   // chunk vertices follow "bone 0"
        std::vector<Request> g_requests;
        EntityHandle g_system;
        int g_decalBudget = 0;                                       // blood marks left per frame

        // Draws the chunks: one command each. Reads the chunk array in PreFinalize only.
        class ChunkDrawable : public IDrawMesh
        {
        public:
            explicit ChunkDrawable(const std::array<Chunk, kMaxChunks>* source) : chunks(source)
            {
                for (auto& c : commands) c = std::make_shared<GoreDrawCommand>();
            }

            GpuData gpu;   // textures and gore level; filled by the system in LoadAssets

            void PreFinalize() override
            {
                pendingCount = 0;
                const double now = Time::GameTime;
                for (const Chunk& c : *chunks)
                {
                    if (!c.active) continue;
                    Item& item = pending[pendingCount++];

                    const float fade = glm::clamp(static_cast<float>(c.dieAt - now) / kShrinkSeconds, 0.0f, 1.0f);
                    item.position = c.position;
                    item.world = glm::translate(mat4(1.0f), c.position) * glm::mat4_cast(c.rotation)
                               * glm::scale(mat4(1.0f), glm::vec3(c.size * fade));
                    item.variant = c.variant;

                    // Light at the chunk, like StaticMesh::PreFinalize does for its entity.
                    item.ambient = glm::vec3(0.3f);
                    item.direct = glm::vec3(0.6f);
                    item.lightDirection = glm::vec3(0, -1, 0);
                    if (Level::Current != nullptr)
                    {
                        const LightVolPointData light = Level::Current->BspData.GetLightvolColorPoint(c.position * MAP_SCALE, false);
                        item.ambient = glm::max(light.ambientColor, glm::vec3(0.01f));
                        item.direct = glm::max(light.directColor, glm::vec3(0.03f));
                        item.lightDirection = light.direction;
                    }
                }
            }

            void FinalizeFrameData() override
            {
                g_meshes.EnsureGpu();
                finalCount = 0;
                if (!bgfx::isValid(g_meshes.vbh) || !bgfx::isValid(g_meshes.ibh) || g_meshes.variants.empty()) return;

                for (int i = 0; i < pendingCount; ++i)
                {
                    const Item& item = pending[i];
                    const FleshPiece& piece = g_meshes.variants[std::min<size_t>(item.variant, g_meshes.variants.size() - 1)];

                    GoreDrawCommand& cmd = *commands[finalCount++];
                    cmd.pass = GoreDrawCommand::Pass::Anatomy;   // same shaders as bones and guts
                    cmd.goreActive = true;
                    cmd.gore = &gpu;
                    cmd.layer = static_cast<float>(kAnatomyLayer);
                    cmd.insetFraction = 0.0f;

                    cmd.VertexBuffer = g_meshes.vbh;
                    cmd.IndexBuffer = g_meshes.ibh;
                    DrawIndexRange range;
                    range.first = piece.first;
                    range.count = piece.count;
                    range.param = glm::vec4(0.0f);               // mask 0: no wounds on a chunk
                    cmd.IndexRanges.assign(1, range);
                    cmd.RangeParamUniform = "u_goreRangeMask";   // must be set, or the mask of the previous draw stays

                    cmd.BoneMatrices = &g_identityPalette;
                    cmd.WorldMatrix = item.world;
                    cmd.WorldBounds = BoundingBox(item.position - glm::vec3(0.3f), item.position + glm::vec3(0.3f));
                    cmd.LightAmbientColor = item.ambient;
                    cmd.LightDirectColor = item.direct;
                    cmd.LightDirection = item.lightDirection;

                    cmd.Cull = StaticMeshDrawCommand::CullFace::None;
                    cmd.TwoSided = true;
                    cmd.DepthPrePass = true;
                    cmd.Material.SurfaceTypeOverride = SurfaceType::Opaque;
                    cmd.Material.CustomId = 0;
                    cmd.IsViewmodel = false;
                    cmd.OnlyShadows = false;
                    cmd.ReceiveDetailShadows = false;
                }
            }

            void CollectDrawCommands(std::vector<IDrawCommand*>& out) override
            {
                for (int i = 0; i < finalCount; ++i) out.push_back(commands[i].get());
            }

            bool IsCameraVisible() override { return pendingCount > 0; }   // the chunks are spread out: no single frustum box

        private:
            struct Item
            {
                mat4 world = mat4(1.0f);
                glm::vec3 position = glm::vec3(0);
                glm::vec3 ambient = glm::vec3(0), direct = glm::vec3(0), lightDirection = glm::vec3(0, -1, 0);
                uint8_t variant = 0;
            };

            const std::array<Chunk, kMaxChunks>* chunks;
            std::array<Item, kMaxChunks> pending;
            int pendingCount = 0, finalCount = 0;
            std::array<std::shared_ptr<GoreDrawCommand>, kMaxChunks> commands;
        };

        class GoreChunkSystem : public Entity
        {
        public:
            GoreChunkSystem()
            {
                ClassName = "gore_chunks";
                SaveGame = false;
                drawable = new ChunkDrawable(&chunks);
                Drawables.push_back(drawable);   // the entity owns and deletes it (Entity::DestroyDrawables)
            }

            void Spawn(const Request& r)
            {
                const double now = Time::GameTime;
                const glm::vec3 up = WorldOrientationManager::GetUpVector();
                const glm::vec3 dir = glm::length(r.direction) > 1.0e-4f ? glm::normalize(r.direction) : up;
                const int limit = glm::clamp(GoreSettings::Get().maxChunks, 1, kMaxChunks);

                for (int k = 0; k < r.count; ++k)
                {
                    // A free slot, else the oldest chunk.
                    Chunk* slot = nullptr;
                    for (int i = 0; i < limit; ++i)
                    {
                        if (!chunks[i].active) { slot = &chunks[i]; break; }
                        if (slot == nullptr || chunks[i].bornAt < slot->bornAt) slot = &chunks[i];
                    }

                    Chunk c;
                    c.active = true;
                    c.variant = Rand01() < 0.2f ? 3 : static_cast<uint8_t>(std::min(static_cast<int>(Rand01() * 3.0f), 2));
                    c.position = r.point + RandVec() * 0.04f;
                    c.velocity = (dir + RandVec() * 0.55f) * (r.speed * glm::mix(0.5f, 1.2f, Rand01())) + up * glm::mix(0.5f, 2.5f, Rand01());
                    const glm::vec3 axis = RandVec() + glm::vec3(0.01f, 0.02f, 0.03f);
                    c.spinAxis = glm::normalize(axis);
                    c.spinSpeed = glm::mix(4.0f, 18.0f, Rand01());
                    c.rotation = glm::angleAxis(Rand01() * 6.28f, c.spinAxis);
                    c.size = r.size * glm::mix(0.6f, 1.5f, Rand01() * Rand01());
                    c.bornAt = now;
                    c.dieAt = now + kFlightSeconds;
                    *slot = c;
                }
            }

            void Update() override
            {
                const float dt = Time::DeltaTimeF;
                if (dt <= 0.0f) return;
                const double now = Time::GameTime;
                const glm::vec3 up = WorldOrientationManager::GetUpVector();

                for (Chunk& c : chunks)
                {
                    if (!c.active) continue;
                    if (now >= c.dieAt) { c.active = false; continue; }
                    if (c.resting) continue;

                    c.velocity -= up * (9.8f * dt);
                    const glm::vec3 next = c.position + c.velocity * dt;
                    const Physics::HitResult hit = Physics::LineTrace(c.position, next, BodyType::World);

                    if (!hit.hasHit)
                    {
                        c.position = next;
                        c.rotation = glm::normalize(glm::angleAxis(c.spinSpeed * dt, c.spinAxis) * c.rotation);
                        continue;
                    }

                    if (!c.decalDone && g_decalBudget > 0)   // a blood mark where it first touches the world
                    {
                        --g_decalBudget;
                        c.decalDone = true;
                        GlobalParticleSystem::SpawnParticleAt("decal_blood", hit.position, hit.normal, glm::vec3(1));
                    }

                    const float into = -glm::dot(c.velocity, hit.normal);
                    if (into > 1.5f && c.bounces < 2)        // a wet bounce: most of the speed is lost
                    {
                        const glm::vec3 reflected = glm::reflect(c.velocity, hit.normal);
                        const glm::vec3 normalPart = hit.normal * glm::dot(reflected, hit.normal);
                        c.velocity = normalPart * 0.3f + (reflected - normalPart) * 0.6f;
                        c.position = hit.position + hit.normal * (c.size * 0.6f);
                        c.spinSpeed *= 0.5f;
                        ++c.bounces;
                    }
                    else                                     // it stays where it hit (meat sticks to walls too)
                    {
                        c.position = hit.position + hit.normal * (c.size * 0.45f);
                        c.velocity = glm::vec3(0);
                        c.resting = true;
                        c.dieAt = now + kRestSeconds;
                    }
                }
            }

        protected:
            void LoadAssets() override
            {
                if (g_meshes.vertices.empty())
                    BuildChunkMeshes(g_meshes.vertices, g_meshes.indices, g_meshes.variants);

                // The same textures the humanoid profile uses (GameData/gore/humanoid.gore).
                GpuData& g = drawable->gpu;
                g.flesh = AssetRegistry::GetTextureFromFile("GameData/textures/gore/flesh_d.png");
                g.fleshNormal = AssetRegistry::GetTextureFromFile("GameData/textures/gore/flesh_n.png");
                g.edgeNoise = AssetRegistry::GetTextureFromFile("GameData/textures/gore/edge_noise.png");
                g.blood = AssetRegistry::GetTextureFromFile("GameData/textures/gore/blood_soak.png");
                g.bone = AssetRegistry::GetTextureFromFile("GameData/textures/gore/bone_d.png");
                g.guts = AssetRegistry::GetTextureFromFile("GameData/textures/lq_flesh/flesh_gut1.jpg");
                g.counts = glm::vec4(0.0f);                  // no wounds, cuts or splats on a chunk
                g.params = glm::vec4(0.0f, 0.0f, 5.0f, static_cast<float>(GoreSettings::Get().level));
            }

        private:
            std::array<Chunk, kMaxChunks> chunks;
            ChunkDrawable* drawable = nullptr;
        };
    }

    void SpawnChunks(const glm::vec3& point, const glm::vec3& direction, int count, float speed, float size)
    {
        if (count <= 0 || g_requests.size() >= 64) return;
        g_requests.push_back({ point, direction, count, speed, size });
    }

    namespace ChunkPool
    {
        void Tick()
        {
            g_decalBudget = 4;
            if (g_requests.empty()) return;

            // Chunks are a "full gore" effect only.
            if (Level::Current == nullptr || GoreSettings::Get().level != GoreSettings::Level::Full)
            {
                g_requests.clear();
                return;
            }

            GoreChunkSystem* system = dynamic_cast<GoreChunkSystem*>(g_system.Resolve());
            if (system == nullptr)   // first use in this level
            {
                system = new GoreChunkSystem();
                Level::Current->AddEntity(system);
                system->LoadAssetsIfNeeded();
                g_system = EntityHandle::FromEntity(system);
            }

            int budget = GoreSettings::Get().maxChunkSpawnsPerFrame;
            for (Request& r : g_requests)
            {
                r.count = std::min(r.count, budget);
                budget -= r.count;
                if (r.count > 0) system->Spawn(r);
            }
            g_requests.clear();
        }
    }
}
```

Facts this code relies on (all checked in the engine; if one is not true any more, stop and ask):

* `IDrawMesh` virtuals: `PreFinalize()` (parallel, end of frame: the only place that may read the entity),
  `FinalizeFrameData()` (main thread: the only place for bgfx buffers), `CollectDrawCommands(vector<IDrawCommand*>&)`,
  `IsCameraVisible()`. `Level` calls `FinalizeFrameData` + `CollectDrawCommands` only when `IsCameraVisible()` is true.
* `Entity::Drawables` are deleted by the entity. `LevelObject::Update()` is the per-frame game-thread update.
* `Level::Current->BspData.GetLightvolColorPoint(position * MAP_SCALE, wallCheck)` returns
  `LightVolPointData { directColor, ambientColor, direction }` (this is what `Entity::GetLightVolData` does).
* `SkeletalMeshDrawCommand::ApplyAdditionalShaderParams` uploads `*BoneMatrices`; one identity matrix is enough because
  every chunk vertex uses bone 0 with weight 1.
* `StaticMeshColorEmissiveMaterial` with no texture binds a black `u_texture`; the inner shaders do not read it.
* `GoreDrawCommand::Draw*` call `CaptureCamera` when `goreActive`, which feeds `u_goreCameraPos`.
* A default-constructed `EntityHandle` resolves to null (as `GibRequest::body` relies on).

### T14.5 Hook and call sites

`GameStart.cpp`: add `#include "Gore/GoreChunks.h"` and, right after `Gore::GibPool::Tick();`:

```cpp
        Gore::ChunkPool::Tick();   // spawns the queued meat chunks (outside the ECS tick groups)
```

`GoreSystem.cpp`: `#include "GoreChunks.h"` and add the calls below, each right after the `SpawnBloodBurst` call of
the same place (T8.4). `point` / direction values are the same ones passed to `SpawnBloodBurst` there.

| Where | Call |
|---|---|
| Bullet, exit wound | `SpawnChunks(exitWorld, hit.direction, 1, 3.0f, 0.022f);` |
| HeavyBullet, exit wound | `SpawnChunks(exitWorld, hit.direction, 3, 4.0f, 0.028f);` |
| Bullet / HeavyBullet entry, only when `hit.lethal` | `SpawnChunks(hit.point, -hit.direction, 2, 2.0f, 0.022f);` |
| Pellet cluster (world `p`, `d` from T8.4) | `SpawnChunks(p + d * 0.15f, d, 2 + std::min(c.count, 6) / 2, 4.5f, 0.03f);` and `SpawnChunks(p, -d, 2, 2.0f, 0.025f);` |
| Slash | `SpawnChunks(hit.point, -hit.direction, 1, 2.0f, 0.025f);` |
| Blast | `SpawnChunks(hit.point, glm::vec3(0, 1, 0), 8, 5.0f, 0.04f);` |
| `MakeCut`, torn cut (`!cut.clean`) | `SpawnChunks(point, hit.direction, 5, 3.5f, 0.035f);` |
| `MakeCut`, clean cut | nothing (a blade does not throw meat) |

### T14.6 Project files and check

Add `Gore\GoreChunks.cpp` / `Gore\GoreChunks.h` to `StealthAction.vcxproj` and `.filters`, run
`Build\ui_cfg_stealth.cmd`. No shader changes.

The look test takes its picture at tick `-gore_wait` (default 130); the shots are fired at tick 71. So `-gore_wait 82`
catches the chunks in the air, the default shows them lying on the floor.

| # | Command (prefix `X -gore_look npc_human_axe -gore_dist 1.6`) | The picture must show |
|---|---|---|
| 1 | `-gore_src weapon_sawed -gore_shots 9 -gore_aim spine_02 -gore_wait 82 -gore_file v2_chunks_air` | Several small red lumps (and maybe a pale splinter) in the air around / behind the enemy. Lit like the scene, not black, not glowing. |
| 2 | same without `-gore_wait`, and with `-gore_yaw 180` | Lumps lying on the floor behind the enemy, each a few centimetres big, blood marks under some of them. |
| 3 | `-gore_src weapon_cannon -gore_shots 1 -gore_aim spine_02 -gore_lethal -gore_wait 85 -gore_file v2_chunks_cannon` | A burst of chunks. |
| 4 | `-gore_src weapon_pistol -gore_shots 1 -gore_alive` | No chunks (a pistol only throws one from an exit wound). |

| Symptom | Cause |
|---|---|
| No chunks at all | `ChunkPool::Tick()` not called from `GameStart::Update`; gore level not `full`; `g_meshes` empty (`LoadAssetsIfNeeded` not called); `IsCameraVisible` false because `PreFinalize` did not run (the drawable is not in `Drawables`). |
| Chunks are black | Light values 0 (check `GetLightvolColorPoint`), or `u_goreCameraPos` not set (`goreActive` false). |
| Chunks are huge or invisible | `size` is a radius in metres (0.02-0.04); the meshes are about 1 unit in radius. |
| All chunks look like the enemy's body, or the game crashes in the draw | The command has no `IndexRanges` (it then draws the whole buffer) or invalid buffers. |
| Chunks hang in the air | `Update()` is not called (entity not added to the level) or `Time::DeltaTimeF` is 0 while paused (correct when paused). |
| Chunks fall through the floor | `LineTrace` start and end are the same point on the first frame, or the trace uses a body type that does not include the level; `BodyType::World` is what the blood droplets use. |
| A wound appears on chunks / parts of them are missing | `range.param` is not 0 or `RangeParamUniform` is not set, so a body's mask leaks into the chunk draw. |

---

## T12. When done (after Part 2: T13 and T14 are finished and checked)

* Update `GORE_IMPLEMENTATION.md` §9 with a short entry: date, "Gore V2: layered flesh + procedural anatomy, see
  GORE_V2_PLAN.md", what was verified with screenshots and what was not.
* At the top of `GORE_DESIGN.md` and of `GORE_IMPLEMENTATION.md` §10 add one line: "Superseded by GORE_V2_PLAN.md
  for wounds, interiors and severing."
* Tell the user exactly which of the T10 checks you ran and looked at, and which you did not.
