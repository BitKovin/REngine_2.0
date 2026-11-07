#include "UiRectAllocator.h"

#include <algorithm>

void UiRectAllocator::Init(int width, int height)
{
    m_width = width;
    m_height = height;
    Reset();
}

void UiRectAllocator::Reset()
{
    m_free.clear();
    if (m_width > 0 && m_height > 0)
        m_free.push_back({ 0, 0, m_width, m_height });
}

bool UiRectAllocator::Allocate(int width, int height, UiAtlasRect& out)
{
    if (width <= 0 || height <= 0)
        return false;
    // Best fit by the smaller leftover side keeps big rectangles whole.
    int best = -1;
    int bestScore = 0;
    for (size_t i = 0; i < m_free.size(); ++i)
    {
        const UiAtlasRect& fr = m_free[i];
        if (width > fr.W || height > fr.H)
            continue;
        const int score = std::min(fr.W - width, fr.H - height);
        if (best < 0 || score < bestScore)
        {
            best = static_cast<int>(i);
            bestScore = score;
        }
    }
    if (best < 0)
        return false;

    const UiAtlasRect fr = m_free[best];   // by value: the push_backs below can reallocate m_free
    out = { fr.X, fr.Y, width, height };
    m_free[best] = m_free.back();
    m_free.pop_back();
    if (fr.W > width)
        m_free.push_back({ fr.X + width, fr.Y, fr.W - width, fr.H });
    if (fr.H > height)
        m_free.push_back({ fr.X, fr.Y + height, width, fr.H - height });
    return true;
}

void UiRectAllocator::Free(const UiAtlasRect& rect)
{
    if (!rect.Valid())
        return;
    m_free.push_back(rect);
    MergeNeighbours();
}

// Two free rectangles that share a whole edge become one.
void UiRectAllocator::MergeNeighbours()
{
    bool merged = true;
    while (merged)
    {
        merged = false;
        for (size_t i = 0; i < m_free.size() && !merged; ++i)
            for (size_t j = i + 1; j < m_free.size() && !merged; ++j)
            {
                UiAtlasRect& a = m_free[i];
                const UiAtlasRect& b = m_free[j];
                if (a.Y == b.Y && a.H == b.H && (a.X + a.W == b.X || b.X + b.W == a.X))
                {
                    a.X = std::min(a.X, b.X);
                    a.W += b.W;
                    merged = true;
                }
                else if (a.X == b.X && a.W == b.W && (a.Y + a.H == b.Y || b.Y + b.H == a.Y))
                {
                    a.Y = std::min(a.Y, b.Y);
                    a.H += b.H;
                    merged = true;
                }
                if (merged)
                {
                    m_free[j] = m_free.back();
                    m_free.pop_back();
                }
            }
    }
}
