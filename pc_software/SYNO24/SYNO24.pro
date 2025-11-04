QT       += core gui serialport axcontainer

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    amidite.cpp \
    delay.cpp \
    filemanager.cpp \
    function.cpp \
    killsequence.cpp \
    loghistory.cpp \
    main.cpp \
    progressbar_timer.cpp \
    serial_custom.cpp \
    state_machine.cpp \
    syno24.cpp \
    trityl.cpp \
    volume_manager.cpp

HEADERS += \
    amidite.h \
    delay.h \
    filemanager.h \
    function.h \
    killsequence.h \
    loghistory.h \
    macro.h \
    progressbar_timer.h \
    serial_custom.h \
    state_machine.h \
    struct.h \
    syno24.h \
    trityl.h \
    volume_manager.h

FORMS += \
    killsequence.ui \
    syno24.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resource.qrc
RC_ICONS = SYNO24_LOGO.ico
QXLSX_PARENTPATH=./         # current QXlsx path is . (. means curret directory)
QXLSX_HEADERPATH=./header/  # current QXlsx header path is ./header/
QXLSX_SOURCEPATH=./source/  # current QXlsx source path is ./source/
include(./QXlsx.pri)

DISTFILES += \
    images/1401561986_chevron-left.png \
    images/1401562173_chevron-down.png \
    images/1401562173_chevron-right.png \
    images/1401562173_chevron-up.png \
    images/1401562699_icon-arrow-down-b.png \
    images/1401562699_icon-arrow-up-b.png \
    images/axis_return.png \
    images/axis_return.svg \
    images/axis_zero.png \
    images/axis_zero.svg \
    images/brake.png \
    images/candle.ico \
    images/candle_16.png \
    images/candle_20.png \
    images/candle_256.png \
    images/collapse.png \
    images/collapse_disabled.png \
    images/cube.png \
    images/cubeFront.png \
    images/cubeLeft.png \
    images/cubeTop.png \
    images/cutter.png \
    images/cutter1.ico \
    images/erase_1.png \
    images/expand.png \
    images/expand_disabled.png \
    images/fit_1.png \
    images/g7567.png \
    images/grblControl1.ico \
    images/grblControl2.ico \
    images/guard.png \
    images/guard.svg \
    images/handle2s.png \
    images/handle2s1.png \
    images/handle2sh.png \
    images/handle_horizontal.png \
    images/handle_small.png \
    images/handle_vertical.png \
    images/home.svg \
    images/icon.png \
    images/icon.svg \
    images/icon1.png \
    images/icon1.svg \
    images/icon2.svg \
    images/icon3.svg \
    images/icon3_16.png \
    images/icon3_20.png \
    images/icon3png.png \
    images/icons.svg \
    images/icons_arrow.svg \
    images/laser.svg \
    images/laser1.svg \
    images/list.png \
    images/list_collapsed.png \
    images/menu.png \
    images/menu.svg \
    images/menu_13.png \
    images/menu_16.png \
    images/menu_collapse.png \
    images/menu_collapse_13.png \
    images/menu_collapse_16.png \
    images/num1.png \
    images/num1.svg \
    images/num2.png \
    images/num3.png \
    images/num4.png \
    images/origin.png \
    images/origin.svg \
    images/restart.png \
    images/restart.svg \
    images/run.png \
    images/run.svg \
    images/safe_z.png \
    images/safe_z.svg \
    images/search_for_home2.png \
    images/search_for_z.png \
    images/search_home.svg \
    images/search_z.svg \
    images/send_1.png \
    images/shadow.png \
    images/small_arrow.png \
    images/small_arrow.svg \
    images/unlock.png \
    images/zero_z.png \
    images/zero_z.svg
