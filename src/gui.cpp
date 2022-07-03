#include "gui.h"

/// GuiData ///

GuiData::GuiData()
{
    m_whiteFontSurface = 0;
    m_blackFontSurface = 0;
    m_guiComboSurface = 0;
    m_guiCornersSurface = 0;
    m_guiTickSurface = 0;
}

GuiData::~GuiData()
{
    if (m_whiteFontSurface)
    {
        SDL_FreeSurface(m_whiteFontSurface);
    }
    if (m_blackFontSurface)
    {
        SDL_FreeSurface(m_blackFontSurface);
    }
}

void GuiData::initData()
{
    m_whiteFontPixels.resize(GuiFont::WIDTH * GuiFont::HEIGHT);
    for (int i = 0; i != GuiFont::WIDTH * GuiFont::HEIGHT; ++i)
    {
        Uint32 pixel = GuiFont::DATA[i];
        pixel = pixel << 24;
        pixel |= 0x00ffffff;
        m_whiteFontPixels[i] = pixel;
    }
    m_whiteFontSurface = SDL_CreateRGBSurfaceFrom(&(m_whiteFontPixels[0]), GuiFont::WIDTH, GuiFont::HEIGHT, 32, GuiFont::WIDTH * 4, SURFACE_MASK);

    m_blackFontPixels.resize(GuiFont::WIDTH * GuiFont::HEIGHT);
    for (int i = 0; i != GuiFont::WIDTH * GuiFont::HEIGHT; ++i)
    {
        Uint32 pixel = GuiFont::DATA[i];
        pixel = pixel << 24;
        pixel |= 0x00000000;
        m_blackFontPixels[i] = pixel;
    }
    m_blackFontSurface = SDL_CreateRGBSurfaceFrom(&(m_blackFontPixels[0]), GuiFont::WIDTH, GuiFont::HEIGHT, 32, GuiFont::WIDTH * 4, SURFACE_MASK);

    m_guiComboSurface = SDL_CreateRGBSurfaceFrom(COMBO_SURFACE_DATA, COMBO_SURFACE_WIDTH, COMBO_SURFACE_HEIGHT, 32, COMBO_SURFACE_WIDTH * 4, SURFACE_MASK);
    m_guiCornersSurface = SDL_CreateRGBSurfaceFrom(CORNERS_SURFACE_DATA, CORNERS_SURFACE_WIDTH, CORNERS_SURFACE_HEIGHT, 32, CORNERS_SURFACE_WIDTH * 4, SURFACE_MASK);
    m_guiTickSurface = SDL_CreateRGBSurfaceFrom(TICK_SURFACE_DATA, TICK_SURFACE_WIDTH, TICK_SURFACE_HEIGHT, 32, TICK_SURFACE_WIDTH * 4, SURFACE_MASK);
}

/*static*/ const Uint32 GuiData::COLOR_BACKGROUND1 = 0xff2c2c2c;
/*static*/ const Uint32 GuiData::COLOR_BUTTON = 0xff000000;
/*static*/ const Uint32 GuiData::COLOR_MOUSEOVER = GuiData::COLOR_BACKGROUND1; //0xff282828;
/*static*/ const Uint32 GuiData::COLOR_CLICKED = 0xff404040;
/*static*/ const Uint32 GuiData::COLOR_EDGE = 0xff686868;
///*static*/ const Uint32 GuiData::COLOR_FRAME = 0xff5c5c5c;
/*static*/ const Uint32 GuiData::COLOR_FRAME = 0xffaaaaaa;
/*static*/ const Uint32 GuiData::COLOR_SELECTED = 0xffffffff;

/*static*/ const int GuiData::COMBO_SURFACE_WIDTH = 8;
/*static*/ const int GuiData::COMBO_SURFACE_HEIGHT = 8;
/*static*/ Uint32 GuiData::COMBO_SURFACE_DATA[] = {
0x00ffffff,0x00ffffff,0x4dffffff,0xa6ffffff,0xa6ffffff,0x4dffffff,0x00ffffff,0x00ffffff,
0x00ffffff,0x4dffffff,0xa6ffffff,0xffffffff,0xffffffff,0xa6ffffff,0x4dffffff,0x00ffffff,
0x4dffffff,0xa6ffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xa6ffffff,0x4dffffff,
0xa6ffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xa6ffffff,
0xa6ffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xa6ffffff,
0x4dffffff,0xa6ffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xa6ffffff,0x4dffffff,
0x00ffffff,0x4dffffff,0xa6ffffff,0xffffffff,0xffffffff,0xa6ffffff,0x4dffffff,0x00ffffff,
0x00ffffff,0x00ffffff,0x4dffffff,0xa6ffffff,0xa6ffffff,0x4dffffff,0x00ffffff,0x00ffffff,
};

