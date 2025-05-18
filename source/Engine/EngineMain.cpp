#include "EngineMain.h"

#include "LevelObjectFactory.h"


EngineMain* EngineMain::MainInstance = nullptr;

UiViewport EngineMain::Viewport;

#if __EMSCRIPTEN__

#include "emscripten/emscripten.h"

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

    int width = canvas_get_width();
    int height = canvas_get_height();


    ScreenSize = ivec2(width, height);

#else

    int w, h;

    SDL_GetWindowSize(Window, &w, &h);


    ScreenSize.x = w;
    ScreenSize.y = h;

#endif // __EMSCRIPTEN__
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

    LevelSaveSystem::InitPersistence();
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
    Viewport.FinalizeChildren();

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


    ImStartFrame();

    bool loadedlevel = Level::LoadPendingLevel();

    Level::Current->LoadAssets();

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
    }

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

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    glDisable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0, 1.0);

    MainRenderer->RenderLevel(Level::Current);

    glDisable(GL_DEPTH_TEST);

    Viewport.Update();

    Viewport.Draw();
    UiRenderer::EndFrame();

    Level::Current->DevUiUpdate();

    RenderImGui();

    SDL_GL_SwapWindow(Window);
}
