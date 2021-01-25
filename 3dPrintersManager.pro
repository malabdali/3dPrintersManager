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
    core/deviceactions.cpp \
    core/devicecomponent.cpp \
    core/devicefilessystem.cpp \
    core/deviceinfo.cpp \
    core/devicemonitor.cpp \
    core/deviceport.cpp \
    core/deviceportdetector.cpp \
    core/deviceproblemsolver.cpp \
    core/devices.cpp \
    core/fileinfo.cpp \
    core/gcode/deletefile.cpp \
    core/gcode/endstopsstates.cpp \
    core/gcode/fileslist.cpp \
    core/gcode/linenumber.cpp \
    core/gcode/m600.cpp \
    core/gcode/powersupply.cpp \
    core/gcode/preprint.cpp \
    core/gcode/printingstats.cpp \
    core/gcode/reporttemperature.cpp \
    core/gcode/settemperatures.cpp \
    core/gcode/startprinting.cpp \
    core/gcode/stopsdprint.cpp \
    core/gcode/uploadfile.cpp \
    core/gcodecommand.cpp \
    core/printcontroller.cpp \
    core/remoteserver.cpp \
    core/system.cpp \
    core/tasks/printtask.cpp \
    core/tasks/task.cpp \
    core/tasks/tasksmanager.cpp \
    core/utilities/loadgcodefuture.cpp \
    main.cpp \
    ui/deviceswidget.cpp \
    ui/devicewidget.cpp \
    ui/filessystemwidget.cpp \
    ui/mainwindow.cpp \
    ui/printercontrol.cpp \
    ui/serialwidget.cpp\
    core/gcode/devicestats.cpp \
    ui/taskswidget.cpp

HEADERS += \
    core/device.h \
    core/deviceactions.h \
    core/devicecomponent.h \
    core/devicefilessystem.h \
    core/deviceinfo.h \
    core/devicemonitor.h \
    core/deviceport.h \
    core/deviceportdetector.h \
    core/deviceproblemsolver.h \
    core/devices.h \
    core/fileinfo.h \
    core/gcode/deletefile.h \
    core/gcode/endstopsstates.h \
    core/gcode/fileslist.h \
    core/gcode/linenumber.h \
    core/gcode/m600.h \
    core/gcode/powersupply.h \
    core/gcode/preprint.h \
    core/gcode/printingstats.h \
    core/gcode/reporttemperature.h \
    core/gcode/settemperatures.h \
    core/gcode/startprinting.h \
    core/gcode/stopsdprint.h \
    core/gcode/uploadfile.h \
    core/gcodecommand.h \
    core/printcontroller.h \
    core/remoteserver.h \
    core/system.h \
    core/tasks/printtask.h \
    core/tasks/task.h \
    core/tasks/tasksmanager.h \
    core/utilities/loadgcodefuture.h \
    ui/deviceswidget.h \
    ui/devicewidget.h \
    ui/filessystemwidget.h \
    ui/mainwindow.h \
    ui/printercontrol.h \
    ui/serialwidget.h\
    core/gcode/devicestats.h \
    ui/taskswidget.h

FORMS += \
    ui/deviceswidget.ui \
    ui/devicewidget.ui \
    ui/filessystemwidget.ui \
    ui/mainwindow.ui \
    ui/printercontrol.ui \
    ui/serialwidget.ui \
    ui/taskswidget.ui

TRANSLATIONS += \
    resources/3dPrintersManager_en_US.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources/images.qrc

RC_FILE = resources/myapp.rc
