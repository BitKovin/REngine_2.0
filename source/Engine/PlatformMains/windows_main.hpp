#define _HAS_STD_BYTE 0
#include "../malloc_override.h"
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include "../imgui/imgui.h"
#include "../Screenshot.h"
#include "../imgui/imgui_impl_bgfx.h"
#include "../imgui/imgui_impl_sdl2.h"
#include <deque>
#include <algorithm>
#include <array>
#include <atomic>
#include <cstdint>
#include <unordered_map>
#include <vector>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_main.h>
#include <direct.h>
#include <Windows.h>
#include <DbgHelp.h>
#pragma comment(lib, "Dbghelp.lib")

#include "../EngineMain.h"
#include "../SoundSystem/SoundManager.hpp"
#include "PlatformWindowData.h"
#include <bgfx/bgfx.h>
using namespace PlatformWindowData;

#pragma message "using windows_main.hpp"

// Global variables
EngineMain* engine = nullptr;

// Function declarations
void update_screen_size(int w, int h);
void InitImGui();
void desktop_render_loop();
bool InitRawInput();
vec2 ReadRawMouseDelta();
void SetRawInputFocus(bool focused);
void ShutdownRawInput();

// Function implementations
void update_screen_size(int w, int h) {
    SDL_SetWindowSize(window, w, h);
}

#include "../imgui/FA6FreeSolidFontData.h"
#include "../imgui/IconsFontAwesome6.h"

// Multi-viewport: lets ImGui windows be dragged out of the game window into their own OS windows.
// It is OFF by default in the engine (Android, Emscripten, ... can't create extra native windows) and each platform main
// opts in explicitly. This is the only switch on Windows: it drives the ImGui config flag, the window style and the renderer backend.
// Headless runs (-headless) switch it off: floating ImGui windows are real OS windows.
static bool kImGuiMultiViewport = true;

// -headless: no window, no OS dialogs, no real input devices. See main().
static bool g_Headless = false;

void InitImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    if (kImGuiMultiViewport) {
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        // io.ConfigViewportsNoTaskBarIcon = true;            // optional: no task bar button per floating window
    }
    ImGui::StyleColorsDark();

    if (kImGuiMultiViewport) {
        // Floating windows are real OS windows and can't be see-through: keep window backgrounds opaque so they look
        // the same inside and outside of the game window.
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // SDL2 platform backend + bgfx renderer backend (no OpenGL context: bgfx owns the graphics API)
    ImGui_ImplSDL2_InitForOther(window);
    ImGui_Implbgfx_Init(255, kImGuiMultiViewport);   // 255 = main ImGui view ID (drawn on top); floating windows use the ids right below it
}

// Raw Input is registered on a dedicated message-only window owned by a
// high-priority thread. SDL's mouse registration is process-wide (one target
// per usage), so this deliberately takes over the mouse Raw Input target after
// SDL relative mode has been initialized. RIDEV_INPUTSINK is used without
// RIDEV_NOLEGACY: SDL's forced warp-relative mode continues receiving ordinary
// WM_MOUSEMOVE / button messages, while the game gets raw counts independently.
static constexpr wchar_t kRawInputWindowClass[] = L"SdlWasmEngineRawMouseInputWindow";

static HANDLE g_RawInputThread = nullptr;
static HANDLE g_RawInputReadyEvent = nullptr;
static DWORD g_RawInputThreadId = 0;
static std::atomic<bool> g_RawInputRegistered{ false };
static DWORD g_RawInputInitError = ERROR_SUCCESS;

// The producer is the Raw Input thread. The consumer can be the main thread
// (initial per-frame sample) or the asynchronous game-update thread (LateUpdate).
// A short SRW lock keeps X/Y as one coherent sample and prevents lost updates.
static SRWLOCK g_RawMouseDeltaLock = SRWLOCK_INIT;
static bool g_RawInputFocused = false; // protected by g_RawMouseDeltaLock
static int64_t g_RawMouseDeltaX = 0;
static int64_t g_RawMouseDeltaY = 0;
static std::atomic<uint64_t> g_RawInputFocusGeneration{ 0 };

