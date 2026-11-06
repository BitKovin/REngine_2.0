# Phase 4: widgets, settings binding, engine kit, game ports

**Goal:**

- markup versions of every interactive widget (wrapping the legacy ones where that saves work)
- scrolling, `Frame` and `ListView`
- two-way settings binding
- the shared engine **kit** (pause, settings, dialogs) used by all four games, with each game keeping only its theme and game-specific pages
- UI and text scale, pseudo-localisation
- RmlUi deleted

The JSON `.ui` layouts and the ImGui editor are deleted in phase 6, not here, because the editor depends on them.

**Prerequisites:** phases 2 and 3.

## Task 4.1: the wrapper pattern (exact)

A wrapper is a `UiElem` that owns one legacy element as a **legacy child**. Phase 1 already draws legacy children through markers and positions them in `UpdateChildrenOffsets`.

1. In the constructor, create the legacy widget and `AddChild` it.
2. In `UpdateChildrenOffsets()` (override, then call the base):
   - `legacy->origin = {0, 0}`, `legacy->pivot = {0, 0}`, `legacy->position = {0, 0}`
   - `legacy->size = ContentRectLocal()` size
   - for legacy widgets whose `GetSize()` ignores `size`, read the class and use the field it uses
3. `ApplyAttribute` maps markup attributes onto legacy fields.
4. In `Tick()`, copy the computed style colours onto legacy colour fields (only the fields the legacy class has).
5. In `Update()` (after the base), compare the legacy value with the last seen value. If it changed: `SetVar("value", …)` on self, then `Fire("valuechanged")`.
6. Measure: `MeasureContent` returns the legacy widget's natural size if it has one, else (0, 0), so CSS must size it.

Read each legacy class completely before wrapping it, and record its fields in the notes.

| Tag | Wraps | Attributes → fields |
|---|---|---|
| `Slider` | `UiSlider` | `min`, `max`, `step`, `value` |
| `DropDown` | `UiDropdown` | `options="a\|b\|c"` (localised items), `selected` (index) |
| `TextEntry` | `UiTextBox` | `text`, `placeholder`, `max-length`, `password` |
| `Movie` | `UiVideo` | `src`, `loop`, `autoplay`, `muted` |
| `ShaderPanel` | `UiCustomShaderImage` | `shader`, plus `param-<name>="x y z w"` → vec4 uniforms |

If a legacy widget has no field for an attribute in this table, drop that attribute and note it.

## Task 4.2: native widgets

- **`ToggleButton`:** a Button. Activate toggles `kStateChecked` (`:checked`) and the var `value` (0/1), then fires `valuechanged`. Attribute `checked`.
- **`RadioButton`:** attribute `group="g"`. Activating checks this one and unchecks the others with the same group **within the same root**.
- **`ProgressBar`:**
  - attribute `value` 0..1 (variables allowed)
  - draws a track (its own box) and a fill child `.fill`, created internally as a `Panel` with `TagName "Panel"` and class `fill`, whose width = value × content width (or height when the attribute `direction="vertical"`)
  - the fill is styleable through `ProgressBar .fill { … }`
- **`CircularProgress`:**
  - attribute `value`
  - paints the `Arc` kind with `start` = attribute `start-angle` (deg, default 0), `sweep = value × 360`, `thickness` = attribute `thickness` (px, default 8)
  - colour from `color`, track from `background-color` (a full ring)
- **`overflow: scroll`** (add the value):
  - the content is laid out unclipped, and the scroll offset is applied as a translation of the children
  - offset range is `0 … max(0, contentExtent − viewport)` per axis
  - input:
    - wheel (`OnScroll`) → 60 px per step, with smooth scrolling (exponential approach, 15/s)
    - pointer drag with capture → direct (kinetic decay 5/s after release)
    - gamepad: when focus moves to a descendant outside the viewport, scroll it into view, with a 16 px margin
  - optional scrollbar: when the attribute `scrollbar="true"`, an internal `Panel.scrollbar > Panel.thumb` is created and styleable
- **`Frame`:**
  - attribute `src` (variables allowed): on change, clear the children and `LoadLayout`-like build the file's top element **as a child** (a nested instance)
  - attribute `snippet="Name"` builds a snippet of the containing file
- **`ListView`:**
  - attributes `snippet="Row"`, `count="{n}"`, `layout="list|grid"`, `item-height` (px, required), `columns` (grid)
  - only visible rows plus 2 extra are built; rows are recycled
  - C++: `std::function<void(int index, UiElem& row)> OnBindRow`
  - a `ListView` must have `overflow: scroll` (warning otherwise)

## Task 4.3: settings binding

### Engine settings registry

`source/Engine/UI/Kit/UiSettings.h`:

```cpp
namespace UiSettings
{
    enum class Kind { Bool, Int, Float, Choice };
    struct Entry { std::string Key; Kind Type; std::function<double()> Get; std::function<void(double)> Set; std::vector<std::string> Choices; double Min = 0, Max = 1, Step = 0; };
    void Register(const Entry& e);
    const Entry* Find(const std::string& key);
    void Snapshot(const std::vector<std::string>& keys, std::vector<double>& out);
    void Restore(const std::vector<std::string>& keys, const std::vector<double>& values);
}
```

### Markup

- `bind="setting:<key>"` on `Slider`, `ToggleButton`, `DropDown` and `RadioButton`.
- When applied: read the value into the widget. On `valuechanged`, call `Set` immediately.
- A page whose subtree has bound widgets:
  - takes a `Snapshot` in `OnPushed`
  - on back with changed values, pushes the kit confirm dialog (`Apply` keeps the values, `Discard` calls `Restore`, `Cancel` stays)
  - this replaces the C++ settings menus' apply/revert logic: read the existing behaviour in `UiVideoSettings.h` first, and match it

