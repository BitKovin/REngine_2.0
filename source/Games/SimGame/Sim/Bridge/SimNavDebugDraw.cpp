#include "SimNavDebugDraw.h"

#ifndef DISTRIBUTION

#include "SimWorldNavFile.h"

#include <Sim/Nav/WorldNavDebug.h>

#include <Time.hpp>

#include <Camera.h>
#include <Console/ConsoleRegister.h>
#include <DebugDraw.hpp>
#include <Level.hpp>
#include <Logger.hpp>

#include <memory>

namespace SimNavDebugDraw
{
	namespace
	{
		constexpr float DrawRadius = 100.0f; // the engine's own nav draw uses the same range

		struct State
		{
			bool enabled = false;
			std::unique_ptr<sim::nav::WorldNav> nav;
			std::string shownMap;                  // map the cached segments belong to
			std::vector<sim::nav::DebugSegment> segments; // whole level, level-local
		};

		State state;

		uint32_t ColourOf(sim::nav::DebugKind kind)
		{
			using sim::nav::DebugKind;
			switch (kind)
			{
			case DebugKind::Edge: return DebugColor::RGBA(60, 90, 160);
			case DebugKind::Border: return DebugColor::RGBA(90, 200, 255);
			case DebugKind::LinkStart: return DebugColor::Green;
			case DebugKind::LinkStartBroken: return DebugColor::Red;
			case DebugKind::LinkEnd: return DebugColor::Yellow;
			}
			return DebugColor::White;
		}

		void Rebuild(const std::string& map)
		{
			state.shownMap = map;
			state.segments.clear();
			const int level = SimWorldNavFile::FindLevel(state.nav->Data(), map);
			if (level < 0)
			{
				Logger::Warning("sim_nav_draw: %s is not a level of %s", map.c_str(), SimWorldNavFile::Path);
				return;
			}
			sim::nav::CollectDebugSegments(*state.nav, level, glm::vec3(0.0f), 0.0f, state.segments);
			Logger::Log("sim_nav_draw: %s: %zu segments", map.c_str(), state.segments.size());
		}
	}

	void SetEnabled(bool enabled)
	{
		if (!enabled)
		{
			state.enabled = false;
			state.shownMap.clear();
			return;
		}
		if (!state.nav)
		{
			sim::nav::WorldNavData data;
			std::string error;
			auto nav = std::make_unique<sim::nav::WorldNav>();
			if (!SimWorldNavFile::Load(data, error) || !nav->Load(data, error))
			{
				Logger::Error("sim_nav_draw: cannot load the world nav: %s", error.c_str());
				return;
			}
			SimWorldNavFile::CheckFresh(data);
			state.nav = std::move(nav);
		}
		state.enabled = true;
		state.shownMap.clear();
	}

	bool IsEnabled() { return state.enabled; }

	void Update()
	{
		if (!state.enabled || !Level::Current)
			return;
		if (state.shownMap != Level::Current->filePath)
			Rebuild(Level::Current->filePath);

		const float duration = Time::DeltaTimeF / 2;
		const glm::vec3 centre = Camera::position;
		for (const auto& segment : state.segments)
		{
			const bool marker = segment.kind != sim::nav::DebugKind::Edge && segment.kind != sim::nav::DebugKind::Border;
			if (!marker)
			{
				const glm::vec2 a(segment.a.x - centre.x, segment.a.z - centre.z), b(segment.b.x - centre.x, segment.b.z - centre.z);
				if (glm::dot(a, a) > DrawRadius * DrawRadius && glm::dot(b, b) > DrawRadius * DrawRadius)
					continue;
			}
			DebugDraw::Line(segment.a, segment.b, duration, 0.01f, ColourOf(segment.kind));
		}
	}
}

CONSOLE_FUNC("sim_nav_draw", "Draw the loaded level's part of the world nav mesh and portal links: sim_nav_draw [on|off] (default: toggle)")
{
	const bool enable = args.empty() ? !SimNavDebugDraw::IsEnabled() : args[0] != "off" && args[0] != "0";
	SimNavDebugDraw::SetEnabled(enable);
	Logger::Log("sim_nav_draw: %s", SimNavDebugDraw::IsEnabled() ? "on" : "off");
}

#endif
