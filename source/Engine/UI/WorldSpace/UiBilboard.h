#pragma once

#include "UiWorldSurface.h"
#include "../UiContentBox.hpp"
#include "../UiCanvas.hpp"

// A world widget made of legacy UI (a status icon over a head, a video on a plane). It is a UiWorldSurface that faces the camera
// and takes no pointer; its Canvas is updated by the surface on the main thread, so Update() is not needed any more.
class UiBilboard : public UiWorldSurface
{
public:
	UiBilboard(Entity* owner) : UiWorldSurface(owner)
	{
		FacingMode = Facing::Billboard;
		InteractionMode = Interaction::None;
		ViewportSize = glm::ivec2(512, 512);
		PixelsPerMeter = 100.0f;

		ContentBox = std::make_shared<UiContentBox>();
		ContentBox->pivot = vec2(0.5f, 0.5f);
		ContentBox->origin = vec2(0.5f, 0.5f);
		Canvas.AddChild(ContentBox);
	}

	// Kept so existing callers compile; the surface updates the canvas itself (main thread, only while visible and in range).
	void Update() {}

	std::shared_ptr<UiContentBox> ContentBox;
	UiCanvas Canvas;
	float& PixelPerMeter = PixelsPerMeter;   // the old name

	void FinalizeFrameData() override
	{
		Canvas.size = vec2((float)ViewportSize.x, (float)ViewportSize.y);
		UiWorldSurface::FinalizeFrameData();
	}

protected:
	UiElement& ContentRoot() override { return Canvas; }
	bool AlwaysRender() const override { return true; }
	void UpdateLegacyContent() override
	{
		Canvas.size = vec2((float)ViewportSize.x, (float)ViewportSize.y);
		Canvas.Update();
		Canvas.UpdateChildrenOffsetRecursive();
	}
};
