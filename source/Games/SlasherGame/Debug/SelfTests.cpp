#include "SelfTests.h"

#ifndef DISTRIBUTION

#include <Actions/ComboResolver.h>
#include <Characters/CombatCharacter.h>
#include <Combat/CombatComponents.h>
#include <Combat/CombatRules.h>
#include <Combat/Spring.h>
#include <Control/CameraImpact.h>
#include <Data/CharacterData.h>
#include <Data/CombatData.h>
#include <Data/MoveSetData.h>
#include <Data/WeaponData.h>
#include <Settings/GameplaySettingsData.h>

#include <FileSystem/FileSystem.h>
#include <Helpers/StringHelper.h>
#include <Logger.hpp>
#include <MathHelper.hpp>

#include <algorithm>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <functional>

namespace
{
	struct TestContext
	{
		int passed = 0;
		int failed = 0;
		std::string failures;

		void Check(bool condition, const char* format, ...)
		{
			if (condition)
			{
				++passed;
				return;
			}

			++failed;
			char buffer[512];
			va_list args;
			va_start(args, format);
			std::vsnprintf(buffer, sizeof(buffer), format, args);
			va_end(args);
			failures += "  FAIL: ";
			failures += buffer;
			failures += "\n";
		}
	};

	const char* DecisionName(ComboDecision d)
	{
		switch (d)
		{
		case ComboDecision::Light: return "Light";
		case ComboDecision::Heavy: return "Heavy";
		case ComboDecision::Hold:  return "Hold";
		default:                   return "None";
		}
	}

	// ---- Combo resolver -------------------------------------------------------------------------------
	// Simulates frames at 60 Hz with clip time == game time (speed 1, no hit-stop). `held(t)` is the physical
	// button state at clip time t. Returns the first non-None decision and the clip time it happened at.
	struct ComboRun
	{
		ComboDecision decision = ComboDecision::None;
		float time = -1.0f;
	};

	ComboRun RunCombo(const ComboTiming& timing, bool heldAtStart, float inputBuffer, const std::function<bool(float)>& held,
		bool holdCarriesOver = true, float endTime = 1.6f)
	{
		CharacterInput input;
		input.SetButton(InputButton::Attack, true, -0.1);         // the press that started the attack
		if (!heldAtStart)
			input.SetButton(InputButton::Attack, false, -0.05);

		ComboResolver resolver;
		resolver.Begin(input.Get(InputButton::Attack), holdCarriesOver);

		const float step = 1.0f / 60.0f;
		for (float t = 0.0f; t <= endTime; t += step)
		{
			input.BeginFrame();
			input.SetButton(InputButton::Attack, held(t), t);
			resolver.Observe(input.Get(InputButton::Attack), timing, t);
			const ComboDecision decision = resolver.Evaluate(timing, t, t, inputBuffer);
			if (decision != ComboDecision::None)
				return { decision, t };
		}
		return {};
	}

	// Hold opener outside a combo window (COMBAT_DESIGN.md §5.4): a tap starts the opener on release, a hold the launcher.
	void TestHoldOpener(TestContext& test)
	{
		CharacterInput input;
		input.SetButton(InputButton::Attack, true, 1.0);
		const ButtonState& attack = input.Get(InputButton::Attack);
		test.Check(DecideOpener(attack, 1.05, 0.1f) == OpenerDecision::Wait, "a press held shorter than holdDelay starts nothing yet");
		test.Check(DecideOpener(attack, 1.12, 0.1f) == OpenerDecision::Hold, "a press held for holdDelay starts the hold opener");
		test.Check(DecideOpener(attack, 1.3, 0.6f) == OpenerDecision::Wait, "inside a move the longer holdDelayInMove applies");
		input.SetButton(InputButton::Attack, false, 1.06);
		test.Check(DecideOpener(input.Get(InputButton::Attack), 1.06, 0.1f) == OpenerDecision::Opener, "a press released early starts the opener");
		CharacterInput tapped;
		tapped.Tap(InputButton::Attack, 2.0);
		test.Check(DecideOpener(tapped.Get(InputButton::Attack), 2.0, 0.1f) == OpenerDecision::Opener, "a tap within one frame starts the opener");
		CharacterInput late;
		late.SetButton(InputButton::Attack, true, 3.0);
		late.SetButton(InputButton::Attack, false, 3.7);
		test.Check(DecideOpener(late.Get(InputButton::Attack), 3.75, 0.6f) == OpenerDecision::Hold,
			"released after holding for the delay (the launcher could not start yet): still the hold, never the opener");
	}

	void TestComboResolver(TestContext& test)
	{
		ComboTiming timing;
		timing.windowStart = 0.4f;
		timing.alternativeAt = 0.6f;
		timing.windowEnd = 0.9f;
		timing.hasLight = true;
		timing.hasHeavy = true;
		const float buffer = 0.4f;
		const float frame = 1.0f / 60.0f + 1e-4f;

		auto between = [](float a, float b) { return [a, b](float t) { return t >= a && t < b; }; };
		auto never = [](float) { return false; };
		auto always = [](float) { return true; };
		auto at = [&](const ComboRun& r, ComboDecision decision, float time) { return r.decision == decision && r.time >= time && r.time < time + frame; };

		// Where no launcher can start: lights on release, heavies at once (held through a / pressed late).
		ComboRun r = RunCombo(timing, false, buffer, between(0.20f, 0.25f));
		test.Check(at(r, ComboDecision::Light, 0.4f), "spam before window -> light at windowStart (got %s at %.3f)", DecisionName(r.decision), r.time);
		r = RunCombo(timing, false, 0.1f, between(0.02f, 0.05f));
		test.Check(r.decision == ComboDecision::None, "stale tap is ignored (got %s at %.3f)", DecisionName(r.decision), r.time);
		r = RunCombo(timing, false, buffer, between(0.45f, 0.50f));
		test.Check(at(r, ComboDecision::Light, 0.5f), "tap in light zone -> light on release (got %s at %.3f)", DecisionName(r.decision), r.time);
		r = RunCombo(timing, false, buffer, between(0.45f, 0.80f));
		test.Check(at(r, ComboDecision::Heavy, 0.6f), "held through alternativeAt -> heavy at alternativeAt (got %s at %.3f)", DecisionName(r.decision), r.time);
		r = RunCombo(timing, false, buffer, between(0.58f, 0.62f));
		test.Check(at(r, ComboDecision::Heavy, 0.6f), "a press still held when the clip reaches alternativeAt is heavy (got %s at %.3f)", DecisionName(r.decision), r.time);
		r = RunCombo(timing, false, buffer, between(0.70f, 0.72f));
		test.Check(at(r, ComboDecision::Heavy, 0.7f), "press in heavy zone -> heavy on press (got %s at %.3f)", DecisionName(r.decision), r.time);
		r = RunCombo(timing, false, buffer, between(0.45f, 5.0f));
		test.Check(at(r, ComboDecision::Heavy, 0.6f), "a long hold without a launcher is a heavy, never a hold (got %s at %.3f)", DecisionName(r.decision), r.time);
		r = RunCombo(timing, true, buffer, always);
		test.Check(at(r, ComboDecision::Heavy, 0.6f), "hold carried over from the previous attack -> heavy (got %s at %.3f)", DecisionName(r.decision), r.time);
		r = RunCombo(timing, true, buffer, between(-1.0f, 0.3f));
		test.Check(r.decision == ComboDecision::None, "carried hold released before alternativeAt -> nothing (got %s at %.3f)", DecisionName(r.decision), r.time);
		r = RunCombo(timing, true, buffer, always, false);
		test.Check(r.decision == ComboDecision::None, "without holdCarriesOver the starting hold counts for nothing (got %s)", DecisionName(r.decision));

		// Where the launcher can start: a heavy waits for its release (a longer hold becomes the launcher instead).
		ComboTiming withHold = timing;
		withHold.hasHold = true;
		withHold.holdDelay = 0.6f;
		r = RunCombo(withHold, false, buffer, between(0.45f, 0.50f));
		test.Check(at(r, ComboDecision::Light, 0.5f), "launcher available: a tap in the light zone is still a light on release (got %s at %.3f)", DecisionName(r.decision), r.time);
		r = RunCombo(withHold, false, buffer, between(0.50f, 0.75f));
		test.Check(at(r, ComboDecision::Heavy, 0.75f), "launcher available: a burst held across alternativeAt -> heavy on release (got %s at %.3f)", DecisionName(r.decision), r.time);
		r = RunCombo(withHold, false, buffer, between(0.70f, 0.72f));
		test.Check(at(r, ComboDecision::Heavy, 0.72f), "launcher available: press in heavy zone -> heavy on release (got %s at %.3f)", DecisionName(r.decision), r.time);
		r = RunCombo(withHold, false, buffer, between(0.85f, 1.0f));
		test.Check(at(r, ComboDecision::Heavy, 1.0f), "a burst near the window end released after it still gets its heavy (got %s at %.3f)", DecisionName(r.decision), r.time);
		r = RunCombo(withHold, false, buffer, between(0.45f, 1.0f));
		test.Check(at(r, ComboDecision::Heavy, 1.0f), "released just short of holdDelay -> its heavy, no launch (got %s at %.3f)", DecisionName(r.decision), r.time);
		r = RunCombo(withHold, false, buffer, between(0.45f, 5.0f));
		test.Check(r.decision == ComboDecision::Hold && r.time >= 1.05f - 1e-3f && r.time < 1.05f + 2.0f * frame,
			"held for holdDelay -> launcher, no heavy before it (got %s at %.3f)", DecisionName(r.decision), r.time);
		r = RunCombo(withHold, false, buffer, between(0.05f, 5.0f));
		test.Check(r.decision == ComboDecision::Hold && r.time >= 0.65f - 1e-3f && r.time < 0.65f + 2.0f * frame,
			"a hold started early becomes the launcher once both the window is open and holdDelay passed (got %s at %.3f)", DecisionName(r.decision), r.time);

		// The link stays armed while a press that may become the launcher is held, at most holdDelay.
		{
			CharacterInput input;
			ComboResolver resolver;
			resolver.Begin(input.Get(InputButton::Attack), true);
			input.SetButton(InputButton::Attack, true, 0.5);
			resolver.Observe(input.Get(InputButton::Attack), withHold, 0.5f);
			test.Check(resolver.IsArmed(withHold, 0.9) && !resolver.IsArmed(withHold, 1.15), "a held press keeps the link open, at most holdDelay");
			test.Check(!resolver.IsArmed(timing, 0.9), "without a launcher nothing waits for a release");
			input.SetButton(InputButton::Attack, false, 0.7);
			resolver.Observe(input.Get(InputButton::Attack), withHold, 0.7f);
			test.Check(!resolver.IsArmed(withHold, 0.75), "released, the link is not held open any more");
		}

		r = RunCombo(timing, false, buffer, never);
		test.Check(r.decision == ComboDecision::None, "no input -> none (got %s)", DecisionName(r.decision));
		r = RunCombo(timing, false, buffer, between(0.95f, 1.0f));
		test.Check(r.decision == ComboDecision::None, "press after windowEnd -> none (got %s)", DecisionName(r.decision));
		r = RunCombo(withHold, false, buffer, between(0.95f, 5.0f));
		test.Check(r.decision == ComboDecision::None, "a hold made after windowEnd is not this link's (got %s)", DecisionName(r.decision));

		// Single-branch links: any qualifying input takes the existing branch.
		ComboTiming lightOnly = timing;
		lightOnly.hasHeavy = false;
		r = RunCombo(lightOnly, false, buffer, between(0.70f, 0.72f));
		test.Check(at(r, ComboDecision::Light, 0.7f), "light-only link, heavy-zone press -> light (got %s at %.3f)", DecisionName(r.decision), r.time);
		ComboTiming heavyOnly = timing;
		heavyOnly.hasLight = false;
		r = RunCombo(heavyOnly, false, buffer, between(0.45f, 0.50f));
		test.Check(at(r, ComboDecision::Heavy, 0.5f), "heavy-only link, tap -> heavy on release (got %s at %.3f)", DecisionName(r.decision), r.time);
	}

