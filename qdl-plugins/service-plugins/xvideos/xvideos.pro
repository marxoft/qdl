QT += core network
QT -= gui
TARGET = xvideos
TEMPLATE = lib

HEADERS += \
    xvideos.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    xvideos.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
