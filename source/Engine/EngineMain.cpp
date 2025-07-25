#include "EngineMain.h"

#include "LevelObjectFactory.h"
#include "Time.hpp"
#include "Physics.h"
#include "SoundSystem/SoundManager.hpp"
#include "Particle/ParticleEmitter.h"
#include "DebugDraw.hpp"
#include "UI/UiRenderer.h"
#include "LoadingScreen/LoadingScreenSystem.h"
#include "FileSystem/FileSystem.h"

EngineMain* EngineMain::MainInstance = nullptr;

UiViewport EngineMain::Viewport;

#if __EMSCRIPTEN__

#include "emscripten/emscripten.h"
#include "emscripten/html5.h"
#include "FileSystem/FileSystem.h"

EM_JS(int, canvas_get_width, (), {
return canvas.width;
    });
EM_JS(int, canvas_get_height, (), {
  return canvas.height;
    });

#endif // __EMSCRIPTEN__

void EngineMain::UpdateScreenSize()
{
#if __EMSCRIPTEN__

    int width, height;

    EMSCRIPTEN_RESULT result = emscripten_get_canvas_element_size("#canvas", &width, &height);

    if (result == EMSCRIPTEN_RESULT_SUCCESS) 
    {
        ScreenSize = ivec2(width, height);
    }
    else
    {
        printf("failed to get screen resolution\n");
    }



#else

    int w, h;

    SDL_GetWindowSize(Window, &w, &h);


    ScreenSize.x = w;
    ScreenSize.y = h;

#endif // __EMSCRIPTEN__
}

void EngineMain::ToggleFullscreen(SDL_Window* Window)
{

    auto context = SDL_GL_GetCurrentContext();

    Uint32 FullscreenFlag = SDL_WINDOW_FULLSCREEN_DESKTOP;
    bool IsFullscreen = SDL_GetWindowFlags(Window) & FullscreenFlag;

    // Ensure pending GL commands are done
    glFinish();


    // Toggle fullscreen
    SDL_SetWindowFullscreen(Window, IsFullscreen ? 0 : FullscreenFlag);

    // Rebind the context to the window
    SDL_GL_MakeCurrent(Window, context);

    // Ensure double-buffer swap is clean
    SDL_GL_SwapWindow(Window);

    SDL_ShowCursor(SDL_ENABLE);

}

void EngineMain::initGame()
{


    Entity* ent = LevelObjectFactory::instance().create("gamestart");

    if (ent == nullptr)
    {
        Logger::Log("failed to create gamestart entity");
        return;
    }
    Level::Current = new Level();
    Level::Current->AddEntity(ent);
    ent->Start();

    return;

    Level::OpenLevel("GameData/Maps/test2.map");


    for (size_t i = 0; i < 000; i++) 
    {
        float angle = i * 0.1f; // Angle step controls tightness of the spiral
        float radius = 1 + 0.05f * i; // Radius increases over time
        float x = radius * cos(angle) + 10;
        float y = 50;
        float z = radius * sin(angle);

        Entity* ent = LevelObjectFactory::instance().create("testnpc");

        ent->Position = vec3(x, y, z);
        ent->Start();

        Level::Current->AddEntity(ent);
    }

}

void EngineMain::InitInputs()
{

    Input::AddAction("click")->LMB = true;

    int maxUniforms;

    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxUniforms);

    printf("max uniforms: %i \n", maxUniforms);

}

void EngineMain::Init()
{
    UpdateScreenSize();

    printf("init\n");

    MainThreadPool = new ThreadPool();

    MainThreadPool->Start();

    SoundManager::Initialize();

    Time::Init();

    Physics::Init();

    FileSystemEngine::Init();

    MainRenderer = new Renderer();

    UiRenderer::Init();

    ParticleEmitter::InitBilboardVaoIfNeeded();

    InitInputs();

    initGame();
}

void EngineMain::FinishFrame()
{

    Level::Current->RemovePendingEntities();
    Level::Current->MemoryCleanPendingEntities();

    Camera::Update(Time::DeltaTime);
    Level::Current->FinalizeFrame();
    

    NavigationSystem::DrawNavmesh();

    DebugDraw::Finalize();

    UpdateScreenSize();

    Camera::ScreenHeight = ScreenSize.y;

    float AspectRatio = static_cast<float>(ScreenSize.x) / static_cast<float>(ScreenSize.y);
    Camera::AspectRatio = AspectRatio;

}

void EngineMain::MainLoop()
{
    // Wait for game update here


    if (DebugUiEnabled)
    {
        ImStartFrame();
    }
    

    bool loadedlevel = Level::LoadPendingLevel();

    Level::Current->LoadAssets();


    Viewport.ResetTouchInputs();

    for (auto& event : Input::TouchActions)
    {

        auto hit = Viewport.GetHitElementUnderPosition(event.second.position);

        if (hit == nullptr) continue;

        hit->TouchEvents.push_back(event.second);

    }
    Viewport.TouchInputPostProcessing();



    if (asyncGameUpdate)
    {
        FinishFrame();
    }


    Time::Update();

    if (loadedlevel)
    {
        Time::Update();
        Time::DeltaTime = 0.05;
        Time::DeltaTimeF = 0.05f;

        LoadingFrames = 5;

    }

    Input::Update();

    Viewport.Update();
    Viewport.FinalizeChildren();

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
        ToggleFullscreen(Window);

        //Level::OpenLevel("GameData/Maps/test.map");

    }


}

void EngineMain::GameUpdate()
{

    NavigationSystem::Update();
	if (Paused == false)
		Physics::Simulate();
    Physics::Update();

    Level::Current->UpdatePhysics();

    Level::Current->Update(Paused);

    Level::Current->AsyncUpdate(Paused);

    Level::Current->LateUpdate(Paused);

    SoundManager::Update();

}

void EngineMain::Render()
{



    glViewport(0, 0, ScreenSize.x, ScreenSize.y);

    glEnable(GL_DEPTH_TEST);

    //glDisable(GL_POLYGON_OFFSET_FILL);
    //glPolygonOffset(1.0, 1.0);

    MainRenderer->RenderLevel(Level::Current);

    glDisable(GL_DEPTH_TEST);

    Viewport.Draw();
    UiRenderer::EndFrame();

    if (DebugUiEnabled)
    {
        Level::Current->DevUiUpdate();

        RenderImGui();
    }



    if (LoadingFrames > 0)
    {
        LoadingFrames--;

        LoadingScreenSystem::Draw();
        return;
    }

    glFinish();
    glFlush();

    SDL_GL_SwapWindow(Window);
}
