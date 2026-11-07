#include "UiElemDropDown.h"

#include <EngineMain.h>
#include <Input.h>
#include <UI/Elements/UiElemLabel.h>
#include <UI/Elements/UiElemPanel.h>
#include <UI/Markup/UiVariables.h>
#include <UI/UiNavigation.h>
#include <UI/UiSystem.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace
{
    std::vector<std::string> SplitBars(const std::string& s)
    {
        std::vector<std::string> out;
        std::string cur;
        for (char c : s)
        {
            if (c == '|') { out.push_back(cur); cur.clear(); }
            else cur += c;
        }
        out.push_back(cur);
        return out;
    }

    UiLength Px(float v)
    {
        UiLength l;
        l.Kind = UiLenKind::Px;
        l.Px = v;
        return l;
    }
}

// The chevron: drawn in the text colour, pointing down while closed and up while open.
class UiElemDropArrow : public UiElem
{
public:
    UiElemDropArrow() { TagName = "Panel"; }
    void SetOpen(bool open)
    {
        if (open != m_open)
        {
            m_open = open;
            MarkPaintDirty();
        }
    }
    void PaintContent(UiPainter& painter, const UiRect& c) override
    {
        glm::vec4 color = Computed().Color;
        color.a *= m_paintOpacity;
        if (color.a <= 0.f || c.W <= 0.f || c.H <= 0.f)
            return;
        const float near = c.Y + c.H * (m_open ? 0.68f : 0.32f), far = c.Y + c.H * (m_open ? 0.32f : 0.68f);
        const std::vector<glm::vec2> points = { { c.X + c.W * 0.1f, near }, { c.X + c.W * 0.5f, far }, { c.X + c.W * 0.9f, near } };
        painter.StrokePolyline(points, std::max(2.f, c.H * 0.14f), color, false);
    }

private:
    bool m_open = false;
};

// The list: a root of its own with a focus trap, so the navigation stays inside it and cancel closes it.
class UiElemDropList : public UiElemPanel
{
public:
    explicit UiElemDropList(UiElemDropDown* owner) : Owner(owner) {}
    void OnNavCancel() override
    {
        if (Owner != nullptr)
            Owner->m_closeRequested = true;
    }
    UiElemDropDown* Owner;
};

class UiElemDropItem : public UiElemTextButton
{
public:
    UiElemDropItem(UiElemDropDown* owner, int index) : Owner(owner), Index(index) {}
    void OnActivate() override
    {
        if (Owner != nullptr)
            Owner->m_pick = Index;
    }
    UiElemDropDown* Owner;
    int Index;
};

UiElemDropDown::UiElemDropDown()
{
    TagName = "DropDown";
    auto text = std::make_shared<UiElemLabel>();
    text->Classes = { "dropdown-text" };
    m_text = text.get();
    AddChild(text);
    auto arrow = std::make_shared<UiElemDropArrow>();
    arrow->Classes = { "dropdown-arrow" };
    m_arrow = arrow.get();
    AddChild(arrow);
    m_binding.ReadWidget = [this]() { return static_cast<double>(Selected()); };
    m_binding.WriteWidget = [this](double v) { SetSelected(static_cast<int>(v)); };
}

UiElemDropDown::~UiElemDropDown()
{
    DestroyList();
}

void UiElemDropDown::OnBuilt()
{
    for (UiElem* child : { static_cast<UiElem*>(m_text), static_cast<UiElem*>(m_arrow) })
    {
        child->ScopeFile = ScopeFile;
        child->Instance = Instance;
    }
}

void UiElemDropDown::RefreshLabel()
{
    if (m_text == nullptr)
        return;
    const bool has = m_selected >= 0 && m_selected < static_cast<int>(m_options.size());
    m_text->SetResolvedText(has ? m_options[m_selected] : m_placeholder);
}

void UiElemDropDown::SetSelected(int index)
{
    const int count = static_cast<int>(m_options.size());
    m_selected = count == 0 ? -1 : std::clamp(index, 0, count - 1);
    RefreshLabel();
}

