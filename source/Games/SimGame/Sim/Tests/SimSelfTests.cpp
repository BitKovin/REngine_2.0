#include "SimSelfTests.h"

#ifndef DISTRIBUTION

#include "SimTestFramework.h"

#include <Console/ConsoleRegister.h>
#include <FileSystem/FileSystem.h>
#include <Logger.hpp>

bool RunSimSelfTests(std::string& summary)
{
	using namespace sim::test;

	Stopwatch timer;
	TestContext context;
	RunKernelTests(context);
	RunNavTests(context);
	RunMovementTests(context);
	RunBridgeTests(context);
	RunBodiesTests(context);
	RunNpcTests(context);
	RunTransitionTests(context);
	RunCharacterTests(context);

	char header[160];
	std::snprintf(header, sizeof(header), "sim self tests: %d passed, %d failed, %.2f s\n", context.passed, context.failed, timer.Seconds());
	summary = header + context.report;
	return context.failed == 0;
}

CONSOLE_FUNC("sim_selftest", "Run the sim self tests; writes SaveData/sim_selftest.log")
{
	std::string summary;
	const bool passed = RunSimSelfTests(summary);
	FileSystemEngine::WriteFile("SaveData/sim_selftest.log", summary);
	Logger::Log(passed ? "sim self tests PASSED" : "sim self tests FAILED (see SaveData/sim_selftest.log)");
}

#endif
