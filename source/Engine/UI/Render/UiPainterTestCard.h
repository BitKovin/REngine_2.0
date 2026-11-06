#pragma once

// Phase 0 visual self-test of UiPainter: a fixed set of boxes and an image at known positions.
// Enabled with the console command `ui_painter_test 1`, disabled with `ui_painter_test 0`.
namespace UiPainterTestCard
{
    void SetEnabled(bool enabled);
    bool Enabled();
    // Draws the card on UiRenderer::CurrentList(). Call inside the main UI pass.
    void Draw();
}