static void AddRawMouseDelta(int64_t dx, int64_t dy)
{
    if (dx == 0 && dy == 0)
        return;

    AcquireSRWLockExclusive(&g_RawMouseDeltaLock);
    if (g_RawInputFocused)
    {
        g_RawMouseDeltaX += dx;
        g_RawMouseDeltaY += dy;
    }
    ReleaseSRWLockExclusive(&g_RawMouseDeltaLock);
}

void SetRawInputFocus(bool focused)
{
    AcquireSRWLockExclusive(&g_RawMouseDeltaLock);
    if (g_RawInputFocused != focused)
    {
        g_RawInputFocused = focused;
        // Do not carry movement across focus transitions. This also discards
        // input delivered through RIDEV_INPUTSINK while another app is active.
        g_RawMouseDeltaX = 0;
        g_RawMouseDeltaY = 0;
        g_RawInputFocusGeneration.fetch_add(1, std::memory_order_relaxed);
    }
    ReleaseSRWLockExclusive(&g_RawMouseDeltaLock);
}

vec2 ReadRawMouseDelta()
{
    int64_t dx;
    int64_t dy;

    AcquireSRWLockExclusive(&g_RawMouseDeltaLock);
    dx = g_RawMouseDeltaX;
    dy = g_RawMouseDeltaY;
    g_RawMouseDeltaX = 0;
    g_RawMouseDeltaY = 0;
    ReleaseSRWLockExclusive(&g_RawMouseDeltaLock);

    // These are unaccelerated raw device counts, matching DirectInput's
    // lX/lY units. Sensitivity, /5 scaling and Y inversion remain in the
    // EngineMain callback so both regular and late samples use identical math.
    return vec2(static_cast<float>(dx), static_cast<float>(dy));
}

static void ProcessRawMouseInput(
    const RAWINPUT* rawInput,
    std::unordered_map<HANDLE, POINT>& previousAbsolutePositions,
    uint64_t& lastFocusGeneration)
{
    if (!rawInput || rawInput->header.dwType != RIM_TYPEMOUSE)
        return;

    // RIDEV_INPUTSINK also reports while another application is foreground.
    // AddRawMouseDelta gates accumulation using SDL_WINDOW_INPUT_FOCUS instead
    // of WM_INPUT's foreground code, which also supports an SDL child window
    // embedded in a separate host process.

    const uint64_t focusGeneration = g_RawInputFocusGeneration.load(std::memory_order_relaxed);
    if (focusGeneration != lastFocusGeneration)
    {
        previousAbsolutePositions.clear();
        lastFocusGeneration = focusGeneration;
    }

    const RAWMOUSE& mouse = rawInput->data.mouse;
    int64_t dx = 0;
    int64_t dy = 0;

    if ((mouse.usFlags & MOUSE_MOVE_ABSOLUTE) == 0)
    {
        // Normal physical mice report relative counts. Do not apply Windows
        // pointer acceleration or DPI scaling: Raw Input already provides the
        // same count-based values expected by the old DirectInput path.
        dx = mouse.lLastX;
        dy = mouse.lLastY;
    }
    else
    {
        // Absolute devices (for example some remote-desktop/tablet devices)
        // report normalized screen coordinates. Convert them to screen pixels
        // and derive per-device deltas instead of treating absolute positions
        // as enormous relative movements.
        const bool virtualDesktop = (mouse.usFlags & MOUSE_VIRTUAL_DESKTOP) != 0;
        const int originX = virtualDesktop ? GetSystemMetrics(SM_XVIRTUALSCREEN) : 0;
        const int originY = virtualDesktop ? GetSystemMetrics(SM_YVIRTUALSCREEN) : 0;
        const int width = GetSystemMetrics(virtualDesktop ? SM_CXVIRTUALSCREEN : SM_CXSCREEN);
        const int height = GetSystemMetrics(virtualDesktop ? SM_CYVIRTUALSCREEN : SM_CYSCREEN);
        if (width <= 1 || height <= 1)
            return;

        POINT absolutePosition{};
        absolutePosition.x = originX + MulDiv(static_cast<int>(mouse.lLastX), width - 1, 65535);
        absolutePosition.y = originY + MulDiv(static_cast<int>(mouse.lLastY), height - 1, 65535);

        auto [it, inserted] = previousAbsolutePositions.try_emplace(rawInput->header.hDevice, absolutePosition);
        if (inserted)
            return; // Establish a baseline; the first absolute report is not motion.

        dx = static_cast<int64_t>(absolutePosition.x) - it->second.x;
        dy = static_cast<int64_t>(absolutePosition.y) - it->second.y;
        it->second = absolutePosition;
    }

    AddRawMouseDelta(dx, dy);
}

