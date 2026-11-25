#pragma once

#include "NpcSkill.h"

// Runs to an alarm point and rings it (NPC_AI_DESIGN.md §17.11). The director gives the job to one guard of a search that
// has seen the intruder; the guard runs there, stands at the bell for the ringing time and then rings it. Nobody is
// told anything before that: stopping the runner on the way (or at the bell) prevents the alarm.
class RaiseAlarmSkill : public NpcSkill
{
public:

	const char* Id() const override { return "raise_alarm"; }

	float Want(const NpcContext& context) const override;
	void Update(NpcContext& context) override;
	void Stop(NpcContext& context) override { Reset(); }
	float MinCommit() const override { return 1.0f; }

	json Serialize(double now) const override;
	void Deserialize(const json& data, double now) override;
	std::string Describe(double now) const override;

private:

	bool ringing = false;
	bool rung = false;
	double ringStartedAt = 0.0;

	void Reset() { ringing = false; rung = false; }
};
