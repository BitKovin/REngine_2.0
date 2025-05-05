#include <SDL2/SDL_video.h>

#include "AssetRegisty.h"

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

    void initDemo();

    void InitInputs();





    void click()
    {
        printf("clicked \n");
    }

	void Init()
	{

        UpdateScreenSize();

        printf("init\n");

        MainThreadPool = new ThreadPool();

        MainThreadPool->Start();

        SoundManager::Initialize();

        Time::Init();

        Physics::Init();

        LevelSaveSystem::InitPersistence();
        MainRenderer = new Renderer();

        UiRenderer::Init();

        ParticleEmitter::InitBilboardVaoIfNeeded();

        InitInputs();

        

        initDemo();

	}

    void FinishFrame()
    {

        Level::Current->RemovePendingEntities();
        Level::Current->MemoryCleanPendingEntities();

        Camera::Update(Time::DeltaTime);
        Level::Current->FinalizeFrame();
        Viewport.FinalizeChildren();

        NavigationSystem::DrawNavmesh();

        DebugDraw::Finalize();

        UpdateScreenSize();

        Camera::ScreenHeight = ScreenSize.y;

        float AspectRatio = static_cast<float>(ScreenSize.x) / static_cast<float>(ScreenSize.y);
        Camera::AspectRatio = AspectRatio;
    }

    // Toggle asynchronous GameUpdate.
    bool asyncGameUpdate = true;

    // Store the future of the async update.
    std::future<void> gameUpdateFuture;

    // Main game loop.
    void MainLoop();


	void GameUpdate()
	{

        NavigationSystem::Update();
        Physics::Simulate();
        Physics::Update();

        Level::Current->UpdatePhysics();

        Level::Current->Update();

        Level::Current->AsyncUpdate();

        Level::Current->LateUpdate();

        SoundManager::Update();

        if (Input::GetAction("test")->Pressed())
        {
            //ToggleFullscreen(window);
            printf("framerate: %f  \n", (1 / Time::DeltaTime));

            Input::LockCursor = !Input::LockCursor;

        }


	}

	void Render()
	{
        glViewport(0, 0, ScreenSize.x, ScreenSize.y);

        // Enable depth testing
        glEnable(GL_DEPTH_TEST);

        glDisable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.0, 1.0);

        MainRenderer->RenderLevel(Level::Current);

        glDisable(GL_DEPTH_TEST);

        Viewport.Update();

        Viewport.Draw();

        Level::Current->DevUiUpdate();

        RenderImGui();

        SDL_GL_SwapWindow(Window);

	}

};