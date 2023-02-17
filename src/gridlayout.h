#ifndef GRIDLAYOUT_H
#define GRIDLAYOUT_H

#include <vector>
#include <tuple>

class GridLayout
{
public:
    GridLayout(int boxX, int boxY, int horizontalMargin, int verticalMargin, int horizontalSpacing, int verticalSpacing);
    void addRow(int height);
    void addColumn(int width);
    int getBoxX() const;
    int getBoxY() const;
    int getBoxWidth() const;
    int getBoxHeight() const;
    int getCellX(int column) const;
    int getCellY(int row) const;
    int getCellWidth(int column, int columns) const;
    int getCellHeight(int row, int rows) const;
    std::tuple<int, int> getCellXW(int column, int columns) const;
    std::tuple<int, int> getCellYH(int column, int columns) const;

private:
    int m_boxX;
    int m_boxY;
    int m_horizontalMargin;
    int m_verticalMargin;
    int m_horizontalSpacing;
    int m_verticalSpacing;
    std::vector<int> m_columnSizes;
    std::vector<int> m_rowSizes;
};

#endif // GRIDLAYOUT_H
