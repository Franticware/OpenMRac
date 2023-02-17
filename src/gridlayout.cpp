#include "gridlayout.h"
//#include <cassert>
#ifdef assert
// kvuli os x
#undef assert
#endif
#define assert(x) ;

GridLayout::GridLayout(int boxX, int boxY, int horizontalMargin, int verticalMargin, int horizontalSpacing, int verticalSpacing) :
    m_boxX(boxX),
    m_boxY(boxY),
    m_horizontalMargin(horizontalMargin),
    m_verticalMargin(verticalMargin),
    m_horizontalSpacing(horizontalSpacing),
    m_verticalSpacing(verticalSpacing)
{
}

void GridLayout::addRow(int height)
{
    m_rowSizes.push_back(height);
}

void GridLayout::addColumn(int width)
{
    m_columnSizes.push_back(width);
}

int GridLayout::getBoxX() const
{
    return m_boxX;
}

int GridLayout::getBoxY() const
{
    return m_boxY;
}

int GridLayout::getBoxWidth() const
{
    assert(!m_columnSizes.empty());
    int boxWidth = 2 * m_horizontalMargin;
    //for (const int& columnSize : m_columnSizes)
    for (unsigned i = 0; i != m_columnSizes.size(); ++i)
    {
        boxWidth += m_columnSizes[i];
        boxWidth += m_horizontalSpacing;
    }
    boxWidth -= m_horizontalSpacing;
    return boxWidth;
}

int GridLayout::getBoxHeight() const
{
    assert(!m_rowSizes.empty());
    int boxHeight = 2 * m_verticalMargin;
    //for (const int& rowSize : m_rowSizes)
    for (unsigned i = 0; i != m_rowSizes.size(); ++i)
    {
        boxHeight += m_rowSizes[i];
        boxHeight += m_verticalSpacing;
    }
    boxHeight -= m_verticalSpacing;
    return boxHeight;
}

int GridLayout::getCellX(int column) const
{
    int cellX = m_boxX + m_horizontalMargin;
    for (int i = 0; i < column; ++i)
    {
        assert(i < static_cast<int>(m_columnSizes.size()));
        cellX += m_columnSizes[i];
        //if (i != 0)
        {
            cellX += m_horizontalSpacing;
        }
    }
    return cellX;
}

int GridLayout::getCellY(int row) const
{
    int cellY = m_boxY + m_verticalMargin;
    for (int i = 0; i < row; ++i)
    {
        assert(i < static_cast<int>(m_rowSizes.size()));
        cellY += m_rowSizes[i];
        //if (i != 0)
        {
            cellY += m_verticalSpacing;
        }
    }
    return cellY;
}

int GridLayout::getCellWidth(int column, int columns) const
{
    int cellWidth = 0;
    for (int i = 0; i != columns; ++i)
    {
        assert(column + i < static_cast<int>(m_columnSizes.size()));
        cellWidth += m_columnSizes[column + i];
        if (i != 0)
        {
            cellWidth += m_horizontalSpacing;
        }
    }
    return cellWidth;
}

int GridLayout::getCellHeight(int row, int rows) const
{
    int cellHeight = 0;
    for (int i = 0; i != rows; ++i)
    {
        assert(row + i < static_cast<int>(m_rowSizes.size()));
        cellHeight += m_rowSizes[row + i];
        if (i != 0)
        {
            cellHeight += m_verticalSpacing;
        }
    }
    return cellHeight;
}

std::tuple<int, int> GridLayout::getCellXW(int column, int columns) const
{
    return std::make_tuple(getCellX(column), getCellWidth(column, columns));
}

std::tuple<int, int> GridLayout::getCellYH(int row, int rows) const
{
    return std::make_tuple(getCellY(row), getCellHeight(row, rows));
}