void UiElemDropDown::SetOptions(const std::vector<std::string>& options)
{
    if (m_open)
        Close();
    m_options = options;
    SetSelected(m_selected);
}

void UiElemDropDown::RefreshBoundOptions()
{
    if (m_optionsFromAttribute || !m_binding.IsBound() || m_binding.Entry()->Type != UiSettings::Kind::Choice)
        return;
    std::vector<std::string> localised;
    for (const std::string& c : UiSettings::ChoicesOf(*m_binding.Entry()))
        localised.push_back(UiSubstitute(*this, c, nullptr));
    if (localised != m_options)
    {
        SetOptions(localised);
        // the list arrives after the value was bound (or changes at run time): show the setting's value again, never the clamped
        // index the new list produced, or opening the page would write that index back into the setting
        if (m_binding.Entry()->Get)
            SetSelected(static_cast<int>(m_binding.Entry()->Get()));
        m_binding.Resync();
    }
}

bool UiElemDropDown::ApplyAttribute(const std::string& name, const std::string& value)
{
    if (name == "options")
    {
        m_optionsFromAttribute = true;
        SetOptions(SplitBars(value));
        return true;
    }
    if (name == "selected-index")
    {
        SetSelected(std::atoi(value.c_str()));
        m_binding.Resync();
        return true;
    }
    if (name == "placeholder")
    {
        m_placeholder = value;
        RefreshLabel();
        return true;
    }
    if (name == "max-visible")
    {
        m_maxVisible = std::max(1, std::atoi(value.c_str()));
        return true;
    }
    if (name == "bind")
    {
        m_binding.SetBind(value);
        RefreshBoundOptions();
        return true;
    }
    return UiElemButton::ApplyAttribute(name, value);
}

void UiElemDropDown::OnActivate()
{
    if (m_open)
        Close();
    else
        Open();
}

void UiElemDropDown::Open()
{
    if (m_open || m_options.empty() || (States & kStateDisabled))
        return;
    UiElem* root = Root();
    auto list = std::make_shared<UiElemDropList>(this);
    list->Classes = { "dropdown-list" };
    list->ScopeFile = root->ScopeFile;
    list->OwnedInstance = root->OwnedInstance;   // the page's sheets style the list
    list->Instance = Instance;
    list->HitCheck = true;                       // the list swallows presses: nothing behind it reacts
    list->DisableFocus = true;
    list->FocusTrap = true;
    list->EnsureScrollbar();

    m_rows.clear();
    for (size_t i = 0; i < m_options.size(); ++i)
    {
        auto row = std::make_shared<UiElemDropItem>(this, static_cast<int>(i));
        row->Classes = { "dropdown-item" };
        row->ScopeFile = root->ScopeFile;
        row->Instance = Instance;
        row->OnBuilt();
        row->LabelElem()->Classes = { "dropdown-item-text" };
        row->LabelElem()->SetResolvedText(m_options[i]);
        if (static_cast<int>(i) == m_selected)
            row->States |= kStateSelected;
        m_rows.push_back(row.get());
        list->AddChild(row);
    }

    // The list lives outside this element's tree, so it takes the text look from here.
    UiComputedStyle& s = list->Style();
    s.FontSize = Computed().FontSize;
    s.Color = Computed().Color;
    s.FontFamily = Computed().FontFamily;
    s.FontWeight = Computed().FontWeight;

    m_list = list;
    m_open = true;
    m_lastGeom = glm::vec4(-1.f);
    m_focusFrames = 6;
    PositionList();
    EngineMain::Viewport.AddChild(list);
    UiSystem::RegisterRoot(list);
    SetClass("open", true);
    if (m_arrow != nullptr)
        m_arrow->SetOpen(true);
}

void UiElemDropDown::DestroyList()
{
    if (!m_list)
        return;
    static_cast<UiElemDropList&>(*m_list).Owner = nullptr;
    UiSystem::UnregisterRoot(m_list);
    if (m_list->parent != nullptr)
        m_list->parent->RemoveChild(m_list);
    m_list.reset();
    m_rows.clear();
    m_open = false;
}

