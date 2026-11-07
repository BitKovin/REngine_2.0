#include "UiInspect.h"

#include <Console/Console.h>
#include <EngineMain.h>
#include <FileSystem/FileSystem.h>
#include <Logger.hpp>
#include <UI/Elements/UiElem.h>
#include <UI/Reflection/UiElementRegistry.h>
#include <UI/UiNavigation.h>
#include <UI/UiSystem.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>

namespace
{
    using nlohmann::json;

    std::string Hex(const glm::vec4& c)
    {
        auto byte = [](float v) { return static_cast<int>(std::lround(std::clamp(v, 0.f, 1.f) * 255.f)); };
        char buffer[16];
        std::snprintf(buffer, sizeof(buffer), "#%02x%02x%02x%02x", byte(c.r), byte(c.g), byte(c.b), byte(c.a));
        return buffer;
    }

    json Length(const UiLength& l)
    {
        const char* kind = "px";
        switch (l.Kind)
        {
        case UiLenKind::Px: kind = "px"; break;
        case UiLenKind::Percent: kind = "percent"; break;
        case UiLenKind::Calc: kind = "calc"; break;
        case UiLenKind::FitChildren: kind = "fit-children"; break;
        case UiLenKind::FillParentFlow: kind = "fill-parent-flow"; break;
        case UiLenKind::HeightPercentage: kind = "height-percentage"; break;
        case UiLenKind::WidthPercentage: kind = "width-percentage"; break;
        case UiLenKind::Auto: kind = "auto"; break;
        }
        json j = { { "kind", kind } };
        if (l.Kind == UiLenKind::Px || l.Kind == UiLenKind::Calc)
            j["px"] = l.Px;
        if (l.Kind == UiLenKind::Percent || l.Kind == UiLenKind::Calc || l.Kind == UiLenKind::HeightPercentage || l.Kind == UiLenKind::WidthPercentage)
            j["percent"] = l.Percent;
        if (l.Kind == UiLenKind::FillParentFlow)
            j["weight"] = l.Weight;
        return j;
    }

    json Rect(float x, float y, float w, float h) { return { { "x", x }, { "y", y }, { "w", w }, { "h", h } }; }

    // Axis-aligned bound of an element's border box in UI units.
    void Bounds(const UiElem& e, glm::vec2& lo, glm::vec2& hi)
    {
        const glm::vec2 corners[4] = {
            UiElement::TransformPoint(e.AbsoluteTransform, glm::vec2(0.f, 0.f)),
            UiElement::TransformPoint(e.AbsoluteTransform, glm::vec2(e.BorderRect.W, 0.f)),
            UiElement::TransformPoint(e.AbsoluteTransform, glm::vec2(0.f, e.BorderRect.H)),
            UiElement::TransformPoint(e.AbsoluteTransform, glm::vec2(e.BorderRect.W, e.BorderRect.H)) };
        lo = hi = corners[0];
        for (const glm::vec2& c : corners)
        {
            lo = glm::min(lo, c);
            hi = glm::max(hi, c);
        }
    }

    json BoundsJson(const UiElem& e)
    {
        glm::vec2 lo, hi;
        Bounds(e, lo, hi);
        return Rect(lo.x, lo.y, hi.x - lo.x, hi.y - lo.y);
    }

    // DOM path segment: Tag#id, or Tag[n] where n counts same-tag siblings before it.
    std::string Segment(const UiElem& e)
    {
        if (!e.Id.empty())
            return e.TagName + "#" + e.Id;
        int index = 0;
        if (const UiElem* parent = e.AsParentElem())
            for (const auto& sibling : parent->children)
            {
                if (sibling.get() == &e)
                    break;
                if (UiElem::IsElem(sibling.get()) && static_cast<const UiElem*>(sibling.get())->TagName == e.TagName)
                    ++index;
            }
        return e.TagName + "[" + std::to_string(index) + "]";
    }

    std::string PathOf(const UiElem& e)
    {
        std::vector<std::string> parts;
        for (const UiElem* p = &e; p != nullptr; p = p->AsParentElem())
            parts.push_back(Segment(*p));
        std::string out;
        for (auto it = parts.rbegin(); it != parts.rend(); ++it)
            out += (out.empty() ? "" : "/") + *it;
        return out;
    }

    json Flags(UiElem& e)
    {
        json j = json::object();
        j["visible"] = e.IsVisible();
        j["hover"] = (e.States & kStateHover) != 0;
        j["active"] = (e.States & kStateActive) != 0;
        j["focus"] = UiNavigation::Focused == &e;
        j["disabled"] = (e.States & kStateDisabled) != 0;
        j["selected"] = (e.States & kStateSelected) != 0;
        j["checked"] = (e.States & kStateChecked) != 0;
        j["hit"] = e.HitCheck;
        j["focusable"] = e.HitCheck && !e.DisableFocus;
        return j;
    }

