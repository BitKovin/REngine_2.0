# UI rework plan: Panorama-style markup

Status: proposal (revision 2, 2026-11-06). Nothing implemented yet. Revision 2 also covers interactive world-space screens (§9). There are **no special root element types**: the menu stack, the HUD layer and world surfaces are *hosts* that can show any element (§7–§9).

Revision 2 follows the owner's direction:

- Replace the JSON `.ui` layout system and the ImGui `UiEditor` with **Panorama-like markup**: XML treated like HTML, plus CSS stylesheets and C++ code-behind.
- Add **new element types (`UiElem*`) that wrap existing elements** where that saves work.
- Build a **separate editor program**.
- Menus are very easy to make and run on a **modal stack**.
- The HUD is easy and has its own logic.
- AI can use it reliably.
- RmlUi is deprecated (unpleasant to use) and gets deleted once nothing needs it.

---

## 1. Decisions at a glance

| Topic | Decision |
|---|---|
| Layout files | `.xml` under `GameData/ui/`. Panorama structure: `<root>` with `<imports>`, `<styles>`, `<snippets>`, then one top element |
| Reuse | **Components**: any XML file can define a tag (`<Component tag="SettingRow" params="label">`) with parameters, slots (`<Children/>`), scoped ids and scoped styles. Layouts include each other freely |
| Styles | Separate `.css` files (`<include>`, `@import`) **or** inline `<style>` blocks in the XML, both in one cascade. A **real CSS syntax subset** with Panorama's layout properties (`flow-children`, `fit-children`, `fill-parent-flow`, `horizontal-align`) |
| Logic | C++ code-behind classes. Markup events call named handlers (`onactivate="Restart()"`) plus a few built-in actions (`Back()`, `Push('…')`, `Quit()`). No scripting language |
| Data | Variables in text and attributes: `text="{ammo} / {ammo_max}"`, `value="{hp}"`, set from C++ with `SetVar`. Localisation keeps the engine's `${KEY}` |
| Elements | New `UiElem*` family (tags without the prefix: `<Panel>`, `<Label>`, `<Image>`…). Simple visuals are native. Complex widgets **wrap** the existing `UiSlider`, `UiDropdown`, `UiTextBox`, `UiVideo`… |
| Hosts | No special root types. Any element (a loaded layout or a C++ element) can be pushed on the **menu stack**, added to the **HUD layer**, or put into a **world surface**. Host options (`pauses-game`, `close-on-back`…) are optional attributes on the root element. Lifecycle hooks and `Tick()` are ordinary `UiElem` virtuals |
| World UI | `UiWorldSurface` component: screens on planes or mesh material slots, with its own input context and menu stack, aim/focus interaction, gameplay hooks |
| Compatibility | One tree: `UiElem` derives from `UiElement`. Old C++ widgets keep working, can sit inside markup, and markup can be added to old trees |
| Rendering | New CSS-capable `UiRenderer` (§10): cached per-element display lists, a small base shader plus dedicated effect programs and multi-pass effects, a primitive data texture (WebGL2/GLES3-safe), SDF boxes (per-corner radius, per-side borders, gradients, analytic shadows), shader clips and gradient masks, free colour-matrix filters, SDF text, offscreen layer atlas for blur, drop-shadow, group opacity and world surfaces. Legacy `UiRenderer::Draw*` calls record into it |
| Editor | **UI Studio**, a separate Qt 6 program that embeds the real engine renderer (reusing QtEditor's `EngineInstance`/`EngineViewportWidget`). Visual canvas, tree, inspector, and live XML/CSS text views kept in sync |
| Removed at the end | JSON `.ui` loader, ImGui `UiEditor`, RmlUi |

---

## 2. What exists today (why change)

- **Look:** flat colour or 9-slice texture, and hover swaps the colour instantly. There are no rounded corners, borders, transitions or press feedback, which is most of the dated feel.
- **Draw calls:** every element is its own `bgfx::submit`, and each builds an `unordered_map<string, vec4>` of uniforms and looks up its shader by string. Text with effects is one draw call per glyph.
- **Text:** a 32 MB RGBA atlas per font and size. Everything is rasterised at 200 px and scaled down with no mipmaps, so small text shimmers. `UiText::GetSize()` re-localises and re-measures on every call.
- **Layout:** only V/H boxes with a single gap. Content boxes size from last frame's child bounds, and layout runs about three passes per frame. Hit testing uses last frame's layout.
- **Authoring:** JSON `.ui` files (verbose, one number per line) and a bare ImGui editor. Only 2 layouts per game use it. Almost all UI is hand-built C++.
- **Reuse:** `UiSettingsMenu.h` is byte-identical in 4 games. About 3,000 lines of pause/settings UI are copied per game, and each game has its own `SettingsStyle` constants.
- **Menus:** each menu adds itself to `Viewport`, hides its parent by hand, and sets `FocusTrap` (36 places). There is no stack, shared back behaviour, or focus restore.
- **HUD:** ad hoc. `UiElement::WorldToScreenSpace` returns `vec2()`, so it is broken.

What to keep: the `UiElement` tree with anchors and rotation, `UiNavigation`'s spatial gamepad navigation, the reflection registry, mtime hot reload, UI drawn into its own frame buffer, and world-space UI via `UiBilboard`.

---

## 3. What it looks like

### 3.1 Pause menu: one XML file, one CSS file, about 10 lines of C++

`GameData/ui/pause.xml`

```xml
<root>
  <styles>
    <include src="ui/styles/theme.css"/>
  </styles>

  <Panel class="pause" pauses-game="true" hides-hud="true">     <!-- host options, read by the menu stack -->
    <Panel class="scrim"/>
    <Panel class="menu">
      <Label class="title" text="${PAUSE_MENU_TITLE}"/>
      <TextButton text="${PAUSE_MENU_RESUME}"   onactivate="Back()" default-focus="true"/>
      <TextButton text="${PAUSE_MENU_SETTINGS}" onactivate="Push('ui/settings.xml')"/>
      <TextButton text="${PAUSE_MENU_RESTART}"  onactivate="Restart()"/>
      <TextButton text="${PAUSE_MENU_QUIT}"     onactivate="Push('ui/confirm_quit.xml')" class="danger"/>
    </Panel>
  </Panel>
</root>
```

`GameData/ui/styles/theme.css`

```css
:root {
    --accent: #c84a3a;
    --panel: rgba(18, 9, 9, 0.95);
}

.scrim { width: 100%; height: 100%; background-color: rgba(10, 2, 2, 0.8); }

.menu {
    flow-children: down;
    horizontal-align: center;
    vertical-align: center;
    gap: 12px;
    padding: 32px;
    background-color: var(--panel);
    border-radius: 14px;
    box-shadow: 0 12px 40px rgba(0, 0, 0, 0.6);
}

.title { font-size: 56px; color: #f0d8c8; horizontal-align: center; margin-bottom: 16px; }

TextButton {
    width: 400px;
    height: 70px;
    border-radius: 10px;
    background-color: #2a1512;
    border: 2px solid rgba(200, 74, 58, 0.4);
    sound: "ui_click";
    transition: background-color 0.12s ease-out, border-color 0.12s, transform 0.08s ease-out;
}
TextButton:hover  { background-color: #3a1a16; border-color: var(--accent); }
TextButton:active { transform: scale(0.97); }
TextButton:focus  { outline: 3px solid var(--accent); }
TextButton Label  { horizontal-align: center; vertical-align: center; font-size: 34px; }
.danger           { background-color: #731f1f; }

.page          { transition: opacity 0.18s ease-out, transform 0.18s ease-out; }   /* the stack adds "page" to what it pushes */
.page:entering { opacity: 0; transform: translateY(24px); }
.page:leaving  { opacity: 0; }
```

`PauseMenu.cpp`: only what markup can't do on its own. Code-behind is any `UiElem` subclass that loads a layout onto itself.

```cpp
class PauseMenu : public UiElemPanel
{
public:
    PauseMenu()
    {
        LoadLayout("ui/pause.xml");
        On("Restart", [] { UiMenus::Clear(); Level::ReloadCurrent(); });
    }
};
UI_REGISTER_CODE_BEHIND("ui/pause.xml", PauseMenu);   // Push('ui/pause.xml') now constructs a PauseMenu
```

The game sets `UiMenus::PauseMenu = "ui/pause.xml";` once. The engine then opens it on the `pause` action, and back/Esc/B closes it. `ui/confirm_quit.xml` needs no C++ at all, because its buttons call `Back()` and `Quit()`.

### 3.2 HUD: markup plus a `Tick()` that sets variables

`GameData/ui/hud/player.xml`

```xml
<root>
  <styles>
    <include src="ui/styles/hud.css"/>
  </styles>

  <snippets>
    <snippet name="EnemyBar">
      <WorldPanel class="enemy-bar" world-offset="0 2.1 0" fade-distance="25">
        <ProgressBar class="hp" value="{hp}"/>
      </WorldPanel>
    </snippet>
  </snippets>

  <Panel class="player-hud">
    <Panel id="vitals">
      <ProgressBar id="health" value="{hp}"/>
      <CircularProgress id="dash" value="{dash_cooldown}"/>
    </Panel>
    <Label id="ammo" text="{ammo} / {ammo_max}"/>
    <Panel id="markers"/>
  </Panel>
</root>
```

```cpp
class PlayerHud : public UiElemPanel
{
public:
    explicit PlayerHud(Player* p) : player(p) { LoadLayout("ui/hud/player.xml"); }

    void Tick() override                       // once per frame, main thread, before layout
    {
        SetVar("hp", player->Health01());      // a no-op if unchanged
        SetVar("ammo", player->Ammo());
        SetVar("ammo_max", player->AmmoMax());
        SetVar("dash_cooldown", player->DashCooldown01());
        SetClass("low-health", player->Health01() < 0.25f);   // CSS pulses the vitals
    }

private:
    Player* player;
};

UiLayers::Hud().AddChild(std::make_shared<PlayerHud>(player));

// enemy bars: one snippet per enemy, positioned in the world every frame
auto bar = hud->AddSnippet("EnemyBar", hud->Find("markers"));
bar->As<UiElemWorldPanel>()->Follow(enemyHandle);
bar->SetVar("hp", 1.f);
```

---

## 4. Markup

### 4.1 File structure (Panorama's)

```xml
<root>
  <imports>                                              <!-- optional: components used by this file (§4.3) -->
    <import src="ui/components/SettingRow.xml"/>
  </imports>
  <styles>                                               <!-- optional, applied in the order written -->
    <include src="ui/styles/theme.css"/>                 <!-- separate file, shared -->
    <style>                                              <!-- inline, for styles only this file needs -->
      .menu { gap: 12px; }
    </style>
  </styles>
  <snippets> <snippet name="Row"> … </snippet> </snippets>  <!-- optional, file-local templates for code -->
  <Component | Panel | any element …> … </…>               <!-- exactly one top element -->
</root>
```

- Parsed with **pugixml**, vendored into `includedLibraries` (MIT, two files, keeps comments and attribute order, and gives byte offsets for `file:line` errors).
- **Styles can be separate or inline, as the author likes.** Shared looks go in `.css` files (`<include>`), and `.css` files can `@import` each other (`theme.css` imports `colors.css`, `buttons.css`). One-off styles can sit in a `<style>` block in the XML. Both feed one cascade in the order written, and both hot-reload.
- Common attributes on every element: `id`, `class`, `style` (inline CSS), `hittest` (`true`/`false`), `focusable`, `default-focus`, `visible`, and event attributes (§6).
- Attribute values can use variables (`{name}`) and localisation keys (`${KEY}`).
- Unknown tag, unknown attribute or a malformed value: an error with `file:line` at load time. Never silent.

### 4.2 Elements

Tags drop the C++ prefix. In C++, the new family is `UiElem*`. Plain `Ui*` classes are the legacy ones.

| Tag | C++ | Implementation |
|---|---|---|
| `Panel` | `UiElemPanel` | native: the box (background, border, radius, shadow, layout) |
| `Label` | `UiElemLabel` | native, new text path: wrap, align, ellipsis, rich text `<b>`, `<color>`, `<icon=interact>` |
| `Image` | `UiElemImage` | native: `src`, `scaling: stretch / contain / cover / none`, 9-slice via `image-slice` |
| `Button`, `TextButton` | `UiElemButton` | native: focusable Panel with activate, hover and pressed. `TextButton` adds a Label from `text` |
| `ToggleButton`, `RadioButton` | `UiElemToggle` | native: `:selected`, `group="…"` for radios |
| `ProgressBar`, `CircularProgress` | `UiElemProgress` | native SDF bar and arc, `value` 0..1, fill styled with CSS |
| `Slider` | `UiElemSlider` | **wraps** `UiSlider` |
| `DropDown` | `UiElemDropDown` | **wraps** `UiDropdown` |
| `TextEntry` | `UiElemTextEntry` | **wraps** `UiTextBox` (caret and IME logic reused) |
| `Movie` | `UiElemMovie` | **wraps** `UiVideo` |
| `ShaderPanel` | `UiElemShader` | **wraps** `UiCustomShaderImage`. `shader="ui/fs_x"`, with standard uniforms (size, time, colour, radius) |
| `WorldPanel` | `UiElemWorldPanel` | native HUD: follows a world point or entity, edge clamp with arrow, distance fade and scale |
| `Frame` | `UiElemFrame` | embeds a whole layout with no parameters. `src` can be a variable (tab pages) |
| `Children` | n/a | slot marker inside a `<Component>` (§4.3) |
| any component tag, e.g. `<SettingRow>` | `UiElemComponent` (or its registered code-behind) | defined by an XML file (§4.3) |
| any registered legacy type, e.g. `<UiJoystick/>` | the legacy class itself | placed in the parent's content box with its own anchor fields as attributes. Not styled except size and position |

**How a wrapper works.** A wrapper owns the legacy element as an internal child stretched over its content box. It maps attributes and computed style onto the legacy fields (colours, value, range), and forwards the legacy callbacks as markup events (`onvaluechanged`). The markup API is fixed, so a wrapped widget can later be rewritten natively without touching any `.xml`.

Games add their own tags: `UI_REGISTER_ELEM("WeaponSlots", UiElemWeaponSlots)`. Every registered tag publishes its attributes to the schema (§13).

### 4.3 Components: layouts that include each other

Any XML file can define a reusable building block, and any layout can use it as a tag. This is how the settings rows, cards, styled buttons and dialog frames that every game copies today become shared pieces.

**Defining one.** The top element is `<Component>`, with its tag name and parameters:

```xml
<!-- ui/components/SettingRow.xml -->
<root>
  <styles>
    <include src="ui/styles/theme.css"/>
    <style>
      .row   { flow-children: right; width: fill-parent-flow(1); height: 90px; padding: 0 24px; }
      .label { vertical-align: center; width: fill-parent-flow(1); font-size: 27px; }
      .hint  { font-size: 20px; color: var(--caption); }
      .row:focus-within { background-color: var(--row-highlight); }
    </style>
  </styles>

  <Component tag="SettingRow" params="label, hint=''">
    <Panel class="row">
      <Panel class="texts">
        <Label class="label" text="{label}"/>
        <Label class="hint"  text="{hint}" visible="{hint}"/>
      </Panel>
      <Children/>                                     <!-- the instance's own children go here -->
    </Panel>
  </Component>
</root>
```

**Using it.** It works like any built-in tag, and components can use other components:

```xml
<Panel class="video-page">
  <SettingRow label="${VIDEO_VSYNC}">
    <ToggleButton id="vsync"/>
  </SettingRow>
  <SettingRow label="${VIDEO_FOV}" hint="${VIDEO_FOV_HINT}" class="advanced">
    <Slider id="fov" min="60" max="120"/>
  </SettingRow>
</Panel>
```

How it works:

- **Parameters** are declared in `params`, with optional defaults (`hint=''`). They are passed as attributes on the instance and become that instance's **variables**, using the same `{name}` mechanism as everywhere else. A parameter can therefore also be set later from C++ with `SetVar`. A missing required parameter or an undeclared attribute is a `file:line` error.
- **Standard attributes** on the instance (`id`, `class`, `style`, `visible`, events) apply to the component's root, and its classes merge with the component's own.
- **Slots.** `<Children/>` marks where the instance's children go. Named slots cover several insertion points: `<Children slot="footer"/>` in the component, and `<Panel slot="footer">…</Panel>` in the instance. Without a `<Children/>`, giving an instance children is an error.
- **Ids are scoped per instance**, as `LoadedLayoutInstance` already does today. Two `SettingRow`s can both have an internal `label`. Slotted children belong to the *using* file, so `Find("vsync")` from the page finds the toggle directly. A path reaches inside an instance when needed: `Find("fov_row/label")`.
- **Styles are scoped by prefix, not isolated.** Rules from a component file are rewritten to match only inside that component (`.row` in `SettingRow.xml` effectively becomes `SettingRow .row`), so they never leak. They come earlier in the cascade than the using file's styles. A page can still restyle a component on purpose (`.advanced .label { color: var(--caption); }`), and theme variables reach everywhere.
- **Events out of a component.** A component raises its own events: `Raise('changed')` in markup, or `Raise("changed")` in C++. The instance handles them with an attribute: `<ColorPicker onchanged="ApplyTint()"/>`.
- **Optional C++ code-behind**: `UI_REGISTER_COMPONENT("SettingRow", UiSettingRow)` when a component needs logic (a rebind slot, a colour picker). Without it, a component is pure markup.
- **Discovery.**
  - `Engine/UI/Kit/components/` and each game's `GameData/ui/components/` are scanned at startup, so their tags work everywhere with no import.
  - Files anywhere else are pulled in with `<import>`.
  - Two components claiming one tag, or a component that includes itself (directly or through another), is an error naming both files.
- **Hot reload.** Saving a component file rebuilds every live instance of it.
- **Creating from code**: `parent->AddComponent("SettingRow", {{"label", "${AUDIO_MASTER}"}})`, for lists built at runtime.

Related tools:

- `<snippet>` is a **file-local component**: the same parameters, slots and scoping, but only visible inside its own file. Use it for list rows a screen builds from code, which nothing else will reuse.
- `<Frame src="ui/widgets/minimap.xml"/>` embeds a whole layout with no parameters, and its `src` can change at runtime (`src="{current_tab_page}"`).

---

## 5. Styles (CSS subset)

### 5.1 Syntax

- Selectors: `Type`, `.class`, `#id`, `:state`, descendant (`A B`), child (`A > B`), and selector lists (`A, B`). No sibling or attribute selectors. Specificity and source order follow CSS.
- States: `:hover :active :focus :focus-within :disabled :selected :entering :leaving`. Classes cover everything else.
- `--var` / `var()` custom properties, `@keyframes`, `@media (max-aspect: 1.5)` for narrow screens. No other at-rules.
- Inherited like CSS: `color`, `font-*`, `text-*`, `letter-spacing`, `line-height`, and `opacity` multiplies down the tree.
- Own parser of about 800 lines. It keeps a source span for every declaration, so the editor can edit a value in place without reformatting the file, and errors point at `file:line`.

### 5.2 Layout properties: Panorama's model

Panorama's layout is simpler than flexbox and maps one to one onto the anchors and V/H boxes the engine already has:

| Property | Values | Today's equivalent |
|---|---|---|
| `flow-children` | `none` (default: children overlay), `down`, `right`, `down-wrap`, `right-wrap` | `none` = anchored children, `down` = `UiVerticalBox`, `right` = `UiHorizontalBox` |
| `width`, `height` | `120px`, `50%`, `fit-children` (default), `fill-parent-flow(1)`, `height-percentage(100%)` (aspect lock) | `size`, `parentRelativeScaling` |
| `min-width`, `max-width`, … | lengths | none |
| `horizontal-align`, `vertical-align` | `left`/`center`/`right`, `top`/`center`/`bottom` | `origin` plus `pivot` |
| `x`, `y` | offset lengths | `position` |
| `margin`, `padding`, `gap` | CSS shorthands. `gap` isn't in Panorama but is too useful to skip | `ContentDistance` |
| `overflow` | `squish`, `clip`, `scroll` (uses `UiScrollRegion` logic), `noclip` | `UiScrollRegion`, masks |
| `visibility` | `visible`, `collapse` (removed from layout) | `visible` |
| `z-index` | integer | child order |

Layout runs as measure (bottom-up) and then arrange (top-down), with dirty flags, in the same frame. Rotation and `worldMatrix` are computed in arrange as now.

### 5.3 Visual properties

The full list of supported properties, and how each is drawn, is in §10.13. In short: backgrounds with gradients and images, borders and per-corner radii, box and text shadows, outlines, opacity, filters, backdrop blur, masks, 2D transforms, and full text styling.

Game-specific extras beyond CSS:

- `wash-color`: Panorama's tint over everything, including images
- `sound` and `sound-hover`: Panorama-style sound hooks, so every button sounds the same without code
- `cache: layer`: render this subtree offscreen and reuse it (§10.11)
- `font-render: bitmap`: keep a hinted bitmap path for very small text (§10.9)

Values: `px`, `%`, `em`, `rem` (follows the player's text-size setting), `vw`, `vh`, `calc()`, `min()`, `max()`, `clamp()`, `var()`.

### 5.4 Precedence

From highest to lowest:

1. Values set from C++ on the element (`elem->Style().backgroundColor = …`)
2. The `style="…"` attribute
3. Stylesheet rules by specificity
4. Inherited values
5. Defaults

Styles resolve only when classes, states, the tree or a sheet change, and are cached per element. Transitions tick only the elements that are animating.

---

## 6. Events and code-behind

### 6.1 Events in markup

`onactivate` (click, tap or gamepad confirm), `onmouseover`, `onmouseout`, `onfocus`, `onblur`, `onvaluechanged`, `oncancel`, `onload`.

The value is a list of calls separated by `;`. Each call is a name with literal arguments. No expressions and no scripting:

```xml
<TextButton onactivate="PlaySound('ui_confirm'); Push('ui/settings.xml')"/>
```

Names resolve in this order:

1. Handlers registered with `On(name, fn)` on the nearest ancestor with code-behind (any C++ `UiElem` subclass that registered them), then on the host (for example a world surface component)
2. Built-in actions:
   - Stack (the stack the screen is on): `Back()`, `Push(path)`, `Replace(path)`, `Pop()`, `Clear()`. `PushMenu(path)` always targets the main stack
   - App: `Quit()`
   - Other: `PlaySound(name)`, `AddClass(sel, c)`, `RemoveClass(sel, c)`, `ToggleClass(sel, c)`, `SetFocus(sel)`, `Raise(event)` (a component firing its own event, §4.3)

A name that doesn't resolve is reported with `file:line` when the screen is pushed, not only when it is clicked.

### 6.2 C++ API on `UiElem`

```cpp
T*      Find<T = UiElem>(std::string_view id);       // within this layout; logs on miss/wrong type
void    On(std::string_view handler, std::function<void()> fn);
void    On(std::string_view handler, std::function<void(UiElem& sender)> fn);
void    SetVar(std::string_view name, float | int | std::string_view);   // inherited down the tree
void    SetClass(std::string_view cls, bool on);  void AddClass(…); void RemoveClass(…); bool HasClass(…);
UiElem* AddSnippet(std::string_view snippet, UiElem* parent = nullptr);
void    ClearChildren();
UiStyle& Style();                                   // C++ overrides, highest precedence
```

**Variables.** `{name}` in text and attributes is replaced from the nearest ancestor that set it, using `{name:%.1f}` or `{time:mm:ss}` for formatting. Localisation runs first, so a translated string can contain variables (`"Press {key} to open"`). Setting a variable to the same value costs a compare and nothing more.

---

## 7. Menus: the modal stack

There is no special page type. **Any element can be pushed**: a layout loaded by path, or any C++ `UiElem`, including code-behind classes. The stack is the host, and it gives the element its menu behaviour.

```cpp
// Lifecycle hooks are ordinary UiElem virtuals (no-ops by default), so any code-behind class can use them.
class UiElem : public UiElement
{
    // ...
    virtual void OnPushed() {}   virtual void OnShown() {}     // pushed / top of a stack again
    virtual void OnHidden() {}   virtual void OnPopped() {}    // covered by another push / removed
    virtual bool OnBack() { return false; }                    // true = swallow (e.g. "discard changes?")
    virtual void Tick() {}                                     // every frame, main thread, before layout
    UiMenuStack* Stack() const;                                // the stack this element's page is on, or nullptr
};

class UiMenuStack : public UiElement                          // one per UiContext (§9.2)
{
public:
    UiElem* Push(std::string_view layoutPath);                // registered code-behind class if any, else a plain element
    UiElem* Push(std::shared_ptr<UiElem> page);
    void Pop();   void PopTo(UiElem* page);   void Replace(std::string_view path);   void Clear();
    UiElem* Top();   bool IsEmpty();
};

namespace UiMenus                                             // the main stack, for the common case
{
    UiMenuStack& Stack();
    UiElem* Push(std::string_view layoutPath);   void Pop();   void Clear();   bool IsOpen();
    inline std::string PauseMenu;                             // layout opened by the "pause" action when set
}
```

**Host options** are optional attributes on the pushed root element. The stack reads them, and any other host ignores them:

| Attribute | Default | Meaning |
|---|---|---|
| `close-on-back` | `true` | back pops this page |
| `shows-below` | `false` | keep drawing the pages below (dialogs) |
| `pauses-game` | `false` | main stack only: pause while this page is on the stack |
| `hides-hud` | `false` | main stack only: fade the HUD out while this page is on the stack |

The stack adds the class `page` to every element it pushes, so one theme rule animates all pages (`.page:entering`).

Each stack handles, once, everything each menu does by hand today:

- **Input and focus:** only the top page gets input and focus. Pushing remembers the focused element, and popping restores it. `default-focus` picks the first focus for gamepad.
- **Back:** `ui_cancel`, Esc and B call `OnBack()`, then pop if `close-on-back` is set. If the stack is now empty, its host handles back: the main stack closes, and a world surface leaves focus mode.
- **Pages below:** with `shows-below="false"` the pages below are hidden (and cost nothing). With `true` they stay drawn, for dialogs, with a dim from the stylesheet.
- **Animation:** push plays `:entering → normal`. Pop plays `normal → :leaving`, then the page is destroyed.

Only the **main** stack also does:

- **Pause:** the game is paused while any `pauses-game` page is on it. This goes through one hook, `UiMenus::OnPauseChanged`, which each game's `PauseGameManager` subscribes to.
- **HUD:** the HUD fades out (through its own transition) while any `hides-hud` page is up.

Built-in actions in markup (`Push`, `Back`, `Replace`, `Pop`, `Clear`) act on **the stack the element is on**. A terminal's "Admin" button that does `Push('ui/world/terminal_admin.xml')` opens a sub-page on that terminal, not a full-screen menu. `PushMenu(path)` explicitly targets the main stack (for example a shop terminal that opens the full-screen inventory).

---

## 8. HUD

- `UiLayers::Hud` sits under the menu stack and `UiLayers::Overlay` (toasts, tooltips, loading) sits over it. Old `Viewport.AddChild(...)` code keeps working beside the layers.
- The HUD layer hosts **any element**: `UiLayers::Hud().AddChild(element)`. A HUD's code-behind is an ordinary `UiElem` subclass, and its `Tick()` (a `UiElem` virtual) runs once per frame on the main thread, before layout. Nothing on the HUD layer takes focus or hit-tests unless it sets `hittest="true"` (mobile controls).
- Data flows one way, game to HUD: `SetVar` and `SetClass` in `Tick()`. There is no binding registry to keep in sync.
- `WorldPanel`: follows a `vec3` or an `EntityHandle`, with `world-offset`, `clamp-to-edge` (with an optional arrow child that rotates toward the target), `fade-distance` and `scale-with-distance`. It replaces per-game `WorldToScreenSpace` code.
- HUD juice is just CSS: `@keyframes` for damage flash and low-health pulse, transitions on bar values (`transition: value 0.2s` smooths a health bar). Any shake follows the camera-comfort rules (low frequency, bounded).

---

## 9. World-space UI and interactive screens

UI also lives in the world: terminals, keypads, vending machines, elevator panels, in-world signs, and labels above NPCs. The player interacts with them during gameplay. Today `UiBilboard` renders a `UiCanvas` into a shared render-texture atlas on a plane, but it takes no input and re-renders every frame.

### 9.1 Same markup everywhere

A world screen is a `UiWorldSurface`: a component on an entity that **draws whatever UI element is put into it**. There is no special markup type. The content is any layout or C++ element, with the same tags, CSS, events, variables and snippets as everywhere else. A layout built for a menu works on a terminal and the reverse, and Studio edits both the same way. The content root's CSS `width`/`height` become the surface's pixel resolution.

```xml
<root>
  <styles><include src="ui/styles/terminal.css"/></styles>
  <Panel class="door-terminal">     <!-- terminal.css: .door-terminal { width: 512px; height: 320px; } -->
    <Label class="status" text="${DOOR_STATUS_{door_state}}"/>
    <TextButton id="open"  text="${DOOR_OPEN}"  onactivate="OpenDoor()"/>
    <TextButton id="admin" text="${DOOR_ADMIN}" onactivate="Push('ui/world/door_admin.xml')"/>  <!-- sub-page on this terminal -->
    <Label class="code" text="{entered_code}"/>
  </Panel>
</root>
```

```cpp
// gameplay side: a component on the terminal entity
class DoorTerminal : public UiWorldSurface                 // component, like StaticMesh today
{
public:
    DoorTerminal(Entity* owner) : UiWorldSurface(owner, "ui/world/door_terminal.xml")
    {
        On("OpenDoor", [this] { if (door) door->Open(); });  // markup event -> gameplay
    }
    void Tick() override { SetVar("door_state", door->StateName()); }   // gameplay -> screen
    EntityHandle door;
};
```

Handlers resolve from the content's own code-behind first, then the surface component's `On()`, then built-ins. The same `door_terminal.xml` can therefore be reused by different entities, each wiring `OpenDoor` to its own door.

### 9.2 `UiWorldSurface`: a host with its own context and stack

- A component on an `Entity`. It owns a `UiContext` with its own `UiMenuStack`. The content element (`SetContent(path)` or `SetContent(element)`) is the bottom page of that stack, so `Push` from inside the content opens sub-pages on the surface. It renders into the **layer atlas** (§10.11, the same atlas allocator `UiBilboard` already has) and is shown either on a **plane** or on **a material slot of any mesh**, for example the screen of a computer model, via the mesh's UVs.
- Facing: `fixed` (diegetic panels), `billboard` (faces the camera, as `UiBilboard` does), or `billboard-y` (upright signs).
- Size: the top page's pixel size plus `pixels-per-meter`. The texture has mipmaps, so a far screen doesn't shimmer.
- Drawn as a world mesh: depth-tested, unlit or emissive (`emissive: 1.5` makes screens glow in the dark), with an optional per-surface post shader (scanlines, CRT curvature, flicker) through the existing `PixelShader` slot.

**Architectural change: `UiContext`.** Focus, hover, pointer capture, navigation state, stylesheet scope and variable root move out of statics (`UiNavigation::Focused`) into a `UiContext`. The screen (stack, HUD and overlay) is one context, and each world surface is its own. Two terminals then keep separate focus and hover, and a terminal keeps its state while a menu is open. `UiNavigation::Focused` stays as an alias for the screen context's focus, so existing code compiles.

### 9.3 Interaction

The input router decides each frame who gets input:

1. Menu stack top
2. The surface in focus mode
3. The aimed-at surface
4. HUD and gameplay

There are two interaction modes per surface (`interaction="aim | focus | both | none"`):

| Mode | How it works | Use for |
|---|---|---|
| **Aim** | Each frame the engine casts the player's aim ray (crosshair, or the mouse ray when the cursor is free) against the surface's collider. The hit's UV becomes a pointer position in surface pixels and drives `:hover` as on screen. The **use** action (or a configurable action) presses and activates. A virtual cursor can be drawn on the surface (`cursor: auto`) | Keypads, buttons and quick panels you poke during play |
| **Focus** | Use on the surface (or `FocusSurface(this)` from code) blends the camera to a framing point, frees or locks the cursor to the surface, and routes mouse, keyboard and gamepad (with spatial navigation) to that surface's context. `ui_cancel` leaves. Optional `pauses-game` | Terminals, shops, computer screens with text entry |

Gameplay hooks, so the screen and the game cooperate:

- `UiWorldSurface::OnAimEnter/OnAimLeave` and an `IsAimingAtInteractive()` query. The HUD can show "[E] Use", and a weapon can skip firing while a button is under the crosshair (`consumes-use="true"`).
- `max-distance` and `max-angle` for interacting, with `:out-of-range` as a state so CSS can dim the screen when you're too far.
- Events from markup reach gameplay through `On()` on the component, which has the entity, other components and `EntityHandle`s at hand. Gameplay pushes data back with `SetVar` and `SetClass` in `Tick()`.
- Any 3D ray or point can drive a surface (`InjectPointer(rayOrigin, rayDir, pressed)`). NPC "using" a screen, physics objects pressing buttons and later VR hands all go through this one call.

### 9.4 Cost control (many screens in a level)

- **Re-render only when dirty.** A surface's draw data is hashed after layout. If nothing changed (no transition running, no variable changed), the atlas region is reused and costs nothing.
- **Skip when not visible.** A frustum-culled or occluded surface doesn't tick layout or render, and catches up when seen again.
- **Distance update rate.** Full rate near the player, then `update-rate: 10hz` beyond a distance, then frozen beyond `max-update-distance`.
- Surfaces keep game time. While the game is paused they get no input but still draw.

### 9.5 Screen-space vs world-space, side by side

| | `WorldPanel` (§8) | `UiWorldSurface` (§9) |
|---|---|---|
| Where it draws | Screen, positioned at a projected world point | In the world, on a plane or mesh, depth-tested |
| Size | Screen pixels, readable at any distance | Metres, perspective-scaled |
| Input | None by default | Aim, focus, or injected rays |
| Use for | Enemy bars, objective markers, damage numbers | Terminals, keypads, signs, labels that belong to the object |

`UiBilboard` stays as the compatibility name. It becomes a thin `UiWorldSurface` with `facing: billboard`, `interaction: none`, and its existing `Canvas`/`ContentBox` fields, so `NpcHumanBase` and `VideoInWorldTest` keep working.

---

## 10. Rendering: a CSS-capable `UiRenderer`

The renderer has to draw most of what CSS can describe, cheaply, on every platform the engine ships on, and keep every existing `Draw()` override working.

### 10.1 Constraints

- **WebGL2 / GLES3 is the floor.** The engine builds for Emscripten and Android (MSAA is already disabled there). That means no storage buffers, no bindless textures and no geometry shaders. Everything below uses vertex buffers, `texelFetch` from float textures, and ordinary 2D textures, all of which bgfx supports on those back ends.
- **A small base shader plus dedicated effect programs.**
  - The base program (`ui2/vs_ui2`, `ui2/fs_ui2`) draws the cheap, common primitives: images, boxes with borders and gradients, SDF glyphs, arcs.
  - Expensive effects get **their own programs**, starting with box shadows. Pixels of an ordinary box then never pay for an effect's loop.
  - Effects that need neighbouring pixels (blur, glow, drop-shadow, backdrop) use **multiple passes** with dedicated shaders (§10.8, §10.11). That gives better quality than single-pass approximations, at lower cost.
  - The batch key includes the program. The draw-list compiler merges a batch into an earlier batch with the same key when nothing in between overlaps it, so alternating programs don't multiply draw calls (§10.4).
- **Premultiplied alpha** everywhere. This matches the existing `BgfxStateManager::Blend::Premultiplied` UI pass.
- **Colours and gradients interpolate in sRGB**, as CSS does, so a colour picked in a browser or Studio looks the same in game.
- **The UI keeps drawing into its own `UiFrameBuffer`**, which is composited over the scene exactly as today. That target is **always 1080p-tall** (`UiManager::GetScaledUiHeight()`, width by aspect), and this stays. One UI unit is one target pixel.
- **Threading**, the same model as the legacy UI's `finalized*` fields:
  - game-thread code may use the whole UI API during the game update (load, add and remove elements, set variables and classes); those calls only change state
  - the main thread processes the UI while the game thread is idle (input, style, layout, recording), then **finalizes a copy** of each root's draw data
  - rendering reads only those copies, so it never races with game code
- **Thin elements must come out right at that resolution:**
  - edges, borders and text baselines snap to whole target pixels when the transform is translation-only
  - anti-aliasing is exactly one target pixel wide
  - a line or border thinner than 1 px is drawn 1 px wide with its alpha scaled by its width, so it never flickers or vanishes
  - images shown smaller than their size use mipmaps

### 10.2 Frame pipeline and invalidation

```
style resolve ──► layout ──► paint record ──► compile batches ──► layer passes ──► main UI pass
 (dirty only)    (dirty only)  (per element,    (whole frame,      (offscreen,       (one bgfx view,
                               cached)          cheap)             only if needed)   sequential)
```

Each property change does the least work it can, as browsers do:

| Change | Work |
|---|---|
| Layout property (`width`, `padding`, `flow-children`, text content, font size…) | re-layout that subtree, re-record its paint |
| Paint property (`background`, `border-color`, `box-shadow`, `color`…) | re-record that element's paint only |
| `transform`, `opacity`, colour-matrix `filter` | **patch** the element's primitive data in place: no layout, no re-record |
| Nothing changed | reuse the cached commands. A static screen costs one memcpy-sized upload and a few submits |

Transform and opacity are therefore the cheapest things to animate, so the kit's default transitions use them.

### 10.3 Display list and `UiPainter`

- Every element records its paint into its own cached **command chunk**: box, image, text run, path, external draw, push/pop clip, push/pop layer. The frame's display list is the chunks in paint order.
- **`UiPainter`** is the one drawing API, used both by the CSS box painter and by C++ elements that draw themselves:
  - `Box(rect, BoxPaint)`: background layers, border, radius, shadows
  - `Image(rect, tex, fit)`, `NineSlice(...)`
  - `Text(run, TextPaint)`
  - `Polyline` / `Path` (stroked and filled, tessellated on the CPU with an anti-aliasing fringe; for graphs, minimaps and radial menus)
  - `Arc`, `Ring`
  - `Custom(shader, uniforms, rect)`
  - `PushClip`, `PopClip`, `PushLayer`, `PopLayer`

  Game elements override `Paint(UiPainter&)` instead of calling bgfx.
- **Legacy calls go through the painter too.** `UiRenderer::DrawTexturedRect`, `DrawText`, `DrawTexturedRect9Slice`, `PushMask` and the rest keep their signatures but record commands instead of submitting. A legacy `PixelShader` draw becomes a `Custom` command. Old `Draw()` overrides render unchanged, and get batched for free where they use standard primitives.

### 10.4 GPU data layout

- **Vertex**: position (screen space, after the element's full transform), local position (for SDF evaluation), UV, and a primitive index. That is 28 bytes, 4 vertices and 6 indices per quad, in one transient buffer per frame.
- **Primitive data texture** (RGBA32F, read with `texelFetch` by primitive index, about 8 texels per primitive):
  - rect size and per-corner radii
  - per-side border widths and colours
  - fill kind and gradient reference
  - shadow parameters
  - clip rect and clip radii
  - colour matrix (filters)
  - mask parameters

  This keeps vertices small and lets a transform or opacity animation update just a few texels (WebRender uses the same approach).
- **Bound together for every standard batch:**
  - glyph SDF atlas
  - UI image atlas pages (up to 4, chosen with a small if-chain, since GLSL ES can't index samplers dynamically)
  - gradient ramp texture
  - primitive data texture
  - layer atlas
- **A batch breaks only on:**
  - an external texture not in an atlas (a video frame, a large background image, a render target)
  - a custom shader
  - a non-normal blend mode
  - a stencil clip

  `ui_stats` counts the breaks and names their reasons (§10.12).

### 10.5 Paint order (CSS order)

Per element:

1. outer `box-shadow`s
2. background colour
3. background images and gradients (last layer first, as CSS)
4. inset `box-shadow`s
5. border (or `border-image`)
6. content (text, image, video, custom)
7. children
8. `outline`

A **stacking context** (non-auto `z-index`, `opacity < 1` with `isolation: isolate`, `filter` that needs a layer, `mix-blend-mode`) paints as a unit at its z position, as in CSS.

### 10.6 Boxes: what the base shader draws

- **Rounded box**: signed distance to a rectangle with **four independent corner radii**. `50%` on a rectangle gives a capsule. Coverage is `clamp(0.5 - d / fwidth(d))`, so edges are anti-aliased at any scale or rotation. Quads are expanded by one pixel so the anti-aliased fringe isn't cut.
- **Border**: per-side widths and per-side colours. Corners split along the diagonal, as browsers do. Only the `solid` style at first. `dashed` and `dotted` (via arc-length on the SDF contour) come later.
- **Fills**:
  - solid colour
  - `linear-gradient` (any angle), `radial-gradient` (circle or ellipse, with position), `conic-gradient` (pie charts and cooldown wedges), and the `repeating-*` versions
  - any number of colour stops, baked into one row of the **gradient ramp texture** (256 px wide, cached by stop list)
  - several background layers per box
- **Images in boxes**: `background-image: url()`, `background-size` (`cover`, `contain`, lengths), `background-position`, `background-repeat` (repeat inside an atlas sub-rect via `fract`). An image is clipped by the box's rounded corners for free, since it is the same SDF.
- **`border-image`** (9-slice, including `fill` and `round`/`stretch`) is built as nine quads on the CPU, keeping today's 9-slice behaviour.
- **Shadows**:
  - outer `box-shadow`: a closed-form Gaussian of a rounded rect (Evan Wallace's approximation), one sample per pixel, any blur and spread
  - inset `box-shadow`: the same function for the inverted shape, clipped to the padding box
  - several shadows per box, in order
- **`outline`** with `outline-offset`: a ring outside the border box that doesn't affect layout, used for focus rings.
- **Pixel snapping**: when an element's transform is translation-only, its edges, 1 px borders and text baselines snap to whole pixels, so thin lines stay crisp and don't shimmer while scrolling.

### 10.7 Clipping and masks

- **`overflow: clip` / `scroll`, including rounded corners**: the clip rect and its corner radii go into each child's primitive data and are tested in the shader. No stencil draws and no batch breaks. Nested rectangular clips intersect on the CPU into one.
- A second **rounded** clip inside a rounded clip, and clips under rotation, fall back to the existing stencil path. That breaks the batch, but it is rare.
- **`mask-image: linear-gradient()` / `radial-gradient()`** evaluated analytically per primitive. This gives the modern "fade out at the edges" look on scroll lists at no cost. Image masks come later, through layers.
- **`clip-path`**: `inset()`, `circle()` and `ellipse()` analytic. `polygon()` via stencil, later.

### 10.8 Filters, opacity and blending

- **Colour-matrix filters cost nothing.** `brightness()`, `contrast()`, `saturate()`, `grayscale()`, `sepia()`, `hue-rotate()` and `invert()` are combined into one 4×5 colour matrix per element. It multiplies down the tree and is applied in the shader, with no offscreen pass. Disabled items (`grayscale(1)`), damage tints and hover brightening are free.
- **`opacity`** multiplies down the tree per primitive by default, as today: cheap, and it looks right for almost all UI. True group opacity (overlapping children fading as one image) happens when `isolation: isolate` is set. It also happens **automatically while a page's enter or leave transition runs**, via a layer, so page fades look right and cost nothing once settled.
- **`filter: blur()` and `drop-shadow()`** need the subtree as an image. It renders into the layer atlas (§10.11), gets a separable Gaussian (downsampled for big radii), and is composited. `drop-shadow` follows the real alpha shape (icons, text, odd art), unlike `box-shadow`.
- **`backdrop-filter`** (frosted glass), in two modes:
  - **scene mode (default, cheap)**: the scene colour buffer is downsampled and blurred **once per frame**, only if any visible element asks. Panels sample it behind their rounded shape and then apply their colour matrix. This covers menus, HUD panels and world-terminal glass.
  - **full mode (later)**: blurs the scene plus the UI drawn below the element. It needs the UI pass split at that element, which costs one extra view and copy each.
- **`mix-blend-mode`**: `normal`, `plus-lighter` (additive glows, sparks, energy bars), `multiply` and `screen`. Each is a blend state, so it breaks the batch.

### 10.9 Text

- **SDF glyph atlas**: R8, generated with `stbtt_GetCodepointSDF` (already vendored) at a base size of about 48 px with an 8 px spread. Pages are added on demand. One atlas set serves all sizes of a font. About 4 MB per page, versus 32 MB per font and size today.
- **From the distance field, in the same draw**:
  - fill
  - `-webkit-text-stroke` / `text-stroke` (threshold shift)
  - **several `text-shadow`s with blur** (offset plus a softer edge)
  - glow
  - synthetic bold (threshold shift) and synthetic italic (skew) when a font has no bold or italic face
- **Text styling**:
  - `@font-face` in CSS maps families and weights to `.ttf` files (one face per weight). The global fallback chain (`AddFallbackFont`) stays.
  - `letter-spacing`, `line-height`, `text-align` (left, centre, right), `white-space` (`normal`, `nowrap`, `pre`), `text-overflow` (`ellipsis`, `shrink`), `text-transform`, and `text-decoration` (underline, line-through)
  - **`font-variant-numeric: tabular-nums`**: equal-width digits, so ammo counts and timers don't jitter
- **Quality**: glyph quads snap to the pixel baseline when the transform is translation-only. Below about 12 px, single-channel SDF goes slightly soft, so a per-font `font-render: bitmap` opt-in keeps a hinted bitmap path for tiny text. MSDF (msdfgen) is a later upgrade if very large titles show rounded corners.
- **Layout cache**: a text run is shaped and wrapped once and reused until its text, font, size, wrap width or variables change. Localisation resolves once per change.
- **Rich text**: `<b>`, `<i>`, `<color=…>`, `<size=…>`, and inline icons `<icon=action:interact>` / `<icon=img:coin>`, which sit on the baseline and scale with the font.

### 10.10 Images and icons

- **Runtime UI atlas**: textures up to 512 px are packed at load time (skyline packer, 4 px padding, mipmapped pages), so different icons don't break batches. Larger images stay standalone textures.
- **SVG icons** through nanosvg (single-header, MIT): rasterised into the atlas at the size they're displayed at, in power-of-two size buckets. Icons are crisp at any UI scale, and one file serves every size.
- `object-fit` and `object-position` on `<Image>`. `image-rendering: pixelated` for pixel art (point sampling). Video frames from `UiVideo` are external textures.
- Images load asynchronously through `AssetRegistry`. Until ready, the element has a `:loading` state (CSS can show a shimmer). A screen can preload everything it uses on `Push`, so nothing pops in.

### 10.11 Layer atlas (offscreen)

- One shared atlas render target (RGBA8, 4096², more pages if needed), reusing the free-rect allocator `UiBilboardAtlas` already has.
- Used for:
  - blur and drop-shadow filters
  - group opacity (isolation, screen transitions)
  - `cache: layer` subtrees (expensive custom shaders, complex static panels)
  - **world surfaces** (§9)
- Layer passes run in their own bgfx views **before** the main UI view, in dependency order. A layer re-renders only when its subtree's recorded commands change (hashed), and is otherwise a single textured quad.
- Budget: layer memory is tracked in `ResourceStatistics`. Layers unused for a few seconds are evicted, and an over-budget request falls back to the non-layer path with a warning.

### 10.12 Debug and performance tools

- `ui_stats` overlay: elements, primitives, batches with break reasons, text runs re-shaped this frame, layers in use, atlas fill, and CPU time per pipeline stage.
- `ui_debug` modes: element outlines, batch colouring, clip rects, layer bounds, and invalidation flashes (what re-recorded this frame).
- Tracy zones for every pipeline stage, using the existing profiling build.

### 10.13 CSS property support

| Area | At launch | Later | Not planned |
|---|---|---|---|
| Background | `background-color`, `background-image` (url, linear/radial/conic and repeating gradients, several layers), `background-size`, `background-position`, `background-repeat`, `background-clip: border-box / padding-box` | `background-clip: text` | `background-attachment` |
| Border | `border`, per-side `border-*-width` / `border-*-color`, `border-radius` per corner, `border-image` | `dashed`, `dotted`, elliptical radii (`a / b`) | `double`, `groove`, `ridge`, `inset`, `outset` |
| Shadow and outline | `box-shadow` (several, inset, spread, blur), `outline`, `outline-offset`, `text-shadow` (several, blur) | | |
| Effects | `opacity`, `filter` (colour-matrix functions, `blur`, `drop-shadow`), `backdrop-filter` (scene mode), `mix-blend-mode` (4 modes), `mask-image` (gradients), `clip-path` (inset, circle, ellipse), `isolation` | backdrop full mode, image masks, `clip-path: polygon()` | `filter: url()` |
| Transform | `transform` 2D (translate, scale, rotate, skew, matrix), `transform-origin` | 3D `perspective` / `rotateX` / `rotateY` (card flips) | |
| Text | `color`, `font-family`/`-size`/`-weight`/`-style`, `@font-face`, `letter-spacing`, `line-height`, `text-align`, `text-overflow`, `white-space`, `text-transform`, `text-decoration`, `text-stroke`, `font-variant-numeric` | `text-align: justify`, `word-break` options | vertical writing, bidi/RTL shaping (no target language needs it yet) |
| Image | `object-fit`, `object-position`, `image-rendering` | | |
| Motion | `transition`, `animation`, `@keyframes`, `cubic-bezier()`, `steps()` | | |
| Values | `px`, `%`, `em`, `rem`, `vw`, `vh`, `calc()`, `min()`, `max()`, `clamp()`, `var()`, hex/`rgb()`/`rgba()`/`hsl()` colours | | `env()`, `attr()` |
| Pseudo-elements | | `::before`, `::after` (decorative only) | `::first-line`, `::selection` |

---

## 11. Other pieces a modern game UI needs

Things the earlier revisions didn't cover. Each one is small on its own, but together they are much of what makes a game's UI feel finished.

### 11.1 Input prompts and device switching

- `<InputPrompt action="interact"/>` and `<icon=action:interact>` in text show the right glyph for the **last-used device** (keyboard and mouse, Xbox, PlayStation, Switch, touch), using `InputIconLibrary`. They update the moment the player switches devices.
- The root gets `.input-gamepad` / `.input-mouse` / `.input-touch`, so CSS can show or hide device-specific hints.
- **Hotkeys on buttons**: `<TextButton hotkey="ui_reset" …>` binds an input action to a button. The `ActionBar` kit component (the console-style footer: "Ⓐ Select  Ⓑ Back  Ⓨ Reset") lists the visible screen's hotkeys automatically. LB/RB tab switching is a `TabBar` with `hotkey-prev` / `hotkey-next`.

### 11.2 Navigation details

- Explicit overrides when spatial navigation guesses wrong: `nav-up="#apply"` (and down, left, right), `nav-wrap="true"` on a container, and `focus-group` (the group remembers its last focus and returns to it).
- Scrolling containers scroll the focused child into view smoothly. Mouse hover and gamepad focus stay one concept, as today.

### 11.3 Two-way binding for settings controls

Variables are one-way (game to UI). Settings controls need two-way: `<Slider bind="setting:audio.master"/>`, `<ToggleButton bind="setting:video.vsync"/>`, `<DropDown bind="setting:video.window_mode" options="…"/>`. They bind to a small settings registry over `GameSettings.cfg`, with **apply / revert / restore defaults** supported by the stack (`OnBack` asks "discard changes?"). The settings kit is built on this, so a new setting is one line of markup plus one registry entry.

### 11.4 Lists, grids, drag and drop, tooltips

- **`<ListView snippet="SaveRow" count="{save_count}">`**: a virtualised list that only builds visible rows and recycles them. C++ fills a row in an `OnBindRow(index, row)` callback. For save slots, server lists and big inventories.
- **Grids**: `flow-children: right-wrap` with fixed cells, plus `ListView layout="grid"` for big ones.
- **Drag and drop**: `draggable="true"`, `ondragstart` / `ondragenter` / `ondragleave` / `ondrop`, and a drag ghost drawn on the overlay layer. On gamepad it becomes **pick up and place** (confirm picks up, confirm again drops). For inventories, loadouts and crafting.
- **Tooltips**: `tooltip="${KEY}"` or `tooltip-layout="ui/tooltips/item.xml"` (with the element's variables). Shown on the overlay layer after a delay, kept on screen, and on gamepad shown for the focused element.
- **Context menus**: `oncontextmenu` (right-click, or long press / a hotkey on gamepad) pushes a small `ShowsBelow` screen positioned at the element.

### 11.5 Accessibility and comfort

- **UI scale** (layout scale) and **text scale** (`rem` units follow the player's text-size setting) are separate settings.
- **Reduced motion**: `@media (prefers-reduced-motion)` plus an engine setting that shortens transitions, disables looping keyframes and caps any UI shake. This fits the camera-comfort rules already in place (low frequency, bounded).
- Colour-blind-friendly themes are just alternative variable sets (`theme-deuteranopia.css` overriding `--danger`, `--ok`…), switched with a root class.
- Hold-to-confirm as an alternative to rapid presses, as a kit button variant.

### 11.6 Localisation robustness

- **Pseudo-localisation mode** (`-ui_pseudoloc`): every string about 35% longer with accented characters, to find overflow and hard-coded text before translators do. `-ui_lint` reports text that overflows its box in this mode.
- `:lang(ja)` style rules for per-language fonts and sizes. Variables get plural and number formatting (`{count:plural:item}`, `{gold:n0}`).

### 11.7 Lifetime safety

Gameplay holds UI and UI calls gameplay, which is the classic source of crashes.

- `On()` handlers registered by a component or entity are removed automatically when it is destroyed.
- `UiElemRef<T>` is a weak handle (like `EntityHandle`) for gameplay code that keeps a pointer to UI. It is null-safe after rebuilds and hot reloads.
- Hot reload rebuilds the tree but keeps variables, classes set from code, focus (by id path), and scroll positions.

### 11.8 Automated UI testing

Console commands and a headless script runner (`-ui_test tests/pause.uitest`), for regression tests and for AI to verify its own work:

```
push ui/pause.xml
expect #resume :focus
nav down
press ui_confirm
expect-top ui/settings.xml
back
expect-top ui/pause.xml
screenshot pause_after_back.png
```

The same runner does **golden screenshot comparison** for renderer work. Phase 0 must reproduce every game's existing UI pixel-close.

### 11.9 Loading screens

How they work today, which stays: level loading runs on the main thread and calls `LoadingScreenSystem::Update(progress)` between its steps. Each call runs one complete frame by itself: update the loading screen's own `UiViewport`, draw it, then `bgfx::frame()`. The normal main loop and the normal UI tree do not run during a load.

What changes:

- A loading screen can be any markup layout (`LoadingScreenSystem::SetLoadingScreen("ui/loading.xml")`) as well as a C++ `UiLoadingScreenBase`. It lives in its **own `UiContext`**, owned by `LoadingScreenSystem`, separate from the main stack and HUD.
- Each `Update(progress)` call sets `{progress}` on that context, runs style, layout and animation with a delta time measured from the real clock between calls (game time doesn't advance during a load), then draws and presents as today.
- `LoadingScreenSystem::Draw` must call `UiRenderer::EndFrame()` before `bgfx::frame()`, because the new renderer's per-frame buffers rely on it.
- Animations on a loading screen only advance when the loader calls `Update`, so a spinner moves in steps during a long blocking step. That is a property of the blocking loader, not of the UI.

---

## 12. UI Studio (separate editor program)

### 12.1 Why Qt, and how it embeds the engine

- `source/QtEditor` already embeds the engine as a static library with an SDL/bgfx child window inside a Qt widget (`EngineInstance`, `EngineViewportWidget`), and links one game's sources. Qt 6.11 MSVC is installed.
- UI Studio is a **second Qt executable** that reuses those two classes. They move into a small shared `EditorHost` library rather than being copied. The canvas is therefore **the real UI renderer**: what you see is exactly what the game draws.
- It links one game's sources, as QtEditor does (`GAME_DIR`), so game-registered tags render for real. Tags it doesn't know show as labelled placeholder boxes taken from the schema.
- Qt gives what the ImGui editor lacks: docking, real text editors (`QSyntaxHighlighter`, completion), `QUndoStack`, native dialogs and drag and drop.

### 12.2 Windows

| Panel | What it does |
|---|---|
| **Project** | Layouts, stylesheets, images, fonts and sounds under `GameData/ui`. Drag an image onto the canvas to create an `<Image src>`. Double-click to open |
| **Canvas** | Engine viewport. Click to select, drag to move, handles to resize, snapping and guides, box-model overlay (margin, padding, content, as in browser devtools), zoom and pan, resolution presets (16:9, 21:9, 4:3, 4K, phone), safe-area overlay |
| **Hierarchy** | Element tree: drag to reorder or reparent, wrap in Panel, copy and paste. Component instances show as one node with their slots. "Open definition" jumps into the component file |
| **Components** | Library of every component tag (engine kit, game, imported) with thumbnails and parameters. Drag one onto the canvas to place it. **"Extract to component"** turns a selected subtree into a new file and offers its literal texts and values as parameters |
| **Inspector** | Attributes from the tag's schema, class chips, and the **computed style** with the winning rule and its `file:line`. Editing a value edits that rule in its `.css` file, or the inline `style` |
| **Code** | XML and CSS text views with highlighting, completion (tags, attributes, properties, class names, variables, handler names) and live error squiggles. Text edits re-parse into the canvas, canvas edits update the text. **The files are the source of truth** |
| **States & data** | Force `:hover`, `:active`, `:focus` or `:disabled` on the selection. Set test variables (`hp = 0.2`) and classes (`low-health`). Replay a page's enter and leave animations |
| **Stack preview** | Push and pop pages, press back, and drive gamepad navigation with the focus graph drawn as arrows |
| **Host preview** | Show the open screen as a world surface at its pixel size, on a plane or on a chosen mesh's material slot, with a simulated aim cursor and focus mode, to test terminals without loading a level |

### 12.3 Saving

- XML is written with pugixml in one canonical format (2-space indent, attributes kept in order, comments kept). AI or hand edits in the same style round-trip unchanged.
- CSS edits are applied at the declaration's source span, so the rest of the file stays byte-identical.
- The running game already hot-reloads on file mtime, so saving in UI Studio updates the game live. Later, a small local socket adds "pick in game, select in Studio".

---

## 13. Making it easy for AI

1. **One way per job.** Any UI is a layout plus optional code-behind (a `UiElem` subclass). Where it shows up is the host's job: `UiMenus::Push` for menus, `UiLayers::Hud()` for HUD, `UiWorldSurface` for world screens. Per-frame game data goes in `Tick()`. Look = CSS. Logic = `On()` handlers. Reusable pieces = components. Runtime lists = snippets. XML and CSS are formats AI already knows well.
2. **Schema.** `-ui_schema` writes `ui_schema.json`: every tag (built-in, game and component), its attributes or parameters and types, slots, events, CSS properties and values, and built-in actions, all generated from the registry. UI Studio's completion uses the same file.
3. **Lint.** `-ui_lint` loads every layout and stylesheet headless and reports unknown tags, attributes or properties, unresolved handlers, missing images, fonts or sounds, missing localisation keys, and unused classes. It exits non-zero on errors.
4. **Check without vision.** `ui_dump` prints the live tree with tag, id, classes, states, computed rect and visibility. `-ui_preview ui/pause.xml -screenshot out.png` renders one screen headless.
5. **Guide.** `Engine/UI/UI_GUIDE.md` has copy-paste recipes: pause menu, settings page, confirm dialog, list from snippets, health bar, enemy marker, toast, custom tag. It replaces the outdated reference doc.

---

## 14. Shared kit

`Engine/UI/Kit/` holds the pause menu, settings (video, sound, input/rebind, game), confirm dialog, rebind capture and save/load, all as XML plus code-behind, built from shared components in `Engine/UI/Kit/components/` (`SettingRow`, `Card`, `StyledButton`, `DialogFrame`, `TabBar`, `KeyBindSlot`, `Toast`). Games can use the components for their own screens. Each game keeps only its `theme.css` and any game-specific settings pages it registers. This removes the 4 copies of about 3,000 lines.

---

## 15. Compatibility and migration

- `UiElem` derives from `UiElement`, so there is one tree, one navigation system and one renderer. All existing C++ UI keeps working unchanged and can be ported screen by screen.
- Legacy elements can be used inside markup by their registered name. Markup can be added anywhere in an old tree (`Viewport.AddChild(UiElem::Load("ui/x.xml"))`).
- The JSON `.ui` loader stays until the two files per game (`settingsMenu.ui`, `StyledButton.ui`) are ported, then it is deleted along with the ImGui `UiEditor`. In-game picking survives as UI Studio's "pick in game" link.
- RmlUi (`RmlUiContext`, `UiModel`, `.rml`/`.rcss`, its shaders and backend) is deleted once the kit replaces its pause and settings documents.
- The only visible change to old UI is sharper text from the SDF path, behind a per-font switch until it's approved by eye.

---

## 16. Phases

| # | Phase | Done when |
|---|---|---|
| 0 | **Renderer core** (§10.2–10.4, 10.9, 10.10): pipeline with invalidation, `UiPainter`, cached display lists, base shader, primitive data texture, atlases, shader clips, SDF text with layout cache, legacy `UiRenderer::Draw*` recording into it, layout dirty flags, hit testing after layout, `ui_stats`, golden screenshot runner (§11.8) | Old UI in every game matches its golden screenshots. Tracy shows the draw-call and CPU drop. Runs on the Emscripten build |
| 1 | **Markup core**: pugixml, `UiElem`, Panel/Label/Image/Button/TextButton, CSS parser (selectors, layout, box, text, values incl. `calc`) with separate and inline styles, Panorama flow layout, components (params, slots, scoped ids and styles, imports, discovery), states, `file:line` errors, `ui_dump`, `-ui_preview`, `-ui_test` | `pause.xml` renders and navigates, checked by a `.uitest` script |
| 2 | **Menus**: `UiMenuStack`, `UiMenus`, host options, lifecycle virtuals, code-behind registration, events and built-in actions, `On()`, variables, snippets, pause hook, input prompts and hotkeys, `ActionBar`, navigation overrides, lifetime-safe handlers | One game's pause and confirm dialog run from markup, with device-correct prompts |
| 3 | **Full CSS painting and motion** (§10.5–10.8): per-side borders, all gradients, several shadows (outer and inset), outline, colour-matrix filters, gradient masks, `clip-path` shapes, blend modes, 2D transforms, transitions, keyframes, sounds, reduced motion | Buttons hover, press and focus with animation. Pages slide in and out. A kit button needs no texture |
| 4 | **Widgets and kit**: wrapped Slider, DropDown, TextEntry, Movie, ShaderPanel. Toggle/Radio, Progress, overflow scroll, Frame, `ListView`, tooltips. Two-way settings binding. Settings kit in the engine. UI and text scale. Pseudo-localisation | 4 games share one settings menu. JSON `.ui` and RmlUi deleted |
| 5 | **HUD**: layers, `Tick()`, `WorldPanel`, value transitions, markup loading screens | One game's HUD and enemy bars in markup |
| 5b | **World UI**: `UiContext` split, `UiWorldSurface` (plane and mesh slot), aim and focus interaction, `InjectPointer`, dirty re-render, distance update rate, `UiBilboard` on top of it | A door terminal opens a door by aim+use and by focus mode with a gamepad. 20 idle screens in a level cost close to nothing |
| 6 | **UI Studio v1**: shared `EditorHost`, canvas, hierarchy, components library, inspector, synced code views, undo, states & data, resolution presets, `-ui_schema`, `-ui_lint` | A new menu can be built entirely in Studio. ImGui `UiEditor` deleted |
| 7 | **Layers and Studio v2** (§10.11): layer atlas, blur and drop-shadow filters, group opacity, scene-mode backdrop blur, `cache: layer`, SVG icons, drag and drop, context menus; pick-in-game link, stack and host preview, animation replay | Frosted-glass pause menu, free when static. An inventory grid with drag and drop on mouse and gamepad |

Phase 0 can run alongside 1 and 2 because it changes no APIs. UI Studio can start as soon as phase 1 has a DOM to edit. It doesn't need to wait for 5.

---

## 17. Open questions for the owner

1. **Layout vocabulary**: Panorama's `flow-children`, `fit-children`, `fill-parent-flow` and `*-align` (as proposed; it maps directly onto today's anchors and boxes), or CSS flexbox names (`display: flex`, `justify-content`, …), which AI knows a bit better?
2. **Studio and games**: one Studio build per game (links that game's tags for real, like QtEditor), or one Studio for all games with game tags shown as schema placeholders?
3. **Pause ownership**: the stack pauses the game through one hook (as proposed), or each game keeps full control and the stack only reports that a pausing screen is open?
4. **Text**: SDF text as the default for old UI too, or only for the new `Label`?
5. **In-game editing**: delete the ImGui `UiEditor` entirely once Studio lands, or keep a tiny in-game inspector (pick plus `ui_dump`) for debugging on devices without Studio?
6. **World screens**: is aim+use (poke while playing) plus focus mode (camera locks onto the terminal) the right pair, and which should be the default for a new surface?
7. **Inventory-style UI**: does any current game need drag and drop and grids soon? That decides whether they move up from phase 7.
8. **Small text**: is the bitmap fallback for text under about 12 px worth keeping, or is SDF everywhere fine for these games' sizes?