struct RawInputThreadState
{
    std::vector<BYTE> buffer;
    std::unordered_map<HANDLE, POINT> previousAbsolutePositions;
    uint64_t lastFocusGeneration = 0;
};

// WM_INPUT is always dispatched on the thread that owns the target HWND.
// This pointer is therefore thread-local and needs no cross-thread locking.
static thread_local RawInputThreadState* g_RawInputThreadState = nullptr;

static void DrainRawInputBuffer(RawInputThreadState& state)
{
    for (;;)
    {
        UINT bufferSize = static_cast<UINT>(state.buffer.size());
        const UINT count = GetRawInputBuffer(
            reinterpret_cast<PRAWINPUT>(state.buffer.data()),
            &bufferSize,
            sizeof(RAWINPUTHEADER));

        if (count == 0)
            return;

        if (count == static_cast<UINT>(-1))
        {
            const DWORD error = GetLastError();
            if (error != ERROR_INSUFFICIENT_BUFFER)
                return;

            const size_t requiredSize = std::max<size_t>(
                static_cast<size_t>(bufferSize), state.buffer.size() * 2);
            if (requiredSize <= state.buffer.size())
                return;

            state.buffer.resize(requiredSize);
            continue;
        }

        PRAWINPUT rawInput = reinterpret_cast<PRAWINPUT>(state.buffer.data());
        for (UINT i = 0; i < count; ++i)
        {
            ProcessRawMouseInput(rawInput, state.previousAbsolutePositions, state.lastFocusGeneration);
            const uintptr_t nextAddress =
                reinterpret_cast<uintptr_t>(rawInput) + rawInput->header.dwSize;

            constexpr uintptr_t alignment = sizeof(ULONG_PTR);

            rawInput = reinterpret_cast<PRAWINPUT>(
                (nextAddress + alignment - 1) & ~(alignment - 1));
        }

        // More reports may have arrived while this batch was being processed.
    }
}

static LRESULT CALLBACK RawInputMessageWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_INPUT && g_RawInputThreadState)
    {
        // Read the packet attached to this message first. Then drain any newer
        // reports that accumulated while the thread was entering this handler.
        // This is the recommended hybrid GetRawInputData + GetRawInputBuffer path
        // for high-frequency mice.
        RAWINPUT rawInput{};
        UINT size = sizeof(rawInput);
        const UINT readResult = GetRawInputData(
            reinterpret_cast<HRAWINPUT>(lParam),
            RID_INPUT,
            &rawInput,
            &size,
            sizeof(RAWINPUTHEADER));

        if (readResult != static_cast<UINT>(-1) && readResult >= sizeof(RAWINPUTHEADER))
        {
            ProcessRawMouseInput(
                &rawInput,
                g_RawInputThreadState->previousAbsolutePositions,
                g_RawInputThreadState->lastFocusGeneration);
        }

        DrainRawInputBuffer(*g_RawInputThreadState);

        // RIM_INPUT requires DefWindowProc cleanup. Calling it for RIM_INPUTSINK
        // is harmless and keeps the message path consistent.
        return DefWindowProcW(hwnd, message, wParam, lParam);
    }

    return DefWindowProcW(hwnd, message, wParam, lParam);
}

