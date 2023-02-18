#ifndef GUI_H
#define GUI_H

#include <SDL2/SDL.h>

#include <string>
#include <vector>
#include <tuple>

struct GUI_Surface
{
    Uint32 w, h;
    std::vector<Uint32> pixels;
};

int GUI_BlitSurface(const GUI_Surface&    src,
                    const SDL_Rect* srcrect,
                    GUI_Surface&    dst,
                    const SDL_Rect*       dstrect);

int GUI_FillRect(GUI_Surface& dst, const SDL_Rect* rect, Uint32 color);

struct GuiFont
{
    static const int WIDTH;
    static const int HEIGHT;
    static const unsigned char DATA[];
    static const int CHARACTER_COUNT;
    static const int CHARACTER_POSITIONS[];
    static const int CHARACTER_WIDTHS[];
};

struct GuiData
{
    void initData();

    std::vector<Uint32> m_whiteFontPixels;
    GUI_Surface m_whiteFontSurface;

    std::vector<Uint32> m_blackFontPixels;
    GUI_Surface m_blackFontSurface;

    GUI_Surface m_guiComboSurface;
    GUI_Surface m_guiCornersSurface;
    GUI_Surface m_guiTickSurface;

    static const Uint32 COLOR_BACKGROUND1;
    static const Uint32 COLOR_BUTTON;
    static const Uint32 COLOR_MOUSEOVER;
    static const Uint32 COLOR_CLICKED;
    static const Uint32 COLOR_EDGE;
    static const Uint32 COLOR_FRAME;
    static const Uint32 COLOR_SELECTED;

private:
    static Uint32 COMBO_SURFACE_DATA[];
    static const int    COMBO_SURFACE_WIDTH;
    static const int    COMBO_SURFACE_HEIGHT;

    static Uint32 CORNERS_SURFACE_DATA[];
    static const int    CORNERS_SURFACE_WIDTH;
    static const int    CORNERS_SURFACE_HEIGHT;

    static Uint32 TICK_SURFACE_DATA[];
    static const int    TICK_SURFACE_WIDTH;
    static const int    TICK_SURFACE_HEIGHT;
};

// performance-oriented design
struct GuiItem
{
    enum EType
    {
        FRAME,
        LABEL,
        BUTTON,
        CHECK,
        COMBO
    };
    GuiItem(EType type, int x, int y, int width, int height, const std::string& text = std::string()) :
        type(type),
        x(x),
        y(y),
        width(width),
        height(height),
        text(text),
        checked(false),
        items(std::vector<std::string>()),
        currentItem(0),
        itemHeight(20),
        visibleItemCount(2),
        pressedByKey(false)
    {
        //assert(type != COMBO);
    }
    GuiItem(EType type, std::tuple<int, int> xw, std::tuple<int, int> yh, const std::string& text = std::string()) :
        type(type),
        x(std::get<0>(xw)),
        y(std::get<0>(yh)),
        width(std::get<1>(xw)),
        height(std::get<1>(yh)),
        text(text),
        checked(false),
        items(std::vector<std::string>()),
        currentItem(0),
        itemHeight(20),
        visibleItemCount(2),
        pressedByKey(false)
    {
        //assert(type != COMBO);
    }
    GuiItem(EType type, int x, int y, int width, int height, const std::vector<std::string>& items, int currentItem, int itemHeight, int visibleItemCount) :
        type(type),
        x(x),
        y(y),
        width(width),
        height(height),
        text(""),
        checked(false),
        items(items),
        currentItem(currentItem),
        itemHeight(itemHeight),
        visibleItemCount(visibleItemCount),
        pressedByKey(false)
    {
        //assert(type == COMBO);
        //assert(visibleItemCount >= 2);
    }
    GuiItem(EType type, std::tuple<int, int> xw, std::tuple<int, int> yh, const std::vector<std::string>& items, int currentItem, int itemHeight, int visibleItemCount) :
        type(type),
        x(std::get<0>(xw)),
        y(std::get<0>(yh)),
        width(std::get<1>(xw)),
        height(std::get<1>(yh)),
        text(""),
        checked(false),
        items(items),
        currentItem(currentItem),
        itemHeight(itemHeight),
        visibleItemCount(visibleItemCount),
        pressedByKey(false)
    {
        //assert(type == COMBO);
        //assert(visibleItemCount >= 2);
    }

    int getComboFields() const;

    EType type;
    int x, y, width, height;
    std::string text; // for other than combo and frame

    // check data
    bool checked;

    // combo data
    std::vector<std::string> items;
    int currentItem;
    int itemHeight;
    int visibleItemCount;
    bool pressedByKey;
};

class GuiDialog
{
public:
    GuiDialog();

    void init(int width, int height, const char* caption);

    void drawText(const char* text, int x, int y, int width);

    void drawCenteredText(const char* text, int x, int y);

    void paint();

    void execute();

    virtual void onQuit();

    virtual void onButton(int itemIndex);

    virtual void onKeyDown(SDL_Keycode k);

    virtual void onKeyUp(SDL_Keycode k);

    int m_width, m_height;
    std::string m_caption;

    SDL_Window* m_guiWindow;
    SDL_Renderer* m_renderer;
    SDL_Texture* m_texture;

    GUI_Surface m_backSurface;

    std::vector<GuiItem> m_items;

    bool m_whiteFont;

    int m_expandedItem;
    int m_expandedComboScroll;

    int m_state; // 0 - none, 1 - mouseOver, 2 - mouseDown
    int m_stateItem;
    int m_stateExpandedComboItem;

    int m_lastMouseX;
    int m_lastMouseY;


    bool m_paint;

    bool m_eventLoopDone;

    static const int COMBO_ARROW_WIDTH;

private:
    void mouseStateChanged(int x, int y, bool leftButton);

    int getItem(int x, int y);

    int getExpandedComboItem(int x, int y);

    Uint32 stateToColor(int state)
    {
        if (state == 0)
        {
            return GuiData::COLOR_BUTTON;
        }
        else if (state == 1)
        {
            return GuiData::COLOR_MOUSEOVER;
        }
        else
        {
            return GuiData::COLOR_CLICKED;
        }
    }

    GuiData m_guiData;
};

#endif // GUI_H
