#include "UiFlowLayout.h"

#include <UI/Elements/UiElem.h>
#include <UI/Markup/UiMarkupLog.h>

#include <algorithm>
#include <cmath>
#include <limits>

namespace
{
    const float kNaN = std::numeric_limits<float>::quiet_NaN();
    bool IsNaN(float v) { return std::isnan(v); }

    struct Size { float W = 0.f, H = 0.f; bool FillW = false, FillH = false; };

    // Percentages of an unknown basis count as 0.
    float ResolveLen(const UiLength& len, float basis)
    {
        if (!len.IsResolvable())
            return 0.f;
        return len.Px + (IsNaN(basis) ? 0.f : len.Percent * 0.01f * basis);
    }

    struct Boxes
    {
        float ml, mr, mt, mb;
        float pl, pr, pt, pb;
        float bl, br, bt, bb;
    };

    Boxes ResolveBoxes(const UiComputedStyle& s, float basis)
    {
        Boxes b;
        b.ml = ResolveLen(s.MarginLeft, basis); b.mr = ResolveLen(s.MarginRight, basis);
        b.mt = ResolveLen(s.MarginTop, basis); b.mb = ResolveLen(s.MarginBottom, basis);
        b.pl = std::max(0.f, ResolveLen(s.PaddingLeft, basis)); b.pr = std::max(0.f, ResolveLen(s.PaddingRight, basis));
        b.pt = std::max(0.f, ResolveLen(s.PaddingTop, basis)); b.pb = std::max(0.f, ResolveLen(s.PaddingBottom, basis));
        b.bl = s.EffectiveBorder(3); b.br = s.EffectiveBorder(1); b.bt = s.EffectiveBorder(0); b.bb = s.EffectiveBorder(2);
        return b;
    }

    std::vector<UiElem*> LayoutChildren(UiElem& e)
    {
        std::vector<UiElem*> out;
        for (auto& c : e.children)
            if (UiElem::IsElem(c.get()))
            {
                UiElem* child = static_cast<UiElem*>(c.get());
                if (child->Computed().Visibility != UiVisibility::Collapse)
                    out.push_back(child);
            }
        return out;
    }

    float Clamp(float v, const UiLength& mn, const UiLength& mx, float basis)
    {
        if (mx.Kind != UiLenKind::Auto)
            v = std::min(v, ResolveLen(mx, basis));
        v = std::max(v, ResolveLen(mn, basis));
        return std::max(0.f, v);
    }

    void WarnPercentOnce(UiElem& e)
    {
        bool& warned = e.m_warnedMissingVars["\x01pct"];
        if (!warned)
        {
            warned = true;
            UiMarkupLog::Warning(e.ScopeFile, 0, 0, "percentage of a fit-children parent on <" + e.TagName + (e.Id.empty() ? "" : " id=\"" + e.Id + "\"") + ">; counted as 0");
        }
    }

