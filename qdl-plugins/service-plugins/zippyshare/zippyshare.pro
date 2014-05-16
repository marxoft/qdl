QT += core network
QT -= gui
TARGET = zippyshare
TEMPLATE = lib

HEADERS += \
    zippyshare.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    zippyshare.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