    json Classes(const UiElem& e)
    {
        json j = json::array();
        for (const std::string& c : e.Classes)
            j.push_back(c);
        for (const std::string& c : e.CodeClasses)
            j.push_back(c);
        return j;
    }

    json Node(UiElement& element, int depth, int maxDepth)
    {
        json j = json::object();
        if (!UiElem::IsElem(&element))
        {
            j["tag"] = "legacy:" + UiElementRegistry::Instance().GetTypeNameForInstance(&element);
            j["legacy"] = true;
            j["rect"] = Rect(element.topLeft.x, element.topLeft.y, element.size.x, element.size.y);
        }
        else
        {
            UiElem& e = static_cast<UiElem&>(element);
            j = UiInspect::ElementRef(e);
            j["classes"] = Classes(e);
            j["rect"] = BoundsJson(e);
            j["flags"] = Flags(e);
        }
        json kids = json::array();
        if (depth < maxDepth)
            for (const auto& child : element.children)
                kids.push_back(Node(*child, depth + 1, maxDepth));
        j["children"] = kids;
        return j;
    }

    // Pre-order elements of every root, the order they paint in.
    std::vector<UiElem*> AllElems()
    {
        std::vector<UiElem*> out;
        for (const auto& root : UiSystem::Roots())
            root->CollectElems(out);
        return out;
    }

    // The logger line buffer is small, so a result goes to SaveData/ui_inspect/<name>.json and the log names the file.
    void Emit(const char* tag, const char* name, const json& payload)
    {
        const std::string path = std::string("SaveData/ui_inspect/") + name + ".json";
        if (FileSystemEngine::WriteFile(path, payload.dump()))
            Logger::Log("[%s] %s", tag, path.c_str());
        else
            Logger::Error("[%s] cannot write %s", tag, path.c_str());
    }

    void CmdDumpJson(const std::vector<std::string>& args)
    {
        const int depth = args.empty() ? 64 : std::atoi(args[0].c_str());
        Emit("UiDumpJson", "tree", UiInspect::TreeJson(depth));
    }

    void CmdPickAt(const std::vector<std::string>& args)
    {
        if (args.size() < 2)
        {
            Logger::Log("usage: ui_pick_at <x> <y>   (UI units)");
            return;
        }
        const json result = UiInspect::PickAt(static_cast<float>(std::atof(args[0].c_str())), static_cast<float>(std::atof(args[1].c_str())));
        Emit("UiPickFile", "pick", result);
        if (result.value("hit", false))
            Logger::Log("[UiPick] %s  %s:%d:%d", result["element"]["path"].get<std::string>().c_str(), result["element"]["file"].get<std::string>().c_str(),
                        result["element"]["line"].get<int>(), result["element"]["col"].get<int>());
        else
            Logger::Log("[UiPick] nothing at %s,%s", args[0].c_str(), args[1].c_str());
    }

    void CmdComputed(const std::vector<std::string>& args)
    {
        if (args.empty())
        {
            Logger::Log("usage: ui_computed <id-or-path>");
            return;
        }
        Emit("UiComputed", "computed", UiInspect::ComputedJson(args[0]));
    }
}

nlohmann::json UiInspect::ViewportJson()
{
    const glm::vec2 size = EngineMain::Viewport.GetSize();
    return { { "w", size.x }, { "h", size.y } };
}

nlohmann::json UiInspect::ElementRef(const UiElem& e)
{
    json j = json::object();
    j["tag"] = e.TagName;
    if (!e.Id.empty())
        j["id"] = e.Id;
    if (!e.ComponentTag.empty())
        j["component"] = e.ComponentTag;
    j["path"] = PathOf(e);
    j["file"] = e.ScopeFile;
    j["line"] = e.SrcLine;
    j["col"] = e.SrcCol;
    return j;
}

nlohmann::json UiInspect::TreeJson(int maxDepth)
{
    json roots = json::array();
    for (const auto& root : UiSystem::Roots())
        roots.push_back(Node(*root, 0, maxDepth));
    json out = json::object();
    out["viewport"] = ViewportJson();
    out["roots"] = roots;
    if (UiElem::IsElem(UiNavigation::Focused))
        out["focus"] = PathOf(*static_cast<UiElem*>(UiNavigation::Focused));
    return out;
}

