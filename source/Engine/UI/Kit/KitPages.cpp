#include "KitPages.h"

#include <FileSystem/FileSystem.h>
#include <Input.h>
#include <Logger.hpp>
#include <SaveSystem/GameSaveSystem.h>
#include <UI/Elements/UiElemButton.h>
#include <UI/Elements/UiElemFrame.h>
#include <UI/Elements/UiElemLabel.h>
#include <UI/Kit/Settings/InputIconLibrary.h>
#include <UI/Kit/Settings/KitSettings.h>
#include <UI/Markup/UiBuilder.h>
#include <UI/Markup/UiTagRegistry.h>
#include <UI/Menus/UiActions.h>
#include <UI/Menus/UiInputPrompts.h>
#include <UI/Menus/UiMenuStack.h>
#include <UI/UiNavigation.h>

#include <SDL2/SDL.h>

#include <algorithm>
#include <ctime>
#include <format>
#include <iomanip>
#include <sstream>
#include <unordered_set>

namespace
{
    const char* kPause = "GameData/ui/kit/pages/pause.xml";
    const char* kSettings = "GameData/ui/kit/pages/settings.xml";
    const char* kInput = "GameData/ui/kit/pages/input.xml";
    const char* kConfirm = "GameData/ui/kit/pages/confirm.xml";
    const char* kRebind = "GameData/ui/kit/pages/rebind.xml";
    const char* kSaveLoad = "GameData/ui/kit/pages/saveload.xml";

    void SetText(UiElem* e, const std::string& text)
    {
        if (e != nullptr)
            e->SetAttribute("text", text);
    }

    void SetVisible(UiElem* e, bool visible)
    {
        if (e != nullptr)
            e->SetAttribute("visible", visible ? "true" : "false");
    }

    std::vector<std::pair<std::string, std::string>>& GameTabs()
    {
        static std::vector<std::pair<std::string, std::string>> tabs;
        return tabs;
    }

    void RequestQuit()
    {
        SDL_Event quit;
        SDL_zero(quit);
        quit.type = SDL_QUIT;
        SDL_PushEvent(&quit);
    }
}

// ── confirm ───────────────────────────────────────────────────────────────

namespace
{
    class KitConfirmPage : public UiElemPanel
    {
    public:
        KitConfirmPage(const std::string& message = "", std::function<void()> yes = nullptr, std::function<void()> no = nullptr)
            : m_yes(std::move(yes)), m_no(std::move(no))
        {
            LoadLayout(kConfirm);
            SetText(FindElem("msg"), message);
            On("Yes", [this]() { Close(m_yes); });
            On("No", [this]() { Close(m_no); });
        }
        bool OnBack() override { Close(m_no); return true; }

    private:
        void Close(std::function<void()> then)
        {
            if (m_closed)
                return;
            m_closed = true;
            if (UiMenuStack* stack = Stack())
                stack->Pop();
            if (then)
                then();
        }
        std::function<void()> m_yes, m_no;
        bool m_closed = false;
    };
}

void KitConfirm::Ask(const std::string& message, std::function<void()> yes, std::function<void()> no)
{
    UiMenus::Stack().Push(std::make_shared<KitConfirmPage>(message, std::move(yes), std::move(no)));
}

// ── pause ─────────────────────────────────────────────────────────────────

KitPauseMenu::KitPauseMenu()
{
    LoadLayout(kPause);
    On("OpenSettings", [this]() { if (UiMenuStack* s = Stack()) s->Push(std::make_shared<KitSettingsMenu>()); });
    On("OpenSave", []() { KitSaveLoad::Open(KitSaveLoad::Mode::Save); });
    On("OpenLoad", []() { KitSaveLoad::Open(KitSaveLoad::Mode::Load); });
    On("AskQuit", []() { KitConfirm::Ask("${PAUSE_QUIT_CONFIRM}", []() { RequestQuit(); }); });
    On("Restart", []()
        {
            if (OnRestart)
                KitConfirm::Ask("${PAUSE_RESTART_CONFIRM}", []() { UiMenus::Clear(); OnRestart(); });
        });
    if (!ShowSaveLoad || (CanSave && !CanSave()))
        SetVisible(FindElem("psave"), false);
    if (!ShowSaveLoad)
        SetVisible(FindElem("pload"), false);
    SetVisible(FindElem("prestart"), OnRestart != nullptr);
}

