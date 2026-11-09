"""Builds Docs/UI/UI_Reference.html: the user documentation of the UI system.

The prose and the examples are written in this file. The reference tables (tags and attributes, CSS properties, paint effects,
animatable properties, actions, kit components, theme tokens, settings) are read from the game's uidb.json, so they cannot
drift from what the engine registers.

    python -I Tools/ui/make_ui_reference.py [path/to/uidb.json]
"""
import html
import json
import os
import re
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
DB = Path(sys.argv[1]) if len(sys.argv) > 1 else REPO / "source/Games/StealthAction/SaveData/UIStudio/uidb.json"
OUT = REPO / "Docs/UI/UI_Reference.html"
db = json.loads(DB.read_text(encoding="utf-8"))
reg = db["registries"]


def esc(s):
    return html.escape(str(s), quote=False)


# ---------------------------------------------------------------------------------------------------------------- examples
EX = {}

EX["minimal"] = """<root>
  <styles>
    <include src="GameData/ui/theme.css"/>      <!-- shared look, a separate .css file -->
    <style>                                     <!-- and/or styles only this file needs -->
      .menu { flow-children: down; gap: 12px; padding: 24px; }
    </style>
  </styles>
  <Panel class="menu">
    <Label text="Hello {name}"/>
    <TextButton text="${MENU_PLAY}" onactivate="Push('GameData/ui/play.xml')"/>
  </Panel>
</root>"""

EX["structure"] = """<root>
  <imports>                                         <!-- optional: component files this file uses from elsewhere -->
    <import src="GameData/ui/shared/Fancy.xml"/>
  </imports>
  <styles>                                          <!-- optional, applied in the order written -->
    <include src="GameData/ui/theme.css"/>
    <style> .row { height: 40px; } </style>
  </styles>
  <snippets>                                        <!-- optional: file-local templates (list rows, built from code) -->
    <snippet name="Row" params="index=0">
      <Label text="Item {index}"/>
    </snippet>
  </snippets>
  <Panel id="page">                                 <!-- exactly one top element: any tag, or <Component> -->
    ...
  </Panel>
</root>"""

EX["richbasic"] = """<Label html="true"
       text="Plain &lt;b&gt;bold&lt;/b&gt; &lt;i&gt;italic&lt;/i&gt; &lt;color=#ff4040&gt;red&lt;/color&gt; &lt;size=40&gt;big&lt;/size&gt;"/>"""

EX["richicons"] = """<!-- the key or button that triggers the action, for the device the player used last -->
<Label html="true" text="Press &lt;icon=action:interact&gt; to open, &lt;icon=action:attack&gt; to strike"/>

<!-- an image from GameData, sized to the text -->
<Label html="true" text="&lt;icon=img:GameData/textures/ui/coin.png&gt; x {coins}"/>"""

EX["richnest"] = """<Label html="true" style="font-size: 28px; width: 420px"
       text="&lt;color=#ffd24a&gt;&lt;b&gt;Critical!&lt;/b&gt;&lt;/color&gt; Hit for
             &lt;size=44&gt;&lt;color=#ff5030&gt;{damage}&lt;/color&gt;&lt;/size&gt; damage.
             Press &lt;icon=action:block&gt; to block."/>"""

EX["richvar"] = """<!-- code: label->SetVar("hp", 42); label->SetVar("max", 100); -->
<Label html="true" text="&lt;color=#ff6060&gt;{hp}&lt;/color&gt; / {max}"/>"""

EX["textfx"] = """<styles><style>
  .big    { font-size: 72px; color: #ffffff; }
  .stroke { text-stroke: 4px #ff0000; }
  .hard   { text-shadow: 6px 6px 0 #000000; }
  .glow   { text-shadow: 0 0 12px #00ffff, 6px 6px 8px rgba(0, 0, 0, 0.8); }   /* several shadows, with blur */
  .both   { text-stroke: 3px #0000ff; text-shadow: 5px 5px 4px #000000; color: #ffff00; }
</style></styles>
<Label class="big stroke" text="Stroke"/>
<Label class="big glow"   text="Glow and soft"/>"""

EX["textlayout"] = """<styles><style>
  #wrap  { width: 200px; font-size: 24px; }                                   /* wraps at 200px */
  #dots  { width: 150px; white-space: nowrap; text-overflow: ellipsis; }     /* "This line is far…" */
  #fit   { width: 150px; height: 40px; text-overflow: shrink; }               /* font shrinks until it fits */
  #shout { text-transform: uppercase; letter-spacing: 2px; }
  #mid   { width: 300px; text-align: center; line-height: 1.4; }
  #log   { white-space: pre; }                                                /* keeps line breaks and tabs */
</style></styles>"""

EX["fontface"] = """@font-face {
  font-family: "Display";
  src: url("GameData/fonts/Display-Bold.ttf");
  font-weight: 700;                 /* one face per weight (and per italic) */
}
.title { font-family: "Display"; font-weight: bold; }"""

EX["vars"] = """<Label text="Hello {name}"/>                         <!-- {name}: variable from the nearest ancestor that set it -->
<Label text="{value:%.2f}"/>                           <!-- number formatting: %f, %d, %i with width/precision -->
<Label text="Score {score:%06d}"/>                     <!-- 000042 -->
<Label text="${PAUSE_MENU_RESUME}"/>                   <!-- localisation key -->
<Label text="${HINT_PRESS}"/>                          <!-- a translated string may itself contain {key} variables -->
<Label>Centered{{braces}}</Label>                      <!-- {{ and }} are literal braces -->
<Panel visible="{hint}"/>                              <!-- attributes can use variables too -->"""

EX["varcpp"] = """label->SetVar("name", "Ann");          // text
label->SetVar("score", 42.0);              // numbers keep their value, so {score:%06d} works
page->SetVar("hp", hp);                    // inherited down the tree: every element below sees it
// setting a variable to the value it already has costs one compare"""

EX["selectors"] = """Label            { color: white; }              /* type */
.menu-item       { height: 60px; }              /* class */
#resume          { width: 300px; }              /* id */
.menu .item      { gap: 4px; }                  /* descendant */
.menu > .item    { gap: 4px; }                  /* child */
Button:hover, Button:focus { background-color: #333; }   /* list + states */
.row:focus-within { background-color: var(--kit-row-alt); }
.page:entering { opacity: 0; }                  /* a page that is being pushed */
:root { --accent: #ffcc00; }                    /* custom properties */
.btn { border: 2px solid var(--accent); }"""

EX["units"] = """width: 300px;            /* px: one UI unit = one pixel of the 1080-tall UI target */
width: 50%;               /* of the parent's content box */
font-size: 1.2rem;        /* rem follows the player's text-size setting */
font-size: 2em;           /* em: this element's font size */
height: 10vh; width: 20vw;
width: calc(100% - 48px); /* + - * / on lengths, numbers divide and multiply */
color: #ff8800;           color: #f80;   color: #ff880080;
color: rgb(255, 136, 0);  color: rgba(255, 136, 0, 0.5);   color: hsl(30, 100%, 50%);"""

EX["layout"] = """.column { flow-children: down;  gap: 12px; }          /* children stacked top to bottom */
.row    { flow-children: right; gap: 8px; }           /* left to right */
.grid   { flow-children: right-wrap; gap: 8px; }      /* wraps to the next line */
.stack  { flow-children: none; }                      /* default: children overlay each other */

.fill   { width: fill-parent-flow(1); }               /* takes the free space left in the flow, by weight */
.half   { width: fill-parent-flow(1); }  .rest { width: fill-parent-flow(2); }   /* 1 : 2 */
.fit    { width: fit-children; }                      /* default: as big as the content */
.square { height: 200px; width: height-percentage(100%); }   /* aspect lock */

.centered { horizontal-align: center; vertical-align: center; }
.corner   { horizontal-align: right; vertical-align: bottom; x: -24px; y: -24px; }
.card     { margin: 8px 16px; padding: 12px; min-width: 200px; max-width: 600px; }
.list     { overflow: scroll; height: 400px; }        /* clip + wheel, drag and gamepad scrolling */"""

EX["scroll"] = """<styles><style>
  #list      { width: 360px; height: 220px; overflow: scroll; flow-children: down; gap: 6px; }
  .scrollbar { width: 8px; background-color: #333333; }                       /* the track */
  .scrollbar .thumb { background-color: #aaaaaa; border-radius: 4px; }          /* the handle: drag it with the mouse */
</style></styles>
<Panel id="list" scrollbar="true">
  <Button class="row"><Label text="Row 0"/></Button>
  ...
</Panel>"""

EX["paint"] = """.panel {
  background-color: #14100c;
  background-image: linear-gradient(to bottom, rgba(255,255,255,0.08), rgba(255,255,255,0)),
                    url("GameData/textures/ui/grain.png");
  background-size: auto, 128px;                          /* one value per layer */
  background-repeat: no-repeat, repeat;
  border: 2px solid #8c2929;
  border-radius: 12px 12px 0 0;                         /* per corner */
  box-shadow: 0 8px 24px 0 rgba(0,0,0,0.6), inset 0 0 0 1px rgba(255,255,255,0.08);
  outline: 2px solid #ffd24a;  outline-offset: 4px;
}
.ring   { background-image: conic-gradient(from 0deg, #f00, #ff0, #0f0, #0ff, #00f, #f0f, #f00); border-radius: 50%; }
.stripe { background-image: repeating-linear-gradient(45deg, #000 0 10px, #222 10px 20px); }
.frame  { border-image: url("GameData/textures/ui/frame.png") 24 / 24px round; }   /* nine-slice */"""

