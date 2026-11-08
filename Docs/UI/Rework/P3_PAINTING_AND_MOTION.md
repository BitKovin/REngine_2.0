# Phase 3: full CSS painting and motion

**Goal:** the "at launch" column of the design's §10.13, minus the layer-based features (blur, drop-shadow, backdrop, isolation: phase 7).

- gradients
- background images (atlas)
- `border-image`
- box and text shadows
- outline
- text stroke
- colour-matrix filters
- gradient masks
- `clip-path` basic shapes
- blend modes
- arcs and paths
- 2D transforms
- transitions, `@keyframes`, UI sounds, reduced motion

**Prerequisites:** phase 1 (phase 2 recommended, for the `:entering` / `:leaving` hooks). All drawing rules from conventions §11 apply. In particular, the AA width comes from texel 2 `.z`, never from derivatives.

## Files

```
source/Engine/UI/Render/UiImageAtlas.h/.cpp      runtime atlas for <= 512 px images
source/Engine/UI/Render/UiGradientRamps.h/.cpp   256 x 256 RGBA8 ramp texture, one row per gradient
source/Engine/UI/Style/UiAnimation.h/.cpp        transitions, keyframes, easing
edited: fs_ui2.sc, UiRenderTypes.h, UiPrimBuffer (aux rows), UiPainter, UiComputedStyle (+ table), UiCssParser (@keyframes), UiFlowLayout (render transform), UiSystem (animation tick), Elements (Image uses atlas)
```

## Task 3.0: effect programs and batch merging

Expensive effects don't go into the base shader. Each has its own program, so ordinary boxes stay cheap.

1. **Shadow program:** create `ui2/fs_ui2_shadow.sc`. It uses the same varyings and the same `uiFetch` helpers. Put the helpers in a shared include, `ui2/ui2_common.sh`, used by both `fs_ui2.sc` and `fs_ui2_shadow.sc`; `.sh` files aren't compiled on their own.
   - Register it with `UiGpu::RegisterProgram("ui2/vs_ui2", "ui2/fs_ui2_shadow")` and keep the index as `kProgramShadow`.
   - The `BoxShadow` kind (task 3.1, item 5) lives **only** in this program. The base shader never contains the shadow loop.
2. **Batch merging** in `UiDrawList::AddTriangles`, so alternating programs (shadow, box, shadow, box…) don't multiply draw calls:
   - Every geometry item keeps the screen-space AABB of its vertices (`glm::vec4 Bounds`), updated as it grows.
   - When the new geometry's key doesn't match the last item, look back over at most **8** earlier geometry items for one with an equal key.
   - The new geometry may be appended to that earlier item only if its AABB intersects **none** of the items after it (geometry or external). Externals count as intersecting everything.
   - Merge into the most recent qualifying item. Otherwise start a new item as before.
   - This keeps the painter's order wherever pixels overlap, so output is unchanged. Count merges in `ui_stats` (`merged N`).
   - Vertex rebasing: a merged item's vertices must be contiguous. So store per-item vertex and index vectors while recording, and concatenate them in `UiGpu::Submit` (one transient buffer for the whole list, as before). Re-check the phase 0 goldens after this change.
3. Multi-pass effects (blur, glow, drop-shadow, backdrop) are phase 7. Don't approximate them in a single pass here.

## Task 3.1: shader additions (fs_ui2.sc, fs_ui2_shadow.sc)

