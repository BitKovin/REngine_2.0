#pragma once
#include <memory>
#include <string>
#include <thread>

class UiElem;

class UiSystem
{
public:
    static void Init();                  // from EngineMain::Init, after UiElementRegistry::RegisterBuiltinTypes() and Localisation::Initialize
    static void Shutdown();
    static void Update();                // main loop, right before Viewport.Update() (R1): input, handlers, Tick, style, layout
    static void Finalize();              // main loop, right after Viewport.FinalizeChildren() (R1): record + copy snapshots, uploads
    static void AssertUiThread();        // debug: main thread or game thread only (R3)
    static void NoteGameThread();        // called at the top of EngineMain::GameUpdate()
    static void RegisterRoot(const std::shared_ptr<UiElem>& root);
    static void UnregisterRoot(const std::shared_ptr<UiElem>& root);   // pages owned by a menu stack are not roots
    static void QueueEvent(UiElem& element, const std::string& event); // runs the element's on<event> calls at the start of the next Update
    static int FrameIndex();             // counts UiSystem::Update calls
    static UiElem* FindById(const std::string& id);   // first match over all live roots, document order (tests)
    static std::thread::id MainThread();
    static bool IsInitialised();
    static int RootCount();
    static void ResetViewport();         // universal UI reset (level change): clears the viewport, then re-attaches the menu stack
    static void RequestGameThreadTest(const std::string& path);   // console command ui_gamethread_test
};