EX["effects"] = """.dim      { filter: brightness(0.6) saturate(0.8); }        /* colour matrix filters */
.ghost    { opacity: 0.5; }
.tint     { wash-color: rgba(255, 0, 0, 0.25); }            /* tint over everything, images included */
.glowfx   { mix-blend-mode: plus-lighter; }                  /* additive: sparks, energy bars */
.fade     { mask-image: linear-gradient(to bottom, #000, transparent); }
.round    { clip-path: circle(50%); }
.tilt     { transform: rotate(-3deg) scale(1.1); transform-origin: 50% 100%; }   /* layout ignores it, clicks follow it */
.pix      { image-rendering: pixelated; }
.cover    { object-fit: cover; object-position: 50% 20%; }   /* on <Image> */"""

EX["layers"] = """.glass   { backdrop-filter: blur(8px) brightness(0.8); border-radius: 12px; }   /* the scene behind the panel, blurred */
.soft    { filter: blur(3px); }                                          /* the element and everything in it */
.glow    { filter: drop-shadow(0px 0px 16px #00ff00); }                  /* follows the alpha of the content: offset 0 is a glow */
.lift    { filter: drop-shadow(6px 8px 6px rgba(0, 0, 0, 0.6)); }
.group   { opacity: 0.5; isolation: isolate; }                           /* overlapping children fade as one picture */
.static  { cache: layer; }                                               /* rendered once, reused until its content changes */"""

EX["dragdrop"] = """<Panel id="inventory">
  <TextButton id="sword" text="Sword" draggable="true" ondragstart="AddClass('#sword','held')"/>
  <Panel id="slot1" class="slot" droptarget="true"
         ondragenter="AddClass('#slot1','over')" ondragleave="RemoveClass('#slot1','over')" ondrop="AddClass('#slot1','full')"/>
</Panel>
<!-- .slot.over { border-color: #00ff00; }   #sword:dragging { opacity: 0.4; } -->"""

EX["dragcode"] = """// Code-behind: the payload is C++ only, set when the drag starts and read where it ends.
sword->On("dragstart", [sword] { sword->DragPayload = ItemId{ 42 }; });
slot->On("drop", [slot] {
    UiElem* from = UiDragDrop::Source();
    if (from) if (auto* item = std::any_cast<ItemId>(&from->DragPayload)) MoveItem(*item, slot);
});"""

EX["context"] = """<Panel id="row" hittest="true" oncontextmenu="ContextMenu('GameData/ui/my_menu.xml')"/>

<!-- my_menu.xml: the element with class context-menu is placed at the pointer; the page itself covers the screen -->
<root>
  <Panel id="menu_page" shows-below="true" close-on-back="true">
    <Panel class="context-menu"><TextButton text="Drop" onactivate="Back()"/></Panel>
  </Panel>
</root>"""

EX["motion"] = """.card         { transition: opacity 0.3s ease, transform 0.3s cubic-bezier(0.2, 0.8, 0.2, 1); }
.card:hover   { transform: scale(1.05); }
.page:entering { opacity: 0; transform: translateY(24px); }       /* a pushed page starts here and animates to normal */
.page:leaving  { opacity: 0; }                                      /* a popped page animates to this, then is removed */

.pulse { animation: pulse 1.2s ease-in-out infinite alternate; }
@keyframes pulse {
  from { opacity: 1; }
  50%  { opacity: 0.4; }
  to   { opacity: 1; }
}
.steps { transition: width 0.5s steps(5); }"""

EX["events"] = """<TextButton text="Play"
            onactivate="PlaySound('GameData/sounds/ui/confirm.wav'); Push('GameData/ui/play.xml')"
            onmouseover="AddClass('#hint', 'show')"
            onmouseout="RemoveClass('#hint', 'show')"/>

<Panel id="page" default-focus="false" oncancel="PlaySound('x.wav')" onload="SetFocus('#play')">"""

EX["codebehind"] = """class MyPage : public UiElemPanel
{
public:
    MyPage()
    {
        LoadLayout("GameData/ui/mypage.xml");                       // builds the markup onto this element

        On("Play", [this] { StartGame(); });                         // onactivate="Play()" finds this handler
        On("Pick", [this](UiElem& sender, const std::vector<std::string>& args) {
            // onactivate="Pick('sword')": args[0] == "sword"
        });
        if (auto* name = Find<UiElemLabel>("title")) name->SetVar("name", "Ann");
    }
    void OnPushed() override { SetClass("fresh", true); }            // menu lifecycle hooks
    bool OnBack() override   { return false; }                        // true = swallow back (e.g. "discard changes?")
    void Tick() override     { SetVar("hp", Player::Hp()); }          // every frame, before layout
};
UI_REGISTER_CODE_BEHIND("GameData/ui/mypage.xml", MyPage);           // Push("GameData/ui/mypage.xml") now builds MyPage"""

EX["tagreg"] = """// Handler lifetime: a handler registered from an entity or component should be guarded,
// so it is skipped after the owner is destroyed:
elem->On("Buy", owner /* std::weak_ptr<void> */, [this] { Buy(); });

// Created from code:
UiElem* row = page->AddSnippet("Row", ...);               // a <snippet> of this file
auto root = UiElem::Load("GameData/ui/hud.xml");            // a layout as a root (hosted by you)
// Hosting a root yourself (a HUD, for instance): add it to the viewport, e.g. viewport.AddChild(root);"""

EX["component_def"] = """<!-- GameData/ui/components/SettingRow.xml -->
<root>
  <Component tag="SettingRow" params="label, hint=''">      <!-- parameters; hint has a default, label is required -->
    <Panel class="row">
      <Label class="label" text="{label}"/>
      <Label class="hint"  text="{hint}" visible="{hint}"/>
      <Children/>                                             <!-- the instance's children go here -->
      <Children slot="footer"/>                               <!-- named slot -->
    </Panel>
  </Component>
</root>"""

EX["component_use"] = """<SettingRow label="${VIDEO_VSYNC}" class="advanced" onchanged="Apply()">
  <ToggleButton id="vsync"/>                                  <!-- slotted children belong to the using file: Find("vsync") works -->
  <Panel slot="footer"><Label text="Needs a restart"/></Panel>
</SettingRow>

<!-- inside the component:  <Raise>  fires the instance's own event:  onactivate="Raise('changed')" -->"""

EX["frame"] = """<Frame id="content" src="GameData/ui/kit/pages/sound.xml"/>     <!-- builds a whole layout inside; the page changes with src -->
<Frame snippet="Row"/>                                                <!-- or a snippet / component of this file -->
<!-- code: frame->SetAttribute("src", "GameData/ui/kit/pages/video.xml");  rebuilds the content -->"""

EX["stack"] = """<Panel id="pause" pauses-game="true" hides-hud="true">         <!-- host options: read by the menu stack -->
  <Button id="resume" default-focus="true" onactivate="Back()">...</Button>
  <TextButton text="Settings" onactivate="Push('GameData/ui/kit/pages/settings.xml')"/>
</Panel>

<Panel id="confirm" shows-below="true" close-on-back="true">   <!-- a dialog: the page below stays visible -->
  ...
</Panel>"""

EX["stackcpp"] = """UiMenus::Push("GameData/ui/kit/pages/pause.xml");     // the main stack; any element can be a page
UiMenus::Stack().Replace("GameData/ui/other.xml");
UiMenus::Pop();   UiMenus::Clear();   bool open = UiMenus::IsOpen();

UiMenus::OnPauseChanged = [](bool paused) { ... };    // the game connects its own pause here"""

EX["nav"] = """<Button id="apply" nav-up="#slider" nav-left="#cancel"/>      <!-- override spatial navigation where it guesses wrong -->
<Button id="play"  default-focus="true"/>                   <!-- first focus of a page -->
<TextButton text="Reset" hotkey="ui_reset" hotkey-label="${UI_RESET}"/>   <!-- an input action activates it while its page is on top -->
<ActionBar class="action-bar"/>                              <!-- footer: one prompt + label per hotkey button of the top page, plus Back -->
<InputPrompt action="ui_confirm"/>                           <!-- image of the key/button for the last-used device -->

<styles><style>
  .input-gamepad .mouse-hint { visibility: collapse; }     /* the root gets input-mouse | input-gamepad | input-touch */
</style></styles>"""

EX["bind"] = """<Slider id="vol" min="0" max="1" step="0.05" bind="setting:audio.master"/>
<ToggleButton id="vsync" bind="setting:video.vsync"/>
<DropDown id="mode" bind="setting:video.window_mode"/>             <!-- items come from the setting -->
<RadioButton group="aa" option="0" bind="setting:video.msaa"/>     <!-- this radio selects value 0 -->
<Label text="{value:%.2f}"/>                                         <!-- sliders expose their value as the variable {value} -->

<!-- page option: leaving with changed bound settings asks "apply or discard?" -->
<Panel confirm-changes="true"> ... </Panel>"""

EX["bindcpp"] = """UiSettings::Entry e;
e.Key = "game.fov";   e.Type = UiSettings::Kind::Float;   e.Min = 60;  e.Max = 120;  e.Step = 1;
e.Label = "${SETTINGS_FOV}";
e.Get = [] { return Camera::Fov(); };
e.Set = [](double v) { Camera::SetFov((float)v); };
UiSettings::Register(e);          // now bind="setting:game.fov" works in any page"""