/*static*/ const int GuiData::CORNERS_SURFACE_WIDTH = 6;
/*static*/ const int GuiData::CORNERS_SURFACE_HEIGHT = 6;
/*static*/ Uint32 GuiData::CORNERS_SURFACE_DATA[] = {
0xff2c2c2c,0xff494949,0xff545454,0xff545454,0xff494949,0xff2c2c2c,0xff494949,0xff414141,
0x00000000,0x00000000,0xff414141,0xff494949,0xff545454,0x00000000,0x00000000,0x00000000,
0x00000000,0xff545454,0xff545454,0x00000000,0x00000000,0x00000000,0x00000000,0xff545454,
0xff494949,0xff414141,0x00000000,0x00000000,0xff414141,0xff494949,0xff2c2c2c,0xff494949,
0xff545454,0xff545454,0xff494949,0xff2c2c2c,};

/*static*/ const int GuiData::TICK_SURFACE_WIDTH = 10;
/*static*/ const int GuiData::TICK_SURFACE_HEIGHT = 8;
/*static*/ Uint32 GuiData::TICK_SURFACE_DATA[] = {
0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x4dffffff,
0xa6ffffff,0x4dffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,
0x4dffffff,0xa6ffffff,0xffffffff,0xa6ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,
0x00ffffff,0x4dffffff,0xa6ffffff,0xffffffff,0xa6ffffff,0x4dffffff,0x4dffffff,0xa6ffffff,
0x4dffffff,0x00ffffff,0x4dffffff,0xa6ffffff,0xffffffff,0xa6ffffff,0x4dffffff,0x00ffffff,
0xa6ffffff,0xffffffff,0xa6ffffff,0x4dffffff,0xa6ffffff,0xffffffff,0xa6ffffff,0x4dffffff,
0x00ffffff,0x00ffffff,0x4dffffff,0xa6ffffff,0xffffffff,0xa6ffffff,0xffffffff,0xa6ffffff,
0x4dffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x4dffffff,0xa6ffffff,0xffffffff,
0xa6ffffff,0x4dffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,
0x4dffffff,0xa6ffffff,0x4dffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,
};

/// GuiItem ///

int GuiItem::getComboFields() const
{
    int fields = visibleItemCount;
    if (fields > static_cast<int>(items.size()))
    {
        fields = items.size();
    }
    return fields;
}

/// GuiDialog ///

GuiDialog::GuiDialog()
{
    m_width = 0;
    m_height = 0;
    m_caption = std::string();

    m_guiData.initData();

    m_backSurface = 0;

    m_whiteFont = true;

    m_expandedItem = -1;

    m_state = 0;

    m_stateItem = -1;

    m_paint = false;

    m_eventLoopDone = false;

}

GuiDialog::~GuiDialog()
{
    if (m_backSurface)
    {
        SDL_FreeSurface(m_backSurface);
    }
}

void GuiDialog::init(int width, int height, const char* caption)
{
    m_width = width;
    m_height = height;
    m_caption = caption;
}

