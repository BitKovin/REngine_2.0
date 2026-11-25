#pragma once

#include "malloc_override.h"

#include <SDL2/SDL_video.h>

#include "Camera.h"
#include "Input.h"

#include "Level.hpp"

#include "ShaderManager.h"

#include "ThreadPool.h"

#include <thread>
#include <functional>

#include "ImGuiEngineImpl.h"

#include "UI/UiViewport.hpp"

#include "MapParser.h"

#include "Renderer/Renderer.h"
#include "SaveSystem/GameSaveSystem.h"

#include <FileSystem/FileSystem.h>


class EngineMain
{

public: 

    using AdditionalMouseDeltaCallback = std::function<glm::vec2()>;


private:

    AdditionalMouseDeltaCallback m_AdditionalMouseDeltaCallback;

public:

    void SetAdditionalMouseDeltaCallback(AdditionalMouseDeltaCallback callback)
    {
        m_AdditionalMouseDeltaCallback = std::move(callback);
    }

    glm::vec2 PullAdditionalMouseDelta()
    {
        if (!m_AdditionalMouseDeltaCallback)
            return glm::vec2(0.0f);

        glm::vec2 delta = m_AdditionalMouseDeltaCallback();

        delta = delta / 5.0f * Input::sensitivity * -1.0f;

        if (Input::FlipMouseY)
            delta.y *= -1.0f;

        return delta;
    }

    GameSaveData pendingRestoreSaveData;// for platforms like android we just save and reload game when minimized

	std::shared_ptr<IFileSystem> FileSystem;

    Renderer* MainRenderer;

	SDL_Window* Window = nullptr;

	static EngineMain* MainInstance;

    static UiViewport Viewport;

    ivec2 ScreenSize = ivec2();

    bool Paused = false;

    bool DebugUiEnabled = false;

    int LoadingFrames = 0;

    unsigned long frame = 0;

	bool SimulatingGameTicks = false;

	// A game can take over the per-frame game update (automated tests step the simulation themselves, several times a
	// frame). Called once per main-loop frame before GameUpdate; return true when it did the stepping, and the
	// normal GameUpdate / FinishFrame of this frame are skipped. Rendering still runs.
	std::function<bool()> FrameHook;
	void RunGameStep() { GameUpdate(); }
	void RunFinishFrame() { FinishFrame(); }
    bool SimulatingPreciseGameTicks = false;

	float UiResolutionScale = 1.f;

	RenderTexture* UiRenderTexture = nullptr;
    RenderTexture* UiRenderTextureStencil = nullptr;
    Framebuffer* UiFrameBuffer = nullptr;


	RenderTexture* FinalFrameRenderTexture = nullptr;


	EngineMain(SDL_Window* window)
	{
		Window = window;
	}
    ~EngineMain();


    std::map<std::string, std::vector<std::string>> Arguments;

    void UpdateScreenSize();

    ThreadPool* MainThreadPool = nullptr;
    ThreadPool* GameUpdateSingleThreadPool = nullptr;

    void ToggleFullscreen();

    void initGame();

    void InitInputs();



    void Init(const std::vector<std::string> args = {});

    static std::map<std::string, std::vector<std::string>> ParseCommands(const std::vector<std::string>& args);

    void FinishFrame();

    // Toggle asynchronous GameUpdate.
    bool asyncGameUpdate = true;

    // Main game loop.
    void MainLoop();

	void SimulateGameTick();
	void SimulateGameTicksForTime(float timeToSimulate);
    void SimulateGameTicksForTimeCombinedPrecision(float timeToSimulate);

    void GameUpdate();

    void Render();

	void ForceUpdateScreenSize();

    void FinishRender();

};