#include "UiInputPrompts.h"

#include <cmath>

namespace
{
    UiInputDevice g_device = UiInputDevice::KeyboardMouse;
    uint32_t g_revision = 1;
    UiInputPrompts::Provider g_provider;
}

UiInputDevice UiInputPrompts::LastDevice() { return g_device; }
uint32_t UiInputPrompts::DeviceRevision() { return g_revision; }

void UiInputPrompts::SetLastDevice(UiInputDevice device)
{
    if (device == g_device)
        return;
    g_device = device;
    ++g_revision;
}

const char* UiInputPrompts::DeviceClass(UiInputDevice device)
{
    switch (device)
    {
    case UiInputDevice::Gamepad: return "input-gamepad";
    case UiInputDevice::Touch: return "input-touch";
    default: return "input-mouse";
    }
}

void UiInputPrompts::SetProvider(Provider provider)
{
    g_provider = std::move(provider);
    ++g_revision;
}

bool UiInputPrompts::HasProvider() { return (bool)g_provider; }

std::string UiInputPrompts::Resolve(const std::string& action)
{
    return ResolveFor(action, g_device);
}

std::string UiInputPrompts::ResolveFor(const std::string& action, UiInputDevice device)
{
    return g_provider ? g_provider(action, device) : std::string();
}

void UiInputPrompts::OnSdlEvent(const SDL_Event& event)
{
    switch (event.type)
    {
    case SDL_KEYDOWN:
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEWHEEL:
        SetLastDevice(UiInputDevice::KeyboardMouse);
        break;
    case SDL_MOUSEMOTION:
        if (std::abs(event.motion.xrel) + std::abs(event.motion.yrel) > 2)
            SetLastDevice(UiInputDevice::KeyboardMouse);
        break;
    case SDL_CONTROLLERBUTTONDOWN:
        SetLastDevice(UiInputDevice::Gamepad);
        break;
    case SDL_CONTROLLERAXISMOTION:
        if (std::abs((float)event.caxis.value) / 32767.f > 0.3f)
            SetLastDevice(UiInputDevice::Gamepad);
        break;
    case SDL_FINGERDOWN:
    case SDL_FINGERMOTION:
        SetLastDevice(UiInputDevice::Touch);
        break;
    default:
        break;
    }
}
