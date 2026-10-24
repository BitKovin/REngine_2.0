#pragma once

#ifndef DISTRIBUTION

#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <string>

namespace sim::test
{
	// Collects check results for one run. Failures carry the current section name, so the log reads as a story.
	struct TestContext
	{
		int passed = 0;
		int failed = 0;
		std::string section;
		std::string report;

		void Section(const char* name)
		{
			section = name;
			report += "== ";
			report += name;
			report += "\n";
		}

		bool Check(bool condition, const char* format, ...)
		{
			if (condition)
			{
				++passed;
				return true;
			}
			++failed;
			char buffer[1024];
			va_list args;
			va_start(args, format);
			std::vsnprintf(buffer, sizeof(buffer), format, args);
			va_end(args);
			report += "  FAIL [" + section + "] " + buffer + "\n";
			return false;
		}

		void Note(const char* format, ...)
		{
			char buffer[1024];
			va_list args;
			va_start(args, format);
			std::vsnprintf(buffer, sizeof(buffer), format, args);
			va_end(args);
			report += "  ";
			report += buffer;
			report += "\n";
		}
	};

	struct Stopwatch
	{
		std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
		double Seconds() const { return std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count(); }
	};

	// Test suites (one file each).
	void RunKernelTests(TestContext& context);
	void RunNavTests(TestContext& context);
	void RunMovementTests(TestContext& context);
	void RunBridgeTests(TestContext& context);
	void RunBodiesTests(TestContext& context);
	void RunNpcTests(TestContext& context);
	void RunTransitionTests(TestContext& context);
	void RunCharacterTests(TestContext& context);
}

#endif
