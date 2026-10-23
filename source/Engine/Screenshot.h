#pragma once

// Saving the presented frame as a PNG: the console command `screenshot [name]`, and the launch argument
// `-screenshot <seconds> [-screenshot_file <file>]` (the frame at that much game time, then the run goes on; combine with -quit_after;
// the launch parser takes one value per option, hence the second option for the file name). Works in
// -headless runs too, since the hidden window still has a swap chain. Files go to SaveData/screenshots/ unless the name has a folder.
//
// Needs the bgfx callback from Callback() in bgfx::Init::callback, so it is installed by the platform main before bgfx::init.

#include <bgfx/bgfx.h>

#include <map>
#include <string>
#include <vector>

namespace Screenshot
{
	// The callback object bgfx reports finished screenshots to. Also takes over bgfx's fatal and trace output.
	bgfx::CallbackI* Callback();

	// Asks for a PNG of the next presented frame. "name" becomes SaveData/screenshots/name.png; a name with a folder or an extension is used as given.
	void Request(const std::string& name);

	// Once per frame, just before bgfx::frame(): fires a pending request and the -screenshot launch argument.
	void BeforePresent(double gameSeconds, const std::map<std::string, std::vector<std::string>>& arguments);
}
