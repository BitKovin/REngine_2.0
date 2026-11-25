#include "UiNpcAwareness.h"

#include <Camera.h>
#include <EngineMain.h>
#include <Level.hpp>
#include <Physics.h>

#include "../../Entities/Npc/NpcBase.h"
#include "../../Entities/Npc/Npc.h"
#include "../../Entities/Player/Player.hpp"

#include <algorithm>
#include <cmath>

namespace
{
	// White while it is only a notice, yellow at Suspect, red at Identify.
	glm::vec4 ColorFor(float value, const NpcMeterTuning& tuning)
	{
		const glm::vec4 white(1.0f, 1.0f, 1.0f, 1.0f);
		const glm::vec4 yellow(1.0f, 0.82f, 0.12f, 1.0f);
		const glm::vec4 red(0.95f, 0.1f, 0.08f, 1.0f);

		if (value >= tuning.identify) return red;

		glm::vec4 color = glm::mix(white, yellow, std::clamp(value / tuning.suspect, 0.0f, 1.0f));
		color.a = 0.45f + 0.55f * std::clamp(value / tuning.suspect, 0.0f, 1.0f);
		return color;
	}
}

void UiNpcAwareness::Refresh(Player* player)
{
	ClearChildren();

	if (player == nullptr || Level::Current == nullptr) return;

	const NpcMeterTuning& tuning = NpcPerceptionTuning::Get().meter;

	const glm::vec2 viewport = EngineMain::Viewport.GetSize();
	const glm::mat4 viewProjection = Camera::projection * Camera::view;
	const glm::vec3 cameraPosition = Camera::position;

	std::vector<NpcBase::AwarenessReading> readings;

	for (LevelObject* object : Level::Current->LevelObjects)
	{
		const NpcBase* npc = dynamic_cast<const NpcBase*>(object);
		if (npc == nullptr || npc->ShowsAwareness() == false) continue;

		float value = 0.0f;
		readings.clear();
		npc->GetAwareness(readings);
		for (const NpcBase::AwarenessReading& reading : readings)
		{
			if (reading.subject == player->Id)
				value = std::max(value, reading.value);
		}

		if (value <= 0.01f) continue;

		const glm::vec3 head = npc->GetAwarenessAnchor() + glm::vec3(0.0f, 0.45f, 0.0f);
		const float distance = glm::distance(head, cameraPosition);

		const glm::vec4 clip = viewProjection * glm::vec4(head, 1.0f);
		const bool inFront = clip.w > 0.05f;
		const glm::vec2 ndc = inFront ? glm::vec2(clip.x, clip.y) / clip.w : glm::vec2(0.0f);

		if (inFront && std::abs(ndc.x) <= 0.97f && std::abs(ndc.y) <= 0.97f)
		{
			// On the screen: shown over NPCs that can be seen, not through walls.
			if (Physics::LineTrace(cameraPosition, head, BodyType::World).hasHit) continue;

			AddMarker(glm::vec2((ndc.x * 0.5f + 0.5f) * viewport.x, (0.5f - ndc.y * 0.5f) * viewport.y), value, distance);
		}
		else if (distance <= ArrowRange)
		{
			// Off the screen: an arrow at the edge toward it (behind the camera the direction flips).
			glm::vec2 direction = inFront ? ndc : -glm::vec2(clip.x, clip.y);
			if (glm::length(direction) < 0.0001f)
				direction = glm::vec2(0.0f, -1.0f);

			AddArrow(viewport, glm::normalize(glm::vec2(direction.x, -direction.y)), value);
		}
	}
}

void UiNpcAwareness::AddMarker(const glm::vec2& screen, float value, float distance)
{
	const NpcMeterTuning& tuning = NpcPerceptionTuning::Get().meter;
	const glm::vec4 color = ColorFor(value, tuning);

	// Smaller with distance, never unreadable.
	const float scale = std::clamp(18.0f / std::max(distance, 1.0f) + 0.55f, 0.6f, 1.2f);

	auto glyph = std::make_shared<UiText>();
	glyph->text = value >= tuning.identify ? "!" : "?";
	glyph->fontSize = 72.0f * scale;
	glyph->pivot = glm::vec2(0.5f, 1.0f);
	glyph->position = screen;
	glyph->color = color;
	glyph->outlineEnabled = true;
	glyph->outlineColor = glm::vec4(0.0f, 0.0f, 0.0f, 0.9f);
	glyph->outlineWidth = 2.0f;
	AddChild(glyph);

	const float barWidth = 54.0f * scale;
	const float barHeight = 6.0f * scale;

	auto barBackground = std::make_shared<UiImage>();
	barBackground->size = glm::vec2(barWidth, barHeight);
	barBackground->pivot = glm::vec2(0.5f, 0.0f);
	barBackground->position = screen + glm::vec2(0.0f, 4.0f);
	barBackground->color = glm::vec4(0.0f, 0.0f, 0.0f, 0.55f);
	AddChild(barBackground);

	auto barFill = std::make_shared<UiImage>();
	barFill->size = glm::vec2(barWidth * std::clamp(value, 0.0f, 1.0f), barHeight);
	barFill->pivot = glm::vec2(0.0f, 0.0f);
	barFill->position = screen + glm::vec2(-barWidth * 0.5f, 4.0f);
	barFill->color = glm::vec4(color.r, color.g, color.b, 1.0f);
	AddChild(barFill);
}

void UiNpcAwareness::AddArrow(const glm::vec2& viewport, const glm::vec2& direction, float value)
{
	const NpcMeterTuning& tuning = NpcPerceptionTuning::Get().meter;

	const glm::vec2 half = viewport * 0.5f - glm::vec2(ArrowMargin);
	const float reachX = std::abs(direction.x) > 0.0001f ? half.x / std::abs(direction.x) : 1.0e9f;
	const float reachY = std::abs(direction.y) > 0.0001f ? half.y / std::abs(direction.y) : 1.0e9f;
	const glm::vec2 position = viewport * 0.5f + direction * std::min(reachX, reachY);

	auto arrow = std::make_shared<UiImage>();
	arrow->ImagePath = "GameData/textures/ui/hud/awareness_arrow.png";
	arrow->size = glm::vec2(56.0f);
	arrow->pivot = glm::vec2(0.5f);
	arrow->position = position;
	arrow->rotation = glm::degrees(std::atan2(direction.x, -direction.y)); // 0 = up, clockwise on a y-down screen
	arrow->color = ColorFor(value, tuning);
	AddChild(arrow);
}
