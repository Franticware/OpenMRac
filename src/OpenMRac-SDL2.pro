TEMPLATE = app
CONFIG += console c++20
CONFIG -= app_bundle
CONFIG -= qt

TARGET = openmrac

QMAKE_CXXFLAGS += -fno-exceptions

QMAKE_CXXFLAGS += -DDIR_OPENMRAC_DAT=/usr/share/openmrac/

QMAKE_CXXFLAGS += -DUSE_MINIAL

#LIBS += -lopenal

LIBS += \
        -lSDL2 -lGL -ljpeg -lpng

INCLUDEPATH += \
        /usr/include

SOURCES += \
        3dm.cpp \
        cam.cpp \
        car2d.cpp \
        collider.cpp \
        controls.cpp \
        cstring1.cpp \
        datkey.cpp \
        dpiaware_win32.cpp \
        fopendir.cpp \
        gameaux.cpp \
        gamemenu.cpp \
        gamemng.cpp \
        gamemng_frame.cpp \
        gamemng_load.cpp \
        gbuff_in.cpp \
        ghost.cpp \
        glext1.cpp \
        glhelpers1.cpp \
        gltext.cpp \
        gridlayout.cpp \
        gui.cpp \
        guifont.cpp \
        load_texture.cpp \
        main.cpp \
        mainmenu.cpp \
        matmng.cpp \
        minial.cpp \
        octopus.cpp \
        particles.cpp \
        pict2.cpp \
        pict2_jpeg.cpp \
        pict2_png.cpp \
        rand1.cpp \
        rbsolver.cpp \
        settings_dat.cpp \
        settingsdialog.cpp \
        skysph.cpp \
        soundmng.cpp \
        triboxint.cpp

HEADERS += \
    3dm.h \
    appdefs.h \
    bits.h \
    cam.h \
    car2d.h \
    collider.h \
    controls.h \
    cstring1.h \
    datkey.h \
    fopendir.h \
    gameaux.h \
    gamemng.h \
    gbuff_in.h \
    ghost.h \
    glext1.h \
    glhelpers1.h \
    glm1.h \
    gltext.h \
    gridlayout.h \
    gui.h \
    load_texture.h \
    mainmenu.h \
    matmng.h \
    minial.h \
    octopus.h \
    particles.h \
    pict2.h \
    platform.h \
    rand1.h \
    rbsolver.h \
    settings_dat.h \
    settingsdialog.h \
    skysph.h \
    soundmng.h \
    triboxint.h

DISTFILES += \
    todo.txt