EX["widgets"] = """<Slider id="sl" min="0" max="1" step="0.1" value="0.5" style="width: 300px; height: 28px; --track-color: #262626; --handle-color: #e0a030"/>
<DropDown id="dd" options="Low|Medium|High" selected-index="0"/>
<TextEntry id="te" placeholder="name" max-length="16"/>
<TextEntry password="true"/>      <TextEntry numeric="true"/>
<ToggleButton id="tg" class="tog"><Label text="Toggle"/></ToggleButton>
<RadioButton id="r1" group="g" checked="true"><Label text="A"/></RadioButton>
<RadioButton id="r2" group="g"><Label text="B"/></RadioButton>

<styles><style>
  .tog:checked  { background-color: #2c8040; }       /* :checked on toggles and radios */
  .tog:focus    { border: 2px solid #ffffff; }
</style></styles>
<!-- events: onvaluechanged (slider, drop-down, text entry, toggle, radio) -->"""

EX["progress"] = """<styles><style>
  #bar  { width: 400px; height: 20px; background-color: #222; }      /* the track */
  #bar .fill { background-color: #d04030; }                           /* the fill: a child, sized by value */
  #ring { width: 100px; height: 100px; color: #40c0ff; background-color: #223344; }   /* color = fill, background = track */
  #cd   { width: 64px; height: 64px; color: #fff; }
</style></styles>
<ProgressBar id="bar" value="0.3"/>                          <ProgressBar value="0.5" direction="vertical"/>
<CircularProgress id="ring" value="0.25" thickness="12"/>
<Arc id="cd" start="0" sweep="270" thickness="6" round-caps="true"/>     <!-- a cooldown ring: sweep it from code -->
<!-- code: bar->SetVar / SetAttribute("value", "0.6");  with a transition on .fill { width } the bar eases -->"""

EX["listview"] = """<snippets>
  <snippet name="SaveRow" params="index=0">
    <Panel class="lrow"><Label text="Save {index}"/></Panel>
  </snippet>
</snippets>
<ListView id="list" snippet="SaveRow" count="1000" item-height="40"/>     <!-- only the visible rows exist; rows are recycled -->
<ListView snippet="Cell" count="200" layout="grid" columns="4" item-height="120"/>"""

EX["svg"] = """<Image src="GameData/ui/icons/sword.svg" style="width: 32px; height: 32px"/>      <!-- crisp at any size -->
.badge { background-image: url("GameData/ui/icons/star.svg"); background-size: 24px 24px; }"""

EX["media"] = """<Image src="GameData/textures/ui/logo.png" style="object-fit: contain; width: 300px; height: 120px"/>
<Movie src="GameData/videos/intro.mp4" loop="true"/>
<ShaderPanel shader="ui/fs_noise" param-tint="1 0.5 0 1" style="width: 200px; height: 200px"/>   <!-- vec4 uniforms as 'x y z w' -->"""

EX["hot"] = """# Run any non-DISTRIBUTION build, edit a .xml / .css / component file under source/Games/<Game>/GameData/ui and save.
# Within about a second the page on screen is rebuilt from the new files. Kept: slider/drop-down/text values, toggles,
# scroll positions, focus, classes and variables set from code, and attributes the page's code set (the open tab, a Frame's src)."""

EX["console"] = """ui_push GameData/ui/kit/pages/settings.xml     ui_pop     ui_back     ui_clear_stack     ui_stack
ui_load <layout>        ui_unload_all          ui_focus <id>      ui_dump [depth]    (prints the element tree)
ui_pseudoloc            (every string about 35% longer, to find overflow)       ui_reduced_motion
ui_overflow_report      ui_db_info    ui_db_path   (the uidb.json catalogue)"""

EX["uitest"] = """# StealthAction: pause menu
wait 8
action pause                              # an input action
wait_frames 10
expect_stack depth == 1
expect_stack top contains pause.xml
expect presume focus == 1                 # id, property, comparison
action ui_down
wait_frames 3
expect psave focus == 1
console ui_focus psettings                # any console command
ui_screenshot 02_pause
set_var name Ann    set_class #box hot    sim_device gamepad    mouse_down 662 65    mouse_up    wheel -3"""

EX["gamewiring"] = """// Game start: the kit pages need the game's pause hook, and the game supplies the look in GameData/ui/theme.css.
KitSetupMenus([](bool paused) { PauseGameManager::SetPaused(paused); });

KitPauseMenu::CanSave  = [] { return !Player::InCutscene(); };
KitPauseMenu::OnRestart = [] { Level::Restart(); };             // shows Restart (after a confirmation)
KitSettingsMenu::RegisterTab("${SETTINGS_GAMEPLAY}", "GameData/ui/gameplay_tab.xml");

KitConfirm::Ask("${QUIT_ASK}", [] { Quit(); });                 // yes/no on the main stack
KitSaveLoad::Open(KitSaveLoad::Mode::Save);
KitRebind::Open(request);                                        // key / button capture page"""

EX["theme"] = """/* GameData/ui/theme.css: the game's look. kit.css only lays things out and reads these variables. */
@import "GameData/ui/kit/styles/kit.css";
:root {
  --kit-accent: #99c7ff;
  --kit-panel: rgba(18, 9, 9, 0.95);
  --kit-radius: 3px;
  --kit-title-size: 1.6rem;
  --kit-menu-glow: 0 0 14px rgba(255, 70, 40, 0.95);     /* a whole text-shadow value */
}
/* a page: <styles><include src="GameData/ui/theme.css"/></styles> */"""

EX["kitpage"] = """<!-- A page built from kit pieces -->
<root>
  <styles><include src="GameData/ui/theme.css"/></styles>
  <Panel class="kit-screen" confirm-changes="true">
    <Panel class="kit-column">
      <SettingRow label="Volume">
        <Slider min="0" max="1" step="0.05" bind="setting:audio.master"/>
        <Label class="setting-value" text="{value:%.2f}"/>
      </SettingRow>
      <SettingRow label="Invert Y">
        <ToggleButton class="kit-toggle" bind="setting:input.invert_y"><Label class="on" text="On"/><Label class="off" text="Off"/></ToggleButton>
      </SettingRow>
      <StyledButton text="Back" onactivate="Back()"/>
    </Panel>
  </Panel>
</root>"""

EX["kitcard"] = """<Card title="Controls">
  <Label text="Move with WASD"/>
  <StyledButton text="Rebind" onactivate="Push('GameData/ui/kit/pages/input.xml')"/>
</Card>

<DialogFrame title="Quit?">
  <Label text="Unsaved progress is lost."/>
  <Panel class="dialog-row">
    <TextButton class="dialog-button" text="Yes" onactivate="Quit()"/>
    <TextButton class="dialog-button" text="No" default-focus="true" onactivate="Back()"/>
  </Panel>
</DialogFrame>

<TabBar> <TextButton text="Video"/> <TextButton text="Sound"/> </TabBar>
<Toast text="Game saved"/>"""

# ---------------------------------------------------------------------------------------------------------------- tag notes
TAG_NOTE = {
    "Panel": ("The box: background, border, radius, shadow and the container of other elements. Its <code>flow-children</code> decides how children are placed.",
              '<Panel class="menu"> ... </Panel>'),
    "Label": ("Text. Content comes from the <code>text</code> attribute or from the element's own text. Wraps, aligns, ellipsizes, "
              "and with <code>html=\"true\"</code> understands rich text (see <a href=\"#richtext\">Rich text</a>).",
              '<Label text="Score {score:%06d}" style="font-size: 32px"/>'),
    "Image": ("A texture drawn with <code>object-fit</code> / <code>object-position</code>.", '<Image src="GameData/textures/ui/logo.png" style="object-fit: contain"/>'),
    "Button": ("Focusable, clickable container: it can hold any children. States <code>:hover :active :focus :disabled</code>.",
               '<Button onactivate="Back()"><Label text="Resume"/></Button>'),
    "TextButton": ("A Button with a label made from <code>text</code>.", '<TextButton text="Play" onactivate="Push(\'GameData/ui/play.xml\')"/>'),
    "ToggleButton": ("A Button that toggles <code>:checked</code>. Event <code>onvaluechanged</code>; variable <code>value</code> (0/1).",
                     '<ToggleButton bind="setting:video.vsync"><Label text="VSync"/></ToggleButton>'),
    "RadioButton": ("One of a group. Radios with the same <code>group</code> inside one root exclude each other.",
                    '<RadioButton group="aa" checked="true"><Label text="Off"/></RadioButton>'),
    "Slider": ("Draggable value. Event <code>onvaluechanged</code>; variable <code>value</code>. Colours come from <code>color</code>, <code>--track-color</code>, <code>--handle-color</code>.",
               '<Slider min="0" max="1" step="0.05" value="0.5"/>'),
    "DropDown": ("A list of choices. Event <code>onvaluechanged</code>; variable <code>value</code> (the index).", '<DropDown options="Low|Medium|High" selected-index="1"/>'),
    "TextEntry": ("Single-line text field with caret and IME. Event <code>onvaluechanged</code>; variable <code>value</code> (the text).", '<TextEntry placeholder="Name" max-length="16"/>'),
    "ProgressBar": ("A track with a <code>.fill</code> child whose size is value × content.", '<ProgressBar value="0.3"/>'),
    "CircularProgress": ("A ring: <code>background-color</code> is the track, <code>color</code> the fill.", '<CircularProgress value="0.25" thickness="12"/>'),
    "Arc": ("A ring segment for cooldowns and radial gauges; the colour is the text colour.", '<Arc start="0" sweep="270" thickness="6" round-caps="true"/>'),
    "Movie": ("A video.", '<Movie src="GameData/videos/intro.mp4" loop="true"/>'),
    "ShaderPanel": ("A panel drawn by a custom pixel shader. Uniforms are <code>param-NAME=\"x y z w\"</code>.", '<ShaderPanel shader="ui/fs_noise" param-tint="1 0.5 0 1"/>'),
    "Frame": ("Builds another layout (<code>src</code>) or a snippet (<code>snippet</code>) inside itself. Changing <code>src</code> rebuilds the content: this is how tab pages work.",
              '<Frame id="content" src="GameData/ui/kit/pages/sound.xml"/>'),
    "ListView": ("A virtualised list or grid: only visible rows are built from the snippet and recycled. Needs <code>overflow: scroll</code>; rows get the variable <code>index</code>.",
                 '<ListView snippet="Row" count="1000" item-height="40"/>'),
    "InputPrompt": ("The image of the key or button that triggers an input action, for the device used last. It collapses when no image exists.", '<InputPrompt action="interact"/>'),
    "ActionBar": ("A footer that lists one prompt and label per hotkey button of the top page, plus a Back entry when the page closes on back.", '<ActionBar class="action-bar"/>'),
}

