#include "UiJoystick.h"

UiJoystick::UiJoystick()
{
	HitCheck = true;

	bgImage = make_shared<UiImage>();
	stickImage = make_shared<UiImage>();
	stickImage->pivot = vec2(0.5);
	stickImage->origin = vec2(0.5);

	AddChild(bgImage);
	AddChild(stickImage);

}

UiJoystick::~UiJoystick()
{


}

vec2 UiJoystick::GetTouchMovement()
{
	return Input::GetTouchEventDelta(TrackingTouch);
}

void UiJoystick::Update()
{

	if (Input::IsTouchEventReleased(TrackingTouch))
	{
		TrackingTouch = 0;
	}

	bgImage->size = GetSize();
	stickImage->size = GetSize() / 2.0f;

	UiElement::Update();

	for (auto& touch : TouchEvents)
	{
		if (touch.pressed && touch.id>1)
		{
			TrackingTouch = touch.id;

			//printf("%i \n", TrackingTouch);

		}

	}

	vec2 center = position + offset + size / 2.0f;

	vec2 relativeTouchPosition = Input::GetTouchEventPosition(TrackingTouch) - center;

	if (Input::IsTouchEventHolding(TrackingTouch) == false)
	{
		relativeTouchPosition = vec2(0);
	}

	InputPosition = relativeTouchPosition / size * 2.0f * vec2(1,-1);

	if (length(InputPosition) > 1)
	{
		InputPosition = normalize(InputPosition);
	}

	stickImage->position = InputPosition * vec2(1, -1) / 2.0f * size;


}
