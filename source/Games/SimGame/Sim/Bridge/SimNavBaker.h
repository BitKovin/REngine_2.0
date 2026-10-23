#pragma once

// World nav bake, engine shell (addendum 4.3, SIM_CONTRACT section 4). Loads every level of a manifest through the
// engine, which builds or loads that level's nav mesh, copies the live navmesh tiles and the level's sim_portal and
// sim_spawn entities, then runs BakeWorldNav and writes the world file.
//
// Manifest: a text file with one map file name per line (relative to GameData/maps); the first is the layout root.
// Blank lines and lines starting with # are ignored.
//
// Start it with the console command `sim_bake_nav [manifest]` or the launch argument `-sim_bake_nav <manifest>`
// (bakes, then quits). Output: GameData/sim/world.nav and the report in SaveData/sim_bake_nav.log.

#ifndef DISTRIBUTION

#include "SimWorldNavFile.h"

#include <string>

namespace SimNavBaker
{
	constexpr const char* DefaultManifest = SimWorldNavFile::DefaultManifest;
	constexpr const char* OutputPath = SimWorldNavFile::Path;
	constexpr const char* ReportPath = "SaveData/sim_bake_nav.log";

	// Starts a bake; returns false (and logs why) if the manifest cannot be read or a bake is running.
	bool Start(const std::string& manifestPath, bool quitWhenDone);

	// Called every frame by GameStart. Advances one level per finished level load.
	void Update();

	bool IsRunning();
}

#endif