void GuiDialog::drawText(const char* text, int x, int y, int width)
{
    size_t textLen = strlen(text);

    int left = x;
    int right = x + width;

    for (size_t i = 0; i != textLen && left < right; ++i)
    {
        char c = text[i];
        unsigned char u = c;
        unsigned char space = ' ';
        unsigned char tilde = '~';
        if (u < space || u > tilde)
        {
            u = GuiFont::CHARACTER_COUNT - 1;
        }
        else
        {
            u -= space;
        }
        int w = right - left;
        if (w > 0)
        {
            SDL_Rect srcRect;
            srcRect.x = GuiFont::CHARACTER_POSITIONS[u];
            srcRect.y = 0;
            srcRect.w = std::min(GuiFont::CHARACTER_WIDTHS[u], w);
            srcRect.h = GuiFont::HEIGHT;
            SDL_Rect dstRect;
            dstRect.x = left;
            dstRect.y = y + 1;
            dstRect.w = w;
            dstRect.h = GuiFont::HEIGHT;
            SDL_BlitSurface(m_whiteFont ? m_guiData.m_whiteFontSurface : m_guiData.m_blackFontSurface, &srcRect, m_backSurface, &dstRect);
        }
        left += GuiFont::CHARACTER_WIDTHS[u];
    }
}

void GuiDialog::drawCenteredText(const char* text, int x, int y)
{
    size_t textLen = strlen(text);

    int textWidth = 0;

    for (size_t i = 0; i != textLen; ++i)
    {
        char c = text[i];
        unsigned char u = c;
        unsigned char space = ' ';
        unsigned char tilde = '~';
        if (u < space || u > tilde)
        {
            u = GuiFont::CHARACTER_COUNT - 1;
        }
        else
        {
            u -= space;
        }
        textWidth += GuiFont::CHARACTER_WIDTHS[u];
    }

    int left = x - textWidth / 2;

    for (size_t i = 0; i != textLen; ++i)
    {
        char c = text[i];
        unsigned char u = c;
        unsigned char space = ' ';
        unsigned char tilde = '~';
        if (u < space || u > tilde)
        {
            u = GuiFont::CHARACTER_COUNT - 1;
        }
        else
        {
            u -= space;
        }
        SDL_Rect srcRect;
        srcRect.x = GuiFont::CHARACTER_POSITIONS[u];
        srcRect.y = 0;
        srcRect.w = GuiFont::CHARACTER_WIDTHS[u];
        srcRect.h = GuiFont::HEIGHT;
        SDL_Rect dstRect;
        dstRect.x = left;
        dstRect.y = y + 1;
        dstRect.w = GuiFont::CHARACTER_WIDTHS[u];
        dstRect.h = GuiFont::HEIGHT;
        SDL_BlitSurface(m_guiData.m_whiteFontSurface, &srcRect, m_backSurface, &dstRect);
        left += GuiFont::CHARACTER_WIDTHS[u];
    }
}

