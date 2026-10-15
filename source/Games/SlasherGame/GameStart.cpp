#include <Combat/CombatEvents.h>
#include <Control/PlayerController.h>
#include <Characters/CombatCharacter.h>
#include <Debug/SelfTests.h>
#include <PauseGameManager.hpp>
#include <Settings/GameSettings.h>
#include <Settings/InputActionRegistry.h>
#include <UI/LoadingScreen/UiDefaultLoadingScreen.h>
#include <UI/Pause/UiPauseMenu.h>

#include <AssetRegistry.h>
#include <Console/Console.h>
#include <EngineMain.h>
#include <Entity.h>
#include <FileSystem/FileSystem.h>
#include <Input.h>
#include <LoadingScreen/LoadingScreenSystem.h>
#include <Logger.hpp>
#include <SoundSystem/SoundManager.hpp>
#include <UI/RmlUi/RmlUiContext.h>
#include <UI/UiManager.h>

#include <SDL2/SDL.h>

#include <memory>
#include <string>

// Engine bootstrap entity ("gamestart"): input actions, settings, shared banks, combat event registration,
// pause handling and the startup map. Persistent across levels.
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
		pendingStartupMap = true;

		LoadingScreenSystem::SetLoadingCanvas(std::make_shared<UiDefaultLoadingScreen>());

		if (initialized)
			return;
		initialized = true;

		UiRenderer::AddFallbackFont("GameData/fonts/NotoSans.ttf");
		UiRenderer::AddFallbackFont("GameData/fonts/NotoSerifJP.ttf");
		UiRenderer::AddFallbackFont("GameData/fonts/NotoSansSC.ttf");

		RegisterInputActions();

		GameSettings::Instance().LoadFromFile();
		GameSettings::Instance().ApplyAll();

		SoundManager::LoadBankFromPath("GameData/sounds/banks/Desktop/SFX.bank");
		SoundManager::LoadBankFromPath("GameData/sounds/banks/Desktop/Weapons.bank");

		RegisterCombatEventTypes();

#ifndef DISTRIBUTION
		if (HasArgument("combat_selftest"))
			RunSelfTestsAndQuit();
#endif
	}

	void Update() override
	{
		UpdatePause();

		UiManager::UiScale = Input::IsScrenTouched ? 1.3f : 1.0f;

		if (pendingStartupMap && !AssetRegistry::HasPendingWork())
		{
			pendingStartupMap = false;
			Level::LoadLevelFromFile(GetStartupMap());
		}

#ifndef DISTRIBUTION
		UpdatePerfCrowd();
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

	static bool HasArgument(const std::string& name)
	{
		return EngineMain::MainInstance->Arguments.find(name) != EngineMain::MainInstance->Arguments.end();
	}

	static std::string GetStartupMap()
	{
		auto map = EngineMain::MainInstance->Arguments.find("map");
		if (map != EngineMain::MainInstance->Arguments.end() && !map->second.empty())
			return "GameData/maps/" + map->second[0];
		return "GameData/maps/example.bsp";
	}

	static void RegisterInputActions()
	{
		Input::AddAction("forward")->AddKeyboardKey(SDL_KeyCode::SDLK_w);
		Input::AddAction("backward")->AddKeyboardKey(SDL_KeyCode::SDLK_s);
		Input::AddAction("left")->AddKeyboardKey(SDL_KeyCode::SDLK_a);
		Input::AddAction("right")->AddKeyboardKey(SDL_KeyCode::SDLK_d);
		Input::AddAction("jump")->AddKeyboardKey(SDL_KeyCode::SDLK_SPACE)->AddButton(GamepadButton::A);
		Input::AddAction("dash")->AddKeyboardKey(SDL_KeyCode::SDLK_LSHIFT)->AddButton(GamepadButton::B);

		Input::AddAction("attack")->AddMouseButton(MouseButton::Left)->AddButton(GamepadButton::X);
		Input::AddAction("attack2")->AddMouseButton(MouseButton::Right)->AddButton(GamepadButton::RightTrigger);
		Input::AddAction("finisher")->AddKeyboardKey(SDL_KeyCode::SDLK_e)->AddButton(GamepadButton::Y);

		Input::AddAction("interact")->AddKeyboardKey(SDL_KeyCode::SDLK_f);
		Input::AddAction("possess")->AddKeyboardKey(SDL_KeyCode::SDLK_q)->AddButton(GamepadButton::LeftShoulder);

#if __EMSCRIPTEN__
		Input::AddAction("pause")->AddKeyboardKey(SDL_KeyCode::SDLK_BACKQUOTE)->AddButton(GamepadButton::Back);
#else
		Input::AddAction("pause")->AddKeyboardKey(SDL_KeyCode::SDLK_ESCAPE)->AddButton(GamepadButton::Start)->AddKeyboardKey(SDL_GetScancodeFromKey(SDLK_AC_BACK));
#endif

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
		InputActionRegistry::Register("jump", "${INPUT_ACTION_JUMP}", "${INPUT_CATEGORY_MOVEMENT}");
		InputActionRegistry::Register("dash", "${INPUT_ACTION_DASH}", "${INPUT_CATEGORY_MOVEMENT}");
		InputActionRegistry::Register("attack", "${INPUT_ACTION_ATTACK}", "${INPUT_CATEGORY_COMBAT}");
		InputActionRegistry::Register("attack2", "${INPUT_ACTION_FIRE}", "${INPUT_CATEGORY_COMBAT}");
		InputActionRegistry::Register("finisher", "${INPUT_ACTION_FINISHER}", "${INPUT_CATEGORY_COMBAT}");
		InputActionRegistry::Register("possess", "${INPUT_ACTION_POSSESS}", "${INPUT_CATEGORY_COMBAT}");
		InputActionRegistry::Register("interact", "${INPUT_ACTION_INTERACT}", "${INPUT_CATEGORY_GENERAL}");
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
	// -perf_crowd <count> [spacing] [types]: a repeatable load for profiling (COMBAT_DESIGN.md §11). Once the startup
	// map is in and the player controls a body: god mode on it, then the perf_crowd console command.
	static void UpdatePerfCrowd()
	{
		static bool done = false;
		const auto argument = EngineMain::MainInstance->Arguments.find("perf_crowd");
		if (done || argument == EngineMain::MainInstance->Arguments.end())
			return;

		PlayerController* player = PlayerController::GetActive();
		CombatCharacter* body = player ? player->GetPossessedBody() : nullptr;
		if (body == nullptr || !body->IsCombatReady() || AssetRegistry::HasPendingWork())
			return;

		done = true;
		std::string command = "perf_crowd";
		for (const std::string& value : argument->second)
			command += " " + value;
		Console::Get().Execute("god");
		Console::Get().Execute(command);
	}

	static void RunSelfTestsAndQuit()
	{
		// Writes need a directory component (the native writer creates the parent directories).
		static constexpr const char* LogPath = "SaveData/combat_selftest.log";

		std::string summary;
		const bool passed = RunCombatSelfTests(summary);
		if (!FileSystemEngine::WriteFile(LogPath, summary))
			Logger::Error("could not write %s", LogPath);
		Logger::Log(passed ? "combat self tests PASSED" : "combat self tests FAILED");

		SDL_Event quit;
		quit.type = SDL_QUIT;
		SDL_PushEvent(&quit);
	}
#endif
};

REGISTER_ENTITY(GameStart, "gamestart")