// ── settings ──────────────────────────────────────────────────────────────

void KitSettingsMenu::RegisterTab(const std::string& labelOrKey, const std::string& page)
{
    for (auto& tab : GameTabs())
        if (tab.second == page)
        {
            tab.first = labelOrKey;
            return;
        }
    GameTabs().emplace_back(labelOrKey, page);
}

KitSettingsMenu::KitSettingsMenu()
{
    LoadLayout(kSettings);
    m_tabs.push_back({ FindElem("tabinput"), "GameData/ui/kit/pages/input.xml" });
    m_tabs.push_back({ FindElem("tabvideo"), "GameData/ui/kit/pages/video.xml" });
    m_tabs.push_back({ FindElem("tabsound"), "GameData/ui/kit/pages/sound.xml" });

    // game tabs come first, as the gameplay button did on the old settings hub
    UiElem* bar = FindElem("tabs");
    if (bar != nullptr)
    {
        size_t position = 0;
        for (const auto& [label, page] : GameTabs())
        {
            auto button = std::make_shared<UiElemTextButton>();
            button->Classes = { "kit-tab" };
            button->ScopeFile = ScopeFile;
            button->Instance = bar->Instance;
            bar->AddChild(button);
            button->SetAttribute("text", label);
            button->SetAttribute("onactivate", "SelectTab('" + page + "')");
            // AddChild appends; keep the game tabs in front
            auto& kids = bar->children;
            std::rotate(kids.begin() + static_cast<long>(position), kids.end() - 1, kids.end());
            ++position;
            m_tabs.push_back({ button.get(), UiActions::NormalizeLayoutPath(page) });
        }
    }
    On("SelectTab", [this](UiElem&, const std::vector<std::string>& args)
        {
            if (!args.empty())
                Select(UiActions::NormalizeLayoutPath(args[0]));
        });
}

void KitSettingsMenu::Select(const std::string& page)
{
    if (page == m_current)
        return;
    m_current = page;
    if (UiElem* frame = FindElem("content"))
        frame->SetAttribute("src", page);
    for (const Tab& tab : m_tabs)
        if (tab.Button != nullptr)
            tab.Button->SetAttribute("selected", tab.Page == page ? "true" : "false");
}

void KitSettingsMenu::OnPushed()
{
    // the video page shows the live state of the window, not what was last saved
    KitSettings::Instance().Video.FromCurrentState();
    Select("GameData/ui/kit/pages/video.xml");
}

void KitSettingsMenu::OnPopped()
{
    KitSettings& settings = KitSettings::Instance();
    settings.Video.FromCurrentState();
    settings.Audio.FromCurrentState();
    settings.ApplyAll();
    settings.SaveToFile();
}

void KitSettingsMenu::Tick()
{
    UiMenuStack* stack = Stack();
    if (stack == nullptr || stack->Top() != this)
        return;
    int step = 0;
    if (Input::GetAction("ui_tab_next")->Pressed()) step = 1;
    else if (Input::GetAction("ui_tab_prev")->Pressed()) step = -1;
    if (step == 0 || m_tabs.empty())
        return;
    size_t current = 0;
    for (size_t i = 0; i < m_tabs.size(); ++i)
        if (m_tabs[i].Page == m_current)
            current = i;
    const size_t next = (current + m_tabs.size() + step) % m_tabs.size();
    Select(m_tabs[next].Page);
}

// ── rebind modal ──────────────────────────────────────────────────────────

