#pragma once
#include <UI/Elements/UiElemButton.h>
#include <UI/Elements/UiElemValue.h>

// A Button that toggles :checked and the variable "value" (0/1) when activated, then fires valuechanged.
class UiElemToggleButton : public UiElemButton
{
public:
    UiElemToggleButton();
    static std::shared_ptr<UiElem> Create() { return std::make_shared<UiElemToggleButton>(); }
    bool ApplyAttribute(const std::string& name, const std::string& value) override;
    void OnActivate() override;
    void Update() override;
    bool Checked() const { return (States & kStateChecked) != 0; }
    void SetChecked(bool on);

private:
    UiValueBinding m_binding{ *this };
};

// One of a group: activating it checks it and unchecks the others of the same group inside the same root.
// With option="N" and bind="setting:<key>" the group edits that setting: the radio whose option equals the setting is checked.
class UiElemRadioButton : public UiElemButton
{
public:
    UiElemRadioButton();
    static std::shared_ptr<UiElem> Create() { return std::make_shared<UiElemRadioButton>(); }
    bool ApplyAttribute(const std::string& name, const std::string& value) override;
    void OnActivate() override;
    void Update() override;
    bool Checked() const { return (States & kStateChecked) != 0; }
    void SetChecked(bool on);
    const std::string& Group() const { return m_group; }

private:
    std::string m_group;
    double m_option = -1.0;
    bool m_hasOption = false;
    UiValueBinding m_binding{ *this };
};

// A track (the element's own box) with a `.fill` child panel whose width (height when direction="vertical") is value x content.
class UiElemProgressBar : public UiElem
{
public:
    UiElemProgressBar();
    static std::shared_ptr<UiElem> Create() { return std::make_shared<UiElemProgressBar>(); }
    bool ApplyAttribute(const std::string& name, const std::string& value) override;
    double Value() const { return m_value; }
    UiElem* Fill() const { return m_fill; }

private:
    void ApplyFill();
    double m_value = 0.0;
    bool m_vertical = false;
    UiElem* m_fill = nullptr;
};

// A ring: the track is background-color, the fill is color, drawn with the arc primitive.
class UiElemCircularProgress : public UiElem
{
public:
    UiElemCircularProgress() { TagName = "CircularProgress"; }
    static std::shared_ptr<UiElem> Create() { return std::make_shared<UiElemCircularProgress>(); }
    bool ApplyAttribute(const std::string& name, const std::string& value) override;
    void PaintContent(UiPainter& painter, const UiRect& contentRect) override;
    bool DrawsBoxBackground() const override { return false; }
    double Value() const { return m_value; }

private:
    double m_value = 0.0;
    float m_start = 0.f, m_thickness = 8.f;
};
