QT += core network
QT -= gui
TARGET = filestoshare
TEMPLATE = lib

HEADERS += \
    filestoshare.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    filestoshare.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