COMMON_ATTRS = {"id", "class", "style", "visible", "enabled", "selected", "hittest", "default-focus", "close-on-back", "shows-below", "pauses-game", "hides-hud",
                "confirm-changes", "nav-up", "nav-down", "nav-left", "nav-right", "onactivate", "onmouseover", "onmouseout", "onfocus", "onblur", "oncancel", "onload"}

# ---------------------------------------------------------------------------------------------------------------- generated parts


def table(head, rows, cls=""):
    out = ['<div class="tw"><table class="%s"><thead><tr>%s</tr></thead><tbody>' % (cls, "".join("<th>%s</th>" % h for h in head))]
    for r in rows:
        out.append("<tr>%s</tr>" % "".join("<td>%s</td>" % c for c in r))
    out.append("</tbody></table></div>")
    return "\n".join(out)


def pre(key):
    return '<pre><code>%s</code></pre>' % esc(EX[key])


def tags_reference():
    out = []
    own = [t for t in reg["tags"] if not t.get("legacy")]
    cats = {}
    for t in own:
        cats.setdefault(t.get("category") or "other", []).append(t)
    for t in sorted(own, key=lambda t: t["name"]):
        note, ex = TAG_NOTE.get(t["name"], (esc(t.get("description", "")), None))
        attrs = [a for a in t["attributes"] if a["name"] not in COMMON_ATTRS]
        out.append('<div class="tag" id="tag-%s"><h4><code>&lt;%s&gt;</code> <span class="chip">%s</span></h4><p>%s</p>' % (t["name"], t["name"], esc(t.get("category", "")), note))
        if attrs:
            out.append(table(["Attribute", "Type", "Default", "Meaning"],
                             [["<code>%s</code>" % esc(a["name"]), esc(a.get("type", "")), "<code>%s</code>" % esc(a.get("default", "")) if a.get("default", "") != "" else "",
                               esc(a.get("description", ""))] for a in attrs]))
        else:
            out.append("<p class=\"dim\">No attributes of its own (only the common ones).</p>")
        if ex:
            out.append("<pre><code>%s</code></pre>" % esc(ex))
        out.append("</div>")
    return "\n".join(out)


def common_attrs_table():
    t = next(t for t in reg["tags"] if t["name"] == "Panel")
    rows = []
    for a in t["attributes"]:
        if a["name"] in COMMON_ATTRS:
            rows.append(["<code>%s</code>" % esc(a["name"]), esc(a.get("type", "")), "<code>%s</code>" % esc(a.get("default", "")) if a.get("default", "") != "" else "", esc(a.get("description", ""))])
    return table(["Attribute", "Type", "Default", "Meaning"], rows)


def css_reference():
    groups = [
        ("Layout", lambda n: n in ("flow-children", "width", "height", "min-width", "max-width", "min-height", "max-height", "horizontal-align", "vertical-align", "x", "y", "gap", "overflow", "visibility", "z-index") or n.startswith(("margin", "padding"))),
        ("Background and images", lambda n: n.startswith(("background", "object", "image-rendering", "mask"))),
        ("Border, outline, shadow", lambda n: n.startswith(("border", "outline", "box-shadow", "clip-path"))),
        ("Text", lambda n: n in ("color", "font-family", "font-size", "font-style", "font-weight", "letter-spacing", "line-height", "text-align", "text-overflow", "text-shadow", "text-transform", "white-space", "text-stroke", "text-stroke-color", "text-stroke-width", "-webkit-text-stroke")),
        ("Effects and transforms", lambda n: n in ("opacity", "filter", "wash-color", "mix-blend-mode", "transform", "transform-origin")),
        ("Motion", lambda n: n.startswith(("animation", "transition"))),
        ("Sound", lambda n: n.startswith("sound")),
    ]
    used = set()
    out = []
    for title, pred in groups:
        rows = []
        for p in reg["cssProperties"]:
            if p["name"] in used or not pred(p["name"]):
                continue
            used.add(p["name"])
            flags = []
            if p.get("inherited"):
                flags.append("inherited")
            if p.get("shorthand"):
                flags.append("shorthand")
            rows.append(["<code>%s</code>" % esc(p["name"]), "<code>%s</code>" % esc(p.get("grammar", "")), "<code>%s</code>" % esc(p.get("initial", "")), esc(", ".join(flags))])
        if rows:
            out.append("<h4>%s</h4>" % title)
            out.append(table(["Property", "Values", "Initial", ""], rows))
    rest = [p for p in reg["cssProperties"] if p["name"] not in used]
    if rest:
        out.append("<h4>Other</h4>" + table(["Property", "Values", "Initial", ""], [["<code>%s</code>" % esc(p["name"]), "<code>%s</code>" % esc(p.get("grammar", "")), "<code>%s</code>" % esc(p.get("initial", "")), ""] for p in rest]))
    return "\n".join(out)


def paint_reference():
    rows = []
    for p in reg["paintEffects"]:
        if p.get("kind") == "element":
            continue
        req = p.get("unavailableReason", "not available") if not p.get("available", True) else ""
        rows.append(["<code>%s</code>" % esc(p["name"]), esc(p.get("category", "")), esc(p.get("description", "")), "<code>%s</code>" % esc(p.get("parameters", "")),
                     '<span class="chip warn">%s</span>' % req if req else '<span class="chip ok">available</span>'])
    return table(["Effect", "Kind", "What it does", "Syntax", "Status"], rows)


def anim_reference():
    names = [a["name"] for a in reg["animations"] if not a["name"].startswith("@")]
    return "<p>" + " ".join("<code>%s</code>" % n for n in names) + "</p>"


def actions_reference():
    rows = []
    for a in reg["actions"]:
        if a.get("kind") == "action":
            rows.append(["<code>%s(%s)</code>" % (esc(a["name"]), esc(a.get("arguments", ""))), esc(a.get("description", ""))])
    ev = [[("<code>%s</code>" % esc(a["attribute"])), esc(a.get("description", ""))] for a in reg["actions"] if a.get("kind") == "event"]
    return table(["Action", "What it does"], rows), table(["Event attribute", "Fires when"], ev)


def kit_components():
    rows = []
    notes = {
        "Card": "A titled card: header, divider, then your children.",
        "DialogFrame": "The frame of a dialog: title, accent line and your children.",
        "KeyBindSlot": "One key/button slot of the controls page (icon + name). Used by the input page's code.",
        "SettingRow": "A settings row: the label on the left, your controls on the right.",
        "StyledButton": "A TextButton with the kit's button look.",
        "TabBar": "The row of tab buttons of the settings page.",
        "Toast": "A short message box.",
    }
    for c in reg["components"]:
        if not c["path"].startswith("ui/kit/"):
            continue
        params = ", ".join(("<code>%s</code>%s" % (esc(p["name"]), "" if p.get("required") else " (optional)")) for p in c["parameters"]) or "none"
        slots = "yes" if c["slots"] else "no"
        rows.append(["<code>&lt;%s&gt;</code>" % esc(c["tag"]), esc(notes.get(c["tag"], "")), params, slots, "<code>%s</code>" % esc(c["path"])])
    return table(["Tag", "Purpose", "Parameters", "Children slot", "File"], rows)


def kit_pages():
    notes = {
        "pause": "Pause menu: Resume, Save, Load, Restart (if the game sets it), Settings, Quit. Pauses the game, hides the HUD. Code-behind <code>KitPauseMenu</code>.",
        "settings": "Tabbed settings (Video, Sound, Input, plus tabs the game registers). Code-behind <code>KitSettingsMenu</code>.",
        "video": "Video tab: resolution, window mode, VSync, MSAA, FXAA, shadows. Bound to the <code>video.*</code> settings.",
        "sound": "Sound tab: master, music and effects volume.",
        "input": "Controls tab: one row per input action with keyboard and gamepad slots. Code-behind <code>KitInputPage</code>.",
        "rebind": "Modal that listens for the next key, mouse button or gamepad button. Opened with <code>KitRebind::Open</code>.",
        "saveload": "Save / load slot list. Open with <code>KitSaveLoad::Open</code>.",
        "confirm": "Yes / no dialog. Open with <code>KitConfirm::Ask</code>.",
        "confirm_changes": "\"Apply or discard changed settings?\" prompt shown by pages with <code>confirm-changes=\"true\"</code>.",
    }
    return table(["Page", "What it is", "File"], [["<code>%s</code>" % esc(p["name"]), notes.get(p["name"], ""), "<code>%s</code>" % esc(p["path"])] for p in reg["kitPages"]])


def kit_tokens():
    return "<p>" + " ".join("<code>%s</code>" % esc(t["name"]) for t in reg["themeTokens"]) + "</p>"