void GuiDialog::paint()
{
    SDL_FillRect(m_backSurface, 0, GuiData::COLOR_BACKGROUND1);


    for (int i = 0; i != static_cast<int>(m_items.size()); ++i)
    {
        GuiItem& item = m_items[i];
        if (item.type == GuiItem::FRAME)
        {
            SDL_Rect rectQuadOuter;
            rectQuadOuter.x = item.x;
            rectQuadOuter.y = item.y;
            rectQuadOuter.w = item.width;
            rectQuadOuter.h = item.height;
            SDL_FillRect(m_backSurface, &rectQuadOuter, GuiData::COLOR_FRAME);

            SDL_Rect rectQuadInner;
            rectQuadInner.x = item.x+1;
            rectQuadInner.y = item.y+1;
            rectQuadInner.w = item.width-2;
            rectQuadInner.h = item.height-2;
            SDL_FillRect(m_backSurface, &rectQuadInner, GuiData::COLOR_BACKGROUND1);
        }
        else if (item.type == GuiItem::COMBO)
        {
            SDL_Rect rectA;
            rectA.x = item.x;
            rectA.y = item.y;
            rectA.w = item.width;
            rectA.h = item.height;
            SDL_FillRect(m_backSurface, &rectA, GuiData::COLOR_EDGE);
            SDL_Rect rectB;
            rectB.x = item.x+1;
            rectB.y = item.y+1;
            rectB.w = item.width-2;
            rectB.h = item.height-2;
            SDL_FillRect(m_backSurface, &rectB, (m_state != 0 && m_stateItem == i && m_expandedItem == -1) ? stateToColor(m_state) : GuiData::COLOR_BUTTON);

            SDL_Rect rectSrc;
            SDL_Rect rectDst;

            // top left
            rectSrc.x = 0;
            rectSrc.y = 0;
            rectSrc.w = 3;
            rectSrc.h = 3;
            rectDst.x = item.x;
            rectDst.y = item.y;
            rectDst.w = 3;
            rectDst.h = 3;
            SDL_BlitSurface(m_guiData.m_guiCornersSurface, &rectSrc, m_backSurface, &rectDst);

            // top right
            rectSrc.x = 3;
            rectSrc.y = 0;
            rectSrc.w = 3;
            rectSrc.h = 3;
            rectDst.x = item.x+item.width-3;
            rectDst.y = item.y;
            rectDst.w = 3;
            rectDst.h = 3;
            SDL_BlitSurface(m_guiData.m_guiCornersSurface, &rectSrc, m_backSurface, &rectDst);

            //if (i != m_expandedItem)
            {
                // bottom left
                rectSrc.x = 0;
                rectSrc.y = 3;
                rectSrc.w = 3;
                rectSrc.h = 3;
                rectDst.x = item.x;
                rectDst.y = item.y+item.height-3;
                rectDst.w = 3;
                rectDst.h = 3;
                SDL_BlitSurface(m_guiData.m_guiCornersSurface, &rectSrc, m_backSurface, &rectDst);

                // bottom left
                rectSrc.x = 3;
                rectSrc.y = 3;
                rectSrc.w = 3;
                rectSrc.h = 3;
                rectDst.x = item.x+item.width-3;
                rectDst.y = item.y+item.height-3;
                rectDst.w = 3;
                rectDst.h = 3;
                SDL_BlitSurface(m_guiData.m_guiCornersSurface, &rectSrc, m_backSurface, &rectDst);
            }

            if (item.currentItem >= 0 && item.currentItem < static_cast<int>(item.items.size()))
            {
                drawText(item.items[item.currentItem].c_str(), item.x + 3, item.y + (item.height - 18) / 2, item.width - 20);
            }

            rectSrc.x = 0;
            rectSrc.y = 4;
            rectSrc.w = 8;
            rectSrc.h = 4;
            rectDst.x = item.x+item.width - 8 - 4;
            rectDst.y = item.y + (item.height - 4) / 2;
            rectDst.w = 8;
            rectDst.h = 4;
            SDL_BlitSurface(m_guiData.m_guiComboSurface, &rectSrc, m_backSurface, &rectDst);
        }
        else if (item.type == GuiItem::BUTTON)
        {
            SDL_Rect rectQuadOuter;
            rectQuadOuter.x = item.x;
            rectQuadOuter.y = item.y;
            rectQuadOuter.w = item.width;
            rectQuadOuter.h = item.height;
            SDL_FillRect(m_backSurface, &rectQuadOuter, GuiData::COLOR_EDGE);

            SDL_Rect rectQuadInner;
            rectQuadInner.x = item.x+1;
            rectQuadInner.y = item.y+1;
            rectQuadInner.w = item.width-2;
            rectQuadInner.h = item.height-2;
            SDL_FillRect(m_backSurface, &rectQuadInner, item.pressedByKey ? GuiData::COLOR_CLICKED : ((m_state != 0 && m_stateItem == i && m_expandedItem == -1) ? stateToColor(m_state) : GuiData::COLOR_BUTTON));

            SDL_Rect rectSrc;
            SDL_Rect rectDst;

            // top left
            rectSrc.x = 0;
            rectSrc.y = 0;
            rectSrc.w = 3;
            rectSrc.h = 3;
            rectDst.x = item.x;
            rectDst.y = item.y;
            rectDst.w = 3;
            rectDst.h = 3;
            SDL_BlitSurface(m_guiData.m_guiCornersSurface, &rectSrc, m_backSurface, &rectDst);

            // top right
            rectSrc.x = 3;
            rectSrc.y = 0;
            rectSrc.w = 3;
            rectSrc.h = 3;
            rectDst.x = item.x+item.width-3;
            rectDst.y = item.y;
            rectDst.w = 3;
            rectDst.h = 3;
            SDL_BlitSurface(m_guiData.m_guiCornersSurface, &rectSrc, m_backSurface, &rectDst);

            // bottom left
            rectSrc.x = 0;
            rectSrc.y = 3;
            rectSrc.w = 3;
            rectSrc.h = 3;
            rectDst.x = item.x;
            rectDst.y = item.y+item.height-3;
            rectDst.w = 3;
            rectDst.h = 3;
            SDL_BlitSurface(m_guiData.m_guiCornersSurface, &rectSrc, m_backSurface, &rectDst);

            // bottom left
            rectSrc.x = 3;
            rectSrc.y = 3;
            rectSrc.w = 3;
            rectSrc.h = 3;
            rectDst.x = item.x+item.width-3;
            rectDst.y = item.y+item.height-3;
            rectDst.w = 3;
            rectDst.h = 3;
            SDL_BlitSurface(m_guiData.m_guiCornersSurface, &rectSrc, m_backSurface, &rectDst);

            drawCenteredText(item.text.c_str(), item.x + item.width / 2, item.y + (item.height - 18) / 2);

        }
        else if (item.type == GuiItem::LABEL)
        {
            drawText(item.text.c_str(), item.x, item.y + (item.height - 18) / 2, item.width);
        }
        else if (item.type == GuiItem::CHECK)
        {
            SDL_Rect rectDst;
            rectDst.x = item.x;
            rectDst.y = item.y + (item.height - 14) / 2;
            rectDst.w = 14;
            rectDst.h = 14;
            SDL_FillRect(m_backSurface, &rectDst, GuiData::COLOR_EDGE);

            rectDst.x = item.x + 1;
            rectDst.y = item.y + (item.height - 14) / 2 + 1;
            rectDst.w = 12;
            rectDst.h = 12;
            SDL_FillRect(m_backSurface, &rectDst, (m_state != 0 && m_stateItem == i && m_expandedItem == -1) ? stateToColor(m_state) : GuiData::COLOR_BUTTON);

            if (item.checked)
            {
                rectDst.x = item.x + 2;
                rectDst.y = item.y + (item.height - 14) / 2 + 3;
                rectDst.w = m_guiData.m_guiTickSurface->w;
                rectDst.h = m_guiData.m_guiTickSurface->h;
                SDL_BlitSurface(m_guiData.m_guiTickSurface, 0, m_backSurface, &rectDst);
            }

            drawText(item.text.c_str(), item.x + 18, item.y + (item.height - 18) / 2, item.width - 18);
        }
    }

    {
        int i = m_expandedItem;
        if (i >= 0 && i < static_cast<int>(m_items.size()))
        {
            GuiItem& item = m_items[i];

            if (item.type == GuiItem::COMBO)
            {
                int fields = item.getComboFields();

                SDL_Rect rectSrc;
                SDL_Rect rectDst;

                rectDst.x = item.x;
                rectDst.y = item.y + item.height - 1;
                rectDst.w = item.width;
                rectDst.h = item.itemHeight * fields + 2;
                SDL_FillRect(m_backSurface, &rectDst, GuiData::COLOR_EDGE);

                rectDst.x = item.x + 1;
                rectDst.y = item.y + 1 + item.height - 1;
                rectDst.w = item.width - 2;
                rectDst.h = item.itemHeight * fields + 2 - 2;
                SDL_FillRect(m_backSurface, &rectDst, GuiData::COLOR_BUTTON);

                for (int j = 0; j != fields; ++j)
                {
                    bool currentSelected = j + m_expandedComboScroll == item.currentItem;
                    if (currentSelected)
                    {
                        m_whiteFont = false;
                        rectDst.x = item.x + 1;
                        rectDst.y = item.y + item.height + item.itemHeight * j;
                        rectDst.w = item.width - 2;
                        rectDst.h = item.itemHeight;
                        SDL_FillRect(m_backSurface, &rectDst, GuiData::COLOR_SELECTED);
                    }
                    else if (j + m_expandedComboScroll == m_stateExpandedComboItem)
                    {
                        rectDst.x = item.x + 1;
                        rectDst.y = item.y + item.height + item.itemHeight * j;
                        rectDst.w = item.width - 2;
                        rectDst.h = item.itemHeight;
                        SDL_FillRect(m_backSurface, &rectDst, stateToColor(m_state));
                    }
                    drawText(item.items[j + m_expandedComboScroll].c_str(), item.x + 3, item.y + 1 + item.height + (item.itemHeight - 18) / 2 + item.itemHeight * j, item.width - 7);
                    if (currentSelected)
                    {
                        m_whiteFont = true;
                    }
                }

                int arrowWidth = COMBO_ARROW_WIDTH;

                if (m_expandedComboScroll != 0)
                {

                    rectDst.x = item.x + 1 + item.width - 2 - arrowWidth;
                    rectDst.y = item.y + item.height;
                    rectDst.w = arrowWidth;
                    rectDst.h = item.itemHeight;
                    SDL_FillRect(m_backSurface, &rectDst, m_stateExpandedComboItem == -2 ? stateToColor(m_state) : GuiData::COLOR_BUTTON);

                    rectSrc.x = 0;
                    rectSrc.y = 0;
                    rectSrc.w = 8;
                    rectSrc.h = 4;
                    rectDst.x = item.x+item.width - 8 - 4;
                    rectDst.y = item.y + item.height + (item.itemHeight - 4) / 2;
                    rectDst.w = 8;
                    rectDst.h = 4;
                    SDL_BlitSurface(m_guiData.m_guiComboSurface, &rectSrc, m_backSurface, &rectDst);
                }

                if (m_expandedComboScroll + fields < static_cast<int>(item.items.size()))
                {
                    rectDst.x = item.x + 1 + item.width - 2 - arrowWidth;
                    rectDst.y = item.y + item.height + item.itemHeight * (fields - 1);
                    rectDst.w = arrowWidth;
                    rectDst.h = item.itemHeight;
                    SDL_FillRect(m_backSurface, &rectDst, m_stateExpandedComboItem == -3 ? stateToColor(m_state) : GuiData::COLOR_BUTTON);

                    rectSrc.x = 0;
                    rectSrc.y = 4;
                    rectSrc.w = 8;
                    rectSrc.h = 4;
                    rectDst.x = item.x+item.width - 8 - 4;
                    rectDst.y = item.y + item.height + item.itemHeight * (fields - 1) + (item.itemHeight - 4) / 2;
                    rectDst.w = 8;
                    rectDst.h = 4;
                    SDL_BlitSurface(m_guiData.m_guiComboSurface, &rectSrc, m_backSurface, &rectDst);
                }
            }
        }
    }

    //cout << "blit" << endl;
    SDL_BlitSurface(m_backSurface, 0, m_windowSurface, 0);
    SDL_UpdateRect(m_windowSurface, 0, 0, 0, 0);
}

