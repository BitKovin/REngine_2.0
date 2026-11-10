# Real-time Procedural Dismemberment — Implementation Plan

Target: **StealthAction** enemies (`NpcBase` / `NpcHumanBase` and subclasses).
Reuse: all game code lives in one self-contained folder, `source/Games/StealthAction/Gore/`, plus
`GameData/shaders/source/gore/`. Only one adapter file touches StealthAction classes, so another game can copy the folder,
write its own adapter, and get the feature.

The engine does **not** get a gore feature. It gets a small set of general extension points (§5) that make the feature
easy to build and fast to run, and that are useful for other things too (hiding mesh parts, custom shaders per mesh).

Reference: Vlachos, *Rendering Wounds in Left 4 Dead 2*, GDC 2010. We keep its core idea (cull pixels with analytic
volumes in **pose / bind space**, ragged edges from a projected texture, a blood layer that fades with distance from the
wound). We drop its weakest part: authored wound meshes that had to fit the body exactly. The inside of the body is
**ray-traced analytically in the pixel shader** from the same volumes and textured with the artist's flesh texture.
That allows any number of wound shapes and positions with no wound geometry, and adds real severing with physics gibs.

---

## 1. Goals and budgets

| Goal | Target |
|---|---|
| Undamaged enemy cost | **Identical to today** (same shaders, same draw calls). Gore costs nothing until the first wound. |
| Wounds per body | 8 ellipsoid wounds + 4 cut planes + 16 blood splats (merge/evict past that, §6.3) |
| Wound placement | Exactly at the hit point, sized and shaped by weapon and damage |
| Severing | Any limb segment, head, upper body; cut plane follows the blade path / blast direction, clamped to valid ranges |
| Severed parts | Same model, physics-driven (single rigid body or partial ragdoll), capped pool, oldest fade out |
| CPU per hit | ~1 µs for a wound; a sever ~10–20 µs plus gib spawn (budgeted per frame, §7) |
| CPU per wounded body per frame | ~12 matrix multiplies (in the already-parallel `PreFinalize`) |
| GPU per wounded body | Front pass: one branch per pixel away from wounds. Interior pass: only the triangles of bones near a wound. |
| Scale | 300 enemies on screen, 50+ damaged in one frame (shotgun crowd / explosion) without a hitch |
| Look | Ragged torn edges, blood-soaked rims, wet flesh cavities, through-holes you can see through, solid stump caps with bone and marrow, arterial spurts, blood trails |

## 2. Decisions taken

* **Severing applies to living enemies too.** An arm shot off makes the enemy drop its weapon, a leg cut off makes it
  fall and crawl, the head always kills. Living enemies with a cut bleed out (§6.8).
* **Gibs come from a capped pool**; when it's full, the oldest gib sinks/fades and is removed. Wounds on corpses persist.
* **Save/load** restores each corpse's wounds, cuts and missing limbs. Loose gibs are not saved.
* **Network**: every peer builds gore locally from the hit events it already receives. All random choices use a seed
  taken from the hit, so wounds and cuts match between peers; gib physics may differ slightly.

## 3. Technique overview

Three primitives, all defined in the model's **bind space** (pre-skinning model space: the space `a_position` is in):

| Primitive | Data | Used for |
|---|---|---|
| **Wound** | ellipsoid (center, 3 axes, 3 radii), anchor bone, edge roughness, rim, seed | bullet entry/exit holes, shotgun craters, through-holes, slash gashes, bites, crushed skull |
| **Cut** | plane (normal, distance), cut bone, bone-core radius, seed | severing: everything on the positive side is gone from this body |
| **Splat** | sphere (center, radius), strength | blood soaked into skin/clothes around hits and below wounds |

Why bind space: a bind-space volume deforms with the skin, so the hole stays glued to the body in every pose. A body
and its severed piece see the **same** bind-space plane, so their two halves always fit exactly, whatever the two
skeletons do afterwards.

Rendering a wounded body (per submesh):

