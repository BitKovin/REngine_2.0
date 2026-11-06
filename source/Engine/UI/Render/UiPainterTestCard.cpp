#include "UiPainterTestCard.h"

#include <UI/Render/UiGpu.h>
#include <UI/Render/UiPainter.h>
#include <UI/UiElement.h>
#include <UI/UiRenderer.h>

namespace
{
    bool s_enabled = false;

    UiBoxPaint Fill(float r, float g, float b, float a)
    {
        UiBoxPaint paint;
        paint.Fill = glm::vec4(r, g, b, a);
        return paint;
    }
}

namespace UiPainterTestCard
{
    void SetEnabled(bool enabled) { s_enabled = enabled; }
    bool Enabled() { return s_enabled; }

    void Draw()
    {
        UiPainter painter(UiRenderer::CurrentList());

        // A: plain red box
        painter.Box({ 100.f, 100.f, 200.f, 100.f }, Fill(1.f, 0.f, 0.f, 1.f));

        // B: rounded green box
        UiBoxPaint rounded = Fill(0.f, 1.f, 0.f, 1.f);
        rounded.Radii = { 40.f, 40.f, 40.f, 40.f };
        painter.Box({ 350.f, 100.f, 200.f, 100.f }, rounded);

        // C: border only, four colours
        UiBoxPaint border = Fill(0.f, 0.f, 0.f, 0.f);
        border.BorderWidth = { 4.f, 4.f, 4.f, 4.f };
        border.BorderColor[0] = glm::vec4(1.f, 0.f, 0.f, 1.f);
        border.BorderColor[1] = glm::vec4(0.f, 1.f, 0.f, 1.f);
        border.BorderColor[2] = glm::vec4(0.f, 0.f, 1.f, 1.f);
        border.BorderColor[3] = glm::vec4(1.f, 1.f, 1.f, 1.f);
        painter.Box({ 600.f, 100.f, 200.f, 100.f }, border);

        // D: half-unit-thin white box (thin-element rule)
        painter.Box({ 850.f, 100.f, 200.f, 0.5f }, Fill(1.f, 1.f, 1.f, 1.f));

        // E: half-unit white border
        UiBoxPaint thinBorder = Fill(0.f, 0.f, 0.f, 0.f);
        thinBorder.BorderWidth = { 0.5f, 0.5f, 0.5f, 0.5f };
        for (glm::vec4& c : thinBorder.BorderColor)
            c = glm::vec4(1.f);
        painter.Box({ 850.f, 150.f, 200.f, 40.f }, thinBorder);

        // F: blue box clipped by a rounded clip
        painter.PushClip({ 100.f, 250.f, 200.f, 100.f }, { 30.f, 30.f, 30.f, 30.f });
        painter.Box({ 50.f, 200.f, 400.f, 250.f }, Fill(0.f, 0.f, 1.f, 1.f));
        painter.PopClip();

        // G: rotated yellow box
        painter.SetTransform(UiElement::Mat3Translate({ 700.f, 350.f }) * UiElement::Mat3Rotate(glm::radians(30.f)) * UiElement::Mat3Translate({ -50.f, -50.f }));
        painter.Box({ 0.f, 0.f, 100.f, 100.f }, Fill(1.f, 1.f, 0.f, 1.f));
        painter.SetTransform(glm::mat3(1.f));

        // H: tinted white image
        painter.Image({ 100.f, 450.f, 64.f, 64.f }, UiGpu::WhiteTexture(), glm::vec4(0.f, 0.f, 1.f, 1.f), glm::vec4(1.f, 0.f, 1.f, 1.f));
    }
}
