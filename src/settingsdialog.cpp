#include "settingsdialog.h"
#include "glext1.h"
#include "platform.h"

SettingsDialog::SettingsDialog(const std::vector<ScreenMode>& screenModes, ScreenMode currentScreenMode, ScreenMode defaultScreenMode, bool currentVsyncState,
    int currentAntialiasingMode,
    int currentTextureFilter,
    bool currentFpsState,
    //
    const std::vector<std::string>& openalDevices, const char* currentOpenalDevice
    ) :
    m_layout(0, 0, 8, 8, 0, 0)
{
    m_continue = true;

    #ifdef DISABLE_ANISOTROPY
    if (currentTextureFilter > 1)
        currentTextureFilter = 1;
    #endif

    const int CHECK_COMBO_HEIGHT = 22;
    const int LABEL_HEIGHT = 18;
    const int FRAME_MARGIN = 4;

    m_layout.addColumn(1);
    m_layout.addColumn(FRAME_MARGIN);
    m_layout.addColumn(105);
    m_layout.addColumn(300);
    m_layout.addColumn(FRAME_MARGIN);
    m_layout.addColumn(1);

    m_layout.addRow(LABEL_HEIGHT); // Video
    m_layout.addRow(2);
    m_layout.addRow(1);
    m_layout.addRow(FRAME_MARGIN);
    m_layout.addRow(CHECK_COMBO_HEIGHT); // Resolution
    m_layout.addRow(2);
    m_layout.addRow(CHECK_COMBO_HEIGHT); // Enable VSync
    m_layout.addRow(2);

    m_layout.addRow(CHECK_COMBO_HEIGHT); // Antialiasing
    m_layout.addRow(2);
    m_layout.addRow(CHECK_COMBO_HEIGHT); // Texture Filter
    m_layout.addRow(2);

    m_layout.addRow(CHECK_COMBO_HEIGHT); // Show Framerate
    m_layout.addRow(FRAME_MARGIN);
    m_layout.addRow(1);
    m_layout.addRow(8);
    m_layout.addRow(LABEL_HEIGHT); // Audio
    m_layout.addRow(2);
    m_layout.addRow(1);
    m_layout.addRow(FRAME_MARGIN);
    m_layout.addRow(CHECK_COMBO_HEIGHT); // Device
    m_layout.addRow(FRAME_MARGIN);
    m_layout.addRow(1);
    m_layout.addRow(16);
    m_layout.addRow(26); // Buttons

    static const char* windowTitle = "OpenMRac Settings";

    init(m_layout.getBoxWidth(), m_layout.getBoxHeight(), windowTitle);

    m_items.push_back(GuiItem(GuiItem::LABEL, m_layout.getCellX(0), m_layout.getCellY(0), m_layout.getCellWidth(0, 6), m_layout.getCellHeight(0, 1), "Video"));
    m_items.push_back(GuiItem(GuiItem::FRAME, m_layout.getCellX(0), m_layout.getCellY(2), m_layout.getCellWidth(0, 6), m_layout.getCellHeight(2, 9+4)));
    m_items.push_back(GuiItem(GuiItem::LABEL, m_layout.getCellX(0), m_layout.getCellY(12+4), m_layout.getCellWidth(0, 6), m_layout.getCellHeight(12+4, 1),
                          #ifdef USE_MINIAL
                              "Audio (SDL)"
                          #else
                              "Audio (OpenAL)"
                          #endif
                              ));
    m_items.push_back(GuiItem(GuiItem::FRAME, m_layout.getCellX(0), m_layout.getCellY(14+4), m_layout.getCellWidth(0, 6), m_layout.getCellHeight(14+4, 5)));

    std::vector<std::string> resolutions;
    for (int i = 0; i != static_cast<int>(screenModes.size()); ++i)
    {
        char buff[1024] = {0};
        if (!screenModes[i].fullscreen)
        {
            snprintf(buff, 1023, "%dx%d %s%s", screenModes[i].width, screenModes[i].height, screenModes[i].getAspectRatioString().c_str(), (screenModes[i].fullscreen ? "Fullscreen" : "Windowed"));
        }
        else
        {
            snprintf(buff, 1023, "Fullscreen");
        }
        resolutions.push_back(buff);
    }

    int currentScreenModeIndex = std::find(screenModes.begin(), screenModes.end(), currentScreenMode) - screenModes.begin();
    m_defaultScreenModeIndex = std::find(screenModes.begin(), screenModes.end(), defaultScreenMode) - screenModes.begin();

    m_items.push_back(GuiItem(GuiItem::LABEL, m_layout.getCellX(2), m_layout.getCellY(4), m_layout.getCellWidth(2, 1), m_layout.getCellHeight(4, 1), "Mode"));
    m_resolutionsComboIndex = m_items.size();
    m_items.push_back(GuiItem(GuiItem::COMBO, m_layout.getCellX(3), m_layout.getCellY(4), m_layout.getCellWidth(3, 1), m_layout.getCellHeight(4, 1), resolutions, currentScreenModeIndex, 20, 7));
    m_vsyncIndex = m_items.size();
    m_items.push_back(GuiItem(GuiItem::CHECK, m_layout.getCellX(2), m_layout.getCellY(6), m_layout.getCellWidth(2, 2), m_layout.getCellHeight(6, 1),
#if USE_VSYNC
        "Enable VSync"
#else
        "Limit Framerate"
#endif
        ));
    m_items[m_vsyncIndex].checked = currentVsyncState;


    m_items.push_back(GuiItem(GuiItem::LABEL, m_layout.getCellX(2), m_layout.getCellY(8), m_layout.getCellWidth(2, 1), m_layout.getCellHeight(8, 1), "Antialiasing"));
    std::vector<std::string> antialiasingModes;
    antialiasingModes.push_back("Off");
    antialiasingModes.push_back("2x");
    antialiasingModes.push_back("4x");
    m_antialiasingIndex = m_items.size();
    m_items.push_back(GuiItem(GuiItem::COMBO, m_layout.getCellX(3), m_layout.getCellY(8), /*m_layout.getCellWidth(3, 1)*/50, m_layout.getCellHeight(8, 1), antialiasingModes, currentAntialiasingMode, 20, 3));

    m_items.push_back(GuiItem(GuiItem::LABEL, m_layout.getCellX(2), m_layout.getCellY(10), m_layout.getCellWidth(2, 1), m_layout.getCellHeight(10, 1), "Texture Filter"));
    std::vector<std::string> textureFilters;
    textureFilters.push_back("Bilinear");
    textureFilters.push_back("Trilinear");

    #ifndef DISABLE_ANISOTROPY
    ASSERT_ANISOTROPY
    textureFilters.push_back("Anisotropic");
    #endif

    m_textureFilterIndex = m_items.size();
    m_items.push_back(GuiItem(GuiItem::COMBO, m_layout.getCellX(3), m_layout.getCellY(10), /*m_layout.getCellWidth(3, 1)*/110, m_layout.getCellHeight(10, 1), textureFilters, currentTextureFilter, 20, 3));


    m_showFpsIndex = m_items.size();
    m_items.push_back(GuiItem(GuiItem::CHECK, m_layout.getCellX(2), m_layout.getCellY(8+4), m_layout.getCellWidth(2, 2), m_layout.getCellHeight(8+4, 1), "Show Framerate"));
    m_items[m_showFpsIndex].checked = currentFpsState;
    m_items.push_back(GuiItem(GuiItem::LABEL, m_layout.getCellX(2), m_layout.getCellY(16+4), m_layout.getCellWidth(2, 1), m_layout.getCellHeight(16+4, 1), "Device"));

    std::vector<std::string> comboOalDevices;
    comboOalDevices.push_back("Default");

    m_openalDevices = openalDevices;

    int currentOpenalDeviceIndex = 0;

    for (int i = 0; i != static_cast<int>(openalDevices.size()); ++i)
    {
        comboOalDevices.push_back(openalDevices[i]);
        if (currentOpenalDevice && currentOpenalDevice == openalDevices[i])
        {
            currentOpenalDeviceIndex = i + 1;
        }
    }

    m_audioDevicesComboIndex = m_items.size();
    m_items.push_back(GuiItem(GuiItem::COMBO, m_layout.getCellX(3), m_layout.getCellY(16+4), m_layout.getCellWidth(3, 1), m_layout.getCellHeight(16+4, 1), comboOalDevices, currentOpenalDeviceIndex, 20, 2));

    const int BUTTON_SPACING = 8;

    int widthOfButtons = m_layout.getCellWidth(1, 4);
    int widthOfButtonOk = 95;
    int widthOfButtonCancel = widthOfButtonOk;
    int widthOfButtonDefaults = widthOfButtons - BUTTON_SPACING * 2 - widthOfButtonOk - widthOfButtonCancel;

    m_okButtonIndex = m_items.size();
    m_items.push_back(GuiItem(GuiItem::BUTTON, m_layout.getCellX(1), m_layout.getCellY(20+4), widthOfButtonOk, m_layout.getCellHeight(20+4, 1), "OK"));
    m_cancelButtonIndex = m_items.size();
    m_items.push_back(GuiItem(GuiItem::BUTTON, m_layout.getCellX(1) + widthOfButtonOk + BUTTON_SPACING, m_layout.getCellY(20+4), widthOfButtonCancel, m_layout.getCellHeight(20+4, 1), "Cancel"));
    m_defaultsButtonIndex = m_items.size();
    m_items.push_back(GuiItem(GuiItem::BUTTON, m_layout.getCellX(1) + widthOfButtonOk + BUTTON_SPACING + widthOfButtonCancel + BUTTON_SPACING, m_layout.getCellY(20+4), widthOfButtonDefaults, m_layout.getCellHeight(20+4, 1), "Set to Defaults"));

    m_returnPressed = false;
    m_enterPressed = false;
    m_escPressed = false;
}

