# Phase 2: menus, events and input

**Goal:**

- a modal menu stack that hosts **any** element (decision D12)
- markup events with handlers and built-in actions
- pointer events with capture
- device-aware input prompts and rich text
- button hotkeys with an action bar
- navigation overrides
- the StealthAction pause menu moved to markup as the pilot

**Prerequisites:** phase 1 done. Reread conventions §2 (threading) and §11 (backends).

## Files

```
source/Engine/UI/Menus/UiMenuStack.h/.cpp      stack element + UiMenus (main stack)
source/Engine/UI/Menus/UiActions.h/.cpp        action-list parser, built-ins, dispatch
source/Engine/UI/Menus/UiInputPrompts.h/.cpp   device tracking, prompt provider
source/Engine/UI/Elements/UiElemInputPrompt.h/.cpp
source/Engine/UI/Elements/UiElemActionBar.h/.cpp
edited: UiElem (events, lifecycle, pointer), UiTextLayout (rich text), UiSystem, UiTestRunner, Input.h/.cpp (simulated mouse), UiNavigation (no behaviour change; read only)
StealthAction: GameData/ui/pause.xml, GameData/ui/confirm_quit.xml, UI/Pause/StealthPauseMenu.h, GameStart.cpp, UI/Pause/UiSettingsMenu.h
```

---

## Task 2.1: lifecycle virtuals and host options on `UiElem`

Add to `UiElem` (these are the "no special root types" hooks):

```cpp
virtual void OnPushed() {}  virtual void OnShown() {}  virtual void OnHidden() {}  virtual void OnPopped() {}
virtual bool OnBack() { return false; }
UiMenuStack* Stack() const;            // walks up to the nearest page; nullptr if not on a stack
struct HostOptions { bool CloseOnBack = true, ShowsBelow = false, PausesGame = false, HidesHud = false; };
HostOptions ReadHostOptions() const;   // from Attributes: close-on-back, shows-below, pauses-game, hides-hud ("true"/"false")
```

- Unknown-attribute warnings must **not** fire for these four attributes, or for `default-focus`.
- `UiMenuStack` reads the options. Other hosts ignore them.

## Task 2.2: `UiMenuStack` and `UiMenus`

```cpp
class UiMenuStack : public UiElemPanel
{
public:
    UiElem* Push(const std::string& layoutPath);           // UiElem::Load(path) (code-behind aware); nullptr on error
    UiElem* Push(std::shared_ptr<UiElem> page);
    void Pop();  void PopTo(UiElem* page);  void Replace(const std::string& path);  void Clear();
    void Back();                                           // what ui_cancel does
    UiElem* Top() const;  bool IsEmpty() const;  int Depth() const;
    std::function<void()> OnEmptyBack;                     // host: what back does when the stack is empty
private:
    struct Entry { std::shared_ptr<UiElem> Page; std::weak_ptr<UiElement> SavedFocus; HostOptions Options; };
    std::vector<Entry> m_entries;
};
namespace UiMenus
{
    UiMenuStack& Stack();                 // created in UiSystem::Init, added to EngineMain::Viewport (useLateDraw = false)
    UiElem* Push(const std::string& path);  void Pop();  void Clear();  bool IsOpen();
    inline std::string PauseMenu;         // when non-empty: the "pause" action pushes it if the stack is empty
    inline std::function<void(bool paused)> OnPauseChanged;
}
```

### The stack element itself

- `width: 100%; height: 100%`.
- `HitCheck = false`.
- It is registered as a root with `UiSystem` (`TagName "MenuStack"`).

### Push

1. Save `UiNavigation::Focused`. Store a `weak_ptr` (`shared_from_this` when it is a `UiElement` owned by `shared_ptr`).
2. Read the host options.
3. `AddChild(page)`, add the class `page`, and set `kStateEntering` (cleared on the next `UiSystem::Update` after style resolve, so phase 3 transitions start from it).
4. The previous top gets `FocusTrap = false`.
   - If the new page has `ShowsBelow == false`, every lower page gets the `Style()` override `Visibility = Collapse`. Otherwise lower pages stay visible.
   - The new top gets `FocusTrap = true`.
5. Focus: `default-focus="true"` element inside the page (first in document order), else `UiNavigation::FocusFirst(page)`.
6. Call `OnPushed()` on the page, and `OnHidden()` on the previous top.
7. Recompute pause (below).

### Pop