def kit_settings():
    rows = []
    for s in reg["settings"]:
        rng = ""
        if "min" in s:
            rng = "%g .. %g%s" % (s["min"], s["max"], (" step %g" % s["step"]) if s.get("step") else "")
        rows.append(["<code>%s</code>" % esc(s["key"]), esc(s["type"]), rng, "<code>%s</code>" % esc(s.get("label", ""))])
    return table(["Key (bind=\"setting:…\")", "Type", "Range", "Label key"], rows)


def pseudo_list():
    d = {"hover": "pointer over the element", "active": "being pressed", "focus": "has navigation focus", "focus-within": "it or a descendant has focus",
         "disabled": "enabled=\"false\"", "selected": "selected=\"true\"", "checked": "toggle / radio is on", "entering": "page being pushed (first style pass)",
         "leaving": "page being popped (until the transition ends, 2 s at most)", "root": "the top element of a document instance"}
    return table(["Pseudo-class", "Matches"], [["<code>:%s</code>" % esc(p["name"]), esc(d.get(p["name"], ""))] for p in reg["pseudoClasses"]])


def input_actions():
    return "<p>" + " ".join("<code>%s</code>" % esc(a["name"]) for a in reg["inputActions"]) + "</p>"


def test_cmds():
    return "<p>" + " ".join("<code>%s</code>" % esc(a["name"]) for a in reg["testCommands"]) + "</p>"


# ---------------------------------------------------------------------------------------------------------------- page
actions_t, events_t = actions_reference()

