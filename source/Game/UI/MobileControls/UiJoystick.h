#pragma once

#include <UI/UiImage.hpp>

class UiJoystick : public UiImage
{
public:
	UiJoystick();
	~UiJoystick();

	vec2 GetTouchMovement();

	void Update();

	int TrackingTouch = 0;

	vec2 InputPosition = vec2(0);

private:

	shared_ptr<UiImage> bgImage;
	shared_ptr<UiImage> stickImage;

};