static DWORD WINAPI RawInputThreadProc(LPVOID)
{
    // High priority is sufficient for prompt delivery without the system-wide
    // starvation risk of THREAD_PRIORITY_TIME_CRITICAL.
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

    RawInputThreadState threadState;
    threadState.buffer.resize(64 * sizeof(RAWINPUT));
    threadState.lastFocusGeneration = g_RawInputFocusGeneration.load(std::memory_order_relaxed);
    g_RawInputThreadState = &threadState;

    const HINSTANCE instance = GetModuleHandleW(nullptr);
    WNDCLASSEXW windowClass{};
    windowClass.cbSize = sizeof(windowClass);
    windowClass.lpfnWndProc = RawInputMessageWindowProc;
    windowClass.hInstance = instance;
    windowClass.lpszClassName = kRawInputWindowClass;

    const ATOM classAtom = RegisterClassExW(&windowClass);
    if (!classAtom)
    {
        g_RawInputInitError = GetLastError();
        g_RawInputThreadState = nullptr;
        SetEvent(g_RawInputReadyEvent);
        return 1;
    }

    HWND rawInputWindow = CreateWindowExW(
        0,
        kRawInputWindowClass,
        L"",
        0,
        0, 0, 0, 0,
        HWND_MESSAGE,
        nullptr,
        instance,
        nullptr);

    if (!rawInputWindow)
    {
        g_RawInputInitError = GetLastError();
        UnregisterClassW(kRawInputWindowClass, instance);
        g_RawInputThreadState = nullptr;
        SetEvent(g_RawInputReadyEvent);
        return 1;
    }

    RAWINPUTDEVICE mouseDevice{};
    mouseDevice.usUsagePage = 0x01; // HID_USAGE_PAGE_GENERIC
    mouseDevice.usUsage = 0x02;     // HID_USAGE_GENERIC_MOUSE
    mouseDevice.dwFlags = RIDEV_INPUTSINK;
    mouseDevice.hwndTarget = rawInputWindow;

    if (!RegisterRawInputDevices(&mouseDevice, 1, sizeof(mouseDevice)))
    {
        g_RawInputInitError = GetLastError();
        DestroyWindow(rawInputWindow);
        UnregisterClassW(kRawInputWindowClass, instance);
        g_RawInputThreadState = nullptr;
        SetEvent(g_RawInputReadyEvent);
        return 1;
    }

    g_RawInputRegistered.store(true, std::memory_order_release);
    SetEvent(g_RawInputReadyEvent);

    // GetMessage removes the current WM_INPUT packet; the window procedure reads
    // it with GetRawInputData and immediately batches any newer queued reports.
    MSG message{};
    BOOL getMessageResult = 0;
    while ((getMessageResult = GetMessageW(&message, nullptr, 0, 0)) > 0)
    {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    // Raw Input registration is process-wide. Remove our mouse registration
    // before the SDL window is destroyed and SDL shuts down.
    RAWINPUTDEVICE removeMouse{};
    removeMouse.usUsagePage = 0x01;
    removeMouse.usUsage = 0x02;
    removeMouse.dwFlags = RIDEV_REMOVE;
    removeMouse.hwndTarget = nullptr;
    RegisterRawInputDevices(&removeMouse, 1, sizeof(removeMouse));

    DestroyWindow(rawInputWindow);
    UnregisterClassW(kRawInputWindowClass, instance);
    g_RawInputThreadState = nullptr;
    return 0;
}

bool InitRawInput()
{
    g_RawInputRegistered.store(false, std::memory_order_relaxed);
    g_RawInputInitError = ERROR_SUCCESS;
    g_RawInputReadyEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    if (!g_RawInputReadyEvent)
    {
        g_RawInputInitError = GetLastError();
        SDL_LogError(SDL_LOG_CATEGORY_INPUT, "CreateEvent for Raw Input failed: %lu", g_RawInputInitError);
        return false;
    }

    g_RawInputThread = CreateThread(
        nullptr, 0, RawInputThreadProc, nullptr, 0, &g_RawInputThreadId);
    if (!g_RawInputThread)
    {
        g_RawInputInitError = GetLastError();
        SDL_LogError(SDL_LOG_CATEGORY_INPUT, "CreateThread for Raw Input failed: %lu", g_RawInputInitError);
        CloseHandle(g_RawInputReadyEvent);
        g_RawInputReadyEvent = nullptr;
        return false;
    }

    WaitForSingleObject(g_RawInputReadyEvent, INFINITE);
    const bool success = g_RawInputRegistered.load(std::memory_order_acquire);
    if (!success)
    {
        SDL_LogError(SDL_LOG_CATEGORY_INPUT, "Raw Input initialization failed: %lu", g_RawInputInitError);
        WaitForSingleObject(g_RawInputThread, INFINITE);
        CloseHandle(g_RawInputThread);
        CloseHandle(g_RawInputReadyEvent);
        g_RawInputThread = nullptr;
        g_RawInputReadyEvent = nullptr;
        g_RawInputThreadId = 0;
        return false;
    }

    SDL_Log("Raw Input mouse thread initialized (buffered, high-priority)");
    return true;
}

void ShutdownRawInput()
{
    if (g_RawInputThread)
    {
        // The thread owns its message queue and window. Post WM_QUIT to that
        // queue, then wait for it to unregister the device and destroy the HWND.
        PostThreadMessageW(g_RawInputThreadId, WM_QUIT, 0, 0);
        WaitForSingleObject(g_RawInputThread, INFINITE);
        CloseHandle(g_RawInputThread);
        g_RawInputThread = nullptr;
    }

    if (g_RawInputReadyEvent)
    {
        CloseHandle(g_RawInputReadyEvent);
        g_RawInputReadyEvent = nullptr;
    }

    g_RawInputThreadId = 0;
    SetRawInputFocus(false);
}

bool pendingResize = false;

// True for window-scoped events delivered to a window other than the game window, i.e. a floating ImGui window.
// Those are for ImGui only and must not reach the game (mouse clicks, typing, resize...).
static bool IsEventForOtherWindow(const SDL_Event& e, Uint32 mainWindowId) {
    Uint32 id = 0;
    switch (e.type) {
    case SDL_WINDOWEVENT:                             id = e.window.windowID; break;
    case SDL_KEYDOWN: case SDL_KEYUP:                 id = e.key.windowID;    break;
    case SDL_TEXTEDITING:                             id = e.edit.windowID;   break;
    case SDL_TEXTINPUT:                               id = e.text.windowID;   break;
    case SDL_MOUSEMOTION:                             id = e.motion.windowID; break;
    case SDL_MOUSEBUTTONDOWN: case SDL_MOUSEBUTTONUP: id = e.button.windowID; break;
    case SDL_MOUSEWHEEL:                              id = e.wheel.windowID;  break;
    default: return false;
    }
    return id != 0 && id != mainWindowId;
}

void desktop_render_loop() {
    SDL_Event event;
    int quit = 0;
    const Uint32 mainWindowId = SDL_GetWindowID(window);
    int currentWidth = 800;
    int currentHeight = 600;

    while (!quit)
    {
        Input::PendingMouseDelta = vec2();
        Input::StartEventsFrame();
        SetRawInputFocus((SDL_GetWindowFlags(window) & SDL_WINDOW_INPUT_FOCUS) != 0);

        while (SDL_PollEvent(&event))
        {
            if (EngineMain::MainInstance->DebugUiEnabled)
                ImGui_ImplSDL2_ProcessEvent(&event);

            if (event.type == SDL_QUIT) quit = 1;

            // With floating ImGui windows open SDL doesn't send SDL_QUIT when the game window is closed
            // (it only does that for the last window), so handle the close request of the main window explicitly.
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
                event.window.windowID == mainWindowId) quit = 1;

            // Track focus transitions immediately instead of waiting until the
            // next frame; the Raw Input thread filters sink/background packets.
            if (event.type == SDL_WINDOWEVENT && event.window.windowID == mainWindowId)
            {
                if (event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
                    SetRawInputFocus(true);
                else if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
                    SetRawInputFocus(false);
            }

            // Events for floating ImGui windows were already given to ImGui above.
            if (IsEventForOtherWindow(event, mainWindowId)) continue;

            // BGFX resize handling (high-DPI aware)
            if (event.type == SDL_WINDOWEVENT &&
                (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED ||
                    event.window.event == SDL_WINDOWEVENT_RESIZED))
            {
                SDL_GetWindowSize(window, &currentWidth, &currentHeight);
                pendingResize = true;
            }

            Input::ReceiveSdlEvent(event);
        }

        // SDL has finished pumping the window's events. Use Raw Input as the
        // authoritative game look delta (rather than adding SDL motion a second
        // time). Reports arriving during GameUpdate/LateUpdate remain available
        // for PullAdditionalMouseDelta().
        const bool hasInputFocus = (SDL_GetWindowFlags(window) & SDL_WINDOW_INPUT_FOCUS) != 0;
        SetRawInputFocus(hasInputFocus);
        if (hasInputFocus)
            Input::PendingMouseDelta = ReadRawMouseDelta();

        engine->MainLoop();

    }
}

// Crash handler (unchanged)
static bool g_SymbolsInitialized = false;
static bool g_SymbolsAvailable = false;

void EnsureDbgHelpInitialized()
{
    if (g_SymbolsInitialized)
        return;
    SymSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_UNDNAME | SYMOPT_LOAD_LINES);
    if (SymInitialize(GetCurrentProcess(), nullptr, TRUE))
    {
        g_SymbolsAvailable = true;
    }
    else
    {
        g_SymbolsAvailable = false;
    }
    g_SymbolsInitialized = true;
}