    // Border-box size of `e`. aw/ah: content size of the parent along each axis, NaN when it depends on the children.
    // forceW/forceH (not NaN) replace the element's own width/height; the parent uses them to give fill-parent-flow
    // elements their final size before the other axis (text height) is computed.
    Size Measure(UiElem& e, float aw, float ah, float forceW = kNaN, float forceH = kNaN)
    {
        const UiComputedStyle& s = e.Computed();
        const Boxes b = ResolveBoxes(s, aw);
        Size out;

        auto axisKnown = [&](const UiLength& len, float avail, bool width, bool& fit, bool& fill) -> float {
            fit = false; fill = false;
            switch (len.Kind)
            {
            case UiLenKind::FitChildren: fit = true; return kNaN;
            case UiLenKind::FillParentFlow: fill = true; return 0.f;
            case UiLenKind::HeightPercentage:
            case UiLenKind::WidthPercentage: return kNaN;   // resolved below
            default:
                if (len.HasPercent() && IsNaN(avail)) WarnPercentOnce(e);
                (void)width;
                return std::max(0.f, ResolveLen(len, avail));
            }
        };
        bool fitW, fitH, fillW, fillH;
        float w = axisKnown(s.Width, aw, true, fitW, fillW);
        float h = axisKnown(s.Height, ah, false, fitH, fillH);
        if (!IsNaN(forceW)) { w = forceW; fitW = false; fillW = false; }
        if (!IsNaN(forceH)) { h = forceH; fitH = false; fillH = false; }
        const bool wDependsOnH = s.Width.Kind == UiLenKind::HeightPercentage;
        const bool hDependsOnW = s.Height.Kind == UiLenKind::WidthPercentage;
        if (wDependsOnH && hDependsOnW)
        {
            UiMarkupLog::Error(e.ScopeFile, 0, 0, "width and height cannot both depend on each other; using fit-children");
            fitW = fitH = true; w = h = kNaN;
        }
        else if (wDependsOnH && !IsNaN(h)) { w = std::max(0.f, s.Width.Percent * 0.01f * h); }
        else if (hDependsOnW && !IsNaN(w)) { h = std::max(0.f, s.Height.Percent * 0.01f * w); }
        else if (wDependsOnH) { fitW = true; }
        else if (hDependsOnW) { fitH = true; }

        const bool needContent = IsNaN(w) || IsNaN(h);
        float contentW = 0.f, contentH = 0.f;
        const float insetW = b.pl + b.pr + b.bl + b.br, insetH = b.pt + b.pb + b.bt + b.bb;
        if (needContent || e.HasContent())
        {
            const float cw = (IsNaN(w) || fillW) ? kNaN : std::max(0.f, w - insetW);
            const float ch = (IsNaN(h) || fillH) ? kNaN : std::max(0.f, h - insetH);
            const std::vector<UiElem*> kids = LayoutChildren(e);
            const float gap = ResolveLen(s.Gap, 0.f);
            std::vector<float> ows, ohs, offs;
            float sumW = 0.f, sumH = 0.f, maxW = 0.f, maxH = 0.f;
            for (UiElem* k : kids)
            {
                const Size ks = Measure(*k, cw, ch);
                const Boxes kb = ResolveBoxes(k->Computed(), cw);
                const float ow = ks.W + kb.ml + kb.mr, oh = ks.H + kb.mt + kb.mb;
                const float offX = ResolveLen(k->Computed().X, cw), offY = ResolveLen(k->Computed().Y, ch);
                ows.push_back(ow); ohs.push_back(oh);
                maxW = std::max(maxW, std::max(0.f, offX) + ow);
                maxH = std::max(maxH, std::max(0.f, offY) + oh);
                sumW += ow; sumH += oh;
            }
            const size_t n = kids.size();
            const float gaps = n > 1 ? gap * (float)(n - 1) : 0.f;
            float plainW = 0.f, plainH = 0.f;
            for (size_t i = 0; i < n; ++i) { plainW = std::max(plainW, ows[i]); plainH = std::max(plainH, ohs[i]); }
            // Greedy lines for wrapping flows when the wrap axis is known.
            auto wrapped = [&](const std::vector<float>& mainSizes, const std::vector<float>& crossSizes, float limit, float& mainOut, float& crossOut) {
                float cursor = 0.f, lineCross = 0.f, widest = 0.f, total = 0.f;
                int lines = 0;
                for (size_t i = 0; i < n; ++i)
                {
                    if (lines == 0 || (cursor > 0.f && cursor + mainSizes[i] > limit))
                    {
                        if (lines > 0) total += lineCross + gap;
                        ++lines;
                        cursor = 0.f;
                        lineCross = 0.f;
                    }
                    cursor += mainSizes[i] + gap;
                    widest = std::max(widest, cursor - gap);
                    lineCross = std::max(lineCross, crossSizes[i]);
                }
                total += lineCross;
                mainOut = widest;
                crossOut = total;
            };
            switch (s.Flow)
            {
            case UiFlow::None: contentW = maxW; contentH = maxH; break;
            case UiFlow::Down: contentW = plainW; contentH = sumH + gaps; break;
            case UiFlow::Right: contentW = sumW + gaps; contentH = plainH; break;
            case UiFlow::RightWrap:
                if (!IsNaN(cw)) wrapped(ows, ohs, cw, contentW, contentH);
                else { contentW = sumW + gaps; contentH = plainH; }
                break;
            case UiFlow::DownWrap:
                if (!IsNaN(ch)) wrapped(ohs, ows, ch, contentH, contentW);
                else { contentW = plainW; contentH = sumH + gaps; }
                break;
            }
            if (e.HasContent())
            {
                const glm::vec2 own = e.MeasureContent(cw, ch);
                contentW = std::max(contentW, own.x);
                contentH = std::max(contentH, own.y);
            }
        }
        if (IsNaN(w)) w = contentW + insetW;
        if (IsNaN(h)) h = contentH + insetH;

        out.W = Clamp(w, s.MinWidth, s.MaxWidth, aw);
        out.H = Clamp(h, s.MinHeight, s.MaxHeight, ah);
        out.FillW = fillW;
        out.FillH = fillH;
        return out;
    }

