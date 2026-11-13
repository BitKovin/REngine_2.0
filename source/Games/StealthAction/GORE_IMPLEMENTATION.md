# Gore / Dismemberment — Step-by-step Implementation Plan

This is the **how**. `GORE_DESIGN.md` is the **why** (read §1–§3 once before starting). Where the two differ, **this
file wins**; the differences are listed in §0.3.

Every task says which files to touch, what to write, and how to check it. Do the tasks **in order**. Do not start a
task before the previous one compiles and passes its check. Do not "improve" the plan while implementing: if something
in the plan looks wrong, stop and ask.

---

## 0. Read this first

### 0.1 Rules (breaking any of these is a bug)

1. **The engine gets no gore code.** Engine tasks (phase 1) add general features only. No word "gore" in `source/Engine`.
2. **Engine changes stay small and keep existing behaviour.** Every existing mesh must render exactly as before.
   After phase 1, ShooterGame, StealthAction and SlasherGame must look identical.
3. **All game code goes into `source/Games/StealthAction/Gore/`** and shaders into
   `source/Games/StealthAction/GameData/shaders/source/gore/`. Only `Gore/StealthGoreAdapter.cpp` may include
   StealthAction headers (`NpcBase.h`, weapons, ...). StealthAction files may include Gore headers.
4. **Files are read only through `FileSystemEngine`** (`FileSystemEngine::ReadFile(path)` returns the text). Never
   `std::ifstream`, never `fopen`.
5. **Data is loaded in `LoadAssets`** (or a static loader called from it), never in a constructor, never per frame.
6. **Use the engine's own framework directly**: `Entity`, `AddComponent/GetComponent/HasComponent`,
   `REGISTER_COMPONENT`, `REGISTER_EVENT`, `EVENT_HANDLER`, `TICK_ENTITY`/`DECLARE_SYSTEM`, `EcsScheduler::Emit`,
   `EntityHandle`. No wrapper layers, no "manager of managers".
7. **Search before writing a helper.** `MathHelper`, `BoundingBox`, `glm`, `StringHelper`, `JsonHelper` already exist.
   `grep` for it first.
8. **No heap allocation per frame** in the gore render path (`PreFinalize`, `FinalizeFrameData`, `Draw*`). Use fixed
   arrays and vectors that keep their capacity (`clear()` + `push_back`, never re-create).
9. **Undamaged enemies cost nothing extra.** If an entity has no `GoreComponent`, no gore code runs for it and its
   mesh uses the default shaders and draws exactly as before.
10. **Draw commands never call back into a mesh or entity** (`IDrawCommand.h` rule). Everything a `Draw*` function
    needs is copied into the command in `FinalizeFrameData`, or is a pointer to an array the mesh owns and only
    changes in `FinalizeFrameData`.
11. **Shaders** are compiled with `source/Games/StealthAction/GameData/shaders/ShaderCompilationAutomation.exe`
    (it compiles every `vs_*.sc` / `fs_*.sc` under `source/` for all platforms, using the nearest `varying.def.sc`).
    Shaders must compile for **all** targets (dx11, gl 440, spirv, gles 300_es).
12. **New files** must be added to the project: `StealthAction.vcxproj` **and** `StealthAction.vcxproj.filters`
    (CMake globs automatically). Engine files go into `Engine/Engine.vcxproj` + `.filters` too.
13. **Commit only when the user asks. Never push.**
14. **Log every engine change** in §9 of this file (file, what, why) as you do it.

### 0.2 Index spaces — the #1 source of mistakes

A model has **two different bone numberings**. Mixing them up gives wrong skinning silently.

| Name in this plan | What it is | Range | Where it comes from |
|---|---|---|---|
| **skel index** (`uint16_t skel`) | position in `model->skeleton.bones[]` (`roj::FlatSkeleton`). DFS pre-order: `bones[i].parentIdx < i`, and the **subtree of bone `i` is exactly `[i, subtreeEnd[i])`**. Contains every node, also non-skin ones. | `0 .. skeleton.boneCount-1` | `skeleton.findBone(name)` |
| **skin index** (`int skin`) | slot in the bone palette (`SkeletalMesh::boneTransforms`, `finalBonesMatrices` in shaders, `VertexData::BlendIndices`, `model->boneBounds`). Equals `BoneInfo::id`. | `0 .. MAX_SKINNED_BONES-1` (256) | `skeleton.bones[skel].skinIdx` (`-1` = node is not a skin bone) or `model->boneInfoMap[name].id` |

Rules:
* Index buffers, subtrees, cut solver, "removed bones" → **skel index**.
* Palette, vertex weights, `boneBounds` → **skin index**.
* Convert with `skeleton.bones[skel].skinIdx` (skel → skin) and the table `skinToSkel` built in task E4.1 (skin → skel).
* A **severable bone must have `skinIdx >= 0`**. The profile loader rejects others.

Matrices (glm, column-major):
* **skin matrix** `palette[skin]` (= `boneTransforms[skin]`) maps **bind space → current model space**.
* `World` = `mesh->GetWorldMatrix()` (gameplay) / `finalizedWorld` (render, after `StaticMesh::PreFinalize`).
* bind → world: `World * palette[skin]`.  world → bind: `inverse(World * palette[skin])`.
* Transform a **point** with `vec3(M * vec4(p, 1))`. Transform a **plane normal** from space A to B with
  `normalize(transpose(inverse(mat3(M))) * n)` where `M` maps A → B points.
* **"3 rows" upload**: to send the affine part of a mat4 `M` as 3 vec4, write `out[i] = glm::row(M, i)` for
  `i = 0,1,2` (`#include <glm/gtc/matrix_access.hpp>`). In GLSL: `q = vec3(dot(r0, vec4(p,1.0)), dot(r1, ...), dot(r2, ...))`.

Times:
* `Time::GameTime` (double) for effects (spurts, fades).
* **Regen clock** (§P5.1) for everything that heals or regrows. Never step timers per frame; store start times.

### 0.3 Differences from GORE_DESIGN.md (intentional)

1. **No `v_goreMask` varying.** Every gore draw is split into index ranges, and each range carries a vec4 mask of the
   primitives that can touch it (`u_goreRangeMask`). Ranges with an empty mask take the normal shading path. This
   is used by the front pass too, not only the interior pass. Fewer vertex uniforms, same locality.
2. **Cuts have an influence sphere.** A cut plane is infinite; without a limit it would also clip the head when an
   arm is cut if the head is on the positive side. Each cut stores a bind-space sphere (centre on the cut,
   radius covering the removed subtree). Outside it the cut does nothing.
3. **The gore state is pushed into the mesh**, not read from ECS by the mesh: `GoreSystem` (game thread) calls
   `GoreSkeletalMesh::SetGoreState(...)` when the state changes. `PreFinalize` never touches ECS.
