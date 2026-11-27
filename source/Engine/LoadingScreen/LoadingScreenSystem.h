#pragma once

#include "UiLoadingScreenBase.h"
#include "../UI/UiViewport.hpp"
#include "../UI/Elements/UiElem.h"

class LoadingScreenSystem
{
public:

	static void Init();

	static void SetLoadingCanvas(std::shared_ptr<UiLoadingScreenBase> canvas);

	// A markup loading screen: the page is loaded into this system's own viewport (nothing of the main UI is touched). Each Update(progress)
	// sets the variable `progress` (0..1) on the page, so a label or a bar can show it:  <Label text="Loading {progress:%.2f}"/>
	static void SetLoadingScreen(const std::string& layoutPath);

	static void Update(float newProgress);

	static void Draw();

	inline static float Progress = 0;

private:

	inline static std::shared_ptr<UiLoadingScreenBase> uiCanvas = nullptr;
	inline static std::shared_ptr<UiElem> markupRoot = nullptr;
	inline static UiViewport viewport;

};

