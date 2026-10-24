// Character representation tests (SLICE_M2F_CHARACTERS.md): the ring's visibility decision, the animator's clip choice, and the model table
// against the real glb files (bones to hide and clips to play must exist).

#include "SimTestFramework.h"

#ifndef DISTRIBUTION

#include <Characters/BodyVisibility.h>
#include <Characters/CharacterAnimator.h>
#include <Characters/CharacterModels.h>

#include <FileSystem/FileSystem.h>
#include <json.hpp>

#include <cstring>
#include <set>
#include <string>

namespace sim::test
{
	namespace
	{
		void TestVisibility(TestContext& t)
		{
			t.Section("characters: where a body is shown (the ring)");
			BodyVisibility::Input in;
			in.visibleRadius = 45.0f;
			in.hysteresis = 5.0f;

			in.wasShown = false; in.warmedUp = false; in.distance = 60.0f;
			t.Check(!BodyVisibility::Shown(in), "in the ring, not warmed up: warming");
			in.warmedUp = true;
			t.Check(!BodyVisibility::Shown(in), "in the ring, warmed up: still warming");
			in.distance = 45.0f;
			t.Check(BodyVisibility::Shown(in), "exactly at the visible radius, warmed up: shown");
			in.distance = 30.0f; in.warmedUp = false;
			t.Check(!BodyVisibility::Shown(in), "inside the radius but not warmed up: still warming (the caller fades it in if it is in sight)");

			in.wasShown = true; in.warmedUp = true;
			in.distance = 47.0f;
			t.Check(BodyVisibility::Shown(in), "shown, a little beyond the radius: stays shown (hysteresis)");
			in.distance = 50.0f;
			t.Check(BodyVisibility::Shown(in), "shown, at radius + hysteresis: stays shown");
			in.distance = 50.5f;
			t.Check(!BodyVisibility::Shown(in), "shown, beyond radius + hysteresis: back to warming");
			in.distance = 46.0f; in.wasShown = false;
			t.Check(!BodyVisibility::Shown(in), "just sent back (46 m): it does not come straight out again until it is inside the radius");

			// Walking in and out along a line never flickers: one change each way.
			bool shown = false;
			int changes = 0;
			for (float distance = 70.0f; distance > 20.0f; distance -= 0.1f)
			{
				BodyVisibility::Input step;
				step.distance = distance; step.wasShown = shown; step.warmedUp = true;
				const bool now = BodyVisibility::Shown(step);
				changes += now != shown ? 1 : 0;
				shown = now;
			}
			for (float distance = 20.0f; distance < 70.0f; distance += 0.1f)
			{
				BodyVisibility::Input step;
				step.distance = distance; step.wasShown = shown; step.warmedUp = true;
				const bool now = BodyVisibility::Shown(step);
				changes += now != shown ? 1 : 0;
				shown = now;
			}
			t.Check(changes == 2, "walking in and out along a line changes the presentation twice (%d)", changes);
		}

