#include "VideoSettingsData.h"

#include <SDL2/SDL.h>
#include <EngineMain.h>
#include <BgfxResetManager.h>

#include <algorithm>
#include <cstdio>

void VideoSettingsData::ApplyToEngine() const
{
    SDL_Window* gWindow = EngineMain::MainInstance->Window;
    if (!gWindow) return;

#if !defined(__EMSCRIPTEN__) && !defined(__ANDROID__)




    // Window mode first, same three-way switch UiVideoSettings::UpdateWindowMode uses.
    if (WindowMode == "fullscreen")
        SDL_SetWindowFullscreen(gWindow, SDL_WINDOW_FULLSCREEN);
    else if (WindowMode == "borderless")
        SDL_SetWindowFullscreen(gWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
    else
        SDL_SetWindowFullscreen(gWindow, 0);

    Uint32 flags = SDL_GetWindowFlags(gWindow);
    if (flags & SDL_WINDOW_FULLSCREEN)
    {
        SDL_DisplayMode mode;
        mode.format = SDL_PIXELFORMAT_UNKNOWN;
        mode.w = Width;
        mode.h = Height;
        mode.refresh_rate = 0;
        mode.driverdata = nullptr;
        SDL_SetWindowDisplayMode(gWindow, &mode);
    }
    else
    {
        SDL_SetWindowSize(gWindow, Width, Height);
    }

#endif // !__EMSCRIPTEN__ ! __ANDROID__

    if (EngineMain::MainInstance->MainRenderer)
    {
        EngineMain::MainInstance->MainRenderer->MultiSampleCount = MSAA;
        EngineMain::MainInstance->MainRenderer->FXAAEnabled = FXAA;
    }

    BgfxResetManager::SetVSync(VSync);

    EngineMain::MainInstance->MainRenderer->DynamicShadows = DynamicShadows;

}

void VideoSettingsData::FromCurrentState()
{
    SDL_Window* gWindow = EngineMain::MainInstance->Window;
    if (!gWindow) return;

    // Resolution: same source UiVideoSettings/VideoSettings::InitModelData
    // treat as the authoritative "current resolution" -- not the raw SDL
    // window size, which can lag behind in exclusive fullscreen.
    Width = EngineMain::MainInstance->ScreenSize.x;
    Height = EngineMain::MainInstance->ScreenSize.y;

    // Window mode: FULLSCREEN_DESKTOP's bits are a superset of plain
    // FULLSCREEN, so it must be checked with an exact match first, or
    // borderless windows get misread as exclusive fullscreen. Same order
    // UiVideoSettings uses to pick the dropdown's starting index.
    Uint32 flags = SDL_GetWindowFlags(gWindow);
    if ((flags & SDL_WINDOW_FULLSCREEN_DESKTOP) == SDL_WINDOW_FULLSCREEN_DESKTOP)
        WindowMode = "borderless";
    else if (flags & SDL_WINDOW_FULLSCREEN)
        WindowMode = "fullscreen";
    else
        WindowMode = "windowed";

    if (EngineMain::MainInstance->MainRenderer)
    {
        MSAA = EngineMain::MainInstance->MainRenderer->MultiSampleCount;
        FXAA = EngineMain::MainInstance->MainRenderer->FXAAEnabled;
    }

    VSync = BgfxResetManager::GetVSync();

    DynamicShadows = EngineMain::MainInstance->MainRenderer->DynamicShadows;

}

std::string VideoSettingsData::CurrentResolution()
{
    return std::to_string(EngineMain::MainInstance->ScreenSize.x) + "x" + std::to_string(EngineMain::MainInstance->ScreenSize.y);
}

std::vector<std::string> VideoSettingsData::ResolutionList()
{
    std::vector<std::string> list;
    SDL_Window* window = EngineMain::MainInstance->Window;
    if (window != nullptr)
    {
        int display = SDL_GetWindowDisplayIndex(window);
        if (display < 0)
            display = 0;
        const int modes = SDL_GetNumDisplayModes(display);
        for (int i = 0; i < modes; ++i)
        {
            SDL_DisplayMode mode;
            if (SDL_GetDisplayMode(display, i, &mode) != 0)
                continue;
            const std::string text = std::to_string(mode.w) + "x" + std::to_string(mode.h);
            if (std::find(list.begin(), list.end(), text) == list.end())   // SDL lists a size once per refresh rate
                list.push_back(text);
        }
    }
    const std::string current = CurrentResolution();
    if (std::find(list.begin(), list.end(), current) == list.end())
        list.push_back(current);
    std::sort(list.begin(), list.end(), [](const std::string& a, const std::string& b)
        {
            int aw = 0, ah = 0, bw = 0, bh = 0;
            std::sscanf(a.c_str(), "%dx%d", &aw, &ah);
            std::sscanf(b.c_str(), "%dx%d", &bw, &bh);
            return aw * ah > bw * bh;
        });
    return list;
}

int VideoSettingsData::WindowModeIndex()
{
    SDL_Window* window = EngineMain::MainInstance->Window;
    if (window == nullptr)
        return 0;
    const Uint32 flags = SDL_GetWindowFlags(window);
    if ((flags & SDL_WINDOW_FULLSCREEN_DESKTOP) == SDL_WINDOW_FULLSCREEN_DESKTOP)
        return 2;   // borderless needs all of its bits
    if (flags & SDL_WINDOW_FULLSCREEN)
        return 1;
    return 0;
}
