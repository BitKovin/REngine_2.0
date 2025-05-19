#if __EMSCRIPTEN__

#include "PlatformMains/emscripten_main.hpp"

#elif _WINDOWS

#include "PlatformMains/windows_main.hpp"

#elif _linux

#include "PlatformMains/linux_main.hpp"

#endif