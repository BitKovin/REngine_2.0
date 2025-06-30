#include "UiDefaultLoadingScreen.h"

UiDefaultLoadingScreen::UiDefaultLoadingScreen()
{

	bgImage = make_shared<UiImage>();
	bgImage->ImagePath = "GameData/cat.png";

	loadingBarImage = make_shared<UiImage>();
	loadingBarImage->ImagePath = "GameData/cat.png";
	loadingBarImage->pivot = vec2(0, 1);
	loadingBarImage->origin = vec2(0, 1);
	loadingBarImage->size = vec2(100,100);

	AddChild(bgImage);
	AddChild(loadingBarImage);

}

UiDefaultLoadingScreen::~UiDefaultLoadingScreen()
{
}

void UiDefaultLoadingScreen::Update()
{

	bgImage->size = GetSize();
	loadingBarImage->size.x = bgImage->size.x * LoadingProgress;

	UiLoadingScreenBase::Update();

}
