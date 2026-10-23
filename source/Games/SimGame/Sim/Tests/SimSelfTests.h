#pragma once

#include <string>

#ifndef DISTRIBUTION
// Headless sim tests run inside the game executable (-sim_selftest, or the sim_selftest console command). Pure sim
// logic, no level or rendering needed. Returns true if everything passed; `summary` is the text report.
bool RunSimSelfTests(std::string& summary);
#endif