void UiElemDropDown::Close()
{
    if (!m_open)
        return;
    bool focusInside = false;
    for (UiElement* p = UiNavigation::Focused; p != nullptr; p = p->parent)
        if (p == m_list.get())
            focusInside = true;
    DestroyList();
    m_closeRequested = false;
    m_pick = -1;
    SetClass("open", false);
    if (m_arrow != nullptr)
        m_arrow->SetOpen(false);
    if (focusInside)
        UiNavigation::SetFocus(this);
}

// Below the header when it fits (above when that has more room), as wide as the header, at most max-visible rows tall.
void UiElemDropDown::PositionList()
{
    if (!m_list)
        return;
    const glm::vec2 tl = UiElement::TransformPoint(AbsoluteTransform, glm::vec2(0.f));
    const glm::vec2 br = UiElement::TransformPoint(AbsoluteTransform, glm::vec2(BorderRect.W, BorderRect.H));
    const glm::vec2 viewport = EngineMain::Viewport.GetSize();

    const float rowHeight = !m_rows.empty() && m_rows[0]->BorderRect.H > 0.f ? m_rows[0]->BorderRect.H : 44.f;
    const float frame = m_list->BorderRect.H > 0.f && m_list->ScrollViewport.y > 0.f ? m_list->BorderRect.H - m_list->ScrollViewport.y : 4.f;
    const float natural = static_cast<float>(m_rows.size()) * rowHeight + frame;
    const float cap = static_cast<float>(m_maxVisible) * rowHeight + frame;
    const float below = viewport.y - br.y - 8.f, above = tl.y - 8.f;
    const bool flip = below < std::min(natural, cap) && above > below;
    const float room = std::max(rowHeight, flip ? above : below);
    const float height = std::min({ natural, cap, room });

    const glm::vec4 geom(std::round(tl.x), std::round(flip ? tl.y - height : br.y), std::round(br.x - tl.x), std::round(std::min(cap, room)));
    if (geom == m_lastGeom)
        return;
    m_lastGeom = geom;
    UiComputedStyle& s = m_list->Style();
    s.X = Px(geom.x);
    s.Y = Px(geom.y);
    s.Width = Px(geom.z);
    s.MaxHeight = Px(geom.w);
}

bool UiElemDropDown::PressedOutside() const
{
    for (const auto& entry : Input::TouchActions)
    {
        const auto& touch = entry.second;
        if (!touch.pressed)
            continue;
        auto inside = [&](const UiElem& e) {
            const glm::vec2 l = UiElement::TransformPoint(glm::inverse(e.AbsoluteTransform), touch.position);
            return l.x >= 0.f && l.y >= 0.f && l.x <= e.BorderRect.W && l.y <= e.BorderRect.H;
        };
        if (!inside(*this) && !(m_list && inside(*m_list)))
            return true;
    }
    return false;
}

void UiElemDropDown::Update()
{
    UiElemButton::Update();
    m_binding.Poll();
    if (!m_open)
        return;

    if (m_pick >= 0)
    {
        const int pick = m_pick;
        Close();
        SetSelected(pick);
        m_binding.Poll();
        return;
    }
    if (m_closeRequested || !IsVisible() || (States & kStateDisabled) || PressedOutside())
    {
        Close();
        return;
    }
    PositionList();

    // keyboard and gamepad: the list starts on the selected row (the pointer decides for itself)
    if (m_focusFrames > 0 && !m_rows.empty() && m_list->BorderRect.W > 0.f)
    {
        --m_focusFrames;
        UiElem* target = m_rows[std::clamp(m_selected, 0, static_cast<int>(m_rows.size()) - 1)];
        UiNavigation::SetFocus(target);
        if (UiNavigation::Focused == target)
        {
            m_focusFrames = 0;
            m_list->ScrollIntoView(*target, 0.f);
        }
    }
}