namespace
{
    class KitRebindModal : public UiElemPanel
    {
    public:
        explicit KitRebindModal(KitRebind::Request request) : m_r(std::move(request))
        {
            LoadLayout(kRebind);
            SetText(FindElem("title"), "${SETTINGS_INPUT_REBIND_TITLE_PREFIX} " + m_r.Action + " ${SETTINGS_INPUT_REBIND_SLOT_LABEL} " + std::to_string(m_r.Slot + 1));
            SetText(FindElem("hint"), m_r.Keyboard ? "${SETTINGS_INPUT_REBIND_HINT_KEYBOARD}" : "${SETTINGS_INPUT_REBIND_HINT_GAMEPAD}");
            On("Cancel", [this]() { Finish(0); });
            On("Unbind", [this]() { Finish(1); });
            m_baselineKeys = Input::activeKeys;
            m_baselineMouse = Input::activeMouseButtons;
            m_baselineJoy = Input::activeJoystickButtons;
            m_baselineLT = Input::leftTriggerAxis > kTrigger;
            m_baselineRT = Input::rightTriggerAxis > kTrigger;
        }

        bool OnBack() override { Finish(0); return true; }
        void OnPushed() override { UiNavigation::SetFocus(nullptr); }

        void Tick() override
        {
            UiMenuStack* stack = Stack();
            if (m_resolved || stack == nullptr || stack->Top() != this)
                return;
            if (m_asking)
                return;
            if (m_r.Keyboard)
            {
                if (HasGamepadInput()) { Finish(0); return; }
                PollKeyboard();
            }
            else
            {
                if (HasKeyboardMouseInput()) { Finish(0); return; }
                PollGamepad();
            }
        }

    private:
        static constexpr float kTrigger = 0.5f;

        // 0 cancel, 1 unbind, 2 captured
        void Finish(int how)
        {
            if (m_resolved)
                return;
            m_resolved = true;
            if (UiMenuStack* stack = Stack())
                stack->Pop();
            if (how == 0 && m_r.OnCancel) m_r.OnCancel();
            else if (how == 1 && m_r.OnUnbind) m_r.OnUnbind();
            else if (how == 2)
            {
                if (m_r.Keyboard && m_r.OnKB) m_r.OnKB(m_kb);
                else if (!m_r.Keyboard && m_r.OnGP) m_r.OnGP(m_gp);
            }
        }

        void Prompt()
        {
            if (m_asking)
                return;
            m_asking = true;
            SetText(FindElem("hint"), "${SETTINGS_INPUT_REBIND_ALREADY_BOUND_PROMPT}");
            SetVisible(FindElem("buttons"), true);
            SetVisible(FindElem("runbind"), m_r.Bound);
            if (UiElem* cancel = FindElem("rcancel"))
                UiNavigation::SetFocus(cancel);
        }

        bool HasGamepadInput() const
        {
            for (int b : Input::activeJoystickButtons)
                if (m_baselineJoy.count(b) == 0) return true;
            return (Input::leftTriggerAxis > kTrigger && !m_baselineLT) || (Input::rightTriggerAxis > kTrigger && !m_baselineRT);
        }

        bool HasKeyboardMouseInput() const
        {
            for (SDL_Scancode sc : Input::activeKeys)
                if (m_baselineKeys.count(sc) == 0) return true;
            for (uint8_t b : Input::activeMouseButtons)
                if (m_baselineMouse.count(b) == 0) return true;
            return false;
        }

        void CaptureKB(const InputBindingSlotKB& slot)
        {
            if (m_r.Bound && slot == m_r.KB) { Prompt(); return; }
            m_kb = slot;
            Finish(2);
        }

        void CaptureGP(const InputBindingSlotGP& slot)
        {
            if (m_r.Bound && slot == m_r.GP) { Prompt(); return; }
            m_gp = slot;
            Finish(2);
        }

        void PollKeyboard()
        {
            for (SDL_Scancode sc : Input::activeKeys)
                if (!m_baselineKeys.count(sc)) { CaptureKB(InputBindingSlotKB::FromKey(sc)); return; }
            for (uint8_t b : Input::activeMouseButtons)
                if (!m_baselineMouse.count(b)) { CaptureKB(InputBindingSlotKB::FromMouseButton(b)); return; }
        }

