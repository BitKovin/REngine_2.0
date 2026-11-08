#pragma once
#include <UI/Elements/UiElem.h>
#include <UI/Render/UiTextLayout.h>

#include <functional>
#include <set>
#include <string>
#include <vector>

// The text fields: TextEntry (one line) and MultilineTextEntry (wrapped, grows with its content up to max-rows, then scrolls).
//
// Focus has two stages. A field that merely has navigation focus (hover, arrows, gamepad) leaves the arrow keys to navigation;
// confirm or a click starts editing, and while editing the field owns the keyboard and keeps the focus. Enter (Ctrl+Enter in a
// multi-line field) commits, Escape / ui_cancel reverts to the text from before the edit, and leaving the field commits.
//
// Keyboard: arrows, Ctrl+arrows (words), Home/End (Ctrl: whole text), PageUp/PageDown, Shift extends the selection, Ctrl+A,
// Ctrl+C / X / V, Ctrl+Z / Y (Ctrl+Shift+Z), Backspace / Delete (Ctrl: word). Pointer: click places the caret, drag selects,
// double click selects a word, triple click the line (the whole text in one-line fields), Shift+click extends.
//
// Events (on<name> attributes, UiElem::Listen, sound-* styles): input, valuechanged (every edit), change (an edit ended with
// another text), submit, editstart, editend, editcancel, inputrejected. Variable: value (the text).
// Style: color / font-* for the text, background-color and border for the box, :focus and the class .editing for the states,
// --caret-color, --selection-color, --placeholder-color, text-align, line-height, letter-spacing, white-space: pre (no wrap).
class UiElemTextInput : public UiElem
{
public:
    explicit UiElemTextInput(bool multiline);
    ~UiElemTextInput() override;   // a field that is destroyed while editing gives the keyboard back, silently

    // ── text (UTF-8; the field stores code points)
    const std::string& Text() const { return m_utf8; }
    void SetText(const std::string& text, bool notify = false);   // notify: raises input and valuechanged like an edit
    void Clear() { SetText(""); }
    size_t Length() const { return m_text.size(); }
    int LineCount() const;

    // ── caret and selection, in code point indices
    size_t Caret() const { return m_caret; }
    size_t SelectionStart() const { return m_caret < m_anchor ? m_caret : m_anchor; }
    size_t SelectionEnd() const { return m_caret < m_anchor ? m_anchor : m_caret; }
    bool HasSelection() const { return m_caret != m_anchor; }
    std::string SelectedText() const;
    void SetSelection(size_t anchor, size_t caret);
    void SelectAll() { SetSelection(0, m_text.size()); }

    // ── editing
    bool IsEditing() const { return m_editing; }
    bool IsMultiline() const { return m_multiline; }
    bool IsReadOnly() const { return m_readOnly; }
    void BeginEditing(bool selectAll = false);
    void EndEditing(bool commit);
    void InsertText(const std::string& utf8);   // as if typed or pasted at the selection (filters and max-length apply)
    bool Undo();
    bool Redo();
    // Return false to refuse a typed or pasted character (after the built-in numeric / allowed-chars rules).
    std::function<bool(uint32_t codepoint)> CharFilter;

    // ── UiElem
    bool ApplyAttribute(const std::string& name, const std::string& value) override;
    void OnBuilt() override;
    void Tick() override;
    bool IsFocusable() const override { return true; }
    bool HasContent() const override { return true; }
    glm::vec2 MeasureContent(float availableWidth, float availableHeight) override;
    glm::vec2 VirtualContentSize() const override;
    void PaintContent(UiPainter& painter, const UiRect& content) override;
    void OnActivate() override {}
    void OnFocused() override;
    void OnUnfocused() override;
    void OnNavConfirm() override;
    bool OnNav(UiNavDir dir) override;
    bool OnFocusedCancel() override;
    bool HoldsFocus() const override { return m_editing; }
    void OnPointerDown(glm::vec2 local) override;
    void OnPointerMove(glm::vec2 local) override;
    void OnPointerUp(glm::vec2 local) override;
    bool CaptureValue(double&, std::string& text) const override { text = m_utf8; return true; }
    void RestoreValue(double, const std::string& text) override { SetText(text); }

private:
    enum class EditKind { None, Typing, DeleteBack, DeleteForward, Other };
    struct Snapshot { std::vector<uint32_t> Text; size_t Caret = 0, Anchor = 0; };

