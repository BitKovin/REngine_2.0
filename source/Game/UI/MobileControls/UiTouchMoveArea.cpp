#include "UiTouchMoveArea.h"

UiTouchMoveArea::UiTouchMoveArea()
{
	HitCheck = true;
}

UiTouchMoveArea::~UiTouchMoveArea()
{
}

vec2 UiTouchMoveArea::GetTouchMovement()
{
	return Input::GetTouchEventDelta(TrackingTouch);
}

void UiTouchMoveArea::Update()
{

	if (Input::IsTouchEventReleased(TrackingTouch))
	{
		TrackingTouch = 0;
	}

	UiElement::Update();

	for (auto& touch : TouchEvents)
	{
		if (touch.pressed && touch.id > 1)
		{
			TrackingTouch = touch.id;

			//printf("%i \n", TrackingTouch);

		}

	}
}