LONG WINAPI EngineUnhandledExceptionFilter(EXCEPTION_POINTERS* pExceptionPointers)
{
    EnsureDbgHelpInitialized();
    SYSTEMTIME st;
    GetLocalTime(&st);
    std::wostringstream dumpName;
    dumpName << L"CrashDump_"
        << std::setw(4) << std::setfill(L'0') << st.wYear
        << std::setw(2) << std::setfill(L'0') << st.wMonth
        << std::setw(2) << std::setfill(L'0') << st.wDay
        << L"_"
        << std::setw(2) << std::setfill(L'0') << st.wHour
        << std::setw(2) << std::setfill(L'0') << st.wMinute
        << std::setw(2) << std::setfill(L'0') << st.wSecond
        << L".dmp";
    std::wstring dumpPath = dumpName.str();
    HANDLE hDumpFile = CreateFileW(
        dumpPath.c_str(),
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );
    if (hDumpFile != INVALID_HANDLE_VALUE)
    {
        MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
        dumpInfo.ThreadId = GetCurrentThreadId();
        dumpInfo.ExceptionPointers = pExceptionPointers;
        dumpInfo.ClientPointers = TRUE;
        MiniDumpWriteDump(
            GetCurrentProcess(),
            GetCurrentProcessId(),
            hDumpFile,
            MiniDumpNormal,
            &dumpInfo,
            nullptr,
            nullptr
        );
        CloseHandle(hDumpFile);
    }
    DWORD exceptionCode = pExceptionPointers->ExceptionRecord->ExceptionCode;
    void* exceptionAddr = pExceptionPointers->ExceptionRecord->ExceptionAddress;
    const USHORT MaxFrames = 62;
    void* backTrace[MaxFrames];
    USHORT framesCaptured = CaptureStackBackTrace(0, MaxFrames, backTrace, nullptr);
    std::wostringstream msg;
    msg << L"An unhandled exception has occurred!\n\n";
    msg << L"Exception code: 0x" << std::hex << exceptionCode << std::dec << L"\n";
    msg << L"Exception address: 0x"
        << std::hex << reinterpret_cast<uintptr_t>(exceptionAddr) << std::dec << L"\n";
    msg << L"Crash time: "
        << std::setw(4) << std::setfill(L'0') << st.wYear << L"-"
        << std::setw(2) << std::setfill(L'0') << st.wMonth << L"-"
        << std::setw(2) << std::setfill(L'0') << st.wDay << L" "
        << std::setw(2) << std::setfill(L'0') << st.wHour << L":"
        << std::setw(2) << std::setfill(L'0') << st.wMinute << L":"
        << std::setw(2) << std::setfill(L'0') << st.wSecond << L"\n\n";
    if (!g_SymbolsAvailable)
    {
        msg << L"*** Symbols not available. Displaying raw addresses only ***\n\n";
    }
    else
    {
        msg << L"Call stack (symbolicated where possible):\n\n";
    }
    for (USHORT i = 2; i < framesCaptured; ++i)
    {
        DWORD64 addr = reinterpret_cast<DWORD64>(backTrace[i]);
        std::wostringstream line;
        line << L"[" << (i - 2) << L"] ";
        if (g_SymbolsAvailable)
        {
            DWORD64 displacementSym = 0;
            DWORD displacementLine = 0;
            BYTE symbolBuffer[sizeof(SYMBOL_INFO) + (MAX_SYM_NAME * sizeof(WCHAR))];
            PSYMBOL_INFO pSymbol = reinterpret_cast<PSYMBOL_INFO>(symbolBuffer);
            pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
            pSymbol->MaxNameLen = MAX_SYM_NAME;
            IMAGEHLP_LINEW64 lineInfo = {};
            lineInfo.SizeOfStruct = sizeof(IMAGEHLP_LINEW64);
            bool gotSymbol = SymFromAddr(GetCurrentProcess(), addr, &displacementSym, pSymbol) != FALSE;
            bool gotLineInfo = SymGetLineFromAddrW64(GetCurrentProcess(), addr, &displacementLine, &lineInfo) != FALSE;
            if (gotSymbol)
            {
                line << pSymbol->Name << L" +0x"
                    << std::hex << displacementSym << std::dec;
                if (gotLineInfo)
                {
                    line << L" (" << lineInfo.FileName << L":" << lineInfo.LineNumber << L")";
                }
            }
            else
            {
                line << L"0x" << std::hex << addr << std::dec;
            }
        }
        else
        {
            line << L"0x" << std::hex << addr << std::dec;
        }
        msg << line.str() << L"\n";
    }
    msg << L"\nMinidump saved to:\n" << dumpPath;
    if (g_Headless)
    {
        // Automated run: a message box would block the run and pop up on the user's desktop.
        fwprintf(stderr, L"%ls\n", msg.str().c_str());
        fflush(stderr);
    }
    else
    {
        MessageBoxW(
            nullptr,
            msg.str().c_str(),
            L"Application Crash",
            MB_ICONERROR | MB_OK
        );
    }
    return EXCEPTION_EXECUTE_HANDLER;
}

