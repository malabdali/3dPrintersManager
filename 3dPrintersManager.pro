QT       += core gui serialport network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    config.h \
    core/device.cpp \
    core/devicefilessystem.cpp \
    core/deviceinfo.cpp \
    core/deviceport.cpp \
    core/deviceportdetector.cpp \
    core/deviceproblemsolver.cpp \
    core/devices.cpp \
    core/gcode/deletefile.cpp \
    core/gcode/devicestats.cpp \
    core/gcode/fileslist.cpp \
    core/gcode/linenumber.cpp \
    core/gcode/powersupply.cpp \
    core/gcode/printingstats.cpp \
    core/gcode/startprinting.cpp \
    core/gcode/uploadfile.cpp \
    core/gcodecommand.cpp \
    core/remoteserver.cpp \
    core/utilities/loadfilefuture.cpp \
    main.cpp \
    ui/deviceswidget.cpp \
    ui/devicewidget.cpp \
    ui/filessystemwidget.cpp \
    ui/mainwindow.cpp \
    ui/serialwidget.cpp

HEADERS += \
    core/device.h \
    core/devicefilessystem.h \
    core/deviceinfo.h \
    core/deviceport.h \
    core/deviceportdetector.h \
    core/deviceproblemsolver.h \
    core/devices.h \
    core/gcode/deletefile.h \
    core/gcode/devicestats.h \
    core/gcode/fileslist.h \
    core/gcode/linenumber.h \
    core/gcode/powersupply.h \
    core/gcode/printingstats.h \
    core/gcode/startprinting.h \
    core/gcode/uploadfile.h \
    core/gcodecommand.h \
    core/remoteserver.h \
    core/utilities/loadfilefuture.h \
    ui/deviceswidget.h \
    ui/devicewidget.h \
    ui/filessystemwidget.h \
    ui/mainwindow.h \
    ui/serialwidget.h

FORMS += \
    ui/deviceswidget.ui \
    ui/devicewidget.ui \
    ui/filessystemwidget.ui \
    ui/mainwindow.ui \
    ui/serialwidget.ui

TRANSLATIONS += \
    resources/3dPrintersManager_en_US.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources/images.qrc