1. The top page gets `kStateLeaving`. In phase 2 it is removed immediately; phase 3 waits for transitions.
2. Remove it from its parent. Call `OnPopped()`.
3. The new top: clear the collapse override, set `FocusTrap = true`, call `OnShown()`.
4. Restore the saved focus if it's still alive and visible. Otherwise apply the same rule as push step 5.
5. Recompute pause.

### Other operations

- **Back:** if the stack is empty, call `OnEmptyBack`. Otherwise, if the top's `OnBack()` returns true, stop. Otherwise pop if `CloseOnBack`.
- **Cancel wiring:** legacy `UiNavigation` calls `trap->OnNavCancel()` on the active `FocusTrap`. Override `UiElem::OnNavCancel()` so that if this element is a page, it calls `Stack()->Back()`.
- **Pause:** `paused = any entry has PausesGame`. When the value changes, call `UiMenus::OnPauseChanged(paused)`.
- **Hides HUD:** set or clear the class `hud-hidden` on the HUD layer when it exists (phase 5). In phase 2, keep the bool only.
- **Pause action:** in `UiSystem::Update`, when `UiMenus::PauseMenu` isn't empty, the stack is empty, and `Input::GetAction("pause")->Pressed()`, call `Push(PauseMenu)`. Then `CleanPressed()` on that action so the game doesn't also see it.

Before coding the focus parts, read `UiNavigation::GetActiveTrap` and `Collect` and confirm that navigation candidates are limited to the active trap's subtree. If they aren't, **STOP** and report.

## Task 2.3: events and actions

- Event attributes on any element: `onactivate`, `onmouseover`, `onmouseout`, `onfocus`, `onblur`, `oncancel`, `onload`. Phase 4 adds `onvaluechanged`, plus every `on<name>` used for component events through `Raise`.
- **Parsing** (once, when the attribute is applied):
  - the value is a list of calls separated by `;`
  - a call is `Name(arg, arg)`
  - an argument is `'…'`, `"…"`, a number, or a bare identifier (as a string)
  - a syntax error is a markup error at the element's line
  - store `std::vector<UiActionCall{Name, Args}>`
- **Handlers:** `void On(const std::string& name, std::function<void()>)`, `void On(const std::string& name, std::function<void(UiElem& sender, const std::vector<std::string>& args)>)`. Store them per element.
- **Dispatch** of `Fire(event)` on element E. For each call:
  1. walk E and its `UiElem` ancestors; the first with a handler of that name runs it
  2. else a built-in runs (table below)
  3. else warning `unhandled action 'X'` (once per element and name)
- **When events fire:**

  | Event | Fires when |
  |---|---|
  | `activate` | `Activate()` |
  | `focus` / `blur` | `UiNavigation` focus changes (compare `Focused` each `UiSystem::Update`) |
  | `mouseover` / `mouseout` | hover state changes |
  | `cancel` | before the stack's back on that page |
  | `load` | once, after the element's first layout |

- **Validation:** when an element is pushed or registered as a root, walk its subtree. Every call name must resolve to a handler on some ancestor-or-self (handlers registered in constructors exist by then) or to a built-in. Unresolved ones get a warning with the element's line.

