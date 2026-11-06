#pragma once
#include <UI/UiElement.h>
#include <UI/Render/UiDrawList.h>
#include <UI/Render/UiPainter.h>
#include <UI/Style/UiComputedStyle.h>
#include <UI/Menus/UiActions.h>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

class UiDocumentInstance;
class UiMenuStack;
struct UiDocumentDef;

enum UiElemState : uint32_t { kStateHover = 1, kStateActive = 2, kStateDisabled = 4, kStateSelected = 8, kStateEntering = 16, kStateLeaving = 32, kStateChecked = 64 };

class UiElem : public UiElement
{
public:
    UiElem();
    ~UiElem() override;

    // ── identity
    std::string TagName;                 // "Panel", "Label", ... (set by the factory)
    std::string ComponentTag;            // set on a component instance's top element
    std::string Id;
    std::string ScopeFile;               // .xml that created this element
    UiDocumentInstance* Instance = nullptr;              // id scope this element belongs to
    std::shared_ptr<UiDocumentInstance> OwnedInstance;   // set on document roots and component instance tops
    bool InstanceRoot = false;           // top element of a document instance (:root)

    // ── API usable from code: main thread or game thread (R3). Calls only change state and set dirty flags.
    static std::shared_ptr<UiElem> Load(const std::string& path);   // creates a root from a layout file
    bool LoadLayout(const std::string& path);                       // applies a layout file onto this element (code-behind)
    UiElem* FindElem(const std::string& idPath) const;              // "id" or "a/b" path; logs on miss
    template <class T = UiElem> T* Find(const std::string& idPath) const
    {
        UiElem* found = FindElem(idPath);
        if (found == nullptr)
            return nullptr;
        T* typed = dynamic_cast<T*>(found);   // load-time use only
        if (typed == nullptr)
            LogFindTypeMismatch(idPath);
        return typed;
    }
    void SetVar(const std::string& name, const std::string& value);
    void SetVar(const std::string& name, double value);
    void SetClass(const std::string& cls, bool on);
    void AddClass(const std::string& cls) { SetClass(cls, true); }
    void RemoveClass(const std::string& cls) { SetClass(cls, false); }
    void ToggleClass(const std::string& cls) { SetClass(cls, !HasClass(cls)); }
    bool HasClass(const std::string& cls) const;
    void SetAttribute(const std::string& name, const std::string& value);  // re-applies, marks dirty
    UiComputedStyle& Style();            // C++ overrides; fields that differ from the defaults win over CSS
    const UiComputedStyle& Computed() const { return m_computed; }
    void AddChild(std::shared_ptr<UiElement> child) override;
    void RemoveChild(std::shared_ptr<UiElement> child) override;
    void ClearChildren() override;

    // ── results of layout (valid after UiSystem::Update)
    UiRect BorderRect;                   // relative to the parent's border-box top-left
    UiRect ContentRectLocal() const;     // inside BorderRect, relative to BorderRect
    UiRect ContentRectScreen() const;    // UI units (axis-aligned bound)
    glm::mat3 AbsoluteTransform = glm::mat3(1.f);   // element-local -> UI units

    // ── subclass hooks
    virtual bool ApplyAttribute(const std::string& name, const std::string& value) { return false; } // true = handled
    virtual glm::vec2 MeasureContent(float availableWidth, float availableHeight) { return glm::vec2(0.f); }
    virtual void PaintContent(UiPainter& painter, const UiRect& contentRect) {}
    virtual void OnActivate() {}         // runs before the "activate" event is queued
    virtual void Tick() {}               // per frame, main thread, before style/layout (used from phase 5)
    virtual bool IsFocusable() const { return false; }
    virtual bool HasContent() const { return false; }   // true: MeasureContent contributes to fit-children

    // ── menu lifecycle (a page is any element pushed on a UiMenuStack; hosts that are not stacks ignore these)
    virtual void OnPushed() {}
    virtual void OnShown() {}
    virtual void OnHidden() {}
    virtual void OnPopped() {}
    virtual bool OnBack() { return false; }      // true = handled, the stack does not pop
    virtual UiMenuStack* AsMenuStack() { return nullptr; }
    UiMenuStack* Stack() const;                  // the stack this element is a descendant of; nullptr if none
    UiElem* PageOf();                            // the child of the stack that contains this element; nullptr if not on a stack
    struct HostOptions { bool CloseOnBack = true, ShowsBelow = false, PausesGame = false, HidesHud = false; };
    HostOptions ReadHostOptions() const;         // from the attributes close-on-back, shows-below, pauses-game, hides-hud

