#include "gui.h"

inline Uint32 GUI_BlendChan(Uint8 s, Uint8 d, Uint8 a)
{
    Uint32 alpha = a;
    Uint32 reva = 255 - alpha;
    Uint32 sc = s;
    Uint32 dc = d;
    return (sc * alpha + dc * reva) / 255;
}

inline void GUI_Blend(Uint32 src, Uint32& dst)
{
    Uint8 alpha = src >> 24;

    Uint8 sr = src >> 16;
    Uint8 sg = src >> 8;
    Uint8 sb = src;

    Uint8 dr = dst >> 16;
    Uint8 dg = dst >> 8;
    Uint8 db = dst;

    dst = 0xff000000 | (GUI_BlendChan(sr, dr, alpha) << 16) | (GUI_BlendChan(sg, dg, alpha) << 8) | (GUI_BlendChan(sb, db, alpha));
}

/* The width and height in srcrect determine the size of the copied rectangle. Only the position is used in the dstrect (the width and height are ignored). Blits with negative dstrect coordinates will be clipped properly.
If srcrect is NULL, the entire surface is copied. If dstrect is NULL, then the destination position (upper left corner) is (0, 0). */
int GUI_BlitSurface(const GUI_Surface&    src,
                    const SDL_Rect* srcrect,
                    GUI_Surface&    dst,
                    const SDL_Rect*       dstrect)
{
    SDL_Rect srcaux;
    if (srcrect == 0)
    {
        srcaux.x = 0;
        srcaux.y = 0;
        srcaux.w = src.w;
        srcaux.h = src.h;
        srcrect = &srcaux;
    }

    SDL_Rect dstaux;
    if (dstrect == 0)
    {
        dstaux.x = 0;
        dstaux.y = 0;
        dstrect = &dstaux;
    }

    for (int y = 0; y != srcrect->h; ++y)
    {
        int yi = y + srcrect->y;
        int yo = y + dstrect->y;
        if (yi >= 0 && yi < int(src.h) && yo >= 0 && yo < int(dst.h))
        {
            for (int x = 0; x != srcrect->w; ++x)
            {
                int xi = x + srcrect->x;
                int xo = x + dstrect->x;
                if (xi >= 0 && xi < int(src.w) && xo >= 0 && xo < int(dst.w))
                {
                    //dst.pixels[xo + yo * dst.w] = src.pixels[xi + yi * src.w];
                    GUI_Blend(src.pixels[xi + yi * src.w], dst.pixels[xo + yo * dst.w]);
                }
            }
        }
    }
    return 0;
}

int GUI_FillRect(GUI_Surface& dst, const SDL_Rect* rect, Uint32 color)
{
    SDL_Rect aux;
    if (rect == 0)
    {
        aux.x = 0;
        aux.y = 0;
        aux.w = dst.w;
        aux.h = dst.h;
        rect = &aux;
    }

    for (int y = 0; y < int(rect->h); ++y)
    {
        int y1 = y + rect->y;
        if (y1 >= 0 && y1 < int(dst.h))
        {
            for (int x = 0; x != rect->w; ++x)
            {
                int x1 = x + rect->x;
                if (x1 >= 0 && x1 < int(dst.w))
                {
                    dst.pixels[x1 + y1 * dst.w] = color;
                }
            }
        }
    }
    return 0;
}


