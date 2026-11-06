# Phase 1: markup core

**Goal:** load `.xml` layouts and `.css` styles into a tree of new `UiElem` elements, resolve styles with a CSS cascade, lay them out with Panorama-style flow layout, draw them through the phase 0 painter (boxes, images and SDF text), and make buttons focusable and activatable through the existing navigation. It includes components (layouts that include each other), snippets, imports, variables, localisation and hot reload. It also establishes the UIDB registry/publication foundation early: feature registries publish metadata as they are implemented, and the first complete `uidb.json` is generated on normal game launch when `UiSystem` initialization is complete. Every behaviour is proven by `.uitest` scripts on all backends.

**Prerequisites:** phase 0 complete; `00_CONVENTIONS.md` read again. Phase 0's `run_ui_tests.py`, `ui_image_probe.py`, `UiPainter` and `UiDrawList` are used throughout.

**Out of scope for phase 1:**

- markup events and actions (`onactivate="…"`), the menu stack, prompts (phase 2)
- gradients, shadows, background images on boxes, transitions, transforms, the image atlas (phase 3)
- scrolling, wrapped widgets, `Frame` (phase 4)
- HUD layers, world surfaces, `UiContext` (phase 5)

---

## Decisions specific to this phase

| # | Decision |
|---|---|
| P1-D1 | **Box sizing is always border-box.** `width` and `height` include padding and border. There is no `box-sizing` property. |
| P1-D2 | **Overflow defaults to `visible`.** Supported values in phase 1: `visible`, `clip`. (`scroll` comes in phase 4.) Panorama's `squish` isn't supported. |
| P1-D3 | `:root` matches the top element of every document instance: the loaded file's top element and every component instance's top element. |
| P1-D4 | The root font size is **24px**. `rem` resolves against it (phase 4 makes it follow the text-size setting). |
| P1-D5 | `white-space: normal` collapses runs of spaces and tabs but **keeps `\n` as a line break**. This deviates from CSS on purpose: game strings use `\n`. |
| P1-D6 | Fonts are mapped with `@font-face`. The family `default` is built in and maps to `GameData/fonts/NotoSans.ttf`, which every in-scope game has. |
| P1-D7 | Markup elements may live only under `EngineMain::Viewport` in phase 1. A root found elsewhere logs a warning once and isn't updated. Phase 5 lifts this. |
| P1-D8 | Each root is re-resolved, re-laid-out and re-recorded **as a whole** when anything in it is dirty. Per-element caching is a later optimisation. Correctness first. |

---

## Files of this phase

```
source/Engine/includedLibraries/pugixml/pugixml.hpp, pugiconfig.hpp, pugixml.cpp   (vendored, v1.14)
source/Engine/UI/Markup/UiMarkupLog.h/.cpp           error/warning formatting and counters
source/Engine/UI/Markup/UiVariables.h/.cpp           {var} substitution, formatting
source/Engine/UI/Markup/UiDocument.h/.cpp            parsed .xml definitions + cache
source/Engine/UI/Markup/UiComponentRegistry.h/.cpp   global component tags, discovery, imports
source/Engine/UI/Markup/UiTagRegistry.h/.cpp         tag -> element factory (built-in + legacy + code-behind)
source/Engine/UI/Preview/UiPreviewRegistry.h/.cpp   typed preview descriptors/providers; introduced early, populated as features arrive
source/Engine/UI/Preview/UiDatabase.h/.cpp           UIDB model, registry publication, deterministic atomic serializer (`SaveData/UIStudio/uidb.json`)
source/Engine/UI/Markup/UiBuilder.h/.cpp             instantiates element trees from definitions
source/Engine/UI/Style/UiCssTokenizer.h/.cpp
source/Engine/UI/Style/UiCssParser.h/.cpp            stylesheet AST
source/Engine/UI/Style/UiSelector.h/.cpp             selector parse + match + specificity
source/Engine/UI/Style/UiStyleSheetCache.h/.cpp      parsed .css files by path
source/Engine/UI/Style/UiComputedStyle.h/.cpp        the computed style struct + value parsers
source/Engine/UI/Style/UiStyleResolver.h/.cpp        cascade
source/Engine/UI/Render/UiFontLibrary.h/.cpp         SDF glyph atlas, faces, @font-face registry
source/Engine/UI/Render/UiTextLayout.h/.cpp          shaping (kerning), wrapping, ellipsis, shrink
source/Engine/UI/Flow/UiFlowLayout.h/.cpp            measure + arrange
source/Engine/UI/Elements/UiElem.h/.cpp              base element
source/Engine/UI/Elements/UiElemPanel.h/.cpp
source/Engine/UI/Elements/UiElemLabel.h/.cpp
source/Engine/UI/Elements/UiElemImage.h/.cpp
source/Engine/UI/Elements/UiElemButton.h/.cpp        Button and TextButton
source/Engine/UI/UiSystem.h/.cpp                     per-frame driver, roots, hot reload, console commands
Tools/ui/assets/GameData/ui/kit/tests/       test layouts (synced to every game)
Tools/ui/scripts/Common/p1_*.uitest     test scripts (game-independent)
```

Edited: `UiDrawList` (prim modes), `UiPainter` (glyph runs), `fs_ui2.sc` (SDF glyph kind), `UiRenderer.cpp` (fallback fonts forward), `EngineMain.cpp` (UiSystem hooks), `UiTestRunner` (new commands), `ConsoleDefaultCommands.cpp`.

---

## Task 1.1: vendor pugixml

1. Ask the owner for permission to download (conventions: downloads need approval). The files are `pugixml.hpp`, `pugiconfig.hpp` and `pugixml.cpp` from tag **v1.14**:
   - `https://raw.githubusercontent.com/zeux/pugixml/v1.14/src/pugixml.hpp`
   - `https://raw.githubusercontent.com/zeux/pugixml/v1.14/src/pugiconfig.hpp`
   - `https://raw.githubusercontent.com/zeux/pugixml/v1.14/src/pugixml.cpp`
2. Put them in `source/Engine/includedLibraries/pugixml/`. Don't modify them.
3. Add the three to `Engine.vcxproj` and re-run configure.
4. Include them as `#include <includedLibraries/pugixml/pugixml.hpp>`.

**Check 1.1:** add a temporary call in `UiSystem::Init` (task 1.14) that parses `<a b="1"/>` and logs the attribute. It builds and logs `1`. Then remove the call.

---

## Task 1.2: markup log

`UiMarkupLog.h`:

```cpp
#pragma once
#include <string>

namespace UiMarkupLog
{
    // "[UI] <path>:<line>:<col>: error: <message>" (conventions §9). line/col 0 when unknown.
    void Error(const std::string& path, int line, int col, const std::string& message);
    void Warning(const std::string& path, int line, int col, const std::string& message);
    int ErrorCount();     // since process start
    int WarningCount();
    // 1-based line and column of a byte offset in `text`.
    void LineColumn(const std::string& text, size_t offset, int& line, int& col);
}
```

`Error` and `Warning` call `Logger::Error` / `Logger::Warning` with the exact format, and increment a counter. `LineColumn` counts `\n` before the offset; the column is the byte count since the last `\n`, plus 1.

**Check 1.2:** it builds.

---


## Task 1.2a: UIDB foundation (do this early, before markup registries)

**Purpose:** establish the metadata contract before implementing tags, CSS properties, components and previewable elements. Later tasks must publish into this system as they create their registries; do not bolt UIDB on after the UI runtime is complete.

**Files:**

- `source/Engine/UI/Preview/UiDatabase.h/.cpp`
- `source/Engine/UI/Preview/UiPreviewRegistry.h/.cpp` (descriptor/API skeleton; concrete recipes are added when their runtime types exist)
- engine startup/build registration needed for the UIDB bootstrap, limited to the files already permitted by this phase

### `UiDatabase` foundation

Create the single owner of UIDB state and serialization. It must provide:

- the versioned top-level envelope and typed structures from conventions §13;
- stable game identity, engine/build/protocol identity and GameData/UI roots;
- capability flags that default to false until the corresponding runtime feature is actually registered;
- typed publication/removal/update methods for registry entries and preview descriptors (or a small typed registry API with equivalent responsibilities);
- `MarkDirty(reason)` / registry revision tracking; reasons are diagnostic only and must not introduce nondeterministic JSON fields;
- deterministic ordering and stable IDs;
- UTF-8 JSON serialization, temporary-file write and atomic rename to `SaveData/UIStudio/uidb.json`;
- path normalization and validation: database paths are GameData-relative, use `/`, preserve case and never contain absolute machine paths or traversal;
- a non-fatal write-failure path that logs a warning but leaves the in-memory metadata available to the runtime bridge;
- an explicit registry-completeness state internally. Publish a snapshot only after every registry expected in the current build/startup stage has finished registering; capabilities absent from that build remain false and their registries remain empty.

Do not duplicate the source registries. `UiDatabase` stores serializable metadata snapshots/handles only; XML/CSS, game code, parser registries, localisation files and preview providers remain authoritative. Do not serialize callbacks, object pointers, player state, translation text, current settings or runtime entity IDs.

### `UiPreviewRegistry` foundation

Define the stable preview descriptor and provider registration interfaces now, even though some host kinds arrive in later phases. The descriptor contract includes stable ID, label/category/description, kind, source refs, host/capability requirements, typed parameters, scenarios and provider version. Provider callbacks remain compiled C++ code and are never serialized. Keep host-specific implementation capability-gated until its phase is complete.

### Bootstrap and publication rules

- Initialize the UIDB service once the game root, GameData root, stable game ID and engine build identity are known. This creates the in-memory model and serializer early; it must not require `UiSystem`, Qt or Studio.
- Hook `UiDatabase::InitBootstrap()` into the existing `EngineMain::Init` sequence after `UiElementRegistry::RegisterBuiltinTypes()` and `Localisation::Initialize(...)`. It writes a valid baseline snapshot from the registries actually available in the current build (including legacy/reflection UI metadata where enumerable); categories whose registries do not exist yet are empty and their capabilities remain false. Do not invent future-phase entries.
- When Task 1.14 adds `UiSystem` and the new markup registries, it calls `UiDatabase::RefreshFromRegistries()` after registration and component discovery, atomically replacing the baseline with the complete Phase 1 snapshot before accepting a Studio connection.
- As Tasks 1.6–1.13 create CSS, tag, component, document and preview registries, each registry publishes its descriptor through the common API and calls `MarkDirty()` when its contents change. The first complete snapshot includes every Phase 1 feature that is available at that point.
- Later phases add their own descriptors in their own implementation tasks. They must never create `uidb_p2.json`, `studio_schema.json` or another sidecar.

**Check 1.2a:** build the early UIDB service without depending on Qt. Launch the game and confirm a valid baseline `SaveData/UIStudio/uidb.json` is emitted from registrations available at this point. Unit-test the serializer using a minimal game identity and empty registries: parse its output with Python's `json` module, verify stable ordering/content across repeated serializations, verify path validation rejects traversal/absolute paths, and simulate an unwritable destination without failing startup. Verify `UiPreviewRegistry` can register/enumerate a test descriptor without serializing a callback or pointer.

---
## Task 1.3: draw list primitive modes and painter text support

### 1.3a: `UiDrawList` local primitives

Element chunks are recorded during `UiSystem::Update` and appended during rendering. Primitive indices into the frame-wide `UiPrimBuffer` aren't known when recording, so a chunk list stores its own primitive blocks.

Add to `UiDrawList`:

```cpp
// Lists are "global" by default (vertex Prim = index in UiGpu::Prims(), as phase 0 does).
// A local list stores its own blocks; vertex Prim = index into LocalPrims().
void SetLocalPrims(bool local) { m_local = local; }
bool IsLocal() const { return m_local; }
// Adds a block: local lists append to m_localPrims and return that index; global lists call UiGpu::Prims().Add.
int AddPrim(const UiRender::PrimBlock& block);
const std::vector<UiRender::PrimBlock>& LocalPrims() const { return m_localPrims; }
// Appends a LOCAL list into this GLOBAL list: adds every local block to UiGpu::Prims(), rewrites vertex Prim
// fields, and raises every item's StencilRef to at least `minStencilRef`. Items from `firstItem` to `lastItem`
// (exclusive) only; use 0 / Items().size() for all.
void AppendResolved(const UiDrawList& local, size_t firstItem, size_t lastItem, uint8_t minStencilRef);
```

