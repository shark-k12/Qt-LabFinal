QT       += core gui sql charts widgets printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# 设置QXlsx的路径（QXlsx文件夹和pro文件同级）
QXLSX_PARENTPATH = ./QXlsx         # QXlsx根目录
QXLSX_HEADERPATH = ./QXlsx/header/ # 头文件路径
QXLSX_SOURCEPATH = ./QXlsx/source/ # 源文件路径
include($$QXLSX_PARENTPATH/QXlsx.pri) # 引入QXlsx的编译配置

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    aboutdialog.cpp \
    main.cpp \
    mainwindow.cpp \
    remindersettingdialog.cpp \
    reminderthread.cpp \
    taskdbmanager.cpp \
    taskdialog.cpp \
    taskstatistic.cpp

HEADERS += \
    aboutdialog.h \
    mainwindow.h \
    remindersettingdialog.h \
    reminderthread.h \
    taskdbmanager.h \
    taskdialog.h \
    taskstatistic.h

FORMS += \
    aboutdialog.ui \
    mainwindow.ui \
    remindersettingdialog.ui \
    taskdialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
