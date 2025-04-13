#include "EngineMain.h"



EngineMain* EngineMain::MainInstance = nullptr;

UiViewport EngineMain::Viewport;

#if __EMSCRIPTEN__

#include "emscripten/emscripten.h"

EM_JS(int, canvas_get_width, (), {
return canvas.width;
    });
EM_JS(int, canvas_get_height, (), {
  return canvas.height;
    });

#endif // __EMSCRIPTEN__

void EngineMain::UpdateScreenSize()
{
#if __EMSCRIPTEN__

    int width = canvas_get_width();
    int height = canvas_get_height();


    ScreenSize = ivec2(width, height);

#else

    int w, h;

    SDL_GetWindowSize(Window, &w, &h);


    ScreenSize.x = w;
    ScreenSize.y = h;

#endif // __EMSCRIPTEN__
}

void EngineMain::InitInputs()
{
    Input::AddAction("forward")->AddKeyboardKey(SDL_GetScancodeFromKey(SDL_KeyCode::SDLK_w));
    Input::AddAction("backward")->AddKeyboardKey(SDL_GetScancodeFromKey(SDL_KeyCode::SDLK_s));
    Input::AddAction("left")->AddKeyboardKey(SDL_GetScancodeFromKey(SDL_KeyCode::SDLK_a));
    Input::AddAction("right")->AddKeyboardKey(SDL_GetScancodeFromKey(SDL_KeyCode::SDLK_d));

    Input::AddAction("jump")->AddKeyboardKey(SDL_GetScancodeFromKey(SDL_KeyCode::SDLK_SPACE));

    Input::AddAction("click")->LMB = true;

    Input::AddAction("attack")->AddButton(5)->LMB = true;

    int maxUniforms;

    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxUniforms);

    printf("max uniforms: %i \n", maxUniforms);

}