Rules:

- `AppendResolved` adds **all** local blocks referenced by the appended items. To keep it simple: on the first call for a given source list in a pass, add all of its blocks and remember the base index. Store `m_resolvedBase` and `m_resolvedFrom` (a pointer to the source list) on the **destination** list, cleared by `Clear()`. Later calls from the same source reuse the base.
- `Clear()` also clears local prims.
- `UiPainter` calls `list.AddPrim(block)` instead of `UiGpu::Prims().Add(block)`. Phase 0 legacy routing keeps calling `UiGpu::Prims().Add` on the global list. Both are fine.

### 1.3b: SDF glyph kind in the shader

In `fs_ui2.sc` add, before `main`:

```
vec4 uiShadeSdfGlyph(int prim, vec4 tex, float px)
{
    vec4 color = uiFetch(prim, 1);
    vec4 g = uiFetch(prim, 13);        // x = UI units per atlas pixel (glyph scale), y = dilation in UI units
    // Atlas encodes distance as 128 + 16 * (distance in atlas pixels), inside > 128 (stbtt_GetCodepointSDF settings).
    float d = (tex.r * 255.0 - 128.0) / 16.0 * g.x + g.y;
    float cov = clamp(0.5 + d / px, 0.0, 1.0);
    return uiPremul(color) * cov;
}
```

In `main`, replace the `// [P1] kind 2 …` comment line with:

```
    else if (kind < 2.5)
        color = uiShadeSdfGlyph(prim, tex, px);
    // [P3] kinds 3..5 go here.
```

Remove the `// [P1] SDF glyph atlas sampler…` and `// [P1] sample the glyph atlas here…` comment lines. The glyph atlas page is the **batch texture** (`s_uiTex0`), so no new sampler is needed.

Sync, then compile all four games' shaders. There must be no `[FAILED]`.

### 1.3c: `UiPainter::GlyphRun`

Add to `UiPainter`:

```cpp
struct UiGlyphQuad { float X, Y, W, H; float U0, V0, U1, V1; uint16_t Page; };
// Draws SDF glyph quads (positions element-local). One primitive block per call per atlas page.
void GlyphRun(const UiGlyphQuad* quads, size_t count, const glm::vec4& color, float glyphScale, float dilation);
```

Rules:

- Group the quads by `Page`. For each page, add one block:
  - `kind = SdfGlyph`
  - `T[1] = color`
  - `T[2] = (0, 0, aa, 0)`, with `aa` from the transform as in `Box`
  - `T[13] = (glyphScale, dilation, 0, 0)`
  - the clip applied
- Each quad: four vertices transformed by the painter transform, local coordinates = the quad's own corners, UVs from the quad.
- Key: `StateLegacyPremultiplied()`, texture = `UiFontLibrary::PageTexture(page)`, stencil = the current mask depth.
- **Snapping:** when the transform is translation-only, the caller snaps the run origin (task 1.11). `GlyphRun` itself doesn't snap glyphs.

**Check 1.3:** it builds, and the shaders compile everywhere.

---

## Task 1.4: SDF font library

`UiFontLibrary.h`:

```cpp
#pragma once
#include <bgfx/bgfx.h>
#include <cstdint>
#include <string>
#include <vector>

struct UiGlyph
{
    bool Valid = false;          // false: no bitmap (space) or missing
    uint16_t Page = 0;
    float U0 = 0, V0 = 0, U1 = 0, V1 = 0;
    float Width = 0, Height = 0;    // bitmap size in atlas pixels (includes the 8 px SDF padding)
    float OffsetX = 0, OffsetY = 0; // bitmap top-left relative to pen/baseline, atlas pixels
    float Advance = 0;              // atlas pixels
    int FaceIndex = 0;              // face the glyph came from (fallbacks differ)
};

struct UiFaceMetrics { float Ascent, Descent, LineGap; }; // atlas pixels at kBaseSize, Descent negative

class UiFontLibrary
{
public:
    static constexpr float kBaseSize = 48.f;    // glyphs are generated at 48 px per em
    static constexpr int   kPadding = 8;        // SDF spread in atlas pixels
    static constexpr int   kPageSize = 2048;
    static constexpr int   kMaxPages = 4;

    static void Init();
    static void Shutdown();

    // @font-face registration. Returns false and logs if the file can't be read or parsed.
    static bool RegisterFace(const std::string& family, int weight, bool italic, const std::string& path);
    static void AddFallback(const std::string& path);   // also called from UiRenderer::AddFallbackFont
    // Picks the closest face: exact family (case-insensitive), then nearest weight, then italic match.
    // Unknown family falls back to "default". Sets `syntheticBold`/`syntheticItalic` when no matching face exists.
    static int ResolveFace(const std::string& family, int weight, bool italic, bool& syntheticBold, bool& syntheticItalic);

    static const UiFaceMetrics& Metrics(int face);
    static const UiGlyph& Glyph(int face, uint32_t codepoint);     // generates on demand
    static float Kerning(int face, uint32_t left, uint32_t right); // atlas pixels; 0 across faces
    static bgfx::TextureHandle PageTexture(uint16_t page);
    // Uploads dirty atlas regions. Called by UiSystem at the end of Update (main thread).
    static void FlushUploads();
};
```

Implementation rules:

- **Faces:** load each `.ttf` with `FileSystemEngine::ReadFileBinary` and keep the bytes alive for the face's lifetime. Parse with `stbtt_InitFont(&info, data, stbtt_GetFontOffsetForIndex(data, 0))`. Scale: `stbtt_ScaleForMappingEmToPixels(&info, kBaseSize)`. Metrics from `stbtt_GetFontVMetrics` × scale.
- `Init` registers `("default", 400, false, "GameData/fonts/NotoSans.ttf")`.
- **Glyph lookup** for `(face, cp)`:
  1. Cache key: `face * 0x110000 + cp` in an `unordered_map<uint64_t, UiGlyph>`.
  2. If `stbtt_FindGlyphIndex(&face.info, cp) == 0`, walk the fallbacks in order. The first fallback face with a non-zero index supplies the glyph, and `FaceIndex` = that fallback's face index (fallbacks are faces too, registered under family `"__fallback<N>"`).
  3. If nothing covers it, use the primary face's index 0 (the tofu box).
- **Generation:**
  - `stbtt_GetGlyphSDF(&info, scale, glyphIndex, kPadding, 128, 16.0f, &w, &h, &xoff, &yoff)`
  - advance from `stbtt_GetGlyphHMetrics` × scale
  - when the bitmap is null (space), `Valid = false` but `Advance` is still set
  - free the bitmap with `stbtt_FreeSDF`
- **Packing:** shelf packer per page, 1 px gap between glyphs.
  - Place left to right on the current shelf.
  - Start a new shelf when the glyph doesn't fit the width.
  - Start a new page when the shelf doesn't fit the height.
  - More than `kMaxPages` pages: log once and return an invalid glyph (with advance).
- **Pages:** R8 textures, `kPageSize`², `BGFX_SAMPLER_UVW_CLAMP` (bilinear), no mips. Each has a CPU mirror and a dirty rect. `FlushUploads` calls `bgfx::updateTexture2D` with `bgfx::copy` of the dirty rows' full width, then clears the dirty rect.
- UVs: `U0 = x / kPageSize`, `V0 = y / kPageSize`, `U1 = (x + w) / kPageSize`, `V1 = (y + h) / kPageSize`.
- Threading: glyph generation and uploads run on the main thread only (layout in `Update`, uploads in `Finalize`). `RegisterFace` (from a stylesheet parsed by game-thread code) is CPU-only and is allowed on the game thread (R3).
- In `UiRenderer::AddFallbackFont`, after its existing work, call `UiFontLibrary::AddFallback(path)`.

**Check 1.4:** a temporary console command `ui_font_test` generates the glyphs of "Ag" and logs their metrics. Then add the `ui_screenshot` of a test in task 1.17. Remove the command.

---

## Task 1.5: text layout

`UiTextLayout.h`:

```cpp
#pragma once
#include <UI/Render/UiPainter.h>
#include <string>
#include <vector>

enum class UiWhiteSpace { Normal, NoWrap, Pre };
enum class UiTextAlign { Left, Center, Right };
enum class UiTextOverflow { Clip, Ellipsis, Shrink };
enum class UiTextTransform { None, Uppercase, Lowercase };

struct UiTextParams
{
    std::string Text;             // already localised and variable-substituted
    std::string Family = "default";
    int Weight = 400;
    bool Italic = false;
    float FontSize = 24.f;        // UI units per em
    float LetterSpacing = 0.f;    // UI units
    float LineHeight = -1.f;      // UI units; < 0 = normal
    float WrapWidth = -1.f;       // UI units; < 0 = no wrapping
    float MaxHeight = -1.f;       // for Shrink; < 0 = unlimited
    UiWhiteSpace WhiteSpace = UiWhiteSpace::Normal;
    UiTextAlign Align = UiTextAlign::Left;
    UiTextOverflow Overflow = UiTextOverflow::Clip;
    UiTextTransform Transform = UiTextTransform::None;
    bool operator==(const UiTextParams&) const = default;
};

struct UiTextLayoutResult
{
    std::vector<UiGlyphQuad> Quads;   // positions relative to the text box top-left
    float Width = 0.f, Height = 0.f;  // measured box
    int Lines = 0;
    float UsedFontSize = 0.f;         // differs from FontSize after Shrink
    float GlyphScale = 0.f;           // UsedFontSize / kBaseSize
    float Dilation = 0.f;             // synthetic bold, UI units
};

// Pure function of params. Callers cache by comparing params.
UiTextLayoutResult UiLayoutText(const UiTextParams& params);
```

Algorithm, exactly:

1. **Decode** UTF-8 into code points. Invalid bytes become U+FFFD.
2. **Transform:**
   - `Uppercase`: map `a–z` → `A–Z`, U+00E0–U+00FE (except U+00F7) → minus 0x20, U+0430–U+044F → minus 0x20, U+0451–U+045F → minus 0x50.
   - `Lowercase` is the reverse of those ranges.
   - Everything else is unchanged.
3. **White space:**
   - `Normal` and `NoWrap`: tabs become spaces, and runs of spaces collapse to one. Leading and trailing spaces of each line are dropped after wrapping.
   - `Pre`: spaces stay, and a tab becomes 4 spaces.
   - `\r` is removed. `\n` is always a hard break (P1-D5).
4. **Face:** `ResolveFace(...)`. `scale = FontSize / kBaseSize` (UI units per atlas pixel). Synthetic bold: `Dilation = 0.025 * FontSize`. Synthetic italic: quads are sheared when emitted (step 9).
5. **Line height:** normal = `(Ascent − Descent + LineGap) × scale`; otherwise the given value. Baseline within a line = `halfLeading + Ascent × scale`, where `halfLeading = (lineHeight − (Ascent − Descent) × scale) / 2`.
6. **Advances:** pen advance per code point = `Glyph.Advance × scale + LetterSpacing`, plus `Kerning(prev, cp) × scale` when both glyphs come from the same face.
7. **Wrapping** (only when `WrapWidth >= 0` and `WhiteSpace == Normal`):
   - Break opportunities: after a space, after `-`, and between two code points when either is CJK (U+3040–U+30FF, U+3400–U+4DBF, U+4E00–U+9FFF, U+F900–U+FAFF, U+FF00–U+FFEF).
   - Greedy: add words while the line width (without trailing spaces) ≤ `WrapWidth`. A single word wider than `WrapWidth` is broken at the last code point that fits, with at least one code point per line.
8. **Ellipsis** (`Overflow == Ellipsis`, applied to the **last** line, which is the only line when not wrapping): when the line width > `WrapWidth` (or, without wrapping, when a `WrapWidth` constraint was given as the box width), drop code points from the end until `width + ellipsisWidth ≤ limit`, then append U+2026. If the face lacks U+2026 (its glyph index is 0), use three `.`.
9. **Emit quads:**
   - for each visible glyph: `X = penX + OffsetX × scale`, `Y = lineTop + baseline + OffsetY × scale`, `W = Width × scale`, `H = Height × scale`, UVs from the glyph
   - italic shear: shift the top edge right by `0.2 × H` and the bottom edge by 0. Store it as a per-quad flag `Italic`, and have `GlyphRun` shear the two top vertices by `+0.2 × H` in local X
   - alignment per line: `Left` offset 0; `Center` `(boxWidth − lineWidth) / 2`; `Right` `boxWidth − lineWidth`. `boxWidth = WrapWidth` when ≥ 0, else the widest line.
