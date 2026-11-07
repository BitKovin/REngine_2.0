#pragma once
#include <UI/Elements/UiElem.h>

#include <functional>
#include <map>
#include <vector>

// Builds another layout file (src) or a snippet of the containing file (snippet) as a nested instance inside itself.
class UiElemFrame : public UiElem
{
public:
    UiElemFrame() { TagName = "Frame"; }
    static std::shared_ptr<UiElem> Create() { return std::make_shared<UiElemFrame>(); }
    bool ApplyAttribute(const std::string& name, const std::string& value) override;
    UiElem* Content() const { return m_content; }
    const std::string& Src() const { return m_src; }

private:
    void Rebuild();
    void ClearContent();
    std::string m_src, m_snippet;
    UiElem* m_content = nullptr;
};

// A virtualised list: only the visible rows (plus 2) are built from a snippet and recycled while scrolling.
// Rows are placed by C++ on the item height; the element needs overflow: scroll.
class UiElemListView : public UiElem
{
public:
    UiElemListView() { TagName = "ListView"; }
    static std::shared_ptr<UiElem> Create() { return std::make_shared<UiElemListView>(); }
    bool ApplyAttribute(const std::string& name, const std::string& value) override;
    void Tick() override;
    glm::vec2 VirtualContentSize() const override;

    // Called after a row is (re)bound to an item; rows also get the variable "index".
    std::function<void(int index, UiElem& row)> OnBindRow;
    int RowCount() const;                          // built rows (tests)
    int Count() const { return m_count; }
    void Rebind() { m_rebindAll = true; }          // the data changed: every built row is bound again

private:
    struct Row { std::shared_ptr<UiElem> Elem; int Index = -1; };
    void PlaceRow(Row& row, int index);
    std::shared_ptr<UiElem> MakeRow();
    std::string m_snippet;
    int m_count = 0, m_columns = 1;
    float m_itemHeight = 0.f;
    bool m_grid = false;
    bool m_rebindAll = false, m_warned = false;
    std::vector<Row> m_rows;
};
