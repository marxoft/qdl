QT += core network
QT -= gui
TARGET = sockshare
TEMPLATE = lib

HEADERS += \
    sockshare.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    sockshare.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