	// ---- Reactions / poise / effects / tokens ---------------------------------------------------------

	void TestReactions(TestContext& test)
	{
		using CombatRules::SelectReaction;
		test.Check(SelectReaction(HitReaction::Flinch, false, 0.0f, false) == HitReaction::Flinch, "flinch unarmored");
		test.Check(SelectReaction(HitReaction::Flinch, true, 0.0f, false) == HitReaction::Stagger, "heavy upgrades flinch to stagger");
		test.Check(SelectReaction(HitReaction::Flinch, true, 0.3f, false) == HitReaction::None, "armor absorbs heavy flinch");
		test.Check(SelectReaction(HitReaction::Launch, false, 0.5f, false) == HitReaction::Stagger, "armor downgrades launch");
		test.Check(SelectReaction(HitReaction::Stagger, false, 0.5f, false) == HitReaction::Flinch, "armor downgrades stagger");
		test.Check(SelectReaction(HitReaction::Knockdown, false, 1.0f, false) == HitReaction::None, "hyper armor absorbs everything");
		test.Check(SelectReaction(HitReaction::Flinch, true, 0.0f, true) == HitReaction::None, "broken body ignores flinch");
		test.Check(SelectReaction(HitReaction::Launch, false, 0.0f, true) == HitReaction::Launch, "broken body can be launched");
		test.Check(CombatRules::PoiseBreakReaction(HitReaction::Launch) == HitReaction::Launch
			&& CombatRules::PoiseBreakReaction(HitReaction::Knockdown) == HitReaction::Knockdown, "a launcher that breaks poise still launches");
		test.Check(CombatRules::PoiseBreakReaction(HitReaction::Flinch) == HitReaction::Stagger, "any other poise break staggers");
		test.Check(CombatRules::ScalePoiseDamage(10.0f, DamageSource::Firearm, 0.3f) == 3.0f, "firearm poise scale");
		test.Check(CombatRules::ScalePoiseDamage(10.0f, DamageSource::Melee, 0.3f) == 10.0f, "melee poise unscaled");
	}

	void TestPoise(TestContext& test)
	{
		PoiseData data;
		data.max = 40.0f;
		data.regenDelay = 1.0f;
		data.regenRate = 10.0f;
		data.breakDuration = 2.0f;

		PoiseComponent poise;
		poise.Configure(data);
		test.Check(!poise.ApplyDamage(30.0f, false) && poise.current == 10.0f, "poise damage below max does not break");
		poise.Tick(0.5f);
		test.Check(poise.current == 10.0f, "no regen during regen delay");
		poise.Tick(0.6f);
		poise.Tick(1.0f);
		test.Check(poise.current > 10.0f, "regen after delay (got %.2f)", poise.current);

		poise.Configure(data);
		poise.armorResist = 0.5f;
		poise.ApplyDamage(20.0f, false);
		test.Check(poise.current == 30.0f, "armor halves poise damage (got %.2f)", poise.current);
		poise.ApplyDamage(20.0f, true);
		test.Check(poise.current == 10.0f, "ignoreArmor bypasses armor (got %.2f)", poise.current);
		test.Check(poise.ApplyDamage(15.0f, true) && poise.IsBroken(), "poise breaks at zero");
		test.Check(!poise.ApplyDamage(100.0f, true), "a broken body takes no further poise damage");
		poise.Tick(2.1f);
		test.Check(!poise.IsBroken() && poise.current == poise.max, "broken state ends and poise refills");
	}

	void TestStatusEffects(TestContext& test)
	{
		auto table = std::make_shared<StatusEffectTable>();
		StatusEffectData boost;
		boost.duration = 2.0f;
		boost.stacking = EffectStacking::Refresh;
		boost.modifiers.damageDealt = 1.5f;
		table->effects["boost"] = boost;

		StatusEffectData stack;
		stack.duration = 5.0f;
		stack.stacking = EffectStacking::Stack;
		stack.maxStacks = 3;
		stack.modifiers.moveSpeed = 0.9f;
		table->effects["slow"] = stack;

		DataLoadReport report;
		test.Check(ResolveStatusEffects(*table, report), "status effect table resolves");

		StatusEffectsComponent effects;
		effects.table = table;
		test.Check(effects.Add("boost") && effects.modifiers.damageDealt == 1.5f, "effect applies modifiers");
		effects.Tick(1.5f);
		effects.Add("boost");
		effects.Tick(1.0f);
		test.Check(effects.Has(table->Find("boost")), "refresh resets duration");
		effects.Add("slow");
		effects.Add("slow");
		effects.Add("slow");
		effects.Add("slow");
		test.Check(std::abs(effects.modifiers.moveSpeed - 0.9f * 0.9f * 0.9f) < 1e-4f, "stacks multiply up to maxStacks (got %.4f)", effects.modifiers.moveSpeed);
		effects.Tick(2.5f);
		test.Check(!effects.Has(table->Find("boost")) && effects.modifiers.damageDealt == 1.0f, "expired effect is removed");
		test.Check(!effects.Add("missing"), "unknown effect id is rejected");
	}