1. **Ramp sampler:** `SAMPLER2D(s_uiRamp, 2);`. The batch always binds `UiGradientRamps::Texture()` on stage 2 (add that to `UiGpu::Submit`). Sample it **inside branches only with `texture2DLod(s_uiRamp, uv, 0.0)`**.
2. **Aux data:** rows 224–255 of the data texture. `UiPrimBuffer::AddAux(const glm::vec4* texels, int count)` returns the first aux texel index (frame-wide, like prims; reset in `ResetFrame`). Fetch with `uiFetchAux(int index)` (row = `224 + index / 1024`). Local draw lists get local aux storage, rebased in `AppendResolved`, exactly like prims.
3. **Fill kinds.** For `Box`, T11 = gradient header `(type, rampRow, repeat, 0)`, with type 0 none, 1 linear, 2 radial, 3 conic:
   - **linear:** T12 = `(p0.x, p0.y, p1.x, p1.y)` in local units; `t = dot(local − p0, p1 − p0) / dot(p1 − p0, p1 − p0)`
   - **radial:** T12 = `(cx, cy, rx, ry)`; `t = length((local − c) / r)`
   - **conic:** T12 = `(cx, cy, startAngleRad, 0)`; `t = fract((atan(dy, dx) − start) / 6.2831853 + 1.0)`, measured clockwise from 12 o'clock as in CSS, so use `atan(dx, −dy)`
   - `repeat` → `t = fract(t)`, else `clamp(t, 0, 1)`
   - colour = `texture2DLod(s_uiRamp, vec2((t × 255 + 0.5) / 256, (rampRow + 0.5) / 256), 0.0)`
   - the gradient colour **replaces** `fill`. When `kFlagHasTexture` is also set, the texture multiplies it.
4. **Image fill with tiling** (background-image and atlas images): T11.w = 1 means the UVs are tiled. T12 holds the atlas sub-rect `(u0, v0, u1, v1)` and `v_texcoord0` carries the **tile** coordinate. The final UV is `mix(u0v0, u1v1, fract(tile))`. Because that is computed in a branch, sample with `texture2DLod(s_uiTex0, uv, 0.0)` for tiled fills.
   - **Exception to the rule:** this loses mips for tiled images, which is acceptable. Untiled images keep the top-level `tex`.
5. **`BoxShadow` kind (3), in `fs_ui2_shadow.sc` only.** Batch key `Program = kProgramShadow`. The quad is the shadow rect inflated by 3σ. T2 = `(shadowW, shadowH, aa, sigma)`, T3 = radii (the largest one is used), T1 = colour, T13.z = 1 for inset. Use exactly:

   ```
   vec2 uiErf2(vec2 x) { vec2 s = sign(x); vec2 a = abs(x); x = 1.0 + (0.278393 + (0.230389 + 0.078108 * (a * a)) * a) * a; x *= x; return s - s / (x * x); }
   float uiGauss(float x, float sigma) { return exp(-(x * x) / (2.0 * sigma * sigma)) / (2.5066283 * sigma); }
   float uiShadowX(float x, float y, float sigma, float corner, vec2 halfSize)
   {
       float delta = min(halfSize.y - corner - abs(y), 0.0);
       float curved = halfSize.x - corner + sqrt(max(0.0, corner * corner - delta * delta));
       vec2 integral = 0.5 + 0.5 * uiErf2((x + vec2(-curved, curved)) * (0.70710678 / sigma));
       return integral.y - integral.x;
   }
   float uiRoundedBoxShadow(vec2 halfSize, vec2 p, float sigma, float corner)
   {
       float low = p.y - halfSize.y; float high = p.y + halfSize.y;
       float start = clamp(-3.0 * sigma, low, high); float end = clamp(3.0 * sigma, low, high);
       float step = (end - start) / 4.0; float y = start + step * 0.5; float value = 0.0;
       for (int i = 0; i < 4; i++) { value += uiShadowX(p.x, p.y - y, sigma, corner, halfSize) * uiGauss(y, sigma) * step; y += step; }
       return value;
   }
   ```

   - Outer shadow: `alpha = uiRoundedBoxShadow(halfSize, local − centre, max(sigma, 0.5), corner)`. Multiply by `1 − coverage(element box SDF)` **only for inset**.
   - Inset shadow: alpha = `(1 − shadowValue) × coverage(element box)`, where the shadow box is the padding box deflated by the spread and moved by the offset.
   - CSS blur radius B → `sigma = B / 2`. Spread grows (outer) or shrinks (inset) the rect and the radius.
