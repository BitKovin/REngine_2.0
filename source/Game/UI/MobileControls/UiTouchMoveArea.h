#pragma once
#include <UI/UiImage.hpp>
#include <glm.h>

class UiTouchMoveArea : public UiImage
{
public:
	UiTouchMoveArea();
	~UiTouchMoveArea();

	vec2 GetTouchMovement();

	void Update();

	int TrackingTouch = 0;

private:

};

