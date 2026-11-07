#pragma once
#include <UI/Elements/UiElemPanel.h>
#include <UI/Kit/Settings/InputBindingTypes.h>

#include <functional>
#include <string>
#include <vector>

// The engine kit: pause, settings, dialogs and save/load pages shared by every game. The markup is in
// GameData/ui/kit/pages/ (synced from Tools/ui/assets); these classes are its code-behind. A game only
// supplies GameData/ui/theme.css (the look) and, where it wants, the hooks below.

// Registers the code-behind of every kit page and the kit settings. Called by UiSystem::Init.
void KitInit();

// Makes the kit pause page the pause menu of the main stack and shows the prompt icons of the kit's icon library.
// `onPauseChanged` is the game's way of pausing or resuming (PauseGameManager lives in the games).
void KitSetupMenus(std::function<void(bool paused)> onPauseChanged);

// Asks a yes/no question on the main menu stack: `message` is text or ${KEY}.
namespace KitConfirm
{
    void Ask(const std::string& message, std::function<void()> yes, std::function<void()> no = nullptr);
}

// Capture of one binding: a modal page that listens for the next key, mouse button or gamepad button.
namespace KitRebind
{
    struct Request
    {
        bool Keyboard = true;
        std::string Action;                       // display name
        int Slot = 0;
        InputBindingSlotKB KB;                    // the current binding of the slot
        InputBindingSlotGP GP;
        bool Bound = false;
        std::function<void(InputBindingSlotKB)> OnKB;
        std::function<void(InputBindingSlotGP)> OnGP;
        std::function<void()> OnCancel;
        std::function<void()> OnUnbind;
    };
    void Open(Request request);
}

class KitPauseMenu : public UiElemPanel
{
public:
    KitPauseMenu();
    static inline bool ShowSaveLoad = true;            // false: the page has no Save and Load items
    static inline std::function<bool()> CanSave;       // null = saving is always possible
    static inline std::function<void()> OnRestart;     // set: the page shows Restart and calls this after a confirmation
};

class KitSettingsMenu : public UiElemPanel
{
public:
    KitSettingsMenu();
    // A game's settings tab: `labelOrKey` is text or ${KEY}, `page` a layout (it is shown in the page's frame).
    static void RegisterTab(const std::string& labelOrKey, const std::string& page);
    void OnPushed() override;
    void OnPopped() override;
    void Tick() override;

private:
    void Select(const std::string& page);
    struct Tab { UiElem* Button; std::string Page; };
    std::vector<Tab> m_tabs;
    std::string m_current;
};

class KitInputPage : public UiElemPanel
{
public:
    KitInputPage();

private:
    struct Row { std::string Action; UiElem* Elem = nullptr; };
    void BuildRows();
    void RefreshRow(const std::string& action);
    void RefreshAll();
    void Capture(const std::string& action, bool keyboard, int slot);
    void Listening(const std::string& action, bool keyboard, int slot, bool on);
    std::vector<Row> m_rows;
};

class KitSaveLoad : public UiElemPanel
{
public:
    enum class Mode { Save, Load };
    static void Open(Mode mode);       // pushes the page on the main stack
    explicit KitSaveLoad(Mode mode = Mode::Load);

private:
    struct Slot { std::string Id, Label, Status; bool Exists = false, Reserved = false; };
    void Refresh();
    std::vector<Slot> m_slots;         // the slots shown
    Mode m_mode;
};
