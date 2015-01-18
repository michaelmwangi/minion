TEMPLATE = app
CONFIG += console
CONFIG += c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    downloader.cpp \
    minion.cpp

HEADERS += \
    downloader.h \
    downloaderstats.h \
    minion.h \
    constants.h \
    optionparser.h \
    proxyconfiguration.h

unix:!macx: LIBS += -L$$PWD/../../../../usr/local/lib/ -lPocoNet

INCLUDEPATH += $$PWD/../../../../usr/local/include
DEPENDPATH += $$PWD/../../../../usr/local/include

unix:!macx: LIBS += -L$$PWD/../../../../usr/local/lib/ -lPocoFoundation

INCLUDEPATH += $$PWD/../../../../usr/local/include
DEPENDPATH += $$PWD/../../../../usr/local/include


