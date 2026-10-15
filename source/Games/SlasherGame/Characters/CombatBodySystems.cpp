#include <Characters/CombatCharacter.h>
#include <Combat/CombatComponents.h>
#include <Combat/CombatFrame.h>

#include <Ecs/Ecs.h>
#include <ThreadPool.h>
#include <tracy/tracy/Tracy.hpp>

#include <algorithm>
#include <atomic>
#include <vector>

namespace
{
	std::atomic<bool> parallelPhase{ false };
	std::vector<CombatCharacter*> bodies;   // this frame's bodies (written on the main thread before they update)
	std::vector<CombatCharacter*> posed;    // CombatBodyPoseSystem's (main thread)

	void CollectBodies(SystemContext& ctx, std::vector<CombatCharacter*>& out)
	{
		out.clear();   // keeps its capacity between frames
		ctx.ForEach<CombatBodyComponent>([&out](entt::entity, CombatBodyComponent& component)
		{
			if (component.body != nullptr && component.body->IsCombatReady())
				out.push_back(component.body);
		});
	}

	// fn(body) for every body, over the thread pool, the bodies handed out one by one as threads free up. A body's cost
	// varies a lot (the controlled body, bodies near the camera evaluate their pose every frame, far ones every 4th),
	// and the pool's ParallelFor cuts a range into equal fixed chunks: one chunk of expensive bodies held the frame
	// (70 % of the pool's time used at 300 bodies). One job per pool thread, each pulling the next body.
	template <typename Fn>
	void ForEachBodyParallel(SystemContext& ctx, const std::vector<CombatCharacter*>& list, Fn fn)
	{
		const size_t count = list.size();
		const size_t jobs = std::min(count, static_cast<size_t>(std::max(ThreadPool::GetNumThreadsForAsyncUpdate(), 1)));
		std::atomic<size_t> next{ 0 };
		ctx.ParallelFor(jobs, [&](size_t)
		{
			for (size_t i = next.fetch_add(1, std::memory_order_relaxed); i < count; i = next.fetch_add(1, std::memory_order_relaxed))
				fn(*list[i]);
		});
	}
}

bool CombatFrame::IsParallel()
{
	return parallelPhase.load(std::memory_order_relaxed);
}

void CombatFrame::SetParallel(bool value)
{
	parallelPhase.store(value, std::memory_order_relaxed);
}

const std::vector<CombatCharacter*>& CombatFrame::GetBodies()
{
	return bodies;
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
	CollectBodies(ctx, bodies);
	TracyPlot("Combat bodies", static_cast<int64_t>(bodies.size()));
	std::sort(bodies.begin(), bodies.end(), [](CombatCharacter* a, CombatCharacter* b)
	{
		return entt::to_integral(a->Handle()) < entt::to_integral(b->Handle());
	});

	CombatFrame::SetParallel(true);
	{
		ZoneScopedN("Combat: begin frame");
		ForEachBodyParallel(ctx, bodies, [](CombatCharacter& body) { body.BeginCombatFrame(); });
	}
	{
		ZoneScopedN("Combat: update");
		ForEachBodyParallel(ctx, bodies, [](CombatCharacter& body) { body.UpdateCombat(); });
	}
	CombatFrame::SetParallel(false);

	ZoneScopedN("Combat: flush");
	for (CombatCharacter* body : bodies)
		if (body->IsCombatReady())   // an earlier flush may have destroyed it (corpse removal)
			body->FlushCombatFrame();
}
REGISTER_SYSTEM(CombatBodyUpdateSystem, .ParallelSafe(true))

// The pose of every body (COMBAT_DESIGN.md §2.2): animation, movement, hitboxes, weapon posing. After the PostUpdate
// event flush resolved this frame's hits, so a hit-stop that started this frame already holds the pose. Each body
// writes only itself and its own physics bodies.
DECLARE_SYSTEM(CombatBodyPoseSystem, PostAsyncUpdate)
{
	CollectBodies(ctx, posed);   // bodies spawned or removed since the combat frame included / left out
	ForEachBodyParallel(ctx, posed, [](CombatCharacter& body)
	{
		ZoneScopedN("Body: pose");
		body.UpdatePose();
	});
}
REGISTER_SYSTEM(CombatBodyPoseSystem, .ParallelSafe(true))
