QT += core network
QT -= gui
TARGET = rapidshare
TEMPLATE = lib

HEADERS += \
    rapidshare.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    rapidshare.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