/// GuiData ///

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
    m_whiteFontSurface.w = GuiFont::WIDTH;
    m_whiteFontSurface.h = GuiFont::HEIGHT;
    m_whiteFontSurface.pixels = m_whiteFontPixels;

    m_blackFontPixels.resize(GuiFont::WIDTH * GuiFont::HEIGHT);
    for (int i = 0; i != GuiFont::WIDTH * GuiFont::HEIGHT; ++i)
    {
        Uint32 pixel = GuiFont::DATA[i];
        pixel = pixel << 24;
        pixel |= 0x00000000;
        m_blackFontPixels[i] = pixel;
    }
    m_blackFontSurface.w = GuiFont::WIDTH;
    m_blackFontSurface.h = GuiFont::HEIGHT;
    m_blackFontSurface.pixels = m_blackFontPixels;

    m_guiComboSurface.w = COMBO_SURFACE_WIDTH;
    m_guiComboSurface.h = COMBO_SURFACE_HEIGHT;
    m_guiComboSurface.pixels.assign(COMBO_SURFACE_DATA, COMBO_SURFACE_DATA + m_guiComboSurface.w * m_guiComboSurface.h);

    m_guiCornersSurface.w = CORNERS_SURFACE_WIDTH;
    m_guiCornersSurface.h = CORNERS_SURFACE_HEIGHT;
    m_guiCornersSurface.pixels.assign(CORNERS_SURFACE_DATA, CORNERS_SURFACE_DATA + m_guiCornersSurface.w * m_guiCornersSurface.h);

    m_guiTickSurface.w = TICK_SURFACE_WIDTH;
    m_guiTickSurface.h = TICK_SURFACE_HEIGHT;
    m_guiTickSurface.pixels.assign(TICK_SURFACE_DATA, TICK_SURFACE_DATA + TICK_SURFACE_WIDTH * TICK_SURFACE_HEIGHT);
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

    m_whiteFont = true;

    m_expandedItem = -1;

    m_state = 0;

    m_stateItem = -1;

    m_paint = false;

    m_eventLoopDone = false;

    m_lastMouseX = 0;
    m_lastMouseY = 0;

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
            GUI_BlitSurface(m_whiteFont ? m_guiData.m_whiteFontSurface : m_guiData.m_blackFontSurface, &srcRect, m_backSurface, &dstRect);
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
        GUI_BlitSurface(m_guiData.m_whiteFontSurface, &srcRect, m_backSurface, &dstRect);
        left += GuiFont::CHARACTER_WIDTHS[u];
    }
}