10. **Result:** `Width` = widest line (or `WrapWidth` when ≥ 0 and some line wrapped), `Height = Lines × lineHeight`.
11. **Shrink** (`Overflow == Shrink`): if the result is wider than `WrapWidth` (when no wrapping) or taller than `MaxHeight`, retry with `FontSize × 0.95ⁿ` for n = 1…14 (down to about 49%). Use the first that fits, or the last attempt.

**Check 1.5:** covered by task 1.17 (`p1_text`).

---

## Task 1.6: CSS tokenizer

The tokenizer itself contributes no public runtime capability entry, but this is where the metadata contract starts being used: subsequent CSS grammar/property registration must publish the supported syntax into `UiDatabase` as it is introduced. Never infer support from a property name found in a source file.

`UiCssTokenizer.h`:

```cpp
enum class UiCssTok { Ident, Function, AtKeyword, Hash, String, Number, Percentage, Dimension,
                      Whitespace, Colon, Semicolon, Comma, LBrace, RBrace, LParen, RParen, LBracket, RBracket,
                      Delim, Url, BadString, Eof };
struct UiCssToken
{
    UiCssTok Type;
    std::string Text;     // ident/function name (without '('), at-keyword (without '@'), hash (without '#'), string contents, url contents, unit for Dimension, delim char
    double Number = 0;    // Number/Percentage/Dimension
    bool IsInteger = false;
    int Line = 1, Col = 1;
};
std::vector<UiCssToken> UiCssTokenize(const std::string& source);   // always ends with Eof
```

Rules (a subset of CSS Syntax Level 3):

- Comments `/* … */` are skipped. An unterminated comment runs to the end.
- Whitespace (space, tab, `\n`, `\r`, `\f`) runs become one `Whitespace` token.
- Strings: `"…"` or `'…'`, with `\` escaping the next character. A newline inside a string gives `BadString`.
- Numbers:
  - an optional sign, digits, an optional `.` followed by digits, an optional exponent (`e`/`E`, optional sign, digits)
  - followed by `%` → `Percentage`
  - followed by an identifier start → `Dimension`, with `Text` = the unit, lowercased
- Identifier: starts with a letter, `_`, `-` (only when followed by a letter, `_` or `-`), or non-ASCII. It continues with letters, digits, `_`, `-` and non-ASCII. Then:
  - `(` right after → `Function`
  - the identifier `url` followed by `(` → read up to `)` as `Url` (quotes inside are stripped)
- `#` followed by name characters → `Hash`.
- `@` followed by an identifier → `AtKeyword`.
- Single-character tokens: `: ; , { } ( ) [ ]`. Any other character is a `Delim`.
- `Line`/`Col` are the token start (1-based).

---

## Task 1.7: CSS parser (stylesheet AST)

`UiCssParser.h`:

```cpp
struct UiCssDeclaration
{
    std::string Name;                        // lowercased; custom properties keep their case after "--"
    std::vector<UiCssToken> Value;           // tokens, leading/trailing whitespace trimmed
    int Line = 0, Col = 0;
};
struct UiCssRule
{
    std::vector<UiSelector> Selectors;       // selector list
    std::vector<UiCssDeclaration> Declarations;
    int SourceOrder = 0;                     // increasing across a sheet
};
struct UiFontFaceRule { std::string Family; std::string Src; int Weight = 400; bool Italic = false; int Line = 0; };
struct UiStyleSheet
{
    std::string Path;                        // file path, or "<inline:file.xml:LINE>" for <style> blocks
    std::vector<UiCssRule> Rules;
    std::vector<std::string> Imports;        // @import targets, resolved before this sheet's own rules
    std::vector<UiFontFaceRule> FontFaces;
    // [P3] keyframes, [P4] media queries
};
std::shared_ptr<UiStyleSheet> UiParseStyleSheet(const std::string& source, const std::string& path, int lineOffset);
```

Grammar handled:

- **Top level:** a sequence of at-rules and qualified rules.
- **`@import`** `url(…)` or `"…"` `;` → `Imports`. Must come before any rule. Later imports give a warning and are ignored.
- **`@font-face { … }`** → declarations `font-family` (string or ident), `src: url(…)`, `font-weight` (normal = 400 | bold = 700 | number), `font-style` (normal | italic) → `FontFaces`.
- **Any other at-rule:** warning `unsupported at-rule '@x'`. Skip its block, or up to `;`.
- **Qualified rule:** prelude tokens up to `{` are parsed as a selector list (task 1.8). Then the declaration block:
  - `name : value ;`, where the value runs to `;` or `}` at nesting depth 0 (track `(`/`)` and `[`/`]` depth)
  - `!important` at the end: warning `!important is not supported`, and drop the `!important` tokens
  - empty declarations are skipped
- **Errors** (`file:line:col`): a bad selector skips the whole rule; a missing `:` skips that declaration; an unclosed block runs to the end of the file.
- `lineOffset` is added to every token line. It is used for `<style>` blocks inside XML, so errors point at the right line of the `.xml`.

`UiStyleSheetCache`:

```cpp
// Returns the parsed sheet for a .css path (cached), or nullptr after logging when the file is missing.
std::shared_ptr<UiStyleSheet> UiStyleSheetCache::Get(const std::string& path);
void UiStyleSheetCache::Invalidate(const std::string& path);   // hot reload
uint32_t UiStyleSheetCache::ModTime(const std::string& path);
```

After parsing, `@font-face` rules are registered with `UiFontLibrary::RegisterFace`. Font faces are global: re-registering the same family, weight and style with a different `src` gives a warning, and the first one wins.

---

## Task 1.8: selectors

`UiSelector.h`:

```cpp
enum class UiPseudo { Hover, Active, Focus, FocusWithin, Disabled, Selected, Root, Entering, Leaving, Checked };
struct UiCompound
{
    std::string Tag;                 // "" = any; "*" also stored as ""
    std::string Id;                  // "" = none
    std::vector<std::string> Classes;
    std::vector<UiPseudo> Pseudos;
};
enum class UiCombinator { Descendant, Child };
struct UiSelector
{
    std::vector<UiCompound> Parts;            // left to right
    std::vector<UiCombinator> Combinators;    // Parts.size() - 1 entries, Combinators[i] joins Parts[i] and Parts[i+1]
    int Specificity = 0;                      // ids * 10000 + (classes + pseudos) * 100 + tags
};
// Parses a selector list from prelude tokens. On error logs and returns false.
bool UiParseSelectorList(const std::vector<UiCssToken>& prelude, const std::string& path, std::vector<UiSelector>& out);
class UiElem;
bool UiSelectorMatches(const UiSelector& selector, const UiElem& element);
```

Parsing:

- A compound is a sequence of: type `Ident` or `*`, `Hash` (id), `.` + `Ident` (class), and `:` + `Ident` (pseudo).
- Whitespace between compounds is a descendant combinator. `>` (a `Delim`) is a child combinator.
- `,` separates selectors.
- Pseudo names: `hover active focus focus-within disabled selected root entering leaving checked`. An unknown pseudo-class logs a warning, and the **whole selector** is dropped (it never matches).
- These are errors: `+`, `~`, `[`, `::` and functional pseudo-classes. The rule is skipped.

Matching, right to left:

1. The last part must match the element.
2. For each combinator going left:
   - `Child`: the element's parent must match the next part
   - `Descendant`: walk up the ancestors until one matches the part; if none does, there is no match
3. Ancestors are the `UiElem` parent chain **only**. Stop at the first non-`UiElem` parent.

A compound matches element E when all of these hold:

- the tag is empty, or equals E's `TagName`, or equals E's `ComponentTag` (both case-sensitive)
- the id is empty or equals E's `Id`
- every class is in E's class list
- every pseudo holds:
  - `Hover`: E's `:hover` state
  - `Active`: `:active`
  - `Focus`: E is `UiNavigation::Focused`
  - `FocusWithin`: E or any descendant is focused
  - `Disabled` and `Selected`: the matching state flag
  - `Root`: E is a document-instance root (P1-D3)
  - `Entering`, `Leaving` and `Checked`: state flags, which are always false in phase 1

---

## Task 1.9: computed style

As computed-style properties and value grammars are registered, publish their canonical name, grammar, initial/inherited values and invalidation class through the UIDB CSS-property registry. Unknown/unsupported properties must not appear as supported completion entries.

`UiComputedStyle.h` holds the full list of phase 1 properties. Later phases add fields here **and** to the property table.

```cpp
#pragma once
#include <glm.h>
#include <string>
#include <UI/Render/UiTextLayout.h>

enum class UiLenKind { Px, Percent, Calc, FitChildren, FillParentFlow, HeightPercentage, WidthPercentage, Auto };
struct UiLength
{
    UiLenKind Kind = UiLenKind::Px;
    float Px = 0.f;        // Px, Calc (pixel part)
    float Percent = 0.f;   // Percent, Calc (percent part), Height/WidthPercentage (0..100)
    float Weight = 1.f;    // FillParentFlow
    float Resolve(float basis) const { return Px + Percent * 0.01f * basis; }   // Px, Percent, Calc only
};
enum class UiFlow { None, Down, Right, DownWrap, RightWrap };
enum class UiHAlign { Left, Center, Right };
enum class UiVAlign { Top, Center, Bottom };
enum class UiOverflow { Visible, Clip };
enum class UiVisibility { Visible, Hidden, Collapse };

struct UiComputedStyle
{
    // ── layout (changing these needs re-layout)
    UiLength Width  { UiLenKind::FitChildren };
    UiLength Height { UiLenKind::FitChildren };
    UiLength MinWidth { UiLenKind::Px }, MaxWidth { UiLenKind::Auto }, MinHeight { UiLenKind::Px }, MaxHeight { UiLenKind::Auto };
    UiFlow Flow = UiFlow::None;
    UiHAlign HAlign = UiHAlign::Left;
    UiVAlign VAlign = UiVAlign::Top;
    UiLength X, Y;                                      // offsets, % of parent content width / height
    UiLength MarginTop, MarginRight, MarginBottom, MarginLeft;
    UiLength PaddingTop, PaddingRight, PaddingBottom, PaddingLeft;
    UiLength Gap;
    UiOverflow Overflow = UiOverflow::Visible;
    UiVisibility Visibility = UiVisibility::Visible;    // inherited
    int ZIndex = 0;
    // ── box paint
    glm::vec4 BackgroundColor = glm::vec4(0.f);        // straight RGBA
    float BorderWidth[4] = { 0, 0, 0, 0 };              // top right bottom left, px
    glm::vec4 BorderColor[4] = { glm::vec4(0, 0, 0, 1), glm::vec4(0, 0, 0, 1), glm::vec4(0, 0, 0, 1), glm::vec4(0, 0, 0, 1) };
    bool BorderSolid[4] = { false, false, false, false }; // style none => width treated as 0
    UiLength Radius[4];                                  // top-left top-right bottom-right bottom-left (% of the box's smaller side)
    float Opacity = 1.f;
    // ── text (inherited unless noted)
    glm::vec4 Color = glm::vec4(1.f);
    std::string FontFamily = "default";
    float FontSize = 24.f;                               // resolved px
    int FontWeight = 400;
    bool FontItalic = false;
    float LetterSpacing = 0.f;
    float LineHeight = -1.f;                             // -1 normal, else px
    UiTextAlign TextAlign = UiTextAlign::Left;
    UiWhiteSpace WhiteSpace = UiWhiteSpace::Normal;
    UiTextOverflow TextOverflow = UiTextOverflow::Clip;  // NOT inherited
    UiTextTransform TextTransform = UiTextTransform::None;
    // ── image (Image element)
    enum class Fit { Fill, Contain, Cover, None, ScaleDown } ObjectFit = Fit::Fill;
    // ── custom properties (inherited): name (with "--") -> raw tokens
    std::unordered_map<std::string, std::vector<UiCssToken>> Custom;
};
```

### Property table

Every property: CSS name, value grammar, inherited, initial value, and what a change invalidates. Implement one parse function per value type, and a table `name → {parse fn, apply fn, inherited, invalidation}`.

| Property | Values | Inh. | Initial | Invalidates |
|---|---|---|---|---|
| `width`, `height` | `<length-percentage>` \| `fit-children` \| `fill-parent-flow(<number>)` \| `height-percentage(<percentage>)` (width only) \| `width-percentage(<percentage>)` (height only) \| `calc()` | no | `fit-children` | layout |
| `min-width` `min-height` | `<length-percentage>` | no | 0 | layout |
| `max-width` `max-height` | `<length-percentage>` \| `none` | no | `none` | layout |
| `flow-children` | `none` \| `down` \| `right` \| `down-wrap` \| `right-wrap` | no | `none` | layout |
| `horizontal-align` | `left` \| `center` \| `right` | no | `left` | layout |
| `vertical-align` | `top` \| `center` \| `bottom` | no | `top` | layout |
| `x` `y` | `<length-percentage>` | no | 0 | layout |
| `margin` (1–4 values), `margin-top/right/bottom/left` | `<length-percentage>` | no | 0 | layout |
| `padding` (1–4), `padding-*` | `<length-percentage>` | no | 0 | layout |
| `gap` | `<length>` | no | 0 | layout |
| `overflow` | `visible` \| `clip` | no | `visible` | layout + paint |
| `visibility` | `visible` \| `hidden` \| `collapse` | yes | `visible` | layout |
| `z-index` | `<integer>` | no | 0 | paint |
| `background-color` | `<color>` | no | `transparent` | paint |
| `border` | `<width> \|\| <style> \|\| <color>` (any order, each optional) | no | `0 none black` | layout + paint |
| `border-width` (1–4), `border-*-width` | `<length>` \| `thin`=1 \| `medium`=3 \| `thick`=5 | no | `medium` (counts only with style `solid`) | layout + paint |
| `border-style` (1–4), `border-*-style` | `none` \| `solid` (others: warning, treated as `solid`) | no | `none` | layout + paint |
| `border-color` (1–4), `border-*-color` | `<color>` | no | `currentcolor` = the element's `color` | paint |
| `border-top`, `border-right`, … | like `border`, one side | no | | layout + paint |
| `border-radius` (1–4), `border-top-left-radius` … | `<length-percentage>` | no | 0 | paint |
| `opacity` | `<number>` (0..1, clamped) \| `<percentage>` | no | 1 | paint |
| `color` | `<color>` | yes | white | paint |
| `font-family` | ident or string, comma list (first known family wins) | yes | `default` | layout |
| `font-size` | `<length>` (em = parent's font size, % of parent's font size) | yes | 24px | layout |
| `font-weight` | `normal`=400 \| `bold`=700 \| 100..900 | yes | 400 | layout |
| `font-style` | `normal` \| `italic` | yes | normal | layout |
| `letter-spacing` | `normal`=0 \| `<length>` | yes | 0 | layout |
| `line-height` | `normal` \| `<number>` (× font-size) \| `<length>` | yes | normal | layout |
| `text-align` | `left` \| `center` \| `right` | yes | left | paint |
| `white-space` | `normal` \| `nowrap` \| `pre` | yes | normal | layout |
| `text-overflow` | `clip` \| `ellipsis` \| `shrink` | **no** | clip | layout |
| `text-transform` | `none` \| `uppercase` \| `lowercase` | yes | none | layout |
| `object-fit` | `fill` \| `contain` \| `cover` \| `none` \| `scale-down` | no | fill | paint |
| `--*` custom | any tokens | yes | – | style |

Value details:

- **Lengths:**
  - `px` = UI units; a unitless `0` is allowed
  - `em` = this element's font size (for `font-size` itself: the parent's font size)
  - `rem` = the root font size (P1-D4)
  - `vw` / `vh` = 1% of `EngineMain::Viewport.GetSize()` x / y
  - `%` is kept as a percentage and resolved at layout
