// Time.hpp first: the engine UI headers pulled in by the pause menu use Time without including it.
#include <Time.hpp>

#include <PauseGameManager.hpp>
#include <Settings/GameSettings.h>
#include <Settings/InputActionRegistry.h>
#include <UI/Pause/UiPauseMenu.h>
#include <Sim/Bridge/SimNavBaker.h>
#include <Sim/Tests/SimSelfTests.h>

#include <AssetRegistry.h>
#include <EngineMain.h>
#include <Entity.h>
#include <FileSystem/FileSystem.h>
#include <Input.h>
#include <Logger.hpp>
#include <UI/RmlUi/RmlUiContext.h>
#include <UI/UiManager.h>
#include <UI/UiRenderer.h>

#include <SDL2/SDL.h>

#include <memory>
#include <string>

// Engine bootstrap entity ("gamestart"): input actions, settings, pause handling and the startup map.
// Persistent across levels.
//
// Launch arguments:
//   -map <file>          startup map inside GameData/maps (default: blank.bsp)
//   -quit_after <secs>   quit after that many seconds of game time; for automated runs (see Docs/README.md)
//   -sim_selftest        run the sim self tests, write SaveData/sim_selftest.log and quit
//   -sim_bake_nav [file] bake the world nav mesh from a manifest (default GameData/sim/world.manifest) and quit
class GameStart : public Entity
{
public:
	GameStart()
	{
		UpdateWhenPaused = true;
		LateUpdateWhenPaused = true;
		PauseGameManager::SetGamePaused(false);
		PauseGameManager::SetGamePausedGameplay(false);
	}

	void Start() override
	{
		Entity::Start();

		// A new gamestart is created with every level; only the first one loads the startup map.
		if (initialized)
			return;
		initialized = true;
		pendingStartupMap = true;

		UiRenderer::AddFallbackFont("GameData/fonts/NotoSans.ttf");
		UiRenderer::AddFallbackFont("GameData/fonts/NotoSerifJP.ttf");
		UiRenderer::AddFallbackFont("GameData/fonts/NotoSansSC.ttf");

		RegisterInputActions();

		GameSettings::Instance().LoadFromFile();
		GameSettings::Instance().ApplyAll();

#ifndef DISTRIBUTION
		if (EngineMain::MainInstance->Arguments.contains("sim_selftest"))
			RunSimSelfTestsAndQuit();

		if (auto bake = EngineMain::MainInstance->Arguments.find("sim_bake_nav"); bake != EngineMain::MainInstance->Arguments.end())
		{
			pendingStartupMap = false; // the baker loads the levels
			SimNavBaker::Start(bake->second.empty() ? SimNavBaker::DefaultManifest : bake->second[0], true);
		}
#endif
	}

	void Update() override
	{
		UpdatePause();

#ifndef DISTRIBUTION
		SimNavBaker::Update();
#endif

		UiManager::UiScale = Input::IsScrenTouched ? 1.3f : 1.0f;

		if (pendingStartupMap && !AssetRegistry::HasPendingWork())
		{
			pendingStartupMap = false;
			Level::LoadLevelFromFile(GetStartupMap());
		}

#ifndef DISTRIBUTION
		UpdateQuitAfter();
#endif
	}

	void Finalize() override
	{
		if (InputAction* toggle = Input::GetAction("debug_ui_toggle"); toggle && toggle->Pressed())
			EngineMain::MainInstance->DebugUiEnabled = !EngineMain::MainInstance->DebugUiEnabled;

		if (InputAction* reload = Input::GetAction("reload_shaders"); reload && reload->Pressed())
		{
			AssetRegistry::ReloadShaders();
			EngineMain::MainInstance->RmlContext->ReloadAllSttyles();
		}
	}

private:
	static inline bool initialized = false;
	bool pendingStartupMap = false;
	bool wasPaused = false;
	std::shared_ptr<UiPauseMenu> pauseMenu;

	static std::string GetStartupMap()
	{
		auto map = EngineMain::MainInstance->Arguments.find("map");
		if (map != EngineMain::MainInstance->Arguments.end() && !map->second.empty())
			return "GameData/maps/" + map->second[0];
		return "GameData/maps/blank.bsp";
	}

