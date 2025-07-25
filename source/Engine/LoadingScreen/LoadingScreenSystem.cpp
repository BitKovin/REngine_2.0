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

void LoadingScreenSystem::Update(float newProgress)
{

	Progress = newProgress;
	Draw();

}

void LoadingScreenSystem::Draw()
{

	if (uiCanvas == nullptr) return;

	uiCanvas->LoadingProgress = Progress;

	glClearColor(0, 0, 0, 1);

	viewport.Update();
	viewport.FinalizeChildren();
	viewport.Draw();

	glFinish();
	glFlush();

	SDL_GL_SwapWindow(EngineMain::MainInstance->Window);

}

