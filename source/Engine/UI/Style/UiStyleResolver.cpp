#include "UiStyleResolver.h"

#include <EngineMain.h>
#include <UI/Elements/UiElem.h>
#include <UI/Markup/UiDocument.h>
#include <UI/Markup/UiMarkupLog.h>
#include <UI/Style/UiAnimation.h>
#include <UI/Style/UiCssParser.h>
#include <UI/Style/UiComputedStyle.h>
#include <UI/Style/UiFxParse.h>
#include <UI/Style/UiSelector.h>
#include <UI/UiSystem.h>

#include <algorithm>
#include <map>
#include <set>

namespace
{
    struct SheetRef
    {
        std::shared_ptr<UiStyleSheet> Sheet;
        int Order = 0;
        std::string ReportPath;   // file named in messages (inline <style> blocks report their .xml)
    };

    struct Match
    {
        int Layer, Specificity, SheetOrder, RuleOrder;
        const UiCssRule* Rule;
        const std::string* Path;
    };

    // The engine's own look for elements that build parts of themselves (the drop-down). It sits below every sheet of the page, so
    // any rule of the page wins whatever its specificity.
    const std::vector<SheetRef>& EngineDefaultSheets()
    {
        static const std::vector<SheetRef> sheets = [] {
            static const char* text =
                "DropDown { width: 300px; height: 44px; flow-children: right; gap: 8px; padding: 0 12px; vertical-align: center;"
                " background-color: #333333; border: 2px solid #5a5a5a; border-radius: 4px; color: #ffffff; }\n"
                "DropDown:hover, DropDown.open { background-color: #444444; }\n"
                "DropDown:focus { border-color: #ffffff; }\n"
                "DropDown:disabled { opacity: 0.45; }\n"
                "DropDown .dropdown-text { width: fill-parent-flow(1); vertical-align: center; text-overflow: ellipsis; }\n"
                "DropDown .dropdown-arrow { width: 14px; height: 14px; vertical-align: center; }\n"
                ".dropdown-list { flow-children: down; overflow: scroll; background-color: #262626; border: 2px solid #5a5a5a; border-radius: 4px; }\n"
                ".dropdown-list .scrollbar { width: 8px; background-color: #1a1a1a; }\n"
                ".dropdown-list .scrollbar .thumb { background-color: #8a8a8a; border-radius: 4px; }\n"
                ".dropdown-item { width: 100%; height: 44px; padding: 0 12px; }\n"
                ".dropdown-item .dropdown-item-text { vertical-align: center; }\n"
                ".dropdown-item:hover, .dropdown-item:focus { background-color: #444444; }\n"
                ".dropdown-item:selected { background-color: #3d4b66; }\n";
            std::vector<SheetRef> list;
            SheetRef ref;
            ref.Sheet = UiParseStyleSheet(text, "<engine default sheet>", 0);
            ref.Order = 0;
            ref.ReportPath = "<engine default sheet>";
            list.push_back(ref);
            return list;
        }();
        return sheets;
    }

    std::set<std::string>& Warned()
    {
        static std::set<std::string> warned;
        return warned;
    }

    void WarnOnce(const std::string& path, int line, int col, const std::string& message)
    {
        const std::string key = path + ":" + std::to_string(line) + ":" + std::to_string(col) + ":" + message;
        if (Warned().insert(key).second)
            UiMarkupLog::Warning(path, line, col, message);
    }

    // Appends the sheet and its @imports (imports first) to `out`.
    void Expand(const std::shared_ptr<UiStyleSheet>& sheet, const std::string& ownerFile, std::vector<SheetRef>& out, int depth)
    {
        if (!sheet || depth > 8)
            return;
        for (const std::string& import : sheet->Imports)
            Expand(UiStyleSheetCache::Get(import), import, out, depth + 1);
        SheetRef ref;
        ref.Sheet = sheet;
        ref.Order = (int)out.size();
        ref.ReportPath = sheet->Path.rfind("<inline:", 0) == 0 ? ownerFile : sheet->Path;
        out.push_back(ref);
    }

    struct RootContext
    {
        std::string RootFile;
        std::vector<SheetRef> DocumentSheets;
        std::map<std::string, std::vector<SheetRef>> ComponentSheets;   // by component file
    };

