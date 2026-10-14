#include "CombatHud.h"

#include <Characters/CombatCharacter.h>
#include <Combat/CombatComponents.h>

#include <cstdio>

namespace
{
	constexpr const char* WhiteTexture = "GameData/textures/generic/white.png";

	std::string Format(const char* format, float a, float b)
	{
		char buffer[64];
		std::snprintf(buffer, sizeof(buffer), format, a, b);
		return buffer;
	}
}

std::shared_ptr<UiProgressBar> CombatHud::MakeBar(const glm::vec2& size, const glm::vec4& color)
{
	auto bar = std::make_shared<UiProgressBar>();
	bar->size = size;
	bar->color = color;
	bar->BackgroundColor = glm::vec4(0.05f, 0.05f, 0.05f, 0.75f);
	bar->ProgressImage = WhiteTexture;
	bar->BackgroundImage = WhiteTexture;
	bar->outlineEnabled = true;
	bar->Progress = 1.0f;
	return bar;
}

std::shared_ptr<UiText> CombatHud::MakeText(float fontSize, const glm::vec4& color)
{
	auto text = std::make_shared<UiText>();
	text->fontSize = fontSize;
	text->color = color;
	text->outlineEnabled = true;
	text->outlineWidth = 2.0f;
	text->outlineColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	return text;
}

CombatHud::CombatHud()
{
	// ---- Controlled body (bottom left) ----
	healthBar = MakeBar(glm::vec2(420.0f, 22.0f), glm::vec4(0.75f, 0.1f, 0.1f, 1.0f));
	healthBar->origin = glm::vec2(0.0f, 1.0f);
	healthBar->pivot = glm::vec2(0.0f, 1.0f);
	healthBar->position = glm::vec2(48.0f, -84.0f);
	AddChild(healthBar);

	poiseBar = MakeBar(glm::vec2(420.0f, 8.0f), glm::vec4(0.85f, 0.75f, 0.3f, 1.0f));
	poiseBar->origin = glm::vec2(0.0f, 1.0f);
	poiseBar->pivot = glm::vec2(0.0f, 1.0f);
	poiseBar->position = glm::vec2(48.0f, -70.0f);
	AddChild(poiseBar);

	healthText = MakeText(30.0f, glm::vec4(1.0f));
	healthText->origin = glm::vec2(0.0f, 1.0f);
	healthText->pivot = glm::vec2(0.0f, 1.0f);
	healthText->position = glm::vec2(48.0f, -112.0f);
	AddChild(healthText);

	effectsText = MakeText(28.0f, glm::vec4(1.0f, 0.85f, 0.4f, 1.0f));
	effectsText->origin = glm::vec2(0.0f, 1.0f);
	effectsText->pivot = glm::vec2(0.0f, 1.0f);
	effectsText->position = glm::vec2(48.0f, -36.0f);
	AddChild(effectsText);

	// ---- Ammo (bottom right) ----
	ammoText = MakeText(40.0f, glm::vec4(0.95f, 0.95f, 0.95f, 1.0f));
	ammoText->origin = glm::vec2(1.0f, 1.0f);
	ammoText->pivot = glm::vec2(1.0f, 1.0f);
	ammoText->position = glm::vec2(-48.0f, -48.0f);
	AddChild(ammoText);

	// ---- Soft target (top center) ----
	targetPanel = std::make_shared<UiElement>();
	targetPanel->origin = glm::vec2(0.5f, 0.0f);
	targetPanel->pivot = glm::vec2(0.5f, 0.0f);
	targetPanel->position = glm::vec2(0.0f, 40.0f);
	targetPanel->size = glm::vec2(520.0f, 80.0f);
	AddChild(targetPanel);

	targetName = MakeText(30.0f, glm::vec4(1.0f));
	targetName->origin = glm::vec2(0.5f, 0.0f);
	targetName->pivot = glm::vec2(0.5f, 0.0f);
	targetPanel->AddChild(targetName);

	targetHealthBar = MakeBar(glm::vec2(520.0f, 14.0f), glm::vec4(0.75f, 0.1f, 0.1f, 1.0f));
	targetHealthBar->origin = glm::vec2(0.5f, 0.0f);
	targetHealthBar->pivot = glm::vec2(0.5f, 0.0f);
	targetHealthBar->position = glm::vec2(0.0f, 40.0f);
	targetPanel->AddChild(targetHealthBar);

	targetPoiseBar = MakeBar(glm::vec2(520.0f, 6.0f), glm::vec4(0.85f, 0.75f, 0.3f, 1.0f));
	targetPoiseBar->origin = glm::vec2(0.5f, 0.0f);
	targetPoiseBar->pivot = glm::vec2(0.5f, 0.0f);
	targetPoiseBar->position = glm::vec2(0.0f, 58.0f);
	targetPanel->AddChild(targetPoiseBar);

	// ---- Center ----
	crosshair = std::make_shared<UiImage>();
	crosshair->origin = glm::vec2(0.5f);
	crosshair->pivot = glm::vec2(0.5f);
	crosshair->size = glm::vec2(6.0f);
	crosshair->color = glm::vec4(1.0f, 1.0f, 1.0f, 0.85f);
	AddChild(crosshair);

	promptText = MakeText(32.0f, glm::vec4(1.0f, 0.9f, 0.5f, 1.0f));
	promptText->origin = glm::vec2(0.5f, 0.5f);
	promptText->pivot = glm::vec2(0.5f, 0.0f);
	promptText->position = glm::vec2(0.0f, 60.0f);
	AddChild(promptText);

	messageText = MakeText(72.0f, glm::vec4(0.9f, 0.15f, 0.15f, 1.0f));
	messageText->origin = glm::vec2(0.5f, 0.35f);
	messageText->pivot = glm::vec2(0.5f, 0.5f);
	AddChild(messageText);
}