    // ── events and actions
    using ActionFn = std::function<void(UiElem& sender, const std::vector<std::string>& args)>;
    struct Handler { ActionFn Fn; std::weak_ptr<void> Guard; bool Guarded = false; };
    void On(const std::string& name, std::function<void()> fn);
    void On(const std::string& name, ActionFn fn);
    // The handler is skipped once `guard` has expired (use it from code that holds entities or components).
    void On(const std::string& name, std::weak_ptr<void> guard, std::function<void()> fn);
    void On(const std::string& name, std::weak_ptr<void> guard, ActionFn fn);
    const Handler* FindHandler(const std::string& name) const;
    void Fire(const std::string& event);         // runs the calls of the on<event> attribute
    void Activate();                             // counts, OnActivate(), queues the "activate" event
    const std::vector<UiActionCall>* EventCalls(const std::string& event) const;
    bool WarnedAction(const std::string& name) { return !m_warnedActions.insert(name).second; }

    // ── pointer events (the element that got the press keeps receiving move and release, even outside its rect)
    virtual void OnPointerDown(glm::vec2 localPoint) {}
    virtual void OnPointerMove(glm::vec2 localPoint) {}
    virtual void OnPointerUp(glm::vec2 localPoint) {}
    virtual void OnScroll(float delta) {}

    // ── state
    uint32_t States = 0;
    int ActivateCount = 0;               // tests read this
    std::vector<std::string> Classes;    // classes from markup (class="")
    std::vector<std::string> CodeClasses;// classes from SetClass
    std::map<std::string, std::string> Attributes;   // raw values, as written
    float m_paintOpacity = 1.f;          // effective opacity while painting (elements read it in PaintContent)

    // dirty flags live on the root: setting any marks the whole root dirty
    void MarkStyleDirty();
    void MarkLayoutDirty();
    void MarkPaintDirty();

    // ── legacy integration
    glm::vec2 GetSize() override;
    void UpdateOffsets() override;
    void UpdateChildrenOffsets() override;
    void Update() override;
    void Draw() override;
    void OnNavConfirm() override;
    void OnNavCancel() override;

    // used by UiSystem / resolver / layout
    UiComputedStyle m_computed;
    UiComputedStyle m_overrides;         // Style()
    std::string m_styleAttr;             // style="" as written
    bool m_visibleOverrideCollapse = false;
    UiDrawList m_chunk;                  // root only: recording target (local prims)
    struct LegacyMarker { size_t ItemIndex; std::shared_ptr<UiElement> Child; glm::mat3 ClipMatrix; glm::vec2 ClipSize; bool Clipped; };
    std::vector<LegacyMarker> m_legacyMarkers;   // root only: recording target
    // Root only: the finalized copy rendering reads (R2). Replaced only by UiSystem::Finalize.
    struct UiFrameData { UiDrawList List; std::vector<LegacyMarker> Markers; };
    std::shared_ptr<const UiFrameData> m_frame;
    std::map<std::string, std::string> m_vars;   // variables set on this element
    std::map<std::string, double> m_numVars;     // numeric values for format specs
    std::vector<std::string> m_varRefs;          // variable names this element's attributes use
    bool m_styleDirty = true, m_layoutDirty = true, m_paintDirty = true;
    glm::vec2 m_lastContainer = glm::vec2(-1.f);  // root only
    glm::mat3 m_lastBase = glm::mat3(0.f);        // root only
    std::vector<std::string> m_dependencies;      // root only: .xml/.css files this tree was built from
    std::string m_layoutPath;                     // root only: the file loaded with LoadLayout
    int m_attachWarned = 0;
    std::map<std::string, bool> m_warnedMissingVars;
    int SrcLine = 0, SrcCol = 0;                  // position of the markup node this element came from
    std::map<std::string, std::vector<UiActionCall>> m_eventCalls;   // event name (without "on") -> parsed calls
    std::map<std::string, Handler> m_handlers;
    std::set<std::string> m_warnedActions;
    bool m_needsScan = true;                      // root only: validate actions and fire load events after the next layout
    bool m_loadFired = false;
    bool m_validated = false;
    bool m_navResolved = false;
    void ResolveNavOverrides();                   // nav-up/down/left/right -> NavUp/... (once, after the tree is built)

    bool IsRoot() const;                 // parent is not a UiElem
    UiElem* Root();
    const UiElem* AsParentElem() const;  // parent as a UiElem, or null
    UiElem* AsParentElem();
    static bool IsElem(const UiElement* element);
    static size_t LiveCount();

    // Applies one attribute (also used again after SetVar). Returns false when it is unknown.
    bool ApplyAttributeValue(const std::string& name, const std::string& rawValue, bool warnUnknown);
    void ReapplyAttributes();            // all attributes that reference variables
    // Pre-order list of this element and its UiElem descendants.
    void CollectElems(std::vector<UiElem*>& out);
    // Called by the builder after an element is fully built (TextButton forwards its label here).
    virtual void OnBuilt() {}

private:
    void LogFindTypeMismatch(const std::string& idPath) const;
    void ApplyPointerStates();
    int m_pressedTouchId = -1;
};
