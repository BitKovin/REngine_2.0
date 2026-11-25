#include "NpcSkill.h"

#include "SkillChase.h"
#include "SkillInvestigate.h"
#include "SkillRaiseAlarm.h"
#include "SkillRoutine.h"
#include "SkillSearch.h"
#include "SkillTendBody.h"

#include <Logger.hpp>

std::unique_ptr<NpcSkill> CreateNpcSkill(const std::string& id)
{
	if (id == "routine") return std::make_unique<RoutineSkill>();
	if (id == "investigate") return std::make_unique<InvestigateSkill>();
	if (id == "chase") return std::make_unique<ChaseSkill>();
	if (id == "search") return std::make_unique<SearchSkill>();
	if (id == "tend_body") return std::make_unique<TendBodySkill>();
	if (id == "raise_alarm") return std::make_unique<RaiseAlarmSkill>();

	Logger::Warning("Unknown NPC skill '%s'", id.c_str());
	return nullptr;
}