void CombatHud::Refresh(const State& state)
{
	CombatCharacter* body = state.body;

	const bool hasBody = body != nullptr;
	healthBar->visible = hasBody;
	poiseBar->visible = hasBody;
	healthText->visible = hasBody;

	if (hasBody)
	{
		healthBar->Progress = body->MaxHealth > 0.0f ? body->Health / body->MaxHealth : 0.0f;
		healthText->text = Format("%.0f / %.0f", body->Health, body->MaxHealth) + (state.possessingOther ? "   (possessed)" : "");

		if (auto poise = body->GetPoise())
		{
			poiseBar->Progress = poise->GetFraction();
			poiseBar->visible = poise->enabled;
		}

		std::string effects;
		if (auto statusEffects = body->GetStatusEffects(); statusEffects && statusEffects->table)
		{
			for (const ActiveStatusEffect& effect : statusEffects->active)
			{
				const StatusEffectData* data = statusEffects->table->Get(effect.index);
				if (!data) continue;
				if (!effects.empty()) effects += "   ";
				effects += (data->hudLabel.empty() ? data->id : data->hudLabel) + Format(" %.0fs", effect.remaining, 0.0f);
			}
		}
		effectsText->text = effects;

		const int magazine = body->GetWeapons().GetMagazine(WeaponSlot::Secondary);
		if (!body->GetWeapons().HasWeapon(WeaponSlot::Secondary))
			ammoText->text = "";
		else if (body->GetWeapons().IsReloading(WeaponSlot::Secondary))
			ammoText->text = "RELOADING";
		else
			ammoText->text = magazine < 0 ? "" : std::to_string(magazine);
	}
	else
	{
		effectsText->text = "";
		ammoText->text = "";
	}

	// Target panel
	auto* target = dynamic_cast<CombatCharacter*>(state.target);
	targetPanel->visible = target != nullptr;
	if (target)
	{
		targetName->text = target->GetData() ? target->GetData()->name : target->Id;
		targetHealthBar->Progress = target->MaxHealth > 0.0f ? target->Health / target->MaxHealth : 0.0f;
		if (auto poise = target->GetPoise())
		{
			targetPoiseBar->Progress = poise->GetFraction();
			targetPoiseBar->color = poise->IsBroken() ? glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) : glm::vec4(0.85f, 0.75f, 0.3f, 1.0f);
		}
	}

	if (state.finisherAvailable)
		promptText->text = "FINISHER";
	else if (state.possessionCandidate)
		promptText->text = "POSSESS";
	else
		promptText->text = "";

	messageText->text = state.message;
}