/*static*/ const int GuiDialog::COMBO_ARROW_WIDTH = 14;

void GuiDialog::mouseStateChanged(int x, int y, bool leftButton)
{
    int newStateItem = m_stateItem;
    int newStateExpandedComboItem = m_stateExpandedComboItem;
    int newState = leftButton ? 2 : 1;
    if (m_expandedItem == -1)
    {
        newStateExpandedComboItem = -1;
        int itemIndex = getItem(x, y);
        if (itemIndex != -1)
        {
            newStateItem = itemIndex;
        }
        else
        {
            newStateItem = -1;
        }
    }
    else
    {
        newStateItem = -1;
        newStateExpandedComboItem = getExpandedComboItem(x, y);
        if (newStateExpandedComboItem == -4)
        {
            newStateExpandedComboItem = -1;
        }
    }
    if (newState != m_state || newStateItem != m_stateItem || newStateExpandedComboItem != m_stateExpandedComboItem)
    {
        m_state = newState;
        m_stateItem = newStateItem;
        m_stateExpandedComboItem = newStateExpandedComboItem;
        m_paint = true;
    }
}

int GuiDialog::getItem(int x, int y)
{
    //assert(m_expandedItem == -1);
    for (int i = m_items.size(); i != 0; --i)
    {
        GuiItem& item = m_items[i-1];
        if (item.type == GuiItem::BUTTON || item.type == GuiItem::CHECK || item.type == GuiItem::COMBO)
        {
            if (x >= item.x && x < item.x + item.width && y >= item.y && y < item.y + item.height)
            {
                return i - 1;
            }
        }
    }
    return -1;
}

