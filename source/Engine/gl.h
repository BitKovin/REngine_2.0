#pragma once
#ifndef __EMSCRIPTEN__
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#else
#include <SDL2/SDL_opengles2.h>
#include <GLES3/gl31.h>

#define glTexImage2DMultisample glTexStorage2DMultisample
#define GL_ES_PROFILE

#endif