void GuiDialog::paint()
{
    GUI_FillRect(m_backSurface, 0, GuiData::COLOR_BACKGROUND1);
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
            GUI_FillRect(m_backSurface, &rectQuadOuter, GuiData::COLOR_FRAME);

            SDL_Rect rectQuadInner;
            rectQuadInner.x = item.x+1;
            rectQuadInner.y = item.y+1;
            rectQuadInner.w = item.width-2;
            rectQuadInner.h = item.height-2;
            GUI_FillRect(m_backSurface, &rectQuadInner, GuiData::COLOR_BACKGROUND1);
        }
        else if (item.type == GuiItem::COMBO)
        {
            SDL_Rect rectA;
            rectA.x = item.x;
            rectA.y = item.y;
            rectA.w = item.width;
            rectA.h = item.height;
            GUI_FillRect(m_backSurface, &rectA, GuiData::COLOR_EDGE);
            SDL_Rect rectB;
            rectB.x = item.x+1;
            rectB.y = item.y+1;
            rectB.w = item.width-2;
            rectB.h = item.height-2;
            GUI_FillRect(m_backSurface, &rectB, (m_state != 0 && m_stateItem == i && m_expandedItem == -1) ? stateToColor(m_state) : GuiData::COLOR_BUTTON);

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
            GUI_BlitSurface(m_guiData.m_guiCornersSurface, &rectSrc, m_backSurface, &rectDst);

            // top right
            rectSrc.x = 3;
            rectSrc.y = 0;
            rectSrc.w = 3;
            rectSrc.h = 3;
            rectDst.x = item.x+item.width-3;
            rectDst.y = item.y;
            rectDst.w = 3;
            rectDst.h = 3;
            GUI_BlitSurface(m_guiData.m_guiCornersSurface, &rectSrc, m_backSurface, &rectDst);

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
                GUI_BlitSurface(m_guiData.m_guiCornersSurface, &rectSrc, m_backSurface, &rectDst);

                // bottom left
                rectSrc.x = 3;
                rectSrc.y = 3;
                rectSrc.w = 3;
                rectSrc.h = 3;
                rectDst.x = item.x+item.width-3;
                rectDst.y = item.y+item.height-3;
                rectDst.w = 3;
                rectDst.h = 3;
                GUI_BlitSurface(m_guiData.m_guiCornersSurface, &rectSrc, m_backSurface, &rectDst);
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
            GUI_BlitSurface(m_guiData.m_guiComboSurface, &rectSrc, m_backSurface, &rectDst);
        }
        else if (item.type == GuiItem::BUTTON)
        {
            SDL_Rect rectQuadOuter;
            rectQuadOuter.x = item.x;
            rectQuadOuter.y = item.y;
            rectQuadOuter.w = item.width;
            rectQuadOuter.h = item.height;
            GUI_FillRect(m_backSurface, &rectQuadOuter, GuiData::COLOR_EDGE);

            SDL_Rect rectQuadInner;
            rectQuadInner.x = item.x+1;
            rectQuadInner.y = item.y+1;
            rectQuadInner.w = item.width-2;
            rectQuadInner.h = item.height-2;
            GUI_FillRect(m_backSurface, &rectQuadInner, item.pressedByKey ? GuiData::COLOR_CLICKED : ((m_state != 0 && m_stateItem == i && m_expandedItem == -1) ? stateToColor(m_state) : GuiData::COLOR_BUTTON));

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
            GUI_BlitSurface(m_guiData.m_guiCornersSurface, &rectSrc, m_backSurface, &rectDst);

            // top right
            rectSrc.x = 3;
            rectSrc.y = 0;
            rectSrc.w = 3;
            rectSrc.h = 3;
            rectDst.x = item.x+item.width-3;
            rectDst.y = item.y;
            rectDst.w = 3;
            rectDst.h = 3;
            GUI_BlitSurface(m_guiData.m_guiCornersSurface, &rectSrc, m_backSurface, &rectDst);

            // bottom left
            rectSrc.x = 0;
            rectSrc.y = 3;
            rectSrc.w = 3;
            rectSrc.h = 3;
            rectDst.x = item.x;
            rectDst.y = item.y+item.height-3;
            rectDst.w = 3;
            rectDst.h = 3;
            GUI_BlitSurface(m_guiData.m_guiCornersSurface, &rectSrc, m_backSurface, &rectDst);

            // bottom left
            rectSrc.x = 3;
            rectSrc.y = 3;
            rectSrc.w = 3;
            rectSrc.h = 3;
            rectDst.x = item.x+item.width-3;
            rectDst.y = item.y+item.height-3;
            rectDst.w = 3;
            rectDst.h = 3;
            GUI_BlitSurface(m_guiData.m_guiCornersSurface, &rectSrc, m_backSurface, &rectDst);

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
            GUI_FillRect(m_backSurface, &rectDst, GuiData::COLOR_EDGE);

            rectDst.x = item.x + 1;
            rectDst.y = item.y + (item.height - 14) / 2 + 1;
            rectDst.w = 12;
            rectDst.h = 12;
            GUI_FillRect(m_backSurface, &rectDst, (m_state != 0 && m_stateItem == i && m_expandedItem == -1) ? stateToColor(m_state) : GuiData::COLOR_BUTTON);

            if (item.checked)
            {
                rectDst.x = item.x + 2;
                rectDst.y = item.y + (item.height - 14) / 2 + 3;
                rectDst.w = m_guiData.m_guiTickSurface.w;
                rectDst.h = m_guiData.m_guiTickSurface.h;
                GUI_BlitSurface(m_guiData.m_guiTickSurface, 0, m_backSurface, &rectDst);
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
                GUI_FillRect(m_backSurface, &rectDst, GuiData::COLOR_EDGE);

                rectDst.x = item.x + 1;
                rectDst.y = item.y + 1 + item.height - 1;
                rectDst.w = item.width - 2;
                rectDst.h = item.itemHeight * fields + 2 - 2;
                GUI_FillRect(m_backSurface, &rectDst, GuiData::COLOR_BUTTON);

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
                        GUI_FillRect(m_backSurface, &rectDst, GuiData::COLOR_SELECTED);
                    }
                    else if (j + m_expandedComboScroll == m_stateExpandedComboItem)
                    {
                        rectDst.x = item.x + 1;
                        rectDst.y = item.y + item.height + item.itemHeight * j;
                        rectDst.w = item.width - 2;
                        rectDst.h = item.itemHeight;
                        GUI_FillRect(m_backSurface, &rectDst, stateToColor(m_state));
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
                    GUI_FillRect(m_backSurface, &rectDst, m_stateExpandedComboItem == -2 ? stateToColor(m_state) : GuiData::COLOR_BUTTON);

                    rectSrc.x = 0;
                    rectSrc.y = 0;
                    rectSrc.w = 8;
                    rectSrc.h = 4;
                    rectDst.x = item.x+item.width - 8 - 4;
                    rectDst.y = item.y + item.height + (item.itemHeight - 4) / 2;
                    rectDst.w = 8;
                    rectDst.h = 4;
                    GUI_BlitSurface(m_guiData.m_guiComboSurface, &rectSrc, m_backSurface, &rectDst);
                }

                if (m_expandedComboScroll + fields < static_cast<int>(item.items.size()))
                {
                    rectDst.x = item.x + 1 + item.width - 2 - arrowWidth;
                    rectDst.y = item.y + item.height + item.itemHeight * (fields - 1);
                    rectDst.w = arrowWidth;
                    rectDst.h = item.itemHeight;
                    GUI_FillRect(m_backSurface, &rectDst, m_stateExpandedComboItem == -3 ? stateToColor(m_state) : GuiData::COLOR_BUTTON);

                    rectSrc.x = 0;
                    rectSrc.y = 4;
                    rectSrc.w = 8;
                    rectSrc.h = 4;
                    rectDst.x = item.x+item.width - 8 - 4;
                    rectDst.y = item.y + item.height + item.itemHeight * (fields - 1) + (item.itemHeight - 4) / 2;
                    rectDst.w = 8;
                    rectDst.h = 4;
                    GUI_BlitSurface(m_guiData.m_guiComboSurface, &rectSrc, m_backSurface, &rectDst);
                }
            }
        }
    }

    SDL_UpdateTexture(m_texture, NULL, &m_backSurface.pixels[0], m_width * sizeof(Uint32));
    SDL_RenderCopy(m_renderer, m_texture, NULL, NULL);
    SDL_RenderPresent(m_renderer);
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
    m_lastMouseX = x;
    m_lastMouseY = y;
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
    m_guiWindow = SDL_CreateWindow(m_caption.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_width, m_height, 0);

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    m_renderer =
            SDL_CreateRenderer(m_guiWindow, -1, SDL_RENDERER_SOFTWARE | SDL_RENDERER_PRESENTVSYNC);

    m_texture = SDL_CreateTexture(m_renderer,
        SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, m_width, m_height);

    m_backSurface.w = m_width;
    m_backSurface.h = m_height;
    m_backSurface.pixels.resize(m_width * m_height);

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

        case SDL_WINDOWEVENT:
            switch (event.window.event)
            {
            case SDL_WINDOWEVENT_EXPOSED:
            case SDL_WINDOWEVENT_RESIZED:
                SDL_RenderCopy(m_renderer, m_texture, NULL, NULL);
                SDL_RenderPresent(m_renderer);
                break;

            }
            break;

        case SDL_MOUSEMOTION:
            mouseStateChanged(event.motion.x, event.motion.y, event.motion.state & SDL_BUTTON_LMASK);
            break;

        case SDL_KEYDOWN:
            onKeyDown(event.key.keysym.sym);
            break;

        case SDL_KEYUP:
            onKeyUp(event.key.keysym.sym);
            break;

        case SDL_MOUSEWHEEL:
        {
            int delta = 0;
            if (event.wheel.y < 0)
            {
                delta = 1;
            }
            else if (event.wheel.y > 0)
            {
                delta = -1;
            }
            if (delta)
            {
                if (m_expandedItem == -1)
                {
                    int itemIndex = getItem(m_lastMouseX, m_lastMouseY);
                    if (itemIndex != -1)
                    {
                        GuiItem& item = m_items[itemIndex];
                        if (item.type == GuiItem::COMBO)
                        {
                            int currentItemDelta = delta;
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
                        int expandedComboIndex = getExpandedComboItem(m_lastMouseX, m_lastMouseY);
                        if (expandedComboIndex != -1)
                        {
                            int fields = item.getComboFields();
                            int scrollDelta = delta;
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
            }
        }
            break;
        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT)
            {
                mouseStateChanged(event.button.x, event.button.y, true);
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

    SDL_DestroyWindow(m_guiWindow);
}

void GuiDialog::onQuit()
{
    m_eventLoopDone = true;
}

void GuiDialog::onButton(int /*itemIndex*/)
{
}

void GuiDialog::onKeyDown(SDL_Keycode /*k*/)
{
}

void GuiDialog::onKeyUp(SDL_Keycode /*k*/)
{
}