```
             ┌──────────────────────────────────────────────────────────────────────┐
 front faces │ gore VS (skin + bind pos)  →  gore FS: discard inside wounds / past   │ normal draw command,
             │ cuts (ragged edge from noise), blood rim + splats, else normal shading│ gore shaders
             ├──────────────────────────────────────────────────────────────────────┤
 back faces  │ "interior" draw command, only bone index ranges near wounds/cuts.     │ extra draw command,
 (interior)  │ FS ray-traces the wound ellipsoids / cut planes in world space,       │ front-face culled
             │ writes the analytic depth, shades cavity walls / stump caps / inner   │
             │ wall with flesh texture, fat ring, bone + marrow; through-holes discard│
             └──────────────────────────────────────────────────────────────────────┘
 severed     │ index ranges of the severed bone subtree are not drawn at all          │
 subtree     │ (bone-partitioned index buffer, §5 E4)                                 │
             └──────────────────────────────────────────────────────────────────────┘
```

A severed piece (**gib**) is a new entity with a `GoreSkeletalMesh` on the **same model** (shared GPU buffers): it
draws only the index ranges of the cut bone's subtree (+ the parent's boundary range), clips with the **inverted** cut
plane, gets the same cap, and is moved by Jolt bodies created from the subset of the model's hitboxes.

---

## 4. Rendering in detail

### 4.1 Spaces and per-frame data

* `bindPos` — vertex shader passes `a_position` as a varying (`v_bindPos`). Clip tests and splats use it, so they are
  exact and pose-independent.
* For the interior ray-trace the volumes are needed in **world space**. Each frame, in `GoreSkeletalMesh::PreFinalize`
  (already parallel per mesh), for every wound/cut:
  `M_world = WorldMatrix * skinPalette[anchorSkinIdx] * M_bind` (skin matrix = bone × inverse bind, so it maps bind
  space straight into the current model space). We upload the **inverse** (world → unit-sphere / plane-local) as 3 vec4
  rows.
  Near joints the skin isn't rigid, so the rigid world-space volume can differ from the deformed clip edge by a few
  millimetres. The anchor is the bone with the highest weight at the hit (from the hitbox), which keeps that error small
  and hidden under the blood rim. L4D2 shipped with the same approximation.

### 4.2 Front-surface pass (`gore/fs_gore`)

Same lighting as `fs_default` (the lighting code moves into `shaders/source/lighting_common.sh`, included by both; rim
light, specular and point lights stay identical, including `MeshCustomShaderParams` like `rim_color`).

```
mask = v_goreMask                       // from VS: conservative "any primitive near this vertex"
if (mask > 0) {
   for each cut   : d = dot(n, bindPos) - dist;  edge = noise(planeUV) * roughness
                    if (d > -edge) discard;       // ragged severed edge
   for each wound : q = woundBindInv * bindPos;   r = length(q)
                    edge = noise(q.xy * scale + seed) * roughness   (L4D2's projected texture)
                    if (r < 1 + edge) discard;
                    rim  = max(rim, 1 - smoothstep(1, 1 + rimWidth, r))
   for each splat : soak = max(soak, splatFalloff(bindPos) * noise)
   albedo = mix(albedo, bloodAlbedo * darken, max(rim, soak)); gloss up where wet
}
```

The `v_goreMask` branch keeps the cost of a wounded body's pixels far from any wound almost equal to `fs_default`.

### 4.3 Interior pass (`gore/fs_gore_interior`, back faces)

The body must be closed (true for the human enemies). Through a hole you see back faces of the same submesh. For each
back-face fragment at world point `B` on the view ray `R(t) = cam + t·dir` (with `tB` = distance to `B`):

```
if bindPos(B) inside any wound or past any cut: discard        // through-hole: the back wall is gone too
tVis = -inf; kind = InnerWall
for each wound:  [tIn, tOut] = rayEllipsoid(R, woundWorldInv)   // analytic, ~25 ALU
                 if hit && tOut < tB && tOut > tVis: tVis = tOut; kind = Cavity(w)
for each cut:    t = rayPlane(R, cutWorld); if ray goes from the removed side into the kept side
                 && t < tB && t > tVis: tVis = t; kind = Cap(c)
if (tVis == -inf && kind == InnerWall && no primitive touches the ray) discard;  // cheap early out: hidden anyway
P = R(tVis) (or B for InnerWall);  gl_FragDepth = depth(P)
shade P:
  Cavity  : normal = -gradient of ellipsoid at P; flesh texture triplanar in wound-local coords; darker with depth
  Cap     : normal = cut normal; flesh cross-section texture in plane coords;
            fat/skin ring where |P - B| is small (close to the outline of the cross-section);
            bone ring + marrow around the cut bone's axis (radius from data), plus optional bone texture
  InnerWall: inner side of the body wall, flesh texture, heavy darkening
```

