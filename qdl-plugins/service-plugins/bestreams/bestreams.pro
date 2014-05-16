QT += core network
QT -= gui
TARGET = bestreams
TEMPLATE = lib

HEADERS += \
    bestreams.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    bestreams.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