- **`calc()`:** supports `+` and `-` between length-percentages, and `*` and `/` by a number. The result must reduce to `px + percent`, stored as `Calc`. Anything else is an error, and the declaration is dropped.
- **Colours:** `#rgb` `#rgba` `#rrggbb` `#rrggbbaa`, `rgb(r, g, b)` and `rgba(r, g, b, a)` (comma or space separated; channels as 0..255 or %, alpha as 0..1 or %), `hsl()`/`hsla()`, `transparent`, `currentcolor`, and these names: black white red green blue yellow cyan magenta gray grey orange purple silver maroon olive lime teal navy aqua fuchsia. `green` = `#008000` and `lime` = `#00ff00`, as in CSS.
- **Shorthands** expand exactly like CSS:
  - 1 value → all 4 sides
  - 2 values → top/bottom, right/left
  - 3 values → top, right/left, bottom
  - 4 values → top, right, bottom, left
  - `border-radius` in corner order: TL, TR, BR, BL (1: all; 2: TL+BR, TR+BL; 3: TL, TR+BL, BR; 4: TL TR BR BL)
- **`var(--name)` / `var(--name, fallback)`:** substitute tokens before parsing the property value. Look the name up in the element's computed `Custom` (its own, else inherited). Missing without a fallback makes the value invalid. Substitution repeats until no `var(` remains, at most 8 times. More than 8 is a cycle error.

---

## Task 1.10: cascade (`UiStyleResolver`)

### Which sheets apply to an element

Each element E has `ScopeFile`: the `.xml` file whose definition created it. Slotted children keep the **using** file (task 1.12).

For a root (a document instance loaded by `UiElem::Load` or `LoadLayout`), the resolver collects:

1. **Component layer:** for every component file used anywhere inside the root, that file's sheets in document order, including `@import`s (imports first, in order) and `<style>` blocks in place. These rules only apply to elements with `ScopeFile ==` that component's file.
2. **Document layer:** the root document's own sheets, same ordering. These apply to every element in the root's tree, including inside component instances.

### Algorithm for element E