// -1 - NO, -2 - up, -3 - down, -4 - neutral
int GuiDialog::getExpandedComboItem(int x, int y)
{
    //assert(m_expandedItem != -1);

    if (m_expandedItem >= 0)
    {
        GuiItem& item = m_items[m_expandedItem];
        if (item.type == GuiItem::COMBO)
        {
            int fields = item.getComboFields();

            if (static_cast<int>(item.items.size()) > fields)
            {

                int upX = item.x + 1 + item.width - 2 - COMBO_ARROW_WIDTH;
                int upY = item.y + item.height;
                int upW = COMBO_ARROW_WIDTH;
                int upH = item.itemHeight;

                int downX = item.x + 1 + item.width - 2 - COMBO_ARROW_WIDTH;
                int downY = item.y + item.height + item.itemHeight * (fields - 1);
                int downW = COMBO_ARROW_WIDTH;
                int downH = item.itemHeight;

                if (x >= upX && x < upX + upW && y >= upY && y < upY + upH)
                {
                    if (m_expandedComboScroll != 0)
                    {
                        return -2;
                    }
                    else
                    {
                        return -4;
                    }
                }
                if (x >= downX && x < downX + downW && y >= downY && y < downY + downH)
                {
                    if (m_expandedComboScroll + fields < static_cast<int>(item.items.size()))
                    {
                        return -3;
                    }
                    else
                    {
                        return -4;
                    }
                }
            }

            for (int j = 0; j != fields; ++j)
            {
                int comboItemX = item.x + 1;
                int comboItemY = item.y + item.height + item.itemHeight * j;
                int comboItemW = item.width - 2;
                int comboItemH = item.itemHeight;

                if (x >= comboItemX && x < comboItemX + comboItemW && y >= comboItemY && y < comboItemY + comboItemH)
                {
                    return j + m_expandedComboScroll;
                }
            }

            int neutralX = item.x;
            int neutralY = item.y + item.height - 1;
            int neutralW = item.width;
            int neutralH = item.itemHeight * fields + 2;

            if (x >= neutralX && x < neutralX + neutralW && y >= neutralY && y < neutralY + neutralH)
            {
                return -4;
            }
        }
    }
    return -1;
}

