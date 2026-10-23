#include "SimDemoPopulation.h"

#ifndef DISTRIBUTION

namespace sim::movement
{
	DemoModule::DemoModule(MovementModule& movement, std::vector<glm::vec3> points, bool commute) : movement(movement), points(std::move(points)), commute(commute) {}

	void DemoModule::ChooseTrip(RuleContext& ctx, Entity walker, int32_t index) const
	{
		if (points.empty())
			return;
		Rng rng = ctx.Random();
		const size_t chosen = index >= 0 ? static_cast<size_t>(index) % points.size() : rng.Below(static_cast<uint32_t>(points.size()));
		ctx.Command(RequestWalk{ walker, points[chosen], Gait::Walk });
	}

	void DemoModule::Register(Registration& reg)
	{
		reg.Event<PopulateDemo>(EventInput);
		reg.Event<TripWanted>(EventInput);

		reg.Command<PlaceDemoWalkers>([this](ApplyContext& ctx, const PlaceDemoWalkers& command)
		{
			if (points.empty())
			{
				ctx.Reject("the world has no spawn points");
				return;
			}
			const size_t existing = ctx.world.Registry().storage<Walker>().size();
			for (size_t i = 0; i < command.count; ++i)
			{
				const Entity walker = movement.Place(ctx, points[(existing + i) % points.size()]);
				ctx.Emit(TripWanted{ walker, -1 });
			}
		});

		reg.Rule<PopulateDemo>("populate", [](RuleContext& ctx, const PopulateDemo& event) { ctx.Command(PlaceDemoWalkers{ event.count }); });
		reg.Rule<TripWanted>("trip", [this](RuleContext& ctx, const TripWanted& event) { ChooseTrip(ctx, event.walker, event.destination); });
		reg.Rule<Arrived>("next_trip", [this](RuleContext& ctx, const Arrived& event)
		{
			if (commute)
				ChooseTrip(ctx, event.walker, -1);
		});
		reg.Rule<PathFailed>("retry", [this](RuleContext& ctx, const PathFailed& event)
		{
			if (commute)
				ChooseTrip(ctx, event.walker, -1);
		});
		reg.Rule<Replaced>("restart", [this](RuleContext& ctx, const Replaced& event)
		{
			if (commute)
				ChooseTrip(ctx, event.walker, -1);
		});
	}
}

#endif
