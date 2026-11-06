#pragma once
#include <UI/Elements/UiElem.h>

class Texture;

class UiElemImage : public UiElem
{
public:
    UiElemImage() { TagName = "Image"; }
    static std::shared_ptr<UiElem> Create() { return std::make_shared<UiElemImage>(); }

    bool ApplyAttribute(const std::string& name, const std::string& value) override;
    glm::vec2 MeasureContent(float availableWidth, float availableHeight) override;
    void PaintContent(UiPainter& painter, const UiRect& contentRect) override;
    bool HasContent() const override { return true; }
    const std::string& Src() const { return m_src; }

private:
    Texture* m_texture = nullptr;
    std::string m_src;
};
