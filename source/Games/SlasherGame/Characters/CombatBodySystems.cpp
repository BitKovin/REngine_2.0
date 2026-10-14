#include <Characters/CombatCharacter.h>
#include <Combat/CombatComponents.h>
#include <Combat/CombatFrame.h>

#include <Ecs/Ecs.h>

#include <algorithm>
#include <atomic>
#include <vector>

namespace
{
	std::atomic<bool> parallelPhase{ false };
}

bool CombatFrame::IsParallel()
{
	return parallelPhase.load(std::memory_order_relaxed);
}

void CombatFrame::SetParallel(bool value)
{
	parallelPhase.store(value, std::memory_order_relaxed);
}

// The combat frame of every body (COMBAT_DESIGN.md §2.2). Runs after Level::Update (the player controller took its
// look input) and before this tick group's event flush resolves the hits:
//   1. BeginCombatFrame, parallel - each body refreshes its own position, grounded state and input edges
//   2. UpdateCombat, parallel     - controller input, moves, blade sweeps, facing, aim. Reads other bodies (their
//                                   step-1 state, components), writes only itself; cross-entity and engine-global
//                                   work is queued (CombatFrame.h)
//   3. FlushCombatFrame, serial   - queued hits, cues, shots and cross-body actions, in a stable body order, so the
//                                   funnel resolves hits the same way whatever thread swept them
DECLARE_SYSTEM(CombatBodyUpdateSystem, PostUpdate)
{
	static std::vector<CombatCharacter*> bodies;   // main thread only; keeps its capacity between frames
	bodies.clear();

	ctx.ForEach<CombatBodyComponent>([](entt::entity, CombatBodyComponent& component)
	{
		if (component.body != nullptr && component.body->IsCombatReady())
			bodies.push_back(component.body);
	});

	std::sort(bodies.begin(), bodies.end(), [](CombatCharacter* a, CombatCharacter* b)
	{
		return entt::to_integral(a->Handle()) < entt::to_integral(b->Handle());
	});

	CombatFrame::SetParallel(true);
	ctx.ParallelFor(bodies.size(), [](size_t i) { bodies[i]->BeginCombatFrame(); });
	ctx.ParallelFor(bodies.size(), [](size_t i) { bodies[i]->UpdateCombat(); });
	CombatFrame::SetParallel(false);

	for (CombatCharacter* body : bodies)
		if (body->IsCombatReady())   // an earlier flush may have destroyed it (corpse removal)
			body->FlushCombatFrame();
}
REGISTER_SYSTEM(CombatBodyUpdateSystem, .ParallelSafe(true))
