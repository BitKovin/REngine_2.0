# Phase 6: UI Studio (separate editor) and retiring the old editor

**Goal:**

- a standalone Qt 6 program for building UI visually
- it renders with the real engine
- it edits the `.xml` / `.css` files, which stay the source of truth
- plus `-ui_schema` and `-ui_lint` for AI and editor completion
- then delete the ImGui `UiEditor` and the JSON `.ui` system

**Prerequisites:** phase 1 (a DOM to edit). It is most useful after phase 4. Qt 6.11 MSVC is installed (Qt Creator kit `Desktop_Qt_6_11_1_MSVC2022_64bit`).

## Task 6.1: `-ui_schema` and `-ui_lint` (engine, headless)

- **`-ui_schema <out.json>`:** writes, then quits:
  - every tag (built-in, component, legacy) with its attributes or params (name, type, default, required), slots and events
  - every CSS property with its grammar summary, `inherited` flag and initial value
  - pseudo-classes and built-in actions
  - generated from the registries and the property table, never hand-written
- **`-ui_lint`:** loads every `.xml` under `GameData/ui/` (recursive: walk `GetFilesInPath` and treat names without an extension as folders) and every `.css` they reference, builds each layout offscreen (not attached), and reports:
  - every markup error and warning
  - missing images, fonts and sounds
  - missing localisation keys (`${KEY}` not found)
  - unresolved handlers that aren't built-ins and aren't registered by a code-behind class for that path
  - `ui_overflow_report` findings with pseudo-loc on
  - writes `SaveData/ui_lint.txt` and quits; the last line is `LINT PASS` or `LINT FAIL <n>`

**Check:** lint passes on all four games. Schema JSON validates with `python -c "import json; json.load(open(...))"`.

## Task 6.2: shared embedding library

1. Move `source/QtEditor/EngineInstance.*` and `EngineViewportWidget.*` into `source/EditorHost/` as a static library (`EditorHost` CMake target, linking `engine` and `Qt::Widgets`).
2. `QtEditor` links it instead of compiling the copies. QtEditor must still build and run, so check by launching it once (it is a GUI; ask the owner before launching).

## Task 6.3: the UiStudio application

`source/UiStudio/`, CMake project like `source/QtEditor/CMakeLists.txt`:

- `GAME_DIR` → the game's sources (decision D2: one Studio build per game)
- `ENGINE_AS_LIBRARY ON`
- links `EditorHost`

### Document model

- `StudioDocument` wraps one `.xml`: a pugixml DOM plus the file text.
- Edit operations: insert node, delete node, move node, set attribute, remove attribute, set text. Each is a `QUndoCommand`.
- After any edit: serialise with `pugi::format_indent` (2 spaces), attributes in their existing order, comments kept, write the file, and let the engine hot-reload it (phase 1 polling).
- CSS edits: `StudioStyleSheet` keeps the token spans from `UiCssTokenize` and replaces **only the value span** of the edited declaration, so the rest of the file stays byte-identical. A new declaration goes in before the rule's `}` on its own line, indented like its neighbours.

### Windows (Qt docks)

| Window | Does |
|---|---|
| Project | tree of `GameData/ui`, filters |
| Canvas | `EngineViewportWidget` showing the document loaded with `ui_load`; overlay drawn by Studio (Qt painter on a transparent widget above): selection, resize handles, margin/padding/content box model, guides; snapping to siblings and parent edges at 6 px |
| Hierarchy | `QTreeView` over the DOM; drag to reorder/reparent; context menu: wrap in Panel, delete, duplicate, extract to component, open definition |
| Components | list from schema (tag, params), drag onto canvas |
| Inspector | attributes per schema (typed editors), class chips, computed style (from the engine, through a new console command `ui_computed <id>` that returns JSON) with the winning rule's `file:line`; editing a value edits that rule (or `style=""` when the value is inline) |
| Code | `QPlainTextEdit` for XML and CSS with `QSyntaxHighlighter`, completion from the schema, error underlines from the engine log lines in `[UI] path:line:col` format |
| States & data | force states on the selection (console `ui_force_state <id> hover 1`), set variables (`set_var`), toggle classes |
| Stack preview | push the document as a page; back; drive navigation |
| Host preview | show the document on a test world surface (the phase 5 test terminal) |

### Engine side for Studio

Add console commands; they are also useful for AI:

- `ui_computed <id>`
- `ui_force_state <id> <state> <0/1>`
- `ui_pick_at <x> <y>`, which logs the element id path and `file:line`

All drag and resize edits become attribute or CSS edits:

- **move:** `x` / `y` (inline `style` unless the element's rule is local to the file and only used once)
- **resize:** `width` / `height`

Prefer inline styles for one-off edits. "Move style to class" in the inspector moves them into the file's `<style>`.

## Task 6.4: retire the old system

1. Delete `source/Engine/UI/Editor/`, `UI/Layout/` (blueprint, serializer, hot reload, loaded instance), and every `.ui` JSON file in the games, after checking with grep that nothing references them:
   - `LoadLayoutFromFile`, `OwnedLayout`, `MemberOfLayout`, `FindNamed`, `UiHotReload`, `ui_editor_*` console commands
   - remove those members from `UiElement`
2. Keep `UI/Reflection/` (legacy tags in markup use it). `UiLayoutSerializer::ApplyProperties` was used by the phase 1 builder: move that function into `UiBuilder.cpp` before deleting `UI/Layout/`.
3. Keep an in-game inspector: `ui_pick` (the next click logs the element path and `file:line`) and `ui_dump` (from phase 1).

**Phase done when:**

- UiStudio builds for StealthAction and can open, edit (canvas drag, inspector, code view) and save `ui/kit/pages/pause.xml` with correct undo
- the game hot-reloads the change
- every regression passes in all four games after the deletions, on every backend

## Implementation notes

(implementer writes here)