4. **Hit batching** uses a pending-hit buffer inside `GoreComponent`, filled by the event handler and consumed by a
   tick system (`EVENT_HANDLER` runs once per event, so it can't batch).
5. **Gibs cast no detail shadow volumes** in this version (only the body gets the gore shadow shaders).
6. **Crawl**: first, losing a leg puts a living enemy into the stunned ragdoll until the leg regrows (P4.2). Then the
   crawl clips are made in Blender through the Blender MCP (P4.5) and the enemy falls with the ragdoll and
   immediately goes on crawling (P4.6).
7. **Suppression of regeneration** uses a per-body regen clock that stops while suppressed (P5.1), so all heal and
   regrow timers stay pure functions of stored values.
8. **Splats are limited to 16 per body and not per-range culled by distance only** — they also get mask bits (z).

### 0.4 Phases (each ends in a working game)

| Phase | Result | Section |
|---|---|---|
| 1 | Engine extension points E1–E7. No visible change anywhere. | §1 |
| 2 | Bullet holes, craters, gashes with cavities and blood on living and dead enemies. | §2 |
| 3 | Severing: caps, bone + marrow, gibs with physics, stump spurts, correct shadows. | §3 |
| 4 | Gameplay: consequences on living enemies, bleed-out, head destroy, blast chunks, body rules (`mortal`), crawl clips made in Blender + crawl locomotion. | §4 |
| 5 | Regeneration and durability (vampire). | §5 |
| 6 | Save/load and network determinism. | §6 |
| 7 | Stress test, profiling, polish (rim displacement, pucker, decals, gore level). | §7 |

---

## 1. Phase 1 — Engine extension points

Before you start: take screenshots of one scene in each game (ShooterGame, StealthAction, SlasherGame) with enemies
in view, from fixed camera positions. You compare against them at the end of phase 1.

### E1 — Shader choice per draw command

Files: `Engine/DrawCommands/StaticMeshDrawCommand.h/.cpp`, `Engine/StaticMesh.h`, `Engine/StaticMesh.cpp`.

1. In `StaticMeshDrawCommand` (header), next to `PixelShader`, add:
   ```cpp
   std::string VertexShader = "vs_default";
   ```
2. Add these virtual functions to `StaticMeshDrawCommand` (public). Defaults reproduce today's behaviour exactly:
   ```cpp
   virtual Shader* GetForwardShader() const
   { return ShaderManager::GetShaderProgram(VertexShader, PixelShader); }

   virtual Shader* GetDepthShader() const
   { return ShaderManager::GetShaderProgram(VertexShader, Material.NeedsAlphaDiscard() ? "fs_mask" : "fs_empty"); }

   virtual Shader* GetCustomIdShader() const
   { return ShaderManager::GetShaderProgram(VertexShader, "fs_customId"); }

   virtual void GetShadowVolumeShaders(Shader*& cap, Shader*& edge) const
   {
       cap  = ShaderManager::GetShaderProgram("shadowVolume/vs_shadowvolcap",  "shadowVolume/fs_shadowvol");
       edge = ShaderManager::GetShaderProgram("shadowVolume/vs_shadowvoledge", "shadowVolume/fs_shadowvol");
   }
   ```
   (`ShaderManager.h` is already included via `StaticMeshColorEmissiveMaterial.h`.)
3. Replace the uses in `StaticMeshDrawCommand.cpp`:
   * `DrawForward`: `Material.GetForwardShader(PixelShader)` → `GetForwardShader()`.
   * `DrawDepth` and `DrawShadow`: `Material.GetDepthShader()` → `GetDepthShader()`.
   * `DrawCustomId`: `ShaderManager::GetShaderProgram("vs_default", "fs_customId")` → `GetCustomIdShader()`.
   * `DrawShadowVolumeStencil`: the two `GetShaderProgram` lines → `Shader* capShader; Shader* edgeShader; GetShadowVolumeShaders(capShader, edgeShader);`
   * `GetSortKey()`: `Material.GetForwardShader(PixelShader)` → `GetForwardShader()`.
   Leave `StaticMeshColorEmissiveMaterial::GetForwardShader/GetDepthShader` in place (other code may use them; grep).
4. `StaticMesh.h`: add `string VertexShader = "vs_default";` next to `PixelShader` (protected) and a public setter
   `void SetVertexShader(string name) { VertexShader = name; }` next to `SetPixelShader`.
5. `StaticMesh::FinalizeFrameData` (`StaticMesh.cpp`): next to `cmd.PixelShader = PixelShader;` add
   `cmd.VertexShader = VertexShader;`.

Check: compiles; nothing looks different.

### E2 — Cull mode per draw command

Files: `StaticMeshDrawCommand.h/.cpp`.

1. Add to `StaticMeshDrawCommand`:
   ```cpp
   enum class CullFace : uint8_t { Back, Front, None };
   CullFace Cull = CullFace::Back;

   // TwoSided wins (old behaviour). Back = what every draw used before (CW).
   BgfxStateManager::Cull ResolveCull() const
   {
       if (TwoSided || Cull == CullFace::None) return BgfxStateManager::Cull::None;
       return Cull == CullFace::Front ? BgfxStateManager::Cull::CCW : BgfxStateManager::Cull::CW;
   }
   ```
2. Replace every `TwoSided ? BgfxStateManager::Cull::None : BgfxStateManager::Cull::CW` (4 places: DrawForward,
   DrawDepth, DrawCustomId, DrawShadow) with `ResolveCull()`. Do **not** touch `DrawShadowVolumeStencil` (it uses
   `Cull::None` on purpose).

Check: compiles; nothing looks different.

### E3 — Index sub-ranges per draw command

Files: `Engine/Shader.hpp/.cpp`, `StaticMeshDrawCommand.h/.cpp`.

1. **`Shader::Submit` discard flags.** Change the declaration to
   `void Submit(uint16_t viewId, uint8_t discardFlags = BGFX_DISCARD_ALL) const;` and the body's last line to
   `bgfx::submit(viewId, m_program, 0, discardFlags);`. (bgfx signature: `submit(ViewId, ProgramHandle, uint32_t depth, uint8_t flags)`.)
2. **vec4 range setter.** Add to `Shader` next to the `const glm::mat4* values, size_t count` overload:
   ```cpp
   void SetUniform(const std::string& uname, const glm::vec4* values, size_t count);   // first `count` of an array
   ```
   Implement it like the `std::vector<glm::vec4>` overload (`FindUniformIndex`, `SafeUniformNum`, `assign`) but
   with the pointer and count. Return early if `count == 0`.
3. Add to `StaticMeshDrawCommand.h` (outside the class, before it):
   ```cpp
   struct DrawIndexRange
   {
       uint32_t first = 0;   // first index (not triangle)
       uint32_t count = 0;   // number of indices (multiple of 3)
       vec4     param = vec4(0.0f);   // uploaded to RangeParamUniform before this range, if set
   };
   ```
   and in the class:
   ```cpp
   // Empty = draw the whole buffer (old behaviour). Kept as members so their capacity is reused every frame.
   std::vector<DrawIndexRange> IndexRanges;
   std::vector<DrawIndexRange> ShadowCapRanges;
   std::vector<DrawIndexRange> ShadowEdgeRanges;
   const char* RangeParamUniform = nullptr;   // e.g. "u_goreRangeMask"; nullptr = no per-range uniform

   protected:
   // Sets the index buffer and submits once per range. Call AFTER all uniforms/textures/vertex buffer are set.
   void SubmitIndexed(Shader* shader, bgfx::IndexBufferHandle ib, const std::vector<DrawIndexRange>& ranges, bgfx::ViewId viewId);
   public:
   ```
4. Implement `SubmitIndexed` in the .cpp exactly like this:
   ```cpp
   void StaticMeshDrawCommand::SubmitIndexed(Shader* shader, bgfx::IndexBufferHandle ib,
       const std::vector<DrawIndexRange>& ranges, bgfx::ViewId viewId)
   {
       if (ranges.empty())
       {
           bgfx::setIndexBuffer(ib);
           BgfxStateManager::Apply();
           shader->Submit(viewId);
           return;
       }

       // Find the last non-empty range: it must carry BGFX_DISCARD_ALL.
       int last = -1;
       for (int i = 0; i < (int)ranges.size(); ++i)
           if (ranges[i].count > 0) last = i;

       if (last < 0)
       {
           bgfx::discard(BGFX_DISCARD_ALL);   // nothing to draw: drop the vertex buffer/state already set
           return;
       }

       BgfxStateManager::Apply();
       for (int i = 0; i <= last; ++i)
       {
           const DrawIndexRange& r = ranges[i];
           if (r.count == 0) continue;
           if (RangeParamUniform) shader->SetUniform(RangeParamUniform, r.param);
           bgfx::setIndexBuffer(ib, r.first, r.count);
           shader->Submit(viewId, i == last ? BGFX_DISCARD_ALL : BGFX_DISCARD_INDEX_BUFFER);
       }
   }
   ```
   `BGFX_DISCARD_INDEX_BUFFER` keeps the vertex buffer, state, stencil, textures and uniforms for the next range.
   `Shader::Submit` re-flushes the buffered uniforms each time; that is correct (just a bit of extra work).
5. Use it. In `DrawForward`, `DrawDepth`, `DrawCustomId`, `DrawShadow`: replace
   ```cpp
   bgfx::setIndexBuffer(IndexBuffer);
   BgfxStateManager::Apply();
   shader->Submit(ViewIdManager::GetCurrentId());
   ```
   with `SubmitIndexed(shader, IndexBuffer, IndexRanges, ViewIdManager::GetCurrentId());`.
   In `DrawShadowVolumeStencil`: replace the cap block's `bgfx::setIndexBuffer(ShadowCapIbh); BgfxStateManager::Apply(); capShader->Submit(viewId);`
   with `SubmitIndexed(capShader, ShadowCapIbh, ShadowCapRanges, viewId);` and the same for edges with
   `ShadowEdgeRanges`. Keep `bgfx::setStencil` and `bgfx::setVertexBuffer` before it.

Check: compiles; nothing looks different (all range vectors are empty).

### E4 — Bone-partitioned index buffers

This is the biggest engine task. Read `skinned_model.hpp/.cpp`, `Helpers/Mesh/shadow_volume.hpp/.cpp` and
`AssetRegistry.cpp` (functions around lines 390–410, 486–500, 726–765, 864–880) before you start.

Goal: in every skinned submesh, triangles are sorted by the **skel index** of the lowest common ancestor (LCA) of all
bones that influence them. Then for each skel bone `b`:
* own triangles = indices `[boneIndexFirst[b], boneIndexFirst[b+1])`
* subtree triangles = indices `[boneIndexFirst[b], boneIndexFirst[subtreeEnd[b]])`

#### E4.1 New engine file `Engine/Helpers/Mesh/bone_partition.hpp/.cpp` (namespace `roj`)

```cpp
#pragma once
#include <glm.h>
#include <vector>
#include <cstdint>

namespace roj
{
    struct FlatSkeleton;
    struct SkinnedMesh;

    // Skeleton topology in a form the partition needs. Built from a FlatSkeleton.
    struct BonePartitionContext
    {
        std::vector<uint16_t> parent;         // parent[skel], INVALID_BONE_IDX for root
        std::vector<uint16_t> subtreeEnd;     // subtree of skel b = [b, subtreeEnd[b])
        std::vector<uint16_t> skinToSkel;     // skinToSkel[skin], INVALID_BONE_IDX if unused; size MAX_SKINNED_BONES
        bool valid = false;
    };

    BonePartitionContext BuildBonePartitionContext(const FlatSkeleton& skeleton);

    // Lowest common ancestor. Works because parent index < child index (DFS pre-order).
    inline uint16_t BoneLCA(const BonePartitionContext& ctx, uint16_t a, uint16_t b)
    {
        while (a != b)
        {
            if (a > b) a = ctx.parent[a]; else b = ctx.parent[b];
        }
        return a;
    }

    // Reorders mesh.indices (stable, by LCA) and fills mesh.boneIndexFirst, mesh.boneOwnBounds, mesh.boneAnyBounds.
    void PartitionMeshByBone(SkinnedMesh& mesh, const BonePartitionContext& ctx);
}
```

Implementation details (write them exactly):
* `BuildBonePartitionContext`: `n = skeleton.boneCount`. If `n == 0` return `{valid=false}`.
  `parent[i] = skeleton.bones[i].parentIdx`. `subtreeEnd[i] = i + 1` for all, then
  `for (int i = n - 1; i >= 1; --i) { p = parent[i]; if (p != INVALID_BONE_IDX) subtreeEnd[p] = max(subtreeEnd[p], subtreeEnd[i]); }`.
  `skinToSkel.assign(MAX_SKINNED_BONES, INVALID_BONE_IDX)`; for each `i` with `0 <= skinIdx < MAX_SKINNED_BONES`:
  `skinToSkel[skinIdx] = i`. Also verify `parent[i] < i` for every `i > 0` with a parent; if not, log an error and
  return `valid=false` (then no partition happens and gore is disabled for that model).
* The root's LCA fallback: if `parent` of the root is `INVALID_BONE_IDX`, `BoneLCA` never walks above it because two
  bones of the same tree always meet at or before the root. If two bones are in **different roots** (a skeleton with
  several roots), the loop would read `parent[INVALID]`: guard it — if `a` or `b` becomes `INVALID_BONE_IDX`,
  return `0`.
* `PartitionMeshByBone`:
  1. `triCount = indices.size() / 3`. For each triangle `t`, `key = INVALID`. For each of its 3 vertices and each
     of the 4 influences with `BlendWeights[k] > 0`: `skel = ctx.skinToSkel[BlendIndices[k]]` (skip INVALID);
     `key = (key == INVALID) ? skel : BoneLCA(ctx, key, skel)`. If the triangle has no influence at all, `key = 0`.
  2. Build `order` = triangle indices `0..triCount-1`; `std::stable_sort` by key.
  3. Write the new `indices` in that order (3 indices per triangle).
  4. `boneIndexFirst.assign(n + 1, 0)`: count triangles per key, prefix-sum, multiply by 3 (so the values are
     index offsets). `boneIndexFirst[n] == indices.size()` must hold (assert).
  5. `boneOwnBounds` (size n, type `BoneBound` from skinned_model.hpp): AABB of the **positions of the vertices of the
     triangles owned** by each bone (bind space = `VertexData::Position`).
  6. `boneAnyBounds` (size n, `BoneBound`): AABB of every vertex that has **any weight > 0** on that bone, keyed by
     `skinToSkel[BlendIndices[k]]`.
* Add the new fields to `roj::SkinnedMesh` (skinned_model.hpp), and clear them in `DestroyBuffers()`:
  ```cpp
  std::vector<uint32_t> boneIndexFirst;   // size skeleton.boneCount + 1, empty = not partitioned
  std::vector<BoneBound> boneOwnBounds;   // per skel bone
  std::vector<BoneBound> boneAnyBounds;   // per skel bone
  ```
  `BoneBound` is declared after `SkinnedMesh` in the header: move the `BoneBound` struct **above** `SkinnedMesh`
  (it has no dependencies).

#### E4.2 Shadow-volume buffers, same order

`BuildShadowVolumePrecomp(const SkinnedMesh& mesh)` → add a second parameter
`const BonePartitionContext* ctx = nullptr`. When `ctx && ctx->valid`:
* Cap triangles (after the degenerate filter, before "3. Cap VB + IB"): key per triangle =
  LCA of `skinToSkel[cv[i].stitchBone]` of its 3 welded vertices (skip INVALID; none → 0). Stable-sort `tri` (as
  triangles) by key. Then write caps in that order and fill `capBoneFirst` (size n+1, in **index** units: 6 indices
  per triangle).
* Edges (after `buildEdges`): key per edge = LCA of the stitch bones of `v0, v1, adj0, adj1`. Stable-sort `edges` by
  key; fill `edgeBoneFirst` (6 indices per edge).
* Add to `ShadowVolumePrecomp`: `std::vector<uint32_t> capBoneFirst, edgeBoneFirst;` and move them in its move
  constructor (the move-assign uses the move constructor, fine).
* When `ctx == nullptr` the function behaves exactly as today.

#### E4.3 Call it from every load path — after the skeleton exists, before the GPU upload

1. `UploadSkinnedMeshGPUBuffers(SkinnedMesh& mesh)` → add `const BonePartitionContext* ctx = nullptr` and pass it to
   `BuildShadowVolumePrecomp(mesh, ctx)`.
2. `ModelLoader<SkinnedMesh>::processMesh` (skinned_model.cpp ~line 1159) uploads **before the skeleton is built**.
   Remove that upload there: delete `if (!DeferGPUUpload) UploadSkinnedMeshGPUBuffers(skinMesh);`.
3. In `ModelLoader<SkinnedMesh>::load` after "Step 6" (the mesh sort) and before `PrewarmSkeletonTopology`, add:
   ```cpp
   if (!SkipVisual)
   {
       const roj::BonePartitionContext ctx = roj::BuildBonePartitionContext(m_model.skeleton);
       for (auto& mesh : m_model.meshes)
       {
           if (ctx.valid) roj::PartitionMeshByBone(mesh, ctx);
           if (!DeferGPUUpload) UploadSkinnedMeshGPUBuffers(mesh, ctx.valid ? &ctx : nullptr);
       }
   }
   ```
   Check first with `grep -n "UploadSkinnedMeshGPUBuffers" skinned_model.cpp` that no other upload remains in the
   loader. Check that the `std::sort` in step 6 moves whole `SkinnedMesh` objects (it does) — the new vectors move
   with them.
4. Async path, `AssetRegistry.cpp` (~line 756): the live model may have **different skin ids** than the parse (that's
   why `AdoptVisualTierGeometry` remaps `BlendIndices`). So partition again with the **live** skeleton, after the
   adopt:
   ```cpp
   model->AdoptVisualTierGeometry(std::move(*parsed));
   const roj::BonePartitionContext ctx = roj::BuildBonePartitionContext(model->skeleton);
   for (auto& mesh : model->meshes)
   {
       if (ctx.valid) roj::PartitionMeshByBone(mesh, ctx);
       roj::UploadSkinnedMeshGPUBuffers(mesh, ctx.valid ? &ctx : nullptr);
   }
   ```
   (The loader already partitioned with the parse's skeleton; doing it again with the live one is correct and cheap.)
5. The other `modelLoader.load(path)` call sites in `AssetRegistry.cpp` (sync paths) go through step 3. Read each
   to confirm they don't set `DeferGPUUpload = true` without uploading afterwards.

Check:
* Everything renders exactly as before (screenshots), shadows included.
* Add a temporary debug log (remove before finishing the task): for `GameData/models/enemies/humanAxe/humanAxe.glb`
  print, per submesh, the bones with own triangles and their counts. Expect torso, limbs, head bones. Sum of counts
  = index count.
* Load times: measure the level load before/after; the difference must be small (< 5%).

### E5 — Palette and skeleton access on `SkeletalMesh`

File: `Engine/SkeletalMesh.hpp/.cpp`.

Add (public):
```cpp
// Skin palette (bind space -> model space), indexed by skin index. Game thread; valid after Update/PasteAnimationPose.
const std::vector<mat4>& GetSkinPalette() const { return boneTransforms; }
int GetSkinIndex(const hashed_string& bone) const;        // -1 if missing (model->boneInfoMap)
uint16_t GetSkeletonIndex(const hashed_string& bone) const; // INVALID_BONE_IDX if missing (model->skeleton.findBone)
const roj::SkinnedModel* GetSkinnedModel() const { return model; }   // check the type of `model` in StaticMesh.h first
```
Add (protected):
```cpp
// The palette the draw commands read. Default: the finalized gameplay palette. Override to render with a modified one.
virtual const std::vector<mat4>* GetRenderPalette() const { return &finalizedBoneTransforms; }
// Finalized gameplay palette (FinalizeFrameData copy), for subclasses building their own render palette.
const std::vector<mat4>& GetFinalizedSkinPalette() const { return finalizedBoneTransforms; }
```
In `OnAssetsReloaded`, replace `= &finalizedBoneTransforms;` with `= GetRenderPalette();` (the field type is
`const std::vector<mat4>*`, so this compiles).

Check `model`'s declared type in `StaticMesh.h` (it is the `roj::SkinnedModel*` used everywhere above). Compiles;
nothing changes.

### E6 — Hitboxes on/off at runtime + ragdoll null-constraint fix

Files: `Engine/Physics.h/.cpp`, `Engine/SkeletalMesh.hpp/.cpp`.

1. `Physics`: add
   ```cpp
   // Takes the body out of the simulation/broadphase but keeps it allocated (re-add with AddBodyToWorld).
   static void RemoveBodyFromWorld(Body* body);
   static void AddBodyToWorld(Body* body, bool activate);
   static void RemoveConstraintFromWorld(Constraint* constraint);
   static void AddConstraintToWorld(Constraint* constraint);
   ```
   Implement them with `physicsMainLock` held, using `bodyInterface->IsAdded(id)` before `RemoveBody`, and
   `bodyInterface->AddBody(id, activate ? JPH::EActivation::Activate : JPH::EActivation::DontActivate)` only when not
   added. Do **not** touch `existingBodies` / `bodyIdMap` (the body still exists). For constraints, read
   `Physics::DestroyConstraint` and use the same physics-system call it uses to remove the constraint, without
   destroying it; add with the matching add call.
   In `Physics::DestroyBody`, guard `bodyInterface->RemoveBody(body->GetID())` with
   `if (bodyInterface->IsAdded(body->GetID()))` so destroying an inactive hitbox is safe.
2. `SkeletalMesh::HitboxBinding`: add `bool active = true;` and `bool constraintInWorld = true;`. In
   `RefreshHitboxBindings`, keep `active`/`constraintInWorld` of a rebuilt binding with the same `boneId` (same loop
   that keeps velocities). Keep a parallel set for the ragdoll branch:
   `std::unordered_set<hashed_string> inactiveHitboxes;` (member, protected by `hitboxMutex`).
3. New public function:
   ```cpp
   // Turns the hitboxes of these bones off (out of the physics world, constraints disabled) or back on.
   void SetHitboxesActive(std::span<const hashed_string> bones, bool active);
   bool IsHitboxActive(const hashed_string& bone) const;
   ```
   Algorithm (lock `hitboxMutex`, call `RefreshHitboxBindings()` first):
   * For each binding whose bone name (`Physics::GetBodyData(body)->hitboxName`) is in `bones` and whose `active`
     differs:
     * **off**: `if (constraint) { constraint->SetEnabled(false); Physics::RemoveConstraintFromWorld(constraint); constraintInWorld = false; }`,
       then `Physics::RemoveBodyFromWorld(body)`, `active = false`, insert the name in `inactiveHitboxes`.
     * **on**: compute the bone's current world transform exactly like the kinematic branch of `UpdateHitboxes`
       does (`world * boneTransforms[boneId] * inverseOffset`, `decomposeMod`), call
       `Physics::SetBodyPositionAndRotationNoActivation`, then `Physics::AddBodyToWorld(body, InRagdoll)`. If
       `InRagdoll`, also set its motion type/layer/mask exactly as `StartRagdoll` does for one body. If it has a
       constraint and its parent hitbox is active: `Physics::AddConstraintToWorld`, `SetEnabled(InRagdoll)`,
       `constraintInWorld = true`. `active = true`, erase from `inactiveHitboxes`.
   * Also, for **children** whose parent hitbox just went off: disable and remove their constraints
     (a constraint to a body that is not in the world must never be enabled). Find children through
     `metaData.hitboxes[i].parentBone`.
4. Skip inactive hitboxes everywhere:
   * `UpdateHitboxes` kinematic branch: `if (!binding.active) continue;` at the top of the loop.
   * `UpdateHitboxes` ragdoll branch — **this also fixes the existing crash**:
     * in the first loop over `metaData.hitboxes`: `if (data.parentBone == "") continue;` then
       `auto it = hitboxConstraints.find(data.boneName); if (it == hitboxConstraints.end() || it->second == nullptr) continue;`
       and `if (inactiveHitboxes.contains(data.boneName) || inactiveHitboxes.contains(data.parentBone)) continue;`.
       Use `it->second->SetEnabled(true)`.
     * in the motor loop use `find` too: never `hitboxConstraints[...]` (operator[] inserts a null pointer).
     * in the loop over `hitboxBodies`: skip bodies whose `hitboxName` is in `inactiveHitboxes`.
   * `StartRagdoll`: skip inactive bindings; don't enable constraints with `constraintInWorld == false`.
   * `StopRagdoll`: iterate `hitboxBindings` (call `RefreshHitboxBindings()` under the lock), skip inactive; only
     disable constraints that are in the world.
   * `ApplyImpulseToAllHitboxes`: skip bodies in `inactiveHitboxes`.
   * `GetBoundingBox` ragdoll branch: skip bodies in `inactiveHitboxes`.
   * `ClearHitboxes`: first turn every inactive hitbox back on (`SetHitboxesActive` with the whole
     `inactiveHitboxes` set copied into a vector, `true`), then destroy as today. This keeps
     `DestroyConstraint` safe. Clear `inactiveHitboxes` at the end.

Check: compiles. StealthAction: kill enemies, ragdolls look as before. Temporary test (remove afterwards): on a key,
call `SetHitboxesActive({"lowerarm_l","upperarm_l"}, false)` on the nearest enemy, shoot that arm — no hit; kill it —
the ragdoll works without crash; turn them on again — the arm is hit again.

### E7 — Bounds over a bone subset

File: `skinned_model.hpp/.cpp`.

Add an overload:
```cpp
// Like ComputeAnimatedBounds, but only skin bones with skinMask[s] != 0 are used (skinMask.size() >= skinMatrices.size()).
BoundingBox ComputeAnimatedBounds(const glm::mat4& worldMatrix, std::span<const glm::mat4> skinMatrices,
                                  std::span<const uint8_t> skinMask) const;
```
Same body as the existing one with `if (!skinMask[s]) continue;` added.

Check: compiles.

### Phase 1 exit check

* Screenshot comparison in all three games: identical.
* Detail shadows on StealthAction NPCs: identical.
* §9 engine change log filled for E1–E7.

---

## 2. Phase 2 — Wounds (no severing yet)

### P2.1 Folder, project files, shared types

Create `source/Games/StealthAction/Gore/` with the files below as they come up. Add each to `StealthAction.vcxproj`
and `.filters` (filter folder `Gore`).

`Gore/GoreTypes.h`:
```cpp
#pragma once
#include <glm.h>
#include <cstdint>
#include <array>
#include <string>
#include <json.hpp>

namespace Gore
{
    constexpr int kMaxWounds = 8;
    constexpr int kMaxCuts = 4;
    constexpr int kMaxSplats = 16;
    constexpr int kMaxPendingHits = 32;
    constexpr int kMaxSkeletonBones = 512;   // models with more skeleton nodes get no gore (logged)

    // Deterministic RNG (same results on every platform/compiler - never use std:: distributions here).
    struct Rng
    {
        uint32_t state;
        explicit Rng(uint32_t seed) : state(seed ? seed : 0x9E3779B9u) {}
        uint32_t NextU32() { state ^= state << 13; state ^= state >> 17; state ^= state << 5; return state; }
        float Next01() { return (NextU32() >> 8) * (1.0f / 16777216.0f); }
        float Range(float a, float b) { return a + (b - a) * Next01(); }
    };

    uint32_t HashSeed(const std::string& victimId, uint32_t hitCounter);   // FNV-1a over id bytes, then mix counter

    enum class WoundKind : uint8_t { Entry, Exit, Crater, Gash, Crush };
    enum class CutPhase  : uint8_t { Severed, Regrowing };

    struct Wound
    {
        glm::vec3 center = glm::vec3(0);       // bind space
        glm::quat rotation = glm::quat(1,0,0,0);// bind space, local z = shot direction / blade normal
        glm::vec3 radii = glm::vec3(0.02f);    // bind-space metres
        int16_t   anchorSkin = -1;             // skin index of the anchor bone (rigid frame for the interior pass)
        uint16_t  anchorSkel = 0;
        WoundKind kind = WoundKind::Entry;
        float roughness = 0.25f;               // edge noise amplitude in unit-sphere units
        float rimWidth = 0.35f;                // blood rim width in unit-sphere units
        float seed = 0.0f;                     // 0..1, offsets the noise
        float reach = 0.3f;                    // max body thickness behind the wound (bind metres)
        double healStart = 0.0;                // regen clock; healDuration 0 = permanent
        float  healDuration = 0.0f;
        uint32_t order = 0;                    // creation order (eviction picks the oldest)
    };

    struct Cut
    {
        glm::vec4 plane = glm::vec4(0,1,0,0);  // bind: removed side is dot(n,p) - d > 0
        glm::vec3 center = glm::vec3(0);       // bind: bone centre on the plane
        float influenceRadius = 0.5f;          // bind: cut does nothing farther than this from center
        float boneRadius = 0.015f;             // bind metres
        float roughness = 0.012f;              // edge noise amplitude in bind metres
        float seed = 0.0f;
        uint16_t boneSkel = 0;                 // severed bone S (skel index)
        int16_t  boneSkin = -1;
        CutPhase phase = CutPhase::Severed;
        double severedAt = 0.0;                // Time::GameTime (FX only)
        double regrowStart = 0.0;              // regen clock (phase 5)
        float  regrowDuration = 0.0f;          // 0 = never regrows
        float  regrowMaxRadius = 0.0f;         // bind metres, covers the whole removed subtree
        bool   hitboxesBack = false;
        bool   charred = false;                // fire: never regrows, charred cap look
        std::string gibId;                     // entity id of the severed piece (empty if none / not saved)
    };

    struct Splat
    {
        glm::vec3 center = glm::vec3(0);       // bind
        float radius = 0.05f;                  // bind metres
        float strength = 1.0f;                 // 0..1
        uint32_t order = 0;
    };

    // Per-hit data from the adapter. Plain data, ~100 bytes.
    struct Hit
    {
        glm::vec3 point = glm::vec3(0);        // world
        glm::vec3 direction = glm::vec3(0,0,1);// world, normalised, direction the damage travels
        glm::vec3 sweepNormal = glm::vec3(0);  // world, blade sweep plane normal (slash only), 0 = none
        float damage = 0.0f;
        uint16_t sourceIndex = 0;              // index into GoreSources table
        char hitBone[32] = {};                 // hitbox bone name (fixed buffer - no heap in the event)
        bool lethal = false;                   // this hit killed
        bool finisher = false;                 // apply the body rules' finisher gore
        uint32_t seed = 0;
        uint32_t counter = 0;                  // per-victim hit counter (ordering + seeds)
    };
}
```
Add NLOHMANN (de)serialisation for `Wound`, `Cut`, `Splat` with `NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT`
(check `JsonHelper.hpp` for existing glm vec3/quat/vec4 converters before writing new ones — `HitboxData` already
serialises `vec3`, so they exist). `GoreTypes.cpp` holds `HashSeed`.

### P2.2 Data files and loaders

#### Model profile — `Gore/GoreProfile.h/.cpp`

File next to the model: `<modelPath>.gore` (e.g. `GameData/models/enemies/humanAxe/humanAxe.glb.gore`). Format: see
GORE_DESIGN.md §6.1. Create one for each enemy model (`humanAxe`, `humanGun`, `zombie`, `npc/base`); they can be
identical. Find the real model paths with `grep -rn "modelPath" Games/StealthAction/Entities`.

```cpp
namespace Gore
{
    enum class SubmeshRole : uint8_t { Volume, Shell, Attachment };   // attachment = no gore at all

    struct SeverableBone
    {
        std::string bone; uint16_t skel = 0; int16_t skin = -1;
        glm::vec2 range = glm::vec2(0.2f, 0.8f);
        float maxTiltDeg = 30.0f;
        float boneRadius = 0.015f;
        enum class GibKind : uint8_t { Rigid, Ragdoll } gib = GibKind::Rigid;
        bool canDestroy = false;
        bool lethal = false;
        float spurtSeconds = 4.0f, pulseHz = 1.6f;
        std::string boneStub;
    };

    struct Profile
    {
        bool valid = false;
        std::string fleshTex, fleshNormalTex, bloodTex, edgeNoiseTex, boneTex;
        Texture* flesh = nullptr; Texture* fleshNormal = nullptr; Texture* blood = nullptr; Texture* edgeNoise = nullptr; Texture* boneT = nullptr;
        float fleshScale = 12.0f;              // texture repeats per metre
        glm::vec3 bloodColor = glm::vec3(0.35f, 0.02f, 0.02f);
        std::vector<SubmeshRole> submeshRoles; // per model->meshes index (by name prefix from the file; default Volume)
        std::vector<SeverableBone> severable;
        std::vector<int16_t> severableBySkel;  // size skeleton.boneCount, index into severable or -1

        // Derived at load (skel-indexed, merged over all submeshes):
        std::vector<roj::BoneBound> anyBounds; // union of mesh.boneAnyBounds
        std::vector<uint16_t> parent, subtreeEnd;
    };

    // Loads (once, cached by model path) and validates. Returns a profile with valid=false when the file is missing,
    // the model isn't partitioned (mesh.boneIndexFirst empty), or validation fails - logged once per model.
    const Profile& GetProfile(const std::string& modelPath, const roj::SkinnedModel& model);
    void ClearProfiles();   // dev hot reload and level change
}
```
Rules for the loader:
* Read with `FileSystemEngine::ReadFile`, parse with `nlohmann::json::parse(text, nullptr, false)`; on
  `is_discarded()` log and return invalid.
* Resolve bone names with `model.skeleton.findBone` and `model.boneInfoMap`; **log every unknown bone** and skip it.
  Reject severable bones without a skin index.
* `skeleton.boneCount > kMaxSkeletonBones` → invalid.
* Textures with `AssetRegistry::GetTextureFromFile(path)` (main thread — `GetProfile` is only called from the game
  thread, in `LoadAssets` or the gore tick).
* Use `roj::BuildBonePartitionContext(model.skeleton)` for `parent`/`subtreeEnd`.
* Cache: `static std::unordered_map<std::string, Profile>`. A model that is not loaded to the Visual tier yet
  (`model.meshes.empty()`) is **not cached as invalid**: return a static invalid profile and try again next time.

#### Sources — `Gore/GoreSource.h/.cpp`, file `GameData/gore/sources.json`

```json
{
  "sources": {
    "default":   { "type": "bullet", "woundRadius": 0.012, "refDamage": 20, "exit": false, "severPower": 0, "tags": ["ranged"] },
    "pistol":    { "type": "bullet", "woundRadius": 0.012, "refDamage": 20, "exit": true,  "severPower": 0.1, "tags": ["ranged"] },
    "revolver":  { "type": "heavyBullet", "woundRadius": 0.02, "refDamage": 45, "exit": true, "severPower": 0.6, "tags": ["ranged"] },
    "sawed":     { "type": "pellet", "woundRadius": 0.01, "refDamage": 8, "clusterRadius": 0.07, "severPower": 0.08, "tags": ["ranged"] },
    "sword":     { "type": "slash", "woundRadius": 0.03, "gashLength": 0.18, "refDamage": 30, "severPower": 1.0, "tags": ["melee"] },
    "cane":      { "type": "blunt", "woundRadius": 0.0, "refDamage": 30, "severPower": 0, "tags": ["melee"] },
    "crucifix":  { "type": "blunt", "woundRadius": 0.0, "refDamage": 30, "severPower": 0, "tags": ["melee", "holy"] },
    "explosion": { "type": "blast", "woundRadius": 0.07, "refDamage": 80, "severPower": 1.0, "tags": ["blast", "fire"] }
  },
  "classMap": { "weapon_pistol": "pistol", "weapon_revolver": "revolver", "weapon_sawed": "sawed",
                "weapon_sword": "sword", "weapon_swords": "sword", "weapon_twinsword": "sword",
                "weapon_cane": "cane", "weapon_crucifix": "crucifix", "explosion": "explosion" }
}
```
Fill `classMap` from the real `ClassName` values (`grep -rn "ClassName = " Games/StealthAction/Entities/Player/Weapons`).
NPC melee attacks (`NpcHumanAxe.cpp`, `NpcKnight.cpp`, ...) pass the NPC itself as weapon: map their class names too.

```cpp
namespace Gore
{
    enum class SourceType : uint8_t { Bullet, HeavyBullet, Pellet, Slash, Blunt, Blast };
    struct Source
    {
        std::string id; SourceType type = SourceType::Bullet;
        float woundRadius = 0.012f, refDamage = 20.0f, clusterRadius = 0.07f, gashLength = 0.15f, severPower = 0.0f;
        bool exit = false;
        uint32_t tagBits = 0;            // bit per tag, see TagBit()
    };
    uint32_t TagBit(const std::string& tag);   // stable: index in a static table, max 32 tags, logs when full
    void LoadSources();                         // GameData/gore/sources.json
    uint16_t SourceIndexForClass(const std::string& className);   // "default" if unknown
    const Source& GetSource(uint16_t index);

    // Attached to projectiles so a bullet carries its weapon's source.
    struct GoreSourceComponent { uint16_t sourceIndex = 0; };
}
REGISTER_COMPONENT(Gore::GoreSourceComponent)
```

#### Settings — `Gore/GoreSettings.h/.cpp`, file `GameData/gore/settings.json`

`maxGibs` (48), `maxGibSpawnsPerFrame` (6), `maxParticlesPerFrame` (64), `level` ("full"/"reduced"/"minimal"),
`freshBand` (0.05). Plain struct with static `Get()` and `Load()`.

Call `LoadSources()`, `GoreSettings::Load()` once from `NpcBase::LoadAssets` through
`StealthGore::LoadAssets()` (adapter; guard with a static bool so it runs once per level load; reset it in
`ClearProfiles`).

### P2.3 `GoreComponent` and `GoreHitEvent`

`Gore/GoreComponent.h`:
```cpp
namespace Gore
{
    struct GoreComponent
    {
        std::array<Wound, kMaxWounds> wounds; uint8_t woundCount = 0;
        std::array<Cut,   kMaxCuts>   cuts;   uint8_t cutCount = 0;
        std::array<Splat, kMaxSplats> splats; uint8_t splatCount = 0;

        std::array<Hit, kMaxPendingHits> pending; uint8_t pendingCount = 0;

        uint32_t version = 1;      // ++ on every change of wounds/cuts/splats
        uint32_t hitCounter = 0;   // per victim, seeds
        uint32_t orderCounter = 0; // creation order for eviction

        // Regen clock (phase 5). clock(t) = t < pausedUntil ? frozenClock : frozenClock + (t - pausedUntil)
        double frozenClock = 0.0;
        double pausedUntil = 0.0;

        // Accumulated sever damage per severable bone, indexed like Profile::severable (max 16, checked by the loader).
        std::array<float, 16>  severDamage{};
        std::array<double, 16> severDamageTime{};
    };
}
REGISTER_COMPONENT(Gore::GoreComponent)
```
Accumulated damage is kept only for *severable* bones (never one entry per skeleton bone).

`Gore/GoreEvents.h`:
```cpp
struct GoreHitEvent { EntityHandle victim; Gore::Hit hit; };
REGISTER_EVENT(GoreHitEvent)
struct GorePartEvent { EntityHandle victim; uint16_t boneSkel = 0; bool lost = true; };   // phase 4
REGISTER_EVENT(GorePartEvent)
```
`EntityHandle` contains a `std::string`; that's fine for events (they are moved).

### P2.4 `GoreSkeletalMesh` (render side) and its draw command

#### Render state the system pushes into the mesh

`Gore/GoreRenderState.h`:
```cpp
namespace Gore
{
    // Snapshot of a GoreComponent for rendering. Copied into the mesh on the game thread when version changes.
    struct RenderState
    {
        uint32_t version = 0;
        bool isGib = false;                    // gib: the single cut keeps its removed side (inverted)
        std::array<Wound, kMaxWounds> wounds; uint8_t woundCount = 0;
        std::array<Cut, kMaxCuts> cuts; uint8_t cutCount = 0;
        std::array<Splat, kMaxSplats> splats; uint8_t splatCount = 0;
        double frozenClock = 0.0, pausedUntil = 0.0;   // regen clock (phase 5)
        std::vector<glm::mat4> gibSeverPalette; // gib only: body palette at the moment of the sever (phase 3)
        const Profile* profile = nullptr;      // set by the system; profiles live until the level changes
    };

    double RegenClock(double frozenClock, double pausedUntil, double t);   // formula in GoreComponent comment
}
```

#### GPU data (uniform arrays) — `Gore/GoreDrawCommands.h`

```cpp
namespace Gore
{
    struct GpuData
    {
        glm::vec4 counts = glm::vec4(0);       // x wounds, y cuts, z splats, w 0
        glm::vec4 params = glm::vec4(0);       // x homogeneousDepth (0/1), y isGib (0/1), z fleshScale, w gore level (0 full, 1 reduced, 2 minimal)
        glm::vec4 bloodColor = glm::vec4(0.35f, 0.02f, 0.02f, 1.0f);
        std::array<glm::vec4, kMaxWounds * 3> woundBind{}, woundWorld{};
        std::array<glm::vec4, kMaxWounds> woundParams{};   // x roughness, y rimWidth, z healScale, w seed
        std::array<glm::vec4, kMaxWounds> woundExtra{};    // x reach (world m), y mean radius (world m), z kind, w 0
        std::array<glm::vec4, kMaxCuts> cutBind{}, cutBindSphere{}, cutRegrowBind{}, cutParams{};
        std::array<glm::vec4, kMaxCuts> cutWorld{}, cutWorldSphere{}, cutRegrowWorld{}, cutCenter{};
        std::array<glm::vec4, kMaxCuts * 2> cutFrame{};
        std::array<glm::vec4, kMaxSplats> splats{};
        Texture* flesh = nullptr; Texture* fleshNormal = nullptr; Texture* edgeNoise = nullptr; Texture* blood = nullptr;
    };
}
```
Meaning of each cut array (index `i` = cut):
* `cutBind[i]` = `plane` (bind). `cutBindSphere[i]` = `(center, influenceRadius)`.
* `cutRegrowBind[i]` = `(center, currentRegrowRadius)`; radius `-1` when not regrowing (phase 5; `-1` in phase 3).
* `cutParams[i]` = `(roughness, seed, boneRadius, flags)` with flags as a float sum: `1` = charred, `2` = regrowing.
* world versions: `M = World * palette[cut.boneSkin]`; `cutWorld[i]` = plane transformed by `M` (§0.2);
  `cutWorldSphere[i]` = `(M * center, influenceRadius * s)`, `s = length(vec3(M[0]))`;
  `cutRegrowWorld[i]` = same for the regrow sphere (radius stays `-1` when not regrowing);
  `cutCenter[i]` = `(M * center, 0)`.
* `cutFrame[2i] = (T, boneRadius * s)`, `cutFrame[2i+1] = (B, reach * s)` (`reach` = 2 × the largest extent of
  `profile.anyBounds[cut.boneSkel]`, the body thickness at the cut) where `T`,`B` are a world tangent basis of the
  world plane normal: `T = normalize(cross(n, abs(n.y) < 0.99 ? vec3(0,1,0) : vec3(1,0,0)))`, `B = cross(n, T)`.

Wounds (index `i`):
* `Mb = translate(center) * mat4_cast(rotation) * scale(radii)` (unit sphere → bind).
* `woundBind[3i..3i+2]` = rows of `inverse(Mb)`.
* `woundWorld[3i..3i+2]` = rows of `inverse(World * palette[anchorSkin] * Mb)`.
* `woundParams[i] = (roughness, rimWidth, healScale, seed)`; `healScale = 1` in phase 2.
* `woundExtra[i] = (reach * s, mean(radii) * s, float(kind), 0)`.

Splats: `splats[i] = (center, radius * strength)`.

#### `GoreDrawCommand` — `Gore/GoreDrawCommands.h/.cpp`

```cpp
namespace Gore
{
    class GoreDrawCommand : public SkeletalMeshDrawCommand
    {
    public:
        enum class Pass : uint8_t { Front, Interior };
        Pass pass = Pass::Front;
        bool goreActive = false;                 // false: behaves exactly like SkeletalMeshDrawCommand
        bool shell = false;                      // submesh role Shell (interior draws only the dark back side)
        const GpuData* gore = nullptr;           // points at GoreSkeletalMesh::finalGpu (changes only in FinalizeFrameData)

        Shader* GetForwardShader() const override;
        Shader* GetDepthShader() const override;
        Shader* GetCustomIdShader() const override;
        void GetShadowVolumeShaders(Shader*& cap, Shader*& edge) const override;
        void ApplyAdditionalShaderParams(Shader* shader) override;

        void DrawForward(const mat4& view, const mat4& projection) override;
        void DrawDepth(const mat4& view, const mat4& projection) override;
        void DrawCustomId(const mat4& view, const mat4& projection) override;

        // Copies everything the interior command shares with its front command (buffers, transform, light, material,
        // palette pointer, flags). Field by field - do not use operator= (it would copy the range vectors' storage).
        void CopyCommonFrom(const GoreDrawCommand& front);

    private:
        mat4 viewProj = mat4(1.0f);   // set by the Draw* overrides before calling the base
        vec3 cameraPos = vec3(0.0f);
    };
}
```
Rules:
* Every getter returns the base version when `!goreActive`.
* Shader names (`vs` always `"gore/vs_gore"` when active):
  | Pass | forward | depth | custom id |
  |---|---|---|---|
  | Front | `gore/fs_gore` | `gore/fs_gore_depth` | `gore/fs_gore_customId` |
  | Interior | `gore/fs_gore_interior` | `gore/fs_gore_interior_depth` | (none: interior sets `Material.CustomId = 0`) |
* Shadow shaders when active and `gore->counts.y > 0` (has cuts) and Front pass:
  `gore/vs_shadowvolcap_gore` / `gore/vs_shadowvoledge_gore` with `shadowVolume/fs_shadowvol`. Otherwise base.
* `Draw*` overrides: `viewProj = projection * view; cameraPos = vec3(glm::inverse(view)[3]);` then call the base
  `SkeletalMeshDrawCommand::Draw*`. (Only when `goreActive`; otherwise call the base directly.)
* `ApplyAdditionalShaderParams`: call `SkeletalMeshDrawCommand::ApplyAdditionalShaderParams(shader)` first (bones).
  If active, upload (only the used counts; **always** upload `u_goreCounts`, `u_goreParams`, `u_goreRangeMask`
  default `vec4(0)` is set per range by `SubmitIndexed`):
  ```
  u_goreCounts, u_goreParams, u_goreBloodColor, u_goreViewProj (mat4), u_goreCameraPos (vec4(cameraPos,1)),
  u_goreWoundBind[woundCount*3], u_goreWoundWorld[woundCount*3], u_goreWoundParams[woundCount], u_goreWoundExtra[woundCount],
  u_goreCutBind/BindSphere/RegrowBind/Params/World/WorldSphere/RegrowWorld/Center [cutCount], u_goreCutFrame[cutCount*2],
  u_goreSplats[splatCount]
  ```
  Textures: `shader->SetTexture("u_goreFlesh", gore->flesh)` etc. (hashed_string overload takes `Texture*`).
  Uniforms that don't exist in a given program are ignored by `Shader` (`AllowMissingUniforms`), so one function
  serves every pass.
* Interior pass: `Cull = CullFace::Front`, `RangeParamUniform = "u_goreRangeMask"`, `Material.CustomId = 0`, all
  shadow handles invalid and shadow counts 0 (so `DrawShadowVolumeStencil` does nothing), `OnlyShadows = false`.
* Front pass with gore active: `RangeParamUniform = "u_goreRangeMask"` too.

#### `GoreSkeletalMesh` — `Gore/GoreSkeletalMesh.h/.cpp`

```cpp
namespace Gore
{
    class GoreSkeletalMesh : public SkeletalMesh
    {
    public:
        explicit GoreSkeletalMesh(Entity* owner) : SkeletalMesh(owner) {}

        // Game thread only (gore tick). Copies when state.version differs from the current one.
        void SetGoreState(const RenderState& state);
        void ClearGoreState();                       // back to a plain SkeletalMesh
        bool HasGore() const { return goreState.version != 0 && (goreState.woundCount || goreState.cutCount || goreState.splatCount); }

        void PreFinalize() override;
        void FinalizeFrameData() override;
        void CollectDrawCommands(std::vector<IDrawCommand*>& out) override;
        BoundingBox GetBoundingBox() override;       // phase 3 (subset bounds); phase 2: return SkeletalMesh::GetBoundingBox()

    protected:
        std::shared_ptr<StaticMeshDrawCommand> CreateDrawCommand() override { return std::make_shared<GoreDrawCommand>(); }
        void OnAssetsReloaded() override;
        const std::vector<mat4>* GetRenderPalette() const override;

    private:
        RenderState goreState;                       // written by SetGoreState (game thread), read in PreFinalize
        uint32_t builtVersion = 0;                   // goreState version the ranges were built for

        GpuData pendingGpu, finalGpu;                // PreFinalize writes pending; FinalizeFrameData copies to final
        std::vector<mat4> pendingPalette, renderPalette;
        bool usingRenderPalette = false, pendingUsePalette = false;

        // Per submesh, rebuilt when builtVersion changes:
        struct SubmeshRanges
        {
            std::vector<DrawIndexRange> front, interior, shadowCap, shadowEdge;
        };
        std::vector<SubmeshRanges> pendingRanges;

        std::vector<std::shared_ptr<GoreDrawCommand>> interiorCommands, finalInteriorCommands;
        bool pendingActive = false;
    };
}
```

Exact behaviour:

1. **`OnAssetsReloaded`**: call `SkeletalMesh::OnAssetsReloaded()`. Then rebuild `interiorCommands`: one
   `std::make_shared<GoreDrawCommand>()` per submesh, `pass = Interior`. Set `builtVersion = 0` (forces a rebuild of
   ranges). Do **not** touch `finalInteriorCommands` here (the renderer may still hold the old ones).
2. **`PreFinalize`** (parallel, one call per mesh — touch only this mesh's data):
   * `SkeletalMesh::PreFinalize();` (this is `StaticMesh::PreFinalize`, it sets `finalizedWorld`).
   * If `!HasGore() || model == nullptr || model->meshes.empty()` → `pendingActive = false; return;`.
   * The mesh never loads the profile itself (`GetProfile` is game-thread only); it uses `goreState.profile`. If
     `goreState.profile == nullptr || !goreState.profile->valid` → `pendingActive = false; return;`.
   * `pendingActive = true`.
   * If `builtVersion != goreState.version`: build ranges (step 4) and set `builtVersion`.
   * Fill `pendingGpu` (formulas above) using `finalizedWorld` and `GetSkinPalette()` (the game-thread palette; it is
     not modified while PreFinalize runs). Heal scales / regrow radii come from the regen clock at
     `Time::GameTime` (phase 5; constants in phase 2/3).
   * Render palette (phase 3): build `pendingPalette` (step 5) and set `pendingUsePalette`. Phase 2: `false`.
3. **`FinalizeFrameData`** (main thread):
   * `SkeletalMesh::FinalizeFrameData();`
   * `finalGpu = pendingGpu;` (plain struct copy, no heap).
   * If `pendingUsePalette`: `renderPalette = pendingPalette;` (same size each frame → no allocation after the first).
   * For each submesh `i` with front command `cmd = static_cast<GoreDrawCommand&>(GetDrawCommandAt(i))`:
     * `cmd.BoneMatrices = GetRenderPalette();` (cheap; re-points after a reload too).
     * `cmd.goreActive = pendingActive && role(i) != Attachment; cmd.gore = &finalGpu;`
     * if active: `cmd.VertexShader`/`PixelShader` are not used (getters overridden). Copy
       `pendingRanges[i].front` into `cmd.IndexRanges` (`assign`, capacity reused), `shadowCap/Edge` into
       `cmd.ShadowCapRanges/ShadowEdgeRanges`, set `cmd.RangeParamUniform = "u_goreRangeMask"`.
     * if not active: `cmd.IndexRanges.clear(); cmd.ShadowCapRanges.clear(); cmd.ShadowEdgeRanges.clear(); cmd.RangeParamUniform = nullptr;`
     * Interior command `ic = *interiorCommands[i]`: `ic.CopyCommonFrom(cmd)`, `ic.goreActive = cmd.goreActive && role(i) != Attachment && !pendingRanges[i].interior.empty()`,
       `ic.shell = role(i) == Shell`, `ic.IndexRanges.assign(pendingRanges[i].interior...)`, and the interior settings
       listed above.
   * `finalInteriorCommands = interiorCommands;` (shared_ptr copy — keeps them alive while the renderer uses them).
4. **Ranges** (in PreFinalize, only when the version changed). For each submesh `m` with `mesh.boneIndexFirst` not
   empty (else: no ranges, gore inactive for that submesh):
   * `n = boneIndexFirst.size() - 1`.
   * For each skel bone `b` in `0..n-1` with own count `boneIndexFirst[b+1] - boneIndexFirst[b] > 0`:
     * if `removed(b)` (phase 3: inside a severed subtree) → skip.
     * `mask = vec4(woundBits, cutBits, splatBits, 0)` from `mesh.boneOwnBounds[b]`:
       * wound bit `i` when the wound's bind AABB, grown by `(1 + roughness + rimWidth)` in radius, overlaps the AABB.
         Wound bind AABB: `extent_k = sqrt(sum_j (R[j][k] * radii[j])^2)` where `R = mat3_cast(rotation)` (column
         `j` is axis `j`), centre = `center`.
       * splat bit `i` when the splat sphere overlaps the AABB (closest-point test).
       * cut bit `i` (phase 3): AABB overlaps the influence sphere **and** the largest signed plane distance of the
         8 corners is `> -0.01`.
       * Bits are stored as float sums: bit `i` adds `exp2(i)` (`float(1u << i)`).
     * **front** list: append `{boneIndexFirst[b], count, mask}`; if the previous range ends exactly at
       `boneIndexFirst[b]` and has the same mask, extend it instead.
     * **interior** list (only `Volume` and `Shell` roles): same, but only bones whose mask has a wound or cut bit.
   * Shadow ranges (phase 3, body with cuts only): from `mesh.shadowVolumePrecomp.capBoneFirst/edgeBoneFirst`,
     skipping removed bones, no masks, coalesced. Phase 2: empty.
5. **`CollectDrawCommands`**: `SkeletalMesh::CollectDrawCommands(out);` then for each `ic` in
   `finalInteriorCommands`: `if (ic->goreActive && !ic->Hidden) out.push_back(ic.get());`. Copy `Hidden` from the
   front command in step 3.
6. **`GetRenderPalette`**: `return usingRenderPalette ? &renderPalette : SkeletalMesh::GetRenderPalette();` where
   `usingRenderPalette` is set from `pendingUsePalette` in `FinalizeFrameData` **before** re-pointing.

Plug it in: `NpcBase::NpcBase()` (`Entities/Npc/NpcBase.cpp`): `mesh = new Gore::GoreSkeletalMesh(this);` and include
`Gore/GoreSkeletalMesh.h`. Nothing else changes: without `SetGoreState` it is a plain `SkeletalMesh`.

Check: StealthAction runs, enemies look identical, frame time unchanged.

### P2.5 Shaders

All in `GameData/shaders/source/gore/`. Includes use `#include <../bgfx_shader.sh>` (like `shadowVolume/`).

**Step 0 — share the lighting.** Create `GameData/shaders/source/lighting_common.sh` by **moving** everything in
`fs_default.sc` from the line `uniform vec4 cameraPosition;` down to the end of `ApplyFog(...)` (inclusive) into it.
In `fs_default.sc` put `#include "lighting_common.sh"` right after the two `SAMPLER2D` lines. Nothing else changes.
Compile; `fs_default` must look identical (screenshot).

**Step 1 — `gore/varying.def.sc`**: copy `../varying.def.sc` exactly and add one line after `v_texcoord4`:
```
vec4 v_goreBind      : TEXCOORD7 = vec4(0.0, 0.0, 0.0, 0.0);
```
(TEXCOORD7 is unused in the root file. Check again before adding.)

**Step 2 — `gore/vs_gore.sc`**: copy `../vs_default.sc`; change the include to `<../bgfx_shader.sh>`; add
`v_goreBind` to `$output`; at the end of `main()` add `v_goreBind = vec4(a_position, 0.0);`. Nothing else.

**Step 3 — `gore/gore_common.sh`** (complete; implement exactly):
```glsl
#ifndef GORE_COMMON_SH
#define GORE_COMMON_SH

#define GORE_MAX_WOUNDS 8
#define GORE_MAX_CUTS 4
#define GORE_MAX_SPLATS 16
#define GORE_MAX_INTERVALS 16
#define GORE_FAR 1.0e9

uniform vec4 u_goreCounts;
uniform vec4 u_goreParams;
uniform vec4 u_goreRangeMask;
uniform vec4 u_goreBloodColor;
uniform vec4 u_goreCameraPos;
uniform mat4 u_goreViewProj;
uniform vec4 u_goreWoundBind[24];
uniform vec4 u_goreWoundWorld[24];
uniform vec4 u_goreWoundParams[8];
uniform vec4 u_goreWoundExtra[8];
uniform vec4 u_goreCutBind[4];
uniform vec4 u_goreCutBindSphere[4];
uniform vec4 u_goreCutRegrowBind[4];
uniform vec4 u_goreCutParams[4];
uniform vec4 u_goreCutWorld[4];
uniform vec4 u_goreCutWorldSphere[4];
uniform vec4 u_goreCutRegrowWorld[4];
uniform vec4 u_goreCutCenter[4];
uniform vec4 u_goreCutFrame[8];
uniform vec4 u_goreSplats[16];

SAMPLER2D(u_goreFlesh,       2);
SAMPLER2D(u_goreFleshNormal, 3);
SAMPLER2D(u_goreEdgeNoise,   4);
SAMPLER2D(u_goreBlood,       5);

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

// Front surface test in bind space. removed = discard. rim/soak/fresh in 0..1.
bool goreSurface(vec3 p, out float rim, out float soak, out float fresh)
{
    rim = 0.0; soak = 0.0; fresh = 0.0;
    bool isGib = u_goreParams.y > 0.5;
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
        vec3 q = goreWoundLocal(u_goreWoundBind[i*3], u_goreWoundBind[i*3+1], u_goreWoundBind[i*3+2], p) / max(wp.z, 0.001);
        float r = length(q);
        float edge = goreNoise(q.xy * 0.5 + 0.5 + wp.w) * wp.x;
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

// ---- interior ------------------------------------------------------------------------------------------

// Ray (world, dir normalised) vs ellipsoid given by world->unit rows. Returns false on miss.
bool goreRayEllipsoid(int i, vec3 ro, vec3 rd, out vec2 t)
{
    float heal = max(u_goreWoundParams[i].z, 0.001);
    vec4 r0 = u_goreWoundWorld[i*3]; vec4 r1 = u_goreWoundWorld[i*3+1]; vec4 r2 = u_goreWoundWorld[i*3+2];
    vec3 o = vec3(dot(r0, vec4(ro,1.0)), dot(r1, vec4(ro,1.0)), dot(r2, vec4(ro,1.0))) / heal;
    vec3 d = vec3(dot(r0.xyz, rd), dot(r1.xyz, rd), dot(r2.xyz, rd)) / heal;
    float a = dot(d, d);
    float b = dot(o, d);
    float c = dot(o, o) - 1.0;
    float disc = b*b - a*c;
    if (disc <= 0.0) { t = vec2(0.0, 0.0); return false; }
    float s = sqrt(disc);
    t = vec2((-b - s) / a, (-b + s) / a);
    return true;
}

bool goreRaySphere(vec3 ro, vec3 rd, vec4 sph, out vec2 t)
{
    vec3 oc = ro - sph.xyz;
    float b = dot(oc, rd);
    float c = dot(oc, oc) - sph.w * sph.w;
    float disc = b*b - c;
    if (sph.w <= 0.0 || disc <= 0.0) { t = vec2(0.0, 0.0); return false; }
    float s = sqrt(disc);
    t = vec2(-b - s, -b + s);
    return true;
}

// kind: 0 cavity (wound), 1 cap (cut plane), 2 regrow front. prim = wound or cut index.
// Returns false when the fragment must be discarded.
bool goreInterior(vec3 worldB, vec3 bindB, out vec3 P, out float kind, out float prim, out float thickness)
{
    P = worldB; kind = 0.0; prim = 0.0; thickness = 0.0;

    float rimD; float soakD; float freshD;
    if (goreSurface(bindB, rimD, soakD, freshD)) return false;      // the back wall itself is removed: through-hole

    vec3 ro = u_goreCameraPos.xyz;
    vec3 rd = normalize(worldB - ro);
    float tB = length(worldB - ro);

    vec2  iv[GORE_MAX_INTERVALS];
    float ivKind[GORE_MAX_INTERVALS];
    float ivPrim[GORE_MAX_INTERVALS];
    float ivReach[GORE_MAX_INTERVALS];
    int count = 0;

    int nw = int(u_goreCounts.x + 0.5);
    int nc = int(u_goreCounts.y + 0.5);

    for (int i = 0; i < GORE_MAX_WOUNDS; i++)
    {
        if (i >= nw) break;
        if (!goreBit(u_goreRangeMask.x, i)) continue;
        vec2 t;
        if (goreRayEllipsoid(i, ro, rd, t) && count < GORE_MAX_INTERVALS)
        {
            iv[count] = t; ivKind[count] = 0.0; ivPrim[count] = float(i); ivReach[count] = u_goreWoundExtra[i].x; count++;
        }
    }
    for (int i = 0; i < GORE_MAX_CUTS; i++)
    {
        if (i >= nc) break;
        if (!goreBit(u_goreRangeMask.y, i)) continue;
        vec2 ts;
        if (!goreRaySphere(ro, rd, u_goreCutWorldSphere[i], ts)) continue;
        vec4 pl = u_goreCutWorld[i];
        if (u_goreParams.y > 0.5) pl = -pl;                    // gib keeps the body's removed side
        float dn = dot(pl.xyz, rd);
        float s0 = dot(pl.xyz, ro) - pl.w;
        vec2 h;
        if (abs(dn) < 1.0e-5) { if (s0 <= 0.0) continue; h = vec2(-GORE_FAR, GORE_FAR); }
        else { float tp = -s0 / dn; h = dn > 0.0 ? vec2(tp, GORE_FAR) : vec2(-GORE_FAR, tp); }
        float a = max(h.x, ts.x);
        float b = min(h.y, ts.y);
        if (a >= b) continue;
        float endKind = 1.0;                                   // ends on the plane (or the influence sphere)
        vec2 g;
        if (u_goreCutRegrowWorld[i].w > 0.0 && goreRaySphere(ro, rd, u_goreCutRegrowWorld[i], g))
        {
            // removed = [a,b] minus [g.x,g.y]
            if (g.x > a && count < GORE_MAX_INTERVALS)
            { iv[count] = vec2(a, min(b, g.x)); ivKind[count] = 2.0; ivPrim[count] = float(i); ivReach[count] = u_goreCutFrame[i*2+1].w; count++; }
            if (g.y < b && count < GORE_MAX_INTERVALS)
            { iv[count] = vec2(max(a, g.y), b); ivKind[count] = endKind; ivPrim[count] = float(i); ivReach[count] = u_goreCutFrame[i*2+1].w; count++; }
        }
        else if (count < GORE_MAX_INTERVALS)
        {
            iv[count] = vec2(a, b); ivKind[count] = endKind; ivPrim[count] = float(i); ivReach[count] = u_goreCutFrame[i*2+1].w; count++;
        }
    }

    // The back face point inside a removed interval = removed back wall (world test agrees with the bind test above;
    // this catches the rigid-approximation cases).
    for (int k = 0; k < GORE_MAX_INTERVALS; k++)
    {
        if (k >= count) break;
        if (tB > iv[k].x && tB < iv[k].y) return false;
    }

    float best = -1.0; int bestK = -1;
    for (int k = 0; k < GORE_MAX_INTERVALS; k++)
    {
        if (k >= count) break;
        float e = iv[k].y;
        if (e <= 0.0 || e >= tB) continue;
        if (tB - e > ivReach[k]) continue;
        bool inside = false;
        for (int j = 0; j < GORE_MAX_INTERVALS; j++)
        {
            if (j >= count) break;
            if (j != k && e > iv[j].x && e < iv[j].y) inside = true;
        }
        if (inside) continue;
        if (e > best) { best = e; bestK = k; }
    }
    if (bestK < 0) return false;

    P = ro + rd * best;
    for (int k = 0; k < GORE_MAX_INTERVALS; k++)
    {
        if (k == bestK) { kind = ivKind[k]; prim = ivPrim[k]; }
    }
    thickness = tB - best;
    return true;
}

float goreDepth(vec3 P)
{
    vec4 clip = mul(u_goreViewProj, vec4(P, 1.0));
    float z = clip.z / clip.w;
    return u_goreParams.x > 0.5 ? z * 0.5 + 0.5 : z;
}

#endif
```
Notes for the implementer:
* The cut reach (`u_goreCutFrame[2i+1].w`) is the body thickness at the cut (see `cutFrame` in `GpuData`).
* Gibs (`u_goreParams.y = 1`) have only one cut and never regrow, so `goreInterior` flips the plane and the regrow
  sphere is `-1`. The cap normal in `fs_gore_interior` is flipped the same way.
* Loops use constant bounds plus `break` (required by some GLSL ES compilers). Keep that pattern.
* Known limitation (documented, do not "fix"): two separate removed regions on one ray inside the same body part pick
  the farther one. It is rare and dark.

**Step 4 — `gore/fs_gore.sc`** (front, forward):
* `$input v_texcoord0, v_color0, v_normal, v_world, v_texcoord1, v_tangent, v_bitangent, v_goreBind`
* includes: `<../bgfx_shader.sh>`, `SAMPLER2D(u_texture, 0); SAMPLER2D(u_textureEmissive, 1);`,
  `#include "../lighting_common.sh"`, `#include "gore_common.sh"`.
* `main()` = `fs_default`'s `main()` with these changes:
  * at the very start:
    ```glsl
    float rim = 0.0; float soak = 0.0; float fresh = 0.0;
    bool anyBits = (u_goreRangeMask.x + u_goreRangeMask.y + u_goreRangeMask.z) > 0.0;
    if (anyBits && goreSurface(v_goreBind.xyz, rim, soak, fresh)) discard;
    ```
  * after `color` is known (before lighting):
    ```glsl
    float blood = u_goreParams.w > 1.5 ? 0.0 : max(rim, soak);
    color = mix(color, u_goreBloodColor.rgb * (0.6 + 0.4 * soak), blood);
    color = mix(color, u_goreBloodColor.rgb * 1.4, fresh * 0.8);     // raw new skin while regrowing
    ```
  * specular: multiply `specularFactor` by `(1.0 + 2.5 * max(blood, fresh))` (wet).
* Keep the `masked` alpha test exactly as in `fs_default`.

**Step 5 — `gore/fs_gore_depth.sc`**: `$input v_texcoord0, v_goreBind`; uniforms `masked`; `SAMPLER2D(u_texture, 0)`;
`#include "gore_common.sh"`. `main`: alpha test like `fs_mask.sc` when `masked.x > 0`, then the same `anyBits` /
`goreSurface` discard. No output color (copy what `fs_empty.sc` / `fs_mask.sc` write, e.g. `gl_FragColor = vec4(0)` if
they do).

**Step 6 — `gore/fs_gore_customId.sc`**: copy `../fs_customId.sc`, add `v_goreBind` to `$input`,
`#include "gore_common.sh"`, and the same discard at the start of `main`.

**Step 7 — `gore/fs_gore_interior.sc`**:
```glsl
$input v_texcoord0, v_color0, v_normal, v_world, v_texcoord1, v_tangent, v_bitangent, v_goreBind
#include <../bgfx_shader.sh>
SAMPLER2D(u_texture, 0);
SAMPLER2D(u_textureEmissive, 1);
#include "../lighting_common.sh"
#include "gore_common.sh"

uniform vec4 u_goreShell;   // x > 0.5: shell submesh (cloth back side only)

vec3 goreTriplanar(vec3 p, vec3 n, float scale)
{
    vec3 w = abs(n); w = w / (w.x + w.y + w.z + 0.0001);
    return texture2D(u_goreFlesh, p.yz * scale).rgb * w.x
         + texture2D(u_goreFlesh, p.xz * scale).rgb * w.y
         + texture2D(u_goreFlesh, p.xy * scale).rgb * w.z;
}

void main()
{
    vec3 P; float kind; float prim; float thickness;
    if (!goreInterior(v_world.xyz, v_goreBind.xyz, P, kind, prim, thickness)) discard;

    vec3 viewDir = normalize(u_goreCameraPos.xyz - P);
    vec3 N; vec3 albedo; float wet = 1.0;
    int i = int(prim + 0.5);

    if (u_goreShell.x > 0.5)
    {
        N = viewDir; albedo = texture2D(u_texture, v_texcoord0).rgb * 0.25;   // dark cloth back side
        wet = 0.0;
    }
    else if (kind < 0.5)                       // cavity wall of wound i
    {
        vec4 r0 = u_goreWoundWorld[i*3]; vec4 r1 = u_goreWoundWorld[i*3+1]; vec4 r2 = u_goreWoundWorld[i*3+2];
        vec3 q = goreWoundLocal(r0, r1, r2, P) / max(u_goreWoundParams[i].z, 0.001);
        N = -normalize(r0.xyz * q.x + r1.xyz * q.y + r2.xyz * q.z);   // inner wall faces the cavity centre
        albedo = goreTriplanar(q * u_goreWoundExtra[i].y, q, u_goreParams.z);
        albedo *= mix(0.35, 1.0, clamp(length(q), 0.0, 1.0));           // darker deeper inside
    }
    else if (kind < 1.5)                       // cap of cut i
    {
        N = u_goreParams.y > 0.5 ? -u_goreCutWorld[i].xyz : u_goreCutWorld[i].xyz;
        vec3 c = u_goreCutCenter[i].xyz;
        vec3 T = u_goreCutFrame[i*2].xyz; vec3 B = u_goreCutFrame[i*2+1].xyz;
        vec2 uv = vec2(dot(P - c, T), dot(P - c, B));
        albedo = texture2D(u_goreFlesh, uv * u_goreParams.z).rgb;
        float fat = 1.0 - smoothstep(0.004, 0.015, thickness);          // ring near the outline
        albedo = mix(albedo, vec3(0.85, 0.72, 0.45), fat * 0.8);
        float boneR = u_goreCutFrame[i*2].w;
        float dc = length(uv);
        if (dc < boneR)        { albedo = dc < boneR * 0.55 ? vec3(0.35, 0.05, 0.04) : vec3(0.92, 0.88, 0.78); wet = 0.4; }
        if (mod(u_goreCutParams[i].w, 2.0) >= 1.0) albedo *= vec3(0.25, 0.18, 0.15);   // charred
    }
    else                                       // regrow front of cut i
    {
        N = normalize(P - u_goreCutRegrowWorld[i].xyz);
        albedo = goreTriplanar(P - u_goreCutCenter[i].xyz, N, u_goreParams.z) * vec3(1.2, 0.8, 0.8);
    }

    if (u_goreParams.w > 0.5) { float l = dot(albedo, vec3(0.3, 0.59, 0.11)); albedo = vec3(l, l, l) * 0.4; }  // reduced/minimal

    vec3 lightDir = normalize(direct_light_dir.xyz);
    vec3 diffuse = CalculateDirectionalDiffuse(N, lightDir);
    vec3 spec = CalculateDirectionalSpecular(N, lightDir, viewDir) * (0.5 + 2.0 * wet);
    vec3 pd = vec3(0.0, 0.0, 0.0); vec3 ps = vec3(0.0, 0.0, 0.0);
    CalculatePointLights(N, P, viewDir, pd, ps);
    vec3 finalColor = albedo * (diffuse + pd) + (spec + ps) * wet;

    gl_FragColor = ApplyFog(vec4(finalColor, 1.0), P);
    gl_FragDepth = goreDepth(P) - 0.000001;    // tiny bias toward the camera (matches the depth pre-pass value)
}
```
`ComputeStyledSpecular` reads `specular_pow`/`specular_scale` defaults from `lighting_common.sh` — fine.
Set `u_goreShell` from `GoreDrawCommand::ApplyAdditionalShaderParams` (`vec4(shell ? 1 : 0)`).

**Step 8 — `gore/fs_gore_interior_depth.sc`**: `$input v_world, v_goreBind`, include `gore_common.sh`; `main`:
`vec3 P; float k; float pr; float th; if (!goreInterior(v_world.xyz, v_goreBind.xyz, P, k, pr, th)) discard; gl_FragDepth = goreDepth(P);`

Compile with `ShaderCompilationAutomation.exe`; fix every error on every platform.

### P2.6 `GoreSystem` — hits to wounds

Files: `Gore/GoreSystem.h/.cpp`.

1. **Event handler** (`GoreSystem.cpp`):
   ```cpp
   EVENT_HANDLER(GoreHitHandler, GoreHitEvent)
   {
       Entity* victim = e.victim.Resolve();
       if (!victim) return;
       auto* mesh = Gore::FindGoreMesh(victim);           // see 3.
       if (!mesh || !mesh->GetSkinnedModel()) return;
       if (!victim->HasComponent<Gore::GoreComponent>()) victim->AddComponent<Gore::GoreComponent>();
       auto gore = victim->GetComponent<Gore::GoreComponent>();
       if (gore->pendingCount < Gore::kMaxPendingHits) gore->pending[gore->pendingCount++] = e.hit;
       else Gore::MergeIntoLastPending(*gore, e.hit);        // add damage to the last pellet hit; keeps the count bounded
   }
   ```
2. **Tick** (runs after the hits of the frame, game thread):
   ```cpp
   TICK_ENTITY(GoreProcessHitsSystem, PostLateUpdate, Gore::GoreComponent, self, gore)
   {
       if (gore.pendingCount > 0) Gore::ProcessPendingHits(self, gore);
       Gore::PushRenderStateIfChanged(self, gore);   // SetGoreState when gore.version changed
   }
   REGISTER_TICK(GoreProcessHitsSystem)
   ```
   Check in `EcsScheduler`/`TickGroup.h` that `PostLateUpdate` runs after the game logic that calls `OnPointDamage`
   (bullets are entities updated in `Update`; events emitted in `Update` are flushed at the end of the next tick group
   run). If events emitted during `Update` only reach the handler after `PostLateUpdate`, the hits are processed one
   frame later; that is acceptable — do not reorder engine tick groups.
3. `FindGoreMesh(Entity*)`: iterate `entity->Drawables` and `dynamic_cast<Gore::GoreSkeletalMesh*>`; cache nothing
   (called once per hit batch). For gibs, `GoreGib` returns its mesh directly (phase 3).
4. **`ProcessPendingHits(Entity*, GoreComponent&)`**:
   * Sort `pending[0..pendingCount)` by `counter` (stable order for determinism).
   * Get `mesh`, `model`, `profile = GetProfile(mesh->filePath, *model)`. If `!profile.valid` → `pendingCount = 0; return`.
   * `World = mesh->GetWorldMatrix()`, `palette = mesh->GetSkinPalette()`. If `palette.empty()` → keep pending for
     next frame (model still loading), return.
   * Group pellet hits (`source.type == Pellet`) and process the rest one by one.
   * For each hit: `anchorSkin = mesh->GetSkinIndex(hitBone)`; if `< 0` (no bone name, e.g. explosions or cane
     projectile with `""`), pick the skin bone whose world-space hitbox body is closest to the point: iterate
     `mesh->hitboxBodies` (positions via `FromPhysics(body->GetPosition())`) and take its `hitboxName`. If there are no
     hitboxes, skip the hit.
   * `toBind = inverse(World * palette[anchorSkin])`; `pBind = vec3(toBind * vec4(point,1))`;
     `dBind = normalize(vec3(toBind * vec4(direction,0)))`.
   * `Rng rng(hit.seed)`.
   * Wound by source type:
     | type | wound(s) |
     |---|---|
     | Bullet | Entry: `r = woundRadius * sqrt(max(damage,1)/refDamage) * rng.Range(0.8,1.2)`; radii `(r, r, r*1.6)`, rotation = `MathHelper`/glm quat rotating `+z` to `dBind` (`glm::rotation(vec3(0,0,1), dBind)` from `<glm/gtx/quaternion.hpp>`), center `pBind + dBind * 0.3r`, roughness 0.3, rimWidth 0.5. Exit (if `source.exit`): see exit below, radii ×1.6, roughness 0.45. |
     | HeavyBullet | like Bullet, entry r ×1.3, exit always, exit r ×2.2 |
     | Pellet | cluster first (below), one Crater per cluster: `r = woundRadius * (1.5 + 0.6*sqrt(pelletsInCluster))`, radii `(r, r, r*0.8)` |
     | Slash | Gash: blade sweep plane normal `nB` (bind, from `sweepNormal`; if zero use `cross(dBind, up)`), radii `(gashLength*0.5, woundRadius*0.35, woundRadius)` oriented: local x along `cross(nB, dBind)`, local y = `nB`, local z = `dBind` (build the quat from that 3×3 with `glm::quat_cast(mat3(x,y,z))`) |
     | Blunt | no wound, splat only (radius `0.08 * damage/refDamage`) |
     | Blast | Crater, `r = woundRadius * clamp(damage/refDamage, 0.5, 2)`, radii `(r, r, r*0.7)` |
   * `reach = 2.0 * maxExtent(profile.anyBounds[anchorSkel])` (bind metres).
   * Exit point: take the hit hitbox body (`mesh->FindHitboxByName(hitBone)`), its world position/rotation and the
     `HitboxData` size from `mesh->metaData.hitboxes` (same bone). Do a ray–OBB slab test with origin = `point`, dir =
     `direction`; the far intersection `tFar` is the exit (if `tFar > 0.01`). Check whether `HitboxData::size` is a full
     or half extent by reading `Physics::CreateHitBoxBody` and use it correctly.
   * Splat with every wound: center `pBind`, radius `2.5 * r`, strength `1`.
   * **Add with budget** (`AddWound`): if `woundCount < kMaxWounds` append; else find the existing wound whose centre is
     closest; if its distance `< (its mean radius + new mean radius)`, **merge** (new centre = weighted mean by
     volume, radii = max, keep the older rotation) else replace the **oldest** (`order` smallest). Same for splats
     (`kMaxSplats`, merge when distance `< sum of radii`). Always `wound.order = gore.orderCounter++`.
   * Pellet clustering: in sorted order, put each pellet into the first cluster whose bind centre is closer than
     `source.clusterRadius`, else start a new cluster; centre = running mean. Each cluster → one crater, direction =
     mean direction.
   * After all: `gore.pendingCount = 0; gore.version++`.
5. **`PushRenderStateIfChanged`**: compare `gore.version` with `mesh->GetGoreVersion()` (add that getter to
   `GoreSkeletalMesh`, returning `goreState.version`). Only when they differ: build a `RenderState` on the stack, copy
   the arrays and counts from the component, set `state.version = gore.version`, `state.profile = &profile` and the
   regen clock values, and call `mesh->SetGoreState(state)`. `SetGoreState` copies the arrays (no heap except
   `gibSeverPalette` for gibs, which is set once at gib creation).

### P2.7 Adapter — damage → `GoreHitEvent`

File: `Gore/StealthGoreAdapter.h/.cpp` (namespace `StealthGore`). Functions called from StealthAction code:

```cpp
namespace StealthGore
{
    void LoadAssets();   // sources, settings (once per level)
    // Called from NpcBase::OnPointDamage AFTER the damage was applied (so `lethal` is known).
    void OnNpcPointDamage(NpcBase* npc, float damage, vec3 point, vec3 direction, const std::string& bone,
                          Entity* causer, Entity* weapon, bool wasAliveBefore);
    // Projectiles copy their weapon's source when fired.
    void TagProjectile(Entity* projectile, Entity* weapon);
}
```
* `NpcBase::OnPointDamage` (`Entities/Npc/NpcBase.cpp:248`): at the top store `bool wasAlive = !dead;`. Right after
  `Entity::OnPointDamage(Damage, ...)` add
  `StealthGore::OnNpcPointDamage(this, Damage, Point, Direction, bone, DamageCauser, Weapon, wasAlive);`.
* `OnNpcPointDamage`:
  * source: `weapon && weapon->HasComponent<Gore::GoreSourceComponent>()` → its index; else
    `Gore::SourceIndexForClass(weapon ? weapon->ClassName : (causer ? causer->ClassName : ""))`.
    Check the field name `ClassName` on `Entity` with grep.
  * `hit.lethal = wasAlive && npc->dead` (check that `dead` is accessible; add a public getter `IsDead()` to NpcBase
    if it is private).
  * `hit.counter = ++npcCounter` where the counter lives in the victim's `GoreComponent` — but the component may not
    exist yet. Keep a small adapter component `Gore::HitCounterComponent { uint32_t counter = 0; }` added on first
    hit (REGISTER_COMPONENT), so the counter is per victim and does not depend on the event order.
  * `hit.seed = Gore::HashSeed(npc->Id, hit.counter)`.
  * `strncpy` the bone name into `hit.hitBone` (truncate, always 0-terminate).
  * `EcsScheduler::Emit(GoreHitEvent{ EntityHandle::FromEntity(npc), hit });`
* `WeaponFirearm.cpp:552` (after `Level::Current->AddEntity(bullet);`): `StealthGore::TagProjectile(bullet, this);`
  (check `this` is the weapon entity there). `TagProjectile` adds `GoreSourceComponent{ SourceIndexForClass(weapon->ClassName) }`.
  Same for the other `AddEntity(bullet)` call sites (cane, crucifix) and cannon/explosion: the explosion entity's own
  class maps to `explosion`.
* Melee: `weapon_sword.cpp:226`, `weapon_twinsword.cpp:394`: the `Direction` passed is the camera forward. Add the
  blade sweep: compute the sweep plane normal from the trace start/end points the weapon already uses (read the
  function around those lines), and pass it through a new optional parameter? **No** — `OnPointDamage` is engine
  API. Instead, before calling `OnPointDamage`, call `StealthGore::SetNextMeleeSweep(victim, sweepNormal)`; the
  adapter stores it in the victim's `HitCounterComponent` and uses it (then clears it) for the next hit of that
  victim.

### P2.8 Debug tools (compiled only `#ifndef DISTRIBUTION`)

`Gore/GoreDebug.cpp`: an ImGui window "Gore" (find where StealthAction draws its debug ImGui windows: grep
`ImGui::Begin` in `Games/StealthAction`), with:
* "Wound under crosshair": trace from the camera (`Physics::LineTrace(Camera::finalizedPosition, end, BodyType::GroupHitTest)`),
  emit a `GoreHitEvent` for the hit NPC with a chosen source.
* Checkbox "draw volumes": for the targeted NPC, `DebugDraw` the wound ellipsoids (8 points on axes is enough) and
  cut planes in world space.
* Text: wound/cut/splat counts, pending hits, version.
* "Check closed mesh": for each `Volume` submesh of the targeted NPC's model, count edges used by exactly one triangle
  (after welding positions with 1e-5 tolerance) and print the count. Must be 0 for body submeshes.

### Phase 2 exit check
* Shoot an enemy with each weapon: holes at the hit point, blood rims, cavities visible inside, darker deeper.
* Look through a hole at a thin part (arm, neck) from an angle where the back is also hit by the exit: you see through.
* Holes stay glued during animation and ragdoll.
* Holes don't show in the custom-id pass and don't show a "filled" depth (fog/post effects behave).
* Undamaged crowd: frame time unchanged (Tracy or the in-game counter).
* 50 enemies each with 8 wounds: record GPU frame time; note it in §9.

---

## 3. Phase 3 — Severing

### P3.1 Cut solver — `Gore/GoreCutSolver.h/.cpp`

```cpp
namespace Gore
{
    struct CutRequest { uint16_t severableIndex; glm::vec3 worldPoint; glm::vec3 worldNormal; };   // normal: desired plane normal (any sign)
    struct CutResult  { bool ok = false; Cut cut; };
    CutResult SolveCut(const Profile& profile, const roj::SkinnedModel& model, const glm::mat4& world,
                       const std::vector<glm::mat4>& palette, const CutRequest& req, Rng& rng);
}
```
Algorithm (all in bind space; `S` = severable bone skel index):
1. `bindJ = vec3(inverse(skeleton.bones[S].invBind)[3])` (joint position in bind). Child joint `C`: among direct
   children `c` of `S` (bones with `parent[c] == S`) that are skin bones, take the one whose subtree any-bounds
   volume is largest; `bindC` = its joint. No child → `bindC` = the corner of `anyBounds[S]` farthest from `bindJ`.
   `axis a = normalize(bindC - bindJ)`, `L = length(bindC - bindJ)`. If `L < 0.01` → fail.
2. `M = world * palette[skin(S)]`; `p = vec3(inverse(M) * vec4(worldPoint,1))`;
   `n = normalize(transpose(mat3(M)) * worldNormal)` (world → bind for normals). If `dot(n,a) < 0` → `n = -n`.
3. Tilt clamp: `cosT = dot(n,a)`; if `cosT < cos(maxTilt)`: `perp = normalize(n - a*cosT)`;
   `n = normalize(a*cos(maxTilt) + perp*sin(maxTilt))`.
4. `s = clamp(dot(p - bindJ, a) / L, range.x, range.y)`; `o = bindJ + a*s*L`.
5. Validity test `Valid(n, o)`: for every skel bone `b` in `(S, subtreeEnd[S])` with `anyBounds[b].hasData`, all 8
   corners `c` satisfy `dot(n, c - o) > 0.002`. Also `anyBounds[S]` must have at least one corner on each side.
6. If not valid: for `k = 1..8`: `t = k/8`; `nk = normalize(mix(n, a, t))`; `ok_ = bindJ + a*mix(s, range.y, t)*L`;
   first valid → use. If none: try `(a, bindJ + a*range.y*L)`; still invalid → `ok = false` (the caller makes a
   wound instead).
7. Fill `Cut`: `plane = vec4(n, dot(n,o))`, `center = o`, `boneRadius = profile radius`, `roughness = 0.012`,
   `seed = rng.Next01()`, `boneSkel = S`, `boneSkin = skin(S)`,
   `influenceRadius = 1.05 * max distance from o to the corners of all any-bounds of bones in [S, subtreeEnd[S]) + 0.02`,
   `regrowMaxRadius = influenceRadius`.

Unit-test it in the debug window: "Solve cut at crosshair" draws the plane and the subtree bounds in `DebugDraw`,
green when valid.

### P3.2 Sever decision (in `ProcessPendingHits`)

* Map the hit to a severable bone: candidates = severable bones that are the hitbox bone or its descendants. For each
  candidate `c` whose `anyBounds[c]` contains `pBind`, pick the one with the smallest box volume. If none, use the
  hitbox bone if severable. If none → no sever.
* Accumulate: `severDamage[idx] = severDamage[idx] * exp(-(now - severDamageTime[idx]) / 3.0) + damage * source.severPower`
  (decay 3 s), `severDamageTime = now` (GameTime is fine here; decay doesn't need determinism across save).
* Sever when `severDamage[idx] >= threshold` where `threshold = 60 * bodyRules.severThresholdScale` (phase 4 rules;
  constant 60 in phase 3), or when `hit.lethal && source.type in {Slash, HeavyBullet, Pellet, Blast}` and
  `source.severPower > 0.5` for the head/neck. Pellet batches use the summed damage of the cluster.
* Desired plane: Slash → normal = `sweepNormal`; others → normal = `direction` (the plane faces the shot).
* Already removed (bone inside a severed subtree of an existing cut): no sever, wound on the stump instead.
* Budget: `cutCount == kMaxCuts` → no sever.
* On success: append the cut, `version++`, turn hitboxes off for the whole subtree (`SetHitboxesActive` with the names
  of all bones in `[S, subtreeEnd[S])` that have hitboxes — collect from `mesh->metaData.hitboxes`), queue the gib
  (`GoreGibPool::Request(...)`), emit `GorePartEvent{victim, S, lost=true}` (phase 4 consumes it).
* Also add a crater wound at the cut centre? **No.** The cap already shows the flesh.

### P3.3 Body rendering with cuts (`GoreSkeletalMesh`)

* `removed(b)` for ranges: `b` is in `(S, subtreeEnd[S])` of a cut in phase `Severed` (descendants only; `S` itself
  stays drawn and is clipped by the plane).
* Cut bits in masks (rule in P2.4 step 4).
* Shadow ranges: as in P2.4 step 4 with `capBoneFirst/edgeBoneFirst` (skip removed bones). Only when
  `CastDetailShadows`.
* Render palette (body): `pendingPalette = GetSkinPalette()` copy into the reused vector (`assign(begin,end)`); for each
  `Severed` cut and each skel bone `b` in `(S, subtreeEnd[S])` with `skinIdx >= 0`:
  `pendingPalette[skin(b)] = pendingPalette[skin(S)]`. `pendingUsePalette = true` when at least one cut is `Severed`.
* `GetBoundingBox`: when there are severed cuts and not `InRagdoll`: build `uint8_t mask[MAX_SKINNED_BONES]` (member
  array, not per call allocation) = 1 for every skin bone not removed; `model->ComputeAnimatedBounds(GetWorldMatrix(), GetSkinPalette(), mask)` (E7). Else `SkeletalMesh::GetBoundingBox()`.
  **Careful:** `GetBoundingBox` runs in `PreFinalize` (parallel). It may only read this mesh's own state.

### P3.4 Shadow shaders

`gore/gore_shadow.sh`:
```glsl
uniform vec4 u_goreCounts;
uniform vec4 u_goreCutBind[4];
uniform vec4 u_goreCutBindSphere[4];
uniform vec4 u_goreCutRegrowBind[4];
vec3 goreFlatten(vec3 p)
{
    int nc = int(u_goreCounts.y + 0.5);
    for (int i = 0; i < 4; i++)
    {
        if (i >= nc) break;
        vec4 pl = u_goreCutBind[i];
        vec4 sph = u_goreCutBindSphere[i];
        float s = dot(pl.xyz, p) - pl.w;
        if (s <= 0.0 || length(p - sph.xyz) > sph.w) continue;
        vec4 rg = u_goreCutRegrowBind[i];
        if (rg.w > 0.0 && length(p - rg.xyz) < rg.w) continue;   // regrown part casts its shadow
        p -= pl.xyz * s;
    }
    return p;
}
```
`gore/vs_shadowvolcap_gore.sc` and `gore/vs_shadowvoledge_gore.sc`: copy the originals from `../shadowVolume/`,
include `gore_shadow.sh`, and pass **every** bind position through `goreFlatten` before it is skinned (cap: the
three triangle positions and `a_position`; edge: `a_position`, p0, p1, adj0, adj1). Read each original carefully to
find all positions (`a_position`, `a_normal`, `a_tangent`, `a_bitangent`, and `vec3(a_texcoord0.xy, a_texcoord1.x)`
for adj1 in the edge shader). They use `#include <../bgfx_shader.sh>` and the root varying file; the gore folder has
its own `varying.def.sc`, which is a superset, so that works.

### P3.5 Gibs — `Gore/GoreGib.h/.cpp`

```cpp
namespace Gore
{
    class GoreGib : public Entity
    {
    public:
        GoreGib();                     // SaveGame = false; mesh = new GoreSkeletalMesh(this); Drawables.push_back(mesh)
        void Init(Entity* body, GoreSkeletalMesh* bodyMesh, const Cut& cut, const Profile& profile,
                  const std::vector<Wound>& woundsInSubtree, glm::vec3 impulse);
        void Update() override;        // check the Entity update signature with grep
        void OnPointDamage(float Damage, vec3 Point, vec3 Direction, string bone, Entity* causer, Entity* weapon) override;
        GoreSkeletalMesh* mesh = nullptr;
        std::string ownerId; uint8_t cutIndex = 0; double spawnTime = 0; bool sinking = false; double sinkStart = 0;
    };

    struct GibRequest { std::string bodyId; uint8_t cutIndex; glm::vec3 impulse; };
    namespace GibPool
    {
        void Request(const GibRequest& r);   // queued; GoreGibSpawnSystem spawns <= maxGibSpawnsPerFrame per frame
        void Tick();                          // spawn queue, sink and destroy the oldest beyond maxGibs
        void OnLevelUnload();
    }
}
```
`Init` steps (exact order):
1. `Position/Rotation` = body's. `mesh->LoadFromFile(bodyMesh->filePath)` (cached model). Copy `TexturesLocation` and
   other material settings the NPC sets in `LoadAssets` (`DepthPrePath = false`, `Masked = true`).
2. `mesh->PasteAnimationPose(bodyMesh->GetAnimationPose()); mesh->SetAnimationPaused(true); mesh->UpdatePose = false;`
   (check these fields; the goal: the gib's skeleton starts in the body's current pose and never animates).
3. Hitboxes: `mesh->metaData = bodyMesh->metaData; mesh->metaData.hitboxes` = only those whose bone is in
   `[S, subtreeEnd[S])`; for `Rigid` gibs keep only the hitbox of `S` (or the first one in the subtree). The kept root
   hitbox gets `parentBone = ""`. If no hitbox is left, create one `HitboxData` for `S`: box from the merged
   any-bounds of the subtree transformed into the bone's local space (`invBind` of `S` applied to the 8 corners, then
   the AABB of those), `position` = its centre, `size` = its extent (full or half per `CreateHitBoxBody`).
4. `mesh->CreateHitboxes(this); mesh->StartRagdoll(); mesh->InRagdoll = true;` (check how `NpcBase::Death` does it and
   copy that order, including `RagdollPoseFollowStrength = 0`).
5. Velocities: for each gib body, set the velocity of the body's hitbox with the same bone name (`Physics::GetLinearVelocity`
   — grep for the getter; `HitboxBinding` stores kinematic velocities privately, so read from the body), then add
   `impulse` with `Physics::AddImpulse(body, impulse * mass)`.
6. Gore state: `RenderState` with `isGib = true`, one cut (index 0 = the body's cut, phase `Severed`), the wounds whose
   centres are inside the subtree's any-bounds, no splats. `gibSeverPalette = bodyMesh->GetSkinPalette()` (copy).
   `mesh->SetGoreState(state)`. Also add a `GoreComponent` to the gib with the same wounds/cut so it can be hit
   (`GoreGib::OnPointDamage` emits a `GoreHitEvent` for itself; `ProcessPendingHits` works on gibs too, with
   `isGib` kept and **no further severing** on gibs except head destroy in phase 4).
7. `mesh->CastDetailShadows = false;`

Gib rendering differences in `GoreSkeletalMesh` (when `goreState.isGib`):
* Ranges: draw only bones `b` in `[S, subtreeEnd[S])`, plus ancestors `a` of `S` whose own bounds overlap the cut's
  influence sphere **and** have corners on both sides of the plane. All these get the cut bit.
* `u_goreParams.y = 1` (the shader keeps the removed side; see `goreSurface`).
* Render palette: let `K = [S, subtreeEnd[S])`, `pal = GetSkinPalette()` (gib's live palette), `pal0 = gibSeverPalette`.
  For every skin bone `b` **not** in `K`: `pendingPalette[b] = pal[skin(S)] * inverse(pal0[skin(S)]) * pal0[b]`.
  Bones in `K`: `pal[b]`. `pendingUsePalette = true` always for gibs.
* Bounds: ragdoll bounds of `SkeletalMesh::GetBoundingBox()` (it uses hitbox bodies; fine).

Pool rules (`GibPool::Tick`, called from a `DECLARE_SYSTEM` in `PostUpdate`):
* Spawn at most `maxGibSpawnsPerFrame` requests per frame; a request older than 3 frames is dropped (spawn the FX
  burst only).
* `live` list ordered by spawn time. While `live.size() > maxGibs`: the oldest not-waiting-for-reattach gib starts
  sinking: `sinking = true`, its hitboxes go to `Physics::SetMotionType(..., Kinematic)`, it moves down 0.5 m over 1 s,
  then `Destroy()`.
* On level unload clear everything (find the level-unload hook used by other StealthAction singletons).

### P3.6 Stump FX — `Gore/GoreFx.cpp`

`DECLARE_SYSTEM(GoreFxSystem, PostLateUpdate)` iterating `GoreComponent` (+ gibs): for each `Severed` cut with
`now - severedAt < spurtSeconds`: pulse `p = max(0, sin(2π·pulseHz·t))^4`, spawn `GlobalParticleSystem::SpawnParticleAt("gore_spurt", worldCenter, MathHelper::FindLookAtRotation(vec3(0), worldNormal), vec3(p * strength))`
at most every 0.05 s per cut, respecting a global per-frame particle budget (`maxParticlesPerFrame`). After the spurt,
`gore_drip` every 0.3 s for 10 s. Particles must exist (art); until they do, use `hit_flesh` and log once.

### Phase 3 exit check
* Sword cuts follow the blade angle; shotgun at a forearm cuts it off; caps show flesh, fat ring, bone and marrow.
* The severed piece falls with physics, its cap matches the stump exactly (no gap, no overlap).
* No shadow of the missing limb on the ground; the gib casts none.
* Shooting the gib makes holes in it.
* 50 bodies with cuts + 48 gibs: frame time recorded in §9.

---

## 4. Phase 4 — Gameplay on living enemies

### P4.1 Body rules — `Gore/GoreBodyRules.h/.cpp`

Files: `GameData/gore/bodies/<archetype>.json` and `GameData/gore/bodies/index.json`
(`{ "classes": { "npc_human_axe": "human", ... }, "default": "human" }`). Parse everything in GORE_DESIGN.md §6.11
(durability, severThresholdScale, bleedPerSecond, parts with `consequence`/`regrowDelay`/`regrowSeconds`, regen).
Missing values take the defaults of `human.json`:
```json
{ "durability": { "mode": "mortal" }, "severThresholdScale": 1.0, "bleedPerSecond": 4,
  "parts": { "neck_01": { "consequence": "death" }, "spine_02": { "consequence": "death" },
             "lowerarm_r": { "consequence": "dropWeapon" }, "upperarm_r": { "consequence": "dropWeapon" },
             "calf_l": { "consequence": "crawl" }, "thigh_l": { "consequence": "crawl" },
             "calf_r": { "consequence": "crawl" }, "thigh_r": { "consequence": "crawl" } },
  "regen": null }
```
`regen: null` = no healing, no regrowth. Parts are keyed by **bone name**; resolve to skel indices per model when
used (rules are model independent). A cut of bone `X` uses the rule listed for exactly `X`; a bone that is not listed
has consequence `none` and the default regen timings. No ancestor lookup.

### P4.2 Consequences — adapter handles `GorePartEvent`

```cpp
EVENT_HANDLER(StealthGorePartHandler, GorePartEvent) { ... in StealthGoreAdapter.cpp ... }
```
* `death`: only when the durability mode allows a kill for the hit that caused it (phase 5; in phase 4 always) →
  `npc->Death()`.
* `dropWeapon`: find NpcBase's existing weapon drop logic (grep `Drop`, `weapon` in NpcBase); if none exists, hide
  `weaponMesh`, set a flag `goreNoWeapon` that the combat behaviour checks to switch to melee/unarmed (grep the combat
  behaviour for how weapon availability is decided). Do not invent new combat behaviour.
* `crawl`: until the crawl clips exist (P4.5/P4.6): `npc->StartStunnedRagdoll()` and keep it down while the leg is
  missing (a flag the stand-up check reads; find where the stunned ragdoll ends — `isStunned()`). With P4.6 the
  enemy falls with the ragdoll and goes straight on crawling.
* On `lost = false` (regrown, phase 5): undo — pick the weapon up again (unhide), allow standing up.

### P4.3 Bleed-out

In the adapter tick (`TICK_ENTITY(StealthGoreBleedSystem, PostUpdate, Gore::GoreComponent, self, gore)`): for a living
NpcBase with `bleedPerSecond > 0` and `n` open (`Severed`) cuts: `npc->OnDamage(bleedPerSecond * n * dt, nullptr, nullptr)`.
Use the engine's delta time (`Time::DeltaTimeF`).

### P4.4 Head destroy and blast chunks

* `canDestroy` bone + lethal heavy hit (HeavyBullet, Pellet cluster, Blast) on it → cut at `range.x` with the plane
  facing the bone axis, **no gib**, burst FX (`gore_burst`), and 3–6 chunk gibs: `GoreGib`-like entities with a
  `StaticMesh` from `GameData/models/gore/chunk_*.glb` (art) and one box body each — implement as
  `GoreChunk : Entity` in `GoreGib.cpp`, same pool.
* Blast: for every severable bone whose world joint is within `radius * 0.5` of the explosion and damage above the
  threshold, sever with the plane facing away from the blast centre (several cuts in one batch, budget permitting).
  Explosion hits come with `bone == ""` (Explosion.cpp:305 passes `hit.position`); `ProcessPendingHits` already picks
  the nearest hitbox.

### P4.5 Crawl animations in Blender (Blender MCP)

Made with the Blender MCP tools (`mcp__Blender__*`; Blender must be running with the MCP add-on connected). Follow the
MCP rules: inspect the scene before changing anything, never modify the enemy model or rig destructively, set the
active object and selection explicitly before each operator, use the data API (`bpy.data`, F-curves/keyframes) for
precise keys.

1. **Find the rig.** Look for the StealthAction enemy source file under `sourceAssets/` (grep/find `*.blend` with
   `human`, `npc`, `enemy`). If there is none, import `GameData/models/enemies/humanAxe/humanAxe.glb` into a new file.
   Also import `GameData/animations/npc/walk.glb` and `standUp.glb` to see how existing clips are set up.
   Write down, before keying anything:
   * the exact bone names and rest pose (they must stay identical: no renames, no applied transforms, no new bones);
   * which bone carries root motion in `walk.glb` and how (compare with `SkeletalMesh::PullRootMotion`), and the
     forward axis;
   * how `standUp.glb` is exported (armature only or with mesh, action naming, frame rate).
2. **Clips** (one action each, same frame rate as the existing clips):
   | Action | Loop | Content |
   |---|---|---|
   | `crawl_idle` | yes | face down, weight on the forearms, breathing, slight head lift |
   | `crawl_fwd` | yes | arms pull the body forward one after the other, hips drag, legs trail limp (≈ 1.2 s per cycle) |
   | `crawl_turn_l` / `crawl_turn_r` | yes | turn in place on the forearms, about 45° per cycle |
   | `crawl_attack` | no | lunge and swipe/grab from the floor (melee enemies) |
   | `crawl_from_ragdoll` | no | from lying face down straight into the first `crawl_fwd` pull (≈ 0.4 s, short: the enemy keeps moving right after the fall) |

   No death clip: death always goes to the ragdoll (`Death()`), crawling or not.
   The clips must still look right when either or both legs, or everything below `spine_02`, are missing (the legs
   are not drawn then, but still animate): keep the torso and arms doing all the work; legs only drag.
   Root motion: the same bone and convention as `walk.glb`. Measure the forward distance per `crawl_fwd` cycle and
   the turn angle per `crawl_turn_*` cycle; write both into §9.2 (the code uses them for speed).
3. **Check in Blender**: viewport renders (`render_viewport_to_path`) of 4 key frames per clip, from the side and
   from above; feet must not go through the floor plane, hands stay planted while pulling (no sliding).
4. **Export** like `standUp.glb` to `GameData/animations/npc/crawl.glb` (all five actions in one file, or one file per
   clip if that is how the existing clips are loaded — follow the existing pattern). Save the source as
   `sourceAssets/StealthAction/animations/npc/crawl.blend`.
5. **Hand over**: the user judges the look and feel in game; send them the renders.

### P4.6 Crawl locomotion in game

Replaces the "stay in the stunned ragdoll" first version once `crawl.glb` exists.
1. Load the clips in `NpcBase::LoadAssets` the same way `getFromRagdollAnimation` loads `standUp.glb`.
2. State in NpcBase: `goreCrawling` (set by the `crawl` consequence). On leg loss the enemy **falls with the
   ragdoll and immediately goes on crawling** — no lying around, no stun wait:
   * start the ragdoll (as `StartStunnedRagdoll` does), keeping the AI and target;
   * as soon as the body has landed — the pelvis hitbox is near the floor (short downward trace) and its speed is
     below ~1.5 m/s, or at the latest 0.8 s after the cut — leave the ragdoll through the existing
     return-from-ragdoll path, but blend into `crawl_from_ragdoll` (or straight into `crawl_fwd` if the clip blend
     already looks right) instead of `standUp`;
   * the AI keeps its current goal the whole time and resumes moving the moment the crawl clip starts.
   Do not use the normal stun duration here (`isStunned()` timing) — that is what makes it lie still.
3. Locomotion: the enemy's existing movement/navigation, with speed = measured crawl distance / cycle time and turn
   rate from the turn clip; pick `crawl_fwd` / `crawl_turn_*` / `crawl_idle` from velocity and turn rate, the same way
   walk/run/idle are picked today (find that code and follow it). Melee enemies use `crawl_attack` at close range;
   ranged enemies don't shoot while crawling unless their combat behaviour already supports a prone state.
4. Capsule: check the character controller for a way to lower it (crouch / set height). If there is one, use the
   crawl height; if not, log it in §9.1 as an open item and keep the capsule.
5. Death while crawling: plain `Death()` (ragdoll), no special handling.
6. Leg regrown (phase 5): play `standUp` through the existing return-from-ragdoll path, clear `goreCrawling`.

### Phase 4 exit check
* Arm with the gun cut off: the enemy drops the gun and keeps fighting per its behaviour. Leg off: falls with the ragdoll
  and right after landing (≤ 0.8 s) crawls on toward its goal (P4.6), bleeds out. Head off: dies. Nothing changes for enemies without gore.

---

## 5. Phase 5 — Regeneration and durability

### P5.1 Regen clock

`Gore::RegenClock(frozen, pausedUntil, t) = t < pausedUntil ? frozen : frozen + (t - pausedUntil)`.
Initial `frozen = 0, pausedUntil = 0` → `clock(t) = t`.
Suppression hit at time `t` with seconds `s` (tag in `regen.suppressedBy`):
```
if (t < pausedUntil) pausedUntil = max(pausedUntil, t + s);
else { frozen = RegenClock(frozen, pausedUntil, t); pausedUntil = t + s; }
```
All heal and regrow times (`healStart`, `regrowStart`) are in **clock** units. Rendering computes
`c = RegenClock(state.frozenClock, state.pausedUntil, Time::GameTime)` in `PreFinalize` and derives:
* wound `healScale = healDuration > 0 ? 1 - smoothstep(healStart, healStart + healDuration, c) : 1`; when it reaches
  `< 0.02` the system removes the wound (tick, `version++`).
* cut regrow radius `= regrowMaxRadius * smoothstep(regrowStart, regrowStart + regrowDuration, c)` while
  `phase == Regrowing`, else `-1`.

### P5.2 Tick — phase changes only (`GoreRegenSystem`, `PostUpdate`)

For each `GoreComponent` whose body rules have `regen`:
* New wounds get `healStart = clock(now) + woundHealDelay`, `healDuration = woundHealSeconds` when created.
* `Severed` cut with `regrowDuration > 0` and not `charred`: when `clock >= severClock + regrowDelay` (store the
  sever's clock value in `regrowStart` at sever time as `clock + delay`): `phase = Regrowing`, `version++`, ranges
  include the subtree again (ranges: `removed(b)` is false for `Regrowing` cuts), render palette rule no longer
  applies to this cut.
* Regrow progress `>= hitboxActiveAt` and `!hitboxesBack` → `SetHitboxesActive(subtree, true)`, `hitboxesBack = true`.
* Progress `>= 1` → remove the cut (`version++`), emit `GorePartEvent{lost=false}`.
* Cutting a bone inside a `Regrowing` subtree: replace that cut with the new one (restart).
* `preventRegrowBy` tag on the severing hit or on a later hit within the cut's influence sphere → `charred = true`,
  `regrowDuration = 0`.
* Health regen (`healthRegenPerSecond` after `healthRegenDelay` without damage) in the adapter.

### P5.3 Durability in `NpcBase::OnDamage`

Add `Damage = StealthGore::FilterDamage(this, Damage, Weapon, DamageCauser);` at the top of `NpcBase::OnDamage`.
* `mortal` → unchanged.
* `immortal` → `min(Damage, Health - minHealth)` (never below `minHealth`).
* `conditional` → if the source's tags intersect `killedBy`: unchanged; else clamp like immortal.
* The source of the current hit: `Weapon`/`DamageCauser` as in P2.7 (same resolution function — reuse it).
* Finisher: when a `conditional` kill happens (`wasAlive && dead` in `OnNpcPointDamage`) and `finisher` is set → set
  `hit.finisher = true`; `ProcessPendingHits` applies the finisher sever (`mode: destroy` → head destroy path).

### P5.4 `onRegrow` modes
* `keep`: nothing (gib stays in the pool).
* `dissolve`: when regrowth starts, the gib gets `dissolveStart`. Add a uniform `u_goreDissolve` (vec4, x = progress
  0..1, declared in `gore_common.sh`, uploaded by `GoreDrawCommand`, a new `GpuData::dissolve` field); in `fs_gore` discard when
  `goreNoise(v_goreBind.xy * 4.0) * 0.5 + 0.5 < dissolve`, darken near the threshold; destroy the gib at 1.
* `reattach`: when regrowth would start and the gib exists within `reattachMaxDistance` and a line trace from the gib
  to the stump hits nothing: gib bodies → kinematic; each frame move the gib's root body along a quadratic arc to the
  stump's current world transform (`bodyMesh->GetBoneMatrixWorld(S)`), over `reattachSeconds`. At the end: destroy
  the gib, remove the cut (no regrowth), emit `GorePartEvent{lost=false}`, turn hitboxes on. If the gib is destroyed
  first or the trace is blocked → normal regrow.

### Phase 5 exit check (vampire)
* Create `vampire_elder.json` (GORE_DESIGN.md §6.11) and map one enemy class (or a test spawn) to it.
* Guns blow parts off; it stays alive; parts regrow (sphere growth front, fresh skin band), hitboxes come back at 60%.
* Only a melee hit kills it; the killing blow destroys the head.
* Crucifix (holy) pauses all healing for 6 s. Explosion (fire) chars stumps: they never regrow.

---

## 6. Phase 6 — Save/load and network

### P6.1 Save
In `NpcBase::Serialize` add `StealthGore::Serialize(this, target["gore"]);`, in `Deserialize`
`StealthGore::Deserialize(this, source["gore"])` (check that it runs before the first render; otherwise apply it
lazily in the gore tick on the first frame).
* Write wounds, cuts, splats with all times **relative**: `regenClockNow = RegenClock(...)`; store
  `t - regenClockNow` for clock times and `pausedUntil - GameTime` (clamped ≥ 0). `gibId` is not saved.
* Load: `frozen = GameTime`, `pausedUntil = GameTime + savedPaused` if > 0 else `GameTime`; clock times =
  `GameTime + saved`. Cuts in `Severed`/`Regrowing` → `SetHitboxesActive(subtree, false)` (or `true` when
  `hitboxesBack`); re-emit `GorePartEvent{lost=true}` for parts with consequences; `version++`.
* A `reattach` cut whose gib is gone after load regrows normally.

### P6.2 Network
* Seeds already come from `(victim id, per-victim hit counter)`. Verify: debug command "dump gore" prints the wound
  list (centres rounded to 1 mm); run two peers, shoot the same enemy, compare.
* Body rules may contain `"network": { "onRegrow": "regrow" }`; when the session is networked (grep how StealthAction
  knows it — `NetworkedEntity`, `networkProxyId`), use it.

---

## 7. Phase 7 — Scale and polish

1. `-gore_stress <count>` launch option (find how StealthAction parses launch options; follow SlasherGame's
   `-perf_crowd` pattern): spawns `count` enemies in view and emits 50 random gore hits per frame (mixed sources).
   Capture with Tracy. Targets: 300 enemies, 50 hits per frame, no frame over budget; undamaged crowd within noise.
2. Only if profiling shows uniform upload matters: move `GpuData` into a per-frame RGBA32F texture (one row per
   body). Don't do this otherwise.
3. Rim displacement, stump pucker, growth bulge in `vs_gore` (GORE_DESIGN.md §4.6). Needs the wound/cut arrays in the
   vertex stage: declare them in a `gore_vs.sh`; keep the count small (wounds bind rows + params, cuts bind + sphere +
   regrow). Test on the web target (vertex uniform limit).
4. Floor/wall blood decals using the existing decal shader, capped per frame.
5. Player gore level option (`GoreSettings::level`): `Reduced` = no particles/decals, interior desaturated (shader
   already reads `u_goreParams.w`); `Minimal` = no blood at all, caps dark. Gameplay identical at every level.

---

## 8. Common mistakes checklist (check before saying a task is done)

| Mistake | Symptom | Rule |
|---|---|---|
| skin index used where skel index is needed (or the reverse) | wrong limb clipped, garbage ranges | §0.2; name variables `...Skin` / `...Skel` |
| `operator[]` on `hitboxConstraints` | crash in ragdoll after a sever | always `find` |
| Upload before the partition | ranges don't match the buffer, flickering triangles | E4.3 order |
| Forgot to filter zero-count ranges | state leaks into the next draw | `SubmitIndexed` as written |
| Cut without influence sphere | head or other arm disappears when one arm is cut | §0.3 (2) |
| Using `std::uniform_real_distribution` | peers disagree | `Gore::Rng` only |
| Reading ECS in `PreFinalize` | random crashes | `SetGoreState` only |
| New `std::vector` per frame | GC-like hitches with many bodies | members with reused capacity |
| `texture2D` inside loops with `discard` | shader compile errors on some platforms / wrong mips | `texture2DLod(..., 0.0)` in `gore_common.sh` |
| Normal transformed like a point | tilted caps, wrong cap side | §0.2 normal rule |
| Interior command drawn without `Cull = Front` | flesh drawn over intact skin | interior settings in P2.4 |
| Times stepped per frame | save/load and network mismatch | store start times, use the regen clock |
| Gib keeps detail shadows | shadow of the whole body under the gib | `CastDetailShadows = false` |

---

## 9. Logs (fill in while implementing)

### 9.1 Engine change log

| ID | Files | Change | Why |
|---|---|---|---|
| E1 | DrawCommands/StaticMeshDrawCommand.h/.cpp, StaticMesh.h/.cpp | `VertexShader` field + virtual `GetForwardShader/GetDepthShader/GetCustomIdShader/GetShadowVolumeShaders`; `StaticMesh::SetVertexShader` | gore swaps the vertex/pixel programs per draw command |
| E2 | DrawCommands/StaticMeshDrawCommand.h/.cpp | `CullFace Cull` + `ResolveCull()` replaces the 4 inline cull expressions | interior pass draws back faces |
| E3 | Shader.hpp/.cpp, DrawCommands/StaticMeshDrawCommand.h/.cpp | `Shader::Submit(viewId, discardFlags)`, `SetUniform(vec4*, count)`, `DrawIndexRange`, `SubmitIndexed`, range vectors | one draw split in index ranges with a per-range uniform |
| E4 | Helpers/Mesh/bone_partition.hpp/.cpp (new, Engine.vcxproj), skinned_model.hpp/.cpp, Helpers/Mesh/shadow_volume.hpp/.cpp, AssetRegistry.cpp | triangles sorted by bone LCA; `SkinnedMesh::boneIndexFirst/boneOwnBounds/boneAnyBounds`, shadow `capBoneFirst/edgeBoneFirst`; GPU upload moved to the end of `load()` and re-partitioned after `AdoptVisualTierGeometry`; `BoneBound` moved above `SkinnedMesh` | draw only the triangles of a bone subtree |
| E5 | SkeletalMesh.hpp/.cpp | `GetSkinPalette`, `GetSkinIndex`, `GetSkeletonIndex`, `GetSkinnedModel`, virtual `GetRenderPalette`, `GetFinalizedSkinPalette` | render with a modified palette |
| E6 | Physics.h/.cpp, SkeletalMesh.hpp/.cpp | `Physics::Remove/AddBodyToWorld`, `Remove/AddConstraintToWorld` (the removed constraint keeps one extra reference), `IsBodyInWorld`; `DestroyBody` guards `RemoveBody` with `IsAdded`; `SkeletalMesh::SetHitboxesActive/IsHitboxActive`, `inactiveHitboxes`; ragdoll branch uses `find` (fixes `hitboxConstraints[]` null insert) and takes `hitboxMutex` before `animationsMutex`; the 4 ragdoll bounds loops skip inactive hitboxes | severed limbs must not be hit or simulated |
| E7 | skinned_model.hpp/.cpp | `ComputeAnimatedBounds(world, skin, skinMask)` | bounds without severed bones |

Phase 1 compiles (2026-11-12, `Build/ui_stealth`). The phase 1 visual exit check (index counts, shadows, hitbox on/off, load time) has not been run.

Phase 2 code (GoreSystem, StealthGoreAdapter, GoreDebug, NpcBase/weapon hooks, shaders, data) compiles (2026-11-12). Not run in game yet: the phase 2 exit check and the 50 x 8 wounds timing are still open. Melee sweep normal is the camera up vector for now (horizontal slash); the sword animations do not give a real swing plane.

Phase 2 visual check (2026-11-13, headless, `-gore_look npc_human_axe -gore_src <class> -gore_dist 1.1 -gore_shots 5 -gore_file <name>`; the test is in `Gore/GoreDebug.cpp`, not compiled with DISTRIBUTION):
* Holes at the hit point with noisy edges, flesh cavity walls and red rims render for pistol, sawed-off, explosion and sword (gash). A hit on a thin part shows a through-hole.
* Found by looking: the engine reads sampler slots from the text of the shader's own `.sc` file only (`Shader::ParseSamplerSlots`), not from includes. The gore samplers were declared in `gore_common.sh`, so they defaulted to slot 0, replaced the albedo texture (enemy looked pale grey) and the cavity sampled an unbound texture (black). The `SAMPLER2D` lines now live in every gore `.sc`. Rule: declare `SAMPLER2D(name, slot)` in the `.sc`, never in an include.
* Not yet checked: glued during animation and ragdoll, custom-id pass, undamaged-crowd frame time, 50 enemies x 8 wounds timing. The robe of the axe enemy is cloth but profiled as Volume (thick flesh under it); consider shell roles per model.

Phase 3 (2026-11-13, headless looks with `-gore_aim <bone> -gore_flip 1 -gore_yaw <deg> -gore_wait <tick>`; all code in `Gore/`, the only call site outside is `Gore::GibPool::Tick()` in `GameStart::Update`):
* Cut solver: plane tests use vertex samples per severable bone (`SeverableBone::partPoints` = vertices that leave with the part, `ownPoints` = vertices the bone keeps), not bounding boxes: the box of a diagonal limb reaches past the next joint and made every cut "invalid". Heads need a wider `range` (hood vertices hang below the neck joint).
* Cut bits (which bones the shader clips/caps) are set only for the cut bone on the body and for the part's subtree on the gib. Setting them for every bone whose box overlaps the influence sphere clipped the torso and robe.
* Gib: new entity with its own `GoreSkeletalMesh`, hitboxes only of the part. Ragdoll creation order is CreateHitboxes (rest pose), PasteAnimationPose, UpdateHitboxes, StartRagdoll, then body velocities. Render palette: bones outside the part keep their cut-time pose rigidly attached (`gibSeverPalette`). Spawning is queued (`GibPool`) and done from `GameStart::Update`, not from a tick group (EcsGuard warning).
* Shadow volumes: `vs_shadowvolcap_gore` / `vs_shadowvoledge_gore` flatten bind positions beyond the cut plane; the draw command falls back to the stock volume shaders when they are missing (a null program crashed `Shader::UseProgram`).
* Stump FX (`GoreFx.cpp`) spawns small droplet jets through `GoreSpurtSystem` (`gore_spurt`), pulsing, then drips. The stock `hit_flesh` particle was far too big.
* Found by looking: a red sleeve gib in front of a red robe looks like part of the body. The gib was drawn the whole time (confirmed by a magenta debug colour); judge gibs against a contrasting background or from the side.
* Not verified yet: cap look head-on (flesh, fat, bone, marrow), sword cut follows the blade angle, shotgun at a forearm, shooting the gib makes holes, 50 bodies + 48 gibs frame time. Gibs are retired by popping, not sinking. Phases 4-7 not started.

Wound cavities as skinned geometry (2026-11-13, replaces the world-space ellipsoid ray test of the interior pass; the interior pass now draws cut caps only):
* `Gore/GoreWoundMesh.cpp` `BuildCavity`: every model triangle that overlaps the wound ellipsoid is subdivided into a barycentric grid (same density on all triangles, so shared edges agree); grid points inside the wound are pushed inward along the authored (vertex-normal) outside direction by the ellipsoid depth under them, limited to 60 % of the body behind; points outside stay on the skin, so the bowl joins the surface along the hole edge. Each point gets bone indices/weights interpolated from its triangle (top four bones). Nothing depends on the shot direction (earlier ray/shot based attempts dug outward on grazing shots and on exit wounds that landed on the entry side).
* Stored as `Wound::cavity` (shared, immutable, rebuilt when wounds merge); `GoreSkeletalMesh::UpdateCavities` creates one vertex/index buffer pair per cavity and one `Pass::Cavity` draw command (`gore/vs_gore` + `gore/fs_gore_cavity`, shared flesh helpers in `gore_flesh.sh`), buffers destroyed 4 frames after the wound is gone. The cavity is drawn with the body's own palette, so it follows animation, cloth and ragdoll.
* Vertex layout gotcha: `vs_gore` reads `a_texcoord2` (= `VertexData::ShadowMapCoords`) as `v_texcoord1`; the cavity stores (lateral fraction, depth fraction) there.
* Also fixed: a submesh with no triangles left (all its bones removed) had an empty range list, and an empty list draws the whole submesh; it now gets a zero-count range.
* Not verified yet: a hole during ragdoll / strong bending (by construction it moves with the skin), healing wounds (cavity does not shrink), cost with 8 large wounds on a dense mesh (grid capped at 3000 triangles per wound).

Cavity building is asynchronous (2026-11-13): the first version scanned every triangle of the model on the game thread per wound and stuttered on dense meshes. Now `GetProfile` builds a spatial grid of the skin triangles once (`Profile::triGrid`); `RequestCavity` (game thread, ~0.1 ms) looks up and copies the few hundred triangles around the wound; `BuildCavityFromTriangles` runs on a private one-thread `ThreadPool` ("GoreCavity", inline where threads are unsupported) and never touches the model; the game thread polls `Wound::cavityTicket` once per tick (`PollCavity`) and bumps the gore version when a cavity arrives. A wound is only handed to the renderer once its cavity exists (the hole alone would show through the body), so a hit appears a few ms later; wounds with no skin to build against (`cavityFailed`) are not drawn. Measured on the axe enemy: prep 0.06-0.14 ms, worker 0.3-1.4 ms per wound (150-250 triangles).

### 9.2 Measurements

| When | Scene | Frame time before | after | Notes |
|---|---|---|---|---|