        void PollGamepad()
        {
            for (int b : Input::activeJoystickButtons)
                if (!m_baselineJoy.count(b)) { CaptureGP(InputBindingSlotGP::FromButton(static_cast<GamepadButton>(b))); return; }
            if (Input::leftTriggerAxis > kTrigger && !m_baselineLT) { CaptureGP(InputBindingSlotGP::FromButton(GamepadButton::LeftTrigger)); return; }
            if (Input::rightTriggerAxis > kTrigger && !m_baselineRT) { CaptureGP(InputBindingSlotGP::FromButton(GamepadButton::RightTrigger)); return; }
        }

        KitRebind::Request m_r;
        InputBindingSlotKB m_kb;
        InputBindingSlotGP m_gp;
        std::unordered_set<SDL_Scancode> m_baselineKeys;
        std::unordered_set<uint8_t> m_baselineMouse;
        std::unordered_set<int> m_baselineJoy;
        bool m_baselineLT = false, m_baselineRT = false;
        bool m_resolved = false, m_asking = false;
    };
}

void KitRebind::Open(Request request)
{
    UiMenus::Stack().Push(std::make_shared<KitRebindModal>(std::move(request)));
}

// ── input settings ────────────────────────────────────────────────────────

KitInputPage::KitInputPage()
{
    LoadLayout(kInput);
    On("ResetAll", [this]()
        {
            KitConfirm::Ask("${SETTINGS_INPUT_CONFIRM_RESET_ALL}", [this]()
                {
                    KitSettings& s = KitSettings::Instance();
                    s.Input.ResetToDefaults();
                    s.Input.ApplyToEngine();
                    s.SaveToFile();
                    RefreshAll();
                });
        });
    On("BindSlot", [this](UiElem& sender, const std::vector<std::string>&)
        {
            Capture(sender.Attributes["data-action"], sender.Attributes["data-device"] == "kb", std::atoi(sender.Attributes["data-slot"].c_str()));
        });
    On("ResetRow", [this](UiElem& sender, const std::vector<std::string>&)
        {
            const std::string action = sender.Attributes["data-action"];
            KitSettings& s = KitSettings::Instance();
            s.Input.ResetBindingToDefault(action);
            s.Input.ApplyToEngine();
            s.SaveToFile();
            RefreshRow(action);
        });
    BuildRows();
}

namespace
{
    std::string DisplayNameFor(const std::string& action)
    {
        InputActionInfo* info = InputActionRegistry::Get(action);
        return info ? info->displayName : action;
    }
}

void KitInputPage::BuildRows()
{
    UiElem* list = FindElem("binds");
    if (list == nullptr)
        return;
    bool first = true;
    for (const std::string& category : InputActionRegistry::GetOrderedCategories())
    {
        auto header = UiBuilder::BuildSnippet(*this, *list, "CategoryHeader");
        if (header)
            header->SetVar("label", category);
        first = false;
        bool alternate = false;
        for (const std::string& action : InputActionRegistry::GetActionsInCategory(category))
        {
            auto row = UiBuilder::BuildSnippet(*this, *list, "ActionRow");
            if (!row)
                continue;
            row->SetVar("label", DisplayNameFor(action));
            if (alternate)
                row->AddClass("alt");
            alternate = !alternate;
            for (const char* id : { "kb0", "kb1", "gp0", "gp1" })
                if (UiElem* slot = row->FindElem(id))
                {
                    slot->Attributes["data-action"] = action;
                    slot->Attributes["data-device"] = id[0] == 'k' ? "kb" : "gp";
                    slot->Attributes["data-slot"] = std::string(1, id[2]);
                    slot->SetAttribute("onactivate", "BindSlot()");
                }
            if (UiElem* reset = row->FindElem("reset"))
            {
                reset->Attributes["data-action"] = action;
                reset->SetAttribute("onactivate", "ResetRow()");
            }
            m_rows.push_back({ action, row.get() });
            RefreshRow(action);
        }
    }
    (void)first;
}

void KitInputPage::Listening(const std::string& action, bool keyboard, int slot, bool on)
{
    for (const Row& r : m_rows)
        if (r.Action == action)
            if (UiElem* e = r.Elem->FindElem(std::string(keyboard ? "kb" : "gp") + std::to_string(slot)))
                e->SetClass("listening", on);
}

