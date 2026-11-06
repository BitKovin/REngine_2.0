#include "UiMenuPreviews.h"

#include <UI/Elements/UiElemButton.h>
#include <UI/Elements/UiElemInputPrompt.h>
#include <UI/Elements/UiElemLabel.h>
#include <UI/Elements/UiElemPanel.h>
#include <UI/Menus/UiInputPrompts.h>

namespace
{
    using nlohmann::json;

    UiPreviewScenario Scenario(const char* id, const char* label, json values)
    {
        UiPreviewScenario s;
        s.Id = id;
        s.Label = label;
        s.Values = std::move(values);
        return s;
    }

    // A sample game-owned runtime preview: an action bar row. It uses the real InputPrompt and Label elements and the real
    // input-device paths, and creates nothing that can reach gameplay (no handlers, no actions).
    class ActionBarPreviewProvider : public IUiPreviewProvider
    {
    public:
        UiPreviewDescriptor Describe() const override
        {
            UiPreviewDescriptor d;
            d.Id = "runtime.engine.action-bar";
            d.Name = "Action bar (sample runtime preview)";
            d.Category = "Menus/Input";
            d.Description = "Prompt and label entries as an ActionBar shows them. Handlers are stubbed: nothing is activated.";
            d.Kind = "runtime";
            d.ProviderId = "engine.ui.preview.action-bar";
            d.ProviderVersion = 1;
            d.SourceRefs.push_back({ "code", "source/Engine/UI/Elements/UiElemActionBar.cpp" });
            d.HostKind = "menu-stack";
            d.RequiredCapabilities = { "menus", "inputPrompts" };
            UiPreviewParameter device;
            device.Name = "device";
            device.Type = "choice";
            device.Default = "mouse";
            device.Choices = { "mouse", "gamepad", "touch" };
            device.Help = "Input device fixture; selects the prompt images";
            d.Parameters.push_back(device);
            UiPreviewParameter back;
            back.Name = "back";
            back.Type = "bool";
            back.Default = true;
            back.Help = "Shows the fixed back entry";
            d.Parameters.push_back(back);
            d.Scenarios.push_back(Scenario("keyboard", "Keyboard and mouse", { { "device", "mouse" } }));
            d.Scenarios.push_back(Scenario("gamepad", "Gamepad", { { "device", "gamepad" } }));
            d.Scenarios.push_back(Scenario("touch", "Touch", { { "device", "touch" } }));
            return d;
        }

        bool CreatePreview(UiPreviewHost&, const json& values, std::shared_ptr<UiElem>& outRoot, std::string&) override
        {
            auto root = std::make_shared<UiElemPanel>();
            root->Style().Flow = UiFlow::Right;
            root->Style().Gap = UiLength{ UiLenKind::Px, 24.f };
            outRoot = root;
            Fill(*root, values);
            return true;
        }

        bool ApplyValues(UiPreviewHost&, UiElem& root, const json& values, std::string&) override
        {
            root.ClearChildren();
            Fill(root, values);
            return true;
        }

        bool ApplyScenario(UiPreviewHost& host, UiElem& root, const std::string& scenarioId, std::string& error) override
        {
            for (const UiPreviewScenario& s : Describe().Scenarios)
                if (s.Id == scenarioId)
                    return ApplyValues(host, root, s.Values, error);
            error = "unknown scenario '" + scenarioId + "'";
            return false;
        }

        void DestroyPreview(UiPreviewHost&, std::shared_ptr<UiElem>& root) override { root.reset(); }

    private:
        static void Fill(UiElem& root, const json& values)
        {
            const std::string device = values.value("device", "mouse");
            UiInputPrompts::SetLastDevice(device == "gamepad" ? UiInputDevice::Gamepad : (device == "touch" ? UiInputDevice::Touch : UiInputDevice::KeyboardMouse));
            struct Entry { const char* Action; const char* Text; };
            std::vector<Entry> entries = { { "ui_confirm", "Select" } };
            if (values.value("back", true))
                entries.push_back({ "ui_cancel", "Back" });
            for (const Entry& e : entries)
            {
                auto item = std::make_shared<UiElemPanel>();
                item->Style().Flow = UiFlow::Right;
                item->Style().Gap = UiLength{ UiLenKind::Px, 8.f };
                root.AddChild(item);
                auto prompt = std::make_shared<UiElemInputPrompt>();
                prompt->Style().Width = UiLength{ UiLenKind::Px, 36.f };
                prompt->Style().Height = UiLength{ UiLenKind::Px, 36.f };
                item->AddChild(prompt);
                prompt->Attributes["action"] = e.Action;
                prompt->ApplyAttributeValue("action", e.Action, false);
                auto label = std::make_shared<UiElemLabel>();
                item->AddChild(label);
                label->SetResolvedText(e.Text);
            }
        }
    };
}

void UiMenuPreviews::RegisterProviders()
{
    UiPreviewRegistry::RegisterProvider(std::make_shared<ActionBarPreviewProvider>());
}

void UiMenuPreviews::AddRecipes()
{
    // Synthetic Button states, replayed through engine input simulation (mouse_move, mouse_down, ui_down, ui_confirm).
    // The scenarios never attach handlers: no Quit, PlaySound, PushMenu or gameplay action can run.
    UiPreviewDescriptor d;
    d.Id = "engine.button.states";
    d.Name = "Button states";
    d.Category = "Menus/Input";
    d.Description = "Idle, hover, pressed, focus, disabled, navigation focus and activation of a Button; handlers are stubbed.";
    d.Kind = "element";
    d.HostKind = "viewport";
    d.RequiredCapabilities = { "events" };
    d.SourceRefs.push_back({ "code", "source/Engine/UI/Elements/UiElemButton.cpp" });
    d.Scenarios.push_back(Scenario("idle", "Idle", { { "state", "idle" } }));
    d.Scenarios.push_back(Scenario("hovered", "Hovered", { { "state", "hovered" }, { "input", json::array({ "mouse_move" }) } }));
    d.Scenarios.push_back(Scenario("pressed", "Pressed", { { "state", "pressed" }, { "input", json::array({ "mouse_move", "mouse_down" }) } }));
    d.Scenarios.push_back(Scenario("pointer-capture", "Pointer capture (drag outside)", { { "state", "pressed" }, { "input", json::array({ "mouse_move", "mouse_down", "mouse_move" }) } }));
    d.Scenarios.push_back(Scenario("focused", "Focused", { { "state", "focused" }, { "input", json::array({ "ui_down" }) } }));
    d.Scenarios.push_back(Scenario("disabled", "Disabled", { { "state", "disabled" } }));
    d.Scenarios.push_back(Scenario("navigation-focus", "Navigation focus", { { "state", "focused" }, { "input", json::array({ "ui_down", "ui_down" }) } }));
    d.Scenarios.push_back(Scenario("keyboard-activation", "Keyboard activation", { { "state", "activated" }, { "input", json::array({ "ui_confirm" }) } }));
    d.Scenarios.push_back(Scenario("gamepad-activation", "Gamepad activation", { { "state", "activated" }, { "device", "gamepad" }, { "input", json::array({ "ui_confirm" }) } }));
    UiPreviewRegistry::RegisterRecipe(d);
}