Built-ins (selector `sel` is `#id` or an id path, resolved with `Find` from the calling element's instance):

| Name | Args | Effect |
|---|---|---|
| `Back` | – | `Stack()->Back()` on the element's stack (main stack if none) |
| `Push` / `Replace` | path | on the element's stack (main stack if none) |
| `Pop` / `Clear` | – | same stack |
| `PushMenu` | path | always the main stack |
| `Quit` | – | push `SDL_QUIT` |
| `PlaySound` | path | `auto s = SoundManager::GetSoundFromPath(path); s->Type = SoundType::UI; s->Is2D = true; s->Play();` |
| `AddClass` / `RemoveClass` / `ToggleClass` | sel, class | on the found element |
| `SetFocus` | sel | `UiNavigation::SetFocus` |
| `Raise` | event | finds the nearest ancestor-or-self with `ComponentTag` non-empty (the instance root R), then fires R's `on<event>` attribute calls with dispatch starting at **R's parent** |

**Lifetime:** `On(name, std::weak_ptr<void> guard, fn)` skips `fn` when `guard` has expired. Use it from gameplay code holding entities or components.

## Task 2.4: pointer events with capture (README constraint 4)

- Add `virtual void OnPointerDown(glm::vec2 p)`, `OnPointerMove(glm::vec2 p)`, `OnPointerUp(glm::vec2 p)` and `OnScroll(float delta)` to `UiElem`. `p` is in element-local units.
- In `UiSystem::Update`, before roots update: for each `Input::TouchActions` entry:
  - `pressed` → hit test `EngineMain::Viewport.GetHitElementUnderPosition(pos)`. If the result is a `UiElem`, capture it for that touch id and call `OnPointerDown`.
  - held → `OnPointerMove` to the captured element (even outside its rect).
  - `released` → `OnPointerUp`, then release the capture.
- Wheel: `Input::MouseScrollDelta != 0` → `OnScroll` on the hovered `UiElem`.
- Local coordinates: `TransformPoint(inverse(AbsoluteTransform), p)`.
- Keep phase 1's `TouchEvents`-based `:hover` / `:active` / activate logic unchanged.

## Task 2.5: simulated mouse for tests

- `Input.h`: `static inline std::optional<TouchEvent> SimulatedMouse;`
- `Input.cpp`, at the line `TouchActions[1] = mouseTouchEvent;`: use `*SimulatedMouse` when it is set, and also set `MousePos` to its position.
- Runner commands:

  | Command | Effect |
  |---|---|
  | `mouse_move x y` | sets `SimulatedMouse` with `pressed = released = false` |
  | `mouse_down` | `pressed = true` for one frame, then held |
  | `mouse_up` | `released = true` for one frame |
  | `click x y` | move + down + up, with waits of 2 frames each |
  | `mouse_off` | clears `SimulatedMouse` |

  Then `wait_frames 2` automatically.
- Read `Input::UpdateMouse` before changing anything. If the mouse touch event isn't built at that line, find where it is built and apply the same override there. Report the location in the notes.

## Task 2.6: input prompts and rich text

- **`UiInputPrompts`:**
  - `enum class UiInputDevice { KeyboardMouse, Gamepad, Touch }`
  - `LastDevice()`, updated from `Input::ReceiveSdlEvent`:
    - keys, mouse buttons and mouse motion of more than 2 px → KeyboardMouse
    - controller buttons, or an axis magnitude above 0.3 → Gamepad
    - finger events → Touch
  - `SetProvider(std::function<std::string(const std::string& action, UiInputDevice)>)` returns an image path. The game supplies it (the pilot wraps StealthAction's `InputIconLibrary`).
  - When the device changes, every root gets exactly one of the classes `input-mouse`, `input-gamepad`, `input-touch`.
- **`<InputPrompt action="x"/>`** (`UiElemInputPrompt : UiElemImage`): `src` = provider(action, LastDevice()), refreshed when the device changes. Empty path → `Visibility` override `Collapse`.
- **Rich text in `Label`** (applies only when the element has `html="true"`, so normal text with `<` stays literal):
  - tags `<b>…</b>`, `<i>…</i>`, `<color=#rrggbb>…</color>`, `<size=NN>…</size>`
  - `<icon=action:NAME>` (prompt image) and `<icon=img:PATH>`
  - an icon is a square of height = the current font size × 1.1, sitting on the baseline, with advance = height + 0.1 em. It is drawn with `UiPainter::Image` from the label's `PaintContent`.
  - `UiTextLayout` gets segment runs. Each segment has its own colour, weight, italic and size. An icon is a special segment of fixed width. Line breaking treats an icon as an unbreakable word.
  - Unknown tags are drawn literally, with a warning.
- **Hotkeys:**
  - attribute `hotkey="action"` on Button/TextButton: when `Input::GetAction(action)->Pressed()`, the button is visible, not disabled, and its page is the top of the main stack (or it has no stack and its root is visible) → `Activate()`, then `CleanPressed()`
  - `hotkey-label="${KEY}"`, default: the button's text
- **`<ActionBar/>`** (`UiElemActionBar : UiElemPanel`, `flow-children: right`, `gap: 24px`):
  - rebuilt when the top page or the device changes
  - one entry per visible hotkey button on the top page (InputPrompt + Label)
  - plus a fixed back entry `ui_cancel` → `${UI_BACK}` when the top page has `close-on-back`
  - add the `UI_BACK` key to every in-scope game's English localisation file (find the folder through `Localisation::Initialize("GameData/localisation", "en")`)
- **Navigation overrides:** `nav-up`, `nav-down`, `nav-left`, `nav-right` = `#id`. After layout, resolve them with `Find` and set the legacy `NavUp` / `NavDown` / `NavLeft` / `NavRight` weak pointers. Check first that `UiNavigation::Navigate` honours them; if it doesn't, **STOP**.

## Task 2.7: pilot, the StealthAction pause menu

1. Read `source/Games/StealthAction/GameStart.cpp` (the pause code around `wasPaused` / `pauseMenu`) and `UI/Pause/UiPauseMenu.h` / `UiSettingsMenu.h`.
2. Write `source/Games/StealthAction/GameData/ui/pause.xml`:
   - a `<Panel class="pause" pauses-game="true" hides-hud="true">` with a scrim and a centred column of `TextButton`s
   - Resume → `Back()`
   - Settings → `OpenSettings()`
   - Restart → `Restart()`
   - Quit → `Push('ui/confirm_quit.xml')`
   - the same localisation keys as the C++ menu
   - styles in `GameData/ui/styles/theme.css`, looking close to the current menu
3. Write `confirm_quit.xml`: `shows-below="true"`, Yes → `Quit()`, No → `Back()`.
4. `UI/Pause/StealthPauseMenu.h`: a `UiElemPanel` code-behind with `LoadLayout("ui/pause.xml")` and handlers:
   - `Restart`: the same code as the C++ menu's restart lambda
   - `OpenSettings`: creates the legacy `UiSettingsMenu`
   - register it with `UI_REGISTER_CODE_BEHIND("ui/pause.xml", StealthPauseMenu)`, and add the file to `StealthAction.vcxproj` and `.filters`
   - change **StealthAction's own** `UiSettingsMenu` constructor to take `std::function<void()> onClosed` instead of the previous-menu pointer, and call it where it used to re-show the previous menu
   - `OpenSettings` sets the page's `visible="false"` attribute, and `onClosed` sets it back to `true`
5. In `GameStart.cpp`:
   - remove the manual `UiPauseMenu` creation and removal
   - set `UiMenus::PauseMenu = "ui/pause.xml";`
   - set `UiMenus::OnPauseChanged = [](bool p){ PauseGameManager::SetGamePaused(p); };`
   - set `UiMenus::Stack().OnEmptyBack = nullptr;`
   - keep `Input::LockCursor` and `EngineMain::MainInstance->Paused` updated from `PauseGameManager` exactly as before
6. Keep StealthAction's C++ `UiPauseMenu.h` in the project but unused. Phase 4 deletes it.

## Tests (StealthAction, all backends; layouts in `ui/kit/tests/`)

1. **`p2_stack`:** push `p2_page_a.xml` (`close-on-back`), push `p2_page_b.xml` (`shows-below="true"`, `default-focus` on `#bb2`), then:
   - check `#bb2` has focus and page A is still visible
   - `action ui_cancel` → B is gone and the focus is back on A's previously focused button
   - push `p2_page_c.xml` (`shows-below` false) → `expect pa visible == 0`
2. **`p2_events`:**
   - a button with `onactivate="AddClass('#target','on'); Push('GameData/ui/kit/tests/p2_page_a.xml')"` → after `ui_confirm`, the target has the class and page A is on top
   - a `TestRow`-style component with `Raise('picked')`, and the page handling `onpicked="AddClass('#target','picked')"`
   - an unknown action gives exactly one warning
3. **`p2_mouse`:** `click` on a button → `activations == 1`. A capture element (a test-only `Panel` with `hittest="true"` and code-behind counting moves): `mouse_move` inside, `mouse_down`, `mouse_move` outside, `mouse_up` → its move count ≥ 1 and up count == 1.
4. **`p2_prompts`:** runner command `sim_device gamepad|mouse` (sets `LastDevice` through a test hook) → the root's class switches. `InputPrompt` `src` changes (`expect id src contains`; add a `src` prop to `expect`). A rich-text label with `<icon=img:GameData/textures/generic/white.png>` → its line width includes the icon.
5. **`stealth_pause`:**
   - `action pause` → the top is the pause page, and `EngineMain::MainInstance->Paused` (new `expect_engine paused == 1`)
   - `ui_down` / `ui_confirm` on Quit → the confirm dialog is on top with the pause page visible below
   - `ui_cancel` → back on the pause page
   - `ui_cancel` → the stack is empty and paused == 0
   - `ui_screenshot` at each step; the look is approved by the owner (**STOP**: send the screenshots)

**Phase done when:**

- all tests pass on every backend, and `cross` is `EQUAL` for every capture
- the phase 0 and phase 1 regressions pass
- the owner approved the pilot look

## Implementation notes

(implementer writes here)
