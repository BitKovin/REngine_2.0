#pragma once
#include <UI/Elements/UiElemValue.h>

class UiSlider;
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
    bool CaptureValue(double& number, std::string&) const override { number = Value(); return true; }
    void RestoreValue(double number, const std::string&) override { SetValue(number); }

private:
    std::shared_ptr<UiSlider> m_slider;
    UiValueBinding m_binding{ *this };
    bool m_wasDragging = false;
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