BODY = """
<header class="hero">
  <h1>UI System Reference</h1>
  <p>Markup (XML), styles (a CSS dialect), text, motion, menus and tooling of the engine's UI. Built from the game's <code>uidb.json</code> (engine build %(build)s, game %(game)s): the tables are what the engine registered.</p>
  <p class="legend"><span class="badge core">Core</span> engine functionality, the same for every game.
     <span class="badge kit">Kit</span> the shared menu kit built <em>on top of</em> the core: pages, components and a theme. A game can use the core without the kit.</p>
</header>

<section id="overview"><h2>1. Overview <span class="badge core">Core</span></h2>
<p>A screen is an <b>XML file</b> (structure and events) plus <b>CSS files</b> (look). C++ is only needed for logic: a class registered for a layout (<em>code-behind</em>) can set variables and classes, handle actions and react to menu events. Data flows one way: the game sets variables and classes, the markup shows them.</p>
<table class="plain"><tr><th>You write</th><th>Where</th></tr>
<tr><td>Layouts, components</td><td><code>GameData/ui/**/*.xml</code></td></tr>
<tr><td>Styles</td><td><code>GameData/ui/**/*.css</code>, or a <code>&lt;style&gt;</code> block in the XML</td></tr>
<tr><td>Text</td><td><code>${KEY}</code> in the markup, strings in the game's localisation files</td></tr>
<tr><td>Logic</td><td>C++ code-behind (optional), registered with <code>UI_REGISTER_CODE_BEHIND</code></td></tr></table>
<p>Smallest useful page:</p>%(minimal)s
<p>Errors never pass silently: an unknown tag, attribute or malformed value is reported with <code>file:line</code> when the page loads. The <a href="#tooling">VS Code extension</a> shows the same problems while you type.</p>
<p><b>Coordinates.</b> The UI is always 1080 units tall (width follows the aspect ratio). One UI unit (<code>px</code>) is one pixel of that target, whatever the window size. Legacy <code>Ui*</code> widgets can sit inside markup, but new work should use the tags below.</p>
</section>

<section id="structure"><h2>2. File structure <span class="badge core">Core</span></h2>%(structure)s
<ul>
<li><b>Exactly one top element</b> inside <code>&lt;root&gt;</code>: any tag, or a <code>&lt;Component&gt;</code> (<a href="#components">Components</a>).</li>
<li><b>Styles</b> from <code>&lt;include&gt;</code> and <code>&lt;style&gt;</code> are applied in the order written, as one cascade. CSS files can <code>@import</code> each other (imports must come first in the file).</li>
<li><b>Escaping.</b> XML rules apply: write <code>&amp;lt;</code>, <code>&amp;gt;</code>, <code>&amp;amp;</code> in attribute values and text. This matters for <a href="#richtext">rich text</a>, whose tags must be escaped.</li>
<li><b>Braces.</b> <code>{name}</code> is a variable; write <code>{{</code> and <code>}}</code> for literal braces.</li>
<li>A page gets the class <code>page</code> when it is pushed on a menu stack, and the root gets <code>:root</code>.</li>
</ul></section>

<section id="elements"><h2>3. Elements <span class="badge core">Core</span></h2>
<p>These tags are built into the engine. Every element accepts the common attributes first, then its own.</p>
<h3>Common attributes</h3>%(common)s
<p class="dim"><code>on…</code> attributes hold a list of calls separated by <code>;</code> (see <a href="#events">Events and actions</a>). <code>close-on-back</code>, <code>shows-below</code>, <code>pauses-game</code>, <code>hides-hud</code>, <code>confirm-changes</code> are <em>host options</em>: only a menu stack reads them, on the page's top element.</p>
<h3>Tags</h3>%(tags)s
<h3>Widgets in practice</h3>%(widgets)s
<h3>Progress and gauges</h3>%(progress)s
<h3>Media</h3>%(media)s
<h3>Legacy elements</h3>
<p>The older <code>Ui*</code> classes (<code>UiImage</code>, <code>UiText</code>, <code>UiButton</code>, <code>UiVerticalBox</code>…) can be placed in markup with their own anchor attributes (<code>Position</code>, <code>Size</code>, <code>Origin</code>…). They are positioned in the parent's content box, and styled only by size and position. Use them only for code that already exists.</p>
</section>

<section id="richtext"><h2>4. Text and rich text <span class="badge core">Core</span></h2>
<p>All text is drawn by <code>&lt;Label&gt;</code>. It uses signed-distance-field glyphs, so one font works at every size, and stroke, shadow and glow come from the same glyphs. A line is shaped and wrapped once and reused until its text, font, size, width or variables change.</p>
<h3>Plain text</h3>
<p>Text comes from <code>text="…"</code> or from the element's own text. Variables <code>{x}</code> and localisation keys <code>${KEY}</code> are expanded first (see <a href="#vars">Variables</a>). Without <code>html="true"</code> a <code>&lt;</code> is just a character.</p>
<h3 id="rich">Rich text: <code>html="true"</code></h3>
<p>With <code>html="true"</code> the text may contain these tags. Tags nest, and an unclosed tag lasts to the end.</p>
%(richtable)s
%(richbasic)s
<div class="callout"><b>You must escape the angle brackets in XML.</b> Inside an attribute, write <code>&amp;lt;b&amp;gt;</code> for <code>&lt;b&gt;</code>. A literal <code>&lt;b&gt;</code> in the element's body would be read as a child element, not as text.</div>
<h4>Colour, size and style together</h4>%(richnest)s
<h4>Input icons that follow the player's device</h4>
<p><code>&lt;icon=action:NAME&gt;</code> asks the game's icon provider for the image of the key or button bound to the input action <code>NAME</code>, for the <em>last used</em> device (keyboard and mouse, gamepad, touch). When the player picks up a gamepad, every such icon in every label changes by itself. The same works as a separate element, <code>&lt;InputPrompt action="…"/&gt;</code>.</p>
%(richicons)s
<h4>Rich text with variables</h4>%(richvar)s
<h4>Details worth knowing</h4>
<ul>
<li><b>Icons</b> are squares of 1.1 × the current font size on the baseline, with a gap of 0.1 em; they scale with <code>&lt;size&gt;</code>. For line breaking an icon is an unbreakable word.</li>
<li><b>Bold and italic</b> use the font's own faces when they exist (see <code>@font-face</code>), otherwise the engine thickens or skews the glyphs.</li>
<li><b>Colours</b> are <code>#rrggbb</code> or <code>#rrggbbaa</code>. Anything else is an error for that tag.</li>
<li><b>Size</b> is in UI pixels and replaces the label's <code>font-size</code> for that run. It shrinks together with the label when <code>text-overflow: shrink</code> is used.</li>
<li><b>Unknown or malformed tags</b> (<code>&lt;foo&gt;</code>, <code>&lt;color=red&gt;</code>) are drawn literally and logged once as a warning.</li>
<li>Rich text composes with every text style below: stroke, shadows, transform, alignment, wrapping, ellipsis.</li>
</ul>
<h3>Text styling</h3>%(textfx)s
<h4>Layout of text</h4>%(textlayout)s
<h4>Fonts</h4>%(fontface)s
<p>A global fallback chain handles missing glyphs (for example CJK). Text breaks at spaces, hyphens and between CJK characters. An <code>text-align</code> value applies to wrapped lines.</p>
<h4>Not available</h4>
<p class="dim">Underline and strike-through, <code>font-variant-numeric</code>, justified text and right-to-left shaping are not implemented.</p>
</section>

<section id="vars"><h2>5. Variables and localisation <span class="badge core">Core</span></h2>%(vars)s
%(varcpp)s
<ul>
<li>A variable is looked up on the element, then up the tree. Components turn their parameters into variables of the instance.</li>
<li>Number formats: <code>%%f</code>, <code>%%d</code>, <code>%%i</code> with flags, width and precision (<code>%%.1f</code>, <code>%%06d</code>). A format on a non-number logs a warning.</li>
<li>Localisation runs first, so <code>"Press {key} to open"</code> can be a translated string. <code>ui_pseudoloc</code> makes every string about 35%% longer to expose overflow.</li>
<li>Elements re-evaluate only the attributes that mention the changed variable.</li>
</ul></section>

<section id="styles"><h2>6. Styles <span class="badge core">Core</span></h2>
<p>A CSS dialect with its own parser. It reports <code>file:line</code> for every bad declaration.</p>
<h3>Selectors</h3>%(selectors)s
<p>Supported: type, class, id, descendant, child, selector lists. Not supported: sibling and attribute selectors, <code>@media</code> (an unsupported at-rule is skipped with a warning). At-rules that work: <code>@import</code>, <code>@keyframes</code>, <code>@font-face</code>.</p>
<h3>Pseudo-classes</h3>%(pseudo)s
<h3>Values and units</h3>%(units)s
<h3>Precedence</h3>
<ol><li>Values set from C++ on the element (<code>elem-&gt;Style()</code>)</li><li>The <code>style="…"</code> attribute</li><li>Stylesheet rules by specificity, then source order</li><li>Inherited values (<code>color</code>, <code>font-*</code>, <code>text-*</code>, <code>letter-spacing</code>, <code>line-height</code>, <code>visibility</code>, <code>image-rendering</code>; <code>opacity</code> multiplies down)</li><li>Defaults</li></ol>
<p>Custom properties (<code>--name</code>) inherit and are used with <code>var(--name)</code>. They are how a theme works.</p>
<h3>Component style scope</h3>
<p>Rules in a component file are rewritten to match only inside that component, so they never leak. A page can still restyle a component on purpose.</p>
</section>

<section id="layout"><h2>7. Layout <span class="badge core">Core</span></h2>
<p>Layout is Panorama-style, not flexbox: <code>flow-children</code> chooses how children are placed, sizes are <code>fit-children</code> by default, and <code>fill-parent-flow(n)</code> shares free space.</p>
%(layout)s
<h3 id="scrolling">Scrolling</h3>
<p><code>overflow: scroll</code> clips the content and scrolls it with the mouse wheel, by dragging the content (touch style, with a little momentum), with gamepad focus (the focused child is scrolled into view), and by the scrollbar. Add <code>scrollbar="true"</code> to get a track and thumb that you style with <code>.scrollbar</code> and <code>.scrollbar .thumb</code>. Drag the thumb, or press the track to jump there. Nothing is drawn when the content fits.</p>
%(scroll)s
<h3>Lists</h3>%(listview)s
</section>

<section id="painting"><h2>8. Painting <span class="badge core">Core</span></h2>%(paint)s
<h3>Effects, blending, transforms</h3>%(effects)s
<h3>Every paint feature and its status</h3>%(painttable)s
<h3 id="svg">SVG icons</h3>
<p>An <code>.svg</code> file works wherever an image does: <code>&lt;Image src&gt;</code>, <code>background-image: url()</code> and <code>border-image</code>. The engine rasterises it for the size it is drawn at (the longer side rounded up to 16, 32, 64, 128, 256 or 512 device pixels; larger shows the 512 raster scaled) and caches each size, so an icon is as crisp at 16 px as at 128 px and costs one raster per size, once. Layout uses the drawing's own <code>width</code> and <code>height</code> (96 dpi px); the raster size never changes layout.</p>
<p>Supported: paths, basic shapes, solid and gradient fills, strokes, transforms and groups. Not supported: text, filters, masks, CSS styling inside the file, animation and external images; convert text to outlines before exporting. A <code>fill</code> in the file is the colour you get; <code>wash-color</code> tints the result like any image.</p>
%(svg)s
<h3 id="layers">Layers: blur, drop-shadow, group opacity, backdrop</h3>
<p>Some effects need the element drawn as one picture first. The engine renders such an element and its children into an offscreen layer (a rectangle of a 4096 px atlas page), applies the effect, and draws the result in place. You never create a layer yourself; these properties do:</p>
<ul>
<li><code>filter: blur()</code> and <code>filter: drop-shadow()</code> (combine them with the colour functions; the colour functions run first, inside the layer).</li>
<li><code>isolation: isolate</code>, and <code>opacity</code> below 1 together with it: the group fades as one picture.</li>
<li>A page that is entering or leaving a menu stack with <code>opacity</code> below 1 is isolated automatically, so a fading page never shows its overlapping children through each other.</li>
<li><code>cache: layer</code>: the layer is rendered once and reused until its recorded content changes, and freed after 3 seconds without use. Use it for static, expensive panels (many shadows, gradients, text). It does not notice an image whose pixels change.</li>
<li><code>backdrop-filter: blur(R)</code> plus colour functions: the element's box shows the blurred scene behind it, clipped to its rounded shape. The blur picks the nearest of three levels (sigma 4, 8 and 16 UI units). Only the scene is blurred, never the UI below.</li>
</ul>
%(layers)s
<p>Limits: <code>clip-path</code> stencil clips inside a layer are ignored, legacy (external) elements inside a layer are skipped, and a layer needs bgfx views (96 per frame) and atlas room; when it cannot be built the element is drawn without the effect and a warning is logged once. <code>backdrop-filter</code> needs the scene colour, which only the main UI pass has (loading screens and world-space UI draw nothing for it). Each 4096 px page is about 64 MB and is created the first time it is needed.</p>
</section>

<section id="motion"><h2>9. Motion <span class="badge core">Core</span></h2>%(motion)s
<ul>
<li>Time is real time: UI motion also runs while the game is paused.</li>
<li>A page pushed on a stack starts in <code>:entering</code> and animates to normal. A popped page animates to <code>:leaving</code> and is removed when the transition ends (2 seconds at most).</li>
<li>With the setting <code>ui.reduced_motion</code> (or <code>ui_reduced_motion</code>) transitions are instant and infinite animations do not run.</li>
<li>Easings: %(easings)s.</li>
</ul>
<h4>Properties that can be transitioned or animated</h4>%(animprops)s
</section>

<section id="events"><h2>10. Events, actions and code-behind <span class="badge core">Core</span></h2>
<p>Event attributes hold calls separated by <code>;</code>. A call is a name with literal arguments. There is no expression language or scripting.</p>
%(events)s
%(events_t)s
<p>Every element raises the generic events: <code>focus</code> / <code>blur</code> (the element itself gained or lost the focus; <code>focusin</code> / <code>focusout</code> also reach every ancestor), <code>clickstart</code> (press), <code>clickend</code> (the press ended, released or the pointer left), <code>click</code>, <code>doubleclick</code> (two clicks within 350 ms and 8 px), <code>mouseover</code> / <code>mouseout</code>, <code>contextmenu</code>, <code>wheel</code>, <code>scroll</code> and <code>enabled</code> / <code>disabled</code>. A press on a disabled element raises <code>denied</code> and nothing else. Controls add their own: <code>valuechanged</code>, <code>checked</code> / <code>unchecked</code>, <code>open</code> / <code>close</code> (drop-down), <code>slidestart</code> / <code>slideend</code>, the drag events, and for text fields <code>input</code>, <code>change</code>, <code>submit</code>, <code>editstart</code>, <code>editend</code>, <code>editcancel</code>, <code>inputrejected</code>. Pages raise <code>pushed</code>, <code>hidden</code>, <code>shown</code>, <code>popped</code>. C++ can subscribe with <code>element->Listen("click", fn)</code>.</p>
<h3 id="sounds">Sounds and pulses in styles</h3>
<p>Sounds belong to the style and follow its states, the way an animation starts when a rule begins to match: <code>sound-enter: url("GameData/sounds/ui/hover.wav")</code> plays when the rule starts to match the element (a pseudo-class or class turns on), <code>sound-exit</code> when it stops matching, and <code>sound-volume</code> of the same rule scales both. They are written per rule, not inherited or cascaded, so <code>Button:hover { sound-enter: ... }</code> and <code>Button:click { sound-enter: ... }</code> each play their own and every button that matches shares them. Building or reloading a page is silent (the first resolve only records the rules). A disabled element plays only the sounds of rules that mention <code>:disabled</code> or <code>:denied</code>. The same file is not restarted within 40 ms.</p>
<p>The states that exist anyway (<code>:hover</code>, <code>:active</code>, <code>:focus</code>, <code>:checked</code>, <code>:selected</code>, <code>:disabled</code>, <code>:entering</code>, <code>:leaving</code>, <code>:dragging</code>) cover most sounds. For things that happen once, an event puts the element in a <b>pulse</b> pseudo-class for the <code>pulse-duration</code> of that pulse (default 0.2 s): <code>:click</code> (a click or confirm), <code>:doubleclick</code>, <code>:contextmenu</code>, <code>:denied</code> (a press on the disabled element), <code>:change</code> (the user changed the value), <code>:input</code> (a character was typed), <code>:submit</code>, <code>:rejected</code> (typing was refused) and <code>:drop</code>. A rule on a pulse can animate and make a sound, and says how long the pulse lasts: <code>Button:doubleclick { pulse-duration: 0.3s; animation: flash 0.3s; sound-enter: "..."; }</code>. Every pulse has its own duration; a rule without a pulse pseudo-class (<code>Button { pulse-duration: 0.1s; }</code>) sets it for all of them, a more specific rule wins. A pulse that comes again while it is on only lasts longer, so it does not replay the sound.</p>
<h3 id="textentry">Text fields</h3>
<p><code>&lt;TextEntry&gt;</code> and <code>&lt;MultilineTextEntry&gt;</code> are native elements. A click, or confirm on a focused field, starts editing; while editing the field owns the keyboard. Enter commits (in a multi-line field Enter inserts a line break and Ctrl+Enter commits), Escape reverts to the text from before the edit, and leaving the field commits. Supported: caret and selection by keyboard and pointer (double click selects a word, triple click the line), Ctrl+arrows and Ctrl+Backspace/Delete by word, Home/End, PageUp/PageDown, clipboard (Ctrl+C/X/V), undo and redo, <code>max-length</code>, <code>numeric</code>, <code>integer</code>, <code>allowed-chars</code>, <code>password</code>, <code>read-only</code>, <code>select-on-focus</code>, <code>placeholder</code>. A field with <code>width: auto</code> grows with its text; a multi-line field grows between <code>min-rows</code> and <code>max-rows</code>, then scrolls. Style it with the box properties, <code>--caret-color</code>, <code>--selection-color</code>, <code>--placeholder-color</code>, the <code>:focus</code> state and the class <code>.editing</code>. The variable <code>value</code> holds the text.</p>
<p>A name resolves in this order: a handler registered with <code>On(name)</code> on the element or the nearest ancestor that has one, then a built-in action, otherwise an "unhandled action" warning (once). Unknown names are reported when the page loads, not only on click.</p>
<h3>Built-in actions</h3>%(actions_t)s
<h3 id="dragdrop">Drag and drop</h3>
<p>Mark an element <code>draggable="true"</code> and the places it can land <code>droptarget="true"</code>. The pointer starts a drag after moving 6 px (a press that moves less is still a click); a translucent ghost of the element follows the pointer, and the source has the <code>:dragging</code> state. With a gamepad, confirm on a draggable picks it up (the ghost follows focus), confirm on a drop target drops it, and cancel aborts. A drag over a scroll container does not scroll it.</p>
<p>Events: <code>ondragstart</code> on the source; <code>ondragenter</code>, <code>ondragleave</code> and <code>ondrop</code> on the target (<code>dragleave</code> also fires just before <code>drop</code>, so a style that marks the hovered target is cleared before the drop runs).</p>
%(dragdrop)s
%(dragcode)s
<h3 id="contextmenu">Context menus</h3>
<p><code>oncontextmenu</code> fires on the element under the pointer (or its nearest ancestor that has the event) for a right mouse click, a 0.5 s touch hold, or the <code>ui_context</code> action (gamepad: the focused element). A game binds <code>ui_context</code> to the right mouse button and a gamepad button. The built-in action <code>ContextMenu('page.xml')</code> pushes a page on the stack and moves its element with class <code>context-menu</code> to where the request happened. Make the page <code>shows-below</code> so the screen stays visible.</p>
%(context)s
<p>Selectors in actions are <code>#id</code> or an id path (<code>#row/label</code>). <code>Push</code>, <code>Back</code>, <code>Replace</code>, <code>Pop</code>, <code>Clear</code> act on <em>the stack the element is on</em>; <code>PushMenu</code> always uses the main stack.</p>
<h3>Code-behind</h3>%(codebehind)s
%(tagreg)s
</section>

<section id="components"><h2>11. Components and snippets <span class="badge core">Core</span></h2>
<p>Any XML file whose top element is <code>&lt;Component&gt;</code> defines a new tag. It works like a built-in tag, and components can use components.</p>
%(component_def)s
%(component_use)s
<ul>
<li><b>Parameters</b> (<code>params</code>) become variables of the instance. A missing required parameter or an undeclared attribute is a <code>file:line</code> error.</li>
<li><b>Standard attributes</b> on the instance (<code>id</code>, <code>class</code>, <code>style</code>, <code>visible</code>, events) apply to the component's root; classes merge with its own.</li>
<li><b>Ids are scoped per instance</b>, so two instances can both contain a <code>label</code>. Slotted children belong to the using file. Reach inside an instance with a path: <code>Find("fov_row/label")</code>.</li>
<li><b>Discovery.</b> Components under the engine kit and under the game's <code>GameData/ui/</code> are found at start, no import needed. Elsewhere, add <code>&lt;import&gt;</code>. Two components with one tag, or a component that includes itself, is an error naming both files.</li>
<li><b>Snippets</b> (<code>&lt;snippet&gt;</code>) are file-local components, mainly for rows a page builds from code or a <code>ListView</code>.</li>
<li><b>Frame</b> embeds a whole layout with no parameters; its <code>src</code> can change at run time.</li>
</ul>%(frame)s
</section>

<section id="menus"><h2>12. Menu stack, focus and input <span class="badge core">Core</span></h2>
<p>There is no special page type: <b>any element can be pushed</b>. The stack gives it input, focus, back handling, animation and the host options.</p>
%(stack)s
%(stackcpp)s
<ul>
<li><b>Input and focus</b>: only the top page is interactive. Pushing remembers the focused element and popping restores it. <code>default-focus</code> chooses the first focus (for gamepad).</li>
<li><b>Back</b> (<code>ui_cancel</code>, Esc, gamepad B) calls <code>OnBack()</code>, then pops if <code>close-on-back</code> is true.</li>
<li><b>Pages below</b> are hidden unless the page has <code>shows-below="true"</code> (dialogs).</li>
<li><b>Main stack only</b>: <code>pauses-game</code> pauses the game through one hook (<code>UiMenus::OnPauseChanged</code>); <code>hides-hud</code> fades the HUD.</li>
<li>Lifecycle hooks on any <code>UiElem</code>: <code>OnPushed</code>, <code>OnShown</code>, <code>OnHidden</code>, <code>OnPopped</code>, <code>OnBack</code>, <code>Tick</code>.</li>
</ul>
<h3>Navigation, hotkeys, prompts</h3>%(nav)s
<p>Input actions the engine knows: %(inputactions)s</p>
</section>

<section id="binding"><h2>13. Settings binding <span class="badge core">Core</span></h2>
<p>Variables are one-way. Controls bind <em>both ways</em> to a settings registry with <code>bind="setting:KEY"</code>. A page with <code>confirm-changes="true"</code> asks to apply or discard the changes made through bound controls when it is left. A new setting is one registry entry plus one line of markup.</p>
%(bind)s
%(bindcpp)s
<p class="dim">The settings the kit registers are listed in <a href="#kit-settings">the kit part</a>.</p>
</section>

<section id="hot"><h2>14. Hot reload <span class="badge core">Core</span></h2>
<p>In builds without <code>DISTRIBUTION</code>, saving a UI file updates the running game. The game runs from a copy of <code>GameData</code> made by the build, so the engine also watches the <em>source</em> files (<code>source/Games/&lt;Game&gt;/GameData</code>), copies changed ones over the running copy and rebuilds every page that depends on them: its XML, imported components, style sheets and their <code>@import</code>s. Structure changes and <code>on…</code> logic count as well as styles.</p>
%(hot)s
<ul>
<li><b>Kept</b>: slider, drop-down and text values, toggles and radios, scroll, focus, classes and variables set from code, attributes set from code.</li>
<li><b>Matching after an edit</b>: elements are matched by <code>id</code>, or by position among siblings of the same tag. If you edit an attribute in the file, the file's value wins.</li>
<li><b>Not kept</b>: private fields of code-behind classes and anything built in <code>OnPushed</code> (it is not run again).</li>
<li>A broken file keeps the old version on screen and logs the error. A component file that is new since the game started needs a restart. Everything is compiled out of <code>DISTRIBUTION</code> builds.</li>
</ul></section>

<section id="tooling"><h2>15. Tools <span class="badge core">Core</span></h2>
<ul>
<li><b>UI Studio</b> (<code>source/UiStudio</code>): a Qt editor that runs the real game inside it. Page tree, properties, visual edits that write back to the XML, preview with the game's own renderer, test runner.</li>
<li><b>VS Code extension</b> (<code>Tools/ui-vscode</code>): colouring, completion, hover, diagnostics, go-to-definition for the XML and the CSS dialect. It reads the game's <code>SaveData/UIStudio/uidb.json</code>, the catalogue this page is made from.</li>
<li><b>Console</b>: %(console)s</li>
<li><b>Picking</b>: <code>ui_pick</code> in a running game writes the element under the pointer to <code>SaveData/ui_pick.json</code> (<code>path</code>, <code>file</code>, <code>line</code>, <code>col</code>, <code>id</code>, <code>ref</code>); UI Studio watches the file and selects the node. <code>ui_pick_at x y</code> does the same for a point, and is what the Studio runtime bridge uses.</li>
<li><b>Test scripts</b> (<code>.uitest</code>, run with <code>-ui_test</code> or <code>Tools/ui/run_ui_tests.py</code> on each graphics backend):</li></ul>
%(uitest)s
<p>Commands: %(testcmds)s</p>
<p><code>-ui_lint</code> reports problems without running the game. Always add <code>-headless</code> for automated runs.</p>
</section>

<h1 class="part" id="kit">Part II: The kit <span class="badge kit">Kit</span></h1>
<p class="lead">The kit is a set of ready-made pages and components in <code>GameData/ui/kit/</code>, built only from the core above: pause menu, settings, controls and rebinding, save and load, dialogs. A game gets a complete menu by supplying a <b>theme</b> and a few hooks. Nothing in the core depends on it.</p>

<section id="kit-use"><h2>K1. Using the kit <span class="badge kit">Kit</span></h2>%(gamewiring)s
<h3>Theme</h3>
<p><code>kit.css</code> only lays things out and reads variables; the game's <code>GameData/ui/theme.css</code> sets them. Change the tokens and every kit page changes.</p>%(theme)s
<p>Tokens: %(tokens)s</p>
<p>The kit's layout classes (<code>kit-screen</code>, <code>kit-column</code>, <code>kit-card</code>, <code>kit-dialog</code>, <code>kit-button</code>, <code>kit-toggle</code>, <code>setting-row</code>, <code>menu-item</code>…) are in <code>kit/styles/kit.css</code>. Your own pages can use them or ignore them.</p>
</section>

<section id="kit-components"><h2>K2. Kit components <span class="badge kit">Kit</span></h2>%(kitcomps)s%(kitcard)s</section>

<section id="kit-pages"><h2>K3. Kit pages <span class="badge kit">Kit</span></h2>%(kitpages)s
<p>The code-behind classes (<code>KitPauseMenu</code>, <code>KitSettingsMenu</code>, <code>KitInputPage</code>, <code>KitSaveLoad</code>) are registered by <code>KitInit()</code>. Hooks a game may set: <code>KitPauseMenu::ShowSaveLoad</code>, <code>CanSave</code>, <code>OnRestart</code>; <code>KitSettingsMenu::RegisterTab</code> adds a tab; <code>KitConfirm::Ask</code>, <code>KitSaveLoad::Open</code> and <code>KitRebind::Open</code> open the modal pages.</p>
<h3>Building your own page from kit pieces</h3>%(kitpage)s
</section>

<section id="kit-settings"><h2>K4. Kit settings <span class="badge kit">Kit</span></h2>
<p>Registered by the kit (<code>bind="setting:KEY"</code>); games add their own with <code>UiSettings::Register</code>.</p>%(kitsettings)s</section>

<h1 class="part" id="appendix">Appendix</h1>
<section id="tags-ref"><h2>A. CSS property reference <span class="badge core">Core</span></h2>%(css)s</section>
<footer>Generated by <code>Tools/ui/make_ui_reference.py</code> from <code>uidb.json</code>. Regenerate it after the engine's UI registries change.</footer>
"""