If the ray first hits intact skin, the depth test rejects the interior pixel (the front face wrote a closer depth). If
the ray crosses a cut plane outside the cross-section, intact skin on the kept side is in front. So the depth buffer
decides what is visible, and the shader needs no knowledge of the whole silhouette.

Clothing/hair submeshes are thin **shells**, not volumes: they clip and bleed like skin, and their interior pass only
draws the dark cloth back side (no cavity or cap). The role of each submesh comes from the gore profile (§6.1).

### 4.4 Depth pre-pass and custom-id pass

The renderer draws opaque depth in a pre-pass (twice: MSAA and resolve), then forward with `LEqual` and **no depth
write** (`Renderer.cpp` ~383–481). Therefore:

* Front pass in depth pre-pass: `gore/fs_gore_depth` (the same discard logic, no shading). Without it, the hole would
  stay "filled" in depth (L4D2's "depth-only pass" note).
* Interior pass in depth pre-pass: `gore/fs_gore_interior_depth` writes the same analytic `gl_FragDepth` (the game
  shaders already write `gl_FragDepth` in `fs_copy_depth`, so every target platform supports it).
* Forward interior writes the identical depth through the shared include function. To be safe against tiny
  differences between the two compiled programs, the forward interior biases its depth 1 ulp toward the camera.
* Custom-id pass uses the clip variant too, so ids don't show through holes.
* Depth is computed from `projection*view*P` with `bgfx::getCaps()->homogeneousDepth` passed as a uniform, so GL and DX
  depth ranges both match the rasterizer.

### 4.5 Detail shadow volumes (no phantom shadows)

`NpcHumanBase` sets `CastDetailShadows = true`, so severed parts must not cast shadow volumes from the body.

* `gore/vs_shadowvolcap_gore`, `gore/vs_shadowvoledge_gore`: before skinning, every bind position (the edge shader's
  p0, p1, adj0, adj1) on the removed side of a cut is **projected onto the cut plane**. The mesh stays closed, so the
  volume stays valid and is cut at the plane.
* The render palette (§6.5) makes bones past the cut follow the cut bone rigidly, so the flattened part stays exactly
  planar (zero volume, no slivers).
* The shadow precomp buffers are bone-partitioned like the main index buffer (E4), so a body skips the severed subtree
  and a gib draws only its own subtree.
* Holes (wounds) don't change shadows.

### 4.6 Vertex shader extras (`gore/vs_gore`)

Same skinning as `vs_default` (`finalBonesMatrices[128]`), plus:

* `v_bindPos`, `v_goreMask` (max influence of any primitive at this vertex, widened by a max edge length so whole
  triangles near a wound get the mask).
* **Rim displacement**: vertices within the rim band of a wound move along their normal: inward for entry holes and
  slashes (torn-in look), outward for exit wounds and blasts (flesh pushed out).
* **Stump pucker**: vertices within ~3 cm on the kept side of a cut pull toward the bone axis by up to ~10%, so the
  skin rolls in around the stump instead of ending as a straight tube.

### 4.7 Uniform layout

Fragment-side arrays (vec4), uploaded with **only the active count** (`Shader::SetUniform(name, ptr, count)`):

| Uniform | vec4 per item | Max items |
|---|---|---|
| `u_goreWoundBind` (bind → unit sphere, 3 rows) | 3 | 8 |
| `u_goreWoundWorld` (world → unit sphere, 3 rows) | 3 | 8 |
| `u_goreWoundParams` (roughness, rim width, rim sign, seed) | 1 | 8 |
| `u_goreCutBind` (plane) / `u_goreCutWorld` (plane) | 1 + 1 | 4 |
| `u_goreCutFrame` (world plane tangent, bitangent, bone center, bone radius) | 2 | 4 |
| `u_goreSplats` (bind center, radius) | 1 | 16 |
| `u_goreCounts` (wounds, cuts, splats, flags) | 1 | 1 |

Worst case 93 vec4, typical (2–3 wounds) about 25. Samplers: `u_goreFlesh`, `u_goreFleshNormal`, `u_goreEdgeNoise`,
`u_goreBlood` (slots 2–5; `fs_default` uses 0–1).

### 4.8 Shader files (in `GameData/shaders/source/gore/`, compiled by `ShaderCompilationAutomation.exe`)

`gore_common.sh` (primitive tests, ray intersections, noise, depth), `varying.def.sc` (adds `v_bindPos`, `v_goreMask`;
follow how `shadowVolume/` is set up), `vs_gore.sc`, `fs_gore.sc`, `fs_gore_depth.sc`, `fs_gore_interior.sc`,
`fs_gore_interior_depth.sc`, `fs_gore_customId.sc`, `vs_shadowvolcap_gore.sc`, `vs_shadowvoledge_gore.sc`. Plus the
shared `lighting_common.sh` taken out of `fs_default.sc` (no visual change to `fs_default`).

---

## 5. Engine changes (general extension points, no gore code)

Each one is small, has no behaviour change for existing meshes, and is useful beyond gore.

**E1 — Shader choice per draw command.**
`StaticMeshColorEmissiveMaterial::GetForwardShader/GetDepthShader` hard-code `vs_default`, and
`DrawCustomId`/`DrawShadowVolumeStencil` hard-code their programs. Add virtuals on `StaticMeshDrawCommand`:
`GetForwardShader()`, `GetDepthShader()`, `GetCustomIdShader()`, `GetShadowVolumeShaders(cap, edge)`, with today's
behaviour as default, plus a `VertexShader` field next to `PixelShader` (and `StaticMesh::SetVertexShader`).
`GetSortKey()` uses `GetForwardShader()`.
Files: `DrawCommands/StaticMeshDrawCommand.{h,cpp}`, `StaticMeshColorEmissiveMaterial.h`, `StaticMesh.{h,cpp}`.

**E2 — Cull mode per draw command.**
Replace the `TwoSided` bool's use in the draw functions with a `Cull` enum field (`Back` default / `Front` / `None`).
`TwoSided = true` maps to `None`, so existing code keeps working. The interior pass needs `Front`.

**E3 — Index sub-ranges per draw command.**
`StaticMeshDrawCommand::IndexRanges` (small inline array of `{first, count}`; empty = whole buffer). A shared
`SubmitGeometry(shader)` sets uniforms once and submits each range (`bgfx::setIndexBuffer(ib, first, num)`, then
`bgfx::submit` with `BGFX_DISCARD_INDEX_BUFFER` between ranges and `BGFX_DISCARD_ALL` after the last).
`Shader::Submit` gets an optional discard-flags parameter; buffered uniforms are flushed once. The same goes for the
shadow-volume cap/edge buffers (`ShadowCapRanges`, `ShadowEdgeRanges`).

**E4 — Bone-partitioned index buffers (load time).**
Skeleton bones are already in DFS pre-order (`FlatSkeleton`, `parentIdx[i] < i`), so the subtree of bone `i` is the
index interval `[i, subtreeEnd[i])`. At model parse (CPU side, before `UploadSkinnedMeshGPUBuffers`), for each skinned
submesh:
1. For each triangle, take the **lowest common ancestor** (skeleton index) of every bone that influences any of its
   three vertices (weight > 0).
2. Stable-sort the triangles by that LCA. Order inside each bone is kept, so vertex-cache locality barely changes.
3. Store per skeleton bone: `ownFirst`, `ownCount` and `subtreeCount`, plus a bind-space AABB of its own triangles.

Do the same in `BuildShadowVolumePrecomp` for cap triangles and edge quads (LCA of the edge's records). Result: "every
triangle that could move with the subtree of bone B, and nothing else" is **one contiguous range**, and any set of
cut-away subtrees means "whole buffer minus k intervals". Also add per-bone **any-influence** bind bounds
(`PrecomputeBoneBounds(0.0f)` kept as a second array beside the existing 0.05 one) and `subtreeEnd` on `FlatSkeleton`.
This is always on: no visual change, O(triangles) at load, a few KB per model.
Files: `skinned_model.{hpp,cpp}`, `AssetRegistry.cpp` (deferred upload path).

**E5 — Skin palette and skeleton access on `SkeletalMesh`.**
`const std::vector<mat4>& GetSkinPalette() const` (current `boneTransforms`, today private),
`int GetSkinIndex(hashed_string)`, `uint16_t GetSkeletonIndex(hashed_string)`. Make `OnAssetsReloaded` keep a
subclass-overridable way to choose which palette vector the draw commands point at (a protected
`virtual const std::vector<mat4>* GetRenderPalette()` returning `&finalizedBoneTransforms` by default), so a subclass
can render with a modified palette without copying gameplay bones.

**E6 — Removing hitboxes at runtime.**
`SkeletalMesh::RemoveHitboxes(std::span<const hashed_string> bones)` destroys the bodies, their constraints and
constraints whose parent is removed, erases them from this instance's `metaData.hitboxes`, and marks bindings dirty.
Fix in `UpdateHitboxes` (ragdoll branch): it indexes `hitboxConstraints[...]` with `operator[]` for every hitbox with a
`parentBone`, which inserts a null constraint and then calls `UpdateSwingTwistMotor(nullptr, ...)` when the parent
hitbox doesn't exist. Use `find`. That bug would crash any partial ragdoll.

**E7 — Bounds over a bone subset.**
`SkinnedModel::ComputeAnimatedBounds(world, palette, std::span<const uint16_t> skinBones)` overload, so a living body
with a missing arm, or a gib before its ragdoll starts, gets tight frustum bounds.

**E8 — Nothing else.** Per-frame ordering (`PreFinalize` parallel → `FinalizeFrameData` main thread →
`CollectDrawCommands`) already gives a subclass every hook it needs. List E1–E7 in the engine change log when done.

---

## 6. Game module (`StealthAction/Gore/`)

```
Gore/
  GoreTypes.h            Wound / Cut / Splat PODs, limits, NLOHMANN serialisation
  GoreProfile.{h,cpp}    per-model data (.gore JSON), loaded in LoadAssets via FileSystemEngine
  GoreSource.h           GoreSourceComponent: what a weapon/projectile/explosion does to flesh
  GoreComponent.h        per-body gore state (ECS component)
  GoreEvents.h           GoreHitEvent, GoreSeveredEvent, GoreHeadDestroyedEvent
  GoreSystem.cpp         hit batching → wounds / cuts / severs; bleeding + gib lifetime tick
  GoreCutSolver.{h,cpp}  turns a desired plane into a valid cut on a severable bone
  GoreSkeletalMesh.{h,cpp}   SkeletalMesh subclass: render state, palette, interior commands
  GoreDrawCommands.{h,cpp}   front / interior draw commands (uniform + texture binding)
  GoreGib.{h,cpp}        severed-piece entity + pool
  GoreFx.cpp             spurts, drips, decals, chunk gibs (through GlobalParticleSystem)
  GoreDebug.cpp          ImGui panel + console commands + stress test (compiled out of DISTRIBUTION)
  StealthGoreAdapter.cpp the only file that includes StealthAction headers (NpcBase hooks, weapon sources)
```

### 6.1 Data

`GameData/models/npc/<model>.glb.gore` (JSON, next to the model's `.skmm`):

```json
{
  "textures": { "flesh": "gore/flesh_d.png", "fleshNormal": "gore/flesh_n.png",
                "blood": "gore/blood_soak.png", "edgeNoise": "gore/edge_noise.png", "bone": "gore/bone_d.png" },
  "fleshScale": 0.08, "bloodColor": [0.35, 0.02, 0.02],
  "submeshes": { "body": "volume", "head": "volume", "shirt": "shell", "hair": "shell", "eyes": "attachment" },
  "severable": [
    { "bone": "lowerarm_l", "range": [0.25, 0.8], "maxTilt": 35, "boneRadius": 0.012, "gib": "rigid",
      "stump": { "spurtSeconds": 4, "pulseHz": 1.6 }, "boneStub": "gore/bone_stub_arm.glb" },
    { "bone": "upperarm_l", "range": [0.2, 0.85], "maxTilt": 30, "boneRadius": 0.016, "gib": "ragdoll" },
    { "bone": "neck_01", "range": [0.1, 0.9], "maxTilt": 25, "boneRadius": 0.014, "gib": "rigid", "canDestroy": true },
    { "bone": "spine_02", "range": [0.2, 0.8], "maxTilt": 55, "boneRadius": 0.02, "gib": "ragdoll", "lethal": true }
  ]
}
```

`range` is a fraction along the bone (joint → child joint) where the plane may sit; `maxTilt` is how far the normal may
lean from the bone axis.

`GameData/gore/sources.json`: what each damage source does (type, wound size from damage, sever power, pellet
clustering, exit wounds). Every weapon, projectile and explosion entity gets a `GoreSourceComponent` filled from it by
its id (`ECS_COMPONENT` on `WeaponBase`, `Bullet` and `Explosion`; a bullet copies its weapon's source when it is fired).

| Type | Wound | Sever rule |
|---|---|---|
| `bullet` (pistol, tommy) | small entry hole, elongated along the shot; exit hole if power is enough | no, unless accumulated limb damage passes the threshold |
| `heavyBullet` (revolver, sniper) | entry + large exit with outward rim, through-hole on thin parts | head: destroy on lethal; limbs: sever on high damage |
| `pellet` (shotgun, sawed-off) | pellets batched per body per frame and clustered into craters | sum per bone region: sever limb / destroy head / torso blast-through |
| `slash` (sword, twin swords) | gash: flat ellipsoid along the blade sweep | lethal or above threshold: sever, plane = blade sweep plane |
| `blunt` (cane, crucifix) | splats and bruise darkening, no holes | lethal head hit: crush (destroy head) |
| `blast` (explosion, cannon) | big craters facing the blast | sever by distance and damage on several bones, chunk gibs |

Melee weapons already sweep their traces; they put the sweep direction and blade axis into the source data with each
hit, because the `Direction` argument of `OnPointDamage` is the camera forward today.

### 6.2 `GoreComponent` (ECS, update state)

Fixed-size arrays, no heap: `Wound wounds[8]`, `Cut cuts[4]`, `Splat splats[16]`, counts, a `std::bitset<256>` of
removed skeleton bones, accumulated damage per severable bone (decays), bleed state per cut, a `uint32 version` that
bumps on every change (the mesh copies render state only when it changed), and the hit counter for RNG seeds.
Serialisable with NLOHMANN macros (§6.9). Added on the first gore hit (`AddComponent<GoreComponent>()`), so undamaged
enemies don't even carry it.

### 6.3 Hits → wounds and severs (`GoreSystem`)

1. `StealthGoreAdapter` emits a `GoreHitEvent` from `NpcBase::OnPointDamage` (and from the gib's own damage handler):
   victim, world point, direction, hitbox bone, damage, lethal flag, the source's `GoreSourceComponent` copy, seed. It is
   a POD of about 96 bytes, buffered by `EcsScheduler::Emit`.
2. The handler sorts the frame's events by victim and processes each victim once, so 21 sawed-off pellets on one
   body are one batch:
   * World → bind: `inverse(World * palette[hitSkin])` (rigid, so the inverse is a transpose plus a translation).
   * Pellets: cluster bind points (greedy, radius from source), each cluster → one crater, sized by pellet count.
   * Bullets: exit point by ray vs the hit hitbox's OBB (`HitboxData` size/rotation/position); exit wound if the source
     can over-penetrate.
   * Accumulate damage per severable region; decide severs from the source's power, lethal flag and region threshold.
3. Budget overflow: merge with the nearest overlapping wound (bounding ellipsoid) or replace the oldest smallest one;
   splats merge by distance.
4. Severs go to the cut solver, then: update `GoreComponent`, `RemoveHitboxes` (E6) for the subtree on the body, queue
   the gib spawn, emit `GoreSeveredEvent { victim, bone, wasLethal }`.

### 6.4 Cut solver

Input: severable bone S, desired world plane (blade sweep plane, or the plane facing the blast). In bind space:

1. Move the plane origin onto the bone segment, clamped to `range`; limit the tilt to `maxTilt`.
2. **Validity**: every descendant bone of S (any-influence bounds, E4) must lie entirely on the removed side, so the
   body may skip their index ranges and nothing is left floating. If not, slerp the normal toward the bone axis and
   slide toward the child joint (binary search, ≤ 8 steps; each step tests ≤ 8 corners × subtree bones).
3. Output the bind plane, the bone-core center and radius on the plane (for the marrow ring), and the index ranges:
   body = everything minus `subtree(child of S)` intervals; gib = `subtree(S)` + own ranges of ancestors whose own-AABB
   crosses the plane.

Result: no two cuts look the same (angle and position follow the actual hit), and topology stays simple and safe.

### 6.5 `GoreSkeletalMesh : SkeletalMesh` (render)

* Role: `Body` or `Gib`. With no gore state it is a plain `SkeletalMesh`: default shaders, no extra commands.
* `CreateDrawCommand()` returns `GoreFrontCommand` (derived from `SkeletalMeshDrawCommand`), which overrides the E1
  shader getters to the gore programs when it's active and binds gore uniforms and textures in
  `ApplyAdditionalShaderParams`.
* Per wounded submesh with role `volume`/`shell`, one `GoreInteriorCommand` (front-face cull, E2; ranges limited to
  bones whose own-AABB touches a wound or cut, E3/E4). Added in an override of `CollectDrawCommands`.
* `PreFinalize` (parallel): if `GoreComponent::version` changed, snapshot the bind data. Every frame, compute the world
  inverses (§4.1) and the **render palette**:
  * Body: bones past a cut get `palette[S]` (rigid with the cut bone), for planar shadow flattening (§4.5).
  * Gib: bones outside the kept set follow S rigidly:
    `skin_b(t) = skin_S(t) · inverse(skin_S(t0)) · skin_b(t0)`, so ancestor-weighted vertices near the cut never
    stretch toward a frozen skeleton.
* `FinalizeFrameData` (main): copy the finished arrays into the commands (plain data, no callbacks; obeys
  `IDrawCommand`'s render-thread rule).
* `GetBoundingBox`: subset bounds (E7) for bodies missing parts; gibs in ragdoll use the existing hitbox-based bounds.

### 6.6 Gibs (`GoreGib : Entity`)

* Spawned through the engine spawner. The mesh is a `GoreSkeletalMesh(Gib)` on the same model path (the AssetRegistry
  cache shares the GPU buffers), with the body's current pose pasted in and the animation paused.
* Physics: copy `metaData.hitboxes`, keep the subtree's hitboxes (`gib: rigid` → only S's), clear S's `parentBone`,
  `CreateHitboxes`, `StartRagdoll`. Initial velocities = the body's hitbox velocities at the cut + an impulse from the
  source (slash sweep / pellet direction / blast).
* Gibs keep hitboxes, so they can be shot again (a severed head can still be blown apart).
* Pool: `maxGibs` (48 default, data). When full, the oldest gib sinks into the floor over 1 s, then is destroyed.
  Off-screen gibs older than N seconds go first.
* **Destroyed head** (`canDestroy`): no head gib. Neck cut + burst FX + 3–6 chunk gibs (artist skull/brain/meat chunk
  meshes, single rigid bodies, same pool).

### 6.7 FX

* **Stump spurts**: per cut, pulsing arterial jets at the cut center along the normal (`pulseHz`, decaying over
  `spurtSeconds`), on both body and gib; drips after.
* Hit sprays: the current `hit_flesh` stays, scaled by gore type; exit wounds spray out of the back.
* Floor and wall blood decals where drops and spray land (existing decal shader), capped per frame.
* Optional bone stub mesh at the cap center along the bone axis (`boneStub`), attached to the cut bone.
* Everything goes through `GlobalParticleSystem::SpawnParticleAt` with a global per-frame particle budget.

### 6.8 Living enemies (`StealthGoreAdapter` + `NpcBase`)

`GoreSeveredEvent` handler on the NPC:

| Lost | Reaction |
|---|---|
| Head / upper body | death (ragdoll) |
| Weapon arm | drop the weapon (physics pickup), switch to melee/unarmed or flee per combat behaviour; a two-handed weapon drops on either arm |
| Off arm | keeps fighting, worse aim (data) |
| Leg | stunned ragdoll fall, then **crawl** locomotion until bled out (needs crawl clips, §8) |
| Any cut | bleed-out: health drains per second per cut (data); AI uses the existing "in trouble" investigation for allies |

The invisible subtree keeps animating (its hitboxes are gone and it isn't drawn), so no animation graph changes are
needed except the crawl state.

### 6.9 Save/load

`GoreComponent` is NLOHMANN-serialised into the NPC's save record. On load the component is restored before the first
render, the cuts re-run `RemoveHitboxes`, and the mesh rebuilds its render state from `version`. No gib records.

### 6.10 Network

`GoreHitEvent` is built from the replicated `OnPointDamage` call on every peer. The seed comes from
`hash(victim id, per-victim hit counter)`, and pellet clustering and the cut solver are deterministic given the same
inputs, so wounds and cuts match. Gib spawn and physics are local.

---

## 7. Performance plan

| Item | Cost | Control |
|---|---|---|
| Undamaged enemy | unchanged | gore path only once a `GoreComponent` exists |
| Hit handling | batched per victim per frame; wound ≈ 1 µs; sever ≈ 10–20 µs | none needed |
| Gib spawn (entity + Jolt bodies) | the expensive part | ≤ 6 spawns/frame, queue ≤ 3 frames; beyond that or far away: `rigid` gib; far and off-screen: burst only |
| Per-frame mesh prep | ~12 mat mults + uniform packing per wounded body | runs in `PreFinalize` (parallel) |
| Uniform upload | active count only (~25 vec4 typical) × passes | if profiling shows it matters: move gore data into one per-frame RGBA32F data texture (one row per body), one upload per frame |
| Front FS | branch on `v_goreMask` | far from wounds ≈ `fs_default` |
| Interior FS | only bone ranges near wounds; early ray-miss discard; late-Z only on those triangles | ranges keep it local |
| Shadow volumes | only bodies with cuts switch shader; ranges skip severed subtrees | small gibs: no detail shadow (data) |
| Particles/decals | global per-frame budget | data |

Verification: a `-gore_stress <count>` launch option (spawns N enemies and sprays mixed gore hits every frame),
measured with a Tracy capture (same workflow as SlasherGame's `-perf_crowd`). Targets: 300 enemies, 50 hit per frame,
no frame over budget; undamaged-crowd frame time within noise of today.

## 8. Art requirements

* **Flesh texture set** (albedo + normal, tileable, triplanar): muscle and fat, wet. This is the "blood texture for the
  insides". One set can serve all humans; a profile may override it (e.g. vampire or zombie flesh).
* **Blood soak** overlay texture (tileable) and **edge noise** (tileable grayscale for ragged edges). One shared set.
* Optional bone texture, bone stub meshes (arm, leg, neck), chunk meshes (skull pieces, brain, meat).
* Enemy body meshes must be **closed** (watertight) where they can be cut or holed. Clothes may be open shells but must
  be separate submeshes, named so the profile can assign roles.
* Hitboxes in `.skmm` must cover every severable bone (they already exist for the ragdoll).
* Crawl animations (forward crawl, turn, death) for leg loss on living enemies.
* Particles: arterial spurt, drip, burst, chunk spray.

## 9. Phases

1. **Engine extension points E1–E7.** Check that ShooterGame, StealthAction and SlasherGame scenes render identically
   (screenshots before/after) and that load times barely change.
2. **Wounds.** `GoreTypes`, `GoreProfile`, `GoreComponent`, `GoreSystem` (no severing), `GoreSkeletalMesh` front +
   interior + depth passes, sources for all weapons. ImGui panel to place wounds by clicking and to draw volumes with
   `DebugDraw`.
3. **Severing.** Cut solver, caps, hitbox removal, gibs (rigid then ragdoll), shadow-volume variants, render palette,
   stump FX.
4. **Gameplay.** Living-enemy reactions, bleed-out, head destroy, blast/chunks, weapon drop.
5. **Persistence and network.** Save/load and seed determinism (two peers, same hit sequence, compare wound lists).
6. **Scale and polish.** Stress test + Tracy, data-texture switch only if measured, rim displacement, pucker, decals.

## 10. Alternatives considered

| Option | Why not (or not now) |
|---|---|
| L4D2 authored wound meshes | must fit each body exactly, art cost per wound × model, a lip around the edge; the analytic interior gives cavities of any size for free |
| CPU mesh slicing (real cut geometry, re-skinned) | per-cut CPU cost and memory per instance, async complexity, LOD-like duplication; bad for 50 cuts in a frame |
| Bone-scale collapse only (shrink the limb to a point) | cuts only at joints, needs authored caps, no holes |
| Pre-authored severed limb meshes | combinatorial art (L4D2's 24,000-variation problem) |
| Per-enemy UV-space blood render targets | best for unlimited blood painting, but memory and extra GPU passes per hit; analytic splats in bind space cover the brutal look for now |
| Index buffer per severed state | the bone-partitioned buffer (E4) gives the same result with ranges and no extra buffers |

## 11. Risks and checks

* **Closed meshes**: an open body mesh shows holes in caps. Add a load-time check (in `GoreDebug`) that reports open
  edges of `volume` submeshes.
* **Depth equality** between the pre-pass and forward interior programs: shared include + 1-ulp bias; verify on DX11,
  Vulkan and GL.
* **Joint deformation** vs rigid world volumes: keep cavities anchored on the dominant bone; check elbows and knees at
  extreme poses.
* **E4 reorder**: confirm vertex-cache impact is negligible (GPU timing of the crowd before/after).
* **Hitbox name match**: the gore profile, hitboxes and skeleton must agree on bone names (UE-style `lowerarm_l`,
  `calf_l`, `neck_01`); the profile loader validates them and logs mismatches.
