#include <SDL2/SDL_video.h>

#include "AssetRegistry.h"

#include "SoundSystem/SoundManager.hpp"

#include "Time.hpp"
#include "Camera.h"
#include "Input.h"
#include "Physics.h"

#include "Level.hpp"

//#include "Entities/Player.hpp"
//#include "Entities/TestNpc.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "skinned_model.hpp"
#include "animator.hpp"

#include "ShaderManager.h"


#include "ThreadPool.h"

#include <future>

#include <thread>

#include "ImGuiEngineImpl.h"

#include "DebugDraw.hpp"

#include "UI/UiButton.hpp"

#include "UI/UiViewport.hpp"
#include "UI/UiText.hpp"

#include "MapParser.h"

#include "Particle/ParticleEmitter.h"

#include "Renderer/Renderer.h"

#include "SaveSystem/LevelSaveSystem.h"

class EngineMain
{
private:

	StaticMesh* mesh = nullptr;

    Texture* texture = 0;



public:

    Renderer* MainRenderer;

	SDL_Window* Window = nullptr;

	static EngineMain* MainInstance;

    static UiViewport Viewport;

    ivec2 ScreenSize = ivec2();

    bool Paused = true;

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