    void Arrange(UiElem& e, const UiRect& rect, const glm::mat3& parentAbs);

    // transform: T(origin) * f1 * f2 ... * T(-origin), origin in border-box units. Layout itself ignores transforms.
    glm::mat3 LocalTransform(const UiComputedStyle& s, float w, float h)
    {
        if (s.Fx.Transform.empty())
            return glm::mat3(1.f);
        const glm::vec2 origin(s.Fx.OriginX.Resolve(w), s.Fx.OriginY.Resolve(h));
        return UiElement::Mat3Translate(origin) * UiTransformListMatrix(s.Fx.Transform, w, h) * UiElement::Mat3Translate(-origin);
    }

    float AlignOffset(int mode, float contentStart, float contentSize, float outer, float marginStart)
    {
        if (mode == 1) return contentStart + (contentSize - outer) * 0.5f + marginStart;
        if (mode == 2) return contentStart + contentSize - outer + marginStart;
        return contentStart + marginStart;
    }

    void Arrange(UiElem& e, const UiRect& rect, const glm::mat3& parentAbs)
    {
        const UiComputedStyle& s = e.Computed();
        e.BorderRect = rect;
        e.AbsoluteTransform = parentAbs * UiElement::Mat3Translate(glm::vec2(rect.X, rect.Y)) * LocalTransform(s, rect.W, rect.H);

        // Percentages inside this element resolve against its own content size; padding % against the parent's width (approximated by own).
        const Boxes b = ResolveBoxes(s, rect.W);
        UiRect content;
        content.X = b.bl + b.pl;
        content.Y = b.bt + b.pt;
        content.W = std::max(0.f, rect.W - b.bl - b.br - b.pl - b.pr);
        content.H = std::max(0.f, rect.H - b.bt - b.bb - b.pt - b.pb);

        if (e.HasContent())
            e.MeasureContent(content.W, content.H);   // text wraps at the final width

        const std::vector<UiElem*> kids = LayoutChildren(e);
        if (kids.empty())
            return;

        struct Item { UiElem* E; Size S; Boxes B; float ow, oh; };
        std::vector<Item> items;
        for (UiElem* k : kids)
        {
            Item it;
            it.E = k;
            it.S = Measure(*k, content.W, content.H);
            it.B = ResolveBoxes(k->Computed(), content.W);
            it.ow = it.S.W + it.B.ml + it.B.mr;
            it.oh = it.S.H + it.B.mt + it.B.mb;
            items.push_back(it);
        }
        const float gap = ResolveLen(s.Gap, content.W);

        auto offsetOf = [&](const Item& it, float& dx, float& dy) {
            dx = ResolveLen(it.E->Computed().X, content.W);
            dy = ResolveLen(it.E->Computed().Y, content.H);
        };

        if (s.Flow == UiFlow::None)
        {
            for (Item& it : items)
            {
                float w = it.S.FillW ? std::max(0.f, content.W - it.B.ml - it.B.mr) : it.S.W;
                float h = it.S.FillH ? std::max(0.f, content.H - it.B.mt - it.B.mb) : it.S.H;
                if (it.S.FillW || it.S.FillH)
                {
                    const Size re = Measure(*it.E, content.W, content.H, it.S.FillW ? w : kNaN, it.S.FillH ? h : kNaN);
                    if (!it.S.FillW) w = re.W;
                    if (!it.S.FillH) h = re.H;
                }
                const float ow = w + it.B.ml + it.B.mr, oh = h + it.B.mt + it.B.mb;
                float dx, dy;
                offsetOf(it, dx, dy);
                const UiComputedStyle& cs = it.E->Computed();
                const float x0 = AlignOffset((int)cs.HAlign, content.X, content.W, ow, it.B.ml) + dx;
                const float y0 = AlignOffset((int)cs.VAlign, content.Y, content.H, oh, it.B.mt) + dy;
                Arrange(*it.E, UiRect{ x0, y0, w, h }, e.AbsoluteTransform);
            }
            return;
        }

        const bool horizontal = s.Flow == UiFlow::Right || s.Flow == UiFlow::RightWrap;
        const bool wrap = s.Flow == UiFlow::RightWrap || s.Flow == UiFlow::DownWrap;
        // Main axis helpers (x for horizontal flows, y for vertical flows).
        auto mainOuter = [&](const Item& it) { return horizontal ? it.ow : it.oh; };
        auto crossOuter = [&](const Item& it) { return horizontal ? it.oh : it.ow; };
        auto mainFill = [&](const Item& it) { return horizontal ? it.S.FillW : it.S.FillH; };
        const float mainContent = horizontal ? content.W : content.H;
        const float crossContent = horizontal ? content.H : content.W;
        const float mainStart = horizontal ? content.X : content.Y;
        const float crossStart = horizontal ? content.Y : content.X;

        auto place = [&](Item& it, float mainPos, float mainSize, float crossPos, float crossSize) {
            float dx, dy;
            offsetOf(it, dx, dy);
            UiRect r;
            if (horizontal) r = UiRect{ mainPos + dx, crossPos + dy, mainSize, crossSize };
            else r = UiRect{ crossPos + dx, mainPos + dy, crossSize, mainSize };
            Arrange(*it.E, r, e.AbsoluteTransform);
        };
        auto crossAlignMode = [&](const Item& it) {
            return horizontal ? (int)it.E->Computed().VAlign : (int)it.E->Computed().HAlign;
        };
        auto crossFill = [&](const Item& it) { return horizontal ? it.S.FillH : it.S.FillW; };
        auto crossMarginStart = [&](const Item& it) { return horizontal ? it.B.mt : it.B.ml; };
        auto crossMarginTotal = [&](const Item& it) { return horizontal ? it.B.mt + it.B.mb : it.B.ml + it.B.mr; };
        auto mainMarginStart = [&](const Item& it) { return horizontal ? it.B.ml : it.B.mt; };
        auto mainMarginTotal = [&](const Item& it) { return horizontal ? it.B.ml + it.B.mr : it.B.mt + it.B.mb; };
        auto crossSizeOf = [&](const Item& it) { return horizontal ? it.S.H : it.S.W; };
        auto mainSizeOf = [&](const Item& it) { return horizontal ? it.S.W : it.S.H; };

        if (!wrap)
        {
            float used = 0.f, totalWeight = 0.f;
            for (const Item& it : items)
            {
                if (mainFill(it))
                {
                    used += mainMarginTotal(it);
                    totalWeight += (horizontal ? it.E->Computed().Width : it.E->Computed().Height).Weight;
                }
                else
                    used += mainOuter(it);
            }
            used += gap * (float)(items.size() - 1);
            const float freeSpace = std::max(0.f, mainContent - used);
            float cursor = mainStart;
            for (Item& it : items)
            {
                float mainSize = mainSizeOf(it);
                if (mainFill(it))
                {
                    const float weight = (horizontal ? it.E->Computed().Width : it.E->Computed().Height).Weight;
                    mainSize = totalWeight > 0.f ? freeSpace * weight / totalWeight : 0.f;
                }
                float crossSize = crossSizeOf(it);
                if (crossFill(it))
                    crossSize = std::max(0.f, crossContent - crossMarginTotal(it));
                if (mainFill(it) || crossFill(it))
                {
                    const float forceMain = mainFill(it) ? mainSize : kNaN, forceCross = crossFill(it) ? crossSize : kNaN;
                    const Size re = Measure(*it.E, content.W, content.H, horizontal ? forceMain : forceCross, horizontal ? forceCross : forceMain);
                    if (!mainFill(it)) mainSize = horizontal ? re.W : re.H;
                    if (!crossFill(it)) crossSize = horizontal ? re.H : re.W;
                }
                const float crossOuterSize = crossSize + crossMarginTotal(it);
                const float crossPos = AlignOffset(crossAlignMode(it), crossStart, crossContent, crossOuterSize, crossMarginStart(it));
                place(it, cursor + mainMarginStart(it), mainSize, crossPos, crossSize);
                cursor += mainSize + mainMarginTotal(it) + gap;
            }
            return;
        }

        // Wrapping flows: lines (rows for RightWrap, columns for DownWrap).
        std::vector<std::vector<Item*>> lines;
        float cursor = 0.f;
        for (Item& it : items)
        {
            if (mainFill(it))
                UiMarkupLog::Warning(e.ScopeFile, 0, 0, "fill-parent-flow() inside a wrapping flow counts as its minimum size");
            if (lines.empty() || (!lines.back().empty() && cursor + mainOuter(it) > mainContent))
            {
                lines.emplace_back();
                cursor = 0.f;
            }
            lines.back().push_back(&it);
            cursor += mainOuter(it) + gap;
        }
        float lineStart = crossStart;
        for (auto& line : lines)
        {
            float lineSize = 0.f;
            for (Item* it : line)
                lineSize = std::max(lineSize, crossOuter(*it));
            float mainCursor = mainStart;
            for (Item* it : line)
            {
                const float crossSize = crossFill(*it) ? std::max(0.f, lineSize - crossMarginTotal(*it)) : crossSizeOf(*it);
                const float crossPos = AlignOffset(crossAlignMode(*it), lineStart, lineSize, crossSize + crossMarginTotal(*it), crossMarginStart(*it));
                place(*it, mainCursor + mainMarginStart(*it), mainSizeOf(*it), crossPos, crossSize);
                mainCursor += mainOuter(*it) + gap;
            }
            lineStart += lineSize + gap;
        }
    }