	// Hit-stop stops the body at the contact, holds and eases back over its last share (COMBAT_DESIGN.md §6.3).
	void TestHitStop(TestContext& test)
	{
		CombatStateComponent state;
		test.Check(state.GetTimeDilation() == 1.0f, "no hit-stop: normal speed");

		state.ApplyHitStop(0.1f, 0.03f);
		test.Check(std::abs(state.GetTimeDilation() - 0.03f) < 1e-4f, "the contact frame is already stopped (got %.3f)", state.GetTimeDilation());
		state.Tick(0.06f);
		test.Check(std::abs(state.GetTimeDilation() - 0.03f) < 1e-4f, "holds until the ease-out (got %.3f)", state.GetTimeDilation());
		state.Tick(0.025f);
		const float easing = state.GetTimeDilation();
		test.Check(easing > 0.03f && easing < 1.0f, "eases back over the last share (got %.3f)", easing);
		state.ApplyHitStop(0.05f, 0.05f);
		test.Check(std::abs(state.GetTimeDilation() - 0.03f) < 1e-4f && std::abs(state.hitStopDuration - 0.05f) < 1e-4f,
			"a hit during a hit-stop stops the body again at the deeper scale");
		state.Tick(0.2f);
		test.Check(state.GetTimeDilation() == 1.0f && state.hitStopDuration == 0.0f, "hit-stop ends at normal speed");

		state.ApplyShake(glm::vec3(1.0f, 0.0f, 0.0f), 0.05f, 0.12f);
		test.Check(state.GetShakeOffset().x > 0.04f, "the contact frame shows the victim displaced along the blow");
		state.Tick(0.01f);
		test.Check(glm::length(state.GetShakeOffset()) > 0.0f, "victim shakes during its hit-stop");
		state.Tick(0.2f);
		test.Check(glm::length(state.GetShakeOffset()) == 0.0f, "shake ends");
	}

	// A launch survives its hit-stop (COMBAT_DESIGN.md §5.8): the blocked-rise check measures in the dilation the speed
	// was applied with.
	void TestLaunchRise(TestContext& test)
	{
		// Launched at 10.5 m/s during a 0.03 hit-stop: the physics kept 0.315 m/s - nothing blocked it, whatever this
		// frame's dilation is (the ease-out jumps it toward 1).
		test.Check(CombatCharacter::ResolveRise(10.5f, 0.03f, 0.315f) == 10.5f, "a launch inside a hit-stop is not taken for a ceiling");
		test.Check(CombatCharacter::ResolveRise(8.0f, 1.0f, 0.0f) == 0.0f, "a rise the capsule could not make is adopted (ceiling)");
		test.Check(std::abs(CombatCharacter::ResolveRise(8.0f, 1.0f, 3.0f) - 3.0f) < 1e-5f, "a partly blocked rise keeps what the capsule made");
		test.Check(CombatCharacter::ResolveRise(2.0f, 0.03f, 0.0f) == 2.0f, "a rise too slow for the controller to leave the ground is not judged");
		test.Check(CombatCharacter::ResolveRise(-3.0f, 1.0f, 0.0f) == -3.0f, "falling is left alone");
	}

	// Air combat vertical motion (COMBAT_DESIGN.md §5.8): rising keeps the arc, falling hangs and sinks slowly.
	void TestAirCombat(TestContext& test)
	{
		const AirCombatData air;
		const float gravity = 21.0f;
		const float frame = 1.0f / 60.0f;
		test.Check(std::abs(CombatCharacter::StepAirCombatVertical(5.0f, gravity, frame, air) - (5.0f - gravity * frame)) < 1e-5f,
			"rising keeps the normal arc (a launch reaches its top)");
		test.Check(std::abs(CombatCharacter::StepAirCombatVertical(-1.0f, gravity, frame, air) - (-1.0f - gravity * air.fallGravityScale * frame)) < 1e-5f,
			"falling, gravity is scaled down");

		float vertical = -12.0f;
		float previous = vertical;
		bool smooth = true;
		for (int i = 0; i < 60; ++i)
		{
			vertical = CombatCharacter::StepAirCombatVertical(vertical, gravity, frame, air);
			smooth = smooth && vertical - previous < 2.0f;   // no snap: eases over frames
			previous = vertical;
		}
		test.Check(smooth && std::abs(vertical + air.maxFallSpeed) < 0.05f, "a fast fall eases to maxFallSpeed (got %.2f)", vertical);

		float hang = 0.0f;
		for (int i = 0; i < 60; ++i)
			hang = CombatCharacter::StepAirCombatVertical(hang, gravity, frame, air);
		test.Check(hang < 0.0f && hang >= -air.maxFallSpeed - 1e-4f, "from the top of the arc the body sinks slowly (%.2f m/s after 1 s)", hang);
	}

	// Procedural feedback spring (COMBAT_DESIGN.md §9.4): a kick peaks where asked and settles.
	void TestSpring(TestContext& test)
	{
		DampedSpring spring;
		spring.Kick(glm::vec3(1.0f, 0.0f, 0.0f), 7.0f, 0.5f);
		float peak = 0.0f;
		float peakTime = 0.0f;
		for (int i = 1; i <= 60; ++i)
		{
			spring.Step(1.0f / 60.0f, 7.0f, 0.5f);
			if (spring.value.x > peak)
			{
				peak = spring.value.x;
				peakTime = i / 60.0f;
			}
		}
		test.Check(std::abs(peak - 1.0f) < 0.05f && peakTime < 0.06f, "a kick peaks at its size within a few frames (%.3f at %.3f s)", peak, peakTime);
		DampedSpring once;
		DampedSpring stepped;
		once.Kick(glm::vec3(1.0f), 7.0f, 0.5f);
		stepped.Kick(glm::vec3(1.0f), 7.0f, 0.5f);
		once.Step(0.1f, 7.0f, 0.5f);
		for (int i = 0; i < 10; ++i)
			stepped.Step(0.01f, 7.0f, 0.5f);
		test.Check(glm::length(once.value - stepped.value) < 1e-4f, "the spring does not depend on the frame rate");
		for (int i = 0; i < 120; ++i)
			spring.Step(1.0f / 30.0f, 7.0f, 0.5f);
		test.Check(spring.IsSettled(), "the kick settles (also at a low frame rate)");

		spring.Displace(glm::vec3(0.0f, 0.0f, 3.0f), 0.5f);
		test.Check(std::abs(spring.value.z - 0.5f) < 1e-5f, "a displacement is clamped to its limit");
	}

	// 360-degree melee targeting (COMBAT_DESIGN.md §5.7).
	void TestAttackTargeting(TestContext& test)
	{
		TargetingData settings;
		const float range = 6.0f;
		auto score = [&](float angle, float distance, bool previous = false)
		{
			return CombatRules::ScoreAttackTarget(angle, distance, range, previous, settings);
		};
		test.Check(score(0.0f, 3.0f) < score(60.0f, 2.0f), "the body closest to the input direction wins over a slightly closer one");
		test.Check(score(30.0f, 1.0f) < score(0.0f, 5.5f), "a close body near the direction wins over a far one dead ahead");
		test.Check(score(170.0f, 2.0f) < 1e30f && score(170.0f, 2.0f) > score(90.0f, 2.0f), "bodies behind are candidates, scored last");
		test.Check(score(20.0f, 2.0f, true) < score(0.0f, 2.0f), "the previous target wins a near tie");
		test.Check(score(90.0f, 2.0f, true) > score(10.0f, 2.0f), "pointing at another body switches the target");
	}

