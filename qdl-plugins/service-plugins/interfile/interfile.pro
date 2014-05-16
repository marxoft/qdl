QT += core network
QT -= gui
TARGET = interfile
TEMPLATE = lib

HEADERS += \
    interfile.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    interfile.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
