#pragma once
#include <vector>

struct UiAtlasRect
{
    int X = 0, Y = 0, W = 0, H = 0;
    bool Valid() const { return W > 0 && H > 0; }
};

// Free-rectangle packer shared by the billboard atlas and the layer atlas (guillotine split, neighbours merge on Free).
class UiRectAllocator
{
public:
    void Init(int width, int height);
    void Reset();   // everything free again
    bool Allocate(int width, int height, UiAtlasRect& out);
    void Free(const UiAtlasRect& rect);
    int Width() const { return m_width; }
    int Height() const { return m_height; }

private:
    std::vector<UiAtlasRect> m_free;
    int m_width = 0, m_height = 0;
    void MergeNeighbours();
};
