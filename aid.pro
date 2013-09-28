CONFIG += static
QT     += network

greaterThan(QT_MAJOR_VERSION, 4){
   DEFINES += QT5_HEADERS
   QT += widgets}
else:
{
   DEFINES += QT4_HEADERS
}

TARGET = aid
TEMPLATE = app

SOURCES +=  $$_PRO_FILE_PWD_/src/main.cpp\
            $$_PRO_FILE_PWD_/src/aidMainWindow.cpp \
            $$_PRO_FILE_PWD_/src/Threads.cpp \
            $$_PRO_FILE_PWD_/src/qwPickUpList.cpp \
            $$_PRO_FILE_PWD_/src/qwGridCanvas.cpp \
            $$_PRO_FILE_PWD_/src/qwDecoratedCanvas.cpp \
            $$_PRO_FILE_PWD_/src/qwBottomPanel.cpp \
            $$_PRO_FILE_PWD_/src/globals.cpp \
            $$_PRO_FILE_PWD_/src/CTcpServer.cpp \
            $$_PRO_FILE_PWD_/src/CNormalizator.cpp \
            $$_PRO_FILE_PWD_/src/CNativeData.cpp \
            $$_PRO_FILE_PWD_/src/CBitParser.cpp \
            $$_PRO_FILE_PWD_/src/qwStatusBar.cpp \
            $$_PRO_FILE_PWD_/src/static.cpp

HEADERS +=  $$_PRO_FILE_PWD_/inc/aidMainWindow.h \
            $$_PRO_FILE_PWD_/inc/Threads.h \
            $$_PRO_FILE_PWD_/inc/qwStatusBar.h \
            $$_PRO_FILE_PWD_/inc/qwPickUpList.h \
            $$_PRO_FILE_PWD_/inc/qwGridCanvas.h \
            $$_PRO_FILE_PWD_/inc/qwDecoratedCanvas.h \
            $$_PRO_FILE_PWD_/inc/qwBottomPanel.h \
            $$_PRO_FILE_PWD_/inc/qwAuxDialogs.h \
            $$_PRO_FILE_PWD_/inc/globals.h \
            $$_PRO_FILE_PWD_/inc/defines.h \
            $$_PRO_FILE_PWD_/inc/CTcpServer.h \
            $$_PRO_FILE_PWD_/inc/commons.h \
            $$_PRO_FILE_PWD_/inc/CNormalizator.h \
            $$_PRO_FILE_PWD_/inc/CNativeData.h \
            $$_PRO_FILE_PWD_/inc/CImgContext.h \
            $$_PRO_FILE_PWD_/inc/CBitParser.h

RESOURCES += \
            $$_PRO_FILE_PWD_\media\resources.qrc
