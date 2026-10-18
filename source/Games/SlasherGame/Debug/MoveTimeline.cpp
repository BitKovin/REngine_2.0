#include "MoveTimeline.h"

#ifndef DISTRIBUTION

#include <Actions/MoveRunner.h>
#include <Data/MoveSetData.h>

#include <imgui/imgui.h>

#include <algorithm>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <string>
#include <vector>

namespace
{
	constexpr float LabelWidth = 72.0f;     // lane names
	constexpr float RulerHeight = 16.0f;
	constexpr float LaneHeight = 17.0f;
	constexpr float ViewMargin = 0.2f;      // s shown after the last window
	constexpr float MinTickSpacing = 56.0f; // px

	const ImU32 Background = IM_COL32(28, 28, 30, 255);
	const ImU32 LaneShade = IM_COL32(38, 38, 42, 255);
	const ImU32 GridLine = IM_COL32(64, 64, 70, 255);
	const ImU32 LabelText = IM_COL32(200, 200, 205, 255);
	const ImU32 DimText = IM_COL32(140, 140, 150, 255);
	const ImU32 BarText = IM_COL32(15, 15, 18, 255);
	const ImU32 HitColor = IM_COL32(225, 70, 70, 255);
	const ImU32 LightColor = IM_COL32(90, 205, 100, 255);
	const ImU32 HeavyColor = IM_COL32(245, 160, 50, 255);
	const ImU32 CancelColor = IM_COL32(80, 140, 245, 255);
	const ImU32 ArmorColor = IM_COL32(170, 150, 125, 255);
	const ImU32 InvulnerableColor = IM_COL32(70, 205, 215, 255);
	const ImU32 HoldColor = IM_COL32(250, 215, 60, 255);
	const ImU32 RecoveryColor = IM_COL32(190, 105, 255, 255);
	const ImU32 ReachColor = IM_COL32(235, 120, 180, 255);
	const ImU32 NowColor = IM_COL32(255, 255, 255, 255);
	const ImU32 HoverColor = IM_COL32(255, 255, 255, 90);

	std::string Format(const char* format, ...)
	{
		char buffer[512];
		va_list args;
		va_start(args, format);
		std::vsnprintf(buffer, sizeof(buffer), format, args);
		va_end(args);
		return buffer;
	}

	template<typename Enum>
	std::string EnumName(Enum value)
	{
		const nlohmann::json json = value;
		return json.is_string() ? json.get<std::string>() : std::string("?");
	}

	std::string TagNames(TagMask mask)
	{
		std::string names;
		for (int tag = 1; tag < static_cast<int>(MoveTag::Count); ++tag)
		{
			if (mask & TagBit(static_cast<MoveTag>(tag)))
				names += (names.empty() ? "" : ", ") + EnumName(static_cast<MoveTag>(tag));
		}
		return names.empty() ? std::string("nothing") : names;
	}

	struct Bar
	{
		float start = 0.0f;
		float end = 0.0f;
		ImU32 color = 0;
		std::string label;    // drawn inside when it fits
		std::string detail;   // hover
	};

	struct Lane
	{
		std::string name;
		std::string help;     // hover on the name
		std::vector<Bar> bars;
	};

	struct Marker
	{
		float time = 0.0f;
		ImU32 color = 0;
		std::string label;
		std::string detail;
	};

	struct Timeline
	{
		std::vector<Lane> lanes;
		std::vector<Marker> markers;
		float interest = 0.0f;     // last time anything happens in the move
	};

