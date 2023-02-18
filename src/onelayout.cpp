#include "onelayout.h"
#include <cassert>
#include <numeric>

OneLayout::OneLayout(int pos, int margin) :
    m_pos(pos),
    m_margin(margin)
{
}

void OneLayout::add(int size)
{
    m_cellSizes.push_back(size);
}

int OneLayout::getPos() const
{
    return m_pos;
}

int OneLayout::getSize() const
{
    return std::accumulate(m_cellSizes.begin(), m_cellSizes.end(), 2 * m_margin);
}

int OneLayout::getCellP(int cell) const
{
    int cellP = m_pos + m_margin;
    for (int i = 0; i < cell; ++i)
    {
        assert(i < static_cast<int>(m_cellSizes.size()));
        cellP += m_cellSizes[i];
    }
    return cellP;
}

int OneLayout::getCellS(int cell, int cells) const
{
    int cellS = 0;
    for (int i = 0; i != cells; ++i)
    {
        assert(cell + i < static_cast<int>(m_cellSizes.size()));
        cellS += m_cellSizes[cell + i];
    }
    return cellS;
}

std::tuple<int, int> OneLayout::getCellPS(int cell, int cells) const
{
    return std::make_tuple(getCellP(cell), getCellS(cell, cells));
}

std::tuple<int, int> OneLayout::getCellPV(int cell, int value) const
{
    return std::make_tuple(getCellP(cell), value);
}
