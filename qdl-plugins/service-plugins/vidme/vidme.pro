QT += core network
QT -= gui
TARGET = vidme
TEMPLATE = lib

HEADERS += \
    vidme.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    vidme.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
