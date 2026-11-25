// Every scenario the suite knows; each tier registers its own in its file. Planned scenarios are in NPC_AI_DESIGN.md §25.

#include "../AiTest.h"

#ifndef DISTRIBUTION

namespace AiTest
{
	void RegisterSenseScenarios();
	void RegisterChaseScenarios();
	void RegisterStealthScenarios();
	void RegisterSocialScenarios();

	void RegisterAllScenarios()
	{
		RegisterSenseScenarios();
		RegisterChaseScenarios();
		RegisterStealthScenarios();
		RegisterSocialScenarios();
	}
}

#endif
