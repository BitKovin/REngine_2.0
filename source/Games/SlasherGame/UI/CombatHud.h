#pragma once

#include <UI/UiCanvas.hpp>
#include <UI/UiImage.hpp>
#include <UI/UiProgressBar.hpp>
#include <UI/UiText.hpp>

#include <memory>
#include <string>

class CombatCharacter;
class Entity;

// Player HUD: the controlled body's health / poise / effects / ammo, the soft target's health / poise with a
// finisher prompt, the possession prompt and a center message. Owned by PlayerController, refreshed in its
// LateUpdate (main thread).
class CombatHud : public UiCanvas
{
public:
	CombatHud();

	struct State
	{
		CombatCharacter* body = nullptr;
		Entity* target = nullptr;
		bool finisherAvailable = false;
		bool possessionCandidate = false;
		bool possessingOther = false;
		std::string message;
	};

	void Refresh(const State& state);

private:
	std::shared_ptr<UiProgressBar> healthBar;
	std::shared_ptr<UiProgressBar> poiseBar;
	std::shared_ptr<UiText> healthText;
	std::shared_ptr<UiText> effectsText;
	std::shared_ptr<UiText> ammoText;
	std::shared_ptr<UiText> weaponsText;

	std::shared_ptr<UiElement> targetPanel;
	std::shared_ptr<UiText> targetName;
	std::shared_ptr<UiProgressBar> targetHealthBar;
	std::shared_ptr<UiProgressBar> targetPoiseBar;

	std::shared_ptr<UiText> promptText;
	std::shared_ptr<UiText> messageText;
	std::shared_ptr<UiImage> crosshair;

	static std::shared_ptr<UiProgressBar> MakeBar(const glm::vec2& size, const glm::vec4& color);
	static std::shared_ptr<UiText> MakeText(float fontSize, const glm::vec4& color);
};