void KitInputPage::RefreshRow(const std::string& action)
{
    for (const Row& r : m_rows)
    {
        if (r.Action != action)
            continue;
        const InputBindingOverride eff = KitSettings::Instance().Input.GetEffectiveBinding(action);
        for (int i = 0; i < 2; ++i)
        {
            struct Info { const char* Id; std::string Icon, Name; bool Bound; };
            const Info infos[2] = {
                { i == 0 ? "kb0" : "kb1", InputIconLibrary::GetKeyboardIcon(eff.kb[i]), InputBindingSerialization::KBDisplayName(eff.kb[i]), eff.kb[i].IsBound() },
                { i == 0 ? "gp0" : "gp1", InputIconLibrary::GetGamepadIcon(eff.gp[i]), InputBindingSerialization::GPDisplayName(eff.gp[i]), eff.gp[i].IsBound() },
            };
            for (const Info& info : infos)
            {
                UiElem* slot = r.Elem->FindElem(info.Id);
                if (slot == nullptr)
                    continue;
                if (UiElem* icon = slot->FindElem("icon"))
                    icon->SetAttribute("src", info.Icon);
                SetText(slot->FindElem("name"), info.Bound ? info.Name : "${SETTINGS_INPUT_UNBOUND}");
                slot->SetClass("unbound", !info.Bound);
                slot->SetClass("listening", false);
            }
        }
    }
}

void KitInputPage::RefreshAll()
{
    for (const Row& r : m_rows)
        RefreshRow(r.Action);
}

void KitInputPage::Capture(const std::string& action, bool keyboard, int slot)
{
    KitSettings& settings = KitSettings::Instance();
    Listening(action, keyboard, slot, true);
    const InputBindingOverride eff = settings.Input.GetEffectiveBinding(action);

    KitRebind::Request request;
    request.Keyboard = keyboard;
    request.Action = DisplayNameFor(action);
    request.Slot = slot;
    request.KB = keyboard ? eff.kb[slot] : InputBindingSlotKB();
    request.GP = keyboard ? InputBindingSlotGP() : eff.gp[slot];
    request.Bound = keyboard ? eff.kb[slot].IsBound() : eff.gp[slot].IsBound();
    request.OnCancel = [this, action, keyboard, slot]() { Listening(action, keyboard, slot, false); };
    request.OnUnbind = [this, action, keyboard, slot]()
    {
        KitSettings& s = KitSettings::Instance();
        if (keyboard) s.Input.ClearKeyboardSlot(action, slot); else s.Input.ClearGamepadSlot(action, slot);
        s.Input.ApplyToEngine();
        s.SaveToFile();
        RefreshRow(action);
    };
    auto commit = [this, action, slot](bool kb, InputBindingSlotKB valueKB, InputBindingSlotGP valueGP)
    {
        KitSettings& s = KitSettings::Instance();
        const std::string conflict = kb ? s.Input.SetKeyboardSlot(action, slot, valueKB, false) : s.Input.SetGamepadSlot(action, slot, valueGP, false);
        auto finish = [this]()
        {
            KitSettings& k = KitSettings::Instance();
            k.Input.ApplyToEngine();
            k.SaveToFile();
            RefreshAll();   // a reassignment can touch the conflicting action's row too
        };
        if (conflict.empty())
        {
            finish();
            return;
        }
        const std::string name = kb ? InputBindingSerialization::KBDisplayName(valueKB) : InputBindingSerialization::GPDisplayName(valueGP);
        const std::string message = "'" + name + "' ${SETTINGS_INPUT_REBIND_CONFLICT_MIDDLE} " + DisplayNameFor(conflict) + "${SETTINGS_INPUT_REBIND_CONFLICT_SUFFIX}";
        KitConfirm::Ask(message,
            [this, kb, action, slot, valueKB, valueGP, finish]()
            {
                KitSettings& k = KitSettings::Instance();
                if (kb) k.Input.SetKeyboardSlot(action, slot, valueKB, true); else k.Input.SetGamepadSlot(action, slot, valueGP, true);
                finish();
            },
            [this, action]() { RefreshRow(action); });
    };
    request.OnKB = [this, action, keyboard, slot, commit](InputBindingSlotKB value) { Listening(action, keyboard, slot, false); commit(true, value, InputBindingSlotGP()); };
    request.OnGP = [this, action, keyboard, slot, commit](InputBindingSlotGP value) { Listening(action, keyboard, slot, false); commit(false, InputBindingSlotKB(), value); };
    KitRebind::Open(std::move(request));
}

