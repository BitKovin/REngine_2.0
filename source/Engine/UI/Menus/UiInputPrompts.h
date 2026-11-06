#pragma once
#include <SDL2/SDL.h>
#include <functional>
#include <string>

enum class UiInputDevice { KeyboardMouse, Gamepad, Touch };

// Tracks the device the player used last and resolves action names to prompt images supplied by the game.
namespace UiInputPrompts
{
    using Provider = std::function<std::string(const std::string& action, UiInputDevice device)>;

    UiInputDevice LastDevice();
    void SetLastDevice(UiInputDevice device);       // tests and games that know better
    uint32_t DeviceRevision();                      // changes whenever the device does
    const char* DeviceClass(UiInputDevice device);  // "input-mouse", "input-gamepad", "input-touch"

    void SetProvider(Provider provider);
    bool HasProvider();
    std::string Resolve(const std::string& action); // image path for the current device; empty = no prompt
    std::string ResolveFor(const std::string& action, UiInputDevice device);

    void OnSdlEvent(const SDL_Event& event);        // from Input::ReceiveSdlEvent
}
