#pragma once

#include "UiSettingsMenu.h"

#include <PauseGameManager.hpp>

#include <EngineMain.h>
#include <Input.h>
#include <Level.hpp>
#include <UI/UiButton.hpp>
#include <UI/UiCanvas.hpp>
#include <UI/UiImage.hpp>
#include <UI/UiText.hpp>
#include <UI/UiVerticalBox.hpp>

#include <SDL2/SDL.h>

// Pause screen: resume, settings, restart the current level, quit.
class UiPauseMenu : public UiCanvas
{
public:
	UiPauseMenu()
	{
		FocusTrap = true;

		background = std::make_shared<UiImage>();
		background->color = vec4(0.05f, 0.0f, 0.0f, 0.6f);
		AddChild(background);

		optionsBox = std::make_shared<UiVerticalBox>();
		optionsBox->origin = vec2(0.5f);
		optionsBox->pivot = vec2(0.5f);
		AddChild(optionsBox);

		resumeButton = AddButton("${PAUSE_MENU_RESUME}", [this]() { Resume(); });
		AddButton("${PAUSE_MENU_SETTINGS}", [this]()
		{
			EngineMain::MainInstance->Viewport.AddChild(std::make_shared<UiSettingsMenu>(shared_from_this()));
			visible = false;
		});
		AddButton("${PAUSE_MENU_RESTART}", [this]()
		{
			Resume();
			if (Level::Current)
				Level::LoadLevelFromFile(Level::Current->filePath);
		});
		AddButton("${PAUSE_MENU_QUIT}", []()
		{
			SDL_Event quit;
			quit.type = SDL_QUIT;
			SDL_PushEvent(&quit);
		});
	}

	void OnNavCancel() override
	{
		Resume();
	}

	void FinalizeChildren() override
	{
		background->size = GetSize();
		UiCanvas::FinalizeChildren();
	}

private:
	std::shared_ptr<UiImage> background;
	std::shared_ptr<UiVerticalBox> optionsBox;
	std::shared_ptr<UiButton> resumeButton;

	static void Resume()
	{
		PauseGameManager::SetGamePaused(false);
		if (InputAction* pause = Input::GetAction("pause"))
			pause->CleanPressed();
		Input::ReleaseAllActions();
	}

	std::shared_ptr<UiButton> AddButton(const std::string& text, std::function<void()> onClick)
	{
		auto button = std::make_shared<UiButton>();
		auto label = std::make_shared<UiText>();

		button->size = vec2(400, 70);
		label->text = text;
		label->pivot = vec2(0.5f);
		label->origin = vec2(0.5f);

		button->AddChild(label);
		button->outlineEnabled = true;
		button->NineSliceEnabled = true;
		button->NineSlice = UiRenderer::NineSliceMargins(16, 16, 16, 16);
		button->ImagePath = "GameData/textures/ui/test_button.png";
		button->shadowEnabled = true;
		button->shadowOffset = vec2(2, 4);
		button->onClick = std::move(onClick);

		optionsBox->AddChild(button);
		return button;
	}
};
