#pragma once
#include <UI/Elements/UiElem.h>

class Texture;
struct UiAtlasImage;

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
    bool Fetch(UiAtlasImage& out);   // atlas image, or the standalone texture when decoding failed; false while not usable
    Texture* m_texture = nullptr;    // only for the fallback
    bool m_standalone = false;
    std::string m_src;
};