6. **`Arc` kind (4):** T2 = `(w, h, aa, thickness)`, T12 = `(startRad, sweepRad, 0, 0)`. The ring SDF is `abs(length(p) − (R − thickness / 2)) − thickness / 2`, with `R = min(w, h) / 2`, intersected with the angle wedge, and round caps if T13.w = 1. The fill can use a gradient (conic for a sweep look).
7. **`Solid` kind (5):** colour T1, coverage = 1. Anti-aliasing comes from tessellated fringe geometry: path fringe vertices carry an alpha in `v_local.x`, so the kind multiplies by `v_local.x`.
8. **Colour matrix:** when T0.w ≥ 0, fetch 5 aux texels (rows of a 4×5 matrix, straight RGBA) and apply them to the **straight** colour before premultiplying. This applies to all kinds, through a helper called before `uiPremul`. Restructure the shading functions to return straight colour plus coverage, and premultiply once in `main`.
9. **Masks:** flag `kFlagMask = 4`, T15 = `(type, rampRow, auxIndex, 0)` with a linear or radial mask whose geometry sits in aux. Coverage is multiplied by the ramp alpha, evaluated in **screen** space (stored in UI units).
10. **`clip-path`:** flag `kFlagClipShape = 8`, T10 reused when set: `(shape, cx, cy, r)` in UI units, where shape 1 = circle, 2 = ellipse (an aux texel holds rx, ry), 3 = inset (uses T9 rect + T10 radii as the normal clip does).
11. **Text stroke and shadow** (`SdfGlyph`): T13 = `(glyphScale, dilation, softness, strokeWidth)`, T14 = stroke colour.
    - fill coverage uses `d`
    - stroke coverage uses `d + strokeWidth`, and stroke is composited under the fill
    - softness > 0 → `cov = smoothstep(-softness, softness, d)` (text-shadow blur = 2 × softness)
    - text shadows are separate glyph runs drawn before the text, offset by the shadow offset, with dilation 0 and the shadow colour

Compile all games' shaders: no `[FAILED]`.

## Task 3.2: atlas and ramps

### `UiImageAtlas`

- `Get(path)` returns `{page texture, uv rect}`, or `{texture handle, full uv}` when the image is larger than 512 px.
- Decode with `Texture::DecodeFromFile(path, false)` on the main thread (first use only, then cached).
- Skyline packer, 4 px padding with the edge pixels duplicated into the padding, RGBA8 2048² pages, max 4 pages.
- Mips: build CPU mips per page once a page is complete or dirty (at the end of `UiSystem::Update`), box filter, upload all levels.
- Overflow → standalone `AssetRegistry` texture.

### `UiGradientRamps`

- `Row(stops)` returns a row index, cached by the exact stop list.
- 256 entries per row: interpolate the stops in sRGB, **premultiplied**, then store straight.
- CSS stop rules:
  - stops without positions spread evenly
  - positions are clamped to be non-decreasing
  - CSS hints aren't supported (warning)