// ── save and load ─────────────────────────────────────────────────────────

namespace
{
    KitSaveLoad::Mode g_pendingMode = KitSaveLoad::Mode::Load;
    constexpr int kManualSlots = 10;

    std::string ManualId(int index)
    {
        std::ostringstream ss;
        ss << "manual_" << std::setw(2) << std::setfill('0') << (index + 1);
        return ss.str();
    }

    std::string FormatTime(uint64_t modificationTime)
    {
        std::time_t t = static_cast<std::time_t>(modificationTime);
        std::tm info{};
#if defined(_WIN32)
        localtime_s(&info, &t);
#else
        localtime_r(&t, &info);
#endif
        return std::format("{:04}-{:02}-{:02} {:02}:{:02}:{:02}", info.tm_year + 1900, info.tm_mon + 1, info.tm_mday, info.tm_hour, info.tm_min, info.tm_sec);
    }
}

void KitSaveLoad::Open(Mode mode)
{
    UiMenus::Stack().Push(std::make_shared<KitSaveLoad>(mode));
}

KitSaveLoad::KitSaveLoad(Mode mode) : m_mode(mode)
{
    LoadLayout(kSaveLoad);
    SetText(FindElem("title"), mode == Mode::Save ? "${PAUSE_SAVE_TITLE}" : "${PAUSE_LOAD_TITLE}");
    On("SlotActivate", [this](UiElem& sender, const std::vector<std::string>&)
        {
            // the row is the snippet's top element, which carries the variable "index"
            UiElem* row = &sender;
            while (row != nullptr && row->m_numVars.find("index") == row->m_numVars.end())
                row = row->AsParentElem();
            if (row == nullptr)
                return;
            const size_t index = static_cast<size_t>(row->m_numVars["index"]);
            if (index >= m_slots.size())
                return;
            const Slot slot = m_slots[index];
            if (m_mode == Mode::Load)
            {
                KitConfirm::Ask("${PAUSE_LOAD_CONFIRM}", [slot]()
                    {
                        GameSaveSystem::LoadGameFromFile(slot.Id);
                        UiMenus::Clear();   // closes the pause menu; the stack unpauses through UiMenus::OnPauseChanged
                    });
            }
            else
            {
                auto save = [this, slot]() { GameSaveSystem::SaveGameToFile(slot.Id); Refresh(); };
                if (slot.Exists) KitConfirm::Ask("${PAUSE_SAVE_CONFIRM_OVERWRITE}", save);
                else save();
            }
        });
    if (auto* list = dynamic_cast<UiElemListView*>(FindElem("slots")))
        list->OnBindRow = [this](int index, UiElem& row)
        {
            if (index < 0 || index >= static_cast<int>(m_slots.size()))
                return;
            const Slot& slot = m_slots[static_cast<size_t>(index)];
            row.SetVar("label", slot.Label);
            row.SetVar("status", slot.Status);
            const bool interactive = m_mode == Mode::Load || !slot.Reserved;
            row.SetAttribute("enabled", interactive ? "true" : "false");
            row.SetClass("locked", !interactive);
        };
    Refresh();
}