    // text and layout
    bool Wraps() const;
    UiTextParams MakeParams(const std::vector<uint32_t>& text, float boxWidth, bool carets) const;
    const UiTextLayoutResult& BuildLayout(float boxWidth) const;
    const UiTextLayoutResult& BuildPlaceholder(float boxWidth) const;
    std::vector<uint32_t> DisplayText() const;
    float LineHeightOf(const UiTextLayoutResult& layout) const;
    glm::vec2 TextOrigin(const UiRect& content, const UiTextLayoutResult& layout) const;   // where the text's top-left is, element-local
    void SyncUtf8();
    void PublishValue();
    std::vector<uint32_t> Sanitize(const std::string& utf8) const;

    // caret geometry
    int LineOf(size_t index) const;
    float CaretX(size_t index) const;
    size_t IndexOnLine(int line, float x) const;
    size_t HitIndex(glm::vec2 local) const;
    size_t LineFirst(int line) const;
    size_t LineLast(int line) const;   // the furthest caret index that is still drawn on the line
    size_t PrevWord(size_t pos) const;
    size_t NextWord(size_t pos) const;
    void SelectWordAt(size_t pos);
    void SelectLineAt(size_t pos);

    // editing
    bool Accept(uint32_t c, const std::vector<uint32_t>& pending) const;
    bool InsertCodepoints(const std::vector<uint32_t>& cps, bool typed);
    void ReplaceRange(size_t begin, size_t end, const std::vector<uint32_t>& insert, EditKind kind);
    void DeleteBy(int direction, bool word);
    void MoveCaretTo(size_t pos, bool extend, bool keepDesiredX = false);
    void MoveHorizontal(int direction, bool word, bool extend);
    void MoveVertical(int lines, bool extend);
    void MoveToLineEdge(bool end, bool wholeText, bool extend);
    void Copy();
    void Cut();
    void Paste();
    void HandleKey(int scancode, bool ctrl, bool shift);
    void Submit();
    void TextChanged(bool fromUser);
    void PushUndo(EditKind kind);
    void ResetBlink();
    void FollowCaret();
    void Queue(const char* event);

    std::vector<uint32_t> m_text;
    std::string m_utf8, m_published;
    size_t m_caret = 0, m_anchor = 0;
    float m_desiredX = -1.f;
    bool m_multiline = false;
    bool m_editing = false, m_readOnly = false, m_password = false, m_numeric = false, m_integer = false;
    bool m_selectOnFocus = false, m_blurOnSubmit = false, m_wrap = true;
    size_t m_maxLength = 0;
    int m_minRows = 1, m_maxRows = 0;
    std::string m_placeholder;
    std::set<uint32_t> m_allowed;

    std::vector<uint32_t> m_textAtEditStart;
    std::vector<Snapshot> m_undo, m_redo;
    EditKind m_lastEdit = EditKind::None;
    size_t m_lastEditCaret = 0;

    // blink and scrolling
    uint64_t m_blinkStart = 0;
    bool m_caretShown = false;
    bool m_followCaret = false;

    // pointer
    bool m_selecting = false, m_barPress = false;
    int m_clicks = 0;
    uint64_t m_lastDownTicks = 0;
    glm::vec2 m_lastDownPos = glm::vec2(0.f);

    // layout caches (the layout is a pure function of its parameters)
    mutable UiTextParams m_lastParams, m_lastPlaceholderParams;
    mutable UiTextLayoutResult m_layout, m_placeholderLayout;
    mutable bool m_haveLayout = false, m_havePlaceholder = false;
};

class UiElemTextEntry : public UiElemTextInput
{
public:
    UiElemTextEntry() : UiElemTextInput(false) { TagName = "TextEntry"; }
    static std::shared_ptr<UiElem> Create() { return std::make_shared<UiElemTextEntry>(); }
};

class UiElemMultilineTextEntry : public UiElemTextInput
{
public:
    UiElemMultilineTextEntry() : UiElemTextInput(true) { TagName = "MultilineTextEntry"; }
    static std::shared_ptr<UiElem> Create() { return std::make_shared<UiElemMultilineTextEntry>(); }
};