void SettingsDialog::onQuit()
{
    GuiDialog::onQuit();
    m_continue = false;
}

void SettingsDialog::onButton(int itemIndex)
{
    if (itemIndex == m_okButtonIndex)
    {
        m_continue = true;
        m_eventLoopDone = true;
    }
    else if (itemIndex == m_cancelButtonIndex)
    {
        m_continue = false;
        m_eventLoopDone = true;
    }
    else if (itemIndex == m_defaultsButtonIndex)
    {
        m_items[m_resolutionsComboIndex].currentItem = m_defaultScreenModeIndex;
        m_items[m_vsyncIndex].checked = false;

        m_items[m_antialiasingIndex].currentItem = 0;
        m_items[m_textureFilterIndex].currentItem = 1;

        m_items[m_showFpsIndex].checked = false;
        m_items[m_audioDevicesComboIndex].currentItem = 0;
    }
}

void SettingsDialog::onKeyDown(SDL_Keycode k)
{
    if (k == SDLK_KP_ENTER)
    {
        m_enterPressed = true;
        m_paint = true;
        m_items[m_okButtonIndex].pressedByKey = true;
    }
    else if (k == SDLK_RETURN)
    {
        m_returnPressed = true;
        m_paint = true;
        m_items[m_okButtonIndex].pressedByKey = true;
    }
    else if (k == SDLK_ESCAPE)
    {
        m_escPressed = true;
        m_paint = true;
        m_items[m_cancelButtonIndex].pressedByKey = true;
    }
}