1. Gather every rule whose selector matches E (if a rule has several matching selectors, use the one with the highest specificity), respecting the scope rule above.
2. Sort the matches by `(layer, specificity, sheet order, rule order)` ascending. The component layer comes before the document layer. Sheet order is the order sheets were collected. Rule order is `SourceOrder`.
3. Walk them in order and put each declaration into `specified[name] = declaration`. Later entries overwrite earlier ones. Expand shorthands into longhands at this point (the shorthand's position in the order counts).
4. Then the `style="…"` attribute: parse it as a declaration list with the same parser (wrap it as `x{…}`), and apply it over the result.
5. Then C++ overrides (`E.Style()`, task 1.13): applied last, already typed.
6. **Compute** in this order:
   1. custom properties (inherit the parent's map, then overlay E's)
   2. `font-size` (needs the parent's)
   3. everything else
   - For each property: if specified, substitute `var()`, parse, and on success store the value. On failure log a warning at the declaration's position and fall back to inherited (inherited props) or initial.
   - If not specified: inherited props copy the parent's computed value; others take the initial value.
7. A root's parent style is the initial style.

Resolution runs on the whole root when it is style-dirty (P1-D8). Cache matched rules per element; that cache is invalidated with the style flag.

---

## Task 1.11: elements

Every built-in and legacy tag registration must publish a UIDB tag descriptor at registration time: stable tag ID/name, category, source/documentation reference where available, attributes, types/defaults/ranges, required flags and capability requirements. Registration calls `UiDatabase::MarkDirty()`; a startup snapshot is written once registries are complete in Task 1.14.

### `UiElem` (base)

```cpp
#pragma once
#include <UI/UiElement.h>
#include <UI/Style/UiComputedStyle.h>
#include <UI/Render/UiDrawList.h>
#include <UI/Render/UiPainter.h>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

class UiDocumentInstance;
struct UiDocumentDef;

enum UiElemState : uint32_t { kStateHover = 1, kStateActive = 2, kStateDisabled = 4, kStateSelected = 8, kStateEntering = 16, kStateLeaving = 32, kStateChecked = 64 };

class UiElem : public UiElement
{
public:
    UiElem();
    ~UiElem() override;

    // ── identity
    std::string TagName;                 // "Panel", "Label", ... (set by the factory)
    std::string ComponentTag;            // set on a component instance's top element
    std::string Id;
    std::string ScopeFile;               // .xml that created this element
    UiDocumentInstance* Instance = nullptr;   // id scope this element belongs to

    // ── API usable from code: main thread or game thread (R3). Calls only change state and set dirty flags.
    static std::shared_ptr<UiElem> Load(const std::string& path);   // creates a root from a layout file
    bool LoadLayout(const std::string& path);                       // applies a layout file onto this element (code-behind)
    template <class T = UiElem> T* Find(const std::string& idPath) const;    // "id" or "a/b" path; logs on miss
    void SetVar(const std::string& name, const std::string& value);
    void SetVar(const std::string& name, double value);
    void SetClass(const std::string& cls, bool on);
    void AddClass(const std::string& cls) { SetClass(cls, true); }
    void RemoveClass(const std::string& cls) { SetClass(cls, false); }
    void ToggleClass(const std::string& cls) { SetClass(cls, !HasClass(cls)); }
    bool HasClass(const std::string& cls) const;
    void SetAttribute(const std::string& name, const std::string& value);  // re-applies, marks dirty
    UiComputedStyle& Style();            // C++ overrides; marks style-dirty (see 1.13)
    const UiComputedStyle& Computed() const { return m_computed; }

    // ── results of layout (valid after UiSystem::Update)
    UiRect BorderRect;                   // relative to the parent's border-box top-left
    UiRect ContentRectLocal() const;     // inside BorderRect, relative to BorderRect
    UiRect ContentRectScreen() const;    // UI units (axis-aligned bound)
    glm::mat3 AbsoluteTransform = glm::mat3(1.f);   // element-local -> UI units

    // ── subclass hooks
    virtual bool ApplyAttribute(const std::string& name, const std::string& value) { return false; } // true = handled
    virtual glm::vec2 MeasureContent(float availableWidth, float availableHeight) { return glm::vec2(0.f); }
    virtual void PaintContent(UiPainter& painter, const UiRect& contentRect) {}
    virtual void OnActivate() {}         // phase 2 turns this into the "activate" event
    virtual void Tick() {}               // per frame, main thread, before style/layout (used from phase 5)
    virtual bool IsFocusable() const { return false; }

    // ── state
    uint32_t States = 0;
    int ActivateCount = 0;               // tests read this
    std::vector<std::string> Classes;
    std::map<std::string, std::string> Attributes;   // raw values, as written

    // dirty flags; setting any marks the root dirty too
    void MarkStyleDirty();
    void MarkLayoutDirty();
    void MarkPaintDirty();

    // ── legacy integration (see "Legacy integration" below)
    glm::vec2 GetSize() override;
    void UpdateOffsets() override;
    void UpdateChildrenOffsets() override;
    void Update() override;
    void Draw() override;
    void OnNavConfirm() override;

    // used by UiSystem / resolver / layout
    UiComputedStyle m_computed;
    UiDrawList m_chunk;                  // root only: recording target (local prims), written in UiSystem::Finalize
    struct LegacyMarker { size_t ItemIndex; std::shared_ptr<UiElement> Child; glm::mat3 ClipMatrix; glm::vec2 ClipSize; bool Clipped; };
    std::vector<LegacyMarker> m_legacyMarkers;   // root only: recording target
    // Root only: the finalized copy rendering reads (R2). Replaced only by UiSystem::Finalize.
    struct UiFrameData { UiDrawList List; std::vector<LegacyMarker> Markers; };
    std::shared_ptr<const UiFrameData> m_frame;
    std::map<std::string, std::string> m_vars;   // variables set on this element
    std::vector<std::string> m_varRefs;          // variable names this element's attributes use
    bool m_styleDirty = true, m_layoutDirty = true, m_paintDirty = true;
    bool IsRoot() const;                 // parent is not a UiElem
    UiElem* Root();
};
```

### Rules for the base element

- **Constructor:** `HitCheck = false`, `DisableFocus = true`. Focusable subclasses (Button) set `HitCheck = true; DisableFocus = false;`.
- **Every public API function** (`SetClass`, `SetVar`, `SetAttribute`, `Style()`, `AddChild`, `RemoveChild`, `Load`, `LoadLayout`, `Find`) starts with `UiSystem::AssertUiThread()` and applies directly. It only changes state and dirty flags (R3). It never records, uploads or touches `m_frame`.
- **Class or state change:** mark the subtree style-dirty, since descendant selectors can depend on ancestors.
- **`SetVar`:** store the value in `m_vars`. Then walk this element's subtree; every element whose `m_varRefs` contains the name re-applies its attributes (task 1.12, attribute application) and is marked dirty.
- **`Find`:** resolve `idPath` split by `/`. The first segment is looked up in this element's `Instance` id map, or in its own owned instance if this element is a document root. Each following segment is looked up in the instance owned by the element found. Log `[UI] … warning: Find: no element 'x'` on a miss, or on a type mismatch (`dynamic_cast`, load time only).

### Legacy integration (exact)

- `GetSize()` returns `glm::vec2(BorderRect.W, BorderRect.H)`.
- `UpdateOffsets()` sets the legacy fields from layout:
  - `worldMatrix = AbsoluteTransform`
  - `size = GetSize()`
  - `topLeft = TransformPoint(AbsoluteTransform, {0, 0})`
  - `bottomRight = topLeft + size`
  - `offset = topLeft`
  - `position = {0, 0}`
  - `visible = (Computed().Visibility == Visible)`, false when hidden or collapsed
- `UpdateChildrenOffsets()`: for each child that is **not** a `UiElem`, set `parentTopLeft` / `parentBottomRight` from `ContentRectScreen()`, set `parent = this`, and call `child->UpdateOffsets()`. `UiElem` children are skipped.
- `Update()`:
  1. Pointer states from `TouchEvents`:
     - `:hover` = `!TouchEvents.empty()`
     - `:active` = a touch with `pressed` was seen and no matching `released` yet (track the id like `UiButton::m_pressedTouchId`)
     - a `released` with the same id while focusable and not disabled → `Activate()`
     - state changes call `MarkStyleDirty()`
  2. Then call `UiElement::Update()`, which updates the children (legacy children get positioned).
  3. When `UiElement::EditModeActive && EngineMain::MainInstance->DebugUiEnabled`, skip step 1, as `UiButton` does.
- `Activate()` (private): `ActivateCount++`, then `OnActivate()`.
- `OnNavConfirm()`: if focusable and not disabled → `Activate()`.
- `Draw()` (**rendering, R2: reads only the snapshot**):
  - if `!IsRoot()`: return, because the root draws the whole subtree
  - `auto frame = m_frame;` (keeps the snapshot and its legacy children alive for this call). If it's null, or `!UiRenderer::InPass()`, return.
  - append `frame->List` in pieces:
    1. `start = 0`
    2. for each marker in `frame->Markers`: `CurrentList().AppendResolved(frame->List, start, marker.ItemIndex, UiRenderer::CurrentMaskDepth())`
    3. then, if `marker.Clipped`: `UiRenderer::PushMask(marker.ClipMatrix, marker.ClipSize)`; `marker.Child->Draw()`; if clipped: `UiRenderer::PopMask()`
    4. `start = marker.ItemIndex`
    5. finally append the rest
- Never call `UiElement::Draw()` from a `UiElem`. Legacy children are drawn only through markers.
- Hit testing uses the legacy `GetHitElementUnderPosition` unchanged. Set `LimitHitTestToBounds = (Overflow == Clip)` after layout.

### `UiElemPanel`

- `TagName "Panel"`. No extra attributes.
- `PaintContent` does nothing (the box itself is painted by the recorder, task 1.13).

### `UiElemLabel`

- Attribute `text`. If the attribute is absent, use the XML node's text content, trimmed. The text goes through substitution (task 1.12).
- Keeps a `UiTextParams m_lastParams` and a `UiTextLayoutResult m_layout`.
- `MeasureContent(availW, availH)`:
  1. Build params from `Computed()` and the final text.
  2. `WrapWidth = availW` if the element's width is not `fit-children` and `WhiteSpace == Normal`, else -1. With `TextOverflow != Clip`, also pass `WrapWidth = availW` (used as the ellipsis/shrink limit) and `MaxHeight = availH`.
  3. Re-run `UiLayoutText` only when the params differ from `m_lastParams`.
  4. Return `(m_layout.Width, m_layout.Height)`.
- `PaintContent(p, content)`:
  1. Translate the quads by `content.X`, `content.Y`. When the painter transform is translation-only, round `content.X + transform tx` and `content.Y + ty` to integers first (run-origin snapping).
  2. `p.GlyphRun(quads, n, Computed().Color with alpha × effective opacity, m_layout.GlyphScale, m_layout.Dilation)`.
- `TagName "Label"`.

### `UiElemImage`

- Attribute `src` (substituted). On change: `m_texture = AssetRegistry::GetTextureFromFile(src)`.
- `MeasureContent` returns `(texture->width, texture->height)` when `texture && texture->valid`, else `(0, 0)` and `MarkLayoutDirty()`, so it re-measures next frame until the texture loads.
- `PaintContent`:
  - if the texture isn't valid: `MarkPaintDirty()` and return
  - compute the destination rect inside `content` by `ObjectFit`, with `iw`, `ih` = the texture size and `cw`, `ch` = the content size:
    - `Fill` → the whole content
    - `Contain` → scale `s = min(cw / iw, ch / ih)`, centred
    - `Cover` → `s = max(cw / iw, ch / ih)`, centred, with UVs cropped to the visible part (not geometry outside the box)
    - `None` → native size, centred, UV-cropped to the content
    - `ScaleDown` → `Contain` if that's smaller than native, else `None`
  - `p.Image(rect, texture->getHandle(), uv, glm::vec4(1, 1, 1, effective opacity))`
- `TagName "Image"`.

### `UiElemButton` (`Button` and `TextButton`)

- `IsFocusable() = true`. The constructor sets `HitCheck = true; DisableFocus = false;`.
- Attribute `enabled` (`"false"`/`"0"` → `kStateDisabled` on and `DisableFocus = true`; else off) and `selected` (`"true"`/`"1"` → `kStateSelected`).
- `TextButton`: the constructor creates a `UiElemLabel` child with `TagName "Label"` and the same `ScopeFile` and `Instance` as the button (set by the builder after construction, task 1.12). Attribute `text` is forwarded to that label's `text`.
- `TagName` is `"Button"` or `"TextButton"`.

---

## Task 1.12: documents, components, builder, variables

Document discovery, component discovery and component parameter/slot registration publish their source paths and schemas into the existing UIDB model. Component/document hot reload updates the registry revision and marks the database dirty; the next safe main-thread flush rewrites the same file atomically. Do not copy XML/CSS source contents into UIDB.

### Definitions (`UiDocument.h`)

```cpp
struct UiNodeDef
{
    std::string Tag;
    std::vector<std::pair<std::string, std::string>> Attributes;   // in file order
    std::string Text;                 // trimmed text content (for Label, <style>)
    std::vector<UiNodeDef> Children;
    int Line = 0, Col = 0;
};
struct UiComponentDef
{
    std::string Tag;                  // from tag="..."
    std::string File;
    std::vector<std::pair<std::string, std::string>> Params;   // name, default ("" + Required flag)
    std::vector<bool> ParamRequired;
    UiNodeDef Body;                   // the single element inside <Component>
};
struct UiDocumentDef
{
    std::string Path;
    std::string SourceText;
    uint32_t ModTime = 0;
    std::vector<std::string> Imports;                                   // component files
    std::vector<std::shared_ptr<UiStyleSheet>> Sheets;                  // <include> and <style>, in order
    std::map<std::string, UiComponentDef> Snippets;
    std::shared_ptr<UiComponentDef> Component;                          // set when the top element is <Component>
    UiNodeDef Top;                                                      // the top element (or Component body)
};
std::shared_ptr<UiDocumentDef> UiDocumentCache_Get(const std::string& path);   // parse once, cached
void UiDocumentCache_Invalidate(const std::string& path);
```

### Parsing rules

- Read the file with `FileSystemEngine::ReadFile`. Missing or empty: error `cannot read file`, return nullptr.
- `pugi::xml_document::load_string(text, pugi::parse_default | pugi::parse_ws_pcdata_single)`. On failure: `UiMarkupLog::Error(path, line, col, result.description())`, with line and col from `result.offset`.
- The document element must be `<root>`, otherwise error.
- Children of `<root>`, in this order (anything else is an error):
  - optional `<imports>`: `<import src="…"/>` children → `Imports`
  - optional `<styles>`: `<include src="…"/>` → `UiStyleSheetCache::Get(src)`; `<style>` → `UiParseStyleSheet(text, "<inline:PATH:LINE>", line - 1)`; both append to `Sheets` in order
  - optional `<snippets>`: `<snippet name="X">` with exactly one element child → `Snippets[X]`, params from its `params=""`
  - exactly one top element
- **Top element `<Component tag="T" params="…">`** with exactly one element child: `Component` is set, `Top = Body`.
  - Param syntax: comma-separated entries `name` or `name=default`, where default is `'…'`, `"…"` or a bare token. An entry without `=` is required.
- Node positions: `UiMarkupLog::LineColumn(SourceText, node.offset_debug(), line, col)`.
- **Attribute names on element nodes** are lowercase-with-dashes for built-ins. Legacy tags keep property names as written (case-sensitive).

### Component registry

```cpp
namespace UiComponentRegistry
{
    void Discover();          // scans GameData/ui/kit/components/ and GameData/ui/components/ (flat, *.xml)
    const UiComponentDef* Find(const std::string& tag);   // global tags
    void RegisterFile(const std::string& path);           // used by Discover and by imports
}
```

- `Discover`:
  1. `FileSystemEngine::GetFilesInPath(dir)`; keep names ending in `.xml`; full path = `dir + name`.
  2. Load each with the document cache. Its `Component` must be set, otherwise a warning `not a component file`.
  3. Register `Tag → def`. A duplicate tag is an error naming both files, and the first one wins.
- Tag lookup order for a node in file F (in the builder):
  1. built-in tags
  2. snippets of F
  3. components imported by F
  4. global components
  5. legacy tags (`UiElementRegistry` names)
  6. otherwise: error `unknown tag 'X'`, and the node is skipped

### Tag registry (built-ins and code-behind)

```cpp
namespace UiTagRegistry
{
    using Factory = std::shared_ptr<UiElem>(*)();
    void RegisterBuiltin(const std::string& tag, Factory f);      // Panel, Label, Image, Button, TextButton
    Factory FindBuiltin(const std::string& tag);
    // Code-behind: a layout path -> class. UiElem::Load(path) constructs the class (which calls LoadLayout itself).
    void RegisterCodeBehind(const std::string& layoutPath, Factory f);
    Factory FindCodeBehind(const std::string& layoutPath);
}
#define UI_REGISTER_CODE_BEHIND(PATH, CLASS) \
    static const bool CONCAT(_uiCodeBehind_, __LINE__) = (UiTagRegistry::RegisterCodeBehind(PATH, []() -> std::shared_ptr<UiElem> { return std::make_shared<CLASS>(); }), true)
```

Built-ins are registered explicitly in `UiSystem::Init` (engine static-library code must not rely on static initialisers; see the comment in `UiElementRegistry.cpp`). `UI_REGISTER_CODE_BEHIND` is for game code, where static registration works, the same as `UI_REGISTER_ELEMENT`.

### Builder: instantiation

`UiDocumentInstance` holds an id map (`std::unordered_map<std::string, std::weak_ptr<UiElem>>`) and a pointer to its `UiDocumentDef`.

**`UiElem::Load(path)`:**

1. If `FindCodeBehind(path)` exists, construct the class. It calls `LoadLayout(path)`, which does the rest.
2. Otherwise create the element from the definition's top tag (built-in factory), then `LoadLayout(path)` on it.
3. Register the result in `UiSystem` as a root (weak).

**`LoadLayout(path)` on element E:**

1. Get the definition. A definition with `Component` set gives the error `cannot load a component file as a layout`.
2. Check the top element's tag equals E's `TagName`, else error `layout top element <X> does not match element <Y>`.
3. Create `UiDocumentInstance I` and store it as E's owned instance. Set E's `Instance = &I`, `ScopeFile = path`.
4. Apply the top node's attributes to E, then build its children into E with context `{instance I, scope file path, def}`.

**Build node N into parent P with context C:**

1. **Component tag** (a snippet, an import or a global component):
   1. Cycle check: keep a stack of component tags being built. If N's tag is already on it, error `component cycle: A -> B -> A`, and skip.
   2. New instance `I2`. Build the component body node into P with context `{I2, def.File, slots = N's children, slotContext = C, params}`.
   3. The created top element R gets `ComponentTag = tag`.
   4. N's attributes:
      - a declared param → `R.m_vars[param] = value` (raw)
      - a standard attribute (`id`, `class`, `style`, `visible`, `enabled`, `selected`, `hittest`, `default-focus`, and from phase 2 any `on*`) → applied to R; `class` is **merged** with R's classes
      - anything else → error `component 'T' has no parameter 'x'`
   5. A missing required param → error and value `""`. Defaults fill unset params.
   6. If N has `id`, register it in **C's** instance map, pointing to R.
2. **`<Children/>` / `<Children slot="x"/>`:** build the slot's nodes into P using **slotContext**. That keeps those children in the outer instance and scope file. Default slot = children without a `slot` attribute. Named slot = children with `slot="x"`. Children given but no matching `<Children>` → error `component 'T' has no slot 'x'`.
3. **Built-in tag:** factory, then:
   - `TagName = N.Tag` (TextButton already sets it)
   - `ScopeFile = C.file`, `Instance = C.instance`
   - apply attributes; for `TextButton`, set its label's `ScopeFile` / `Instance` the same way
   - register the id in `C.instance`
   - `P->AddChild(elem)`, then build N's children with C
4. **Legacy tag:** create through `UiElementRegistry::Instance().Create(tag)`. Apply attributes by building a `LayoutBlueprintNode{type = tag}` whose `properties` JSON holds each attribute: the value parsed as JSON when it starts with `[`, `{`, a digit, `-`, or is `true`/`false`; otherwise a JSON string. Then call `UiLayoutSerializer::ApplyProperties(child, node)`. `P->AddChild(child)`. Legacy children of a legacy tag in markup are an error (legacy tags are leaves in markup).

### Applying attributes to an element (also on re-apply after `SetVar`)

- `id`: set `Id` (not substituted).
- `class`: split on spaces (not substituted). Classes from code (`SetClass`) are kept in a separate set and merged when applied.
- `style`: stored raw (not substituted) for the resolver.
- `visible` (substituted): falsey (`""`, `0`, `false`) → `Visibility` override `Collapse` through `Style()`. Truthy clears the override.
- `enabled`, `selected`, `hittest` (`"true"` → `HitCheck = true`), `default-focus` (stored; used from phase 2).
- Anything else goes to `ApplyAttribute(name, substituted value)`. If that returns false: warning `unknown attribute 'x' on <Tag>`.
- Every substituted value records the variable names it used into `m_varRefs`.

### Variables and substitution (`UiVariables.h`)

```cpp
// Replaces {name} and {name:fmt} with values found by walking from `element` up its UiElem ancestors.
// "${KEY}" is localisation, not a variable: a '{' directly after '$' is skipped. "{{" is a literal '{'.
// Then localises, then substitutes again (translated strings may contain {vars}). Values may contain
// {refs} themselves; substitution repeats at most 4 levels deep.
std::string UiSubstitute(const UiElem& element, const std::string& raw, std::vector<std::string>* usedNames);
```

- Lookup: `element.m_vars`, then each ancestor's, up to the root.
- A missing name → empty string, plus one warning per (element, name).
- Formatting:
  - `{name:%.1f}` (any printf format starting with `%` and ending in `f`, `d` or `i`) is applied when the value parses as a number; `d`/`i` cast to `long long`
  - a bad or unsupported format → the raw value plus a warning
- Order: substitute variables → `Localisation::LocalizeString` → substitute again.
- `SetVar(name, double)` stores the shortest round-trip text (`%.6g`) and keeps the double for formats.

---

## Task 1.13: flow layout and paint recording

### Layout (`UiFlowLayout.h`)

```cpp
namespace UiFlowLayout
{
    // Lays out a root inside its container (size in UI units) and sets every element's BorderRect,
    // AbsoluteTransform (from `base`), and the legacy fields (by calling UpdateOffsets()).
    void LayoutRoot(UiElem& root, glm::vec2 containerSize, const glm::mat3& base);
}
```

Definitions:

- `outerW(child) = child.borderW + marginLeft + marginRight`, and `outerH` likewise.
- Margins and padding in % resolve against the **parent's content width** (CSS rule).
- `x` resolves against the parent's content width, and `y` against its content height.
- Elements with `Visibility == Collapse` are skipped completely (no space, no paint). `Hidden` ones take space but aren't painted or hit-tested.
- Clamp every resolved size with min/max (max `Auto` = no limit). Sizes are never below 0.

**`Measure(E, availW, availH)`** returns E's border-box size. `availW`/`availH` are the content size of the parent along each axis, or `NaN` when the parent's size on that axis depends on its children.

For each axis (width shown; height is symmetric, with `height-percentage` ↔ `width-percentage`):

- `Px`, `Percent`, `Calc` → `Resolve(availW)`. When `availW` is NaN and the value has a percent part: use 0 for the percent part, and log a warning once per element (`percentage of a fit-children parent`).
- `FitChildren` → computed from the content (below).
- `FillParentFlow` → NaN here. The parent assigns it during its arrange. If the parent doesn't flow on this axis, it behaves like `100%` of `availW` minus margins.
- `HeightPercentage(p)` (width only) → `p% × E's resolved height` (resolve the height first). `WidthPercentage(p)` (height only) → `p% × E's resolved width`. Both at once is an error, and both become fit-children.

Content measure for `FitChildren` on an axis:

1. `contentAvailW` = E's resolved width minus padding minus border (NaN if not resolved).
2. Measure every non-collapsed child with `(contentAvailW, contentAvailH)`. A `FillParentFlow` child along E's flow axis contributes its min size (0 by default).
3. Combine per flow:
   - `None`: content W = max over children of `(child.x offset if positive) + outerW`; H likewise
   - `Down`: W = max outerW; H = sum outerH + gap × (n − 1)
   - `Right`: W = sum outerW + gap × (n − 1); H = max outerH
   - `RightWrap` / `DownWrap` with an unresolved size on the wrap axis behave like `Right` / `Down`
4. Add `MeasureContent(contentAvailW, contentAvailH)` for elements with content (Label, Image): content W = max(children W, content W), and the same for H.
5. border-box = content + padding + border.

**`Arrange(E, rect)`**, where `rect` is E's border box relative to its parent's border box:

1. Set `E.BorderRect = rect`. `content` = rect deflated by border widths and padding (local to E).
2. **Re-measure every child** with the now-known content size: `Measure(child, content.W, content.H)`. This resolves percentages and fills that depended on E.
3. Place the children by `Flow`:
   - **`None`:** for each child, `w`/`h` = the measured size (fill → content minus margins).
     - `x0 = content.X + marginLeft` for `Left`; `content.X + (content.W − outerW) / 2 + marginLeft` for `Center`; `content.X + content.W − outerW + marginLeft` for `Right`
     - `y0` likewise with `VAlign`
     - then add the child's `x` / `y` offsets
   - **`Down`:**
     1. `free = content.H − sum(outerH of non-fill children) − sum(marginTop + marginBottom of fill children) − gap × (n − 1)`
     2. each fill child gets `max(0, free) × weight / totalWeight` as its border height
     3. the cursor starts at `content.Y`. Each child: `y = cursor + marginTop`; x by the child's `HAlign` inside `content.W` as in `None` (a cross-axis fill means `width = content.W − margins`); then add the x/y offsets
     4. `cursor += outerH + gap`
     - The child's `VAlign` is ignored in `Down`. To centre a column, centre the container.
   - **`Right`:** symmetric to `Down` (cross axis uses `VAlign`, and `HAlign` is ignored).
   - **`RightWrap`:**
     - Lines: `cursorX = 0`. A child goes on the current line when the line is empty or `cursorX + outerW ≤ content.W`; else a new line starts.
     - `lineH` = max outerH in the line. Lines stack with `gap`, and children in a line are separated by `gap`.
     - Within a line, children align by their `VAlign` inside `lineH`.
     - Fill children in wrap mode count as their min size, with a warning.
   - **`DownWrap`:** symmetric.
4. Arrange each child recursively with its rect.
5. Children are kept in document order for layout. **Paint order** is a stable sort by `ZIndex`.

`AbsoluteTransform(E) = AbsoluteTransform(parent) × Mat3Translate(E.BorderRect.X, E.BorderRect.Y)`, and for the root it's `base × Mat3Translate(rect)`. Phase 3 inserts render transforms here.

**`LayoutRoot`:** treat the container as a `flow-children: none` parent with content `(0, 0, containerSize)`. Measure the root with `containerSize`, place it by its align, x/y and margins exactly as in `None`, arrange it, then call `UpdateOffsets()` on every `UiElem` in the tree (legacy fields).

### Paint recording (in `UiSystem`, root only, local-prim list)

`Record(root)`:

1. `m_chunk.Clear(); m_chunk.SetLocalPrims(true); m_legacyMarkers.clear();`
2. `UiPainter p(m_chunk)`.
3. Recurse `Paint(E, p, parentOpacity)`:
   1. If `Visibility != Visible`, return (collapsed and hidden elements don't paint).
   2. `op = parentOpacity × Opacity`.
   3. `p.SetTransform(E.AbsoluteTransform)`.
   4. **Box:** if the background alpha > 0 or any border side is solid with width > 0, then `p.Box({0, 0, W, H}, paint)` with:
      - fill = background with alpha × op
      - radii resolved: % of `min(W, H)`, clamped to `min(W, H) / 2`
      - border widths: a side counts only when its style is `solid`
      - border colours with alpha × op
   5. `E.PaintContent(p, ContentRectLocal())`, passing `op` through a member or parameter (set `p`'s current opacity; use one member `float m_paintOpacity` that the element reads).
   6. If `Overflow == Clip`: `p.PushClip({0, 0, W, H}, radii)`.
   7. Children in paint order:
      - a `UiElem` child → recurse
      - a legacy child → push a marker `{m_chunk.Items().size(), child (the shared_ptr from `children`), AbsoluteTransform, {W, H}, clipped = (any ancestor in this root clips)}`. The legacy child is drawn by `Draw()` at that position. The clip matrix/size used is the **nearest** clipping ancestor's.
   8. If it clipped: `p.PopClip()`.
4. Clear every paint-dirty flag.
5. **Finalize the copy:** `m_frame = std::make_shared<const UiFrameData>(UiFrameData{ m_chunk, m_legacyMarkers });`. This copy is the only thing rendering reads. When nothing was re-recorded, keep the old `m_frame` (no copy).

---

## Task 1.14: `UiSystem` and first complete UIDB publication

```cpp
class UiSystem
{
public:
    static void Init();                  // from EngineMain::Init, after UiElementRegistry::RegisterBuiltinTypes() and Localisation::Initialize
    static void Shutdown();
    static void Update();                // main loop, right before Viewport.Update() (R1): input, handlers, Tick, style, layout
    static void Finalize();              // main loop, right after Viewport.FinalizeChildren() (R1): record + copy snapshots, uploads
    static void AssertUiThread();        // debug: main thread or game thread only (R3)
    static void NoteGameThread();        // called at the top of EngineMain::GameUpdate()
    static void RegisterRoot(const std::shared_ptr<UiElem>& root);
    static UiElem* FindById(const std::string& id);   // first match over all live roots, document order (tests)
    static std::thread::id MainThread();
};
```

### `Init`

- Record the main thread id.
- `UiFontLibrary::Init()`.
- Register built-in tags: Panel, Label, Image, Button, TextButton.
- `UiComponentRegistry::Discover()`.
- After all game/engine UI registrations and component discovery, initialise `UiPreviewRegistry` built-in recipes for every safely instantiable document, component, built-in tag and legacy tag. Call `UiDatabase::RefreshFromRegistries()` and flush the complete deterministic Phase 1 snapshot to `SaveData/UIStudio/uidb.json` before accepting a Studio runtime connection. This atomically refreshes the baseline generated by Task 1.2a; it is not a second database or writer. Later phases extend this same database.
- Register the console commands:
  - `ui_load <path>`: `auto e = UiElem::Load(path); EngineMain::Viewport.AddChild(e);`
  - `ui_unload_all`: removes every registered root from its parent
  - `ui_dump [depth]`: prints every root tree: indent, `<Tag#id.class1.class2>`, `[hover active focus disabled selected]`, `rect=(x,y,w,h)` (screen AABB), `hidden` / `collapsed`; legacy children as `<legacy TypeName>`
  - `ui_focus <id>`: `UiNavigation::SetFocus(FindById(id))`
- Read `-ui_preview <path>` (`EngineMain::MainInstance->Arguments["ui_preview"]`). If present, load it on the first `Update` after `EngineMain`'s frame counter reaches 10 (keep a local frame count in `UiSystem`).

### `Update` (exact order)

1. Hot reload: every 0.5 s of real time (`SDL_GetTicks64`), compare `FileSystemEngine::GetFileModificationTime` of every `.xml` and `.css` any live root depends on (each root keeps its set of files). On a change:
   - invalidate those caches
   - **rebuild** each affected root: remember its root `m_vars` and code classes, clear its children, call `LoadLayout(path)` again, restore the vars and classes
   - log `[UI] reloaded <path>`
   - if a component file changed, re-run `Discover()`
2. For every live root, in registration order:
   1. **Attached check:** walk `parent` pointers. Reaching `&EngineMain::Viewport` means attached. Otherwise skip it (P1-D7: warn once if it has a non-null parent that isn't under the viewport).
   2. Call `Tick()` on every element (depth-first).
   3. Container: `parent == &Viewport` → size `Viewport.GetSize()`, base identity. Else size `parent->bottomRight − parent->topLeft` and base `parent->worldMatrix`. If size or base differ from last frame → layout-dirty.
   4. Style-dirty → resolve the whole root, which also makes it layout-dirty and paint-dirty.
   5. Layout-dirty → `LayoutRoot`, which makes it paint-dirty.
3. Drop expired roots.

### `Finalize` (exact order)

1. For every live, attached root that is paint-dirty: `Record(root)`, which ends by replacing `m_frame` (task 1.13, step 5).
2. `UiFontLibrary::FlushUploads()`. Text layout during `Update` may have generated glyphs; this uploads them before rendering.

### `EngineMain.cpp`

- In `Init`, after `Localisation::Initialize(...)`: `UiDatabase::InitBootstrap(...)` is already called by Task 1.2a; once the new UI subsystem exists, call `UiSystem::Init();`. `UiSystem::Init` completes UI registry discovery and refreshes the UIDB snapshot as specified above. Keep the bootstrap before `UiSystem::Init` so intermediate builds still generate a truthful baseline.
- In `MainLoop`, directly before `Viewport.Update();`: `UiSystem::Update();`.
- In `MainLoop`, directly after `Viewport.FinalizeChildren();`: `UiSystem::Finalize();`.
- At the top of `EngineMain::GameUpdate()`: `UiSystem::NoteGameThread();`.
- In shutdown, before `UiRenderer::Shutdown()`: `UiSystem::Shutdown();`.

---

## Task 1.15: test runner commands

Register them through `UiTestRunner::RegisterCommand` in `UiSystem::Init`:

| Command | Arguments | Pass when |
|---|---|---|
| `expect_rect <id> <x> <y> <w> <h> [tol]` | numbers, tol default 0.5 | the element's `ContentRectScreen()`-style **border-box** screen AABB matches each value within tol |
| `expect <id> <prop> <op> <value>` | prop ∈ `x y w h lines focus hover active disabled selected visible activations text font_size`; op ∈ `== != < <= > >= contains` | the comparison holds; numbers use tol 0.5 for `==`; `text` is the final substituted text of a Label; `lines` of a Label |
| `expect_exists <id>` / `expect_missing <id>` | | found / not found |
| `expect_ui_errors <n>` / `expect_ui_warnings <n>` | | `UiMarkupLog` counts since start equal n |
| `set_var <id> <name> <value>` | | always passes; calls `SetVar` |
| `set_class <id> <class> <0/1>` | | always passes |

On failure, the message includes the actual value: `FAIL 12 expect #b w == 140: got 138.0`.

---

## Task 1.16: test layouts

All of these go into `Tools/ui/assets/GameData/ui/kit/tests/`. Sync them to all games.

### `p1_layout.xml` (expected rects are exact; they were computed by hand from the rules above)

```xml
<root>
  <styles>
    <style>
      .root { width: 1000px; height: 1000px; }
      .col  { flow-children: down; width: 300px; padding: 10px; gap: 5px; }
      .a    { width: 100px; height: 40px; }
      .b    { width: 50%; height: 20px; }
      .c    { width: fill-parent-flow(1); height: 30px; }
      .row  { flow-children: right; width: 400px; height: 100px; x: 400px; gap: 10px; }
      .r1   { width: 100px; height: 50px; vertical-align: center; }
      .r2   { width: fill-parent-flow(1); height: 100%; }
      .r3   { width: fill-parent-flow(3); height: 20px; vertical-align: bottom; }
      .wrap { flow-children: right-wrap; width: 250px; y: 400px; gap: 10px; }
      .w    { width: 100px; height: 30px; }
      .abs  { width: 200px; height: 200px; x: 400px; y: 400px; }
      .tr   { width: 50px; height: 50px; horizontal-align: right; }
      .ctr  { width: 60px; height: 40px; horizontal-align: center; vertical-align: center; }
      .bl   { width: 30px; height: 30px; vertical-align: bottom; margin: 5px; }
      .fit  { padding: 8px; x: 700px; y: 400px; flow-children: right; gap: 4px; }
      .fitc { width: 20px; height: 10px; }
      .marg { flow-children: down; width: 200px; y: 700px; }
      .m1   { width: 100px; height: 20px; margin: 10px 0px 5px 20px; }
      .m2   { width: 100px; height: 20px; horizontal-align: right; }
    </style>
  </styles>
  <Panel id="root" class="root">
    <Panel id="col" class="col"><Panel id="a" class="a"/><Panel id="b" class="b"/><Panel id="c" class="c"/></Panel>
    <Panel id="row" class="row"><Panel id="r1" class="r1"/><Panel id="r2" class="r2"/><Panel id="r3" class="r3"/></Panel>
    <Panel id="wrap" class="wrap"><Panel id="w1" class="w"/><Panel id="w2" class="w"/><Panel id="w3" class="w"/><Panel id="w4" class="w"/><Panel id="w5" class="w"/></Panel>
    <Panel id="abs" class="abs"><Panel id="tr" class="tr"/><Panel id="ctr" class="ctr"/><Panel id="bl" class="bl"/></Panel>
    <Panel id="fit" class="fit"><Panel id="f1" class="fitc"/><Panel id="f2" class="fitc"/><Panel id="f3" class="fitc"/></Panel>
    <Panel id="marg" class="marg"><Panel id="m1" class="m1"/><Panel id="m2" class="m2"/></Panel>
  </Panel>
</root>
```

`Tools/ui/scripts/Common/p1_layout.uitest`:

```
wait 6
console ui_load GameData/ui/kit/tests/p1_layout.xml
wait_frames 3
expect_rect root 0 0 1000 1000
expect_rect col 0 0 300 120
expect_rect a 10 10 100 40
expect_rect b 10 55 140 20
expect_rect c 10 80 280 30
expect_rect row 400 0 400 100
expect_rect r1 400 25 100 50
expect_rect r2 510 0 70 100
expect_rect r3 590 80 210 20
expect_rect wrap 0 400 250 110
expect_rect w1 0 400 100 30
expect_rect w2 110 400 100 30
expect_rect w3 0 440 100 30
expect_rect w4 110 440 100 30
expect_rect w5 0 480 100 30
expect_rect abs 400 400 200 200
expect_rect tr 550 400 50 50
expect_rect ctr 470 480 60 40
expect_rect bl 405 565 30 30
expect_rect fit 700 400 84 26
expect_rect f1 708 408 20 10
expect_rect f2 732 408 20 10
expect_rect f3 756 408 20 10
expect_rect marg 0 700 200 55
expect_rect m1 20 710 100 20
expect_rect m2 100 735 100 20
expect_ui_errors 0
quit
```

How the less obvious numbers were derived, so a failure can be checked by hand:

- `b`: 50% of col content 280 = 140, at y = 10 + 40 + 5.
- `c`: a fill on the cross axis = content width 280, at y = 55 + 20 + 5.
- `col` height = 10 + 40 + 5 + 20 + 5 + 30 + 10.
- `row`: free = 400 − 100 − 2×10 = 280. r2 = 280 × 1/4 = 70, r3 = 210.
- `wrap`: lines `[w1 w2] [w3 w4] [w5]`, height 30 + 10 + 30 + 10 + 30.
- `fit`: width 3×20 + 2×4 + 2×8 = 84, height 10 + 16.
- `marg`: m1 at y = 700 + 10, the cursor after it = 710 + 20 + 5 = 735, height = 10 + 20 + 5 + 20.

### `p1_visual.xml` and probes

```xml
<root>
  <styles>
    <style>
      .v  { horizontal-align: left; vertical-align: top; }
      #v1 { x: 100px; y: 100px; width: 200px; height: 100px; background-color: #ff0000; }
      #v2 { x: 350px; y: 100px; width: 200px; height: 100px; background-color: lime; border-radius: 40px; }
      #v3 { x: 600px; y: 100px; width: 200px; height: 100px; border: 4px solid white;
            border-top-color: red; border-right-color: lime; border-bottom-color: blue; }
      #v4 { x: 850px; y: 100px; width: 200px; height: 100px; opacity: 0.5; }
      #v4c { width: 100%; height: 100%; background-color: white; }
      #v5 { x: 100px; y: 250px; width: 200px; height: 100px; border-radius: 30px; overflow: clip; }
      #v5c { x: -50px; y: -50px; width: 400px; height: 250px; background-color: blue; }
      #v6 { x: 100px; y: 450px; width: 64px; height: 64px; }
      #v7 { x: 350px; y: 250px; width: 100px; height: 100px; background-color: var(--accent, #00ffff); }
      :root { --accent: #ffff00; }
    </style>
  </styles>
  <Panel id="vroot" class="v">
    <Panel id="v1" class="v"/>
    <Panel id="v2" class="v"/>
    <Panel id="v3" class="v"/>
    <Panel id="v4" class="v"><Panel id="v4c"/></Panel>
    <Panel id="v5" class="v"><Panel id="v5c"/></Panel>
    <Image id="v6" class="v" src="GameData/textures/generic/white.png"/>
    <Panel id="v7" class="v"/>
  </Panel>
</root>
```

The root `vroot` is fit-children, so it grows to contain everything. Children are positioned by x/y from the root's top-left at (0, 0).

`p1_visual.probes` (`X Y R G B A TOL LABEL`; UI target pixels; premultiplied):

```
150 150 255 0 0 255 1 v1 fill
99 150 0 0 0 0 1 v1 outside left
450 150 0 255 0 255 1 v2 fill
351 101 0 0 0 0 1 v2 rounded corner outside
700 101 255 0 0 255 1 v3 top border
797 150 0 255 0 255 1 v3 right border
700 198 0 0 255 255 1 v3 bottom border
602 150 255 255 255 255 1 v3 left border
700 150 0 0 0 0 1 v3 inside
950 150 128 128 128 128 2 v4 half opacity child
200 300 0 0 255 255 1 v5 clipped child inside
101 251 0 0 0 0 1 v5 clip corner
132 482 255 255 255 255 1 v6 image
400 300 255 255 0 255 1 v7 var from root
```

The script `p1_visual.uitest`: `wait 6`, `console ui_load GameData/ui/kit/tests/p1_visual.xml`, `wait_frames 3`, `ui_screenshot visual`, `expect_ui_errors 0`, `quit`.

### `p1_text.xml`

```xml
<root>
  <styles>
    <style>
      #t1 { x: 50px; y: 50px; font-size: 32px; }
      #t2 { x: 50px; y: 150px; width: 200px; font-size: 24px; }
      #t3 { x: 50px; y: 350px; width: 150px; font-size: 24px; white-space: nowrap; text-overflow: ellipsis; }
      #t4 { x: 50px; y: 450px; font-size: 24px; text-transform: uppercase; }
      #t5 { x: 400px; y: 50px; width: 300px; font-size: 20px; text-align: center; }
    </style>
  </styles>
  <Panel id="troot">
    <Label id="t1" text="Hello {name}"/>
    <Label id="t2" text="The quick brown fox jumps over the lazy dog again and again"/>
    <Label id="t3" text="This line is far too long to fit"/>
    <Label id="t4" text="shout"/>
    <Label id="t5">Centered{{braces}}</Label>
  </Panel>
</root>
```

`p1_text.uitest`:

```
wait 6
console ui_load GameData/ui/kit/tests/p1_text.xml
wait_frames 2
set_var troot name World
wait_frames 3
expect t1 text == Hello World
expect t1 lines == 1
expect t2 lines >= 3
expect t2 w <= 200
expect t3 lines == 1
expect t3 w <= 150
expect t3 text contains This
expect t4 text == SHOUT
expect t5 text == Centered{braces}
ui_screenshot text
expect_ui_errors 0
quit
```

`expect … text` compares the **displayed** text: after substitution, localisation, transform and ellipsis. For `t3` the displayed text ends with `…`. Open `text.png` with the Read tool and check visually:

- t1 one line
- t2 wraps onto 3 or more lines within 200 px
- t3 ends with an ellipsis
- t4 is in capitals
- t5 is centred in a 300 px box

### `p1_components.xml` plus a component

`ui/kit/tests/TestRow.xml`:

```xml
<root>
  <styles>
    <style>
      .row   { flow-children: right; width: 400px; height: 40px; gap: 8px; background-color: #202020; }
      .label { width: fill-parent-flow(1); vertical-align: center; }
      .tag   { background-color: #ff00ff; width: 10px; height: 10px; }
    </style>
  </styles>
  <Component tag="TestRow" params="label, hint='none'">
    <Panel class="row">
      <Label id="label" class="label" text="{label} ({hint})"/>
      <Children/>
      <Panel class="tag"/>
    </Panel>
  </Component>
</root>
```

`p1_components.xml`:

```xml
<root>
  <imports>
    <import src="GameData/ui/kit/tests/TestRow.xml"/>
  </imports>
  <styles>
    <style>
      #page { flow-children: down; gap: 4px; x: 50px; y: 50px; }
      .tag  { background-color: #00ff00; }   /* page rule: reaches into the component on purpose */
      .slotted { width: 30px; height: 30px; background-color: #0000ff; }
      .label { color: red; }                 /* page rule also matches the component's label */
    </style>
  </styles>
  <Panel id="page">
    <TestRow id="row1" label="First"><Panel id="s1" class="slotted"/></TestRow>
    <TestRow id="row2" label="Second" hint="yes" class="extra"/>
  </Panel>
</root>
```

`p1_components.uitest`:

```
wait 6
console ui_load GameData/ui/kit/tests/p1_components.xml
wait_frames 3
expect_exists row1
expect_exists s1
expect_missing label
expect row1 w == 400
expect s1 w == 30
ui_screenshot components
expect_ui_errors 0
quit
```

- `expect_missing label` proves the ids inside the component are scoped: `label` exists only in the instance's own id map, not in the page's.
- Also add `expect` support for an id path (`row1/label`), and check `expect row1/label text == First (none)` and `expect row2/label text == Second (yes)`.

Visual checks in `components.png` (Read tool):

- the `.tag` squares are **green**: the page rule wins over the component rule because the document layer comes later
- the labels are red
- row 1 has a blue slotted square between its label and its tag

### `p1_errors.xml`, error reporting

`ui/kit/tests/CycleA.xml` (a component that uses `CycleB`) and `CycleB.xml` (uses `CycleA`) both import each other. `p1_errors.xml`:

```xml
<root>
  <imports><import src="GameData/ui/kit/tests/CycleA.xml"/></imports>
  <styles><style>
    .bad { width: banana; }
    .ok  { width: 10px; height: 10px; }
  </style></styles>
  <Panel id="eroot">
    <CycleA/>
    <NoSuchTag/>
    <Panel class="bad ok" unknown-attr="1"/>
  </Panel>
</root>
```

`p1_errors.uitest`: load it, wait 3 frames, then `expect_ui_errors 2` (the cycle and the unknown tag) and `expect_ui_warnings 2` (the invalid `width` value and the unknown attribute). Then `ui_dump` via `console ui_dump` and `quit`. In the game log, each of the four messages must show a correct `file:line:col`. Check them by opening the files.

### `p1_buttons.xml`, focus and activation through the existing navigation

```xml
<root>
  <styles><style>
    #menu { flow-children: down; gap: 10px; x: 100px; y: 100px; }
    TextButton { width: 300px; height: 60px; background-color: #303030; border-radius: 8px; }
    TextButton:focus { background-color: #a04030; }
    TextButton Label { horizontal-align: center; vertical-align: center; }
  </style></styles>
  <Panel id="menu">
    <TextButton id="b1" text="One"/>
    <TextButton id="b2" text="Two"/>
    <TextButton id="b3" text="Three" enabled="false"/>
  </Panel>
</root>
```

`p1_buttons.uitest`:

```
wait 6
console ui_load GameData/ui/kit/tests/p1_buttons.xml
wait_frames 3
console ui_focus b1
wait_frames 3
expect b1 focus == 1
action ui_down
wait_frames 3
expect b2 focus == 1
action ui_down
wait_frames 3
expect b2 focus == 1
action ui_confirm
wait_frames 3
expect b2 activations == 1
expect b3 activations == 0
ui_screenshot buttons
quit
```

`b3` is disabled, so it isn't focusable and the second `ui_down` keeps the focus on `b2`. Visual check: b2 is drawn in the focus colour.

---

### `p1_gamethread`, the UI API used from game-thread code

Add the console command `ui_gamethread_test <path>`. It makes the **next** `GameUpdate` (game thread) do all of the following:

- `UiElem::Load(path)`, then `EngineMain::Viewport.AddChild`
- `SetVar` and `SetClass` on it
- `RemoveChild` of a child that was drawn last frame

Script:

1. `wait 6`
2. `console ui_gamethread_test GameData/ui/kit/tests/p1_buttons.xml`
3. `wait_frames 4`
4. expect the loaded element's rect and the removed child to be missing
5. `ui_screenshot gamethread`

It must pass on every backend, with no assertion from `AssertUiThread`. Run it 20 times in a loop script (`console` repeated) to shake out races, with no crash.

## Task 1.17: run everything

For StealthAction, on **all backends**, run `p1_layout`, `p1_visual`, `p1_text`, `p1_components`, `p1_errors`, `p1_buttons` and `p1_gamethread`:

- every script's `result.txt` says `RESULT PASS`
- `p1_visual` probes are all `OK` on every backend
- `cross` reports `EQUAL` for every script that captures (`p1_visual`, `p1_text`, `p1_components`, `p1_buttons`)

Then run the phase 0 regression for all four games (the legacy goldens and the painter card). They must still be `EQUAL`.

**Check 1.17:** all of the above. Also run each `p1_*` script once on ShooterGame, SimGame and SlasherGame (dx11 only). They must pass there too, which proves the synced assets and the absence of game assumptions.

## Task 1.18: verify the completed Phase 1 UIDB snapshot

This task is verification only. Do not first implement `UiDatabase`, the serializer, the preview registry or launch generation here; those foundations belong to Task 1.2a and publication is integrated in Task 1.14.

1. Launch the pilot game normally and confirm `SaveData/UIStudio/uidb.json` appears before Studio accepts a runtime connection. It must be valid UTF-8 JSON and contain deterministic Phase 1 registry entries.
2. Launch twice without changing registries; semantic JSON content must be identical.
3. Verify paths are GameData-relative and no absolute machine path, runtime object pointer, live player value or localisation translation is present.
4. Point a temporary Studio discovery test at the game's `GameData` directory and confirm it finds the sibling `SaveData/UIStudio/uidb.json` without a `GAME_NAME` argument.
5. Confirm the database lists discovered documents, CSS dependencies/properties, built-in and legacy tags, component definitions/parameters/slots, and safely instantiable document/component/element preview recipes.
6. Register a temporary custom preview provider for a test `Panel`; confirm the database lists its descriptor but not its callback/pointer.
7. Change a component and trigger hot reload; confirm the registry revision changes and the same UIDB file is atomically refreshed.
8. Make the database destination unwritable; game startup and UI rendering must still succeed, the logger must report the metadata write failure, and a connected runtime must retain its in-memory database.

**Check 1.18:** every item above passes on the pilot game. Confirm the early baseline comes from Task 1.2a and is refreshed by Task 1.14 after the new markup registries are ready; there is still only one writer and one canonical metadata file.

## Phase wrap-up

The UIDB contract, serializer and baseline launch generation were established in Task 1.2a; Task 1.14 adds the full Phase 1 registry refresh, and Task 1.18 verifies it. Later phases publish their own registries/preview descriptors into this same database; do not create `uidb_p2.json`, separate Studio schemas or phase-specific sidecars.

1. All tasks' checks pass.
2. All four games build.
3. The shaders compile for every target.
4. Update the README status row, and write the implementation notes below.

## Implementation notes

Status: done and verified on StealthAction (pilot game) on dx11, dx12, vk and gl. ShooterGame, SimGame and SlasherGame were deliberately not run: the user confirmed the engine behaves the same there (two share code, the third is almost identical). Nothing is committed.

What exists (all under `source/Engine/UI/`, new unless noted):
- `Markup/`: `UiMarkupLog`, `UiDocument` (pugixml v1.14 in `includedLibraries/pugixml`, document cache, `UiComponentRegistry`), `UiTagRegistry`, `UiVariables` (`{{`/`}}`, `${KEY}`, number formats), `UiBuilder` (components with params/slots, cycle check, legacy tags through `UiLayoutSerializer`).
- `Style/`: tokenizer, parser, selectors, computed style, resolver (component layer, document layer, specificity, source order, `style=""`, C++ `Style()`).
- `Flow/UiFlowLayout`: Measure/Arrange, fill axes, fit-children, wrapping text.
- `Render/`: `UiFontLibrary` (SDF atlas, R8 2048x2048, up to 4 pages), `UiTextLayout`, glyph runs in `UiPainter`, `UiDrawList` local prims and `AppendResolved`/`Barrier`, `uiShadeSdfGlyph` (kind 2) in `fs_ui2.sc`.
- `Elements/`: `UiElem`, `UiElemPanel`, `UiElemLabel`, `UiElemImage`, `UiElemButton`.
- `UiSystem`: init, hot reload poll (500 ms), per-frame style/layout, `Finalize` (record, atlas upload, UIDB flush), console and test commands.
- `Preview/`: `UiDatabase` (`SaveData/UIStudio/uidb.json`, atomic write) and `UiPreviewRegistry`.
- Tests: `Tools/ui/scripts/Common/p1_*.uitest` and assets in `Tools/ui/assets/GameData/ui/kit/tests/`.

Gotchas found while verifying (keep for later phases):
- A bgfx texture created with initial data is immutable: `updateTexture2D` is silently ignored. The glyph atlas pages are created without data and the first flush uploads the zeroed page. The first version showed no text at all because of this.
- The game sets `Input::LockCursor = !paused` every frame and the legacy `UiFocusPointer` clears focus while the cursor is locked. Tests that use focus must pause the game first (`action pause`). The pause menu then holds a legacy focus trap, so `p1_buttons` calls the test-only `focus_trap <id>` command; Phase 2 replaces it with real focus handling.
- The run folder (`Build/StealthAction/windows/GameData`) only receives changed assets when the game re-links. Shaders are not compiled by the build: run `ShaderCompilationAutomation.exe` and copy `compiled/windows` into the run folder by hand.
- `UiComponentRegistry::Discover` only scans `ui/kit/components/` and `ui/components/`. Components that come in through `<imports>` are listed in `uidb.json` from the loaded document cache.
- Static registries that elements touch from destructors (the live-element set) are leaked on purpose, otherwise shutdown crashes.

Verification results:
- p1_layout, p1_visual (probes), p1_text, p1_components, p1_errors, p1_buttons, p1_gamethread (22 loads, no crash) and p1_uidb: RESULT PASS on all four backends.
- Cross-backend: dx12 and vk equal dx11 (buttons.png max difference 2 on dx12). vk buttons.png differs by 118 px (max 10) and gl differs by about 666 px (max 82) in every capture, including pages with no markup text, so it comes from legacy HUD text and not from the new UI. The user accepted these as float/sampling differences between backends.
- Phase 0 regression: `legacy_menus` (9 shots x 4 backends) and `painter_card` (4 backends) are pixel-equal to their baselines.
- UIDB (task 1.18): file appears at launch, two launches are byte-identical, no absolute paths or pointers, lists documents, CSS properties, tags, components (parameters and slots), pseudo classes and test commands; the preview-provider, hot-reload refresh and unwritable-destination checks run in `uidb_selftest`.
- Shaders: 228 compiled, 0 failed for StealthAction; synced to all four games with `Tools/ui/sync_ui_assets.py`. The other three games' shaders were not recompiled.
