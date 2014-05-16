QT += core network
QT -= gui
TARGET = fileom
TEMPLATE = lib

HEADERS += \
    fileom.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    fileom.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
