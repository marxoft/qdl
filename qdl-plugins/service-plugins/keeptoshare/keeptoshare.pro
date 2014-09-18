QT += core network
QT -= gui
TARGET = keeptoshare
TEMPLATE = lib

HEADERS += \
    keeptoshare.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    keeptoshare.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
