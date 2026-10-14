#pragma once

#include <UI/UiButton.hpp>
#include <UI/UiVerticalBox.hpp>
#include <UI/UiHorizontalBox.hpp>
#include <UI/UiText.hpp>
#include <UI/UiImage.hpp>
#include <UI/UiCanvas.hpp>
#include <UI/UiSlider.hpp>
#include <UI/UiTextBox.hpp>

#include <PauseGameManager.hpp>

#include "Settings/GameSettings.h"
#include "UiSettingsStyle.hpp"

#include <algorithm>
#include <iomanip>
#include <sstream>

// ---------------------------------------------------------------------------
// UiGameSettings
//
// Gameplay-settings screen. Same shell as UiVideoSettings / UiInputSettings
// (UiCanvas, FocusTrap, scrim, title, card panels, backButton wired to both
// onClick and OnNavCancel).
//
// Camera card (real): Camera Shake, a slider + numeric textbox kept in sync
// (UiInputSettings::BuildSensitivitySection idiom), writing
// GameSettings::Instance().Gameplay.CameraShake. The value is read live by
// PlayerCamera, so it applies while dragging; it is saved on drag end and on
// textbox submit.
//
// Gameplay card (placeholder): the three rows (Difficulty / Tutorial Hints /
// Autosave) are mocked, not functional -- they exist to communicate the shape
// of what's coming, not to be interacted with. Everything in a row has
// DisableFocus=true and HitCheck=false, same idiom UiInputSettings uses for
// its category headers, so gamepad/keyboard nav skips over them.
// ---------------------------------------------------------------------------

class UiGameSettings : public UiCanvas
{
public:

    std::shared_ptr<UiElement> parentMenu;

    UiGameSettings(std::shared_ptr<UiElement> parentMenu_)
    {
        FocusTrap = true;
        this->parentMenu = parentMenu_;

        background = std::make_shared<UiImage>();
        background->color = SettingsStyle::Scrim;
        AddChild(background);

        rootBox = std::make_shared<UiVerticalBox>();
        rootBox->origin = vec2(0.5f);
        rootBox->pivot = vec2(0.5f);
        rootBox->ContentDistance = 20.f;

        titleText = std::make_shared<UiText>();
        titleText->text = "${SETTINGS_GAME_TITLE}";
        titleText->fontSize = SettingsStyle::TitleSize;
        titleText->pivot = vec2(0.5f, 0.f);
        titleText->origin = vec2(0.5f, 0.f);

        BuildCameraSection();
        BuildPlaceholderSection();

        buttonsRow = std::make_shared<UiHorizontalBox>();
        buttonsRow->origin = vec2(0.5f, 0);
        buttonsRow->pivot = vec2(0.5f, 0);

        backButton = MakeButton("${SETTINGS_BACK}");
        buttonsRow->AddChild(backButton);

        rootBox->AddChild(titleText);
        rootBox->AddChild(cameraPanel);
        rootBox->AddChild(gameplayPanel);
        rootBox->AddChild(MakeDivider(SettingsStyle::ContentWidth * 0.6f, vec4(1.f, 1.f, 1.f, 0.10f)));
        rootBox->AddChild(buttonsRow);

        AddChild(rootBox);

        backButton->onClick = [this]()
        {
            parentMenu->visible = true;
            RemoveFromParent();
        };

        UpdateChildrenOffsetRecursive();
        UpdateChildrenOffsetRecursive();
    }

    void OnNavCancel() override { backButton->onClick(); }

    void FinalizeChildren() override
    {
        background->size = GetSize();
        UiCanvas::FinalizeChildren();
    }

private:

    std::shared_ptr<UiImage> background;
    std::shared_ptr<UiVerticalBox> rootBox;
    std::shared_ptr<UiText> titleText;

    // ── Camera ──────────────────────────────────────────────────────────────
    std::shared_ptr<UiElement> cameraPanel;
    std::shared_ptr<UiSlider> cameraShakeSlider;
    std::shared_ptr<UiTextBox> cameraShakeText;

    // ── Gameplay (placeholder) ─────────────────────────────────────────────
    std::shared_ptr<UiElement> gameplayPanel;