## Task 4.4: move the shared settings code to the engine

1. **Diff first.** Compare the four games' `Settings/*` and `UI/Pause/**` files, using hashes and then `git diff --no-index` for the pairs that differ. Expected:
   - identical, except SlasherGame's `GameSettings.h` / `.cpp` (adds `GameplaySettingsData`) and the games' `UiGameSettings.h` / `UiPauseMenu.h`
   - other differences: **STOP** and report them
2. **Move** to `source/Engine/UI/Kit/Settings/`:
   - `VideoSettingsData`, `AudioSettingsData`, `InputSettingsData`, `InputBindingTypes`, `InputActionRegistry`, `InputIconLibrary`
   - a `KitSettings` class equal to the common `GameSettings` (Video, Audio, Input, `ApplyAll`, `Save`, `Load`, `Serialize`, `Deserialize`), plus `std::vector<std::function<…>>` hooks so a game can add sections (SlasherGame's Gameplay section registers through it, with the same INI section name, so existing `GameSettings.cfg` files still load)
3. **Each game:**
   - delete its copies and include the engine headers
   - keep a thin `GameSettings` facade only where game code calls it; prefer changing the call sites (list them in the notes)
   - update `<Game>.vcxproj` / `.filters`
4. **Register every setting** the kit pages use in `UiSettings` (in `KitSettings`), with the same keys as the INI field names (`video.vsync`, `audio.master`, …).

## Task 4.5: kit markup

Canonical files under `source/Engine/UI/Assets/GameData/ui/kit/`, synced to all games:

```
kit/components/   SettingRow.xml Card.xml StyledButton.xml DialogFrame.xml TabBar.xml KeyBindSlot.xml Toast.xml
kit/pages/        pause.xml settings.xml video.xml sound.xml input.xml confirm.xml rebind.xml saveload.xml
kit/styles/       kit.css (structure only: layout, sizes; uses var(--…) for every colour, radius and font)
```

- Code-behind for the kit pages lives in `source/Engine/UI/Kit/` (`KitPauseMenu`, `KitSettingsMenu`, `KitInputPage`, `KitRebindModal`, `KitSaveLoad`). Port the behaviour from the game C++ menus one to one. Read them fully.
- `settings.xml` has a `TabBar` (`hotkey-prev="ui_tab_prev"`, `hotkey-next="ui_tab_next"`) and a `Frame` whose `src` follows the selected tab.
- Games add tabs with `KitSettingsMenu::RegisterTab("${KEY}", "ui/game_settings.xml")`.
- Each game: `GameData/ui/theme.css` defines the variables, and `@import`s `kit/styles/kit.css`. Look values come from the game's current `SettingsStyle` constants (the same colours and sizes).
- `UiMenus::PauseMenu = "ui/kit/pages/pause.xml"` in every game. Restart, Quit and Settings behave as before.
- Delete the games' `UI/Pause/` C++ menus, `UiSettingsStyle.hpp` and the RmlUi `.rml` / `.rcss` files.

## Task 4.6: delete RmlUi

1. Remove `source/Engine/UI/RmlUi/`, the `RmlContext` usage in `EngineMain`, and every game's RmlUi includes and usage (`GameStart.cpp`, `Settings/VideoSettings.h`).
2. Remove the RmlUi and freetype `add_subdirectory` and link lines in the root `CMakeLists.txt`, but **only after** checking nothing else links freetype (grep it). Remove the `rmlui` shader folders from the games.
3. Remove the `Engine.vcxproj` entries.

## Task 4.7: scale, pseudo-loc, lint basics

- **Text scale:** `UiSystem::TextScale` (setting `ui.text_scale`, 0.75–1.5). The root font size = 24 × TextScale (`rem`). Changing it marks every root style-dirty.
- **UI scale:** keep `UiManager::UiScale` as is. Expose it as the setting `ui.scale`.
- **Reduced motion:** setting `ui.reduced_motion` → `UiSystem::ReducedMotion`.
- **`-ui_pseudoloc`:** `UiSubstitute`, after localisation, maps every Latin letter to an accented form (a table of 52 mappings: a→á, e→é, …) and pads the string by 35% with `~`, wrapped in `[` `]`.
- **`ui_overflow_report`** console command: logs every Label whose text layout is wider than its box (when not wrapping) or taller (when wrapping). Run it with pseudo-loc on the kit pages and fix the kit layout until it reports nothing.

## Tests

1. **`p4_widgets`** (StealthAction, all backends):
   - slider via gamepad `ui_right` ×3 → `value` changed
   - DropDown select by confirm/down/confirm
   - TextEntry: test command `type <text>` writes into `Input::TextInputBuffer`
   - Toggle/Radio groups
   - Progress at 0.3 → fill width = 30% of the content (`expect_rect`)
   - CircularProgress probe
   - scroll container: wheel (test command `wheel <delta>`) moves the content by 60 px steps; gamepad focus of a hidden child scrolls it into view (`expect_rect` changes)
   - ListView with count 1000 builds ≤ visible + 2 rows (`expect lv children <= N`)
2. **`p4_settings_bind`:** a slider bound to a test key → `Set` is called; back with a change shows the confirm dialog; Discard restores.
3. **`kit_menus` per game** (all four games, all backends): pause → settings → each tab → back → resume, with `ui_screenshot` at each step. **STOP** for owner approval of each game's screenshots. They replace the phase 0 legacy goldens: store the approved run as label `kit_base` for later regressions.
4. All four games build without RmlUi, and their earlier test scripts still pass (the legacy-menu goldens are retired: note that in the README).

## Implementation notes

(implementer writes here)
