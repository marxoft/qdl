QT += core network
QT -= gui
TARGET = gofourup
TEMPLATE = lib

HEADERS += \
    gofourup.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    gofourup.cpp

icon.files = "$$TARGET".jpg
settings.files = "$$TARGET".xml

unix {
    icon.path = /opt/qdl/icons
    settings.path = /opt/qdl/settings
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon settings
}
