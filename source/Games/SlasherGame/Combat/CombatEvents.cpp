#include "CombatEvents.h"

// ParallelEventBus creates an event type's flusher on the first Push of that type. If the first Push happens
// inside an event handler (ResolveHitRequest emits the feedback events below), the flusher list would grow
// while EcsScheduler is iterating it during the tick-group flush. Emitting one empty event of every type once at
// startup (outside any tick group) creates all flushers up front. Handlers ignore events whose handles do not
// resolve, so these empty events have no effect.
void RegisterCombatEventTypes()
{
	EcsScheduler::Emit(HitRequestEvent{});
	EcsScheduler::Emit(HitLandedEvent{});
	EcsScheduler::Emit(DeathEvent{});
	EcsScheduler::Emit(PoiseBrokenEvent{});
	EcsScheduler::Emit(WeakPointBrokenEvent{});
	EcsScheduler::Emit(FinisherEvent{});
}