	Timeline Build(const MoveData& move, float endTime)
	{
		Timeline timeline;
		auto note = [&](float t) { timeline.interest = std::max(timeline.interest, t); };
		auto until = [&](float end) { return end < 0.0f ? endTime : end; };

		Lane hits{ "Hits", "When the weapon deals damage: the blade (or a bone sphere) sweeps for victims while a bar is lit. Each victim is hit once per window.", {} };
		for (size_t i = 0; i < move.hits.size(); ++i)
		{
			const HitWindowData& hit = move.hits[i];
			std::string detail = Format("Hit %zu  %.2f - %.2f s\n%.0f damage, %.0f poise, %s", i, hit.start, hit.end, hit.damage, hit.poiseDamage,
				EnumName(hit.reaction).c_str());
			if (hit.reaction == HitReaction::Launch)
				detail += Format(" (%.1f m/s up)", hit.launch);
			detail += Format("\nknockback %.1f m/s, hit-stop %.2f s", hit.knockback, hit.hitStop);
			if (hit.camera > 0.0f)
				detail += Format(", camera kick %.2f", hit.camera);
			hits.bars.push_back({ hit.start, hit.end, HitColor, Format("hit %zu %s", i, EnumName(hit.reaction).c_str()), detail });
			note(hit.end);
		}
		timeline.lanes.push_back(std::move(hits));

		const ComboLinkData& combo = move.combo;
		if (combo.IsValid())
		{
			Lane lane{ "Combo", "When Attack chains into the next attack. Presses before the window wait for it (input buffer).\n"
				"Held for holdDelayInMove instead: the launcher.", {} };
			const std::string next = combo.next.empty() ? std::string("-") : combo.next;
			const std::string alternative = combo.alternative.empty() ? std::string("-") : combo.alternative;
			lane.bars.push_back({ combo.windowStart, combo.alternativeAt, LightColor, "light -> " + next,
				Format("LIGHT zone %.2f - %.2f s\nA tap released here (or made earlier in this attack) starts '%s'.", combo.windowStart, combo.alternativeAt, next.c_str()) });
			lane.bars.push_back({ combo.alternativeAt, combo.windowEnd, HeavyColor, "heavy -> " + alternative,
				Format("HEAVY zone %.2f - %.2f s\nAttack still held when it begins, or pressed inside it, starts '%s' - at once, or on release\n"
					"where the launcher can start (held for holdDelayInMove instead: the launcher).\nNothing by %.2f: the combo is over.",
					combo.alternativeAt, combo.windowEnd, alternative.c_str(), combo.windowEnd) });
			timeline.lanes.push_back(std::move(lane));
			note(combo.windowEnd);
		}

		for (size_t i = 0; i < move.cancels.size(); ++i)
		{
			const CancelWindowData& cancel = move.cancels[i];
			const std::string names = TagNames(cancel.intoMask);
			std::string detail = Format("Cancel %.2f - %s\nThese may interrupt the move: %s.", cancel.start,
				cancel.end < 0.0f ? "end" : Format("%.2f s", cancel.end).c_str(), names.c_str());
			if (cancel.intoMask & TagBit(MoveTag::Locomotion))
				detail += "\nLocomotion: pushing the stick ends the move (unless Attack is held or pressed - the combo goes on).";
			timeline.lanes.push_back({ i == 0 ? "Cancel" : "", "Which actions may interrupt the move, and from when. Without an open window a press waits (input / action buffer).",
				{ { cancel.start, until(cancel.end), CancelColor, names, detail } } });
			note(cancel.end < 0.0f ? cancel.start : cancel.end);
		}

		if (!move.armor.empty())
		{
			Lane lane{ "Armor", "Hits taken here stagger less: their reaction is downgraded by the poise resist (1 = no reaction at all).", {} };
			for (const ArmorWindowData& armor : move.armor)
			{
				lane.bars.push_back({ armor.start, until(armor.end), ArmorColor, Format("armor %.0f%%", armor.poiseResist * 100.0f),
					Format("Armor %.2f - %.2f s\npoise resist %.0f%%", armor.start, until(armor.end), armor.poiseResist * 100.0f) });
				note(until(armor.end));
			}
			timeline.lanes.push_back(std::move(lane));
		}

		if (!move.invulnerable.empty())
		{
			Lane lane{ "Invuln", "Hits do not land here (i-frames).", {} };
			for (const TimeWindowData& window : move.invulnerable)
			{
				lane.bars.push_back({ window.start, until(window.end), InvulnerableColor, "i-frames",
					Format("Invulnerable %.2f - %.2f s", window.start, until(window.end)) });
				note(until(window.end));
			}
			timeline.lanes.push_back(std::move(lane));
		}

		const ReachData& reach = move.reach;
		if (reach.IsActive())
		{
			const float from = std::max(0.0f, reach.resolvedStart - reach.blendIn);
			const float to = reach.resolvedEnd + reach.blendOut;
			timeline.lanes.push_back({ "Reach", "How much longer the weapon gets: it grows into the bar, is fully out over its middle and shrinks back at its end.",
				{ { from, to, ReachColor, Format("+%.2f m", reach.length),
					Format("Reach +%.2f m (%s)\ngrows %.2f - %.2f s, fully out %.2f - %.2f s, back by %.2f s", reach.length,
						EnumName(reach.weapon).c_str(), from, reach.resolvedStart, reach.resolvedStart, reach.resolvedEnd, to) } } });
			note(to);
		}

		if (move.hold.IsValid())
		{
			timeline.markers.push_back({ move.hold.until, HoldColor, "hold",
				Format("Hold until %.2f s\nAttack must stay held through the wind-up; released earlier, '%s' takes over.", move.hold.until, move.hold.release.c_str()) });
			note(move.hold.until);
		}
		if (move.recoveryStart >= 0.0f)
		{
			timeline.markers.push_back({ move.recoveryStart, RecoveryColor, "recovery",
				Format("Recovery from %.2f s\nThe move releases the body: moving or any action is possible. The clip plays out (weapon back to idle)\nuntil the body moves.", move.recoveryStart) });
			note(move.recoveryStart);
		}
		return timeline;
	}

