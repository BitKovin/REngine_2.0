#pragma once

#include <string>

#ifndef DISTRIBUTION
// Pure-logic combat tests run inside the real engine (no stubs): combo resolver timing, reactions, poise, status
// effects, attack tokens, moveset parsing / validation / round trip. Returns true if everything passed.
bool RunCombatSelfTests(std::string& summary);
#endif
