#pragma once
#include <UI/Elements/UiElemValue.h>

class UiSlider;
class UiDropdown;
class UiTextBox;
class UiVideo;
class UiCustomShaderImage;

// A markup element that owns one legacy widget as a legacy child (the wrapper pattern): the legacy widget fills the
// content box, markup attributes map onto its fields, and the computed style colours are copied onto its colour fields.
class UiElemLegacyWrapper : public UiElem
{
public:
    void UpdateChildrenOffsets() override;
    bool HasContent() const override { return true; }
    glm::vec2 MeasureContent(float, float) override { return m_natural; }

protected:
    void Attach(const std::shared_ptr<UiElement>& legacy);
    std::shared_ptr<UiElement> m_legacy;
    glm::vec2 m_natural = glm::vec2(0.f);
};

class UiElemSlider : public UiElemLegacyWrapper
{
public:
    UiElemSlider();
    static std::shared_ptr<UiElem> Create() { return std::make_shared<UiElemSlider>(); }
    bool ApplyAttribute(const std::string& name, const std::string& value) override;
    void Tick() override;
    void Update() override;
    double Value() const;
    void SetValue(double v);

private:
    std::shared_ptr<UiSlider> m_slider;
    UiValueBinding m_binding{ *this };
};

class UiElemDropDown : public UiElemLegacyWrapper
{
public:
    UiElemDropDown();
    static std::shared_ptr<UiElem> Create() { return std::make_shared<UiElemDropDown>(); }
    bool ApplyAttribute(const std::string& name, const std::string& value) override;
    void Tick() override;
    void Update() override;
    void SetOptions(const std::vector<std::string>& options);   // already localised
    int Selected() const;
    void SetSelected(int index);

private:
    void RefreshBoundOptions();
    std::shared_ptr<UiDropdown> m_drop;
    UiValueBinding m_binding{ *this };
    bool m_optionsFromAttribute = false;
};

class UiElemTextEntry : public UiElemLegacyWrapper
{
public:
    UiElemTextEntry();
    static std::shared_ptr<UiElem> Create() { return std::make_shared<UiElemTextEntry>(); }
    bool ApplyAttribute(const std::string& name, const std::string& value) override;
    void Tick() override;
    void Update() override;
    const std::string& Text() const;
    void SetText(const std::string& text);

private:
    std::shared_ptr<UiTextBox> m_box;
    std::string m_lastText;
};

class UiElemMovie : public UiElemLegacyWrapper
{
public:
    UiElemMovie();
    static std::shared_ptr<UiElem> Create() { return std::make_shared<UiElemMovie>(); }
    bool ApplyAttribute(const std::string& name, const std::string& value) override;

private:
    std::shared_ptr<UiVideo> m_video;
};

class UiElemShaderPanel : public UiElemLegacyWrapper
{
public:
    UiElemShaderPanel();
    static std::shared_ptr<UiElem> Create() { return std::make_shared<UiElemShaderPanel>(); }
    bool ApplyAttribute(const std::string& name, const std::string& value) override;

private:
    std::shared_ptr<UiCustomShaderImage> m_panel;
};