- 256 rows; when full, the least recently used row is reused (rows touched this frame aren't eligible).

## Task 3.3: new style properties

Add to the computed style and property table. All are non-inherited unless marked.

| Property | Grammar (CSS) | Invalidates |
|---|---|---|
| `background-image` | comma list of `url()`, `linear-gradient()`, `radial-gradient()`, `conic-gradient()`, `repeating-*` | paint |
| `background-size` `background-position` `background-repeat` | CSS values, per layer (`repeat` `no-repeat` `repeat-x` `repeat-y`; `cover` `contain` `<len>{1,2}`; keywords and lengths) | paint |
| `border-image` (shorthand), `border-image-source/slice/width/repeat` | `url()` + slice numbers (texels) + `fill`; `stretch` \| `round` | paint |
| `box-shadow` | comma list: `[inset] <x> <y> [blur] [spread] <color>` | paint |
| `outline`, `outline-width/style/color`, `outline-offset` | like `border`; offset `<length>` | paint |
| `text-shadow` (inh.) | comma list: `<x> <y> [blur] <color>` | paint |
| `-webkit-text-stroke`, `text-stroke` (inh.) | `<width> <color>` | paint |
| `filter` | list of `brightness() contrast() saturate() grayscale() sepia() hue-rotate() invert()` (others: warning; `blur` / `drop-shadow` are phase 7) | paint (patch) |
| `mask-image` | `linear-gradient()` \| `radial-gradient()` | paint |
| `clip-path` | `inset()` \| `circle()` \| `ellipse()` | paint |
| `mix-blend-mode` | `normal` \| `plus-lighter` \| `multiply` \| `screen` | paint |
| `transform`, `transform-origin` | 2D CSS functions; origin default `50% 50%` | transform (patch) |
| `wash-color` | `<color>`: tint over everything, i.e. colour matrix lerp to the colour by its alpha | paint |
| `image-rendering` | `auto` \| `pixelated` | paint |
| `object-position` | like `background-position` | paint |
| `transition` (+ longhands) | CSS | – |
| `animation` (+ longhands), `@keyframes` | CSS | – |
| `sound-enter`, `sound-exit`, `sound-volume` | `<string>` path, number | – |

Rules:

- **Colour matrices:** use the standard filter-effects formulas. `grayscale(a)`, `sepia(a)`, `saturate(a)` and `hue-rotate(θ)` use the W3C Filter Effects matrices. `brightness`, `contrast` and `invert` are linear per channel.
  - The element's matrix = parent matrix × own matrix.
  - Identity → no aux.
  - `wash-color` is applied after the filters.
- **Blend modes:**

  | Mode | Blend function |
  |---|---|
  | `plus-lighter` | `ONE, ONE` |
  | `multiply` | `DST_COLOR, INV_SRC_ALPHA` |
  | `screen` | `ONE, INV_SRC_COLOR` |

  Each is a different `State` in the batch key, so it breaks the batch. That is expected.
- **Transforms:**
  - local transform = `T(origin) × fn1 × fn2 × … × T(−origin)` (origin in border-box units)
  - `AbsoluteTransform = parent × T(rect.xy) × local`
  - hit testing follows through `worldMatrix`, since `UpdateOffsets` copies it
  - layout ignores transforms
  - `px` AA = 1 / sqrt(|det|)
  - `pixelated` sets sampler point flags on that draw (`bgfx::setTexture` flags `BGFX_SAMPLER_POINT`) via a `BatchKey` field `SamplerFlags`

## Task 3.4: animation engine (`UiAnimation`)

- **Time:** real time. `dt` = `SDL_GetTicks64` delta, clamped to 0.1 s. It runs while the game is paused.
- **Animatable types:** float, length (px; % only if both ends are %), colour (interpolate premultiplied), transform (match function lists pairwise by type, else interpolate the decomposed 2D matrices: translate, rotate, scale, skew), and shadow lists of equal length.
- **Transitions:** after style resolve, compare each transitionable property's new computed value with the **currently displayed** value. If it differs and a transition is defined for it: start `from = displayed`, `to = new`, with duration, delay and easing. A running transition re-targeted mid-way starts from the current displayed value.
- **Animations:** `@keyframes` with percentages, `from` and `to`. Supported longhands: `animation-name`, `duration`, `timing-function`, `delay`, `iteration-count` (number \| `infinite`), `direction` (`normal` `reverse` `alternate` `alternate-reverse`), `fill-mode` (`none` `forwards` `backwards` `both`).
- **Easing:**
  - `linear`
  - `ease` = cubic-bezier(.25, .1, .25, 1)
  - `ease-in` = (.42, 0, 1, 1)
  - `ease-out` = (0, 0, .58, 1)
  - `ease-in-out` = (.42, 0, .58, 1)
  - `cubic-bezier()`, solved by Newton then bisection, error < 1e-5
  - `steps(n, start|end)`
  - **First search `MathHelper.hpp` for existing bezier or easing helpers, and reuse them** if they match.
- **Writing values:** animated values are written into a separate "animated" overlay applied over the computed style each frame. A property with layout invalidation marks layout-dirty while it animates. Transform, opacity and filter only re-record their chunk.
- **Reduced motion:** `UiSystem::ReducedMotion` (a setting, phase 4). When true, every transition duration is 0 and `infinite` animations don't run.
- **Pages:**
  - `kStateEntering` is set on push and cleared one frame later, so transitions run from `:entering` values
  - `Pop` sets `kStateLeaving` and removes the page when no transition or animation is running on it, or after 2 s at most
  - `UiMenuStack` gets a `m_leaving` list for pages that are animating out
- **Sounds:** `sound-enter` plays when the rule that declares it starts to match the element (a pseudo-class turns on), `sound-exit` when it stops; events such as click, doubleclick or denied put the element in a pulse pseudo-class (`:click`, `:doubleclick`, `:denied`, ... for `pulse-duration`), so they can play sounds and animations the same way.

## UIDB and visual-preview integration

Extend the same `uidb.json` with the real computed-style and painter capabilities. Publish each property/effect/animation descriptor when its runtime registry or parser support is added, then call `UiDatabase::MarkDirty()`; do not add a second writer or wait until the whole phase is finished:

- CSS property metadata already registered by phase 1, now including every phase 3 property, grammar, defaults, invalidation category, backend limitations and whether it requires an offscreen layer.
- Paint primitives/effects available to markup (`Arc`, gradients, background images, shadows, outline, stroke, filters, masks, blend modes and transforms) with accepted parameters.
- Registered keyframe names, keyframe property tracks, transitionable property names and easing functions. Do not serialize a runtime animation instance.
- Add preview states for each effect-bearing element: baseline, transformed, shadowed, filtered, clipped and animated. Include a deterministic animation timeline fixture with pause/scrub controls in Studio; the actual interpolation remains in `UiAnimation`.
- Runtime preview providers may declare `animationScenarios` (start, mid-point, end, interrupted/re-targeted) so Studio can preview a game HUD's animated crosshair expansion, damage flash, cooldown ring or notification entrance.
- Include required capabilities/backends for effects that differ by renderer. Studio displays unsupported backend/effect combinations rather than silently removing the effect.

A provider's visual fixture must use the actual engine style resolver, animation engine, painter and shaders. Qt may draw timeline/curve controls outside the game viewport, but not the animated UI itself.

**Check:** `uidb.json` lists the phase 3 properties/functions and a sample effect preview. Use a preview scenario to capture baseline/mid/end screenshots, and verify a target lacking the required layer capability is reported unavailable rather than approximated.

## Tests (StealthAction, all backends; layouts in `ui/kit/tests/p3_*.xml`)

1. **`p3_paint`**, probes at fixed points (premultiplied):
   - a 256 px linear gradient `#ff0000 → #0000ff` left to right: at x = 128 expect ≈ (128, 0, 127, 255), tol 3
   - a radial gradient centre = the first stop
   - a conic quarter
   - outer shadow: a pixel 6 px outside the box has 0 < alpha < 255; 40 px outside, alpha 0
   - inset shadow darkens the inner edge
   - outline ring pixel = the outline colour; the gap pixel (`outline-offset`) is transparent
   - `filter: grayscale(1)` on red → r = g = b
   - `mix-blend-mode: plus-lighter` over grey adds
   - a rotated 30° box via `transform`: centre colour and a corner outside are transparent; `expect` its hit test through `click` at the centre activates it
   - an `Arc` 0–90° filled at 45°, empty at 135°
   - a gradient mask fading to 0 at the bottom edge
   - `background-image` from the atlas, tiled 4×: the probe at tile 2 equals tile 1
2. **`p3_text_fx`:** text stroke and text shadow. Visual check plus `cross`.
3. **`p3_motion`:**
   - `set_class box hot 1` triggers `transition: opacity 0.5s linear` from 1 to 0
   - after `wait 0.25`, `expect box opacity_now > 0.2` and `< 0.8` (add `opacity_now`: the animated value)
   - after `wait 0.5`: `== 0`
   - keyframes `pulse`, infinite: `opacity_now` changes between two reads
   - reduced motion (`console ui_reduced_motion 1`): the next change is instant
4. **`p3_pages`** (if phase 2 is done): a page with `.page:entering { opacity: 0 }` and a 0.2 s transition. One frame after push, `opacity_now < 1`; after 0.3 s, == 1. Pop → the page is still present at 0.1 s, gone at 0.4 s.

**Phase done when:**

- all tests pass on every backend, with `cross` `EQUAL`
- the earlier regressions pass
- the design-doc pause example (theme.css from design §3.1) renders, and its screenshot is sent to the owner

## Implementation notes

**What exists.** Painting: `Render/UiBoxPainter` (box, shadows, gradients, border-image, outline, filter matrix, mask, clip-path, blend, pixelated), `UiGradientRamps` (256x256 ramp texture, LRU rows), `UiImageAtlas` (2048 pages, skyline packing, 4 px padding, CPU mips), `UiDrawList` with per-item geometry and batch merging (`merged N` in `ui_stats`), shaders `fs_ui2.sc`, `fs_ui2_shadow.sc`, `ui2_common.sh`. Style: `UiFxStyle`, `UiFxParse`, `UiLength`, `@keyframes` in `UiCssParser`. Motion: `Style/UiAnimation` (transitions, keyframe animations, transform interpolation, reduced motion, pause and seek), transforms in `UiFlowLayout` (`PatchTransforms`), leaving pages in `UiMenuStack`. New element `Arc`. UIDB: registries `paintEffects`, `animations`, `easings`; capabilities `painting` and `motion` true, `layers` false; previews `engine.effects.element` and `engine.animation.timeline`.

**Deviations from the plan.**
- Shaders return premultiplied colour; the colour matrix un-premultiplies, applies, and premultiplies again in `uiFinish`.
- Arc angles are in T13, not T12. The clip shape lives in aux (T15.w). The gradient header is (type, ramp row, period, start).
- Outer shadows are knocked out under the element box (CSS-correct).
- `image-rendering: pixelated` is done in the shader (nearest via `floor(uv*size)+0.5`), not through a sampler flag, because `Shader::SetTexture` has no sampler flags.
- Image minification differs slightly from phase 2 because images now go through the atlas.

**Known limits.** Gradient `background-size` tiling is at most 64 tiles (per-tile clips). An ellipse clip-path is inaccurate under rotation. `border-image` repeat supports stretch and round only. There is no `background` shorthand. `background-size` is not animatable. `blur()`, `drop-shadow()`, `backdrop-filter` and group opacity need offscreen layers and are listed unavailable (`requiresLayer`).

**Shader rules kept.** The anti-aliasing width comes from the CPU (T2.z); implicit-LOD `texture2D` only at the top of `main`, branches use `texture2DLod`; `SAMPLER2D` declarations are in each `.sc` file because the engine parses the slots from the `.sc` text.

**Test commands added.** `anim_pause`, `anim_seek <id> <s>`, console `ui_reduced_motion`, `expect` props `opacity_now` and `animating`, `expect_uidb` keys `nocap:`, `unavailable:`, `preview:`. Scripts `Scripts/Common/p3_{paint,text_fx,motion,pages,design,uidb,timeline}.uitest` plus `p3_paint.probes`; pages in `ui/kit/tests/p3_*.xml`; `p3_design_theme.css` is the design doc 3.1 theme verbatim.

**Diagnostics.** `ui_stats` also prints the CPU time of UI update, finalize and submit in microseconds. In Release the whole UI costs about 10-20 us for update plus finalize and 30-120 us for GPU submit per frame, also while the pause menu animates.

**Pause menu.** Reworked beyond the plan at the owner's request: text-only items on the left, retro look, hover/focus slide, glow and a red underline filling left to right, all in `theme.css` (transitions on `transform`, `color`, `text-shadow`; the underline is a `scaleX` bar). Test runs (`-ui_test`) skip the network connect and host socket in `GameStart.cpp`; `NetworkManager` still runs as a lone server on a socket-less transport, because the player spawns through it.

**Results (StealthAction, run `fin`/`m4`).** All earlier regression scripts and all seven `p3_*` scripts PASS on dx11, dx12, vk, gl (85 of 85 runs in `m4`). `p3_paint` probes OK. Cross vs dx11: dx12 EQUAL everywhere; vk EQUAL for paint, text fx and timeline (design frames differ by 123 px, max 10, legacy HUD text; `p3_design_enter` is mid-animation); gl differs by the same 666 px (max 82) in every capture, including ones with no new UI, so it is the legacy HUD, as in phase 1. `p3_motion_mid` is captured mid-transition on real time and is not compared across backends.
Shaders for ShooterGame, SimGame and SlasherGame compile with 0 failed (not run).