	// Camera impact comfort (COMBAT_DESIGN.md §9.4): bounded view speed, nothing piling up, everything settles.
	void TestCameraImpact(TestContext& test)
	{
		const PlayerCameraImpactData settings;
		test.Check(settings.KickPeakSpeed() <= PlayerCameraImpactData::MaxKickSpeed && settings.ShakePeakSpeed() <= PlayerCameraImpactData::MaxShakeSpeed
			&& settings.kickFrequency <= PlayerCameraImpactData::MaxFrequency && settings.shakeFrequency <= PlayerCameraImpactData::MaxFrequency,
			"default impact settings are within the comfort limits (kick %.0f, rumble %.0f deg/s)", settings.KickPeakSpeed(), settings.ShakePeakSpeed());

		const float frame = 1.0f / 60.0f;
		auto run = [&](CameraImpact& impact, int frames, const std::function<void(int)>& before)
		{
			float fastest = 0.0f;
			glm::vec2 previous = impact.GetRotation();
			for (int i = 0; i < frames; ++i)
			{
				before(i);
				impact.Update(frame);
				fastest = std::max(fastest, glm::length(impact.GetRotation() - previous) / frame);
				previous = impact.GetRotation();
			}
			return fastest;
		};

		// Ten maximal kicks on consecutive frames (a burst) never turn the view faster than one.
		CameraImpact burst;
		burst.Configure(settings);
		const float burstSpeed = run(burst, 60, [&](int i) { if (i < 10) burst.Kick(glm::vec2(1.0f, 0.0f), settings.maxKick); });
		test.Check(burstSpeed <= settings.KickPeakSpeed() * 1.01f, "stacked kicks stay under the speed of one maximal kick (%.0f deg/s)", burstSpeed);
		run(burst, 60, [](int) {});
		test.Check(glm::length(burst.GetRotation()) < 1e-3f && glm::length(burst.GetOffset()) < 1e-4f, "kicks settle");

		CameraImpact sideways;
		sideways.Configure(settings);
		sideways.Kick(glm::vec2(1.0f, 0.0f), 1.0f);
		sideways.Update(frame * 2.0f);
		test.Check(sideways.GetRotation().y > 0.0f && sideways.GetRotation().x < 0.0f && sideways.GetOffset().x < 0.0f,
			"a sideways blow moves the image along it and down; the camera slides against it");

		// Rumble: the strongest trauma wins, the speed stays bounded, it ends.
		CameraImpact rumble;
		rumble.Configure(settings);
		rumble.Shake(1.0f);
		rumble.Shake(0.4f);
		test.Check(rumble.GetTrauma() == 1.0f, "rumble takes the strongest trauma instead of adding up");
		const float rumbleSpeed = run(rumble, 90, [](int) {});
		test.Check(rumbleSpeed <= settings.ShakePeakSpeed() * 1.01f, "rumble stays under its speed bound (%.0f deg/s)", rumbleSpeed);
		test.Check(rumble.GetTrauma() == 0.0f && glm::length(rumble.GetRotation()) == 0.0f && glm::length(rumble.GetOffset()) == 0.0f,
			"rumble ends after 1 / shakeDecay seconds");

		// The validation keeps shipped tuning comfortable.
		auto parse = [](const char* text, DataLoadReport& report)
		{
			PlayerConfigData config;
			nlohmann::json json;
			return ParseDataJson(text, json, report) && ConvertDataJson(json, config, "", report) && ResolvePlayerConfig(config, report);
		};
		DataLoadReport fast;
		test.Check(!parse(R"({ "camera": { "impact": { "shakeFrequency": 18 } } })", fast), "rejects a rumble above 12 Hz (blur at 60 fps)");
		DataLoadReport ringing;
		test.Check(!parse(R"({ "camera": { "impact": { "kickDamping": 0.2 } } })", ringing), "rejects a kick that rings");
		DataLoadReport wild;
		test.Check(parse(R"({ "camera": { "impact": { "shakeAngle": 2.5 } } })", wild) && !wild.Warnings.empty(),
			"warns about a rumble turning the view too fast");

		// The player's Camera Shake setting (scales all of the above) round-trips through the settings file, clamped.
		GameplaySettingsData gameplay;
		test.Check(gameplay.CameraShake == GameplaySettingsData::MaxCameraShake, "camera shake defaults to full");
		gameplay.CameraShake = 0.35f;
		const std::string saved = gameplay.Serialize();
		GameplaySettingsData loaded;
		const size_t equals = saved.find('=');
		loaded.ApplyLine(saved.substr(0, equals), saved.substr(equals + 1));
		test.Check(std::abs(loaded.CameraShake - 0.35f) < 1e-4f, "camera shake survives a save / load (%s)", saved.c_str());
		loaded.ApplyLine("CameraShake", "4");
		test.Check(loaded.CameraShake == GameplaySettingsData::MaxCameraShake, "camera shake is clamped on load");
		loaded.ApplyLine("CameraShake", "loud");
		test.Check(loaded.CameraShake == GameplaySettingsData::MaxCameraShake, "a malformed camera shake keeps the current value");
	}

	void TestAttackTokens(TestContext& test)
	{
		AttackTokensComponent tokens;
		AttackTokenCapacityData capacity;
		capacity.melee = 2;
		tokens.Configure(capacity);

		const EntityHandle a = EntityHandle::FromNetworked(9001);
		const EntityHandle b = EntityHandle::FromNetworked(9002);
		const EntityHandle c = EntityHandle::FromNetworked(9003);

		test.Check(tokens.TryAcquire(a, AttackToken::Melee, 0.0, 5.0f), "first melee token");
		test.Check(tokens.TryAcquire(b, AttackToken::Melee, 0.0, 5.0f), "second melee token");
		test.Check(!tokens.TryAcquire(c, AttackToken::Melee, 0.0, 5.0f), "capacity reached");
		test.Check(tokens.TryAcquire(a, AttackToken::Melee, 1.0, 5.0f) && tokens.CountHeld(AttackToken::Melee) == 2, "renewal does not take a second token");
		tokens.Release(a);
		test.Check(tokens.TryAcquire(c, AttackToken::Melee, 1.0, 5.0f), "released token is available");
		test.Check(tokens.TryAcquire(c, AttackToken::None, 1.0, 5.0f), "None never needs a token");
	}

	// ---- Moveset parsing ------------------------------------------------------------------------------

	bool ParseMoveSetText(const char* text, MoveSetData& out, DataLoadReport& report)
	{
		nlohmann::json json;
		return ParseDataJson(text, json, report) && ConvertDataJson(json, out, "", report) && ResolveMoveSet(out, report);
	}