    std::shared_ptr<UiHorizontalBox> buttonsRow;
    std::shared_ptr<UiButton> backButton;

    // ───────────────────────────────────────────────────────────────────────
    // Camera section
    // ───────────────────────────────────────────────────────────────────────

    void BuildCameraSection()
    {
        using namespace SettingsStyle;

        auto content = std::make_shared<UiVerticalBox>();
        content->ContentDistance = 14.f;

        auto header = std::make_shared<UiText>();
        header->text = "${SETTINGS_GAME_CAMERA_HEADER}";
        header->fontSize = PanelHeaderSize;
        header->pivot = vec2(0.f, 0.0f);
        header->origin = vec2(0.f, 0.0f);
        content->AddChild(header);
        content->AddChild(MakeDivider(ContentWidth - PanelPadding * 2.f));

        const float shake = GameSettings::Instance().Gameplay.CameraShake;

        cameraShakeSlider = std::make_shared<UiSlider>();
        cameraShakeSlider->MinValue = GameplaySettingsData::MinCameraShake;
        cameraShakeSlider->MaxValue = GameplaySettingsData::MaxCameraShake;
        cameraShakeSlider->Value = shake;
        cameraShakeSlider->Step = 0.05f;
        cameraShakeSlider->size = vec2(360.f, 28.f);

        cameraShakeText = std::make_shared<UiTextBox>();
        cameraShakeText->NumericOnly = true;
        cameraShakeText->SetText(FormatFloat(shake));
        cameraShakeText->size = vec2(90.f, 40.f);

        cameraShakeSlider->onValueChanged = [this](float v)
        {
            GameSettings::Instance().Gameplay.CameraShake = v;
            cameraShakeText->SetText(FormatFloat(v));
        };
        cameraShakeSlider->onDragEnd = [](float) { GameSettings::Instance().SaveToFile(); };

        cameraShakeText->onSubmit = [this](const std::string& text)
        {
            const float v = ParseFloatClamped(text, GameplaySettingsData::MinCameraShake, GameplaySettingsData::MaxCameraShake);
            GameSettings::Instance().Gameplay.CameraShake = v;
            cameraShakeSlider->Value = v;
            cameraShakeText->SetText(FormatFloat(v));
            GameSettings::Instance().SaveToFile();
        };

        content->AddChild(MakeSliderRow("${SETTINGS_GAME_CAMERA_SHAKE}", cameraShakeSlider, cameraShakeText));

        auto note = std::make_shared<UiText>();
        note->text = "${SETTINGS_GAME_CAMERA_SHAKE_NOTE}";
        note->fontSize = CaptionSize;
        note->textColor = CaptionColor;
        note->DisableFocus = true;
        note->HitCheck = false;
        note->pivot = vec2(0.f, 0.f);
        note->origin = vec2(0.f, 0.f);
        content->AddChild(note);

        cameraPanel = std::make_shared<UiCardPanel>(vec2(ContentWidth, 200.f), content);
    }

    std::shared_ptr<UiHorizontalBox> MakeSliderRow(const std::string& label, std::shared_ptr<UiElement> control,
                                                   std::shared_ptr<UiElement> secondaryControl)
    {
        auto labelCell = std::make_shared<UiElement>();
        labelCell->size = vec2(320.f, 44.f);

        auto txt = std::make_shared<UiText>();
        txt->text = label;
        txt->fontSize = SettingsStyle::RowLabelSize;
        txt->pivot = vec2(0.f, 0.5f);
        txt->origin = vec2(0.f, 0.5f);
        labelCell->AddChild(txt);

        auto row = std::make_shared<UiHorizontalBox>();
        row->ContentDistance = 14.f;
        row->AddChild(labelCell);
        row->AddChild(control);
        if (secondaryControl)
            row->AddChild(secondaryControl);
        return row;
    }

