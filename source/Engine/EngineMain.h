#include <SDL2/SDL_video.h>

#include "StaticMesh.hpp"

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

#include "SkeletalMesh.hpp"

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

    ThreadPool MainThreadPool;

    void ToggleFullscreen(SDL_Window* Window)
    {
        Uint32 FullscreenFlag = SDL_WINDOW_FULLSCREEN_DESKTOP;
        bool IsFullscreen = SDL_GetWindowFlags(Window) & FullscreenFlag;
        SDL_SetWindowFullscreen(Window, IsFullscreen ? 0 : FullscreenFlag);
        SDL_ShowCursor(SDL_ENABLE);
    }

    void initDemo();

    void InitInputs();



    std::shared_ptr<UiButton> img;

    std::shared_ptr<UiText> text;

    void click()
    {
        printf("clicked \n");
    }

	void Init()
	{

        UpdateScreenSize();

        printf("init\n");

        MainThreadPool.Start();

        SoundManager::Initialize();

        Time::Init();

        Physics::Init();

        MainRenderer = new Renderer();

        UiRenderer::Init();

        ParticleEmitter::InitBilboardVaoIfNeeded();

        InitInputs();

        

        initDemo();




        img = make_shared<UiButton>();

        // Define a lambda function
        auto clickHandler = []() {
            std::cout << "Button clicked!" << std::endl;
            };

        // Assign it to the pointer (needs dynamic allocation)
        img->onClick = new std::function<void()>(clickHandler);


        img->position = vec2(100,100);
        img->size = vec2(100);

        text = make_shared<UiText>();

        text->position = vec2(20, -20);

        text->origin = vec2(0, 1);
        text->pivot = vec2(0, 1);

        text->text = "press T to lock/unlock cursor";

        Viewport.AddChild(img);
        Viewport.AddChild(text);

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
    void MainLoop() 
    {

        // Wait for game update here
       

        ImStartFrame();

        if (asyncGameUpdate)
        {
            FinishFrame();
        }
        

        Time::Update();
        Input::Update();
        
        Input::UpdateMouse();

        // Start GameUpdate here, either asynchronously or synchronously.
        if (asyncGameUpdate) {
            // Optionally, check if a previous async GameUpdate is still running.

            // Launch GameUpdate asynchronously.
            gameUpdateFuture = std::async(std::launch::async, &EngineMain::GameUpdate, this);
        }
        else {
            // Run GameUpdate on the main thread.
            GameUpdate();
        }

        if (asyncGameUpdate == false)
        {
            FinishFrame();
        }

        Render();


        if (asyncGameUpdate)
        {
            if (gameUpdateFuture.valid()) {
                // If it's not done yet, wait (or you could choose to skip/warn).
                if (gameUpdateFuture.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
                    gameUpdateFuture.wait();
                }
            }
        }


        if (Input::GetAction("fullscreen")->Pressed())
        {
            //ToggleFullscreen(Window);

            Level::OpenLevel("GameData/Maps/test.map");

        }



    }


	void GameUpdate()
	{

        NavigationSystem::Update();
        Physics::Simulate();

        Level::Current->UpdatePhysics();

        Level::Current->Update();

        Level::Current->AsyncUpdate();

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