    void CollectFiles(UiElem& e, std::set<std::string>& files)
    {
        files.insert(e.ScopeFile);
        for (auto& c : e.children)
            if (UiElem::IsElem(c.get()))
                CollectFiles(static_cast<UiElem&>(*c), files);
    }

    // Fields that differ from a default-constructed style win over CSS (C++ overrides).
    void ApplyOverrides(UiComputedStyle& s, const UiComputedStyle& o)
    {
        const UiComputedStyle d;
#define UI_OVR(field) if (!(o.field == d.field)) s.field = o.field;
        UI_OVR(Width) UI_OVR(Height) UI_OVR(MinWidth) UI_OVR(MaxWidth) UI_OVR(MinHeight) UI_OVR(MaxHeight)
        UI_OVR(Flow) UI_OVR(HAlign) UI_OVR(VAlign) UI_OVR(X) UI_OVR(Y)
        UI_OVR(MarginTop) UI_OVR(MarginRight) UI_OVR(MarginBottom) UI_OVR(MarginLeft)
        UI_OVR(PaddingTop) UI_OVR(PaddingRight) UI_OVR(PaddingBottom) UI_OVR(PaddingLeft)
        UI_OVR(Gap) UI_OVR(Overflow) UI_OVR(Visibility) UI_OVR(ZIndex) UI_OVR(BackgroundColor)
        UI_OVR(Opacity) UI_OVR(Color) UI_OVR(FontFamily) UI_OVR(FontSize) UI_OVR(FontWeight) UI_OVR(FontItalic)
        UI_OVR(LetterSpacing) UI_OVR(LineHeight) UI_OVR(TextAlign) UI_OVR(WhiteSpace) UI_OVR(TextOverflow)
        UI_OVR(TextTransform) UI_OVR(ObjectFit)
#undef UI_OVR
        for (int i = 0; i < 4; ++i)
        {
            if (o.BorderWidth[i] != d.BorderWidth[i]) s.BorderWidth[i] = o.BorderWidth[i];
            if (o.BorderColor[i] != d.BorderColor[i]) s.BorderColor[i] = o.BorderColor[i];
            if (o.BorderSolid[i] != d.BorderSolid[i]) s.BorderSolid[i] = o.BorderSolid[i];
            if (!(o.Radius[i] == d.Radius[i])) s.Radius[i] = o.Radius[i];
        }
        for (const auto& [name, value] : o.Custom)
            s.Custom[name] = value;
    }

    struct Applier
    {
        UiComputedStyle& S;
        UiStyleParseContext Ctx;
        const std::string& DefaultPath;
        std::string CurrentPath;

        void Apply(const UiCssDeclaration& decl, const std::string& path)
        {
            if (!UiIsKnownProperty(decl.Name))
            {
                WarnOnce(path, decl.Line, decl.Col, "unknown property '" + decl.Name + "'");
                return;
            }
            std::vector<UiCssToken> tokens;
            std::string error;
            if (!UiSubstituteVars(decl.Value, S.Custom, tokens, error))
            {
                WarnOnce(path, decl.Line, decl.Col, "invalid value for '" + decl.Name + "': " + error);
                return;
            }
            UiLonghandList longhands;
            if (!UiExpandProperty(decl.Name, tokens, longhands, error))
            {
                WarnOnce(path, decl.Line, decl.Col, "invalid value for '" + decl.Name + "': " + error);
                return;
            }
            for (const auto& [name, value] : longhands)
            {
                if (!UiApplyLonghand(S, name, value, Ctx, error))
                    WarnOnce(path, decl.Line, decl.Col, "invalid value for '" + decl.Name + "': " + error);
            }
        }
    };

