#include "settingsdialog.h"

#include <cstring>

#include "gl1.h"

SettingsDialog::SettingsDialog(
    int renderer,
    const std::vector<ScreenMode>& screenModes,
    ScreenMode currentScreenMode,
    ScreenMode defaultScreenMode,
    bool currentVsyncState,
    int currentAntialiasingMode,
    int currentTextureFilter,
    bool currentFpsState,
    //
    const std::vector<std::string>& openalDevices,
    const char* currentOpenalDevice,
    int currentRateIndex,
    bool lowLatency,
    bool minial
    ) :
    m_horLay(0, 8),
    m_horBtnLay(0, 8),
    m_verLay(0, 8)
{
    m_continue = true;

    #ifdef DISABLE_ANISOTROPY
    if (currentTextureFilter > 1)
        currentTextureFilter = 1;
    #endif

    const int CHECK_COMBO_HEIGHT = 22;
    const int LABEL_HEIGHT = 18;
    const int FRAME_MARGIN = 4;

    m_horLay.add(1);
    m_horLay.add(FRAME_MARGIN);
    m_horLay.add(105);
    m_horLay.add(110);
    m_horLay.add(30);
    m_horLay.add(160);
    m_horLay.add(FRAME_MARGIN);
    m_horLay.add(1);

    m_verLay.add(LABEL_HEIGHT); // Video
    m_verLay.add(2);
    m_verLay.add(1);
    m_verLay.add(FRAME_MARGIN);
    m_verLay.add(CHECK_COMBO_HEIGHT); // Renderer
    m_verLay.add(2);
    m_verLay.add(CHECK_COMBO_HEIGHT); // Resolution
    m_verLay.add(2);
    m_verLay.add(CHECK_COMBO_HEIGHT); // Enable VSync, Show Framerate
    m_verLay.add(2);
    m_verLay.add(CHECK_COMBO_HEIGHT); // Antialiasing
    m_verLay.add(2);
    m_verLay.add(CHECK_COMBO_HEIGHT); // Texture Filter
    m_verLay.add(0);
    m_verLay.add(0);
    m_verLay.add(FRAME_MARGIN);
    m_verLay.add(1);
    m_verLay.add(8);
    m_verLay.add(LABEL_HEIGHT); // Audio
    m_verLay.add(2);
    m_verLay.add(1);
    m_verLay.add(FRAME_MARGIN);
    m_verLay.add(CHECK_COMBO_HEIGHT); // Device
    m_verLay.add(2);
    m_verLay.add(CHECK_COMBO_HEIGHT); // Low Latency
    m_verLay.add(FRAME_MARGIN);
    m_verLay.add(1);
    m_verLay.add(16);
    m_verLay.add(26); // Buttons

    static const char* windowTitle = "OpenMRac Settings";

    init(m_horLay.getSize(), m_verLay.getSize(), windowTitle);

    m_items.push_back(GuiItem(GuiItem::LABEL, m_horLay.getCellPS(0, 6), m_verLay.getCellPS(0, 1), "Video"));
    m_items.push_back(GuiItem(GuiItem::FRAME, m_horLay.getCellPS(0, 8), m_verLay.getCellPS(2, 15)));

    char audioStrBuff[64] = {0};
    if (minial)
    {
        snprintf(audioStrBuff, 63, "Audio (SDL/%s)", SDL_GetCurrentAudioDriver());
    }
    else
    {
        strncpy(audioStrBuff, "Audio (OpenAL)", 63);
    }
    m_items.push_back(GuiItem(GuiItem::LABEL, m_horLay.getCellPS(0, 6), m_verLay.getCellPS(18, 1), audioStrBuff));
    m_items.push_back(GuiItem(GuiItem::FRAME, m_horLay.getCellPS(0, 8), m_verLay.getCellPS(20, 6)));

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

    m_items.push_back(GuiItem(GuiItem::LABEL, m_horLay.getCellPS(2, 1), m_verLay.getCellPS(4, 1), "Renderer"));

    std::vector<std::string> renderers;
    const char* strProfiles[3] = {"OpenGL Compat. profile", "OpenGL ES 2", "OpenGL 3.3 Core profile"};
    for (int i = PROFILE_MIN; i <= PROFILE_MAX; ++i)
    {
        renderers.push_back(strProfiles[i]);
    }

    m_rendererComboIndex = m_items.size();
    m_items.push_back(GuiItem(GuiItem::COMBO, m_horLay.getCellPS(3, 3), m_verLay.getCellPS(4, 1), renderers, renderer - PROFILE_MIN, 20, 6));

    m_items.push_back(GuiItem(GuiItem::LABEL, m_horLay.getCellPS(2, 1), m_verLay.getCellPS(6, 1), "Mode"));
    m_resolutionsComboIndex = m_items.size();
    m_items.push_back(GuiItem(GuiItem::COMBO, m_horLay.getCellPS(3, 3), m_verLay.getCellPS(6, 1), resolutions, currentScreenModeIndex, 20, 7));
    m_vsyncIndex = m_items.size();
    m_items.push_back(GuiItem(GuiItem::CHECK, m_horLay.getCellPS(2, 2), m_verLay.getCellPS(8, 1), "Enable VSync"));
    m_items[m_vsyncIndex].checked = currentVsyncState;

    m_items.push_back(GuiItem(GuiItem::LABEL, m_horLay.getCellPS(2, 1), m_verLay.getCellPS(10, 1), "Antialiasing"));
    std::vector<std::string> antialiasingModes;
    antialiasingModes.push_back("Off");
    antialiasingModes.push_back("2x");
    antialiasingModes.push_back("4x");
    m_antialiasingIndex = m_items.size();
    m_items.push_back(GuiItem(GuiItem::COMBO, m_horLay.getCellPV(3, 50), m_verLay.getCellPS(10, 1), antialiasingModes, currentAntialiasingMode, 20, 3));

    m_items.push_back(GuiItem(GuiItem::LABEL, m_horLay.getCellPS(2, 1), m_verLay.getCellPS(12, 1), "Texture Filter"));
    std::vector<std::string> textureFilters;
    textureFilters.push_back("Bilinear");
    textureFilters.push_back("Trilinear");

    #ifndef DISABLE_ANISOTROPY
    ASSERT_ANISOTROPY
    textureFilters.push_back("Anisotropic");
    #endif

    m_textureFilterIndex = m_items.size();
    m_items.push_back(GuiItem(GuiItem::COMBO, m_horLay.getCellPV(3, 110), m_verLay.getCellPS(12, 1), textureFilters, currentTextureFilter, 20, 3));

    m_showFpsIndex = m_items.size();
    m_items.push_back(GuiItem(GuiItem::CHECK, m_horLay.getCellPS(5, 2), m_verLay.getCellPS(8, 1), "Show Framerate"));
    m_items[m_showFpsIndex].checked = currentFpsState;

    m_items.push_back(GuiItem(GuiItem::LABEL, m_horLay.getCellPS(2, 1), m_verLay.getCellPS(22, 1), "Device"));

    m_lowLatencyIndex = m_items.size();
    m_items.push_back(GuiItem(GuiItem::CHECK, m_horLay.getCellPS(5, 1), m_verLay.getCellPS(24, 1), "Low Latency", !minial));
    m_items[m_lowLatencyIndex].checked = lowLatency;

    m_items.push_back(GuiItem(GuiItem::LABEL, m_horLay.getCellPS(2, 1), m_verLay.getCellPS(24, 1), "Rate", !minial));

    std::vector<std::string> rateVec;
    rateVec.push_back("22050 Hz");
    rateVec.push_back("44100 Hz");
    rateVec.push_back("48000 Hz");
    m_freqComboIndex = m_items.size();
    m_items.push_back(GuiItem(GuiItem::COMBO, m_horLay.getCellPS(3, 1), m_verLay.getCellPS(24, 1), rateVec, currentRateIndex, 20, 2, !minial));

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
    m_items.push_back(GuiItem(GuiItem::COMBO, m_horLay.getCellPS(3, 3), m_verLay.getCellPS(22, 1), comboOalDevices, currentOpenalDeviceIndex, 20, 3));

    const int BUTTON_SPACING = 8;
    const int widthOfButtons = m_horLay.getCellS(0, 8);
    const int widthOfButtonOk = 95;
    const int widthOfButtonCancel = widthOfButtonOk;
    const int widthOfButtonDefaults = widthOfButtons - BUTTON_SPACING * 2 - widthOfButtonOk - widthOfButtonCancel;

    m_horBtnLay.add(widthOfButtonOk);
    m_horBtnLay.add(BUTTON_SPACING);
    m_horBtnLay.add(widthOfButtonCancel);
    m_horBtnLay.add(BUTTON_SPACING);
    m_horBtnLay.add(widthOfButtonDefaults);

    m_okButtonIndex = m_items.size();
    m_items.push_back(GuiItem(GuiItem::BUTTON, m_horBtnLay.getCellPS(0, 1), m_verLay.getCellPS(28, 1), "OK"));
    m_cancelButtonIndex = m_items.size();
    m_items.push_back(GuiItem(GuiItem::BUTTON, m_horBtnLay.getCellPS(2, 1), m_verLay.getCellPS(28, 1), "Cancel"));
    m_defaultsButtonIndex = m_items.size();
    m_items.push_back(GuiItem(GuiItem::BUTTON, m_horBtnLay.getCellPS(4, 1), m_verLay.getCellPS(28, 1), "Set to Defaults"));

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
        m_items[m_rendererComboIndex].currentItem = DEFAULT_PROFILE;
        m_items[m_resolutionsComboIndex].currentItem = m_defaultScreenModeIndex;
        m_items[m_vsyncIndex].checked = false;

        m_items[m_antialiasingIndex].currentItem = 0;
        m_items[m_textureFilterIndex].currentItem = 1;

        m_items[m_showFpsIndex].checked = false;
        m_items[m_audioDevicesComboIndex].currentItem = 0;
        m_items[m_freqComboIndex].currentItem = 2;
        m_items[m_lowLatencyIndex].checked = true;
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

int SettingsDialog::getRenderer()
{
    const GuiItem& item = m_items[m_rendererComboIndex];
    return item.currentItem + PROFILE_MIN;
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

int SettingsDialog::getFreqIndex() const
{
    const GuiItem& item = m_items[m_freqComboIndex];
    return item.currentItem;
}

bool SettingsDialog::getLowLatencyChecked() const
{
    return m_items[m_lowLatencyIndex].checked;
}