	void TestMoveSetData(TestContext& test)
	{
		const char* valid = R"({
			"version": 1, "name": "test", "animations": ["a.glb"],
			"combos": { "Attack": "light_1" },
			"holdOpeners": { "Attack": "launch" },
			"actions": { "Dash": "dash", "Secondary": "fire", "Draw": "draw" },
			"reactions": { "Flinch": "flinch" },
			"moves": {
				"light_1": { "clip": "attack_a", "heavy": false, "tags": ["Attack", "Melee"],
					"entry": { "afterHeavy": { "blendIn": 0.2, "startTime": 0.1 }, "afterHold": { "blendIn": 0.1, "startTime": 0.08 } },
					"hits": [ { "start": 0.2, "end": 0.3, "damage": 10 } ], "reach": { "length": 0.5 },
					"combo": { "windowStart": 0.4, "alternativeAt": 0.6, "windowEnd": 0.9, "next": "light_2", "alternative": "heavy_2" },
					"cancels": [ { "start": 0.4, "into": ["Dash"] }, { "start": 0.6, "into": ["Locomotion"] } ] },
				"light_2": { "clip": "attack_b", "tags": ["Attack"], "recovery": { "start": 0.5, "lowerBodyBlend": 0.2 } },
				"heavy_2": { "clip": "attack_b", "speed": 0.7, "heavy": true, "tags": ["Attack"],
					"combo": { "windowStart": 0.3, "alternativeAt": 0.5, "windowEnd": 0.7, "next": "light_2" },
					"recovery": { "enabled": false } },
				"dash": { "behavior": "Dash", "clip": "", "endTime": 0.4, "tags": ["Dash"], "params": { "distance": 5 } },
				"flinch": { "behavior": "Reaction", "clip": "hurt", "tags": ["Reaction"] },
				"fire": { "behavior": "Fire", "clip": "fire_r", "variants": ["fire_l"], "channel": "Overlay", "bodyMask": "UpperBody",
					"tags": ["Fire"], "params": { "restartOnShot": true } },
				"draw": { "behavior": "Draw", "clip": "draw", "channel": "Overlay", "bodyMask": "Bones", "maskBones": ["clavicle_r"],
					"params": { "weapon": "Main" } },
				"launch": { "clip": "launch", "heavy": true, "tags": ["Attack"], "hold": { "until": 0.25, "release": "light_1" },
					"hits": [ { "start": 0.27, "end": 0.37, "reaction": "Launch", "launch": 10 } ], "reach": { "length": 0.8, "start": 0.2, "blendIn": 0 },
					"params": { "launchSelf": { "onHit": true, "verticalSpeed": 9.6, "requireHeld": false } } },
				"air_1": { "clip": "attack_a", "tags": ["Attack"], "bodyMask": "NoLegs", "startState": "Air",
					"hits": [ { "start": 0.1, "end": 0.2, "air": { "stall": 0.0 } }, { "start": 0.3, "end": 0.45 } ],
					"tracking": { "until": 0.05, "advance": 2 },
					"params": { "air": { "matchTargetHeight": false }, "plunge": { "start": 0.3, "speed": 15, "hold": 0.4 } } }
			}
		})";

		MoveSetData data;
		DataLoadReport report;
		const bool ok = ParseMoveSetText(valid, data, report);
		test.Check(ok, "valid moveset loads (%s)", report.Errors.empty() ? "" : report.Errors.front().c_str());
		if (!ok)
			return;

		const MoveData* light1 = data.GetMove(data.FindMove("light_1"));
		test.Check(light1 && light1->combo.nextIndex == data.FindMove("light_2"), "combo next resolves to an index");
		test.Check(light1 && light1->combo.alternativeIndex == data.FindMove("heavy_2"), "combo alternative resolves to an index");
		test.Check(light1 && light1->entry.afterHeavy.startTime == 0.1f && light1->entry.fresh.startTime == 0.0f, "entry variants parse with defaults");
		test.Check(light1 && light1->cancels[0].intoMask == TagBit(MoveTag::Dash), "cancel tags resolve to a mask");
		test.Check(light1 && light1->cancels[1].intoMask == TagBit(MoveTag::Locomotion), "a Locomotion cancel resolves");
		test.Check(light1 && light1->hits[0].camera == 0.0f && light1->hits[0].impactScale == 1.0f && light1->hits[0].hitStopScale < 0.05f,
			"hits default to no camera kick, a full hit effect and a near-freeze hit-stop");
		test.Check(data.actionBuffer > 0.0f && data.actionBuffer < data.inputBuffer, "actions buffer shorter than attacks by default");
		test.Check(data.GetComboOpener(ComboSlot::Attack) == data.FindMove("light_1"), "combo opener resolves");
		const MoveData* launch = data.GetMove(data.FindMove("launch"));
		test.Check(data.GetHoldOpener(ComboSlot::Attack) == data.FindMove("launch") && data.GetHoldOpener(ComboSlot::AirAttack) < 0 && data.holdDelay > 0.0f,
			"hold opener resolves per slot, holdDelay defaults");
		test.Check(launch && launch->hold.IsValid() && launch->hold.releaseIndex == data.FindMove("light_1"), "hold release resolves to a move");
		test.Check(light1 && light1->entry.afterHold.startTime == 0.08f && light1->entry.Get(MoveEntryKind::AfterHold).blendIn == 0.1f,
			"the afterHold entry parses");
		test.Check(data.GetMove(data.FindMove("heavy_2"))->HasTag(MoveTag::Heavy), "heavy moves get the Heavy tag");
		test.Check(data.GetMove(data.FindMove("dash"))->animationIndex == -1, "clip-less move has no animation");
		const MoveData* fire = data.GetMove(data.FindMove("fire"));
		test.Check(fire && fire->GetClipCount() == 2 && fire->GetClipHash(1) == hashed_string("fire_l") && fire->GetClipHash(2) == fire->clipHash,
			"clip variants index after the clip and wrap");
		test.Check(data.GetActionMove(ActionSlot::Draw) == data.FindMove("draw"), "Draw action resolves");
		const MoveData* draw = data.GetMove(data.FindMove("draw"));
		test.Check(draw && draw->bodyMask == BodyMask::Bones && draw->maskBoneHashes.size() == 1 && draw->maskBoneHashes[0] == hashed_string("clavicle_r"),
			"bone mask resolves its bones");
		test.Check(light1 && light1->tracking.resolvedUntil == 0.2f && light1->tracking.resolvedMagnetUntil == 0.3f,
			"tracking turns until the first strike and pulls until the last one ends by default");
		test.Check(light1 && light1->tracking.startTurnRate > light1->tracking.turnRate && light1->tracking.startTurnTime > 0.0f,
			"attacks turn faster at their start by default");
		const MoveData* air = data.GetMove(data.FindMove("air_1"));
		test.Check(air && air->tracking.resolvedUntil == 0.05f && air->tracking.resolvedMagnetUntil == 0.45f && air->tracking.advance == 2.0f,
			"explicit tracking times win, the magnet still ends with the last strike");
		test.Check(air && air->bodyMask == BodyMask::NoLegs, "NoLegs body mask parses");
		test.Check(light1 && light1->airLegs && data.GetOpenerSlot(false) == ComboSlot::Attack,
			"ground moves take the air pose on the legs in the air by default; without an AirAttack opener the air uses Attack's");
		test.Check(air && air->hits[0].air.stall == 0.0f && air->hits[1].air.stall > 0.0f && air->hits[1].hitStopScale > 0.0f,
			"hit air data parses with defaults");
		test.Check(light1 && light1->reach.resolvedStart == 0.2f && light1->reach.resolvedEnd == 0.3f && light1->reach.weapon == WeaponSlot::Main,
			"reach is fully out over the blade hits by default");
		test.Check(light1 && light1->reach.At(0.25f) == 0.5f && std::abs(light1->reach.At(0.2f - 0.06f) - 0.25f) < 1e-4f
			&& light1->reach.At(0.0f) == 0.0f && light1->reach.At(0.3f + 0.2f) == 0.0f && std::abs(light1->reach.At(0.4f) - 0.25f) < 1e-4f,
			"reach grows over blendIn before start and shrinks over blendOut after end");
		test.Check(launch && launch->reach.resolvedStart == 0.2f && launch->reach.resolvedEnd == 0.37f
			&& launch->reach.At(0.19f) == 0.0f && launch->reach.At(0.2f) == 0.8f, "an explicit reach start wins; blendIn 0 snaps out");
		test.Check(!data.GetMove(data.FindMove("heavy_2"))->reach.IsActive() && data.GetMove(data.FindMove("heavy_2"))->reach.At(0.5f) == 0.0f,
			"moves keep the weapon's length by default");
		test.Check(light1 && light1->recoveryStart == 0.9f, "recovery starts at combo.windowEnd by default");
		test.Check(data.GetMove(data.FindMove("light_2"))->recoveryStart == 0.5f, "explicit recovery.start without a combo link");
		test.Check(data.GetMove(data.FindMove("light_2"))->recovery.lowerBodyBlend == 0.2f
			&& data.GetMove(data.FindMove("light_2"))->recovery.upperBodyBlend == 0.35f, "recovery blends parse with defaults");
		test.Check(data.GetMove(data.FindMove("heavy_2"))->recoveryStart < 0.0f, "disabled recovery");
		test.Check(data.GetMove(data.FindMove("dash"))->recoveryStart < 0.0f && data.GetMove(data.FindMove("flinch"))->recoveryStart < 0.0f,
			"moves without a combo link or recovery.start do not recover");
		test.Check(report.Warnings.empty(), "valid moveset has no warnings (%s)", report.Warnings.empty() ? "" : report.Warnings.front().c_str());

		// Round trip: serialize, parse again, serialize again -> identical JSON.
		const nlohmann::json first = data;
		MoveSetData reparsed;
		DataLoadReport roundTripReport;
		test.Check(ParseMoveSetText(first.dump().c_str(), reparsed, roundTripReport), "serialized moveset parses again");
		test.Check(nlohmann::json(reparsed) == first, "moveset round trip is lossless");

		auto expectError = [&](const char* text, const char* what)
		{
			MoveSetData broken;
			DataLoadReport brokenReport;
			test.Check(!ParseMoveSetText(text, broken, brokenReport), "rejects %s", what);
		};

		expectError(R"({ "animations": ["a.glb"], "moves": { "m": { "clip": "c", "combo": { "windowStart": 0.4, "alternativeAt": 0.6, "windowEnd": 0.9, "next": "nope" } } } })", "unknown combo target");
		expectError(R"({ "animations": ["a.glb"], "moves": { "m": { "clip": "c", "combo": { "windowStart": 0.6, "alternativeAt": 0.4, "windowEnd": 0.9, "next": "m" } } } })", "unordered combo window");
		expectError(R"({ "animations": ["a.glb"], "moves": { "m": { "clip": "c", "tags": ["Atack"] } } })", "unknown tag");
		expectError(R"({ "animations": ["a.glb"], "moves": { "m": { "clip": "c", "behavior": "Fly" } } })", "unknown behaviour");
		expectError(R"({ "animations": ["a.glb"], "moves": { "m": { "clip": "" } } })", "clip-less move without endTime");
		expectError(R"({ "animations": ["a.glb"], "combos": { "Kick": "m" }, "moves": { "m": { "clip": "c" } } })", "unknown combo slot");
		expectError(R"({ "animations": ["a.glb"], "moves": { "m": { "clip": "c", "behavior": "Dash", "params": { "distance": -1 } } } })", "invalid behaviour params");
		expectError(R"({ "animations": ["a.glb"], "moves": { "m": { "clip": "c", "speed": "fast" } } })", "type mismatch");
		expectError(R"({ "animations": ["a.glb"], "moves": { "m": { "clip": "c", "combo": { "windowStart": 0.4, "alternativeAt": 0.6, "windowEnd": 0.9, "next": "m" }, "recovery": { "start": 0.5 } } } })", "recovery before combo.windowEnd");
		expectError(R"({ "animations": ["a.glb"], "moves": { "m": { "clip": "", "endTime": 0.5, "recovery": { "start": 0.2 } } } })", "recovery without a clip");
		expectError(R"({ "animations": ["a.glb"], "moves": { "m": { "clip": "", "endTime": 0.5, "variants": ["b"], "recovery": { "enabled": false } } } })", "clip variants without a clip");
		expectError(R"({ "animations": ["a.glb"], "moves": { "m": { "clip": "c", "channel": "Overlay", "recovery": { "start": 0.2 } } } })", "recovery on an overlay move");
		expectError(R"({ "animations": ["a.glb"], "moves": { "m": { "clip": "c", "bodyMask": "Bones" } } })", "a bone mask without bones");
		expectError(R"({ "animations": ["a.glb"], "moves": { "m": { "clip": "c", "hits": [ { "start": 0, "end": 0.1, "hitStopScale": 0 } ] } } })", "a hit-stop that would freeze");
		expectError(R"({ "animations": ["a.glb"], "moves": { "m": { "clip": "c", "tags": ["Locomotion"] } } })", "Locomotion as a move tag");
		expectError(R"({ "animations": ["a.glb"], "moves": { "m": { "clip": "c", "hits": [ { "start": 0, "end": 0.1, "camera": -1 } ] } } })", "a negative camera kick");
		expectError(R"({ "animations": ["a.glb"], "actionBuffer": -0.1, "moves": { "m": { "clip": "c" } } })", "a negative action buffer");
		expectError(R"({ "animations": ["a.glb"], "moves": { "m": { "clip": "c", "hold": { "until": 0.2 } } } })", "a hold without a release move");
		expectError(R"({ "animations": ["a.glb"], "moves": { "m": { "clip": "c", "hold": { "until": 0.2, "release": "nope" } } } })", "a hold releasing into an unknown move");
		expectError(R"({ "animations": ["a.glb"], "holdOpeners": { "AirAttack": "m" }, "moves": { "m": { "clip": "c" } } })", "a hold opener without a combo opener in its slot");
		expectError(R"({ "animations": ["a.glb"], "moves": { "m": { "clip": "c", "reach": { "length": 0.5 } } } })", "a reach without times or blade hits");
		expectError(R"({ "animations": ["a.glb"], "moves": { "m": { "clip": "c", "hits": [ { "start": 0, "end": 0.1, "bone": "foot_r" } ], "reach": { "length": 0.5 } } } })",
			"a reach timed by a bone-sphere hit");
		expectError(R"({ "animations": ["a.glb"], "moves": { "m": { "clip": "c", "hits": [ { "start": 0, "end": 0.1 } ], "reach": { "length": -0.5 } } } })", "a negative reach");
		expectError(R"({ "animations": ["a.glb"], "moves": { "m": { "clip": "c", "reach": { "length": 0.5, "start": 0.4, "end": 0.2 } } } })", "a reach ending before it starts");
		expectError(R"({ "animations": ["a.glb"], "holdDelay": -1, "moves": { "m": { "clip": "c" } } })", "a negative hold delay");
		expectError(R"({ "animations": ["a.glb"], "moves": { "m": { "clip": "c", "params": { "launchSelf": { "onHit": true, "verticalSpeed": 0 } } } } })",
			"an on-hit self launch without speed");
		expectError(R"({ "animations": ["a.glb"], "moves": { "m": { "clip": "c", "params": { "plunge": { "start": 0.3, "hold": 0.2 } } } } })",
			"a plunge holding its strike before it dives");
		expectError(R"({ "animations": ["a.glb"], "moves": { "m": { "clip": "c", "behavior": "Fire", "channel": "Overlay", "params": { "moveSpeed": 2 } } } })", "fire speeding the body up");
		expectError(R"({ "animations": ["a.glb"], "moves": { "m": { "clip": "c", "behavior": "Dash", "params": { "tailLocomotion": 2 } } } })", "dash tail above run speed");
		expectError(R"({ "animations": ["a.glb"], "moves": { "m": { "clip": "c", "behavior": "Glide", "params": { "holdDelay": -1 } } } })", "negative glide hold delay");

		MoveSetData early;
		DataLoadReport earlyReport;
		ParseMoveSetText(R"({ "animations": ["a.glb"], "moves": { "m": { "clip": "c", "hold": { "until": 0.3, "release": "m" },
			"hits": [ { "start": 0.2, "end": 0.3 } ] } } })", early, earlyReport);
		test.Check(earlyReport.Ok() && !earlyReport.Warnings.empty(), "a hit window opening before the hold commits warns");

		MoveSetData steering;
		DataLoadReport steeringReport;
		ParseMoveSetText(R"({ "animations": ["a.glb"], "moves": { "m": { "clip": "c",
			"combo": { "windowStart": 0.4, "alternativeAt": 0.6, "windowEnd": 0.9, "next": "m" }, "cancels": [ { "start": 0.45, "into": ["Locomotion"] } ] } } })",
			steering, steeringReport);
		test.Check(steeringReport.Ok() && !steeringReport.Warnings.empty(), "a Locomotion cancel inside the light zone warns");

		MoveSetData typo;
		DataLoadReport typoReport;
		ParseMoveSetText(R"({ "animations": ["a.glb"], "moves": { "m": { "clip": "c", "sped": 2 } } })", typo, typoReport);
		test.Check(!typoReport.Warnings.empty(), "unknown key produces a warning");
	}
}

