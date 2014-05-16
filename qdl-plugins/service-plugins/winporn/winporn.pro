QT += core network xml
QT -= gui
TARGET = winporn
TEMPLATE = lib

HEADERS += \
    winporn.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    winporn.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
