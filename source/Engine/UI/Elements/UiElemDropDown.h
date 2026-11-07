#pragma once
#include <UI/Elements/UiElemButton.h>
#include <UI/Elements/UiElemValue.h>

class UiElemLabel;
class UiElemDropArrow;
class UiElemDropList;

// A closed header (the element itself: a Button with a Label.dropdown-text and a chevron .dropdown-arrow) and a list that opens as
// its own root above everything else, so no scroll container or clip of the page can cut it. Rows are TextButtons of class
// .dropdown-item inside a .dropdown-list that scrolls when it is longer than max-visible rows. Style them from the page's CSS;
// the engine's default sheet gives them a plain look.
class UiElemDropDown : public UiElemButton
{
public:
    UiElemDropDown();
    ~UiElemDropDown() override;
    static std::shared_ptr<UiElem> Create() { return std::make_shared<UiElemDropDown>(); }
    bool ApplyAttribute(const std::string& name, const std::string& value) override;
    void OnBuilt() override;
    void OnActivate() override;   // opens or closes the list
    void Update() override;

    void SetOptions(const std::vector<std::string>& options);   // already localised
    int Selected() const { return m_selected; }
    void SetSelected(int index);
    bool IsOpen() const { return m_open; }
    void Open();
    void Close();

    bool CaptureValue(double& number, std::string&) const override { number = Selected(); return true; }
    void RestoreValue(double number, const std::string&) override { SetSelected(static_cast<int>(number)); }

private:
    friend class UiElemDropList;
    friend class UiElemDropItem;
    void RefreshBoundOptions();
    void RefreshLabel();
    void PositionList();
    void DestroyList();
    bool PressedOutside() const;

    std::vector<std::string> m_options;
    int m_selected = -1;
    std::string m_placeholder;
    bool m_optionsFromAttribute = false;
    int m_maxVisible = 6;

    UiElemLabel* m_text = nullptr;
    UiElemDropArrow* m_arrow = nullptr;
    std::shared_ptr<UiElem> m_list;
    std::vector<UiElem*> m_rows;     // the .dropdown-item rows of the open list
    glm::vec4 m_lastGeom = glm::vec4(-1.f);   // x, y, width, max height last given to the list
    bool m_open = false;
    int m_pick = -1;                 // a row was activated: applied by Update, never while the list is in the middle of its own update
    bool m_closeRequested = false;   // cancel pressed inside the list
    int m_focusFrames = 0;           // frames still to try focusing the selected row (the list is laid out one frame after it opens)
    UiValueBinding m_binding{ *this };
};
