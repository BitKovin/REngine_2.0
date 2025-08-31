#pragma once

#ifdef __EMSCRIPTEN__

#include <Fmod_ems/fmod.hpp>
#include <Fmod_ems/fmod_common.h>
#include <Fmod_ems/fmod_errors.h>
#include <Fmod_ems/fmod_studio.hpp>

#else

#include <Fmod_desktop/fmod.hpp>
#include <Fmod_desktop/fmod_common.h>
#include <Fmod_desktop/fmod_errors.h>
#include <Fmod_desktop/fmod_studio.hpp>

#endif // __EMSCRIPTEN__