    void PatchRecursive(UiElem& e, const glm::mat3& parentAbs)
    {
        const UiRect& r = e.BorderRect;
        e.AbsoluteTransform = parentAbs * UiElement::Mat3Translate(glm::vec2(r.X, r.Y)) * LocalTransform(e.Computed(), r.W, r.H);
        for (auto& c : e.children)
            if (UiElem::IsElem(c.get()))
                PatchRecursive(static_cast<UiElem&>(*c), e.AbsoluteTransform);
    }

    void UpdateOffsetsRecursive(UiElem& e)
    {
        e.UpdateOffsets();
        for (auto& c : e.children)
            if (UiElem::IsElem(c.get()))
                UpdateOffsetsRecursive(static_cast<UiElem&>(*c));
    }
}

void UiFlowLayout::PatchTransforms(UiElem& root, const glm::mat3& base)
{
    PatchRecursive(root, base);
    UpdateOffsetsRecursive(root);
}

void UiFlowLayout::LayoutRoot(UiElem& root, glm::vec2 containerSize, const glm::mat3& base)
{
    const UiComputedStyle& s = root.Computed();
    if (s.Visibility == UiVisibility::Collapse)
    {
        root.BorderRect = UiRect{};
        UpdateOffsetsRecursive(root);
        return;
    }
    const Size size = Measure(root, containerSize.x, containerSize.y);
    const Boxes b = ResolveBoxes(s, containerSize.x);
    float w = size.FillW ? std::max(0.f, containerSize.x - b.ml - b.mr) : size.W;
    float h = size.FillH ? std::max(0.f, containerSize.y - b.mt - b.mb) : size.H;
    const float ow = w + b.ml + b.mr, oh = h + b.mt + b.mb;
    const float x0 = AlignOffset((int)s.HAlign, 0.f, containerSize.x, ow, b.ml) + ResolveLen(s.X, containerSize.x);
    const float y0 = AlignOffset((int)s.VAlign, 0.f, containerSize.y, oh, b.mt) + ResolveLen(s.Y, containerSize.y);
    Arrange(root, UiRect{ x0, y0, w, h }, base);
    // Arrange derives the root transform from `base` and the rect; children use it as their parent transform.
    UpdateOffsetsRecursive(root);
}
