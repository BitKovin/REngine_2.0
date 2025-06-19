#pragma once

#include "UiElement.h"
#include "../Camera.h"
#include "UiManager.h"

class UiViewport : public UiElement
{
public:
	UiViewport()
	{

	}
	~UiViewport()
	{

	}

	void Update()
	{
		size = GetSize();

		parentTopLeft = vec2();
		parentBottomRight = size;


		UiElement::Update();

	}

	vec2 GetSize()
	{
		return vec2(UiManager::GetScaledUiHeight() * Camera::AspectRatio, UiManager::GetScaledUiHeight());
	}

private:

};