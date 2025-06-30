#include "LoadingScreenSystem.h"
#include "../UI/UiRenderer.h"
#include "../EngineMain.h"
#include "../AssetRegistry.h"



void LoadingScreenSystem::Init()
{

}

void LoadingScreenSystem::SetLoadingCanvas(std::shared_ptr<UiLoadingScreenBase> canvas)
{

	viewport.ClearChildren();

	viewport.AddChild(canvas);
	uiCanvas = canvas;

}

void LoadingScreenSystem::Draw()
{

	if (uiCanvas == nullptr) return;

	uiCanvas->LoadingProgress = Progress;

	viewport.Update();
	viewport.FinalizeChildren();
	viewport.Draw();

	SDL_GL_SwapWindow(EngineMain::MainInstance->Window);

}