// Main function
int main(int argc, char* args[])
{
    EnsureDbgHelpInitialized();
    SetUnhandledExceptionFilter(EngineUnhandledExceptionFilter);

    std::vector<std::string> args_s(args, args + argc);
    auto args_m = EngineMain::ParseCommands(args_s);
    for (auto a : args_m)
    {
        Logger::Log(a.first);
        for (auto o : a.second)
        {
            Logger::Log("arg: " + o);
        }
    }

    auto workingDirOverride = args_m.find("working_dir");
    if (workingDirOverride != args_m.end())
    {
        Logger::Log("switching working directory to " + workingDirOverride->second[0]);
        _chdir(workingDirOverride->second[0].c_str());
    }

    // -headless: for automated runs that must not disturb the user. The window is created hidden (never shown, no focus
    // steal; a real one because Direct3D needs a native handle, so SDL's dummy video driver is no option here), SDL
    // audio uses its dummy driver, there is no multi-viewport ImGui, no Raw Input thread / mouse capture, and sound is muted.
    // Checked on the raw arguments: ParseCommands splits a single-dash all-letter flag like -headless into letters.
    g_Headless = std::find(args_s.begin(), args_s.end(), "-headless") != args_s.end() ||
        std::find(args_s.begin(), args_s.end(), "--headless") != args_s.end();
    if (g_Headless)
    {
        SDL_setenv("SDL_AUDIODRIVER", "dummy", 1);
        kImGuiMultiViewport = false;
        SoundManager::GlobalVolume = 0.0f;
        Logger::Log("headless mode");
    }

    bgfx::RendererType::Enum renderApi = bgfx::RendererType::Direct3D11;   // default to Direct3D11 on Windows

    auto renderApiOverride = args_m.find("renderapi");
    if (renderApiOverride != args_m.end())
    {
        const std::string& renderApiStr = renderApiOverride->second[0];
        if (renderApiStr == "dx11")
        {
            renderApi = bgfx::RendererType::Direct3D11;
            Logger::Log("Using Direct3D11 renderer");
        }
        else if (renderApiStr == "dx12")
        {
            renderApi = bgfx::RendererType::Direct3D12;
            Logger::Log("Using Direct3D12 renderer");
        }
        else if (renderApiStr == "vk")
        {
            renderApi = bgfx::RendererType::Vulkan;
            Logger::Log("Using Vulkan renderer");
        }
        else if (renderApiStr == "gl")
        {
            renderApi = bgfx::RendererType::OpenGL;
            Logger::Log("Using OpenGL renderer");
        }
        else if (renderApiStr == "noop")
        {
            // No GPU work at all. It cannot find compiled shaders (they are looked up per renderer type), so only use
            // it for runs that never render.
            renderApi = bgfx::RendererType::Noop;
            Logger::Log("Using Noop renderer");
        }
        else
        {
            Logger::Log("Unknown render API specified: " + renderApiStr + ", using default");
        }
    }

    // -lock_window: a tool (UI Studio) has put the window inside its own; the click that gives the window focus must still
    // reach the game, otherwise the first click after using another panel is lost.
    if (std::find(args_s.begin(), args_s.end(), "-lock_window") != args_s.end())
        SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) < 0) {
        fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    // ====================== WINDOW (no OpenGL flags) ======================
    int flags = (g_Headless ? SDL_WINDOW_HIDDEN : SDL_WINDOW_SHOWN) | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE;
    window = SDL_CreateWindow("Image", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, flags);
    if (!window) {
        fprintf(stderr, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    // ====================== BGFX INITIALIZATION (replaces all OpenGL) ======================
    bgfx::Init init;
    init.type = renderApi;
    init.callback = Screenshot::Callback();   // saving frames: console `screenshot`, launch -screenshot <seconds> <file>
    init.debug = false;
    init.profile = false;


    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    if (!SDL_GetWindowWMInfo(window, &wmInfo)) {
        fprintf(stderr, "SDL_GetWindowWMInfo failed: %s\n", SDL_GetError());
        return 1;
    }
    init.swapChain.nwh = wmInfo.info.win.window;   // Windows native handle (of the hidden window when headless)

    init.swapChain.width = 800;
    init.swapChain.height = 600;
    init.swapChain.flags = BGFX_RESET_NONE;   // no vsync (matches your old SDL_GL_SetSwapInterval(0))


    if (!bgfx::init(init)) {
        fprintf(stderr, "bgfx::init failed!\n");
        return 1;
    }

    BgfxResetManager::swapChainPtr = &init.swapChain;

    //bgfx::setDebug(BGFX_DEBUG_TEXT | BGFX_DEBUG_STATS);

    //bgfx::setDebug(BGFX_DEBUG_STATS);
    // Default clear + views
    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x000000ff, 1.0f, 0);
    bgfx::setViewRect(0, 0, 0, 800, 600);
    bgfx::setViewRect(255, 0, 0, 800, 600);

    InitImGui();

    SDL_SetHintWithPriority(SDL_HINT_MOUSE_RELATIVE_MODE_WARP, "1", SDL_HINT_OVERRIDE);
    SDL_SetHintWithPriority(SDL_HINT_MOUSE_RELATIVE_MODE_CENTER, "1", SDL_HINT_OVERRIDE);
    if (!g_Headless)    // would capture the user's real cursor
    {
        SDL_SetRelativeMouseMode(SDL_TRUE);

        // Start after SDL relative mode: Raw Input registration is process-wide,
        // and our message-only window becomes the mouse input target. The forced
        // SDL warp mode above keeps legacy window mouse events working for SDL/UI.
        InitRawInput();
    }

    Input::AddAction("test")->AddKeyboardKey(SDL_GetScancodeFromKey(SDLK_t));
    Input::AddAction("fullscreen")->AddKeyboardKey(SDL_GetScancodeFromKey(SDLK_F11));

    engine = new EngineMain(window);
    engine->asyncGameUpdate = ThreadPool::Supported();

    engine->SetAdditionalMouseDeltaCallback([]() -> glm::vec2
        {
            return ReadRawMouseDelta();
        });

    EngineMain::MainInstance = engine;
    engine->Init(args_s);

    desktop_render_loop();

    delete engine;
    ShutdownRawInput();

    // Renderer backend first (releases the swap chains of floating windows), then the platform backend (closes their OS windows).
    ImGui_Implbgfx_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    bgfx::frame();      // final present before shutdown
    //bgfx::shutdown();   // replaces SDL_GL_DeleteContext

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}