nlohmann::json UiInspect::PickAt(float x, float y)
{
    UiElem* best = nullptr;
    for (UiElem* e : AllElems())
    {
        if (!e->IsVisible() || e->InternalOverlay)
            continue;
        glm::vec2 lo, hi;
        Bounds(*e, lo, hi);
        if (x >= lo.x && x <= hi.x && y >= lo.y && y <= hi.y)
            best = e;   // later in paint order wins; descendants come after their ancestors
    }
    if (best == nullptr)
        return { { "hit", false } };
    json out = { { "hit", true }, { "element", ElementRef(*best) }, { "rect", BoundsJson(*best) } };
    json chain = json::array();
    for (UiElem* p = best->AsParentElem(); p != nullptr; p = p->AsParentElem())
        chain.push_back(ElementRef(*p));
    out["ancestors"] = chain;
    return out;
}

UiElem* UiInspect::Resolve(const std::string& idOrPath)
{
    if (idOrPath.empty())
        return nullptr;
    if (UiElem* byId = UiSystem::FindById(idOrPath))
        return byId;
    for (UiElem* e : AllElems())
        if (PathOf(*e) == idOrPath)
            return e;
    return nullptr;
}

nlohmann::json UiInspect::ComputedJson(const std::string& idOrPath)
{
    UiElem* e = Resolve(idOrPath);
    if (e == nullptr)
        return { { "found", false }, { "query", idOrPath } };

    const UiComputedStyle& c = e->Computed();
    json out = json::object();
    out["found"] = true;
    out["element"] = ElementRef(*e);
    out["classes"] = Classes(*e);
    out["flags"] = Flags(*e);

    glm::vec2 origin = UiElement::TransformPoint(e->AbsoluteTransform, glm::vec2(0.f));
    out["borderRect"] = BoundsJson(*e);
    const UiRect content = e->ContentRectScreen();
    out["contentRect"] = Rect(content.X, content.Y, content.W, content.H);
    out["localRect"] = Rect(e->BorderRect.X, e->BorderRect.Y, e->BorderRect.W, e->BorderRect.H);
    out["origin"] = { { "x", origin.x }, { "y", origin.y } };
    json m = json::array();
    for (int col = 0; col < 3; ++col)
        for (int row = 0; row < 3; ++row)
            m.push_back(e->AbsoluteTransform[col][row]);
    out["transform"] = m;   // column-major 3x3, element-local to UI units

    json values = json::object();
    values["width"] = Length(c.Width);
    values["height"] = Length(c.Height);
    values["min-width"] = Length(c.MinWidth);
    values["max-width"] = Length(c.MaxWidth);
    values["min-height"] = Length(c.MinHeight);
    values["max-height"] = Length(c.MaxHeight);
    static const char* const kFlow[] = { "none", "down", "right", "down-wrap", "right-wrap" };
    static const char* const kOverflow[] = { "visible", "clip", "scroll" };
    static const char* const kVisibility[] = { "visible", "hidden", "collapse" };
    values["flow-children"] = kFlow[static_cast<int>(c.Flow)];
    values["overflow"] = kOverflow[static_cast<int>(c.Overflow)];
    values["visibility"] = kVisibility[static_cast<int>(c.Visibility)];
    values["x"] = Length(c.X);
    values["y"] = Length(c.Y);
    values["margin"] = { Length(c.MarginTop), Length(c.MarginRight), Length(c.MarginBottom), Length(c.MarginLeft) };
    values["padding"] = { Length(c.PaddingTop), Length(c.PaddingRight), Length(c.PaddingBottom), Length(c.PaddingLeft) };
    values["gap"] = Length(c.Gap);
    values["z-index"] = c.ZIndex;
    values["background-color"] = Hex(c.BackgroundColor);
    values["color"] = Hex(c.Color);
    values["opacity"] = c.Opacity;
    values["font-family"] = c.FontFamily;
    values["font-size"] = c.FontSize;
    values["font-weight"] = c.FontWeight;
    values["letter-spacing"] = c.LetterSpacing;
    json borders = json::array();
    for (int i = 0; i < 4; ++i)
        borders.push_back({ { "width", c.EffectiveBorder(i) }, { "color", Hex(c.BorderColor[i]) } });
    values["border"] = borders;
    out["computed"] = values;

    json attributes = json::object();
    for (const auto& [name, value] : e->Attributes)
        attributes[name] = value;
    if (!e->m_styleAttr.empty())
        attributes["style"] = e->m_styleAttr;
    out["attributes"] = attributes;

    // Which declaration won each property is not recorded by the resolver yet; say so instead of guessing.
    out["provenance"] = "unavailable";
    return out;
}

void UiInspect::RegisterConsoleCommands()
{
    Console& console = Console::Get();
    console.RegisterCommand({ "ui_dump_json", "ui_dump_json [depth] - logs the markup trees as one JSON document", CmdDumpJson });
    console.RegisterCommand({ "ui_pick_at", "ui_pick_at <x> <y> - logs the element under a UI-unit point with its source position", CmdPickAt });
    console.RegisterCommand({ "ui_computed", "ui_computed <id-or-path> - logs rectangles, states and computed values of one element as JSON", CmdComputed });
}