RICH_TABLE = table(["Tag", "Effect", "Example"], [
    ["<code>&lt;b&gt;…&lt;/b&gt;</code>", "bold", "<code>&amp;lt;b&amp;gt;Warning&amp;lt;/b&amp;gt;</code>"],
    ["<code>&lt;i&gt;…&lt;/i&gt;</code>", "italic", "<code>&amp;lt;i&amp;gt;whisper&amp;lt;/i&amp;gt;</code>"],
    ["<code>&lt;color=#rrggbb&gt;…&lt;/color&gt;</code>", "text colour (<code>#rrggbbaa</code> adds alpha)", "<code>&amp;lt;color=#ff4040&amp;gt;red&amp;lt;/color&amp;gt;</code>"],
    ["<code>&lt;size=NN&gt;…&lt;/size&gt;</code>", "font size in UI pixels for this run", "<code>&amp;lt;size=40&amp;gt;BIG&amp;lt;/size&amp;gt;</code>"],
    ["<code>&lt;icon=action:NAME&gt;</code>", "the key / button image of an input action, for the last-used device", "<code>&amp;lt;icon=action:interact&amp;gt;</code>"],
    ["<code>&lt;icon=img:PATH&gt;</code>", "an image from GameData, sized to the text", "<code>&amp;lt;icon=img:GameData/textures/ui/coin.png&amp;gt;</code>"],
    ["<code>&lt;icon=…,valign=middle&gt;</code>", "vertical placement of either icon form: <code>baseline</code> (default, bottom on the baseline), <code>middle</code> (centred on the capital letters), <code>top</code>, <code>bottom</code>", "<code>&amp;lt;icon=action:jump,valign=middle&amp;gt;</code>"],
])

