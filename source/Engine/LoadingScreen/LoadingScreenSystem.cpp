#include "LoadingScreenSystem.h"
#include "../UI/UiRenderer.h"
#include "../EngineMain.h"
#include "../AssetRegistry.h"
#include "../UI/UiSystem.h"
#include "../UI/Style/UiAnimation.h"
#include "../UI/Render/UiFontLibrary.h"
#include "../UI/Render/UiImageAtlas.h"
#include "../Screenshot.h"

#include <algorithm>

#include <BgfxStateManager.h>

#include <Renderer/Abstractions/ViewIdManager.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

void LoadingScreenSystem::Init()
{

}

void LoadingScreenSystem::SetLoadingCanvas(std::shared_ptr<UiLoadingScreenBase> canvas)
{

	viewport.ClearChildren();

	viewport.AddChild(canvas);
	uiCanvas = canvas;

}

void LoadingScreenSystem::SetLoadingScreen(const std::string& layoutPath)
{
	viewport.ClearChildren();
	uiCanvas = nullptr;
	markupRoot = UiElem::Load(layoutPath);
	if (!markupRoot)
		return;
	UiSystem::RegisterViewport(viewport);
	viewport.AddChild(markupRoot);
}

void LoadingScreenSystem::Update(float newProgress)
{

	Progress = newProgress;
	Draw();

}

void LoadingScreenSystem::Draw()
{
    if (uiCanvas == nullptr && markupRoot == nullptr)
        return;

    // -ui_test_loading_shots [dir]: a screenshot at progress 0, 0.5 and 1 (the test runner cannot run during a blocking load)
    if (EngineMain::MainInstance != nullptr)
    {
        const auto shots = EngineMain::MainInstance->Arguments.find("ui_test_loading_shots");
        if (shots != EngineMain::MainInstance->Arguments.end())
        {
            static bool taken[3] = { false, false, false };
            const float at[3] = { 0.f, 0.5f, 1.f };
            const std::string dir = shots->second.empty() ? "ui_test_out/" : shots->second[0];
            for (int i = 0; i < 3; ++i)
                if (!taken[i] && Progress >= at[i])
                {
                    taken[i] = true;
                    Screenshot::Request(dir + "loading_" + std::to_string(i) + ".png");
                }
        }
    }

	BgfxStateManager::SetDepthTest(BgfxStateManager::DepthTest::Always);

    if (uiCanvas)
        uiCanvas->LoadingProgress = Progress;

    const uint16_t viewId = ViewIdManager::GetCurrentId(); // loading screen view

    const uint16_t width = EngineMain::MainInstance->ScreenSize.x;
    const uint16_t height = EngineMain::MainInstance->ScreenSize.y;

    // Configure view
    bgfx::setViewRect(viewId, 0, 0, width, height);
    bgfx::setViewClear(
        viewId,
        BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH,
        0x000000ff, // black
        1.0f,
        0
    );

    bgfx::touch(viewId); // ensure view clears even if nothing submitted

    // Update and draw UI
    viewport.Update();
    if (markupRoot)
    {
        static uint64_t lastTicks = 0;
        const uint64_t now = SDL_GetTicks64();
        const float dt = lastTicks == 0 ? 0.f : std::min(static_cast<float>(now - lastTicks) * 0.001f, 0.1f);
        lastTicks = now;
        markupRoot->SetVar("progress", static_cast<double>(Progress));
        UiAnimation::Tick(dt);
        UiSystem::UpdateRootsUnder(viewport, false);
        viewport.Update();
        UiSystem::FinalizeRootsUnder(viewport);
        UiFontLibrary::FlushUploads();
        UiImageAtlas::Update();
    }
    viewport.FinalizeChildren();
    UiRenderer::BeginPass();
    viewport.Draw(); // must submit bgfx draw calls internally
    UiRenderer::EndPass(viewId);

    // Process events (still needed for SDL)
    SDL_PollEvent(nullptr);

    // Present frame
    UiRenderer::EndFrame();
    UiRenderer::OnBeforeBgfxFrame();
    bgfx::frame();

#ifdef emscripten_sleep

    emscripten_sleep(0); // yield to browser → canvas updates visually
#endif

    ViewIdManager::Reset();

}