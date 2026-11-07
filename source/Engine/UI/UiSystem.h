#pragma once
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <glm/glm.hpp>

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
    static std::vector<std::shared_ptr<UiElem>> Roots();   // live roots in registration order (inspection, lint)
    static UiElem* FindById(const std::string& id);   // first match over all live roots, document order (tests)
    static void RecordSubtree(UiElem& source, UiElem& host, float opacity, glm::vec2 offset);   // paints `source` and its children into `host` (an unattached root), shifted and faded as one picture (the drag ghost)
    static std::thread::id MainThread();
    static bool IsInitialised();
    static int RootCount();
    static void ResetViewport();         // universal UI reset (level change): clears the viewport, then re-attaches the menu stack
    static void RequestGameThreadTest(const std::string& path);   // console command ui_gamethread_test

    // Accessibility. The root font size (rem) is 24 x TextScale; changing it restyles every root.
    static inline float TextScale = 1.f;            // setting ui.text_scale, 0.75 .. 1.5
    static void SetTextScale(float scale);
    static bool ReducedMotion();                    // setting ui.reduced_motion
    static void SetReducedMotion(bool reduced);
    static bool PseudoLoc();                        // -ui_pseudoloc: every localised string is accented, padded and bracketed
    static std::string PseudoLocalise(const std::string& text);
};
