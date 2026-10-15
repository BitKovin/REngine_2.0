#pragma once

#include <vector>

class CombatCharacter;

// The combat body frame (COMBAT_DESIGN.md §2.2). CombatBodyUpdateSystem (Characters/CombatBodySystems.cpp) runs the
// update of every body concurrently on the engine thread pool. While it does, body code must not touch other
// entities or engine-global state (spawning, sounds, particles, the camera, time scale, another body's moves):
// such work is queued on the body (CombatCharacter::RunOnMainThread / QueueHit / PlayCue) and runs in the
// system's serial flush, in body order.
namespace CombatFrame
{
	bool IsParallel();             // true while body updates may be running on worker threads
	void SetParallel(bool value);  // CombatBodyUpdateSystem only (main thread)

	// The bodies of this combat frame, in body order - valid while they update (IsParallel), when their
	// BeginCombatFrame state (position, grounded, team, death) is final. Read-only (AI sight).
	const std::vector<CombatCharacter*>& GetBodies();
}