		void TestClipChoice(TestContext& t)
		{
			t.Section("characters: which clips the animator plays");
			const CharacterModel& citizen = CharacterModels::Get(CharacterModels::Citizen);
			const CharacterModel& guard = CharacterModels::Get(CharacterModels::Guard);

			auto name = [](const char* a, const char* b) { return std::strcmp(a, b) == 0; };
			CharacterAnimator::Choice choice = CharacterAnimator::Choose(citizen, glm::vec2(0.0f, 2.4f), false, false);
			t.Check(name(choice.forward, "walk") && name(choice.idle, "idle"), "a citizen walking forward: walk over idle");
			choice = CharacterAnimator::Choose(citizen, glm::vec2(0.0f, 8.0f), false, true);
			t.Check(name(choice.forward, "run"), "running forward: run");
			choice = CharacterAnimator::Choose(citizen, glm::vec2(0.0f, -3.0f), false, true);
			t.Check(name(choice.forward, "run_b"), "running backwards: run_b");
			choice = CharacterAnimator::Choose(citizen, glm::vec2(-3.0f, 0.1f), false, true);
			t.Check(name(choice.side, "run_l"), "running to the left: run_l");
			choice = CharacterAnimator::Choose(citizen, glm::vec2(1.0f, 0.5f), true, false);
			t.Check(name(choice.forward, "crouch_f") && name(choice.side, "crouch_r") && name(choice.idle, "crouch_idle"), "crouched: the crouch set");

			choice = CharacterAnimator::Choose(guard, glm::vec2(0.0f, -3.0f), false, true);
			t.Check(name(choice.forward, "run") && name(choice.idle, "idle"), "a guard has only forward clips: backing up plays run");
			choice = CharacterAnimator::Choose(guard, glm::vec2(0.0f, 1.0f), true, false);
			t.Check(name(choice.forward, "walk") && name(choice.idle, "idle"), "a guard has no crouch clips: crouching plays walk and idle");
		}

		// The clip and bone names of a glb, from its JSON chunk.
		struct GlbNames
		{
			bool loaded = false;
			std::set<std::string> nodes;
			std::set<std::string> animations;
		};

		GlbNames ReadGlb(const std::string& path)
		{
			GlbNames names;
			const std::vector<uint8_t> bytes = FileSystemEngine::ReadFileBinary(path);
			if (bytes.size() < 20 || std::memcmp(bytes.data(), "glTF", 4) != 0)
				return names;
			uint32_t length = 0;
			std::memcpy(&length, bytes.data() + 12, 4);
			if (20 + length > bytes.size())
				return names;
			const auto json = nlohmann::json::parse(bytes.begin() + 20, bytes.begin() + 20 + length, nullptr, false);
			if (json.is_discarded())
				return names;
			for (const auto& node : json.value("nodes", nlohmann::json::array()))
				names.nodes.insert(node.value("name", ""));
			for (const auto& animation : json.value("animations", nlohmann::json::array()))
				names.animations.insert(animation.value("name", ""));
			names.loaded = true;
			return names;
		}

		void TestModelTable(TestContext& t)
		{
			t.Section("characters: the model table against the glb files");
			t.Check(CharacterModels::Count() == 3, "three models (%d)", CharacterModels::Count());
			for (int id = 0; id < CharacterModels::Count(); ++id)
			{
				const CharacterModel& model = CharacterModels::Get(id);
				const GlbNames mesh = ReadGlb(model.mesh);
				const GlbNames clips = ReadGlb(model.clips);
				if (!t.Check(mesh.loaded && clips.loaded, "%s: %s and %s load as glb", model.name, model.mesh, model.clips))
					continue;
				t.Check(!model.firstPersonHidden.empty(), "%s: hides something in first person", model.name);
				for (const char* bone : model.firstPersonHidden)
					t.Check(mesh.nodes.count(bone) == 1, "%s: the mesh has the bone %s to hide", model.name, bone);
				std::vector<const char*> needed = { "idle", "walk", "run" };
				if (model.directional)
					for (const char* clip : { "walk_b", "walk_l", "walk_r", "run_b", "run_l", "run_r", "crouch_idle", "crouch_f", "crouch_b", "crouch_l", "crouch_r" })
						needed.push_back(clip);
				for (const char* clip : needed)
					t.Check(clips.animations.count(clip) == 1, "%s: the clips file has %s", model.name, clip);
				// The clips play on the mesh because they drive bones by name: every bone the first-person body hides must be animated by the clips' skeleton too.
				for (const char* bone : model.firstPersonHidden)
					t.Check(clips.nodes.count(bone) == 1, "%s: the clips' skeleton has the bone %s", model.name, bone);
			}
		}
	}

	void RunCharacterTests(TestContext& context)
	{
		TestVisibility(context);
		TestClipChoice(context);
		TestModelTable(context);
	}
}

#endif