namespace
{
	// ---- Moveset editor: behaviour params are edited with all their defaults, stored without them (§11) -----------

	void TestParamsEditing(TestContext& test)
	{
		MoveData move;
		move.behavior = "Attack";
		move.params = nlohmann::json::parse(R"({ "plunge": { "start": 0.24, "speed": 18 } })");

		nlohmann::json all = MoveParamsWithDefaults(move);
		test.Check(all.contains("launchSelf") && all.contains("locomotion") && all["plunge"]["hold"].get<float>() == -1.0f,
			"the editor sees every parameter of the behaviour, defaults included");
		test.Check(all["plunge"]["speed"].is_number_float() && all["plunge"]["speed"].get<float>() == 18.0f,
			"a float parameter written as 18 stays a float");
		test.Check(MoveParamsToStore(all, move) == move.params, "nothing edited: the params block stays as written");

		all["launchSelf"]["onHit"] = true;
		all["plunge"]["start"] = -1.0f;
		const nlohmann::json stored = MoveParamsToStore(all, move);
		test.Check(stored["launchSelf"].size() == 1 && stored["launchSelf"]["onHit"] == true && !stored.contains("locomotion"),
			"an edited parameter is stored alone, defaults are not written out");
		test.Check(stored["plunge"]["start"].get<float>() == -1.0f && stored["plunge"].contains("speed"),
			"a written parameter stays written, even at its default value");
	}

	// ---- Debug-panel save (COMBAT_DESIGN.md §10.4): the authored file keeps everything but the edited values -----

	void TestMoveSetSave(TestContext& test)
	{
		const std::string authored =
			"{\n"
			"\t\"version\": 1,\n"
			"\t\"name\": \"saved\",\n"
			"\t\"animations\": [\"a.glb\"],\n"
			"\t\"inputBuffer\": 0.4,\n"
			"\t\"combos\": { \"Attack\": \"a\" },\n"
			"\t\"moves\": {\n"
			"\t\t\"a\": {\n"
			"\t\t\t\"_note\": \"keep me\",\n"
			"\t\t\t\"clip\": \"c\",\n"
			"\t\t\t\"tags\": [\"Attack\", \"Melee\"],\n"
			"\t\t\t\"entry\": { \"fresh\": { \"blendIn\": 0.12, \"startTime\": 0.0 } },\n"
			"\t\t\t\"hits\": [ { \"start\": 0.25, \"end\": 0.4, \"damage\": 12, \"hitStop\": 0.07 } ]\n"
			"\t\t},\n"
			"\t\t\"b\": { \"clip\": \"d\", \"tags\": [\"Attack\"] }\n"
			"\t}\n"
			"}\n";
		auto has = [](const std::string& text, const char* fragment) { return text.find(fragment) != std::string::npos; };

		MoveSetData base;
		std::string error;
		const bool loaded = LoadMoveSetText(authored, base, error);
		test.Check(loaded, "save test moveset loads (%s)", error.c_str());
		if (!loaded)
			return;
		const size_t a = static_cast<size_t>(base.FindMove("a"));
		const size_t b = static_cast<size_t>(base.FindMove("b"));

		std::string saved;
		test.Check(PatchMoveSetText(authored, base, base, saved, error) && saved == authored, "saving without edits leaves the file as it is");

		// Value edits: tokens replaced, missing keys inserted in the style of their object.
		MoveSetData edited = base;
		edited.inputBuffer = 0.3f;
		edited.moves[a].speed = 0.7f;                         // absent: own line in a multi-line object
		edited.moves[a].entry.fresh.blendIn = 0.1f;           // token in a nested one-line object
		edited.moves[a].entry.afterLight.startTime = 0.05f;   // absent object: only its edited field
		edited.moves[a].hits[0].hitStop = 0.09f;              // token in a one-line array element
		edited.moves[b].heavy = true;                         // absent: joins a one-line object
		const bool savedEdits = PatchMoveSetText(authored, base, edited, saved, error);
		test.Check(savedEdits, "value edits save (%s)", error.c_str());
		test.Check(has(saved, "\"_note\": \"keep me\"") && has(saved, "\t\t\t\"tags\": [\"Attack\", \"Melee\"],\n"),
			"notes and untouched lines stay byte for byte");
		test.Check(has(saved, "\"inputBuffer\": 0.3,") && has(saved, "\"hitStop\": 0.09 }") && has(saved, "{ \"blendIn\": 0.1, \"startTime\": 0.0 }"),
			"edited values replace their tokens (shortest floats, decimal style kept)");
		test.Check(has(saved, "],\n\t\t\t\"speed\": 0.7\n\t\t}"), "a missing key gets its own line in a multi-line object");
		test.Check(has(saved, "\"b\": { \"clip\": \"d\", \"tags\": [\"Attack\"], \"heavy\": true }"), "a missing key joins a one-line object");
		test.Check(has(saved, "\"afterLight\": { \"startTime\": 0.05 }"), "a missing object gets only its edited fields");
		test.Check(!has(saved, "0000"), "no float noise");

		// Structure: a resized array is written fresh, a removed move cut, a new move written without defaults.
		MoveSetData restructured = base;
		HitWindowData extra;
		extra.start = 0.5f;
		extra.end = 0.6f;
		extra.damage = 20.0f;
		restructured.moves[a].hits.push_back(extra);
		restructured.moves.erase(restructured.moves.begin() + static_cast<std::ptrdiff_t>(b));
		MoveData added;
		added.id = "c";
		added.clip = "e";
		added.tags = { MoveTag::Attack };
		restructured.moves.push_back(added);
		const bool savedStructure = PatchMoveSetText(authored, base, restructured, saved, error);
		test.Check(savedStructure, "structural edits save (%s)", error.c_str());
		test.Check(!has(saved, "\"b\"") && has(saved, "\"c\": { \"clip\": \"e\", \"tags\": [\"Attack\"] }"),
			"a removed move is cut, a new one is written without default fields");
		test.Check(has(saved, "{ \"start\": 0.5, \"end\": 0.6, \"damage\": 20 }"), "a resized array is written fresh without default fields");

		// Edits are relative to what the editor started from: changes made to the file meanwhile survive.
		std::string external = authored;
		const std::string before = "\"hitStop\": 0.07";
		external.replace(external.find(before), before.size(), "\"hitStop\": 0.08");
		MoveSetData speedOnly = base;
		speedOnly.moves[a].speed = 0.7f;
		test.Check(PatchMoveSetText(external, base, speedOnly, saved, error) && has(saved, "\"hitStop\": 0.08") && has(saved, "\"speed\": 0.7"),
			"a save keeps what changed in the file since the editor read it");

		test.Check(!PatchMoveSetText("{ \"moves\": ", base, edited, saved, error), "a file that does not load is not overwritten");
	}

