QT += core network
QT -= gui
TARGET = xhamster
TEMPLATE = lib

HEADERS += \
    xhamster.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    xhamster.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