void KitSaveLoad::Refresh()
{
    std::unordered_set<std::string> existing;
    for (const auto& s : GameSaveSystem::GetAllSaves())
        existing.insert(s);
    struct Entry { std::string Id, Label; bool Reserved; };
    std::vector<Entry> all = { { "quicksave", "${PAUSE_SAVE_SLOT_QUICKSAVE}", true }, { "autosave", "${PAUSE_SAVE_SLOT_AUTOSAVE}", true } };
    for (int i = 0; i < kManualSlots; ++i)
        all.push_back({ ManualId(i), "${PAUSE_SAVE_SLOT_MANUAL} " + std::to_string(i + 1), false });
    m_slots.clear();
    for (const Entry& e : all)
    {
        Slot slot;
        slot.Id = e.Id;
        slot.Label = e.Label;
        slot.Reserved = e.Reserved;
        slot.Exists = existing.count(e.Id) != 0;
        if (m_mode == Mode::Load && !slot.Exists)
            continue;
        slot.Status = slot.Exists ? FormatTime(FileSystemEngine::GetFileModificationTime(GameSaveSystem::saveDataPath + e.Id + ".savg")) : "${PAUSE_SAVE_SLOT_EMPTY}";
        if (slot.Reserved && m_mode == Mode::Save)
            slot.Status += "  ${PAUSE_SAVE_SLOT_AUTO_ONLY}";
        m_slots.push_back(slot);
    }
    SetVisible(FindElem("empty"), m_slots.empty());
    SetVisible(FindElem("slots"), !m_slots.empty());
    if (auto* list = dynamic_cast<UiElemListView*>(FindElem("slots")))
    {
        list->SetAttribute("count", std::to_string(m_slots.size()));
        list->Rebind();
    }
}

// ── registration ──────────────────────────────────────────────────────────

void KitSetupMenus(std::function<void(bool paused)> onPauseChanged)
{
    UiMenus::PauseMenu = "ui/kit/pages/pause.xml";
    UiMenus::OnPauseChanged = std::move(onPauseChanged);
    UiMenus::Stack().OnEmptyBack = nullptr;
    UiInputPrompts::SetProvider([](const std::string& actionName, UiInputDevice device) -> std::string
        {
            InputAction* action = Input::GetAction(actionName);
            if (action == nullptr || action == &InputAction::NullAction)
                return "";
            if (device == UiInputDevice::Gamepad)
            {
                if (action->buttons.empty())
                    return "";
                return InputIconLibrary::GetGamepadIcon(InputBindingSlotGP::FromButton(action->buttons[0]));
            }
            if (device == UiInputDevice::KeyboardMouse)
            {
                if (!action->keys.empty())
                    return InputIconLibrary::GetKeyboardIcon(InputBindingSlotKB::FromKey(action->keys[0]));
                if (!action->mouseButtons.empty())
                    return InputIconLibrary::GetKeyboardIcon(InputBindingSlotKB::FromMouseButton(action->mouseButtons[0]));
            }
            return "";
        });
}

void KitInit()
{
    UiTagRegistry::RegisterCodeBehind(kPause, []() -> std::shared_ptr<UiElem> { return std::make_shared<KitPauseMenu>(); });
    UiTagRegistry::RegisterCodeBehind(kSettings, []() -> std::shared_ptr<UiElem> { return std::make_shared<KitSettingsMenu>(); });
    UiTagRegistry::RegisterCodeBehind(kInput, []() -> std::shared_ptr<UiElem> { return std::make_shared<KitInputPage>(); });
    UiTagRegistry::RegisterCodeBehind(kConfirm, []() -> std::shared_ptr<UiElem> { return std::make_shared<KitConfirmPage>(); });
    UiTagRegistry::RegisterCodeBehind(kSaveLoad, []() -> std::shared_ptr<UiElem> { return std::make_shared<KitSaveLoad>(); });
    KitSettings::Instance().RegisterUiSettings();
    // tab keys of the settings page: shoulder buttons, unless the game already defined the actions
    if (Input::GetAction("ui_tab_prev") == &InputAction::NullAction)
        Input::AddAction("ui_tab_prev")->AddButton(GamepadButton::LeftShoulder);
    if (Input::GetAction("ui_tab_next") == &InputAction::NullAction)
        Input::AddAction("ui_tab_next")->AddButton(GamepadButton::RightShoulder);
}
