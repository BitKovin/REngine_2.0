#pragma once

// Where a body is shown: the decision of SLICE_M2F_CHARACTERS.md section 6, pure so the tests can walk every row of its table.
//
// Bodies are made farther out than they are shown. In the ring between the visible radius and the spawn radius a body is Warming: a real body
// that walks and animates but is not drawn, so that it is already going at pace when it reaches the visible radius. It is shown when it is
// within the visible radius and has lived for the warm-up time; once shown it keeps being shown until it is `hysteresis` metres beyond the radius.

namespace BodyVisibility
{
	struct Input
	{
		float distance = 0.0f;       // from the player
		bool wasShown = false;
		bool warmedUp = false;       // lived for WarmupSeconds
		float visibleRadius = 45.0f;
		float hysteresis = 5.0f;
	};

	// True when the body should be shown.
	inline bool Shown(const Input& in)
	{
		if (in.wasShown)
			return in.distance <= in.visibleRadius + in.hysteresis;
		return in.distance <= in.visibleRadius && in.warmedUp;
	}
}
