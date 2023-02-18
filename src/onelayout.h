#ifndef ONELAYOUT_H
#define ONELAYOUT_H

#include <vector>
#include <tuple>

class OneLayout
{
public:
    OneLayout(int pos, int margin);
    void add(int size);
    int getPos() const;
    int getSize() const;
    int getCellP(int cell) const;
    int getCellS(int cell, int cells) const;
    std::tuple<int, int> getCellPS(int cell, int cells) const;
    std::tuple<int, int> getCellPV(int cell, int value) const;

private:
    int m_pos;
    int m_margin;
    std::vector<int> m_cellSizes;
};

#endif // ONELAYOUT_H