	static void RegisterInputActions()
	{
		Input::AddAction("forward")->AddKeyboardKey(SDL_KeyCode::SDLK_w);
		Input::AddAction("backward")->AddKeyboardKey(SDL_KeyCode::SDLK_s);
		Input::AddAction("left")->AddKeyboardKey(SDL_KeyCode::SDLK_a);
		Input::AddAction("right")->AddKeyboardKey(SDL_KeyCode::SDLK_d);
		Input::AddAction("crouch")->AddKeyboardKey(SDL_KeyCode::SDLK_c);
		Input::AddAction("jump")->AddKeyboardKey(SDL_KeyCode::SDLK_SPACE)->AddButton(GamepadButton::A);

		Input::AddAction("attack")->AddMouseButton(MouseButton::Left)->AddButton(GamepadButton::RightTrigger);
		Input::AddAction("attack2")->AddMouseButton(MouseButton::Right)->AddButton(GamepadButton::LeftTrigger);

		Input::AddAction("interact")->AddKeyboardKey(SDL_KeyCode::SDLK_f);

#if __EMSCRIPTEN__
		Input::AddAction("pause")->AddKeyboardKey(SDL_KeyCode::SDLK_BACKQUOTE)->AddButton(GamepadButton::Back);
#else
		Input::AddAction("pause")->AddKeyboardKey(SDL_KeyCode::SDLK_ESCAPE)->AddButton(GamepadButton::Start)->AddKeyboardKey(SDL_GetScancodeFromKey(SDLK_AC_BACK));
#endif

		Input::AddAction("qSave")->AddKeyboardKey(SDL_KeyCode::SDLK_F6);
		Input::AddAction("qLoad")->AddKeyboardKey(SDL_KeyCode::SDLK_F7);

		Input::AddAction("debug_ui_toggle")->AddKeyboardKey(SDL_KeyCode::SDLK_F2);
		Input::AddAction("reload_shaders")->AddKeyboardKey(SDL_KeyCode::SDLK_F3);

		Input::AddAction("ui_confirm")->AddKeyboardKey(SDL_KeyCode::SDLK_RETURN)->AddButton(GamepadButton::A);
		Input::AddAction("ui_cancel")->AddKeyboardKey(SDL_KeyCode::SDLK_ESCAPE)->AddButton(GamepadButton::B)->AddKeyboardKey(SDL_GetScancodeFromKey(SDLK_AC_BACK));
		Input::AddAction("ui_down")->AddKeyboardKey(SDL_KeyCode::SDLK_DOWN)->AddButton(GamepadButton::DPadDown);
		Input::AddAction("ui_up")->AddKeyboardKey(SDL_KeyCode::SDLK_UP)->AddButton(GamepadButton::DPadUp);
		Input::AddAction("ui_left")->AddKeyboardKey(SDL_KeyCode::SDLK_LEFT)->AddButton(GamepadButton::DPadLeft);
		Input::AddAction("ui_right")->AddKeyboardKey(SDL_KeyCode::SDLK_RIGHT)->AddButton(GamepadButton::DPadRight);

		// Rebindable actions (settings menu), in display order.
		InputActionRegistry::Register("forward", "${INPUT_ACTION_FORWARD}", "${INPUT_CATEGORY_MOVEMENT}");
		InputActionRegistry::Register("backward", "${INPUT_ACTION_BACKWARD}", "${INPUT_CATEGORY_MOVEMENT}");
		InputActionRegistry::Register("left", "${INPUT_ACTION_LEFT}", "${INPUT_CATEGORY_MOVEMENT}");
		InputActionRegistry::Register("right", "${INPUT_ACTION_RIGHT}", "${INPUT_CATEGORY_MOVEMENT}");
		InputActionRegistry::Register("crouch", "${INPUT_ACTION_CROUCH}", "${INPUT_CATEGORY_MOVEMENT}");
		InputActionRegistry::Register("jump", "${INPUT_ACTION_JUMP}", "${INPUT_CATEGORY_MOVEMENT}");
		InputActionRegistry::Register("attack", "${INPUT_ACTION_ATTACK}", "${INPUT_CATEGORY_COMBAT}");
		InputActionRegistry::Register("attack2", "${INPUT_ACTION_ATTACK2}", "${INPUT_CATEGORY_COMBAT}");
		InputActionRegistry::Register("interact", "${INPUT_ACTION_INTERACT}", "${INPUT_CATEGORY_GENERAL}");
		InputActionRegistry::Register("qSave", "${INPUT_ACTION_QUICK_SAVE}", "${INPUT_CATEGORY_GENERAL}");
		InputActionRegistry::Register("qLoad", "${INPUT_ACTION_QUICK_LOAD}", "${INPUT_CATEGORY_GENERAL}");
	}

	void UpdatePause()
	{
		if (InputAction* pause = Input::GetAction("pause"); pause && pause->Pressed() && !PauseGameManager::GetGamePaused())
			PauseGameManager::SetGamePaused(true);

		Input::LockCursor = !PauseGameManager::GetGamePaused();
		EngineMain::MainInstance->Paused = PauseGameManager::isGameAnyPaused();

		const bool paused = PauseGameManager::GetGamePaused();
		if (paused && !wasPaused)
		{
			pauseMenu = std::make_shared<UiPauseMenu>();
			EngineMain::MainInstance->Viewport.AddChild(pauseMenu);
		}
		else if (!paused && pauseMenu)
		{
			pauseMenu->RemoveFromParent();
			pauseMenu.reset();
		}
		wasPaused = paused;
	}

#ifndef DISTRIBUTION
	static void RunSimSelfTestsAndQuit()
	{
		// Writes need a directory component (the native writer creates the parent directories).
		static constexpr const char* LogPath = "SaveData/sim_selftest.log";

		std::string summary;
		const bool passed = RunSimSelfTests(summary);
		if (!FileSystemEngine::WriteFile(LogPath, summary))
			Logger::Error("could not write %s", LogPath);
		Logger::Log(passed ? "sim self tests PASSED" : "sim self tests FAILED");

		SDL_Event quit;
		quit.type = SDL_QUIT;
		SDL_PushEvent(&quit);
	}

	// -quit_after <seconds>: a run that ends by itself, so automated and headless runs do not need a kill timeout.
	static void UpdateQuitAfter()
	{
		const auto argument = EngineMain::MainInstance->Arguments.find("quit_after");
		if (argument == EngineMain::MainInstance->Arguments.end() || argument->second.empty())
			return;

		if (Time::GameTimeNoPause < std::stod(argument->second[0]))
			return;

		Logger::Log("quit_after reached");

		SDL_Event quit;
		quit.type = SDL_QUIT;
		SDL_PushEvent(&quit);
	}
#endif
};

REGISTER_ENTITY(GameStart, "gamestart")
