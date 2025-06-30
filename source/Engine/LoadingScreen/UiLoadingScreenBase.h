#pragma once

#include "../UI/UiCanvas.hpp"

class UiLoadingScreenBase : public UiCanvas
{
public:
	UiLoadingScreenBase() = default;

	float LoadingProgress = 0; //0.0 to 1.0

private:

};