    static std::string FormatFloat(float v)
    {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2) << v;
        return ss.str();
    }

    static float ParseFloatClamped(const std::string& text, float minV, float maxV)
    {
        try { return std::clamp(std::stof(text), minV, maxV); }
        catch (...) { return minV; }
    }

    // ───────────────────────────────────────────────────────────────────────
    // Placeholder section
    // ───────────────────────────────────────────────────────────────────────

    void BuildPlaceholderSection()
    {
        using namespace SettingsStyle;

        auto content = std::make_shared<UiVerticalBox>();
        content->ContentDistance = 14.f;

        auto header = std::make_shared<UiText>();
        header->text = "${SETTINGS_GAME_HEADER}";
        header->fontSize = PanelHeaderSize;
        header->pivot = vec2(0.f, 0.0f);
        header->origin = vec2(0.f, 0.0f);
        content->AddChild(header);
        content->AddChild(MakeDivider(ContentWidth - PanelPadding * 2.f));

        content->AddChild(MakePlaceholderRow("${SETTINGS_GAME_DIFFICULTY}"));
        content->AddChild(MakePlaceholderRow("${SETTINGS_GAME_TUTORIAL_HINTS}"));
        content->AddChild(MakePlaceholderRow("${SETTINGS_GAME_AUTOSAVE}"));

        content->AddChild(MakeSpacer(vec2(1.f, 4.f)));

        auto note = std::make_shared<UiText>();
        note->text = "${SETTINGS_GAME_COMING_SOON_NOTE}";
        note->fontSize = CaptionSize;
        note->textColor = CaptionColor;
        note->DisableFocus = true;
        note->HitCheck = false;
        note->pivot = vec2(0.f, 0.f);
        note->origin = vec2(0.f, 0.f);
        content->AddChild(note);

        gameplayPanel = std::make_shared<UiCardPanel>(vec2(ContentWidth, 350.f), content);
    }

    // A label + an inert "pill" the same footprint as a real toggle
    // (invertYButton in UiInputSettings is the model) -- so when a row here
    // gets wired up for real, the layout doesn't shift around it.
    std::shared_ptr<UiElement> MakePlaceholderRow(const std::string& label)
    {
        using namespace SettingsStyle;

        auto labelCell = std::make_shared<UiElement>();
        labelCell->size = vec2(320.f, 44.f);
        labelCell->DisableFocus = true;
        labelCell->HitCheck = false;

        auto labelTxt = std::make_shared<UiText>();
        labelTxt->text = label;
        labelTxt->fontSize = RowLabelSize;
        labelTxt->DisableFocus = true;
        labelTxt->HitCheck = false;
        labelTxt->pivot = vec2(0.f, 0.5f);
        labelTxt->origin = vec2(0.f, 0.5f);
        labelCell->AddChild(labelTxt);

        auto placeholderPill = std::make_shared<UiElement>();
        placeholderPill->size = vec2(120.f, 40.f);
        placeholderPill->DisableFocus = true;
        placeholderPill->HitCheck = false;

        auto pillBg = std::make_shared<UiImage>();
        pillBg->color = ToggleOff;
        pillBg->size = placeholderPill->size;
        pillBg->DisableFocus = true;
        pillBg->HitCheck = false;
        placeholderPill->AddChild(pillBg);

        auto pillLabel = std::make_shared<UiText>();
        pillLabel->text = "${SETTINGS_GAME_PLACEHOLDER_SOON}";
        pillLabel->fontSize = 24.f;
        pillLabel->textColor = CaptionColor;
        pillLabel->DisableFocus = true;
        pillLabel->HitCheck = false;
        pillLabel->pivot = vec2(0.5f);
        pillLabel->origin = vec2(0.5f);
        placeholderPill->AddChild(pillLabel);

        auto row = std::make_shared<UiHorizontalBox>();
        row->ContentDistance = 14.f;
        row->AddChild(labelCell);
        row->AddChild(placeholderPill);

        return row;
    }

    std::shared_ptr<UiButton> MakeButton(std::string text)
    {
        std::shared_ptr<UiButton> btn = std::make_shared<UiButton>();
        std::shared_ptr<UiText> txt = std::make_shared<UiText>();

        btn->size = vec2(400, 70);
        txt->text = text;
        txt->fontSize = SettingsStyle::ButtonLabelSize;
        txt->pivot = vec2(0.5f);
        txt->origin = vec2(0.5f);
        btn->AddChild(txt);

        return btn;
    }

};