    // `docFile` is the layout file of the page (or frame content) the element belongs to: a page pushed on a menu stack is not the
    // root of the tree, so its own style sheets must still reach the components built inside it.
    void ResolveElement(UiElem& e, const UiComputedStyle* parent, const RootContext& rc, const UiStyleParseContext& base, const std::string& parentDocFile)
    {
        std::string docFile = parentDocFile;
        if (e.InstanceRoot && e.OwnedInstance && e.OwnedInstance->Def && !e.OwnedInstance->Def->Component)
            docFile = e.ScopeFile;
        // 1-2. matching rules, component layer then document layer
        std::vector<Match> matches;
        auto collect = [&](const std::vector<SheetRef>& sheets, int layer) {
            for (const SheetRef& ref : sheets)
                for (const UiCssRule& rule : ref.Sheet->Rules)
                {
                    int best = -1;
                    for (const UiSelector& sel : rule.Selectors)
                        if (sel.Specificity > best && UiSelectorMatches(sel, e))
                            best = sel.Specificity;
                    if (best >= 0)
                        matches.push_back({ layer, best, ref.Order, rule.SourceOrder, &rule, &ref.ReportPath });
                }
        };
        collect(EngineDefaultSheets(), -1);
        if (e.ScopeFile != rc.RootFile)
        {
            const auto it = rc.ComponentSheets.find(e.ScopeFile);
            if (it != rc.ComponentSheets.end())
                collect(it->second, 0);
        }
        if (!docFile.empty() && docFile != rc.RootFile && docFile != e.ScopeFile)
        {
            const auto page = rc.ComponentSheets.find(docFile);
            if (page != rc.ComponentSheets.end())
                collect(page->second, 1);
        }
        collect(rc.DocumentSheets, 1);
        std::stable_sort(matches.begin(), matches.end(), [](const Match& a, const Match& b) {
            if (a.Layer != b.Layer) return a.Layer < b.Layer;
            if (a.Specificity != b.Specificity) return a.Specificity < b.Specificity;
            if (a.SheetOrder != b.SheetOrder) return a.SheetOrder < b.SheetOrder;
            return a.RuleOrder < b.RuleOrder;
        });

        struct Source { const UiCssDeclaration* Decl; std::string Path; };
        std::vector<Source> decls;
        for (const Match& m : matches)
            for (const UiCssDeclaration& d : m.Rule->Declarations)
                decls.push_back({ &d, *m.Path });

        // 4. style="" attribute
        std::vector<UiCssDeclaration> inlineDecls;
        if (!e.m_styleAttr.empty())
            inlineDecls = UiParseDeclarationList(e.m_styleAttr, e.ScopeFile, 1, 1);
        for (const UiCssDeclaration& d : inlineDecls)
            decls.push_back({ &d, e.ScopeFile });

        // 6. compute
        UiComputedStyle s;
        if (parent != nullptr)
        {
            s.Visibility = parent->Visibility;
            s.Color = parent->Color;
            s.FontFamily = parent->FontFamily;
            s.FontSize = parent->FontSize;
            s.FontWeight = parent->FontWeight;
            s.FontItalic = parent->FontItalic;
            s.LetterSpacing = parent->LetterSpacing;
            s.LineHeight = parent->LineHeight;
            s.TextAlign = parent->TextAlign;
            s.WhiteSpace = parent->WhiteSpace;
            s.TextTransform = parent->TextTransform;
            s.Custom = parent->Custom;
            s.Fx.TextShadow = parent->Fx.TextShadow;
            s.Fx.TextStrokeWidth = parent->Fx.TextStrokeWidth;
            s.Fx.TextStrokeColor = parent->Fx.TextStrokeColor;
            s.Fx.ImageRendering = parent->Fx.ImageRendering;
        }
        UiStyleParseContext ctx = base;
        if (parent == nullptr)
            s.FontSize = base.RootFontSize;   // 24 x the text scale
        ctx.ParentFontSize = parent != nullptr ? parent->FontSize : base.RootFontSize;
        ctx.FontSize = s.FontSize;
        Applier ap{ s, ctx, e.ScopeFile, e.ScopeFile };

        for (const Source& src : decls)
            if (src.Decl->Name.rfind("--", 0) == 0)
                s.Custom[src.Decl->Name] = src.Decl->Value;
        for (const Source& src : decls)
            if (src.Decl->Name == "font-size")
                ap.Apply(*src.Decl, src.Path);
        ap.Ctx.FontSize = s.FontSize;
        for (const Source& src : decls)
            if (src.Decl->Name == "color")
                ap.Apply(*src.Decl, src.Path);
        ap.Ctx.CurrentColor = s.Color;
        for (int i = 0; i < 4; ++i)
            s.BorderColor[i] = s.Color;
        for (const Source& src : decls)
        {
            const std::string& n = src.Decl->Name;
            if (n.rfind("--", 0) == 0 || n == "font-size" || n == "color")
                continue;
            ap.Apply(*src.Decl, src.Path);
        }

        // 5. C++ overrides
        ApplyOverrides(s, e.m_overrides);
        if (e.m_visibleOverrideCollapse)
            s.Visibility = UiVisibility::Collapse;
        {
            // sound-enter / sound-exit belong to the rules: the rules that match now are compared with the ones of the last resolve
            std::vector<UiElem::RuleSound> sounds;
            for (const Match& m : matches)
            {
                UiRuleSounds rs;
                for (const UiCssDeclaration& d : m.Rule->Declarations)
                {
                    std::string ignored;   // an invalid value was already reported by the Applier
                    if (UiFxIsRuleSoundProperty(d.Name))
                        UiFxParseRuleSound(d.Name, d.Value, rs, ignored);
                }
                if (rs.Enter.empty() && rs.Exit.empty())
                    continue;
                UiElem::RuleSound sound;
                sound.Key = std::to_string(m.Layer) + ":" + std::to_string(m.SheetOrder) + ":" + std::to_string(m.RuleOrder) + ":" + *m.Path;
                sound.Enter = rs.Enter;
                sound.Exit = rs.Exit;
                sound.Volume = rs.Volume;
                for (const UiSelector& sel : m.Rule->Selectors)
                    if (UiSelectorMatchesDisabledState(sel))
                        sound.WhileDisabled = true;
                sounds.push_back(std::move(sound));
            }
            e.UpdateSoundRules(std::move(sounds));
        }
        UiAnimation::OnStyleResolved(e, s);   // transitions and animations replace animated values by their current ones
        e.m_computed = std::move(s);

        for (auto& c : e.children)
            if (UiElem::IsElem(c.get()))
                ResolveElement(static_cast<UiElem&>(*c), &e.m_computed, rc, base, docFile);
    }
}