	// What the move does at `time`, in words.
	std::string DescribeNow(const MoveData& move, const MoveSlot& slot)
	{
		const float time = slot.time;
		std::vector<std::string> parts;

		if (move.hold.IsValid() && time < move.hold.until)
			parts.push_back(Format("WIND-UP: keep Attack held until %.2f (released earlier -> %s)", move.hold.until, move.hold.release.c_str()));
		for (size_t i = 0; i < move.hits.size(); ++i)
			if (time >= move.hits[i].start && time <= move.hits[i].end)
				parts.push_back(Format("HIT %zu active", i));

		const ComboLinkData& combo = move.combo;
		if (combo.IsValid())
		{
			if (time < combo.windowStart)
				parts.push_back(Format("combo: presses wait for %.2f", combo.windowStart));
			else if (time < combo.alternativeAt)
				parts.push_back("LIGHT zone: tap -> " + combo.next);
			else if (time <= combo.windowEnd)
				parts.push_back("HEAVY zone: hold / press -> " + combo.alternative);
		}

		TagMask open = 0;
		for (const CancelWindowData& cancel : move.cancels)
			if (time >= cancel.start && (cancel.end < 0.0f || time <= cancel.end))
				open |= cancel.intoMask;
		if (open)
			parts.push_back("can cancel into " + TagNames(open));

		for (const ArmorWindowData& armor : move.armor)
			if (time >= armor.start && (armor.end < 0.0f || time <= armor.end))
				parts.push_back(Format("armor %.0f%%", armor.poiseResist * 100.0f));
		for (const TimeWindowData& window : move.invulnerable)
			if (time >= window.start && (window.end < 0.0f || time <= window.end))
				parts.push_back("invulnerable");
		if (move.reach.At(time) > 0.005f)
			parts.push_back(Format("weapon +%.2f m", move.reach.At(time)));

		if (slot.sustaining)
			parts.push_back("sustained past its end by its behaviour");
		if (parts.empty())
			parts.push_back(move.recoveryStart >= 0.0f && time < move.recoveryStart ? "playing out to the recovery" : "playing out");

		std::string text;
		for (const std::string& part : parts)
			text += (text.empty() ? "" : "  |  ") + part;
		return text;
	}

	void HelpMarker(const char* text)
	{
		ImGui::TextDisabled("(?)");
		ImGui::SetItemTooltip("%s", text);
	}

	// Text inside [min, max] when it fits, clipped otherwise.
	void BarLabel(ImDrawList* draw, const ImVec2& min, const ImVec2& max, const std::string& text)
	{
		const ImVec2 size = ImGui::CalcTextSize(text.c_str());
		if (max.x - min.x < size.x + 6.0f)
			return;
		draw->AddText(ImVec2(min.x + 3.0f, min.y + (max.y - min.y - size.y) * 0.5f), BarText, text.c_str());
	}

