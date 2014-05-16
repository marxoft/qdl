QT += core network
QT -= gui
TARGET = freakshare
TEMPLATE = lib

HEADERS += \
    freakshare.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    freakshare.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