CSS = """
:root{--bg:#fbfaf8;--fg:#1d1b19;--dim:#6f6a64;--line:#e3dfd8;--panel:#f3f0ea;--code:#1e1d1b;--codefg:#ece7de;--core:#2f6f8f;--kit:#a8501c;--ok:#2e7d4f;--warn:#a2631a;--link:#245f9a}
@media (prefers-color-scheme:dark){:root{--bg:#171614;--fg:#ece8e1;--dim:#9b958c;--line:#33302b;--panel:#201e1b;--code:#0f0e0d;--codefg:#e6e0d6;--core:#6bb0d1;--kit:#e49a64;--ok:#6cc08c;--warn:#e0aa5c;--link:#7fb3ea}}
*{box-sizing:border-box}html{scroll-behavior:smooth}
body{margin:0;background:var(--bg);color:var(--fg);font:16px/1.6 system-ui,Segoe UI,Roboto,sans-serif}
.wrap{display:grid;grid-template-columns:260px minmax(0,1fr);max-width:1280px;margin:0 auto}
nav{position:sticky;top:0;align-self:start;height:100vh;overflow:auto;padding:24px 16px;border-right:1px solid var(--line);font-size:14px}
nav b{display:block;margin:14px 0 4px;color:var(--dim);font-size:12px;letter-spacing:.06em;text-transform:uppercase}
nav a{display:block;color:var(--fg);text-decoration:none;padding:3px 8px;border-radius:6px}nav a:hover{background:var(--panel)}
main{padding:32px 40px 80px;min-width:0}
h1{font-size:2.1rem;margin:0 0 8px}h1.part{margin-top:64px;padding-top:24px;border-top:3px solid var(--line)}
h2{font-size:1.5rem;margin:48px 0 12px;padding-top:12px;border-top:1px solid var(--line)}h3{font-size:1.2rem;margin:28px 0 8px}h4{font-size:1rem;margin:20px 0 6px}
p,li{max-width:78ch}a{color:var(--link)}.dim{color:var(--dim)}.lead{font-size:1.1rem}
code{font:13px/1.4 ui-monospace,Consolas,Menlo,monospace;background:var(--panel);border:1px solid var(--line);border-radius:5px;padding:1px 5px}
pre{background:var(--code);color:var(--codefg);border-radius:10px;padding:14px 16px;overflow:auto;margin:12px 0}
pre code{background:none;border:0;padding:0;color:inherit;font-size:13px;line-height:1.5}
.tw{overflow:auto;margin:10px 0}table{border-collapse:collapse;width:100%;font-size:14px}
th,td{border-bottom:1px solid var(--line);padding:6px 10px;text-align:left;vertical-align:top}th{background:var(--panel);font-weight:600;position:sticky;top:0}
table.plain{width:auto}
.badge{display:inline-block;font-size:.65rem;font-weight:700;letter-spacing:.06em;text-transform:uppercase;border-radius:20px;padding:2px 9px;vertical-align:middle;margin-left:8px;color:#fff}
.badge.core{background:var(--core)}.badge.kit{background:var(--kit)}
.legend{background:var(--panel);border:1px solid var(--line);border-radius:10px;padding:10px 14px}
.chip{font-size:.7rem;font-weight:600;border:1px solid var(--line);border-radius:12px;padding:1px 8px;color:var(--dim)}.chip.ok{color:var(--ok);border-color:var(--ok)}.chip.warn{color:var(--warn);border-color:var(--warn)}
.callout{border-left:4px solid var(--warn);background:var(--panel);padding:10px 14px;border-radius:0 8px 8px 0;margin:14px 0;max-width:78ch}
.tag{border:1px solid var(--line);border-radius:12px;padding:6px 18px 14px;margin:16px 0}.tag h4{margin:12px 0 4px;font-size:1.1rem}
footer{margin-top:60px;color:var(--dim);font-size:13px}
@media(max-width:860px){.wrap{display:block}nav{position:static;height:auto;border:0;border-bottom:1px solid var(--line)}main{padding:20px 16px}}
"""

NAV = [("Core", [("overview", "Overview"), ("structure", "File structure"), ("elements", "Elements"), ("richtext", "Text and rich text"), ("vars", "Variables, localisation"), ("styles", "Styles"), ("layout", "Layout, scrolling"),
                 ("painting", "Painting"), ("svg", "SVG icons"), ("layers", "Layers, effects"), ("motion", "Motion"), ("events", "Events, code-behind"), ("components", "Components"), ("menus", "Menu stack, input"), ("binding", "Settings binding"), ("hot", "Hot reload"), ("tooling", "Tools")]),
       ("Kit", [("kit-use", "Using the kit"), ("kit-components", "Components"), ("kit-pages", "Pages"), ("kit-settings", "Settings")]),
       ("Appendix", [("tags-ref", "CSS properties")])]


def build():
    ctx = {k: pre(k) for k in EX}
    ctx.update(dict(
        build=esc(db["engine"].get("buildId", "")), game=esc(db["game"].get("displayName", "")),
        common=common_attrs_table(), tags=tags_reference(), richtable=RICH_TABLE, pseudo=pseudo_list(),
        painttable=paint_reference(), animprops=anim_reference(), easings=", ".join("<code>%s</code>" % e["name"] for e in reg["easings"]),
        events_t=events_t, actions_t=actions_t, inputactions=input_actions(), testcmds=test_cmds(), console=pre("console"),
        kitcomps=kit_components(), kitpages=kit_pages(), tokens=kit_tokens(), kitsettings=kit_settings(), css=css_reference()))
    ctx["console"] = pre("console")
    body = BODY % ctx
    nav = "".join("<b>%s</b>%s" % (g, "".join('<a href="#%s">%s</a>' % (i, t) for i, t in items)) for g, items in NAV)
    doc = ('<!doctype html><html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">'
           "<title>UI System Reference</title><style>%s</style></head><body><div class=\"wrap\"><nav>%s</nav><main>%s</main></div></body></html>") % (CSS, nav, body)
    OUT.write_text(doc, encoding="utf-8", newline="\n")
    print("wrote", OUT, len(doc) // 1024, "KB")


build()