void UiStyleResolver::ResolveRoot(UiElem& root)
{
    RootContext rc;
    rc.RootFile = root.ScopeFile;

    // Document layer: the root document's own sheets.
    if (root.OwnedInstance && root.OwnedInstance->Def)
        for (const auto& sheet : root.OwnedInstance->Def->Sheets)
            Expand(sheet, rc.RootFile, rc.DocumentSheets, 0);

    // Component layer: sheets of every other file used inside the root.
    std::set<std::string> files;
    CollectFiles(root, files);
    for (const std::string& file : files)
    {
        if (file.empty() || file == rc.RootFile)
            continue;
        const auto def = UiDocumentCache_Get(file);
        if (!def)
            continue;
        std::vector<SheetRef> list;
        for (const auto& sheet : def->Sheets)
            Expand(sheet, file, list, 0);
        rc.ComponentSheets[file] = std::move(list);
    }

    // @keyframes of every sheet in scope; the document's own sheets win over components'.
    auto keyframes = std::make_shared<UiKeyframeMap>();
    for (const auto& [file, sheets] : rc.ComponentSheets)
        for (const SheetRef& ref : sheets)
            for (const UiKeyframes& k : ref.Sheet->Keyframes)
                (*keyframes)[k.Name] = k;
    for (const SheetRef& ref : rc.DocumentSheets)
        for (const UiKeyframes& k : ref.Sheet->Keyframes)
            (*keyframes)[k.Name] = k;
    root.m_keyframes = keyframes;

    UiStyleParseContext base;
    base.Viewport = EngineMain::Viewport.GetSize();
    base.RootFontSize = 24.f * UiSystem::TextScale;
    ResolveElement(root, root.StyleParent != nullptr ? &root.StyleParent->Computed() : nullptr, rc, base, std::string());
    ++root.m_styleGen;
}

bool UiStyleResolver::ApplyDeclaration(UiComputedStyle& style, const UiCssDeclaration& decl, const UiStyleParseContext& ctx, std::string& error)
{
    if (!UiIsKnownProperty(decl.Name))
    {
        error = "unknown property '" + decl.Name + "'";
        return false;
    }
    std::vector<UiCssToken> tokens;
    if (!UiSubstituteVars(decl.Value, style.Custom, tokens, error))
        return false;
    UiLonghandList longhands;
    if (!UiExpandProperty(decl.Name, tokens, longhands, error))
        return false;
    for (const auto& [name, value] : longhands)
        if (!UiApplyLonghand(style, name, value, ctx, error))
            return false;
    return true;
}

void UiStyleResolver::ResetWarnings()
{
    Warned().clear();
}
