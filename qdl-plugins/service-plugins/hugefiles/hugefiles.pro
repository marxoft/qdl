QT += core network
QT -= gui
TARGET = hugefiles
TEMPLATE = lib

HEADERS += \
    hugefiles.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    hugefiles.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