void SettingsDialog::onKeyUp(SDL_Keycode k)
{
    if (k == SDLK_KP_ENTER && m_enterPressed)
    {
        m_enterPressed = false;
        m_paint = true;
        m_continue = true;
        m_eventLoopDone = true;
        m_items[m_okButtonIndex].pressedByKey = false;
    }
    else if (k == SDLK_RETURN && m_returnPressed)
    {
        m_returnPressed = false;
        m_paint = true;
        m_continue = true;
        m_eventLoopDone = true;
        m_items[m_okButtonIndex].pressedByKey = false;
    }
    else if (k == SDLK_ESCAPE && m_escPressed)
    {
        m_escPressed = false;
        m_paint = true;
        m_continue = false;
        m_eventLoopDone = true;
        m_items[m_cancelButtonIndex].pressedByKey = false;
    }
}

int SettingsDialog::getSelectedScreenMode() const
{
    const GuiItem& item = m_items[m_resolutionsComboIndex];
    return item.currentItem;
}

bool SettingsDialog::getVsyncChecked() const
{
    return m_items[m_vsyncIndex].checked;
}

int SettingsDialog::getAntialiasingMode() const
{
    const GuiItem& item = m_items[m_antialiasingIndex];
    return item.currentItem;
}

int SettingsDialog::getTextureFilter() const
{
    const GuiItem& item = m_items[m_textureFilterIndex];
    return item.currentItem;
}

bool SettingsDialog::getShowFpsChecked() const
{
    return m_items[m_showFpsIndex].checked;
}

const char* SettingsDialog::getOpenalDevice() const
{
    const GuiItem& item = m_items[m_audioDevicesComboIndex];
    if (item.currentItem == 0) // Default
    {
        return 0;
    }
    else
    {
        return m_openalDevices[item.currentItem - 1].c_str();
    }
}