	// ---- Weapon data -----------------------------------------------------------------------------------

	bool ParseWeaponText(const char* text, WeaponData& out, DataLoadReport& report)
	{
		nlohmann::json json;
		return ParseDataJson(text, json, report) && ConvertDataJson(json, out, "", report) && ResolveWeapon(out, report);
	}

	void TestWeaponData(TestContext& test)
	{
		WeaponData rig;
		DataLoadReport rigReport;
		const bool ok = ParseWeaponText(R"({ "kind": "Melee",
			"meshes": [ { "model": "w.glb", "attach": "Rig", "hand": { "bone": "hand_r", "clip": "hold", "hide": ["holstered"] },
			              "holster": { "hide": ["active"] } } ],
			"stance": { "idle": "idle", "hold": "idle", "holdBones": ["clavicle_r"] },
			"blade": { "base": { "bone": "edge_start" }, "tip": { "bone": "edge_end", "offset": { "x": 0, "y": 0.1, "z": 0 } } } })", rig, rigReport);
		test.Check(ok, "valid Rig weapon loads (%s)", rigReport.Errors.empty() ? "" : rigReport.Errors.front().c_str());
		test.Check(ok && rig.meshes[0].hand.boneHash == hashed_string("hand_r") && rig.meshes[0].holster.bone.empty(),
			"Rig follow bones resolve (an empty one = parent-local posing)");
		test.Check(ok && rig.HasRigMesh() && rig.sockets.size() == 2 && rig.blade.base.socket == 0 && rig.blade.tip.socket == 1
			&& rig.blade.tip.offset.y == 0.1f, "blade points resolve to two sockets");
		test.Check(ok && rig.stance.resolvedAnimation == "w.glb", "stance clips default to the weapon model");
		test.Check(ok && rig.extend.bone.empty(), "weapons keep their length by default");
		test.Check(rigReport.Warnings.empty(), "valid Rig weapon has no warnings (%s)", rigReport.Warnings.empty() ? "" : rigReport.Warnings.front().c_str());

		WeaponData akimbo;
		DataLoadReport akimboReport;
		const bool akimboOk = ParseWeaponText(R"({ "kind": "Firearm", "holsterDelay": 1.5,
			"meshes": [ { "model": "p.glb", "attach": "Rig", "visibleWhenHolstered": false } ],
			"firearm": { "muzzles": [ { "bone": "muzzle_r" }, { "bone": "muzzle_l" }, { "bone": "muzzle_r", "offset": { "x": 0, "y": 0, "z": 0.1 } } ] } })",
			akimbo, akimboReport);
		test.Check(akimboOk && akimbo.sockets.size() == 2 && akimbo.firearm.muzzles[2].socket == akimbo.firearm.muzzles[0].socket,
			"muzzles sharing a bone share its socket (%s)", akimboReport.Errors.empty() ? "" : akimboReport.Errors.front().c_str());

		auto expectError = [&](const char* text, const char* what)
		{
			WeaponData broken;
			DataLoadReport brokenReport;
			test.Check(!ParseWeaponText(text, broken, brokenReport), "rejects %s", what);
		};
		expectError(R"({ "kind": "Melee", "meshes": [ { "model": "w.glb", "attach": "Rig" } ] })", "Rig melee weapon without blade bones");
		expectError(R"({ "kind": "Melee", "meshes": [ { "model": "w.glb", "hand": { "bone": "hand_r" } } ], "extend": { "bone": "tip" } })",
			"an extend bone on a rigid Bone weapon");
		expectError(R"({ "kind": "Melee", "meshes": [ { "model": "w.glb", "attach": "Rig" } ],
			"blade": { "base": { "bone": "a" }, "tip": { "bone": "b" } }, "extend": { "bone": "b", "axis": { "x": 0, "y": 0, "z": 0 } } })", "a zero extend axis");

		WeaponData extending;
		DataLoadReport extendingReport;
		const bool extendingOk = ParseWeaponText(R"({ "kind": "Melee", "meshes": [ { "model": "w.glb", "attach": "Rig" } ],
			"blade": { "base": { "bone": "a" }, "tip": { "bone": "b" } }, "extend": { "bone": "b", "axis": { "x": 0, "y": 0, "z": 2 } } })", extending, extendingReport);
		test.Check(extendingOk && extending.extend.boneHash == hashed_string("b") && extending.extend.resolvedAxis == glm::vec3(0.0f, 0.0f, 1.0f),
			"extend bone resolves, its axis normalized (%s)", extendingReport.Errors.empty() ? "" : extendingReport.Errors.front().c_str());
		expectError(R"({ "kind": "Firearm", "meshes": [ { "model": "w.glb", "attach": "Rig" } ] })", "Rig firearm without a muzzle bone");
		expectError(R"({ "kind": "Firearm", "meshes": [ { "model": "w.glb" } ], "firearm": { "muzzles": [] } })", "firearm without muzzles");
		expectError(R"({ "kind": "Melee", "meshes": [ { "model": "w.glb", "hand": { "bone": "hand_r" } } ], "stance": { "hold": "idle" } })", "stance hold without holdBones");
		expectError(R"({ "kind": "Melee", "meshes": [ { "model": "w.glb", "hand": { "bone": "hand_r", "hide": [""] } } ] })", "empty hide entry");
		expectError(R"({ "kind": "Firearm", "holsterDelay": -1, "meshes": [ { "model": "w.glb", "hand": { "bone": "hand_r" } } ] })", "negative holsterDelay");
		expectError(R"({ "kind": "Firearm", "meshes": [ { "model": "w.glb", "hand": { "bone": "hand_r" } } ], "stance": { "aimBones": ["spine_01"], "aimMaxPitch": 95 } })", "aim pitch limit beyond 89 degrees");

		WeaponData aiming;
		DataLoadReport aimingReport;
		const bool aimingOk = ParseWeaponText(R"({ "kind": "Firearm", "meshes": [ { "model": "w.glb", "hand": { "bone": "hand_r" } } ],
			"stance": { "hold": "aim", "holdBones": ["spine_01"], "aimBones": ["spine_01", "spine_02"], "aimMaxYaw": 45 } })", aiming, aimingReport);
		test.Check(aimingOk && aiming.stance.aimBones.size() == 2 && aiming.stance.aimMaxYaw == 45.0f && aiming.stance.aimMaxPitch == 70.0f,
			"stance aim bones and limits parse with defaults");
		expectError(R"({ "kind": "Melee", "meshes": [ { "model": "w.glb", "attach": "Socket" } ] })", "unknown attach mode");
		expectError(R"({ "kind": "Melee", "meshes": [ { "model": "w.glb", "hand": { "bone": "hand_r" } } ], "stance": { "handBones": ["hand_r"] } })",
			"grip hand bones without a hold clip");
		test.Check(aimingOk && aiming.stance.handBones.empty(), "hand bones default to none");
		expectError(R"({ "kind": "Melee", "meshes": [ { "model": "w.glb", "hand": { "bone": "hand_r" } } ], "trailStart": 1 })", "a trail starting at the tip");

		// The swing trail spans each sample along the forward of its look-at rotation (SetTrailTransform): the
		// texture's tip edge relies on that forward pointing from the base to the tip.
		const glm::vec3 from(0.3f, 1.0f, -0.2f), to(1.1f, 2.2f, 0.9f);
		const glm::vec3 forward = MathHelper::GetForwardVector(MathHelper::FindLookAtRotation(from, to));
		test.Check(glm::length(forward - glm::normalize(to - from)) < 1e-3f, "a look-at rotation's forward points at its target (got %.2f %.2f %.2f)",
			forward.x, forward.y, forward.z);
		expectError(R"({ "kind": "Melee", "meshes": [ { "model": "w.glb", "hand": { "bone": "" } } ] })", "Bone mesh without a hand bone");

		WeaponData prop;
		DataLoadReport propReport;
		ParseWeaponText(R"({ "kind": "Melee", "meshes": [ { "model": "w.glb", "hand": { "bone": "hand_r", "clip": "hold" } } ] })", prop, propReport);
		test.Check(propReport.Ok() && !propReport.Warnings.empty(), "Bone mesh warns about an unused clip");
	}
}