void GuiDialog::execute()
{
    m_windowSurface = SDL_SetVideoMode(m_width, m_height, 0, SDL_SWSURFACE);
    SDL_WM_SetCaption(m_caption.c_str(), NULL);
    m_backSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, m_width, m_height, 32, SURFACE_MASK);

    paint();

    while (!m_eventLoopDone)
    {
        SDL_Event event;
        SDL_WaitEvent(&event);
        // check for messages
        switch (event.type)
        {
        case SDL_QUIT: // exit if the window is closed
            onQuit();
            break;
        case SDL_VIDEOEXPOSE:
        case SDL_VIDEORESIZE:
            SDL_BlitSurface(m_backSurface, 0, m_windowSurface, 0);
            SDL_UpdateRect(m_windowSurface, 0, 0, 0, 0);
            break;
        case SDL_MOUSEMOTION:
            mouseStateChanged(event.motion.x, event.motion.y, event.motion.state & SDL_BUTTON_LMASK);
            break;
        case SDL_ACTIVEEVENT:
            if (event.active.gain == 0 && event.active.state == SDL_APPMOUSEFOCUS)
            {
                mouseStateChanged(-1, -1, false);
            }
            break;
        case SDL_KEYDOWN:
            #if defined(__WIN32__)
            if (event.key.keysym.sym == SDLK_F4 && (event.key.keysym.mod & (KMOD_LALT | KMOD_RALT)))
            {
                onQuit();
            }
            #endif
            onKeyDown(event.key.keysym.sym);
            break;
        case SDL_KEYUP:
            onKeyUp(event.key.keysym.sym);
            break;
        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT)
            {
                mouseStateChanged(event.button.x, event.button.y, true);
            }
            else if (event.button.button == SDL_BUTTON_WHEELUP || event.button.button == SDL_BUTTON_WHEELDOWN)
            {
                if (m_expandedItem == -1)
                {
                    int itemIndex = getItem(event.button.x, event.button.y);
                    if (itemIndex != -1)
                    {
                        GuiItem& item = m_items[itemIndex];
                        if (item.type == GuiItem::COMBO)
                        {
                            int currentItemDelta = event.button.button == SDL_BUTTON_WHEELUP ? -1 : 1;
                            item.currentItem += currentItemDelta;
                            if (item.currentItem < 0)
                            {
                                item.currentItem = 0;
                            }
                            else if (item.currentItem >= static_cast<int>(item.items.size()))
                            {
                                item.currentItem = static_cast<int>(item.items.size()) - 1;
                            }
                            m_paint = true;
                        }
                    }
                }
                else
                {
                    GuiItem& item = m_items[m_expandedItem];
                    if (item.type == GuiItem::COMBO)
                    {
                        int expandedComboIndex = getExpandedComboItem(event.button.x, event.button.y);
                        if (expandedComboIndex != -1)
                        {
                            int fields = item.getComboFields();
                            int scrollDelta = event.button.button == SDL_BUTTON_WHEELUP ? -1 : 1;
                            m_expandedComboScroll += scrollDelta;
                            if (m_expandedComboScroll < 0)
                            {
                                m_expandedComboScroll = 0;
                            }
                            else if (m_expandedComboScroll + fields > static_cast<int>(item.items.size()))
                            {
                                m_expandedComboScroll = static_cast<int>(item.items.size()) - fields;
                            }
                        }
                    }
                    m_paint = true;
                }

                mouseStateChanged(event.button.x, event.button.y, false);
            }
            break;
        case SDL_MOUSEBUTTONUP:
            if (event.button.button == SDL_BUTTON_LEFT)
            {
                if (m_expandedItem == -1)
                {
                    int itemIndex = getItem(event.button.x, event.button.y);
                    if (itemIndex != -1)
                    {
                        GuiItem& item = m_items[itemIndex];
                        if (item.type == GuiItem::CHECK)
                        {
                            item.checked = !item.checked;
                            m_paint = true;
                        }
                        else if (item.type == GuiItem::BUTTON)
                        {
                            m_paint = true;
                            onButton(itemIndex);
                        }
                        else if (item.type == GuiItem::COMBO)
                        {
                            m_paint = true;
                            m_expandedItem = itemIndex;


                            int fields = item.getComboFields();

                            m_expandedComboScroll = item.currentItem - (fields / 2);
                            if (m_expandedComboScroll < 0)
                            {
                                m_expandedComboScroll = 0;
                            }
                            else if (m_expandedComboScroll + fields > static_cast<int>(item.items.size()))
                            {
                                m_expandedComboScroll = static_cast<int>(item.items.size()) - fields;
                            }
                        }
                    }
                }
                else
                {
                    GuiItem& item = m_items[m_expandedItem];
                    if (item.type == GuiItem::COMBO)
                    {
                        int expandedComboIndex = getExpandedComboItem(event.button.x, event.button.y);
                        int fields = item.getComboFields();

                        if (expandedComboIndex == -2 || expandedComboIndex == -3)
                        {
                            int scrollDelta = expandedComboIndex == -2 ? -1 : 1;
                            m_expandedComboScroll += scrollDelta;
                            if (m_expandedComboScroll < 0)
                            {
                                m_expandedComboScroll = 0;
                            }
                            else if (m_expandedComboScroll + fields > static_cast<int>(item.items.size()))
                            {
                                m_expandedComboScroll = static_cast<int>(item.items.size()) - fields;
                            }
                            m_paint = true;
                        }
                        else if (expandedComboIndex == -1)
                        {
                            m_expandedItem = -1;
                            m_paint = true;
                        }
                        else if (expandedComboIndex == -4)
                        {
                            // do nothing
                        }
                        else
                        {
                            //assert(expandedComboIndex >= 0);
                            // select item from combo box
                            item.currentItem = expandedComboIndex;
                            m_expandedItem = -1;
                            m_paint = true;
                        }
                    }
                }
                mouseStateChanged(event.button.x, event.button.y, false);
            }
            break;
        }
        if (m_eventLoopDone)
            break;

        if (m_paint)
        {
            paint();
            m_paint = false;
        }
    }
}

void GuiDialog::onQuit()
{
    m_eventLoopDone = true;
}

void GuiDialog::onButton(int /*itemIndex*/)
{
}

void GuiDialog::onKeyDown(SDLKey /*k*/)
{
}

void GuiDialog::onKeyUp(SDLKey /*k*/)
{
}