	float TickStep(float span, float width)
	{
		static const float steps[] = { 0.02f, 0.05f, 0.1f, 0.2f, 0.25f, 0.5f, 1.0f, 2.0f, 5.0f };
		for (float step : steps)
			if (width * step / std::max(span, 1e-3f) >= MinTickSpacing)
				return step;
		return 10.0f;
	}
}

void DrawMoveTimeline(const char* label, const MoveSlot& slot, float playbackSpeed)
{
	if (!slot.IsActive())
	{
		ImGui::TextDisabled("%s: -", label);
		return;
	}

	const MoveData& move = *slot.move;
	const float endTime = std::max(slot.endTime, 0.001f);
	const float speed = std::max(playbackSpeed, 0.001f);

	ImGui::Text("%s: %s", label, move.id.c_str());
	ImGui::SameLine();
	ImGui::TextDisabled("clip %.2f / %.2f s   speed x%.2f (real %.2f / %.2f s)", slot.time, endTime, speed, slot.time / speed, endTime / speed);
	ImGui::SameLine();
	HelpMarker(
		"Timeline of the running move in clip seconds (the move clock: real seconds = clip seconds / speed).\n"
		"\n"
		"Hits (red)        the weapon deals damage while a bar is lit\n"
		"Combo (green)     LIGHT zone: a tap starts the next attack\n"
		"Combo (orange)    HEAVY zone: Attack held when it begins, or pressed in it, starts the alternative\n"
		"                  (Attack held for holdDelayInMove instead starts the launcher)\n"
		"Cancel (blue)     which actions may interrupt the move ('Locomotion' = moving the stick)\n"
		"Armor / Invuln    hits stagger less / do not land\n"
		"hold (yellow)     Attack must stay held until here, or the move hands over\n"
		"recovery (purple) from here the move releases the body; its clip plays out until the body moves\n"
		"white line        now\n"
		"\n"
		"The view zooms to the part of the clip where the move's windows are; the rest is only its tail.\n"
		"Hover the timeline for the details at any time, a lane name for what the lane means.");

	ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.85f), "Now: %s", DescribeNow(move, slot).c_str());

	const Timeline timeline = Build(move, endTime);
	const float viewEnd = timeline.interest > 0.0f ? std::min(endTime, timeline.interest + ViewMargin) : endTime;

	const float width = std::max(ImGui::GetContentRegionAvail().x, LabelWidth + 100.0f);
	const float lanesHeight = static_cast<float>(timeline.lanes.size()) * LaneHeight;
	const float height = RulerHeight + lanesHeight + 2.0f;
	const ImVec2 origin = ImGui::GetCursorScreenPos();
	ImGui::InvisibleButton("##timeline", ImVec2(width, height));
	const bool hovered = ImGui::IsItemHovered();

	ImDrawList* draw = ImGui::GetWindowDrawList();
	const float x0 = origin.x + LabelWidth;
	const float x1 = origin.x + width;
	const float top = origin.y + RulerHeight;
	auto x = [&](float t) { return x0 + (x1 - x0) * std::clamp(t / viewEnd, 0.0f, 1.0f); };

	draw->AddRectFilled(ImVec2(x0, origin.y), ImVec2(x1, origin.y + height), Background);

	// Ruler and grid.
	const float step = TickStep(viewEnd, x1 - x0);
	for (int i = 0; i * step <= viewEnd + 1e-4f; ++i)
	{
		const float t = i * step;
		const float tx = x(t);
		draw->AddLine(ImVec2(tx, origin.y + RulerHeight - 4.0f), ImVec2(tx, origin.y + height), GridLine);
		const std::string text = step < 0.1f ? Format("%.2f", t) : Format("%.1f", t);
		draw->AddText(ImVec2(tx + 2.0f, origin.y), DimText, text.c_str());
	}
	if (viewEnd < endTime)
	{
		const std::string tail = Format("clip goes on to %.2f >", endTime);
		const ImVec2 size = ImGui::CalcTextSize(tail.c_str());
		draw->AddText(ImVec2(x1 - size.x - 2.0f, origin.y), DimText, tail.c_str());
	}

	// Lanes.
	for (size_t lane = 0; lane < timeline.lanes.size(); ++lane)
	{
		const float y0 = top + lane * LaneHeight;
		const float y1 = y0 + LaneHeight - 2.0f;
		if (lane % 2 == 0)
			draw->AddRectFilled(ImVec2(x0, y0), ImVec2(x1, y1 + 2.0f), LaneShade);
		draw->AddText(ImVec2(origin.x, y0 + 1.0f), LabelText, timeline.lanes[lane].name.c_str());
		for (const Bar& bar : timeline.lanes[lane].bars)
		{
			if (bar.start > viewEnd)
				continue;
			const ImVec2 min(x(bar.start), y0 + 1.0f);
			const ImVec2 max(std::max(x(bar.end), min.x + 2.0f), y1);
			draw->AddRectFilled(min, max, bar.color, 2.0f);
			BarLabel(draw, min, max, bar.label);
		}
	}

	// Markers (labelled in the ruler) and the current time.
	for (const Marker& marker : timeline.markers)
	{
		const float mx = x(marker.time);
		draw->AddLine(ImVec2(mx, top), ImVec2(mx, origin.y + height), marker.color, 2.0f);
		draw->AddText(ImVec2(mx + 3.0f, origin.y + height - ImGui::GetFontSize() - 1.0f), marker.color, marker.label.c_str());
	}
	const float nowX = x(slot.time);
	draw->AddLine(ImVec2(nowX, origin.y + RulerHeight - 6.0f), ImVec2(nowX, origin.y + height), NowColor, 2.0f);
	draw->AddTriangleFilled(ImVec2(nowX - 4.0f, origin.y + RulerHeight - 7.0f), ImVec2(nowX + 4.0f, origin.y + RulerHeight - 7.0f),
		ImVec2(nowX, origin.y + RulerHeight - 1.0f), NowColor);
	if (slot.time > viewEnd)
		draw->AddText(ImVec2(x1 - 70.0f, origin.y + height - ImGui::GetFontSize() - 1.0f), NowColor, "now >>");

	if (!hovered)
		return;

	// Hover: a lane name explains the lane; the timeline lists everything at the time under the cursor.
	const ImVec2 mouse = ImGui::GetIO().MousePos;
	if (mouse.x < x0)
	{
		const int lane = static_cast<int>((mouse.y - top) / LaneHeight);
		if (lane >= 0 && lane < static_cast<int>(timeline.lanes.size()))
		{
			// Unnamed lanes continue the one above (several cancel windows).
			int named = lane;
			while (named > 0 && timeline.lanes[static_cast<size_t>(named)].name.empty())
				--named;
			ImGui::SetTooltip("%s", timeline.lanes[static_cast<size_t>(named)].help.c_str());
		}
		return;
	}

	const float time = viewEnd * std::clamp((mouse.x - x0) / (x1 - x0), 0.0f, 1.0f);
	draw->AddLine(ImVec2(mouse.x, top), ImVec2(mouse.x, origin.y + height), HoverColor);

	std::string text = Format("clip %.3f s  (real %.3f s at x%.2f)", time, time / speed, speed);
	const float slack = viewEnd * 4.0f / (x1 - x0);   // markers within 4 px
	for (const Lane& lane : timeline.lanes)
		for (const Bar& bar : lane.bars)
			if (time >= bar.start && time <= bar.end)
				text += "\n\n" + bar.detail;
	for (const Marker& marker : timeline.markers)
		if (std::abs(time - marker.time) <= slack)
			text += "\n\n" + marker.detail;
	ImGui::SetTooltip("%s", text.c_str());
}

void DrawRecoveryStatus(const RecoverySlot& recovery)
{
	if (!recovery.IsActive())
		return;
	ImGui::TextColored(ImVec4(0.75f, 0.45f, 1.0f, 1.0f), "Recovery: %s  clip %.2f / %.2f s  %s", recovery.move->id.c_str(), recovery.time,
		recovery.endTime, recovery.released ? "(released: legs and upper body hand over to locomotion)" : "(whole body until the body moves)");
	ImGui::SetItemTooltip("The attack is over - the body is free (moving or any move is possible). Its clip plays out so the\n"
		"weapon returns to idle: on the whole body until the body moves, then it hands over to locomotion.");
}

#endif
