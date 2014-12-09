QT += core network
QT -= gui
TARGET = filespace
TEMPLATE = lib

HEADERS += \
    filespace.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    filespace.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
