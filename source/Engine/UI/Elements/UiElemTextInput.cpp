#include "UiElemTextInput.h"

#include <Input.h>
#include <UI/Elements/UiElemValue.h>
#include <UI/UiNavigation.h>
#include <UI/UiSystem.h>

#include <SDL2/SDL.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace
{
    constexpr float kCaretW = 2.f;          // width of the caret, also kept free at the end of a line
    constexpr uint64_t kBlinkMs = 530;
    constexpr uint64_t kMultiClickMs = 400;

    bool IsFalsey(const std::string& v) { return v.empty() || v == "0" || v == "false" || v == "False" || v == "FALSE"; }

    // 0 white space, 1 word characters, 2 punctuation: a word movement or a double click stays inside one class
    int CharClass(uint32_t c)
    {
        if (c == ' ' || c == '\n' || c == '\t')
            return 0;
        if (c >= 128 || c == '_' || (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
            return 1;
        return 2;
    }

    void FillRect(UiPainter& painter, float x, float y, float w, float h, const glm::vec4& color)
    {
        if (w <= 0.f || h <= 0.f || color.a <= 0.f)
            return;
        UiBoxPaint paint;
        paint.Fill = color;
        painter.Box(UiRect{ x, y, w, h }, paint);
    }
}

UiElemTextInput::UiElemTextInput(bool multiline) : m_multiline(multiline)
{
    HitCheck = true;
    DisableFocus = false;
    // the box scrolls its text: the layout measures the content, clamps the offset and places the scrollbar
    Style().Overflow = UiOverflow::Scroll;
}

void UiElemTextInput::OnBuilt()
{
    if (!m_multiline)
        return;
    const auto it = Attributes.find("scrollbar");
    if (it == Attributes.end() || !IsFalsey(it->second))
        EnsureScrollbar();
}

// ── text

std::vector<uint32_t> UiElemTextInput::Sanitize(const std::string& utf8) const
{
    std::vector<uint32_t> out;
    for (uint32_t c : UiDecodeUtf8(utf8))
    {
        if (c == '\r')
            continue;
        if (c == '\t')
        {
            out.insert(out.end(), 4, uint32_t(' '));
            continue;
        }
        if (c == '\n' && !m_multiline)
            c = ' ';
        if ((c < 0x20 && c != '\n') || c == 0x7F)
            continue;
        out.push_back(c);
    }
    if (m_maxLength > 0 && out.size() > m_maxLength)
        out.resize(m_maxLength);
    return out;
}

UiElemTextInput::~UiElemTextInput()
{
    if (m_editing)
        Input::TextInputActive = false;
}

void UiElemTextInput::SyncUtf8()
{
    m_utf8 = UiEncodeUtf8(m_text);
}

void UiElemTextInput::PublishValue()
{
    if (m_utf8 == m_published)
        return;
    m_published = m_utf8;
    SetVar("value", m_utf8);
    if (UiElem* parent = AsParentElem())
        parent->SetVar("value", m_utf8);
}

void UiElemTextInput::SetText(const std::string& text, bool notify)
{
    std::vector<uint32_t> cps = Sanitize(text);
    if (cps == m_text)
        return;
    m_text = std::move(cps);
    m_caret = m_anchor = m_text.size();
    m_undo.clear();
    m_redo.clear();
    m_lastEdit = EditKind::None;
    TextChanged(notify);
}

void UiElemTextInput::TextChanged(bool fromUser)
{
    m_caret = std::min(m_caret, m_text.size());
    m_anchor = std::min(m_anchor, m_text.size());
    SyncUtf8();
    PublishValue();
    m_desiredX = -1.f;
    m_followCaret = true;
    ResetBlink();
    MarkLayoutDirty();
    if (fromUser)
    {
        Queue("input");
        Queue("valuechanged");
    }
}

std::string UiElemTextInput::SelectedText() const
{
    return UiEncodeUtf8(std::vector<uint32_t>(m_text.begin() + SelectionStart(), m_text.begin() + SelectionEnd()));
}

void UiElemTextInput::SetSelection(size_t anchor, size_t caret)
{
    m_anchor = std::min(anchor, m_text.size());
    m_caret = std::min(caret, m_text.size());
    m_lastEdit = EditKind::None;
    m_desiredX = -1.f;
    m_followCaret = true;
    ResetBlink();
}

void UiElemTextInput::Queue(const char* event)
{
    UiSystem::QueueEvent(*this, event);
}

void UiElemTextInput::ResetBlink()
{
    m_blinkStart = SDL_GetTicks64();
    MarkPaintDirty();
}

std::vector<uint32_t> UiElemTextInput::DisplayText() const
{
    if (m_password)
        return std::vector<uint32_t>(m_text.size(), uint32_t('*'));
    return m_text;
}

// ── layout

bool UiElemTextInput::Wraps() const
{
    const UiWhiteSpace ws = Computed().WhiteSpace;
    return m_multiline && m_wrap && ws != UiWhiteSpace::Pre && ws != UiWhiteSpace::NoWrap;
}

UiTextParams UiElemTextInput::MakeParams(const std::vector<uint32_t>& text, float boxWidth, bool carets) const
{
    const UiComputedStyle& s = Computed();
    UiTextParams p;
    p.Text = UiEncodeUtf8(text);
    p.Family = s.FontFamily;
    p.Weight = s.FontWeight;
    p.Italic = s.FontItalic;
    p.FontSize = s.FontSize;
    p.LetterSpacing = s.LetterSpacing;
    p.LineHeight = s.LineHeight;
    p.Align = s.TextAlign;
    p.WhiteSpace = Wraps() ? UiWhiteSpace::PreWrap : UiWhiteSpace::Pre;
    p.WrapWidth = boxWidth < 0.f ? -1.f : std::max(0.f, boxWidth - kCaretW);
    p.WantCarets = carets;
    return p;
}

const UiTextLayoutResult& UiElemTextInput::BuildLayout(float boxWidth) const
{
    const UiTextParams p = MakeParams(DisplayText(), boxWidth, true);
    if (!m_haveLayout || !(p == m_lastParams))
    {
        m_layout = UiLayoutText(p);
        m_lastParams = p;
        m_haveLayout = true;
    }
    return m_layout;
}

const UiTextLayoutResult& UiElemTextInput::BuildPlaceholder(float boxWidth) const
{
    const UiTextParams p = MakeParams(Sanitize(m_placeholder), boxWidth, false);
    if (!m_havePlaceholder || !(p == m_lastPlaceholderParams))
    {
        m_placeholderLayout = UiLayoutText(p);
        m_lastPlaceholderParams = p;
        m_havePlaceholder = true;
    }
    return m_placeholderLayout;
}

float UiElemTextInput::LineHeightOf(const UiTextLayoutResult& layout) const
{
    return layout.LineHeight > 0.f ? layout.LineHeight : Computed().FontSize * 1.2f;
}

glm::vec2 UiElemTextInput::TextOrigin(const UiRect& content, const UiTextLayoutResult& layout) const
{
    float y = content.Y - ScrollOffset.y;
    if (!m_multiline)
        y += std::max(0.f, (content.H - LineHeightOf(layout)) * 0.5f);   // one line sits in the middle of a taller box
    return glm::vec2(content.X - ScrollOffset.x, y);
}

int UiElemTextInput::LineCount() const
{
    const float w = BorderRect.W > 0.f ? ContentRectLocal().W : -1.f;
    return std::max<int>(1, (int)BuildLayout(w).LineInfo.size());
}

glm::vec2 UiElemTextInput::MeasureContent(float availableWidth, float)
{
    const float known = std::isnan(availableWidth) ? -1.f : availableWidth;
    const UiTextLayoutResult& layout = BuildLayout(known);
    const float lh = LineHeightOf(layout);
    float width = layout.Width;
    int lines = std::max(1, layout.Lines);
    if (m_text.empty() && !m_placeholder.empty())
    {
        const UiTextLayoutResult& ph = BuildPlaceholder(known);
        width = std::max(width, ph.Width);
        lines = std::max(lines, ph.Lines);
    }
    lines = std::max(lines, std::max(1, m_minRows));
    if (m_maxRows > 0)
        lines = std::min(lines, m_maxRows);
    return glm::vec2(width + kCaretW, lh * (float)(m_multiline ? lines : 1));
}

glm::vec2 UiElemTextInput::VirtualContentSize() const
{
    const float w = BorderRect.W > 0.f ? ContentRectLocal().W : -1.f;
    const UiTextLayoutResult& layout = BuildLayout(w);
    return glm::vec2(layout.Width + kCaretW, std::max(layout.Height, LineHeightOf(layout)));
}

// ── caret geometry

int UiElemTextInput::LineOf(size_t index) const
{
    const float w = BorderRect.W > 0.f ? ContentRectLocal().W : -1.f;
    const UiTextLayoutResult& layout = BuildLayout(w);
    return index < layout.Carets.size() ? layout.Carets[index].Line : 0;
}

float UiElemTextInput::CaretX(size_t index) const
{
    const float w = BorderRect.W > 0.f ? ContentRectLocal().W : -1.f;
    const UiTextLayoutResult& layout = BuildLayout(w);
    return index < layout.Carets.size() ? layout.Carets[index].X : 0.f;
}

size_t UiElemTextInput::LineFirst(int line) const
{
    const float w = BorderRect.W > 0.f ? ContentRectLocal().W : -1.f;
    const UiTextLayoutResult& layout = BuildLayout(w);
    if (layout.LineInfo.empty())
        return 0;
    return (size_t)layout.LineInfo[std::clamp<int>(line, 0, (int)layout.LineInfo.size() - 1)].First;
}

size_t UiElemTextInput::LineLast(int line) const
{
    const float w = BorderRect.W > 0.f ? ContentRectLocal().W : -1.f;
    const UiTextLayoutResult& layout = BuildLayout(w);
    if (layout.LineInfo.empty())
        return 0;
    const int n = (int)layout.LineInfo.size();
    line = std::clamp(line, 0, n - 1);
    const UiLineInfo& info = layout.LineInfo[line];
    // at a soft wrap the end boundary is drawn as the start of the next line
    const bool soft = line + 1 < n && layout.LineInfo[line + 1].First == info.End;
    return (size_t)(soft ? std::max(info.First, info.End - 1) : info.End);
}

size_t UiElemTextInput::IndexOnLine(int line, float x) const
{
    const float w = BorderRect.W > 0.f ? ContentRectLocal().W : -1.f;
    const UiTextLayoutResult& layout = BuildLayout(w);
    if (layout.LineInfo.empty())
        return 0;
    const int n = (int)layout.LineInfo.size();
    line = std::clamp(line, 0, n - 1);
    const UiLineInfo& info = layout.LineInfo[line];
    const size_t last = LineLast(line);
    size_t best = (size_t)info.First;
    float bestDistance = 1e30f;
    for (size_t i = (size_t)info.First; i <= last && i < layout.Carets.size(); ++i)
    {
        const float cx = (int)i == info.End ? info.EndX : layout.Carets[i].X;
        const float d = std::fabs(cx - x);
        if (d < bestDistance)
        {
            bestDistance = d;
            best = i;
        }
    }
    return best;
}

size_t UiElemTextInput::HitIndex(glm::vec2 local) const
{
    const float w = BorderRect.W > 0.f ? ContentRectLocal().W : -1.f;
    const UiTextLayoutResult& layout = BuildLayout(w);
    const glm::vec2 origin = TextOrigin(ContentRectLocal(), layout);
    const float lh = LineHeightOf(layout);
    const int n = std::max<int>(1, (int)layout.LineInfo.size());
    const int line = std::clamp((int)std::floor((local.y - origin.y) / lh), 0, n - 1);
    return IndexOnLine(line, local.x - origin.x);
}

size_t UiElemTextInput::PrevWord(size_t pos) const
{
    pos = std::min(pos, m_text.size());
    while (pos > 0 && CharClass(m_text[pos - 1]) == 0)
        --pos;
    if (pos > 0)
    {
        const int cls = CharClass(m_text[pos - 1]);
        while (pos > 0 && CharClass(m_text[pos - 1]) == cls)
            --pos;
    }
    return pos;
}

size_t UiElemTextInput::NextWord(size_t pos) const
{
    const size_t n = m_text.size();
    pos = std::min(pos, n);
    if (pos < n)
    {
        const int cls = CharClass(m_text[pos]);
        if (cls != 0)
            while (pos < n && CharClass(m_text[pos]) == cls)
                ++pos;
        while (pos < n && CharClass(m_text[pos]) == 0)
            ++pos;
    }
    return pos;
}

void UiElemTextInput::SelectWordAt(size_t pos)
{
    const size_t n = m_text.size();
    if (n == 0)
        return;
    size_t i = std::min(pos, n);
    if (i == n)
        --i;
    const int cls = CharClass(m_text[i]);
    size_t a = i, b = i + 1;
    while (a > 0 && CharClass(m_text[a - 1]) == cls)
        --a;
    while (b < n && CharClass(m_text[b]) == cls)
        ++b;
    SetSelection(a, b);
}

void UiElemTextInput::SelectLineAt(size_t pos)
{
    if (!m_multiline)
    {
        SelectAll();
        return;
    }
    const int line = LineOf(pos);
    SetSelection(LineFirst(line), LineLast(line));
}

// ── editing

bool UiElemTextInput::Accept(uint32_t c, const std::vector<uint32_t>& pending) const
{
    if (c == '\n')
        return m_multiline;
    if ((c < 0x20) || c == 0x7F || (c >= 0xD800 && c <= 0xDFFF))
        return false;
    if (!m_allowed.empty() && m_allowed.find(c) == m_allowed.end())
        return false;
    if (m_numeric || m_integer)
    {
        if (c >= '0' && c <= '9')
            return true;
        // the text that stays when the selection is replaced
        const size_t a = SelectionStart(), b = SelectionEnd();
        auto has = [&](uint32_t ch) {
            for (size_t i = 0; i < m_text.size(); ++i)
                if ((i < a || i >= b) && m_text[i] == ch)
                    return true;
            return std::find(pending.begin(), pending.end(), ch) != pending.end();
        };
        if (c == '-')
            return a + pending.size() == 0 && !has('-');
        if (c == '.' && !m_integer)
            return !has('.');
        return false;
    }
    if (CharFilter && !CharFilter(c))
        return false;
    return true;
}

bool UiElemTextInput::InsertCodepoints(const std::vector<uint32_t>& cps, bool typed)
{
    if (m_readOnly)
        return false;
    const size_t a = SelectionStart(), b = SelectionEnd();
    size_t capacity = (size_t)-1;
    if (m_maxLength > 0)
    {
        const size_t kept = m_text.size() - (b - a);
        capacity = m_maxLength > kept ? m_maxLength - kept : 0;
    }
    std::vector<uint32_t> clean;
    bool rejected = false;
    for (uint32_t c : cps)
    {
        if (c == '\r')
            continue;
        const int repeat = c == '\t' ? 4 : 1;
        for (int r = 0; r < repeat; ++r)
        {
            uint32_t ch = c == '\t' ? uint32_t(' ') : c;
            if (ch == '\n' && !m_multiline)
                ch = ' ';
            if (!Accept(ch, clean) || clean.size() >= capacity)
            {
                rejected = true;
                continue;
            }
            clean.push_back(ch);
        }
    }
    if (rejected)
        Queue("inputrejected");
    if (clean.empty())
        return false;
    ReplaceRange(a, b, clean, typed && clean.size() == 1 ? EditKind::Typing : EditKind::Other);
    return true;
}

void UiElemTextInput::InsertText(const std::string& utf8)
{
    if (!m_editing)
        BeginEditing(false);
    InsertCodepoints(UiDecodeUtf8(utf8), false);
}

void UiElemTextInput::PushUndo(EditKind kind)
{
    const bool joins = (kind == EditKind::Typing || kind == EditKind::DeleteBack || kind == EditKind::DeleteForward) && kind == m_lastEdit &&
                       !HasSelection() && m_caret == m_lastEditCaret && !m_undo.empty();
    m_redo.clear();
    if (joins)
        return;
    m_undo.push_back(Snapshot{ m_text, m_caret, m_anchor });
    if (m_undo.size() > 200)
        m_undo.erase(m_undo.begin());
}

void UiElemTextInput::ReplaceRange(size_t begin, size_t end, const std::vector<uint32_t>& insert, EditKind kind)
{
    begin = std::min(begin, m_text.size());
    end = std::clamp(end, begin, m_text.size());
    if (begin == end && insert.empty())
        return;
    PushUndo(kind);
    m_text.erase(m_text.begin() + begin, m_text.begin() + end);
    m_text.insert(m_text.begin() + begin, insert.begin(), insert.end());
    m_caret = m_anchor = begin + insert.size();
    m_lastEdit = kind;
    m_lastEditCaret = m_caret;
    TextChanged(true);
}

void UiElemTextInput::DeleteBy(int direction, bool word)
{
    if (m_readOnly)
        return;
    if (HasSelection())
    {
        ReplaceRange(SelectionStart(), SelectionEnd(), {}, EditKind::Other);
        return;
    }
    if (direction < 0)
    {
        if (m_caret == 0)
            return;
        ReplaceRange(word ? PrevWord(m_caret) : m_caret - 1, m_caret, {}, EditKind::DeleteBack);
    }
    else
    {
        if (m_caret >= m_text.size())
            return;
        ReplaceRange(m_caret, word ? NextWord(m_caret) : m_caret + 1, {}, EditKind::DeleteForward);
    }
}

bool UiElemTextInput::Undo()
{
    if (m_undo.empty())
        return false;
    m_redo.push_back(Snapshot{ m_text, m_caret, m_anchor });
    const Snapshot s = m_undo.back();
    m_undo.pop_back();
    m_text = s.Text;
    m_caret = s.Caret;
    m_anchor = s.Anchor;
    m_lastEdit = EditKind::None;
    TextChanged(true);
    return true;
}

bool UiElemTextInput::Redo()
{
    if (m_redo.empty())
        return false;
    m_undo.push_back(Snapshot{ m_text, m_caret, m_anchor });
    const Snapshot s = m_redo.back();
    m_redo.pop_back();
    m_text = s.Text;
    m_caret = s.Caret;
    m_anchor = s.Anchor;
    m_lastEdit = EditKind::None;
    TextChanged(true);
    return true;
}

void UiElemTextInput::Copy()
{
    if (m_password || !HasSelection())
        return;
    SDL_SetClipboardText(SelectedText().c_str());
}

void UiElemTextInput::Cut()
{
    if (m_readOnly || m_password || !HasSelection())
        return;
    Copy();
    DeleteBy(-1, false);
}

void UiElemTextInput::Paste()
{
    if (m_readOnly)
        return;
    char* text = SDL_GetClipboardText();
    if (text != nullptr)
    {
        if (*text != 0)
            InsertCodepoints(UiDecodeUtf8(text), false);
        SDL_free(text);
    }
}

void UiElemTextInput::MoveCaretTo(size_t pos, bool extend, bool keepDesiredX)
{
    m_caret = std::min(pos, m_text.size());
    if (!extend)
        m_anchor = m_caret;
    if (!keepDesiredX)
        m_desiredX = -1.f;
    m_lastEdit = EditKind::None;
    m_followCaret = true;
    ResetBlink();
}

void UiElemTextInput::MoveHorizontal(int direction, bool word, bool extend)
{
    if (!extend && HasSelection())
    {
        MoveCaretTo(direction < 0 ? SelectionStart() : SelectionEnd(), false);   // the selection collapses to the side that was pressed
        return;
    }
    size_t pos = m_caret;
    if (word)
        pos = direction < 0 ? PrevWord(pos) : NextWord(pos);
    else if (direction < 0)
        pos = pos > 0 ? pos - 1 : 0;
    else
        pos = std::min(pos + 1, m_text.size());
    MoveCaretTo(pos, extend);
}

void UiElemTextInput::MoveVertical(int lines, bool extend)
{
    if (!m_multiline)
    {
        MoveCaretTo(lines < 0 ? 0 : m_text.size(), extend);   // one line: up and down go to the ends
        return;
    }
    const int line = LineOf(m_caret);
    const int count = LineCount();
    const int target = std::clamp(line + lines, 0, count - 1);
    if (target == line)
    {
        MoveCaretTo(lines < 0 ? 0 : m_text.size(), extend, true);
        return;
    }
    const float x = m_desiredX >= 0.f ? m_desiredX : CaretX(m_caret);
    MoveCaretTo(IndexOnLine(target, x), extend, true);
    m_desiredX = x;
}

void UiElemTextInput::MoveToLineEdge(bool end, bool wholeText, bool extend)
{
    if (wholeText)
    {
        MoveCaretTo(end ? m_text.size() : 0, extend);
        return;
    }
    const int line = LineOf(m_caret);
    MoveCaretTo(end ? LineLast(line) : LineFirst(line), extend);
}

void UiElemTextInput::HandleKey(int scancode, bool ctrl, bool shift)
{
    switch (scancode)
    {
    case SDL_SCANCODE_LEFT: MoveHorizontal(-1, ctrl, shift); break;
    case SDL_SCANCODE_RIGHT: MoveHorizontal(1, ctrl, shift); break;
    case SDL_SCANCODE_UP: MoveVertical(-1, shift); break;
    case SDL_SCANCODE_DOWN: MoveVertical(1, shift); break;
    case SDL_SCANCODE_HOME: MoveToLineEdge(false, ctrl, shift); break;
    case SDL_SCANCODE_END: MoveToLineEdge(true, ctrl, shift); break;
    case SDL_SCANCODE_PAGEUP:
    case SDL_SCANCODE_PAGEDOWN:
    {
        const float lh = LineHeightOf(BuildLayout(BorderRect.W > 0.f ? ContentRectLocal().W : -1.f));
        const int page = std::max(1, (int)std::floor(ScrollViewport.y / std::max(1.f, lh)) - 1);
        MoveVertical(scancode == SDL_SCANCODE_PAGEUP ? -page : page, shift);
        break;
    }
    case SDL_SCANCODE_BACKSPACE: DeleteBy(-1, ctrl); break;
    case SDL_SCANCODE_DELETE: DeleteBy(1, ctrl); break;
    case SDL_SCANCODE_A: if (ctrl) SelectAll(); break;
    case SDL_SCANCODE_C: if (ctrl) Copy(); break;
    case SDL_SCANCODE_X: if (ctrl) Cut(); break;
    case SDL_SCANCODE_V: if (ctrl) Paste(); break;
    case SDL_SCANCODE_Z: if (ctrl) { if (shift) Redo(); else Undo(); } break;
    case SDL_SCANCODE_Y: if (ctrl) Redo(); break;
    case SDL_SCANCODE_INSERT:
        if (shift) Paste();
        else if (ctrl) Copy();
        break;
    default: break;
    }
}

void UiElemTextInput::Submit()
{
    if (!m_editing)
        return;
    Queue("submit");
    EndEditing(true);
    if (m_blurOnSubmit)
        UiNavigation::SetFocus(nullptr);
}

void UiElemTextInput::BeginEditing(bool selectAll)
{
    if (m_editing)
        return;
    if (m_readOnly || (States & kStateDisabled))
    {
        Queue("denied");
        return;
    }
    m_editing = true;
    Input::TextInputActive = true;
    Input::TextInputBuffer.clear();
    Input::TextKeyEvents.clear();
    m_textAtEditStart = m_text;
    m_undo.clear();
    m_redo.clear();
    m_lastEdit = EditKind::None;
    m_caret = m_anchor = m_text.size();
    if (selectAll)
        SelectAll();
    SetClass("editing", true);
    m_followCaret = true;
    ResetBlink();
    Queue("editstart");
}

void UiElemTextInput::EndEditing(bool commit)
{
    if (!m_editing)
        return;
    const bool changed = m_text != m_textAtEditStart;
    if (!commit && changed)
    {
        m_text = m_textAtEditStart;
        m_caret = m_anchor = m_text.size();
        TextChanged(true);
    }
    m_editing = false;
    m_selecting = false;
    Input::TextInputActive = false;
    m_anchor = m_caret;
    SetClass("editing", false);
    if (!commit)
        Queue("editcancel");
    else if (changed)
        Queue("change");
    Queue("editend");
    MarkPaintDirty();
}

// ── navigation

void UiElemTextInput::OnFocused()
{
    MarkPaintDirty();
}

void UiElemTextInput::OnUnfocused()
{
    if (m_editing)
        EndEditing(true);
    MarkPaintDirty();
}

void UiElemTextInput::OnNavConfirm()
{
    if (!m_editing)
    {
        BeginEditing(m_selectOnFocus);
        return;
    }
    if (!m_multiline)
    {
        Submit();
        return;
    }
    const bool ctrl = Input::activeKeys.count(SDL_SCANCODE_LCTRL) || Input::activeKeys.count(SDL_SCANCODE_RCTRL);
    if (ctrl)
        Submit();
    else
        InsertCodepoints({ uint32_t('\n') }, true);
}

bool UiElemTextInput::OnNav(UiNavDir dir)
{
    if (!m_editing)
        return false;
    // The keyboard moves the caret from its own key events (they carry Shift, Ctrl and key repeat); a gamepad has no such event.
    const int key = dir == UiNavDir::Left ? SDL_SCANCODE_LEFT : dir == UiNavDir::Right ? SDL_SCANCODE_RIGHT : dir == UiNavDir::Up ? SDL_SCANCODE_UP : SDL_SCANCODE_DOWN;
    for (const Input::TextKeyEvent& k : Input::TextKeyEvents)
        if ((int)k.Scancode == key)
            return true;
    HandleKey(key, false, false);
    return true;
}

bool UiElemTextInput::OnFocusedCancel()
{
    if (!m_editing)
        return false;
    EndEditing(false);
    return true;
}

// ── pointer

void UiElemTextInput::OnPointerDown(glm::vec2 local)
{
    UiElem::OnPointerDown(local);   // finds out whether the press holds the scrollbar thumb
    m_barPress = ScrollbarPressActive();
    if (m_barPress || (States & kStateDisabled) || m_readOnly)
        return;
    UiNavigation::SetFocus(this);
    const bool wasEditing = m_editing;
    if (!wasEditing)
        BeginEditing(m_selectOnFocus);
    if (!m_editing)
        return;
    if (!wasEditing && m_selectOnFocus)
    {
        m_clicks = 1;
        return;
    }
    const uint64_t now = SDL_GetTicks64();
    const bool again = m_clicks > 0 && now - m_lastDownTicks <= kMultiClickMs && glm::length(local - m_lastDownPos) <= 6.f;
    m_clicks = again ? (m_clicks % 3) + 1 : 1;
    m_lastDownTicks = now;
    m_lastDownPos = local;
    const size_t index = HitIndex(local);
    const bool shift = Input::activeKeys.count(SDL_SCANCODE_LSHIFT) || Input::activeKeys.count(SDL_SCANCODE_RSHIFT);
    if (m_clicks == 2)
        SelectWordAt(index);
    else if (m_clicks == 3)
        SelectLineAt(index);
    else
    {
        MoveCaretTo(index, shift);
        m_selecting = true;
    }
}

void UiElemTextInput::OnPointerMove(glm::vec2 local)
{
    if (m_barPress)
    {
        UiElem::OnPointerMove(local);
        return;
    }
    if (m_selecting && m_editing)
        MoveCaretTo(HitIndex(local), true);
}

void UiElemTextInput::OnPointerUp(glm::vec2 local)
{
    UiElem::OnPointerUp(local);
    m_selecting = false;
    m_barPress = false;
}

// ── per frame

void UiElemTextInput::FollowCaret()
{
    if (!m_followCaret || ScrollViewport.x <= 0.f || ScrollViewport.y <= 0.f)
        return;   // not laid out yet: try again next frame
    m_followCaret = false;
    const float boxWidth = BorderRect.W > 0.f ? ContentRectLocal().W : -1.f;
    const UiTextLayoutResult& layout = BuildLayout(boxWidth);
    if (m_caret >= layout.Carets.size())
        return;
    const UiCaretStop stop = layout.Carets[m_caret];
    const float lh = LineHeightOf(layout);
    const glm::vec2 view = ScrollViewport;
    const glm::vec2 extent(layout.Width + kCaretW, std::max(layout.Height, lh));
    glm::vec2 offset = ScrollOffset;
    if (stop.X < offset.x)
        offset.x = stop.X;
    else if (stop.X + kCaretW > offset.x + view.x)
        offset.x = stop.X + kCaretW - view.x;
    if (stop.Y < offset.y)
        offset.y = stop.Y;
    else if (stop.Y + lh > offset.y + view.y)
        offset.y = stop.Y + lh - view.y;
    offset = glm::clamp(offset, glm::vec2(0.f), glm::max(extent - view, glm::vec2(0.f)));
    if (offset != ScrollOffset || offset != ScrollTarget)
    {
        ScrollOffset = ScrollTarget = offset;
        Root()->m_transformDirty = true;
        MarkPaintDirty();
    }
}

void UiElemTextInput::Tick()
{
    if (m_editing)
    {
        if ((States & kStateDisabled) || !IsVisible())
            EndEditing(true);
    }
    if (m_editing)
    {
        // a press anywhere else ends the edit (hovering elsewhere does not: the field keeps the focus)
        for (const auto& entry : Input::TouchActions)
        {
            if (!entry.second.pressed)
                continue;
            const glm::vec2 l = UiElement::TransformPoint(glm::inverse(AbsoluteTransform), entry.second.position);
            if (l.x < 0.f || l.y < 0.f || l.x > BorderRect.W || l.y > BorderRect.H)
            {
                EndEditing(true);
                break;
            }
        }
    }
    if (m_editing)
    {
        for (const Input::TextKeyEvent& k : Input::TextKeyEvents)
            HandleKey((int)k.Scancode, k.Ctrl, k.Shift);
        if (!Input::TextInputBuffer.empty())
        {
            // the control bytes that ride along (backspace, delete, enter, escape) are handled from the key events and navigation
            std::string text;
            for (char c : Input::TextInputBuffer)
                if (c != '\b' && c != '\x7F' && c != '\r' && c != '\n' && c != '\x1B')
                    text += c;
            Input::TextInputBuffer.clear();
            if (!text.empty())
                InsertCodepoints(UiDecodeUtf8(text), true);
        }
    }

    const bool shown = m_editing && !m_readOnly && ((SDL_GetTicks64() - m_blinkStart) / kBlinkMs) % 2 == 0;
    if (shown != m_caretShown)
    {
        m_caretShown = shown;
        MarkPaintDirty();
    }
    FollowCaret();
}

// ── attributes

bool UiElemTextInput::ApplyAttribute(const std::string& name, const std::string& value)
{
    if (name == "text") { SetText(value); return true; }
    if (name == "placeholder") { m_placeholder = value; m_havePlaceholder = false; return true; }
    if (name == "max-length")
    {
        m_maxLength = (size_t)std::max(0, std::atoi(value.c_str()));
        if (m_maxLength > 0 && m_text.size() > m_maxLength)
        {
            m_text.resize(m_maxLength);
            TextChanged(false);
        }
        return true;
    }
    if (name == "password") { m_password = !IsFalsey(value); return true; }
    if (name == "numeric") { m_numeric = !IsFalsey(value); return true; }
    if (name == "integer") { m_integer = !IsFalsey(value); return true; }
    if (name == "read-only")
    {
        m_readOnly = !IsFalsey(value);
        if (m_readOnly && m_editing)
            EndEditing(true);
        return true;
    }
    if (name == "allowed-chars")
    {
        m_allowed.clear();
        for (uint32_t c : UiDecodeUtf8(value))
            m_allowed.insert(c);
        return true;
    }
    if (name == "select-on-focus") { m_selectOnFocus = !IsFalsey(value); return true; }
    if (name == "blur-on-submit") { m_blurOnSubmit = !IsFalsey(value); return true; }
    if (name == "wrap") { m_wrap = !IsFalsey(value); return true; }
    if (name == "rows" || name == "min-rows") { m_minRows = std::max(1, std::atoi(value.c_str())); return true; }
    if (name == "max-rows") { m_maxRows = std::max(0, std::atoi(value.c_str())); return true; }
    return false;
}

// ── paint

void UiElemTextInput::PaintContent(UiPainter& painter, const UiRect& content)
{
    if (content.W <= 0.f || content.H <= 0.f)
        return;
    const UiComputedStyle& s = Computed();
    const UiTextLayoutResult& layout = BuildLayout(content.W);
    const float lh = LineHeightOf(layout);

    glm::vec2 origin = TextOrigin(content, layout);
    const glm::mat3& t = painter.Transform();
    if (t[0][0] == 1.f && t[1][1] == 1.f && t[0][1] == 0.f && t[1][0] == 0.f)
    {
        // whole target pixels for the text origin, so glyph edges look the same on every backend (as the Label does)
        origin.x = std::round(origin.x + t[2][0]) - t[2][0];
        origin.y = std::round(origin.y + t[2][1]) - t[2][1];
    }

    painter.PushClip(content, UiCorners{});

    // selection
    if (m_editing && HasSelection() && !layout.LineInfo.empty())
    {
        const glm::vec4 base = UiCustomColor(s, "--selection-color", glm::vec4(0.30f, 0.50f, 0.90f, 0.45f));
        const glm::vec4 color(base.r, base.g, base.b, base.a * m_paintOpacity);
        const int a = (int)SelectionStart(), b = (int)SelectionEnd();
        for (size_t li = 0; li < layout.LineInfo.size(); ++li)
        {
            const UiLineInfo& info = layout.LineInfo[li];
            const int from = std::max(a, info.First), to = std::min(b, info.End);
            const bool hardBreak = li + 1 < layout.LineInfo.size() && layout.LineInfo[li + 1].First == info.End + 1;
            const bool newlineSelected = hardBreak && b > info.End && a <= info.End;
            if (from > to || (from == to && !newlineSelected))
                continue;
            const float x0 = from == info.End ? info.EndX : layout.Carets[from].X;
            float x1 = to == info.End ? info.EndX : layout.Carets[to].X;
            if (newlineSelected)
                x1 += s.FontSize * 0.3f;   // the selected line break shows as a short tail
            FillRect(painter, origin.x + x0, origin.y + info.Y, x1 - x0, lh, color);
        }
    }

    // text, or the placeholder while empty
    const UiTextLayoutResult* shown = &layout;
    glm::vec4 textColor = s.Color;
    if (m_text.empty() && !m_placeholder.empty())
    {
        shown = &BuildPlaceholder(content.W);
        textColor = UiCustomColor(s, "--placeholder-color", glm::vec4(s.Color.r, s.Color.g, s.Color.b, s.Color.a * 0.45f));
    }
    textColor.a *= m_paintOpacity;
    if (!shown->Quads.empty())
    {
        std::vector<UiGlyphQuad> quads = shown->Quads;
        for (UiGlyphQuad& q : quads)
        {
            q.X += origin.x;
            q.Y += origin.y;
        }
        for (const UiTextRun& run : shown->Runs)
            painter.GlyphRun(quads.data() + run.First, run.Count, textColor, run.GlyphScale, run.Dilation);
    }

    // caret
    if (m_caretShown && m_caret < layout.Carets.size())
    {
        glm::vec4 caret = UiCustomColor(s, "--caret-color", s.Color);
        caret.a *= m_paintOpacity;
        const UiCaretStop stop = layout.Carets[m_caret];
        FillRect(painter, origin.x + stop.X, origin.y + stop.Y, kCaretW, lh, caret);
    }

    painter.PopClip();
}
