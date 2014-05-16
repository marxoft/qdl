QT += core network
QT -= gui
TARGET = onefichier
TEMPLATE = lib

HEADERS += \
    onefichier.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    onefichier.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
