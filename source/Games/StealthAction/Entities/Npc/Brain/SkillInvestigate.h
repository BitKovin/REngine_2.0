#pragma once

#include "NpcSkill.h"

// Checks out whatever the Mind is curious about (NPC_AI_DESIGN.md §12): a glance first, then a walk to a spot 2-4 m from
// the point with a view of it, a look around, and the concern is settled. It works as the investigator or the backup of an
// investigate situation: the investigator looks into it, the backup covers from farther off and watches until the
// investigator is done. A hostile that is in view is closed on up to a hold distance and watched (the combat skills of M3
// take over from there). Weapon and pace follow how serious the concern is.
class InvestigateSkill : public NpcSkill
{
public:

	const char* Id() const override { return "investigate"; }

	float Want(const NpcContext& context) const override;
	void Update(NpcContext& context) override;
	void Stop(NpcContext& context) override { active = false; }
	float MinCommit() const override { return 1.5f; }

	json Serialize(double now) const override;
	void Deserialize(const json& data, double now) override;
	std::string Describe(double now) const override;

private:

	enum class Phase : int
	{
		React,      // stands and looks, for the reaction time
		Approach,   // walks to the post (or holds at a distance from a hostile in view)
		LookAround, // the head sweeps
		Cover       // the backup holds its post and watches the point
	};

	bool active = false;            // a concern is being worked on
	std::string subject;
	NpcConcernType type = NpcConcernType::Curious;
	NpcRole role = NpcRole::None;   // its part in the investigate situation (none for a hostile in view)
	vec3 point = vec3();            // the newest information about where
	vec3 plannedPoint = vec3();     // the point the approach was planned for
	Phase phase = Phase::React;
	double startedAt = 0.0;
	double phaseStartedAt = 0.0;
	bool hasPost = false;
	vec3 post = vec3();
	vec3 lookBase = vec3(0.0f, 0.0f, 1.0f);

	void Begin(NpcContext& context, const NpcConcern& concern);
	void PlanApproach(NpcContext& context);
	void EnterPhase(NpcContext& context, Phase next);
	void Finish(NpcContext& context);
};
