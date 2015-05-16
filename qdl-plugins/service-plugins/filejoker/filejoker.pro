QT += core network
QT -= gui
TARGET = filejoker
TEMPLATE = lib

HEADERS += \
    filejoker.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    filejoker.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
