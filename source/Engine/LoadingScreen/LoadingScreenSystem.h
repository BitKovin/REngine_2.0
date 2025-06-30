#pragma once

#include "UiLoadingScreenBase.h"
#include "../UI/UiViewport.hpp"

class LoadingScreenSystem
{
public:

	static void Init();

	static void SetLoadingCanvas(std::shared_ptr<UiLoadingScreenBase> canvas);

	static void Draw();

	inline static float Progress = 0;

private:

	inline static std::shared_ptr<UiLoadingScreenBase> uiCanvas = nullptr;
	inline static UiViewport viewport;

};

