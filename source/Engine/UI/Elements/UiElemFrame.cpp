#include "UiElemFrame.h"

#include <UI/Markup/UiBuilder.h>
#include <UI/Markup/UiMarkupLog.h>
#include <UI/Menus/UiActions.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>

// ── Frame

void UiElemFrame::ClearContent()
{
    if (m_content == nullptr)
        return;
    std::vector<std::shared_ptr<UiElement>> remove;
    for (auto& c : children)
        if (UiElem::IsElem(c.get()) && !static_cast<UiElem&>(*c).InternalOverlay)
            remove.push_back(c);
    for (auto& c : remove)
        RemoveChild(c);
    m_content = nullptr;
}

void UiElemFrame::Rebuild()
{
    ClearContent();
    std::shared_ptr<UiElem> built;
    if (!m_src.empty())
    {
        built = UiElem::LoadNested(m_src);
        if (built)
        {
            AddChild(built);
            // a change of the nested file reloads the root that holds this frame
            std::vector<std::string>& deps = Root()->m_dependencies;
            for (const std::string& d : built->m_dependencies)
                if (std::find(deps.begin(), deps.end(), d) == deps.end())
                    deps.push_back(d);
        }
    }
    else if (!m_snippet.empty())
        built = UiBuilder::BuildSnippet(*this, *this, m_snippet);
    m_content = built.get();
}

bool UiElemFrame::ApplyAttribute(const std::string& name, const std::string& value)
{
    if (name == "src")
    {
        const std::string path = value.empty() ? value : UiActions::NormalizeLayoutPath(value);
        if (path != m_src)
        {
            m_src = path;
            Rebuild();
        }
        return true;
    }
    if (name == "snippet")
    {
        if (value != m_snippet)
        {
            m_snippet = value;
            if (m_src.empty())
                Rebuild();
        }
        return true;
    }
    return false;
}

// ── ListView

bool UiElemListView::ApplyAttribute(const std::string& name, const std::string& value)
{
    if (name == "snippet") { m_snippet = value; m_rebindAll = true; return true; }
    if (name == "count") { m_count = std::max(0, std::atoi(value.c_str())); return true; }
    if (name == "layout") { m_grid = value == "grid"; m_rebindAll = true; return true; }
    if (name == "item-height") { m_itemHeight = std::max(0.f, static_cast<float>(std::atof(value.c_str()))); m_rebindAll = true; return true; }
    if (name == "columns") { m_columns = std::max(1, std::atoi(value.c_str())); m_rebindAll = true; return true; }
    return false;
}

int UiElemListView::RowCount() const
{
    int n = 0;
    for (const Row& r : m_rows)
        if (r.Index >= 0)
            ++n;
    return n;
}

glm::vec2 UiElemListView::VirtualContentSize() const
{
    const int columns = m_grid ? m_columns : 1;
    const int lines = (m_count + columns - 1) / columns;
    return glm::vec2(0.f, static_cast<float>(lines) * m_itemHeight);
}

std::shared_ptr<UiElem> UiElemListView::MakeRow()
{
    return UiBuilder::BuildSnippet(*this, *this, m_snippet);
}

void UiElemListView::PlaceRow(Row& row, int index)
{
    const int columns = m_grid ? m_columns : 1;
    const int line = index / columns, column = index % columns;
    UiComputedStyle& s = row.Elem->Style();
    UiLength across, high, x, y;
    across.Kind = UiLenKind::Percent;
    across.Percent = 100.f / static_cast<float>(columns);
    high.Px = m_itemHeight;
    x.Kind = UiLenKind::Percent;
    x.Percent = 100.f * static_cast<float>(column) / static_cast<float>(columns);
    y.Px = static_cast<float>(line) * m_itemHeight;
    s.Width = across;
    s.Height = high;
    s.X = x;
    s.Y = y;
    row.Index = index;
    row.Elem->SetVar("index", static_cast<double>(index));
    if (OnBindRow)
        OnBindRow(index, *row.Elem);
}

void UiElemListView::Tick()
{
    if (m_snippet.empty() || m_itemHeight <= 0.f)
        return;
    if (!m_warned && BorderRect.H > 0.f && !IsScrollContainer())
    {
        m_warned = true;
        UiMarkupLog::Warning(ScopeFile, SrcLine, SrcCol, "ListView needs overflow: scroll");
    }

    const int columns = m_grid ? m_columns : 1;
    const float viewport = ScrollViewport.y > 0.f ? ScrollViewport.y : 1080.f;
    const int firstLine = std::max(0, static_cast<int>(std::floor(ScrollOffset.y / m_itemHeight)));
    const int visibleLines = static_cast<int>(std::ceil(viewport / m_itemHeight)) + 1;
    const int first = firstLine * columns;
    const int last = std::min(m_count, (firstLine + visibleLines) * columns + 2);   // visible rows plus 2

    // free the rows that left the window, then bind the missing indexes to freed or new rows
    std::vector<bool> have(static_cast<size_t>(std::max(0, last - first)), false);
    std::vector<size_t> pool;   // indexes into m_rows (the vector may grow below)
    for (size_t i = 0; i < m_rows.size(); ++i)
    {
        Row& r = m_rows[i];
        if (r.Index < first || r.Index >= last)
        {
            r.Index = -1;
            pool.push_back(i);
        }
        else
            have[static_cast<size_t>(r.Index - first)] = true;
    }
    if (m_rebindAll)
    {
        m_rebindAll = false;
        for (Row& r : m_rows)
            if (r.Index >= 0)
                PlaceRow(r, r.Index);
    }
    for (int i = first; i < last; ++i)
    {
        if (have[static_cast<size_t>(i - first)])
            continue;
        if (!pool.empty())
        {
            const size_t slot = pool.back();
            pool.pop_back();
            PlaceRow(m_rows[slot], i);
            continue;
        }
        Row row;
        row.Elem = MakeRow();
        if (!row.Elem)
            return;
        m_rows.push_back(row);
        PlaceRow(m_rows.back(), i);
    }
    // surplus unused rows are removed so the tree stays at visible + 2 rows
    for (size_t slot : pool)
    {
        RemoveChild(m_rows[slot].Elem);
        m_rows[slot].Elem.reset();
    }
    m_rows.erase(std::remove_if(m_rows.begin(), m_rows.end(), [](const Row& r) { return !r.Elem; }), m_rows.end());
}
