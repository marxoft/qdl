QT += core network
QT -= gui
TARGET = bliptv
TEMPLATE = lib

INCLUDEPATH += \
    ../interfaces

HEADERS += \
    bliptv.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    bliptv.cpp

icon.files = "$$TARGET".jpg
settings.files = "$$TARGET".xml

unix {
    icon.path = /opt/qdl/icons
    settings.path = /opt/qdl/settings
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon settings
}
