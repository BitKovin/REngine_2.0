#pragma once
#include <UI/Elements/UiElem.h>
#include <UI/Render/UiTextLayout.h>

class UiElemLabel : public UiElem
{
public:
    UiElemLabel() { TagName = "Label"; }
    static std::shared_ptr<UiElem> Create() { return std::make_shared<UiElemLabel>(); }

    bool ApplyAttribute(const std::string& name, const std::string& value) override;
    glm::vec2 MeasureContent(float availableWidth, float availableHeight) override;
    void PaintContent(UiPainter& painter, const UiRect& contentRect) override;
    bool HasContent() const override { return true; }

    // Used by TextButton, which forwards its already substituted text.
    void SetResolvedText(const std::string& text);
    const std::string& Text() const { return m_text; }
    const std::string& DisplayText() const { return m_layout.DisplayText; }
    int Lines() const { return m_layout.Lines; }
    float TextWidth() const { return m_layout.Width; }
    float UsedFontSize() const { return m_layout.UsedFontSize; }

private:
    std::string m_text;
    bool m_html = false;
    UiTextParams m_lastParams;
    bool m_haveLayout = false;
    UiTextLayoutResult m_layout;
};