namespace
{
	// ---- Character data ----------------------------------------------------------------------------------

	bool ParseCharacterText(const char* text, CharacterData& out, DataLoadReport& report)
	{
		nlohmann::json json;
		return ParseDataJson(text, json, report) && ConvertDataJson(json, out, "", report) && ResolveCharacter(out, report);
	}

	void TestCharacterData(TestContext& test)
	{
		CharacterData character;
		DataLoadReport report;
		const bool ok = ParseCharacterText(R"({ "team": "Player", "moveset": "m.json",
			"locomotion": { "idle": "idle", "sources": { "idle": "weapon.glb" } } })", character, report);
		test.Check(ok && character.locomotion.sources.at("idle") == "weapon.glb", "locomotion clip source override parses (%s)",
			report.Errors.empty() ? "" : report.Errors.front().c_str());

		CharacterData broken;
		DataLoadReport brokenReport;
		test.Check(!ParseCharacterText(R"({ "team": "Player", "moveset": "m.json", "locomotion": { "sources": { "sprint": "a.glb" } } })",
			broken, brokenReport), "rejects a source override for an unknown locomotion clip");

		test.Check(ok && character.locomotion.strafe.legsBoneHash == hashed_string("pelvis") && character.locomotion.strafe.counterBoneHashes.size() == 3
			&& character.locomotion.lean.boneHash == hashed_string("spine_01") && character.animationLod.halfRateDistance > 0.0f,
			"strafe, lean and animation LOD parse with defaults");
		float recoilWeights = 0.0f;
		for (const HitRecoilBoneData& bone : character.hitRecoil.bones)
			recoilWeights += bone.weight;
		test.Check(ok && !character.hitRecoil.bones.empty() && character.hitRecoil.bones[0].boneHash == hashed_string("spine_01")
			&& std::abs(recoilWeights - 1.0f) < 1e-4f, "hit recoil defaults to the spine chain, weights adding up to the full angle");
		test.Check(ok && character.hitEffect.particle.empty() && character.targeting.keepBonus > 0.0f, "hit effect and targeting parse with defaults");

		auto expectError = [&](const char* text, const char* what)
		{
			CharacterData rejected;
			DataLoadReport rejectedReport;
			test.Check(!ParseCharacterText(text, rejected, rejectedReport), "rejects %s", what);
		};
		expectError(R"({ "team": "Player", "moveset": "m.json", "locomotion": { "strafe": { "maxTwist": 120 } } })", "a strafe twist beyond 90 degrees");
		expectError(R"({ "team": "Player", "moveset": "m.json", "animationLod": { "halfRateDistance": 30, "quarterRateDistance": 10 } })", "animation LOD distances out of order");
		expectError(R"({ "team": "Player", "moveset": "m.json", "movement": { "turnSharpness": 0 } })", "a zero turn sharpness");
		expectError(R"({ "team": "Player", "moveset": "m.json", "hitRecoil": { "bones": [ { "bone": "" } ] } })", "a hit recoil bone without a name");
		expectError(R"({ "team": "Player", "moveset": "m.json", "targeting": { "inputThreshold": 0 } })", "a zero targeting input threshold");
		expectError(R"({ "team": "Player", "moveset": "m.json", "airCombat": { "maxFallSpeed": 0 } })", "an air combat fall speed of zero");
		test.Check(ok && character.airCombat.fallGravityScale < character.airCombat.riseGravityScale && character.airCombat.hold > 0.0f,
			"air combat parses with defaults (hangs falling, keeps the arc rising)");
	}

	// ---- Shipped data ------------------------------------------------------------------------------------

	// Every character in GameData/combat/characters, the movesets and weapons it references, effects.json and
	// player.json load through CombatData without errors or warnings. (Clip names are checked when a body
	// loads its animations - CharacterAnimator logs missing clips.)
	void TestShippedData(TestContext& test)
	{
		CombatData::LoadScope scope;

		auto expectClean = [&](const std::string& path)
		{
			const std::vector<std::string> warnings = CombatData::GetWarnings(path);
			test.Check(warnings.empty(), "%s loads without warnings (%s)", path.c_str(), warnings.empty() ? "" : warnings.front().c_str());
		};

		const std::string directory = "GameData/combat/characters/";
		int characters = 0;
		for (const std::string& file : FileSystemEngine::GetFilesInPath(directory))
		{
			if (!StringHelper::EndsWith(file, ".character.json"))
				continue;
			++characters;

			const std::string path = directory + file;
			std::shared_ptr<const CharacterData> character = CombatData::LoadCharacter(path);
			test.Check(character != nullptr, "%s loads with its moveset and weapons", path.c_str());
			expectClean(path);
			if (!character)
				continue;

			expectClean(character->moveset);
			for (const auto& [slot, weaponFile] : character->weapons)
				expectClean(weaponFile);

			// The debug-panel save on the real file: one edited value is a local change that loads back (§10.4).
			if (character->moveSet && !character->moveSet->moves.empty())
			{
				const std::string text = FileSystemEngine::ReadFile(character->moveset);
				MoveSetData edited = *character->moveSet;
				edited.moves.front().speed += 0.25f;
				std::string saved;
				std::string error;
				const bool ok = PatchMoveSetText(text, *character->moveSet, edited, saved, error);
				size_t prefix = 0;
				while (prefix < text.size() && prefix < saved.size() && text[prefix] == saved[prefix])
					++prefix;
				size_t suffix = 0;
				while (suffix < text.size() - prefix && suffix < saved.size() - prefix
					&& text[text.size() - 1 - suffix] == saved[saved.size() - 1 - suffix])
					++suffix;
				const size_t touched = std::max(text.size(), saved.size()) - prefix - suffix;
				test.Check(ok && touched > 0 && touched <= 40, "%s: a one-value save is a local edit (%zu chars touched; %s)",
					character->moveset.c_str(), touched, error.c_str());
			}
		}
		test.Check(characters > 0, "shipped character files found in %s", directory.c_str());

		test.Check(CombatData::LoadStatusEffects() != nullptr, "%s loads", CombatData::StatusEffectsPath);
		expectClean(CombatData::StatusEffectsPath);
		test.Check(CombatData::LoadPlayerConfig() != nullptr, "%s loads", CombatData::PlayerConfigPath);
		expectClean(CombatData::PlayerConfigPath);
	}
}

bool RunCombatSelfTests(std::string& summary)
{
	TestContext test;
	TestComboResolver(test);
	TestHoldOpener(test);
	TestReactions(test);
	TestPoise(test);
	TestStatusEffects(test);
	TestHitStop(test);
	TestSpring(test);
	TestLaunchRise(test);
	TestAirCombat(test);
	TestAttackTargeting(test);
	TestCameraImpact(test);
	TestAttackTokens(test);
	TestMoveSetData(test);
	TestMoveSetSave(test);
	TestParamsEditing(test);
	TestWeaponData(test);
	TestCharacterData(test);
	TestShippedData(test);

	char header[128];
	std::snprintf(header, sizeof(header), "combat self tests: %d passed, %d failed\n", test.passed, test.failed);
	summary = header + test.failures;

	if (test.failed == 0)
		Logger::Info("%s", header);
	else
		Logger::Error("%s%s", header, test.failures.c_str());

	return test.failed == 0;
}

#endif
