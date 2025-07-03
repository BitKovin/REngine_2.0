#pragma once

#include <SDL2/SDL_video.h>

#include "Camera.h"
#include "Input.h"


#include "Level.hpp"


#include "ShaderManager.h"


#include "ThreadPool.h"

#include <future>

#include <thread>

#include "ImGuiEngineImpl.h"

#include "UI/UiViewport.hpp"

#include "MapParser.h"

#include "Renderer/Renderer.h"

#include "SaveSystem/LevelSaveSystem.h"

class EngineMain
{
private:



public:

    Renderer* MainRenderer;

	SDL_Window* Window = nullptr;

	static EngineMain* MainInstance;

    static UiViewport Viewport;

    ivec2 ScreenSize = ivec2();

    bool Paused = false;

    int LoadingFrames = 0;

	EngineMain(SDL_Window* window)
	{
		Window = window;
	}
	~EngineMain()
	{

	}




    void UpdateScreenSize();

    ThreadPool* MainThreadPool;

    void ToggleFullscreen(SDL_Window* Window)
    {
        Uint32 FullscreenFlag = SDL_WINDOW_FULLSCREEN_DESKTOP;
        bool IsFullscreen = SDL_GetWindowFlags(Window) & FullscreenFlag;
        SDL_SetWindowFullscreen(Window, IsFullscreen ? 0 : FullscreenFlag);
        SDL_ShowCursor(SDL_ENABLE);
    }

    void initGame();

    void InitInputs();





    void click()
    {
        printf("clicked \n");
    }

    void Init();

    void FinishFrame();

    // Toggle asynchronous GameUpdate.
    bool asyncGameUpdate = true;

    // Store the future of the async update.
    std::future<void> gameUpdateFuture;

    // Main game loop.
    void MainLoop();


    void GameUpdate();

    void Render();

};